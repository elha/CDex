/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 - 2007 Albert L. Faber
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
#include "Ini.h"
#include "Util.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIni::CIni(  )
{
}


CIni::~CIni()
{
}



// **********************************************************************************

BOOL CIni::SetValue(const CUString& strSection, const CUString& strItem, const CUString& strVal)
{
    BOOL returnValue = FALSE;
	ASSERT( !m_strFileName.IsEmpty() );


    CUStringConvert strCnv;
    LPCSTR utfString = strCnv.ToUTF8( strVal );
    CUString utfEncoded( utfString, -1 );

#ifdef _UNICODE
    returnValue =  WritePrivateProfileString( strSection, strItem, utfEncoded, m_strFileName  );
#else
    CUStringConvert strCnv1;
    CUStringConvert strCnv2;
    CUStringConvert strCnv3;
	returnValue =  WritePrivateProfileString( strCnv.ToT( strSection ), strCnv1.ToT( strItem ), strCnv2.ToT( strVal ), strCnv3.ToT( m_strFileName ) );
#endif
    return returnValue;
}

BOOL CIni::SetValue(const CUString& strSection, const CUString& strItem, const int iVal)
{
	CUString strEntry;
	strEntry.Format( _W( "%d" ), iVal );
	return SetValue( strSection, strItem, strEntry );
}

BOOL CIni::SetValue(const CUString& strSection, const CUString& strItem, const long lVal)
{
	return SetValue( strSection, strItem, (int) lVal );
}

BOOL CIni::SetValue(const CUString& strSection, const CUString& strItem, const DWORD dwVal)
{
	return SetValue( strSection, strItem, (int)dwVal );
}



CUString CIni::GetValue( const CUString& strSection, const CUString& strItem, CUString strDefault )
{
	ASSERT( !m_strFileName.IsEmpty() );

	TCHAR lpszValue[ 1024 ] = {'\0',};

    CUStringConvert strCnv;
    CUStringConvert strCnv1;
    CUStringConvert strCnv2;
    CUStringConvert strCnv3;

	::GetPrivateProfileString(	strCnv1.ToT( strSection ),
								strCnv2.ToT( strItem ),
								strCnv3.ToT( strDefault ),
								lpszValue,
								_countof( lpszValue),
								strCnv.ToT( m_strFileName ) );

	return CUString( lpszValue, CP_UTF8 );
}

INT CIni::GetValue( const CUString& strSection, const CUString& strItem, const INT nDefault )
{
	INT nReturn = nDefault;

	CUString strReturn = GetValue( strSection, strItem, _W( "" ) );

	if ( !strReturn.IsEmpty() )
	{
		nReturn = _wtoi( strReturn );
	}

	return nReturn;
}


LONG CIni::GetValue( const CUString& strSection, const CUString& strItem, const LONG nDefault )
{
	return (LONG) GetValue ( strSection, strItem, (INT) nDefault );
}

DWORD CIni::GetValue( const CUString& strSection, const CUString& strItem, const DWORD nDefault )
{
	return (DWORD) GetValue ( strSection, strItem, (INT) nDefault );
}



void CIni::SetBom()
{ 
    // for UNICODE, write BOM to file if it does not exist, otherwhise
    // we can't write unicode strings into the INI file
#ifdef _UNICODE
    // UTF16-LE BOM(FFFE)
    WORD wBOM = 0xFEFF;
    LONG dataSize = 0;
    BYTE* pData = NULL;
    FILE* pFile = CDexOpenFile( m_strFileName, _W("r") );

    if ( pFile != NULL )
    {
        fseek( pFile, 0, SEEK_END );
        dataSize = ftell( pFile );
        pData = new BYTE[ dataSize ];
        fseek( pFile, 0, SEEK_SET );
        fread( pData, dataSize, 1, pFile );
        fclose( pFile );
    }

    if ( !( ( pData != NULL ) && 
            ( dataSize > 2 ) && 
            ((short*)pData)[0] == wBOM ) )
    {
        pFile = CDexOpenFile( m_strFileName, _W( "wb" ) );

        if ( pFile != NULL )
        {
            fwrite( &wBOM, sizeof( wBOM ), 1, pFile );
            if ( pData )
            {
                fwrite( pData, dataSize, 1, pFile );
            }

            fclose( pFile );
        }
    }

    delete [] pData;

#endif
}