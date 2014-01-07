/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 Albert L. Faber
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


#ifndef INI_H_INCLUDED
#define INI_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CIni  
{
private:

	CUString	m_strFileName;

public:
	BOOL	SetValue( const CUString& strSection, const CUString& strItem, const CUString& strVal );
	BOOL	SetValue( const CUString& strSection, const CUString& strItem, const int iVal );
	BOOL	SetValue( const CUString& strSection, const CUString& strItem, const long lVal );
	BOOL	SetValue( const CUString& strSection, const CUString& strItem, const DWORD lVal );


	CUString GetValue( const CUString& strSection, const CUString& strItem, CUString strDefault );
	INT		GetValue( const CUString& strSection, const CUString& strItem, const INT nDefault );
	LONG	GetValue( const CUString& strSection, const CUString& strItem, const LONG nDefault );
	DWORD	GetValue( const CUString& strSection, const CUString& strItem, const DWORD nDefault );

	CIni( );
	virtual ~CIni();
    void SetBom();
	void SetIniFileName( const CUString& strFileName ) { m_strFileName = strFileName; }
};

#endif
