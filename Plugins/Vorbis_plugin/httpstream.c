/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
** Copyright (C) Jack Moffit (jack@icecast.org)
** Copyright (C) Michael Smith (msmith@labyrinth.net.au)
** Copyright (C) Aaron Porter (aaron@javasource.org)
**
** http://cdexos.sourceforge.net/
** http://sourceforge.net/projects/cdexos 
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <string.h>
#include <process.h>
#include <commctrl.h>

#include <vorbis/vorbisfile.h>

#include "httpstream.h"
#include "resource.h"

#define OGG_PROTOCOL "http://"
#define OGG_MIME_TYPE "application/x-ogg"
#define USER_AGENT "OggVorbis Plugin"
#define CIRCULAR_BUFFER_SIZE 65536 // 64K should be plenty

typedef struct
{
    SOCKET  tcpSocket;

    int     killBufferThread; // trigger to kill buffering thread
    HANDLE  bufferThreadHandle;	// the handle to the http buffering thread

    int     circularBufferStart;
    int     circularBufferEnd;

    long    bytesRead;

    char    circularBuffer[CIRCULAR_BUFFER_SIZE];
    char    httpResponseHeader[ 16384 ]; // 16K HTTP header should be enough
} BufferThreadData;

char    lastUrlChecked[ 2048 ];
int     lastUrlOgg = 0;

char    proxy[ 256 ];

HWND    hwndPlayer = 0;
HWND    hwndMessage = 0;

WINDOWPOS messageWindowPos;

HWND    hwndText = 0;
HWND    hwndProgress = 0;

void    BufferThread(void *vp); // the buffer thread procedure

char    iniFilename[ MAX_PATH ];

BufferThreadData * createBufferThreadData()
{
    BufferThreadData * pData = malloc(sizeof(BufferThreadData));

    if (!pData)
        return 0;

    pData->tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    pData->killBufferThread = 0;
    pData->bufferThreadHandle = INVALID_HANDLE_VALUE;
    pData->circularBufferStart = 0;
    pData->circularBufferEnd = 0;
    pData->bytesRead = 0;
    *(pData->httpResponseHeader) = 0;

    return pData;
}

void showMessage(const char * message)
{
    RECT playerRect;
    RECT messageRect;

    int top;

    if (!hwndMessage || !hwndText || !hwndProgress)
        return;

    ShowWindow(hwndProgress, SW_HIDE);
    ShowWindow(hwndText, SW_SHOW);

    SetWindowText(hwndText, message);

    GetWindowRect(hwndPlayer, &playerRect);
    GetWindowRect(hwndMessage, &messageRect);

    top = playerRect.top - (messageRect.bottom - messageRect.top);

    if (top < 0)
        top = 0;

    MoveWindow(hwndMessage, playerRect.left, top, messageRect.right - messageRect.left, messageRect.bottom - messageRect.top, TRUE);

    ShowWindow(hwndMessage, SW_SHOW);

    UpdateWindow(hwndMessage);
}

void showError(const char * message)
{
    showMessage(message);

    SetTimer(hwndMessage, 2, 5000, NULL);
}

void showPercentage(int percentage)
{
    PostMessage(hwndProgress, PBM_SETPOS, (WPARAM) percentage, 0);

    RedrawWindow(hwndMessage, 0, 0, 0);
}

void hideMessage()
{
    if (hwndMessage)
        ShowWindow(hwndMessage, SW_HIDE);
}

int readHttpResponseHeader(BufferThreadData * pThreadData)
{
	int index;
	char * space;

	for (index = 0; index < 8191; index++)
	{
		if (recv(pThreadData->tcpSocket, pThreadData->httpResponseHeader + index, 1, 0) == SOCKET_ERROR)
			return -1;

		if (pThreadData->httpResponseHeader[index] == '\r')
			index--; // discard any CRs
        else if ((index > 0) && (pThreadData->httpResponseHeader[index] == '\n') && (pThreadData->httpResponseHeader[index-1] == '\n'))
			break; // "\n\n" ends the HTTP header
	}

	pThreadData->httpResponseHeader[index + 1] = 0;

	space = strchr(pThreadData->httpResponseHeader, ' ');

	if (space)
	{
		char status[4];

		for (index = 0; index < 3; index++)
			status[index] = *(space + 1 + index);

		status[3] = 0;

		return atoi(status);
	}

	return -1;
}

int httpConnect(const char * constUrl, BufferThreadData * pThreadData, BOOL getContent, BOOL showMessages)
{
    char * url;
	char * colon;
	char * slash;
	char * server;
	int port;
    unsigned long ulAddr=0;
	struct sockaddr_in sockaddr;
	struct hostent * host_ent;
	char buff[2048];
	int status;

	char uri[2048];

	uri[0] = 0;
        	
	url = strdup(constUrl + strlen(OGG_PROTOCOL));

	colon = strchr(url, ':');
	slash = strchr(url, '/');

    if (*proxy)
        colon = strchr(proxy, ':');

	if (colon)
		*colon = 0;

	if (slash)
		*slash = 0;

    if (*proxy)
        server = proxy;
    else
	    server = url;

    if (getContent && showMessages)
    {
        _snprintf(buff, 2048, "Looking up %s...", server);
        showMessage(buff);
    }

	host_ent = gethostbyname(server);

	if (!host_ent)
	{
		_snprintf(buff, 2048, "Couldn't find server %s!", server);

        showError(buff);

		return 0;
	}

    memcpy(&sockaddr.sin_addr, host_ent->h_addr, host_ent->h_length);

	port = 80;

	if (colon && (!slash || (colon < slash)))
		port = atoi(colon + 1);

	if (slash)
	{
		int index;
		int count;

		*slash = '/';

		count = strlen(slash);

		for (index = 0; index < count; index++) // URL encode
		{
			char c = slash[index];
			
			if ((c == '/') || (c == '.') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))
				strncat(uri, slash + index, 1);
			else
			{
				int uriLength;

				strcat(uri, "%");

				uriLength = strlen(uri);

				_snprintf(uri + uriLength, 2048 - uriLength, "%X", c);
			}
		}

        *slash = 0;
	}
	else
		strcpy(uri, "/");

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons((unsigned short) port);

    if (getContent && showMessages)
    {
        _snprintf(buff, 2048, "Connecting to %s...", server);
        showMessage(buff);
    }

	if (connect(pThreadData->tcpSocket, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		_snprintf(buff, 2048, "Couldn't connect to server %s!", server);
		
        showError(buff);
		return 0;
	}

    if (getContent && showMessages)
    {
        _snprintf(buff, 2048, "Requesting %s...", uri);
        showMessage(buff);
    }

    if (*proxy)
    {
        server = url;

	    colon = strchr(server, ':');

	    if (colon)
        {
		    *colon = 0;
		    port = atoi(colon + 1);
        }
        else
	        port = 80;

        _snprintf(buff, 2048, "%s http://%s:%d%s HTTP/1.0\nHost: %s\nUser-Agent: %s\nAccept: %s, */*\n\n", (getContent ? "GET" : "HEAD"), server, port, uri, server, USER_AGENT, OGG_MIME_TYPE);
    }
    else
        _snprintf(buff, 2048, "%s %s HTTP/1.0\nHost: %s\nUser-Agent: %s\nAccept: %s, */*\n\n", (getContent ? "GET" : "HEAD"), uri, server, USER_AGENT, OGG_MIME_TYPE);

	if (send(pThreadData->tcpSocket, buff, strlen(buff), 0) == SOCKET_ERROR)
	{
		_snprintf(buff, 2048, "Couldn't send request to server %s!", server);

        showError(buff);

        return 0;
	}

	status = readHttpResponseHeader(pThreadData);

	if (status != 200)
	{
        hideMessage();

		if (status == -1)
		{
			_snprintf(buff, 2048, "Error reading response from %s!", server);

            showError(buff);
			
			return 0;
		}

        _snprintf(buff, 2048, "Server Error: %s", strchr(pThreadData->httpResponseHeader, ' ') + 1);

        showError(buff);

		return 0;
	}

	free(url);

	return 1;
}

char * getHttpResponseHeader(BufferThreadData *pThreadData, const char * headerName)
{
	char * lwrHeader;
	char * lwrHeaderName;
	char * valueStart;
	char * lineEnd;

	char * value;

	lwrHeader = _strlwr(strdup(pThreadData->httpResponseHeader));
	lwrHeaderName = _strlwr(strdup(headerName));

	valueStart = strstr(lwrHeader, lwrHeaderName);

	if (!valueStart)
		return NULL;

	valueStart = pThreadData->httpResponseHeader + (valueStart - lwrHeader); // Use the real header not the lower case one

	valueStart += strlen(headerName);

	if (*valueStart != ':')
		return NULL;

	valueStart++;

	while (*valueStart == ' ')
		valueStart++;

	lineEnd = strchr(valueStart, '\n');

	if (!lineEnd)
		return NULL;

	*lineEnd = 0;

	value = strdup(valueStart);

	*lineEnd = '\n';

	return value;
}

BOOL CALLBACK NullDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SETFOCUS:
            SetFocus(hwndPlayer);
            return 0;
        case WM_INITDIALOG:
            SetFocus(hwndPlayer);
            return 0;
        case WM_TIMER:
            switch (wParam)
            {
                case 1:
                    {
                        RECT playerRect;
                        RECT messageRect;

                        int top;

                        if (!hwndMessage || !hwndText || !hwndProgress)
                            break;

                        GetWindowRect(hwndPlayer, &playerRect);
                        GetWindowRect(hwndMessage, &messageRect);

                        top = playerRect.top - (messageRect.bottom - messageRect.top);

                        if (top < 0)
                            top = 0;

                        if ((messageRect.left != playerRect.left) || (messageRect.top != top))
                            MoveWindow(hwndMessage, playerRect.left, top, messageRect.right - messageRect.left, messageRect.bottom - messageRect.top, TRUE);
                    }

                    return 0;
                case 2:
                    {
                        hideMessage();

                        KillTimer(hwndMessage, wParam);
                    }
                    return 0;
            }
            break;
    }

    return 0;
}

void httpSetHwnd(HWND hWnd)
{
    hwndPlayer = hWnd;
}

void httpSetProxy(const char *string)
{
    strcpy(proxy, string);
}

void httpInit() 
{
	WORD wVersionRequested = MAKEWORD(1, 0);
	WSADATA wsaData;
    HRSRC dialogResource;

    iniFilename[0] = 0;

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		MessageBox(hwndPlayer, "Error Initializing Windows Sockets", "Vorbis Init Error", MB_OK);
    }

    lastUrlChecked[0] = 0;
    proxy[0] = 0;

    dialogResource = FindResource(GetModuleHandle("in_vorbis.dll"), MAKEINTRESOURCE(IDD_MESSAGE), RT_DIALOG);

    if (dialogResource)
    {
        HGLOBAL hGlobal = LoadResource(GetModuleHandle("in_vorbis.dll"), dialogResource);

        if (hGlobal)
        {
            hwndMessage = CreateDialogIndirect((HINSTANCE) GetWindowLong(hwndPlayer, GWL_HINSTANCE), LockResource(hGlobal), hwndPlayer, NullDialogProc);

            if (hwndMessage)
            {
                hwndText = GetDlgItem(hwndMessage, IDC_TEXT);
                hwndProgress = GetDlgItem(hwndMessage, IDC_PROGRESS);

                SetTimer(hwndMessage, 1, 20, NULL);
            }
        }
    }
}

void httpShutdown()
{ 
    WSACleanup();
}

int isOggUrl(const char *url) 
{
	if (strncmp(url, OGG_PROTOCOL, strlen(OGG_PROTOCOL)) == 0)
	{
        BufferThreadData * pThreadData = 0;

        if (strcmp(url, lastUrlChecked) == 0)
            return lastUrlOgg;

        strcpy(lastUrlChecked, url);

        if (stricmp(url + strlen(url) - 4, ".ogg") == 0)
            return lastUrlOgg = 1;

        pThreadData = createBufferThreadData();

		if (!httpConnect(url, pThreadData, 0, FALSE))
        {
            free(pThreadData);

			return lastUrlOgg = 0;
        }

		closesocket(pThreadData->tcpSocket);

		if (strcmp(getHttpResponseHeader(pThreadData, "Content-Type"), OGG_MIME_TYPE) == 0)
			return lastUrlOgg = 1;

        free(pThreadData);
	}

	return lastUrlOgg = 0; 
} 

size_t httpRead(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	int bytesRequested = size * nmemb;
	int bytesRead = 0;
	int bytesToCopy;

    BufferThreadData * pThreadData = (BufferThreadData *) datasource;

	while ((bytesRead < bytesRequested) && (!pThreadData->killBufferThread || (pThreadData->circularBufferStart != pThreadData->circularBufferEnd)))
	{
		if (pThreadData->circularBufferStart != pThreadData->circularBufferEnd)
		{
			if (pThreadData->circularBufferStart < pThreadData->circularBufferEnd)
				bytesToCopy = pThreadData->circularBufferEnd - pThreadData->circularBufferStart;
			else
				bytesToCopy = CIRCULAR_BUFFER_SIZE - pThreadData->circularBufferStart;

			if (bytesRead + bytesToCopy > bytesRequested)
				bytesToCopy = bytesRequested - bytesRead;

			memcpy((char *) ptr + bytesRead, pThreadData->circularBuffer + pThreadData->circularBufferStart, bytesToCopy);

			pThreadData->circularBufferStart = (pThreadData->circularBufferStart + bytesToCopy) % CIRCULAR_BUFFER_SIZE;

			bytesRead += bytesToCopy;
		}
	}

	return bytesRead / size;
}

int httpSeek(void *datasource, ogg_int64_t offset, int whence)
{
    return -1; // No HTTP seeking allowed yet. Maybe we can add support for the HTTP "Range" header later.
}

int httpClose(void *datasource)
{
    BufferThreadData * pThreadData = (BufferThreadData *) datasource;

	int retVal = closesocket(pThreadData->tcpSocket) ? 0 : EOF; // same as CloseHandle below

    free(pThreadData);

    return retVal;
}

long httpTell(void *datasource)
{
    BufferThreadData * pThreadData = (BufferThreadData *) datasource;

	return pThreadData->bytesRead;
}


void * httpStartBuffering(const char *url, OggVorbis_File *oggVorbisFile, BOOL showMessages) 
{ 
	vorbis_info *vi = NULL;
	ov_callbacks callbacks = {httpRead, httpSeek, httpClose, httpTell};

    BufferThreadData * pThreadData = createBufferThreadData();

	if (!httpConnect(url, pThreadData, 1, showMessages))
		return 0;

	pThreadData->bufferThreadHandle = (HANDLE)_beginthread(BufferThread, 0, (void *)(pThreadData));

	if (ov_open_callbacks(pThreadData, oggVorbisFile, NULL, 0, callbacks) < 0)
	{
		showError("ov_open_callbacks failed in httpstream. :-(");

		pThreadData->killBufferThread = 1;
		
		return 0;
	}
	
	return (void *) pThreadData; 
}

void httpStopBuffering(void * vp) 
{ 
    BufferThreadData * pThreadData = (BufferThreadData *) vp;

	if (pThreadData->bufferThreadHandle != INVALID_HANDLE_VALUE)
    {
		pThreadData->killBufferThread = 1;
		
		if (WaitForSingleObject(pThreadData->bufferThreadHandle, INFINITE) == WAIT_TIMEOUT)
        {
			showError("Error asking thread to die!");

			TerminateThread(pThreadData->bufferThreadHandle, 0);
		}

		pThreadData->bufferThreadHandle = INVALID_HANDLE_VALUE;
	}

	pThreadData->circularBufferStart = 0;
	pThreadData->circularBufferEnd = 0;

    hideMessage();
}

char *httpGetTitle(const char *url)
{
	if (strncmp(url, OGG_PROTOCOL, strlen(OGG_PROTOCOL)) == 0)
	{
		char * period;
		char * title = strrchr(url, '/');

		if (title)
        {
			title = strdup(title + 1);

		    period = strrchr(title, '.');

		    if (period && (period - title == (int) strlen(title) - 4) && (stricmp(period, ".ogg") == 0)) // remove the ".ogg" extension if there is one
			    *period = 0;

		    return title;
        }
	}

    return NULL;
}

void BufferThread(void *vp)
{
    BufferThreadData * pThreadData = (BufferThreadData *) vp;
	int bytesRead;

	int bytesToRead;

    DWORD lastUpdate = 0;

    showPercentage(0);

    ShowWindowAsync(hwndText, SW_HIDE);
    ShowWindowAsync(hwndProgress, SW_SHOW);

	while (!pThreadData->killBufferThread)
	{
		// read into the buffer if it isn't full
		if (((pThreadData->circularBufferEnd + 1) % CIRCULAR_BUFFER_SIZE) != pThreadData->circularBufferStart)
		{
			if (pThreadData->circularBufferStart > pThreadData->circularBufferEnd)
				bytesToRead = pThreadData->circularBufferStart - pThreadData->circularBufferEnd - 1;
			else
				bytesToRead = CIRCULAR_BUFFER_SIZE - pThreadData->circularBufferEnd;

			bytesRead = recv(pThreadData->tcpSocket, pThreadData->circularBuffer + pThreadData->circularBufferEnd, 1, 0);
			
			if ((bytesRead == SOCKET_ERROR) || (bytesRead == 0))
				break;

			pThreadData->bytesRead += bytesRead;

			pThreadData->circularBufferEnd = (pThreadData->circularBufferEnd + bytesRead) % CIRCULAR_BUFFER_SIZE;
		}
		else
			Sleep(10);

        if (GetTickCount() - lastUpdate > 100)
        {
            int start = pThreadData->circularBufferStart;
            int end = pThreadData->circularBufferEnd;

            if (end < start)
                end += CIRCULAR_BUFFER_SIZE;

            showPercentage((end - start) * 100 / CIRCULAR_BUFFER_SIZE);

            lastUpdate = GetTickCount();
        }
	}

	pThreadData->killBufferThread = 1;
	
	_endthread();
}

void setHttpVars()
{
    static HWND hwndWinamp = 0;
    static DWORD lastProxyCheck = 0;

    if (GetTickCount() - lastProxyCheck > 5000) // Don't check to see if proxy changed if we checked less than 5 seconds ago
    {
        if (*iniFilename == 0)
        {
            if (GetModuleFileName(GetModuleHandle(NULL), iniFilename, MAX_PATH))
            {
                char * lastSlash = strrchr(iniFilename, '\\');

                *(lastSlash + 1) = 0;

                strcat(iniFilename, "winamp.ini");
            }
        }

        if (*iniFilename != 0)
        {
            char proxy[256];

            GetPrivateProfileString("Winamp", "proxy", "", proxy, 256, iniFilename);

            httpSetProxy(proxy);
        }

        lastProxyCheck = GetTickCount();
    }

    if (!hwndWinamp)
	    httpSetHwnd(hwndWinamp = FindWindow("Winamp v1.x", NULL));
}

