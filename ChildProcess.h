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


#ifndef CHILEPROCESS_H_INCLUDED
#define CHILEPROCESS_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CChildProcess : public CObject
{
public:
	virtual BOOL WaitForFinish( DWORD Timeout );
	virtual BOOL StartProcess();
	CChildProcess(BOOL abConsole, const CUString& strCommandLine, DWORD aPriorityClass, HANDLE hIn, HANDLE hOut, HANDLE hErr, BOOL bHideWindow );
	virtual ~CChildProcess();

	BOOL SubstituteParameter( const CUString& strFrom, const CUString& strTo );

protected:
	DWORD	m_dwPriorityClass;
	DWORD	m_dwExitCode;
	BOOL	m_bConsole;
	HANDLE	m_hProcess;
	CUString m_strCommandLine;
	HANDLE	m_hStdin;
	HANDLE	m_hStdout;
	HANDLE	m_hStderr;
	BOOL	m_bHideWindow;
};

#endif // CHILEPROCESS_H_INCLUDED
