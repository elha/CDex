/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
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
#include "TextFile.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CHAR_EOF 26
#define CHAR_LF  10
#define CHAR_CR  13

const CUString CTextFile::m_strCRLF( CUString( (CHAR)CHAR_CR ) + CUString( (CHAR)CHAR_LF ) );
const CUString CTextFile::m_strLF( CUString( (CHAR)CHAR_LF ) );

IMPLEMENT_DYNAMIC( CTextFile, CObject)


// Always open the file as a binary file, the class will be
// transparent regarding unix/dos test files

CTextFile::CTextFile( )
		:CObject( ),
		m_pFile( NULL ),
		m_bWriteAsDosFile( false )

{
}



void CTextFile::SetWriteAsDosFile( bool bValue  )
{
	m_bWriteAsDosFile = bValue;
}

BOOL  CTextFile::Open( LPCTSTR lpszFileName, TEXTFILE_MODE nOpenFlags )
{
	BOOL bReturn = FALSE;

	CUString strFlags;

	switch ( nOpenFlags ) 
	{
		case modeRead  : strFlags = _W( "rb" ); break;
		case modeWrite : strFlags = _W( "wb" ); break;
		case modeAppend: strFlags = _W( "ab" ); break;
		default:
			ASSERT( FALSE );
	}

	m_pFile = CDexOpenFile( lpszFileName, strFlags );

	if ( NULL != m_pFile )
	{
		bReturn = TRUE;
	}

	return bReturn;
}


CTextFile::~CTextFile()
{
}

BOOL  CTextFile::ReadString( CUString& rString )
{
	BOOL bResult = FALSE;
	int nPos = 0;
	int nInputChar;

	rString = _T("");

	vector <CHAR> vChars;


	ASSERT( m_pFile );

	while ( EOF != ( nInputChar = fgetc( m_pFile ) ) )
	{
		bResult = TRUE;

		if ( CHAR_LF == (CHAR)nInputChar ||
			 CHAR_EOF == (CHAR)nInputChar )
		{
			break;
		}
//		if ( CHAR_CR != (char)nInputChar )
		{
			vChars.push_back( (CHAR)nInputChar );
		}
	}

	// terminate string
	vChars.push_back( _T( '\0' ) );

	rString = CUString( &vChars[0], CP_UTF8 );

	return bResult;
}

void  CTextFile::WriteString( const CUString& wString )
{
	ASSERT( m_pFile );

	CUString strOut( wString );

	if ( m_bWriteAsDosFile )
	{
		strOut += m_strCRLF;
	}
	else
	{
		strOut += m_strLF;
	}
#ifdef _UNICODE
	CHAR* pszUtf = new CHAR[ strOut.GetLength() * 4 ];

		// convert ASCII string to WideCharacter string based on active code page
	int nLen = WideCharToMultiByte(	CP_UTF8 , 
									0,
									strOut,
									-1,
									pszUtf,
									strOut.GetLength() * 4,
									NULL,
									NULL );

	fwrite( pszUtf, 1, nLen - 1, m_pFile );

	delete [] pszUtf;
#else
	fwrite( strOut, 1, strOut.GetLength(), m_pFile );
#endif
}

void CTextFile::Close()
{
	ASSERT( m_pFile );

	if ( m_pFile )
	{
		fclose( m_pFile );
		m_pFile = NULL;
	}
}


int CTextFile::Seek( long offset, TEXTFILE_SEEK seekMode )
{
	int origin = SEEK_SET;

	ASSERT( m_pFile );

	switch( seekMode )
	{
		case seekBegin:   origin = SEEK_SET; break;
		case seekEnd:     origin = SEEK_END; break;
		case seekCurrent: origin = SEEK_CUR; break;
		default:
			ASSERT( FALSE );
	}

	return fseek( m_pFile, offset, origin );
}

int CTextFile::GetPosition()
{
	long offset = -1;

	ASSERT( m_pFile );

	return ftell( m_pFile );
}
