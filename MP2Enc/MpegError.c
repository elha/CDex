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

#include "Common.h"
#include "MpegError.h"

#if defined(_WIN32) && !defined(__CYGWIN__) 

void MyLocalErrorFunc(char* lpszFile,int nLine,char* lpszError)
{
	char strTmp[255];
	sprintf(strTmp,"Fatal Error %s at file:%s at line %d\n",lpszError,lpszFile,nLine);

#ifdef _WIN32
	OutputDebugString(strTmp);
#else
	fprintf(stderr,strTmp);
#endif

}



//ERRORFUNC pErrorFunc=MyLocalErrorFunc;
static BOOL gs_bLogFile=FALSE;

void DebugPrintf(const char* pzFormat, ...)
{
    char	szBuffer[1024]={'\0',};
	char	szFileName[MAX_PATH+1]={'\0',};
    va_list ap;

	// Get the full module file name
//	GetModuleFileName(gs_hModule,szFileName,sizeof(szFileName));

	// change file name extention
//	szFileName[strlen(szFileName)-3]='t';
//	szFileName[strlen(szFileName)-2]='x';
//	szFileName[strlen(szFileName)-1]='t';

	// start at beginning of the list
	va_start(ap, pzFormat);

	// copy it to the string buffer
	_vsnprintf(szBuffer, sizeof(szBuffer), pzFormat, ap);

	// log it to the file?
	if (gs_bLogFile) 
	{	
        FILE* fp = NULL;
		
		// try to open the log file
		fp=fopen(szFileName, "a+");

		// check file open result
        if (fp)
		{
			// write string to the file
            fputs(szBuffer,fp);

			// close the file
            fclose(fp);
        }
    }

    OutputDebugString(szBuffer);

	va_end(ap);
}

void MP2LibError(const char* pzFormat, ...)
{
    char	szBuffer[1024]={'\0',};
	char	szFileName[MAX_PATH+1]={'\0',};
    va_list ap;

	// start at beginning of the list
	va_start(ap, pzFormat);

	// copy it to the string buffer
	_vsnprintf(szBuffer, sizeof(szBuffer), pzFormat, ap);

#ifdef _BLADEDLL
    OutputDebugString(szBuffer);
#else
	fprintf(stderr,szBuffer);
#endif

//	ASSERT(FALSE);

	va_end(ap);
}

#endif
