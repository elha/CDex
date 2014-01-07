/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2004 Albert L. Faber
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


#include <Windows.h>
#include "Logging.h"
#include <wchar.h>
#include <tchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

CHAR g_LOG_lpszFileName[ MAX_PATH + 1 ] = {'\0',};
TCHAR g_LOG_lastError[ MAX_PATH + 1 ] = {'\0',};


void LOG_SetLogFileName( const CHAR* lpszLogFileName )
{
	strncpy( g_LOG_lpszFileName, lpszLogFileName, sizeof( g_LOG_lpszFileName ) );
	g_LOG_lpszFileName[ sizeof( g_LOG_lpszFileName ) - 1 ] = '\0'; 
}


//CMutex g_DebugPrintfMutex( FALSE, _T( "CDEXLOG" ) );


void DebugPrintfW( const TCHAR* pzFormat, ...)
{
	BOOL	bLogFileLine = FALSE;


//	USES_CONVERSION;

//#ifdef _DEBUG
    TCHAR szBuffer[ 8192 ];

// AF TODO	g_DebugPrintfMutex.Lock();

    va_list ap;

	// Clear buffer
	memset( szBuffer, 0x00, sizeof( szBuffer ) );

    va_start(ap, pzFormat);
    _vsntprintf(szBuffer, sizeof(szBuffer), pzFormat, ap);

	if ( 0 == _tcsnccmp( szBuffer, LOG_FILE_LINE_STAMP, _tcslen( LOG_FILE_LINE_STAMP ) ) )
	{
		bLogFileLine = TRUE;
	}

    FILE* fp = NULL;
	time_t t = {0,};
	struct tm* psTM;

	time(&t);
	psTM = localtime(&t);

	if ( strlen( g_LOG_lpszFileName ) )
	{
		fp = fopen( g_LOG_lpszFileName, "a" );

		if ( NULL != fp )
		{
			if ( bLogFileLine )
			{
				_ftprintf( fp, _T( "%02d:%02d:%02d (%d): %s" ), psTM->tm_hour, psTM->tm_min, psTM->tm_sec,GetTickCount(), &szBuffer[ _tcslen( LOG_FILE_LINE_STAMP ) ] );
			}
			else
			{
				_ftprintf( fp, _T( "%s\n" ) , szBuffer );
			}

			fclose(fp);
		}
	}

#if defined _DEBUG || defined SPECIALDEBUG
	if ( bLogFileLine )
	{
	    OutputDebugString( &szBuffer[ _tcslen( LOG_FILE_LINE_STAMP ) ] );
	}
	else
	{
		_tcscat( szBuffer, _T( "\n\r" ) );
		OutputDebugString( szBuffer );
	}
#endif

	va_end(ap);

// AF TODO	g_DebugPrintfMutex.Unlock();

}

void DebugPrintfA( const CHAR* pzFormat, ...)
{
	BOOL	bLogFileLine = FALSE;


//	USES_CONVERSION;

    CHAR szBuffer[ 8192 ];

// AF TODO	g_DebugPrintfMutex.Lock();

    va_list ap;

	// Clear buffer
	memset( szBuffer, 0x00, sizeof( szBuffer ) );

    va_start(ap, pzFormat);
    _vsnprintf(szBuffer, sizeof(szBuffer), pzFormat, ap);

	if ( 0 == strncmp( szBuffer, LOG_FILE_LINE_STAMPA, strlen( LOG_FILE_LINE_STAMPA ) ) )
	{
		bLogFileLine = TRUE;
	}

    FILE* fp = NULL;
	time_t t = {0,};
	struct tm* psTM;

	time(&t);
	psTM = localtime(&t);

	if ( strlen( g_LOG_lpszFileName ) )
	{
		fp = fopen( g_LOG_lpszFileName, "a" );

		if ( NULL != fp )
		{
			if ( bLogFileLine )
			{
				_ftprintf( fp, _T( "%02d:%02d:%02d (%d): %s" ), psTM->tm_hour, psTM->tm_min, psTM->tm_sec,GetTickCount(), &szBuffer[ _tcslen( LOG_FILE_LINE_STAMP ) ] );
			}
			else
			{
				_ftprintf( fp, _T( "%s\n" ) , szBuffer );
			}

			fclose(fp);
		}
	}

#if defined _DEBUG || defined SPECIALDEBUG
	if ( bLogFileLine )
	{
	    OutputDebugStringA( &szBuffer[ strlen( LOG_FILE_LINE_STAMPA ) ] );
	}
	else
	{
		strcat( szBuffer, "\n\r");
		OutputDebugStringA( szBuffer );
	}
#endif

	va_end(ap);

// AF TODO	g_DebugPrintfMutex.Unlock();

}


TCHAR* GetLastErrorString( )
{
	FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
		GetLastError(),
		0,
		g_LOG_lastError,
        sizeof( g_LOG_lastError ),
        NULL );

	return g_LOG_lastError;
}

