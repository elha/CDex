/*
** Copyright (C) 2000 Albert L. Faber
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <process.h>
#include <commctrl.h>

#include "httpstream.h"
#include "resource.h"

#define MP3_PROTOCOL "http://"
#define MP3_MIME_TYPE "application/x-mp3"
#define USER_AGENT "CDex in_mpg123 Winamp Plugin"

typedef struct
{
    SOCKET  tcpSocket;

    int     killBufferThread; // trigger to kill buffering thread
    HANDLE  bufferThreadHandle;	// the handle to the http buffering thread

    int     circularBufferStart;
    int     circularBufferEnd;
	int		circularBufferFilled;

    long    bytesRead;

    char    circularBuffer[ CIRCULAR_BUFFER_SIZE ];
    char    httpResponseHeader[ 16384 ]; // 16K HTTP header should be enough

} BufferThreadData;

char	lastUrlChecked[ 2048 ] = { '\0',};
int		lastUrl = 0;
char	proxy[ 256 ];

// Uncomment the following line to include the status window
//#define INCLUDE_STATUS_WINDOW

#ifdef INCLUDE_STATUS_WINDOW
	static HWND			hwndMessage = 0;
	static WINDOWPOS	messageWindowPos;
	static HWND			hwndText = 0;
	static HWND			hwndProgress = 0;
	static BOOL			isDialogTimer = 0;
#endif

static HWND    hwndPlayer = 0;

void	BufferThread(void *vp); // the buffer thread procedure

char    iniFilename[MAX_PATH] = { '\0',};



BufferThreadData * createBufferThreadData()
{
    BufferThreadData * pData = malloc( sizeof( BufferThreadData ) );

	// check malloc result
    if ( !pData )
	{
		// return failure
        return 0;
	}

    pData->tcpSocket = socket( AF_INET, SOCK_STREAM, 0 );
    pData->killBufferThread = 0;
    pData->bufferThreadHandle = INVALID_HANDLE_VALUE;
    pData->circularBufferStart = 0;
    pData->circularBufferEnd = 0;
	pData->circularBufferFilled = 0;
    pData->bytesRead = 0;

    *(pData->httpResponseHeader) = 0;

    return pData;
}

void showMessage( const char* message )
{
#ifdef INCLUDE_STATUS_WINDOW
    RECT playerRect;
    RECT messageRect;

    int top;

    if (!hwndMessage || !hwndText || !hwndProgress)
        return;

    ShowWindow( hwndProgress, SW_HIDE );
    ShowWindow( hwndText, SW_SHOW );

    SetWindowText(hwndText, message);

    GetWindowRect(hwndPlayer, &playerRect);
    GetWindowRect(hwndMessage, &messageRect);

    top = playerRect.top - (messageRect.bottom - messageRect.top);

    if (top < 0)
        top = 0;

	MoveWindow(hwndMessage, playerRect.left, top, messageRect.right - messageRect.left, messageRect.bottom - messageRect.top, TRUE);

	ShowWindow(hwndMessage, SW_SHOW);

    if (!isDialogTimer)
    {
        SetTimer(hwndMessage, 1, 20, NULL);
        isDialogTimer = 1;
    }

	UpdateWindow(hwndMessage);
#endif
}

#ifdef INCLUDE_STATUS_WINDOW
BOOL wasLastMessageError = 0;
#endif

void showError(const char * message)
{
#ifdef INCLUDE_STATUS_WINDOW
    if (!hwndMessage)
        return;

	showMessage(message);

	wasLastMessageError = 1;

	SetTimer(hwndMessage, 2, 5000, NULL);
#else
    MessageBox(hwndPlayer, message, "Winamp Vorbis Plugin", MB_OK);
#endif
}

#ifdef INCLUDE_STATUS_WINDOW
void showPercentage(int percentage)
{
    if (!hwndMessage)
        return;
    
    PostMessage(hwndProgress, PBM_SETPOS, (WPARAM) percentage, 0);

	RedrawWindow(hwndMessage, 0, 0, 0);
}
#endif

void hideMessage()
{
#ifdef INCLUDE_STATUS_WINDOW
	if (!hwndMessage)
        return;

    ShowWindow(hwndMessage, SW_HIDE);

    if (isDialogTimer)
    {
	    KillTimer(hwndMessage, 1);
        isDialogTimer = 0;
    }
#endif
}

int readHttpResponseHeader( BufferThreadData * pThreadData )
{
	int index;
	char * space;

	for (index = 0; index < 8191; index++)
	{
		if (recv(pThreadData->tcpSocket, pThreadData->httpResponseHeader + index, 1, 0) == SOCKET_ERROR)
			return -1;

		if (pThreadData->httpResponseHeader[index] == '\r')
		{
			index--; // discard any CRs
		}
        else 
		{
			if ((index > 0) && (pThreadData->httpResponseHeader[index] == '\n') && (pThreadData->httpResponseHeader[index-1] == '\n'))
				break; // "\n\n" ends the HTTP header
		}
	}

	pThreadData->httpResponseHeader[ index + 1 ] = 0;

	space = strchr( pThreadData->httpResponseHeader, ' ' );

	if ( space )
	{
		char status[4];

		for ( index = 0; index < 3; index++ )
			status[ index ] = *( space + 1 + index );

		status[ 3 ] = 0;

		return atoi( status );
	}

	return -1;
}

int httpConnect(	const char*			constUrl,
					BufferThreadData*	pThreadData,
					BOOL				getContent,
					BOOL				showMessages )
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
        	
	url = strdup(constUrl + strlen(MP3_PROTOCOL));

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

        _snprintf(buff, 2048, "%s http://%s:%d%s HTTP/1.0\nHost: %s\nUser-Agent: %s\nAccept: %s, */*\n\n", (getContent ? "GET" : "HEAD"), server, port, uri, server, USER_AGENT, MP3_MIME_TYPE);
    }
    else
        _snprintf(buff, 2048, "%s %s HTTP/1.0\nHost: %s\nUser-Agent: %s\nAccept: %s, */*\n\n", (getContent ? "GET" : "HEAD"), uri, server, USER_AGENT, MP3_MIME_TYPE);

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

#ifdef INCLUDE_STATUS_WINDOW
BOOL isDialogVisible = 0;		// gotta love crap like this ;-)
BOOL isPlayerForeground = 0;

BOOL CALLBACK NullDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_SETFOCUS:
            SetFocus(hwndPlayer);
            return 0;
        case WM_INITDIALOG:
            SetFocus(hwndPlayer);
            return 1;
        case WM_TIMER:
            switch (wParam)
            {
                case 1:
                    {
                        RECT playerRect;
                        RECT messageRect;

                        int top;

						if (!isDialogVisible && IsWindowVisible(hwndPlayer))
						{
							ShowWindow(hwndDlg, SW_SHOW);
							BringWindowToTop(hwndDlg);
							SetFocus(hwndPlayer);
							isDialogVisible = 1;
						}
						if (!IsWindowVisible(hwndPlayer))
						{
							if (isDialogVisible)
							{
								ShowWindow(hwndDlg, SW_HIDE);
								isDialogVisible = 0;
							}
							break;
						}

						if (GetForegroundWindow() == hwndPlayer)
						{
							if (!isPlayerForeground)
							{
								BringWindowToTop(hwndDlg);
								SetFocus(hwndPlayer);
								isPlayerForeground = 1;
							}
						}
						else
							isPlayerForeground = 0;

                        if (!hwndText || !hwndProgress)
                            break;

                        GetWindowRect(hwndPlayer, &playerRect);
                        GetWindowRect(hwndDlg, &messageRect);

                        top = playerRect.top - (messageRect.bottom - messageRect.top);

                        if (top < 0)
                            top = 0;

                        if ((messageRect.left != playerRect.left) || (messageRect.top != top))
                            MoveWindow(hwndDlg, playerRect.left, top, messageRect.right - messageRect.left, messageRect.bottom - messageRect.top, TRUE);
                    }

                    return 0;
                case 2:
                    {
                        hideMessage();
						
						wasLastMessageError = 0;

                        KillTimer(hwndDlg, wParam);
                    }
                    return 0;
            }
            break;
    }

    return 0;
}
#endif

void httpSetHwnd( HWND hWnd )
{
    hwndPlayer = hWnd;
}

void httpSetProxy( const char* string )
{
    strcpy( proxy, string );
}


void httpInit() 
{
	WORD wVersionRequested = MAKEWORD(1, 0);
	WSADATA wsaData;

#ifdef INCLUDE_STATUS_WINDOW
    HRSRC dialogResource;
#endif

	iniFilename[0] = 0;

	if ( WSAStartup( wVersionRequested, &wsaData) != 0 )
	{
		MessageBox(hwndPlayer, "Error Initializing Windows Sockets", "Vorbis Plugin Init Error", MB_OK);
	}

	lastUrlChecked[0] = 0;
	proxy[0] = 0;

#ifdef INCLUDE_STATUS_WINDOW
	dialogResource = FindResource(	GetModuleHandle( "in_mpg123.dll" ),
									MAKEINTRESOURCE( IDD_MESSAGE ),
									RT_DIALOG );

	if ( dialogResource )
	{
		HGLOBAL hGlobal = LoadResource(GetModuleHandle( "in_mpg123.dll" ), dialogResource );

		if ( hGlobal )
		{
			hwndMessage = CreateDialogIndirect((HINSTANCE) GetWindowLong(hwndPlayer, GWL_HINSTANCE), LockResource(hGlobal), hwndPlayer, NullDialogProc);

			if (hwndMessage)
			{
				hwndText = GetDlgItem(hwndMessage, IDC_TEXT);
				hwndProgress = GetDlgItem(hwndMessage, IDC_PROGRESS);

//				SetTimer(hwndMessage, 1, 20, NULL);
			}
		}
	}
#endif
}

void httpShutdown()
{
#ifdef INCLUDE_STATUS_WINDOW
    if (hwndMessage)
    {
        DestroyWindow(hwndMessage);
        hwndMessage = NULL;
    }
#endif

    WSACleanup();
}

int IsUrl( const char *filename ) 
{
    BufferThreadData*	pThreadData = NULL;
    char				url[ 4096 ] = { '\0', };

    strcpy( url, filename );

    // Check to see if the filename starts with "http://" or looks like a URL.
    if ( 0 != strncmp( url, MP3_PROTOCOL, strlen( MP3_PROTOCOL ) ) )
    {
        char* colon = NULL;
		char* slash = NULL;
		char* dot = NULL;
        
        colon = strchr( url, ':' );
        slash = strchr( url, '/' );
        dot = strchr( url, '.' );

        if ( dot && (( dot < slash ) || ( dot < colon ) ) )
		{
            _snprintf( url, 4096, "http://%s", url );
		}
    }

	if ( 0 == strncmp( url, MP3_PROTOCOL, strlen( MP3_PROTOCOL ) ) )
	{

        if ( 0 == strcmp( url, lastUrlChecked ) )
		{
            return lastUrl;
		}

        strcpy( lastUrlChecked, url );

        // Can we get away without connecting?
        if ( 0 == stricmp( url + strlen( url ) - 4, ".mp3" ) )
		{
            return lastUrl = 1;
		}

        pThreadData = createBufferThreadData();

		// try to make a connection
		if ( !httpConnect( url, pThreadData, 0, FALSE ) )
        {
			// failed to connect
            free( pThreadData );
			return lastUrl = 0;
        }

		// close the connection
		closesocket( pThreadData->tcpSocket );


//		if ( NULL != strstr( MP3_MIME_TYPE, getHttpResponseHeader( pThreadData, "Content-Type" ) ) )
		{
			// ok response
            lastUrl = 1;
		}
//		else
//		{
//			lastUrl = 0;
//		}

		// free thread data buffer
        free( pThreadData );

		return lastUrl;
	}

	return 0; 
} 


size_t httpRead(	void*	ptr,
					size_t	size,
					size_t	nmemb,
					void*	datasource )
{
	int bytesRequested = size * nmemb;
	int bytesRead = 0;
	int bytesToCopy;

    BufferThreadData * pThreadData = (BufferThreadData *) datasource;

	while ((bytesRead < bytesRequested) && (!pThreadData->killBufferThread || (pThreadData->circularBufferFilled > 0)))
	{
		if (pThreadData->circularBufferFilled > 0)
		{
			if (pThreadData->circularBufferStart < pThreadData->circularBufferEnd)
				bytesToCopy = pThreadData->circularBufferEnd - pThreadData->circularBufferStart;
			else
				bytesToCopy = CIRCULAR_BUFFER_SIZE - pThreadData->circularBufferStart;

			if (bytesRead + bytesToCopy > bytesRequested)
				bytesToCopy = bytesRequested - bytesRead;

			memcpy((char *) ptr + bytesRead, pThreadData->circularBuffer + pThreadData->circularBufferStart, bytesToCopy);

			pThreadData->circularBufferStart = (pThreadData->circularBufferStart + bytesToCopy) % CIRCULAR_BUFFER_SIZE;
			
			pThreadData->circularBufferFilled -= bytesToCopy;

#ifdef INCLUDE_STATUS_WINDOW
            showPercentage(pThreadData->circularBufferFilled * 100 / CIRCULAR_BUFFER_SIZE);
#endif

			bytesRead += bytesToCopy;
		}
	}

/* Who knows... maybe there's still some data in the buffer? *CM*

	// Don't give any data back if we're supposed to be dead
	if (pThreadData->killBufferThread)
		return 0;
*/
	return bytesRead / size;
}

char* httpStartOfStream( void*	datasource, INT nBytesRequested )
{

    BufferThreadData * pThreadData = (BufferThreadData *) datasource;

	while ( ( pThreadData->circularBufferFilled < nBytesRequested ) && 
			( !pThreadData->killBufferThread ) )
	{
		Sleep( 100 );

	}

	if ( pThreadData->killBufferThread )
	{
		return NULL;
	}
	else
	{
		return pThreadData->circularBuffer;
	}
}


int httpClose( void *datasource )
{
	int retVal;

    BufferThreadData * pThreadData = (BufferThreadData *) datasource;

	if ( !pThreadData->killBufferThread )
	{
		httpStopBuffering( pThreadData );
	}

#ifdef INCLUDE_STATUS_WINDOW
	if (!wasLastMessageError) hideMessage();
#endif

	retVal = closesocket(pThreadData->tcpSocket) ? 0 : EOF; // same as CloseHandle below

    free( pThreadData );

    return retVal;
}

long httpTell(void *datasource)
{
    BufferThreadData * pThreadData = (BufferThreadData *) datasource;

	return pThreadData->bytesRead;
}


void * httpStartBuffering(	const char*		url,
							BOOL			showMessages ) 
{ 
	BufferThreadData * pThreadData = createBufferThreadData();

	// open the connection
	if ( !httpConnect( url, pThreadData, 1, showMessages ) )
	{
		// free the allocated buffer data
		free( pThreadData );

		// failed to open the URL
		return NULL;
	}

	// create the receive data thread
	pThreadData->bufferThreadHandle = (HANDLE)_beginthread( BufferThread, 
															0,
															(void *)( pThreadData ) );

	// return the 
	return (void *) pThreadData; 
}

void httpStopBuffering( void * vp ) 
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
	pThreadData->circularBufferFilled = 0;

	hideMessage();
}

char *httpGetTitle(const char *url)
{
	if (strncmp(url, MP3_PROTOCOL, strlen(MP3_PROTOCOL)) == 0)
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

    return 0;
}

void BufferThread( void *vp )
{
    BufferThreadData*	pThreadData = (BufferThreadData *) vp;
	int					bytesRead	= 0;
	int					bytesToRead = 0;

#ifdef INCLUDE_STATUS_WINDOW
    showPercentage(0);

    ShowWindowAsync(hwndText, SW_HIDE);
    ShowWindowAsync(hwndProgress, SW_SHOW);
#endif

	while ( !pThreadData->killBufferThread )
	{
		// read into the buffer if it isn't full

		if ( pThreadData->circularBufferFilled < CIRCULAR_BUFFER_SIZE )
		{
			if ( pThreadData->circularBufferStart > pThreadData->circularBufferEnd )
			{
				bytesToRead = pThreadData->circularBufferStart - pThreadData->circularBufferEnd;
			}
			else
			{
				bytesToRead = CIRCULAR_BUFFER_SIZE - pThreadData->circularBufferEnd;
			}

			if ( bytesToRead > 4096 )
			{
				bytesToRead = 4096;
			}

			bytesRead = recv(	pThreadData->tcpSocket, 
								pThreadData->circularBuffer + pThreadData->circularBufferEnd, 
								bytesToRead,
								0 );
			
			if ( (  SOCKET_ERROR == bytesRead ) || ( bytesRead == 0 ) )
			{
				break;
			}

			pThreadData->bytesRead += bytesRead;
			pThreadData->circularBufferFilled += bytesRead;

			pThreadData->circularBufferEnd = (pThreadData->circularBufferEnd + bytesRead) % CIRCULAR_BUFFER_SIZE;
		}
		else
		{
			// wait for more data to come
            Sleep(50);
		}
	}

	pThreadData->killBufferThread = 1;
	
	_endthread();
}


void setHttpVars()
{
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

}

