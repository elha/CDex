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


#ifndef CDPLAYERINI_H_INCLUDED
#define CDPLAYERINI_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


typedef vector<CUString>				ASTRING;
typedef vector<CUString>::iterator	ASTRING_IT;
typedef vector<ASTRING>				AASTRING;
typedef vector<ASTRING>::iterator	AASTRING_IT;

class CDPlayerIni  
{
private:
	CMutex		m_ThreadLock;
	ASTRING		m_vSection;
	AASTRING	m_vvEntries;

	int		FindItem( int nSec, const CUString& strItem, CUString &strVal );
	bool	RemoveSection( const CUString& strSection );
	bool	IsSection( const ASTRING_IT itSec );
	int		FindSection( const CUString& strSection );
	bool	IsSectionString( const CUString&  strTest );
	int		InsertSection( const CUString& strSection );

public:
	bool	SetValue( const CUString& strSection, const CUString& strItem, const CUString& cVal );
	bool	SetValue( const CUString& strSection, const CUString& strItem, const int iVal );
	bool	SetValue( const CUString& strSection, const CUString& strItem, const long lVal );
	bool	SetValue( const CUString& strSection, const CUString& strItem, const DWORD lVal );


	CUString GetValue( const CUString& strSection, const CUString& strItem, CUString strDefault );
	INT		GetValue( const CUString& strSection, const CUString& strItem, const INT nDefault );
	LONG	GetValue( const CUString& strSection, const CUString& strItem, const LONG nDefault );
	DWORD	GetValue( const CUString& strSection, const CUString& strItem, const DWORD nDefault );

	void	Clear();

	bool	Write( const CUString& strFileName );
	bool	Read( const CUString& strFileName );

	CDPlayerIni( );
	virtual ~CDPlayerIni();

};

#endif
