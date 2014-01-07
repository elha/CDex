/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 - 2007 Albert L. Faber
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


#include <StdAfx.h>
#include <limits.h>
#include "TaskInfo.h"
#include "Config.h"
#include "FileName.h"

const double CTaskInfo::NORM_TUNE_FACTOR   = 0.99999;
const double CTaskInfo::NORM_DEFAULT_VALUE = 1.00000;


CTaskInfo::CTaskInfo()
{
	m_strName			= _T( "" );
	m_strNonSplitArtist = _T( "" );

	m_dwStartSector		= 0;
	m_dwEndSector		= 0;

	m_strOutDir			= _T( "" );
	m_strOutName		= _T( "" );

	m_nEncoderType		= ENCODER_FIXED_WAV;

	m_dwLengthInMs		= 0;

	m_bRip				= FALSE;
	m_bNormalize		= FALSE;
	m_bConvert			= FALSE;
	m_bDeleteWav		= FALSE;
	m_bRiffWav			= FALSE;
	m_nPeakValue		= INT_MAX;
	m_dNormalizationFactor = NORM_DEFAULT_VALUE ;
	m_bValidTagData		= false;

	memset( m_btaRawToc, 0 , sizeof( m_btaRawToc ) );
}

CTaskInfo::~CTaskInfo()
{
}


CUString CTaskInfo::GetOutDir() const 
{
	return m_strOutDir;
}


CUString CTaskInfo::GetNonSplitArtist() const 
{
	return m_strNonSplitArtist;
}

DWORD CTaskInfo::GetStartSector() const 
{
	return m_dwStartSector;
}

DWORD CTaskInfo::GetEndSector() const 
{
	return m_dwEndSector;
}


DWORD CTaskInfo::GetLengthInMs() const 
{
	return m_dwLengthInMs;
}

CUString CTaskInfo::GetInDir() const 
{
	// get the full path name string
	CUString strTmp=m_strName;

	// find the last backslash
	int nPos=strTmp.ReverseFind( _T('\\' ));

	// return everything before the last backslash
	return strTmp.Left(nPos+1);
}

void CTaskInfo::SetInDir( CUString strDir ) 
{
	// get the file name, path excluded 
	CUString strTmp = GetFileName( );

	if ( strDir[ strDir.GetLength() - 1 ] != _W( '\\' ) )
	{
		strDir += _W( "\\" );
	}

	SetFullFileName( strDir + strTmp );

}


CUString CTaskInfo::GetFullFileName( ) const 
{
	return m_strName;
}


CUString CTaskInfo::GetFullFileNameNoExt( ) const 
{
	CUString strTmp=m_strName;

	int nPos=strTmp.ReverseFind( _T( '.' ) );

	// there is a DOT somewhere
	if (nPos)
	{
		// cut everything after the last ., . included
		strTmp = strTmp.Left(nPos);
	}

	// return resulting string
	return strTmp;
}

CUString CTaskInfo::GetFileName( ) const 
{
	// get the full path name string
	CUString strTmp=m_strName;

	// find the last backslash
	int nPos=strTmp.ReverseFind( _T('\\') );

	// return everything after the last backslash
	return strTmp.Right(strTmp.GetLength()-nPos-1);
}

CUString CTaskInfo::GetFileNameNoExt( ) const 
{
	// get the full path name string
	CUString strTmp=m_strName;

	// find the last backslash
	int nPosS = strTmp.ReverseFind( _T( '\\' ));

    // find the last .
	int nPosE = strTmp.ReverseFind( _T( '.' ));

    if ( nPosE < 0 )
    {
		nPosE=strTmp.GetLength();
    }

    // return everything before the last backslash
	return strTmp.Mid( nPosS + 1, nPosE - nPosS - 1 );
}


void CTaskInfo::BuildFileName( CUString& strName,CUString& strDir )
{
	CUString strBuild[ NUMFILENAME_IDS ];
	
	// Build playlist filename
	strBuild[ 0] = m_tagData.GetArtist( );									// %1 = Artist
	strBuild[ 1] = m_tagData.GetAlbum( );									// %2 = Album
    strBuild[ 2].Format( _W( "%d" ), (LONG)m_tagData.GetTrackNumber( ) + m_tagData.GetTrackOffset() );				// %3 = Track Number
    strBuild[ 3] = m_tagData.GetTitle( );								// %4 = Track Name
	strBuild[ 4].Format( _W( "%08x" ), m_tagData.GetVOLID( ) );				// %5 = CD VolumeID
	strBuild[ 5].Format( _W( "%08x" ), m_tagData.GetCDBID( ) );				// %6 = CDDB ID 
	strBuild[ 6].Format( _W( "%02d" ), (LONG)m_tagData.GetTrackNumber() + m_tagData.GetTrackOffset() ); //%7 = Track Number Leading Zero
	strBuild[ 7].Format( _W( "%02d" ), m_tagData.GetTotalTracks() + m_tagData.GetTrackOffset() ); // %8 = Total Number of Tracks
	strBuild[ 8] = m_tagData.GetYear( );									// %Y = Year
	strBuild[ 9] = m_tagData.GetGenre( );									// %G = Genre
	strBuild[10] = GetNonSplitArtist( );							// %A = Artist Name (not affected by the split option)

	// Build the new file name
	::BuildFileName( g_config.GetFileFormat(), strBuild, strName, strDir );
}


CUString CTaskInfo::GetFileExt( ) const 
{
	// get the full path name string
	CUString strTmp=m_strName;

	// find the last dot
	int nPos=strTmp.ReverseFind( _T( '.' ));

	if (nPos)
		return strTmp.Right(strTmp.GetLength()-nPos-1);

	return _W( "" );
}

void CTaskInfo::SetFileExt( CUString str)
{
	// get the full path name string
	CUString strTmp( m_strName );

	// find the last dot
	int nPos = strTmp.ReverseFind( _T( '.' ) );

//	int nTst=strTmp.GetLength();
//	if ( (strTmp.GetLength()-nPos)<=4 && nPos>=0)
	if ( nPos >= 0 )
	{
		strTmp=strTmp.Left( nPos );
	}

	// Set the file name + extention
	m_strName = strTmp + _W( "." ) + str;
}


CUString CTaskInfo::GetOutFullFileName( ) const 
{
	return m_strOutName;
}

void CTaskInfo::SetOutFullFileName( const CUString& str) 
{ 
    m_strOutName = str;
}


CUString CTaskInfo::GetOutFullFileNameNoExt( ) const 
{
	CUString strTmp=m_strOutName;

	int nPos=strTmp.ReverseFind( _T( '.' ) );

	// there is a DOT somewhere
	if (nPos)
	{
		// return everything except the file extention
		return strTmp.Left(nPos);
	}

	// no extention found, return entire string
	return strTmp;
}


CUString CTaskInfo::GetOutFileName() const 
{
	// get the full path name string
	CUString strTmp=m_strOutName;

	// find the last backslash
	int nPos=strTmp.ReverseFind( _T( '\\' ));

	// return everything after the last backslash
	return strTmp.Right(strTmp.GetLength()-nPos-1);
}

CUString CTaskInfo::GetOutFileNameNoExt( ) const 
{
	// get the full path name string
	CUString strTmp=m_strOutName;

	// find the last backslash
	int nPosS=strTmp.ReverseFind( _T( '\\' ));
	int nPosE=strTmp.ReverseFind( _T( '.' ));

	if (nPosE<0) 
    {
        nPosE=strTmp.GetLength();
    }
	// return everything before the last backslash
	strTmp = strTmp.Mid(nPosS+1,nPosE-nPosS-1);

	return strTmp;
}

CUString CTaskInfo::GetOutFileExt( ) const 
{
	// get the full path name string
	CUString strTmp( m_strOutName );

	// find the last dot
	int nPos=strTmp.ReverseFind( _T( '.' ));

	if (nPos)
		return strTmp.Right( strTmp.GetLength()-nPos-1 );

	return _W( "" );
}


void CTaskInfo::SetOutFileExt( CUString str )
{
	// get the full path name string
	CUString strTmp( m_strName );

	// find the last dot
	int nPos=strTmp.ReverseFind( _T( '.' ) );

	int nTst=strTmp.GetLength();

//	if ( (strTmp.GetLength()-nPos)<=4 && nPos>=0)
	if ( nPos >= 0 )
	{
		strTmp = strTmp.Left( nPos );
	}

	// Set the file name + extention
	m_strOutName = strTmp + _W( "." ) + str;
}

void CTaskInfo::SetEncoderType( ENCODER_TYPES nType )
{
	m_nEncoderType = nType;
}

int CTaskInfo::GetPeakValue( ) const 
{
	return m_nPeakValue;
}
