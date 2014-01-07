/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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


#include "stdafx.h"
#include "cdex.h"
#include "ChildProcess.h"
#include <io.h>
#include <fcntl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChildProcess::CChildProcess(BOOL abConsole, 
							 const CUString& strCommandLine, 
							 DWORD aPriorityClass,
							 HANDLE hIn, 
							 HANDLE hOut, 
							 HANDLE hErr,
							 BOOL	bHideWindow )
{
	m_bConsole = abConsole ;
	m_strCommandLine = strCommandLine ;

	if ( aPriorityClass == IDLE_PRIORITY_CLASS ||
	     aPriorityClass == NORMAL_PRIORITY_CLASS ||
		 aPriorityClass == HIGH_PRIORITY_CLASS ||
		 aPriorityClass == REALTIME_PRIORITY_CLASS )
	{
		m_dwPriorityClass = aPriorityClass ;
	}
	else
	{
		m_dwPriorityClass = NORMAL_PRIORITY_CLASS ;
	}

	m_hStdin = hIn ;
	m_hStdout = hOut ;
	m_hStderr = hErr ;
	m_hProcess = NULL ;
	m_dwExitCode = STILL_ACTIVE ;
	m_bHideWindow = bHideWindow;
}

CChildProcess::~CChildProcess()
{
	// FIXME: Should we kill our child?
	// normally child exits by itself, as we stop sending data,
	// but this should not be taken for granted
	if ( m_hProcess )
	{
		CloseHandle ( m_hProcess ) ;
	}
}

BOOL CChildProcess::StartProcess()
{
	BOOL bConsoleCreated = FALSE ;
	BOOL rc = FALSE ;
	BOOL bReturn = TRUE;
	HWND hChildWindowHandle = NULL;
	PROCESS_INFORMATION pi ;
	STARTUPINFO sInfo ;

	if ( m_bConsole ) 
	{
		if ( !m_bHideWindow )
		{
			// YOU ALREADY HAVE A CONSOLE - DELETE THIS?
			bConsoleCreated = AllocConsole();

			if (! bConsoleCreated)
			{
				// FIXME: Error message
				bReturn = FALSE ;
			}
			else
			{
				// Get Window Handle of Child process console
				TCHAR oldConsoleTitle[ 1024 ] = { _T( '\0' ),};
				TCHAR newConsoleTitle[ 1024 ] = { _T( '\0' ),};

				GetConsoleTitle( oldConsoleTitle, sizeof ( oldConsoleTitle ) );

				_sntprintf( newConsoleTitle, sizeof ( newConsoleTitle ), _T( "CDex.ChildProcess.%u" ) , GetCurrentProcessId() );
				SetConsoleTitle( newConsoleTitle );

				hChildWindowHandle = FindWindow( NULL, newConsoleTitle );

				SetConsoleTitle( oldConsoleTitle );

				ShowWindow( hChildWindowHandle , SW_NORMAL );
			}	
		}

		if ( TRUE == bReturn )
		{
			if ( ! m_hStdin  ) m_hStdin  = GetStdHandle ( STD_INPUT_HANDLE  ) ;
			if ( ! m_hStdout ) m_hStdout = GetStdHandle ( STD_OUTPUT_HANDLE ) ;
			if ( ! m_hStderr ) m_hStderr = GetStdHandle ( STD_ERROR_HANDLE  ) ;
		}
	}

	if ( TRUE == bReturn )
	{
		memset( &sInfo, 0, sizeof ( sInfo ) ) ;
		sInfo.cb            = sizeof(STARTUPINFO);
		sInfo.dwFlags       = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		sInfo.wShowWindow   = m_bHideWindow ? SW_HIDE : SW_NORMAL ;
		sInfo.hStdInput     = m_hStdin ; 
		sInfo.hStdOutput    = m_hStdout ;
		sInfo.hStdError     = m_hStderr ; 

        CUStringConvert strCnv;

		rc = ::CreateProcess (	NULL, 
							    strCnv.ToT( m_strCommandLine ),
								NULL,
								NULL,
								TRUE,
								m_dwPriorityClass,
								NULL,
								NULL,
								&sInfo,
								&pi ) ;

		if ( !rc )
		{
			// FIXME: Error message
			bReturn = FALSE ;
		} 
		else 
		{
			m_hProcess = pi.hProcess ;
		}
	}

	if ( bConsoleCreated )
	{
		FreeConsole();
	}

	return bReturn;
}

// returns FALSE until Child process is running
BOOL CChildProcess::WaitForFinish(DWORD TimeOut)
{
	BOOL rc;
	rc = WaitForSingleObject( m_hProcess, TimeOut ) ;
	switch ( rc )
	{
	case WAIT_OBJECT_0:
		GetExitCodeProcess( m_hProcess, &m_dwExitCode ) ;
		return TRUE ;
	default:
		return FALSE ;
	}
}

// substitutes  string 'From' with string 'To' in Child process 
// command line

BOOL CChildProcess::SubstituteParameter( const CUString& strFrom, const CUString& strTo)
{
	int nPos;

	while ( ( nPos = m_strCommandLine.Find( strFrom ) ) >=0 )
	{
		m_strCommandLine =	m_strCommandLine.Left( nPos ) + 
							strTo + 
							m_strCommandLine.Right( m_strCommandLine.GetLength() - nPos - strFrom.GetLength() );
	}
	return TRUE ;
}
