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


#include "StdAfx.h"
#include "CDPLayerIni.h"
#include "util.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#include <stdlib.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDPlayerIni::CDPlayerIni(  )
{

	// Avoid needless copy constructors for speed reasons
	m_vSection.reserve( 50 );
}


CDPlayerIni::~CDPlayerIni()
{
	Clear();
}


bool CDPlayerIni::IsSectionString( const CUString&  strTest )
{
	CUString strItem( strTest );

	if ( strItem.GetLength() < 2 )
		return false;

	// quick check, to speed it up a bit
	if ( _T( '[' ) != strItem[ 0 ] )
	{
		return false;
	}
	else
	{
		strItem.TrimRight();

		if (	strItem.GetLength() > 2 && strItem.Left(1) == _T( '[' ) && 
				strItem.Right(1) == _T( ']' ) ) 
		{
			return true;
		}
	}
	return false;
}


bool CDPlayerIni::Read( const CUString& strFileName )
{
	FILE*	pFile = NULL;
	DWORD	dwBytes = 0;
	TCHAR	buf[ 1024 ];
	int		nSection = -1;
	int		nEntry = 0;

	Clear();

	CMutex	processLock( TRUE, _T( "CDEX_CPI" ) );

    pFile = CDexOpenFile( strFileName, _W( "r" ) );

	if ( NULL != pFile  )
	{

		while ( NULL !=  _fgetts( buf, sizeof( buf ) -1 , pFile ) )
		{
			// kill new line character
			if ( buf[ _tcslen( buf ) -1 ] == _T( '\n' ) )
			{
				buf[ _tcslen( buf ) -1 ] = _T( '\0' );
			}

			CUString strBuf( buf, CP_UTF8 );

			if ( IsSectionString( strBuf ) ) 
			{
				ASTRING vNewEntry;
				vNewEntry.reserve( 16 );

				nSection++;
				m_vSection.push_back( strBuf );
				m_vvEntries.push_back( vNewEntry );
			}
			else
			{
				if ( nSection >= 0 && strBuf.GetLength() > 0 )
				{
					m_vvEntries[ nSection ].push_back( strBuf );
				}
			}
		}

		fclose( pFile );
	}

	processLock.Unlock();

	return true;
}

bool CDPlayerIni::Write( const CUString& strFileName )
{
	FILE*	pFile = NULL;
	unsigned int		nItem = 0;
	int		nStringItems = m_vSection.size();
	int		nSection = 0;

	CMutex	processLock( TRUE, _T( "CDEX_CPI" ) );

    pFile = CDexOpenFile( strFileName, _W( "w" ) );

	if ( NULL != pFile  )
	{
		for ( nSection = 0; nSection < (int)m_vSection.size(); nSection++)
		{

			if ( nSection > 0 )
			{
				fputs( "\n" , pFile );
			}

			CUString strLine( m_vSection[ nSection ] + _W( "\n" ) );
           
            CUStringConvert strCnv;
            fputs( strCnv.ToUTF8( strLine ), pFile );

			for ( nItem = 0; nItem < m_vvEntries[ nSection ].size(); nItem++)
			{
				CUString strLine( m_vvEntries[ nSection ][ nItem ] + _W( "\n" ) );
                CUStringConvert strCnv; 

				_fputts( strCnv.ToT( strLine ), pFile );
			}
		}

		fclose( pFile );
	}

	processLock.Unlock();

	return true;
}

// **********************************************************************************

void CDPlayerIni::Clear()
{
	m_vSection.clear();
	m_vvEntries.clear();
}

// **********************************************************************************



int CDPlayerIni::FindSection( const CUString& strSection )
{
	CUString strSectionTag;
	unsigned int nIdx = 0;

	strSectionTag.Format( _W( "[%s]" ) , (LPCWSTR)strSection );

	strSectionTag.TrimLeft();
	strSectionTag.TrimRight();

	for ( nIdx = 0 ; nIdx < m_vSection.size(); nIdx++ )
	{
		CUString strLine( m_vSection[ nIdx ] );

		strLine.TrimLeft();
		strLine.TrimRight();
		

		if ( 0 == strSectionTag.CompareNoCase( strLine ) )
		{
			return nIdx;
		}
	}

	return -1;
}

int CDPlayerIni::InsertSection( const CUString& strSection )
{
	ASSERT( -1 == FindSection( strSection ) );

	CUString strSectionTag;

	strSectionTag.Format( _W( "[%s]" ), (LPCWSTR)strSection );

	ASTRING vNewEntry;

	m_vSection.push_back( strSectionTag );
	m_vvEntries.push_back( vNewEntry );

	return m_vvEntries.size() - 1;
}


int CDPlayerIni::FindItem( int nSec, const CUString& strItem, CUString &strVal)
{
	ASSERT( nSec >=0 );
	ASSERT( nSec < (int)m_vvEntries.size() );
	ASSERT( strItem );

	unsigned int nItem = 0;

	CUString strItemTag(strItem);
	CUString strLook;

	strItemTag += _W( "=" );

	int iLen = strItemTag.GetLength();

	for ( nItem = 0; nItem < m_vvEntries[ nSec ].size(); nItem++ )
	{
		// get the current item
		strLook = m_vvEntries[ nSec ][ nItem ];

		// compare item
		if ( 0 == strItemTag.CompareNoCase( strLook.Left( iLen ) ) ) 
		{
			// return proper value
			if ( strLook.GetLength() == iLen )
			{
				strVal = _W( "" );
				return nItem;
			}
			else
			{
				strVal = strLook.Right( strLook.GetLength() - iLen );
				return nItem;
			}
		}
	}

	return -1;
}




bool CDPlayerIni::RemoveSection(const CUString& strSection)
{
/*
	ASTRING_IT itStart = FindSection( strSection );
	ASTRING_IT itEnd = itStart;

	if ( NULL != itStart )
	{
		while ( itEnd < m_vSection.end() &&
				false == IsSection( itEnd) )
		{
			itEnd++;
		}
		m_vSection.erase( itStart, itEnd );
	}
*/
	return true;
}


// **********************************************************************************

bool CDPlayerIni::SetValue(const CUString& strSection, const CUString& strItem, const CUString& cVal)
{
	m_ThreadLock.Lock();

	int nSection = -1;
	int nItem = -1;

	CUString strVal;
	CUString strValue;
	strVal.Format( _W( "%s=%s" ), (LPCWSTR)strItem, (LPCWSTR)cVal );

	if ( -1 == ( nSection = FindSection( strSection ) ) )
	{
		nSection = InsertSection( strSection );
	}

	if ( -1 == ( nItem = FindItem( nSection, strItem, strValue) ) )
	{
		m_vvEntries[ nSection ].push_back( strVal );
	} else
	{
		m_vvEntries[ nSection ][ nItem ] = strVal;
	}

	m_ThreadLock.Unlock();
	return true;
}

bool CDPlayerIni::SetValue(const CUString& strSection, const CUString& strItem, const int iVal)
{
	CUString strEntry;
	strEntry.Format( _W( "%d" ), iVal );
	return SetValue( strSection, strItem, strEntry );
}

bool CDPlayerIni::SetValue(const CUString& strSection, const CUString& strItem, const long lVal)
{
	return SetValue( strSection, strItem, (int) lVal );
}

bool CDPlayerIni::SetValue(const CUString& strSection, const CUString& strItem, const DWORD dwVal)
{
	return SetValue( strSection, strItem, (int)dwVal );
}



// **********************************************************************************


CUString CDPlayerIni::GetValue( const CUString& strSection, const CUString& strItem, CUString strDefault )
{
	m_ThreadLock.Lock();

	CUString		strVal = strDefault;
	int			nSection = -1;
	int			nItem = -1;

	if ( -1 != ( nSection = FindSection( strSection ) ) )
	{
		if ( -1 != ( nItem = FindItem( nSection, strItem, strVal) ) )
		{
			m_ThreadLock.Unlock();
			return strVal;
		}
	}


	m_ThreadLock.Unlock();

	return strDefault;
}

INT CDPlayerIni::GetValue( const CUString& strSection, const CUString& strItem, const INT nDefault )
{
	m_ThreadLock.Lock();

	int		nSection = -1;
	int		nItem = -1;
	CUString strVal;

	if ( -1 != ( nSection = FindSection( strSection ) ) )
	{
		if ( -1 != ( nItem = FindItem( nSection, strItem, strVal) ) )
		{
			m_ThreadLock.Unlock();
			return _wtoi( strVal );
		}
	}

	m_ThreadLock.Unlock();

	return nDefault;
}


LONG CDPlayerIni::GetValue( const CUString& strSection, const CUString& strItem, const LONG nDefault )
{
	return (LONG) GetValue ( strSection, strItem, (INT) nDefault );
}

DWORD CDPlayerIni::GetValue( const CUString& strSection, const CUString& strItem, const DWORD nDefault )
{
	return (DWORD) GetValue ( strSection, strItem, (INT) nDefault );
}



