/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
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

#include <math.h>

#include "CDInfo.h"
#include "CDdb.h"
#include "MCI_CD.h"
#include "ID3Tag.h"
#include "CDPlayerIni.h"
#include "GetRemoteCDInfoDlg.h"

INITTRACE( _T( "CDInfo" ) );


static const int LEAD_OUT_NBR = 0xAA;

PlayList::PlayList( const CUString& strPlayListFileName )
{
	m_strFileName = strPlayListFileName;
}

void PlayList::AddEntry( CUString strEntry )
{
	if (g_config.GetM3UPlayList())
		AddM3UEntry(strEntry);
	if (g_config.GetPLSPlayList())
		AddPLSEntry(strEntry);

}


CUString PlayList::MakeEntry(	const CUString& strListName, 
								const CUString& strEntry )
{
	CUString strLDir;
	CUString strEDir;
	CUString strRet;

	// nPos
	int nPos=0;


	// search for last backslash
	nPos= strListName.ReverseFind( _T( '\\' ) );

	// get everything before the backslash (= dir)
	strLDir = strListName.Left( nPos );

	// search for last backslash
	nPos = strEntry.ReverseFind( _T( '\\' ) );

	// get everything before the backslash (= dir)
	strEDir = strEntry.Left( nPos );

	nPos=0;

	strLDir.MakeLower();

	strEDir.MakeLower();

	// check if playlist dir is the same as the entry directory
	if ( -1 != strEDir.Find( strLDir ) )
	{
		// use relative path
		strRet = strEntry.Mid( strLDir.GetLength() + 1 );
	}
	else
	{
		// use full file name
		strRet = strEntry;
	}

	return strRet;
}

void PlayList::AddPLSEntry( CUString strEntry )
{
	CUString strNumber;
	CUString	strPName( m_strFileName + _W( ".pls" ) );
	TCHAR	pszEntry[ MAX_PATH + 1 ] = { _T( '\0' ),};
	BOOL	bFound = FALSE;
	int		i = 0;


    CUStringConvert strCnv;

	// get the number of Playlist entires
	int nNumEntries = GetPrivateProfileInt(	_T( "playlist" ),
											_T( "NumberOfEntries" ),
											0,
											strCnv.ToT( strPName ) );
	// check if entry is already present
	for ( i = 1; i<= nNumEntries; i++ )
	{
		// Rewrite the number of files
		strNumber.Format( _W( "%d" ), i );

        CUStringConvert strCnv1;

		GetPrivateProfileString(	_T( "playlist" ),
									strCnv.ToT( CUString( _W( "File" ) + strNumber ) ),
									NULL,
									pszEntry,
									sizeof( pszEntry ),
									strCnv1.ToT( strPName ) );

		if ( 0 == MakeEntry( strPName, strEntry ).Compare( CUString( pszEntry, CP_ACP ) ) )
		{
			bFound = TRUE;
			break;
		}
	}
	
	if ( FALSE == bFound )
	{
		// increment the number of entries
		nNumEntries++;

		// Rewrite the number of files
		strNumber.Format( _W( "%d" ) , nNumEntries  );

        CUStringConvert strCnv;
        CUStringConvert strCnv1;
        CUStringConvert strCnv2;

		// Write number of entries
		WritePrivateProfileString(	_T( "playlist" ), 
									_T( "NumberOfEntries" ),
									strCnv1.ToT( strNumber ),
									strCnv.ToT( strPName ) );

		// Write version tag
		WritePrivateProfileString(	_T( "playlist" ),
									_T( "Version" ),
									_T( "2" ),
									strCnv.ToT( strPName ) );

		// Write Entry
		WritePrivateProfileString(	_T( "playlist" ),
									strCnv.ToT( CUString( _W( "File" ) + strNumber ) ),
									strCnv1.ToT( MakeEntry( strPName, strEntry ) ),
									strCnv2.ToT( strPName ) );
	}
}

void PlayList::AddM3UEntry(CUString strEntry)
{
	CUString strTmp;
	CUString strPName( m_strFileName + _W( ".m3u" ) );
	TCHAR	pszEntry[ MAX_PATH + 1 ] = { _T( '\0' ),};
	BOOL	bFound = FALSE;

	// Try to open the file:
	FILE* pFile = CDexOpenFile( strPName, _W( "a+" ) );
	
	if ( NULL != pFile )
	{
		// search if the entry is already present in the file
		while ( _fgetts( pszEntry, sizeof( pszEntry ), pFile  ) )
		{
			// check if entry is present
			if ( 0 == ( MakeEntry( strPName, strEntry ) + _W( "\n" ) ).Compare( pszEntry ) )
			{
				bFound = TRUE;
				break;
			}
		}

		if ( FALSE == bFound )
		{
            CUStringConvert strCnv;

			// Write the Entry
			_fputts( strCnv.ToT( MakeEntry( strPName, strEntry ) + _W( "\n" ) ), pFile );
		}

		// Close the playlist file
		fclose( pFile );
	}
}



// CDInfo CONSTRUCTOR
CDInfo::CDInfo()
{
	m_dwVolID = 0x00;
	m_dwDiscID = 0x00;
	m_nNumTracks = 0;
	m_strVolID= _T( "" );
	m_nRevision = -1;
	m_strCDDBCat = _T( "" );
	m_strGenre = _T( "" );
	m_strCDDBCat = _T( "" );
	m_strExtTitle  = _T( "" );
	m_strMCDI  = _T( "" );
	m_strMCN  = _T( "" );
	m_strSubmission  = _T( "" );
	m_strTitle = _T( "" );
	m_strYear = _T( "" );

}

// CDInfo DESTRUCTOR
CDInfo::~CDInfo()
{
}

BOOL CDInfo::ReadWinampDBString( CFile& cFile, CUString& strRead )
{
	BOOL bReturn = TRUE;

	vector<CHAR> vChars;

	ENTRY_TRACE( _T( "CDInfo::ReadWinampDBString()" ) );

	// clear return string
	strRead = _T( "" );

	CHAR ch;

	while ( 1 == cFile.Read( &ch, sizeof( CHAR) )  && ( ch != _T( '\0' ) ) )
	{
		vChars.push_back( ch );
	}

	if ( ch != _T( '\0' ) )
	{
		bReturn = FALSE;
	}

    strRead = CUString( &vChars[0], CP_UTF8 );


	ENTRY_TRACE( _T( "CDInfo::ReadWinampDBString(\"%s\"), return value: %d" ), strRead, bReturn );

	return bReturn;
}

// Read entry from Winamp (2.78) database file
BOOL CDInfo::ReadFromWinampDB()
{
	BOOL bReturn = TRUE;
	BOOL bEntryFound = FALSE;

	ENTRY_TRACE( _T( "CDInfo::ReadFromWinampDB()" ) );

	if ( g_config.GetWinampDBPath().GetLength() > 0 )
	{
		CUString strWinampDBFileName( g_config.GetWinampDBPath() + _W( "in_cdda.cdb" ) );
        CUStringConvert strCnv;


		CFile cWinampFile;

		if ( FALSE == cWinampFile.Open( strCnv.ToT( strWinampDBFileName ), CFile::modeRead | CFile::shareDenyNone  ) )
		{
			bReturn = FALSE;
		}
		else
		{
			DWORD dwMagic     = 0;
			DWORD dwOffsetIDs = 0;

			if ( ( TRUE == bReturn ) &&
				 ( sizeof( DWORD ) != cWinampFile.Read( &dwMagic, sizeof( DWORD ) ) ) )
			{
				bReturn = FALSE;
			}

			// check Magic ID, only support Winamp 2.78 format
			if ( 0xBEEFF00D != dwMagic )
			{
				bReturn = FALSE;
			}

			// Read CDDBID offset point
			if ( ( TRUE == bReturn ) &&
				 ( sizeof( DWORD ) != cWinampFile.Read( &dwOffsetIDs, sizeof( DWORD ) ) ) )
			{
				bReturn = FALSE;
			}

			// Seek tp the CDDBID offset
			if ( ( TRUE == bReturn ) &&
				( -1 == cWinampFile.Seek( dwOffsetIDs, CFile::begin ) ) )
			{
				bReturn = FALSE;
			}

			// search for CDDBID entry
			while ( ( TRUE == bReturn ) &&
					( FALSE  == bEntryFound ) )  
			{

				DWORD dwDiscID    = 0;
				DWORD dwSize      = 0;
				DWORD dwOffset    = 0;

				if ( ( TRUE == bReturn ) &&
					 ( sizeof( DWORD ) != cWinampFile.Read( &dwDiscID, sizeof( DWORD ) ) ) )
				{
					bReturn = FALSE;
				}

				if ( ( TRUE == bReturn ) &&
					 ( sizeof( DWORD ) != cWinampFile.Read( &dwSize, sizeof( DWORD ) ) ) )
				{
					bReturn = FALSE;
				}

				if ( ( TRUE == bReturn ) &&
					 ( sizeof( DWORD ) != cWinampFile.Read( &dwOffset, sizeof( DWORD ) ) ) )
				{
					bReturn = FALSE;
				}

				// Check if this is the CDDBID entry were looking for
				if ( ( TRUE == bReturn ) &&
					 ( dwDiscID == m_dwDiscID ) )
				{
					bEntryFound = TRUE;

					// seek to file entry
					if ( ( TRUE == bReturn ) &&
						( -1 == cWinampFile.Seek( dwOffset + 12, CFile::begin ) ) )
					{
						bReturn = FALSE;
					}
				}
			}

			// Read entry when it has been located
			if ( TRUE == bEntryFound )
			{
				CUString strRead;
				DWORD	dwTrack  = 0;
				DWORD	dwTracks = 0;

				// Get Artist-Album tag
				bReturn = ReadWinampDBString( cWinampFile, strRead );

				if ( bReturn == TRUE )
				{
					SetTitle( strRead );
				}

				// Get Artist tag
				if ( bReturn == TRUE )
				{
					bReturn = ReadWinampDBString( cWinampFile, strRead );
				}

				if ( bReturn == TRUE )
				{
					SetArtist( strRead );
				}

				// Get ??? tag
				if ( bReturn == TRUE )
				{
					bReturn = ReadWinampDBString( cWinampFile, strRead );
				}

				// Read num tracks
				if ( ( TRUE == bReturn ) &&
					 ( sizeof( DWORD ) != cWinampFile.Read( &dwTracks, sizeof( DWORD ) ) ) )
				{
					bReturn = FALSE;
				}

				while ( ( TRUE == bReturn ) &&
						( dwTrack != dwTracks ) )
				{
					bReturn = ReadWinampDBString( cWinampFile, strRead );
					SetTrackName( strRead, dwTrack );
					dwTrack++;
				}

			}
		}
	}
	else
	{
		bReturn = FALSE;
	}

	ENTRY_TRACE( _T( "CDInfo::ReadFromWinampDB(), return value: %d" ), bReturn );

	return bReturn;
}



BOOL CDInfo::IsAudioTrack(int p_track)
{
	// Loop through the available tracks
	for (int i=1; i<=m_nNumTracks; i++) 
	{
		// Located track
		if (m_TrackInfo[i-1].GetTrack() == p_track) 
		{
			// Check if this is an AUDIO track
			if ( !(m_TrackInfo[i-1].GetFlags() & CDROMDATAFLAG) )
				return TRUE;
		}
	}
	return FALSE;
}



void CDInfo::SetStartSector( int p_track, DWORD dwOffset)
{
	int i;

	// Loop through the available tracks
	for (i=0; i<m_nNumTracks; i++) 
	{
		BYTE bTest=m_TrackInfo[i].GetTrack();
		if (m_TrackInfo[i].GetTrack() == p_track) 
		{
			// Set start
			m_TrackInfo[i].SetStartSector(dwOffset);
			return;
		}
	}
}

LONG CDInfo::GetStartSector( int p_track )
{
	int lReturn = -1;
	int i;

	ENTRY_TRACE( _T( "CDInfo::GetStartSector( %d )" ), p_track );
	
	// Loop through the available tracks
	for ( i = 0; i < m_nNumTracks; i++ ) 
	{
		BYTE bTest=m_TrackInfo[i].GetTrack();
		if (m_TrackInfo[i].GetTrack() == p_track) 
		{
			// Return next track start record -1
			lReturn = m_TrackInfo[i].GetStartSector();
			break;
		}
	}

	EXIT_TRACE( _T( "CDInfo::GetStartSector( ), return value %d" ), lReturn );

	return lReturn;
}

BOOL CDInfo::IsLastAudioTrack(int p_track)
{
	int i;
	BOOL	bReturn = FALSE;
	DWORD	dwEndSector = 0;

	ENTRY_TRACE( _T( "CDInfo::IsLastAudioTrack( %d )" ), p_track );

	// Loop through the available tracks
	for ( i = 1; i <= m_nNumTracks; i++ ) 
	{
		if (	( p_track      == m_TrackInfo[ i - 1 ].GetTrack() ) ||
				( LEAD_OUT_NBR == m_TrackInfo[ i - 1 ].GetTrack() ) ) 
		{
			// check if we cross a session border
			// and check if the next track is not the default audio lead-out
			// but not the first track, due to extra CD format
			// where first track could be a data track, the second an audio track
			if ( ( ( m_TrackInfo[ i - 1 ].m_btFlags & CDROMDATAFLAG ) != ( m_TrackInfo[ i ].m_btFlags & CDROMDATAFLAG ) ) &&
				 ( LEAD_OUT_NBR != m_TrackInfo[i].m_btTrack ) )
			{
				bReturn = TRUE;
			}

			break;
		}
	}

	EXIT_TRACE( _T( "CDInfo::IsLastAudioTrack(), return value %d"), bReturn );

	return bReturn;
}

LONG CDInfo::GetSaveEndSector(int p_track)
{
	LONG	lReturn = -1;

	ENTRY_TRACE( _T( "CDInfo::GetSaveEndSector( %d )" ), p_track );

	lReturn = GetEndSector( p_track );

	if ( IsLastAudioTrack( p_track ) )
	{
		lReturn -= 10;
	}

	EXIT_TRACE( _T( "CDInfo::GetSaveEndSector(), return value %d"), lReturn );

	return lReturn;

}

LONG CDInfo::GetEndSector(int p_track)
{
	int i;
	LONG	lReturn = -1;
	DWORD	dwEndSector = 0;

	ENTRY_TRACE( _T( "CDInfo::GetEndSector( %d )" ), p_track );

	// Loop through the available tracks
	for ( i = 1; i <= m_nNumTracks; i++ ) 
	{
		if (	( p_track      == m_TrackInfo[ i - 1 ].GetTrack() ) ||
				( LEAD_OUT_NBR == m_TrackInfo[ i - 1 ].GetTrack() ) ) 
		{
			// check if we cross a session border
			// and check if the next track is not the default audio lead-out
			// but not the first track, due to extra CD format
			// where first track could be a data track, the second an audio track
			if ( IsLastAudioTrack( p_track ) )
			{
				// If we have crossed a session border, we get an additional leadout
				// Session lead-out = 1m30s => 6750 sectors
				// Session lead-in  = 1m00s => 4500 sectors
				// Thus a total gap op 6750 + 4500 = 11250 sectors
				// Since were not interested in the additional data (non-audio) we tweak the
				// start sector
				LTRACE( _T( "Correct end-sector of track %d for session lead-out time by 11250 sectors " ), i + 1 );
				dwEndSector = m_TrackInfo[ i ].m_dwStartSector - 11250 - 1;
			}
			else
			{
				dwEndSector = m_TrackInfo[i].m_dwStartSector-1;
			}

			lReturn = dwEndSector;

			break;
		}
	}

	EXIT_TRACE( _T( "CDInfo::GetEndSector(), return value %d"), lReturn );

	return lReturn;
}


int CDInfo::FirstTrack()
{
	int i;

	// Loop through the available tracks
	for (i=0; i<m_nNumTracks; i++) 
	{
		WORD wTrack=m_TrackInfo[i].GetTrack();

//		if ( wTrack != LEAD_OUT_NBR && (m_TrackInfo[i].GetFlags() == AUDIOTRKFLAG ))
//			return wTrack;
	}
	// Track not found, return -1
	return 0;
}

void CDInfo::DumpToc(LPBYTE pbtBuffer,int nSize)
{
	int			i;
	DWORD		dw;
	unsigned	mins;
	double		secnds;

	// Open Dump File
	CADebug myLogFile( CUString( _W( "CDEX_TOC.txt" ) ) );

	// A header would not hurt
	myLogFile.printf("Start of TOC dump\n\n");

	// Dump binary data
	myLogFile.printf("Dumping binary data first, buffer size=%d =>%d toc entries\n\n",nSize,nSize/8);
	
	myLogFile.DumpBinaray(pbtBuffer,nSize);

	dw = GetStartSector(m_nNumTracks-1);

	// Calculate number of minutes
	mins= dw / (TRACKSPERSEC * 60L );

	secnds= (fmod(dw , 60 * TRACKSPERSEC) / (DOUBLE)TRACKSPERSEC);


	myLogFile.printf("\n\nDumping Toc Entries\n");
	for (i=0; i<m_nNumTracks; i++) 
	{
		myLogFile.printf("Toc Entry=%2d  StartSector=%8d Flags=%2d",i,m_TrackInfo[i].GetStartSector(),m_TrackInfo[i].GetFlags());
		// Skip nonaudio tracks 
//		if (m_TrackInfo[i].GetFlags() != AUDIOTRKFLAG) 
//			continue;					


		if (m_TrackInfo[i].GetTrack() <= MAXTRK) 
		{
			DWORD dw2;

			dw2= m_TrackInfo[i].GetStartSector();

			dw = m_TrackInfo[i+1].GetStartSector();

			mins= (dw - dw2) / (TRACKSPERSEC*60L);
			secnds= fmod(dw-dw2 , 60*TRACKSPERSEC) / TRACKSPERSEC;


			CUString strDuration=GetTrackDuration(i+1);
			CUString strStart=GetStartTime(i+1);

			CUString strNew;
			myLogFile.printf(" => Start time=%s Duration=%s\n",strStart,strDuration);
		}

	}
	myLogFile.printf("\n\nEnd of TOC dump\n");
}



CUString CDInfo::GetTrackDuration(int nTrack)
{
	int		nMins = 0;
	int		nSecs = 0;
	int		nFrame = 0;
	CUString	strTime;

	ENTRY_TRACE( _T( "CDInfo::GetTrackDuration( %d )" ), nTrack );

	LONG lStartSector = GetStartSector( nTrack );
	LONG lEndSector = GetEndSector( nTrack );

	if ( ( -1 != lStartSector ) &&  ( -1 != lEndSector ) )
	{
		LONG	lSectors = 0;

		lSectors = lEndSector - lStartSector + 1;
		if ( lSectors > 0 )
		{
			nFrame	 = lSectors % TRACKSPERSEC;
			nMins	 = lSectors / (TRACKSPERSEC * 60L );
			nSecs	 = (int)(fmod( (double)lSectors, 60 * TRACKSPERSEC ) / TRACKSPERSEC);
		}
	}		


	strTime.Format( _W( "%u:%02u.%02u" ), nMins, nSecs, nFrame );

	EXIT_TRACE( _T( "CDInfo::GetTrackDuration(), return value %s" ), strTime );

	return strTime;
}


CUString CDInfo::GetStartTime( int nTrack )
{
	int		nMins;
	int		nSecs;
	int		nFrame;
	CUString strTime;

	ENTRY_TRACE( _T( "CDInfo::GetStartTime( %d )" ), nTrack );

	strTime;

	LONG  lSectors = GetStartSector(nTrack);

	nMins	= lSectors / (TRACKSPERSEC * 60L );
	nSecs	= (int)(fmod( (double)lSectors, 60L * TRACKSPERSEC ) / TRACKSPERSEC);
	nFrame	= lSectors % TRACKSPERSEC;

	strTime.Format( _W( "%u:%02u.%02u" ), nMins, nSecs, nFrame );

	EXIT_TRACE( _T( "CDInfo::GetStartTime(), return value %s" ), strTime );

	return strTime;
}



int cddb_sum(int n)
{
	char	buf[12] = { 0, };
	char*	p = NULL;
	int	ret = 0;

	// For backward compatibility this algorithm must not change
	sprintf(buf, "%lu", n);

	for (p = buf; *p != '\0'; p++)
	{
		ret += (*p - '0');
	}

	return ( ret );
}

DWORD CDInfo::CalculateDiscID()
{
	DWORD dwRet;
	DWORD t = 0;
	DWORD n = 0;
	DWORD	dwTwoSecOffset=0;

	ENTRY_TRACE( _T( "CDInfo::CalculateDiscID()" ) );

	m_dwTotalSecs=0;

	// For backward compatibility this algorithm must not change
	dwTwoSecOffset = 2 * TRACKSPERSEC;

	for (int i = 0; i < m_nNumTracks; i++) 
	{

		// Keep in mind the two seconds offset
		DWORD dwSectors = m_TrackInfo[i].GetStartSector()+dwTwoSecOffset;

		n += cddb_sum(dwSectors/TRACKSPERSEC);

		// Keep in mind the two seconds offset
		DWORD dwSectorsNext = m_TrackInfo[i+1].GetStartSector()+dwTwoSecOffset;

		t += (dwSectorsNext/TRACKSPERSEC-dwSectors/TRACKSPERSEC);
	}

	dwRet=( (n % 0xff) << 24 | t << 8 | (DWORD)(m_nNumTracks));

	// Get total playing time
	m_dwTotalSecs=( GetEndSector( m_nNumTracks ) + 1 + 2 * TRACKSPERSEC ) / TRACKSPERSEC;
	
	EXIT_TRACE( _T( "CDInfo::CalculateDiscID(), return value 0x%x" ), dwRet );

	return dwRet;
}


DWORD CDInfo::GetSize(int nTrack)
{
	LONG lSectors = (LONG)GetEndSector( nTrack )- (LONG)GetStartSector( nTrack );
	if ( lSectors <= 0 )
		lSectors = 0;

	return lSectors*CB_CDDASECTORSIZE;

}

CCDTrackInfo::CCDTrackInfo()
{
	m_reserved1=0;
	m_btFlags=0;
	m_btTrack=0;
	m_reserved2=0;
	m_dwStartSector=0;
    m_nCDDBType = 0;
    m_dwGap = 0;
    m_strTrackName = _T("");
    m_strISRC = _T("");
    m_strExtTrackName =_T("");
}


void CDInfo::Init()
{
	CUString strLang;

	ENTRY_TRACE( _T( "CDInfo::Init() " ) );

    strLang = g_language.GetString( IDS_NOARTIST );
	SetArtist( strLang );
    strLang = g_language.GetString( IDS_NOTITLE );
	SetTitle( strLang );
	SetExtTitle( CUString( _T("") ) );
	m_nRevision = -1;
	SetCDDBCat( CUString( _T("") ) );
	m_strYear= _T("");

	strLang = g_language.GetString( IDS_UNKNOWN );
	m_strGenre= strLang;

	int nNumTracks=GetNumTracks();

	for (int i=0;i<MAXTRK;i++)
	{
		CUString strTmp;

		strLang = g_language.GetString( IDS_AUDIOTRACK );

		strTmp.Format( _W("%s %02d"), (LPCWSTR)strLang, i+1 );
		SetTrackName( strTmp, i );
		SetExtTrackName( CUString( _T("") ), i );
	}

	for (int i=0;i<nNumTracks;i++)
	{
		CUString strTmp;
		if ( IsAudioTrack( i+1 ) )
		{
			strLang = g_language.GetString( IDS_AUDIOTRACK );
		}
		else
		{
			strLang = g_language.GetString( IDS_DATATRACK );
		}
		strTmp.Format( _W( "%s %02d" ), (LPCWSTR)strLang, i + 1 );

		// Set track name
		SetTrackName(strTmp,i);
		SetExtTrackName( _W(""),i);
	}

	// Set revision level
	m_nRevision=-1;

	// Set submission
	m_strSubmission= _T( "CDex(local)" );

	EXIT_TRACE( _T( "CDInfo::Init() " ) );

}


BOOL CDInfo::ReadCDText()
{
	CDEX_ERR	bErr=CDEX_ERROR;
	const int	nBufferSize=4 + 8 * sizeof(CDTEXTPACK) * 256;
	BYTE*		pbtBuffer=NULL;
	INT			nCDTextSize=0;
	INT			nTrack=0;
	CHAR*		lpZero = NULL;

	ENTRY_TRACE( _T( "CDInfo::ReadCDText()" ) );

	// Allocate CD-Text buffer
	pbtBuffer = new BYTE[ nBufferSize ];

	// Try to Read CD-Text section
	bErr = CR_ReadCDText( pbtBuffer, nBufferSize, &nCDTextSize );

	// Check size CD-Text buffer
	if ( nCDTextSize < 4 )
		bErr = CDEX_ERROR;

	// Did it succeed ?
	if ( CDEX_OK == bErr )
	{
		int			i;
		int			nNumPacks=(nCDTextSize-4)/sizeof(CDTEXTPACK);
		CDTEXTPACK* pCDtextPacks=NULL;
		char		lpszBuffer[1024]={'\0',};
		int			nInsertPos=0;
		CUString		strText = _T( "" );

		LTRACE( _T( "CDInfo::ReadCDText nCDTextSize = %d, nNumPacks = %d" ), nCDTextSize, nNumPacks );

		bErr = CDEX_ERROR;

		/* loop through all the packets */
		for ( i=0; i < nNumPacks; i++ )
		{
			/* get CD-Text packet overlay */
			pCDtextPacks=(CDTEXTPACK*)&pbtBuffer[ i * sizeof(CDTEXTPACK) + 4 ];

			LTRACE( _T( "CDInfo::ReadCDText pack %02X packType %02X trackNumber %02X sequenceNumber %02X characterPosition %02X block %02X bDBC %02X" ),
				i,
				pCDtextPacks->packType,
				pCDtextPacks->trackNumber,
				pCDtextPacks->sequenceNumber,
				pCDtextPacks->characterPosition,
				pCDtextPacks->block,
				pCDtextPacks->bDBC );

			LTRACE( _T( "data %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X" ),
				pCDtextPacks->data[0],
				pCDtextPacks->data[1],
				pCDtextPacks->data[2],
				pCDtextPacks->data[3],
				pCDtextPacks->data[4],
				pCDtextPacks->data[5],
				pCDtextPacks->data[6],
				pCDtextPacks->data[7],
				pCDtextPacks->data[8],
				pCDtextPacks->data[9],
				pCDtextPacks->data[10],
				pCDtextPacks->data[11] );


			/* only get block number zero */
			if ( 0 == pCDtextPacks->block )
			{
				WORD	nChar = 0;

				while ( nChar < sizeof( pCDtextPacks->data ) / sizeof( pCDtextPacks->data[0] ))
				{
					BOOL	bFoundTeminateChar = FALSE;
					while ( nChar < sizeof( pCDtextPacks->data ) / sizeof( pCDtextPacks->data[0] ) &&
							!bFoundTeminateChar )
					{

						if ( _T( '\0' ) == (CHAR)pCDtextPacks->data[ nChar ] )
						{
							bFoundTeminateChar = TRUE;
						}
						else
						{
							strText+= (CHAR)pCDtextPacks->data[ nChar ];
						}

						nChar++;
					}


					// process all strings
					if (	bFoundTeminateChar  && 
							!strText.IsEmpty() )
					{
						LTRACE( _T( "CDTextEntry: type = %d, trackNumber = %d value = \"%s\" " ),
									pCDtextPacks->packType,
									pCDtextPacks->trackNumber,
									strText );
						
						switch ( pCDtextPacks->packType )
						{
							case CDT_TRACK_TITLE:
								if ( 0 == pCDtextPacks->trackNumber )
								{
									SetTitle( strText );
									bErr = CDEX_OK;
								}
								else
								{
									bErr = CDEX_OK;
									SetTrackName(	strText,
													pCDtextPacks->trackNumber - 1 );
								}
							break;

							case CDT_PERFORMER:
								if ( 0 == pCDtextPacks->trackNumber )
								{
									bErr = CDEX_OK;
									SetArtist( strText );
								}
								else
								{
									bErr = CDEX_OK;
									SetTrackName(	strText + _W( " / " ) + GetTrackName( pCDtextPacks->trackNumber - 1 ),
													pCDtextPacks->trackNumber - 1 );
								}
							break;
							default:
							break;
						}

						pCDtextPacks->trackNumber++;

						strText = _T( "" );
					}
				}
			}
		}
	}

	// Delete CD-Text buffer
	delete[] pbtBuffer;

	EXIT_TRACE( _T( "CDInfo::ReadCDText(), return value: %d" ), CDEX_OK == bErr );

	return ( CDEX_OK == bErr );
}

BOOL CDInfo::ReadCDPlayerIni()
{
	BOOL		bFound=FALSE;
	TCHAR		lpszPathName[ 255 ];
	CDPlayerIni	cdPlayerIni;

	ENTRY_TRACE( _T( "CDInfo::ReadCDPlayerIni()" ) );

	GetWindowsDirectory( lpszPathName, sizeof( lpszPathName ) -1 );
	_tcscat( lpszPathName, _T( "\\CDPlayer.ini" ) );

	cdPlayerIni.Read( lpszPathName );

	LTRACE( _T( "Ini File read from disk " ) );

	CUString strIni;
	int nIni = 0;
	
	strIni = cdPlayerIni.GetValue( m_strVolID, _T( "artist" ) , _T( "" ) );


	if ( !strIni.IsEmpty() ) 
	{
		LTRACE( _T( "Entry found in CDPlayer.ini file " ) );

		bFound = TRUE;

		strIni = cdPlayerIni.GetValue( m_strVolID, _T( "artist" ), GetArtist() );
		SetArtist( strIni );

		strIni = cdPlayerIni.GetValue( m_strVolID, _T( "title" ), GetTitle() );
		SetTitle( strIni );

		strIni = cdPlayerIni.GetValue( m_strVolID, _T( "year" ), GetYear() );
		SetYear( strIni );

		strIni = cdPlayerIni.GetValue( m_strVolID, _T( "genre" ), GetGenre() );
		SetGenre( strIni );

		// Read Track Information
		int nNumTracks=GetNumTracks();

		for ( int i=0; i<nNumTracks; i++ )
		{
			CUString strTrack;
			strTrack.Format( _W( "%d" ), i );

			strIni = cdPlayerIni.GetValue( m_strVolID, strTrack, GetTrackName( i ) );
			SetTrackName( strIni, i );
		}
	}

	EXIT_TRACE( _T( "CDInfo::ReadCDPlayerIni() with return value %d" ), bFound );

	return bFound;
}

void CDInfo::ReadCDInfo()
{
	ENTRY_TRACE( _T( "CDInfo::ReadCDInfo()" ) );

	// Set DiscID labal
	m_dwDiscID = CalculateDiscID();


	// Initialize Tracknames etc.
	Init();

	LTRACE( _T( "CDInfo::ReadCDInfo(), try to read entry from ReadLocalCDDB" ) );

	if ( m_nNumTracks > 0 )
	{
		// Try to get Disc info first from CDDB, if not found, then from CDPlayer.ini
		if ( FALSE == ReadLocalCDDB() )
		{
			LTRACE( _T( "CDInfo::ReadCDInfo(), try to read entry from ReadCDPlayerIni" ) );

			if ( FALSE == ReadCDPlayerIni() )
			{
				LTRACE( _T( "CDInfo::ReadCDInfo(), try to read entry from WinampDB" ) );

				if ( FALSE == ReadFromWinampDB() )
				{
					LTRACE( _T( "CDInfo::ReadCDInfo(), try to read entry from ReadCDText" ) );

					// Finally rry to read CD-Text
					ReadCDText();
				}
			}
		}
	}

	EXIT_TRACE( _T( "CDInfo::ReadCDInfo()" ) );
}


void CDInfo::SaveCDInfo()
{
	ENTRY_TRACE( _T( "CDInfo::SaveCDInfo()" ) );

	if ( g_config.GetSaveToLocCDDB() )
	{
		CDdb localCDDB( this, NULL );

		// Save it to local CDDB
		localCDDB.WriteCurrentEntry();
	}

	if ( g_config.GetSaveToCDPlayer() )
	{
		SaveCDPlayerIni();
	}

	EXIT_TRACE( _T( "CDInfo::SaveCDInfo()" ) );
}


void CDInfo::SaveCDPlayerIni()
{
	BOOL		bFound=FALSE;
	TCHAR		lpszPathName[ MAX_PATH + 1 ];
	CDPlayerIni	cdPlayerIni;

	ENTRY_TRACE( _T( "CDInfo::SaveCDPlayerIni()" ) );

	GetWindowsDirectory( lpszPathName, MAX_PATH );

	_tcscat( lpszPathName, _T( "\\CDPlayer.ini" ) );

	cdPlayerIni.Read( lpszPathName );

	// Get total number of tracks
	int nNumTracks=GetNumTracks();

	cdPlayerIni.SetValue( m_strVolID, _T( "entryType" ), 1 );
	cdPlayerIni.SetValue( m_strVolID, _T( "artist" ), GetArtist() );
	cdPlayerIni.SetValue( m_strVolID, _T( "title" ), GetTitle() );
	cdPlayerIni.SetValue( m_strVolID, _T( "numTracks" ), nNumTracks );
						 
	// Write track info
	for (int i=0;i<nNumTracks;i++)
	{
		CUString strTrack;
		strTrack.Format( _W( "%d" ), i );
		cdPlayerIni.SetValue( m_strVolID, strTrack ,GetTrackName(i) );
	}


	cdPlayerIni.SetValue( m_strVolID, _W( "numplay" ), nNumTracks );
	cdPlayerIni.SetValue( m_strVolID, _W( "year" ), GetYear() );
	cdPlayerIni.SetValue( m_strVolID, _W( "genre" ), GetGenre() );

	cdPlayerIni.Write( lpszPathName );

	EXIT_TRACE( _T( "CDInfo::SaveCDPlayerIni()" ) );

}

BOOL CDInfo::ReadLocalCDDB()
{
	ENTRY_TRACE( _T( "CDInfo::ReadLocalCDDB()" ) );

	// Can take a while
	CWaitCursor Wait;

	if ( m_nNumTracks > 0 )
	{
	//	Init();
		CDdb localCDDB( this, NULL );

        CUStringConvert strCnv;


		if ( GetPrivateProfileInt(	_T( "Debug" ),
									_T( "AlwaysAddToBatch" ),
									0,
									strCnv.ToT( g_config.GetIniFileName() ) ) )
		{
			LTRACE( _T( "CDInfo::ReadLocalCDDB() store to Batch Query" ) );

			localCDDB.AddRemoveQueryBatch( TRUE );
		}

		if ( localCDDB.QueryLocal() )
		{
			LTRACE( _T( "CDInfo::ReadLocalCDDB() found, return TRUE" ) );
			return TRUE;
		}
		else
		{
			if (g_config.GetCDDBAutoConnect()==TRUE)
			{
				LTRACE( _T( "CDInfo::ReadLocalCDDB() Auto Connect" ) );

				// Create Remote CDDB dialog box
				// CCDdbDlg dlg( ReadRemoteCDDBThreadFunc, this );
                CGetRemoteCDInfoDlg dlg( this, false );
                
                // dlg.CDInfoPtr
				dlg.DoModal();

				LTRACE( _T( "CDInfo::ReadLocalCDDB() found, return TRUE" ) );

				return TRUE;
			}

		}

		LTRACE( _T( "CDInfo::ReadLocalCDDB() store to Batch Query" ) );

		// Add to batch query 
		localCDDB.AddRemoveQueryBatch( TRUE );
	}
	else
	{
		LTRACE( _T( "CDInfo::ReadLocalCDDB() m_nNumtracks < 0" ) );
	}

	LTRACE( _T( "CDInfo::ReadLocalCDDB() not found, return FALSE" ) );

	return FALSE;
}



BOOL CDInfo::ReadRemoteCDDB( CWnd* pWnd, volatile BOOL& bAbort )
{

	LTRACE( _T( "CDInfo::ReadRemoteCDDB( %x )" ), pWnd );

	CDdb localCDDB( this, pWnd );
	BOOL bReturn= localCDDB.QueryRemote( );

	// Do we need to save this information to the local CDPlayer.ini file?
	if (	( TRUE == bReturn) && 
			(g_config.GetSaveToCDPlayer() ) )
	{
		SaveCDPlayerIni();
	}

	LTRACE( _T( "CDInfo::ReadRemoteCDDB( ) returns %d " ), bReturn );

	return bReturn;
}

BOOL CDInfo::ReadRemoteCDDBBatch( CWnd* pWnd, volatile BOOL& bAbort )
{
	TCHAR	lpszQuery[ 2048 ] = { _T( '\0'),};
	CDInfo	cdInfo;
	BOOL	bReturn = TRUE;

	LTRACE( _T( "CDInfo::ReadRemoteCDDBBatch( %x )" ), pWnd );

	// open the batch remote file

	while ( ( FALSE == bAbort ) )
	{
        FILE* fp = NULL;
        
        fp = CDexOpenFile( CDDB_BATCH_FNAME, _W( "r" ) );
        
        if ( fp == NULL )
        {
            break;
        }

		TCHAR* linePtr = _fgetts( lpszQuery, sizeof( lpszQuery ), fp );

        fclose( fp );

        if ( linePtr == NULL )
        {
            break;
        }

		TCHAR*	pStrTok = NULL;
		INT		nTracks=0;
		INT		i=0;

		CUString	strLine( lpszQuery );

		fclose( fp );


		pStrTok= _tcstok( lpszQuery, _T( " " ) );

		LTRACE( _T( "CDInfo::ReadRemoteCDDBBatch( ) getting: \"%s\"" ), lpszQuery );

		// Get CDDB disc ID
		if ( pStrTok )
		{
			int nID=0;
			_stscanf(lpszQuery, _T( "%x" ), &nID );
			cdInfo.SetDiscID( nID );
			pStrTok = _tcstok( NULL, _T( " " ) );
		}

		// Get number of tracks
		if ( pStrTok )
		{
			nTracks = _ttoi( pStrTok );

			cdInfo.SetNumTracks( nTracks );
			pStrTok = _tcstok( NULL, _T( " " ) );
		}

		if (pStrTok)
		{
			// Get track offset
			for (i=0;i<nTracks;i++)
			{
				DWORD dwSec=_ttoi( pStrTok );

				cdInfo.SetTrack( i, i + 1 );

				cdInfo.SetStartSector( i + 1, dwSec );

				pStrTok = _tcstok( NULL, _T( " " ) );

                if ( pStrTok == NULL )
                {
                    break;
                }
			}
		}

		if ( pStrTok )
		{
			DWORD dwSec=_ttoi( pStrTok );
			cdInfo.SetTotalSecs( dwSec );
		}

		if ( cdInfo.GetNumTracks() < 99 )
		{
			if ( pStrTok )
			{
				CDdb	localCDDB( &cdInfo, pWnd );
				bReturn= localCDDB.QueryRemote();
			}
		}

		// remove invalid entry
		CDdb	localCDDB( &cdInfo, pWnd );
		localCDDB.RemoveFromCDDBBatch( strLine );

    }

	bReturn = TRUE;

	EXIT_TRACE( _T( "CDInfo::ReadRemoteCDDBBatch( ) with value %d" ), bReturn );

	return bReturn;
}

BOOL CDInfo::SubmitRemoteCDDB( CWnd* pWnd, volatile BOOL& bAbort  )
{
	ENTRY_TRACE( _T( "CDInfo::SubmitRemoteCDDB()" ) );

	CDdb localCDDB( this, pWnd );
	BOOL bReturn= localCDDB.SubmitRemote( );

	EXIT_TRACE( _T( "CDInfo::SubmitRemoteCDDB(), return value %d" ), bReturn );

	return bReturn;
}

UINT __cdecl ReadRemoteCDDBThreadFunc( void* pParam, CWnd* pWnd, volatile BOOL& bAbort  )
{
	return ((CDInfo*)pParam)->ReadRemoteCDDB( pWnd, bAbort );
}

UINT __cdecl SubmitRemoteCDDBThreadFunc(void* pParam, CWnd* pWnd, volatile BOOL& bAbort  )
{
	return ((CDInfo*)pParam)->SubmitRemoteCDDB( pWnd, bAbort );
}

UINT __cdecl BatchRemoteCDDBThreadFunc(void* pParam, CWnd* pWnd, volatile BOOL& bAbort  )
{
	return ((CDInfo*)pParam)->ReadRemoteCDDBBatch( pWnd, bAbort );
}


CDEX_ERR CDInfo::ReadUpcAndISRC()
{
	CDEX_ERR	bErr = FALSE;


	ENTRY_TRACE( _T( "CDInfo::ReadUpcAndISRC()" ) );

    MCISRC mcisrc ;
    CR_ReadAndGetISRC( &mcisrc );

    m_strMCN  = CUString( mcisrc.mmCatalog, CP_UTF8 );

	// Get the number of 
	int nTocEntries=CR_GetNumTocEntries();

	m_strMCDI = _T( "" );


	for (int i=0;i<=nTocEntries;i++)
	{
        CUString isrc( (LPCSTR)mcisrc.isrc[ i ], CP_ACP );
        m_TrackInfo[i].SetISRC( isrc );
	}

	EXIT_TRACE( _T( "CDInfo::ReadUpcAndISRC()" ) );

	return bErr;
}

CDEX_ERR CDInfo::ReadToc()
{
	CUString		strTmp;
	CDEX_ERR	bErr = FALSE;
	int			nAudioTracks = 0;

	ENTRY_TRACE( _T( "CDInfo::ReadToc()" ) );

	// Get information from CD-ROM
	bErr=CR_ReadToc();


    m_strMCN  = _T( "" );

	// Get the number of 
	int nTocEntries=CR_GetNumTocEntries();

	m_strMCDI = _T( "" );

	memset( m_btaRawToc, 0, sizeof( m_btaRawToc ) );

	for (int i=0;i<=nTocEntries;i++)
	{
		TOCENTRY myTocEntry;

		myTocEntry = CR_GetTocEntry(i);

		memcpy( &m_btaRawToc[ i * sizeof( TOCENTRY ) ], &myTocEntry, sizeof( TOCENTRY ) );

		m_TrackInfo[i].m_reserved1=0;
		m_TrackInfo[i].m_btFlags=myTocEntry.btFlag;
		m_TrackInfo[i].m_btTrack=myTocEntry.btTrackNumber;
		m_TrackInfo[i].m_reserved2=0;
		m_TrackInfo[i].m_dwStartSector=myTocEntry.dwStartSector;

        m_TrackInfo[i].SetISRC( _T("") );

        if ( ( 0 == i ) && myTocEntry.dwStartSector >= 80 * 60 * TRACKSPERSEC )
		{
			m_TrackInfo[i].m_dwStartSector = 0;
		}

 
		LTRACE( _T( "Track 0x%x, btFlags=0x%x, dwStartSector=%d" ),
				m_TrackInfo[i].m_btTrack,
				m_TrackInfo[i].m_btFlags,
				m_TrackInfo[i].m_dwStartSector
			
			);

	}

	// Get proper volume ID, and fix TOC using MCI
	m_dwVolID=0;

	m_nNumTracks=nTocEntries;


	// Compat = 1 for use with CDPLAYER.EXE 
	DWORD dwCompat=1;
	DWORD dwVolID=0;
	DWORD dwMagic=0;

	int i_nTocEntries=0;

	for (i_nTocEntries=0;i_nTocEntries<nTocEntries;i_nTocEntries++)
	{
		DWORD dwSector=m_TrackInfo[i_nTocEntries].m_dwStartSector+150;

		// Transform to MSF
		DWORD dwMin=(dwSector/(TRACKSPERSEC*60));
		DWORD dwSec=(dwSector/TRACKSPERSEC)%60;
		DWORD dwFrm=(dwSector%TRACKSPERSEC);

		// check if we cross a section border, if so, were done
		if ((i_nTocEntries>1) && (  ( m_TrackInfo[i_nTocEntries].m_btFlags & CDROMDATAFLAG ) !=
						( m_TrackInfo[i_nTocEntries-1].m_btFlags & CDROMDATAFLAG ) ) )
		{
			break;
		}

		// Add to total
		dwVolID += (dwMin<<16)+(dwSec<<8) + dwFrm;

		if (i_nTocEntries==0)
		{
			dwMagic = dwFrm;
		}
	}

	switch(i_nTocEntries)
	{
		case 1:
		case 2:
			dwVolID += (DWORD)( (int)m_TrackInfo[i_nTocEntries].m_dwStartSector - (int)dwMagic );
		break;
		default:
			// OKEY DOKEY
		break;
	}

	for (int i = 0; i <= nTocEntries; i++ )
	{
		// check if this is an audio track
	    if( m_TrackInfo[i].m_btFlags & CDROMDATAFLAG)
		{
			// Assume we have crossed a session boundary
			strTmp.Format( _W( "+%X" ), m_TrackInfo[ i ].m_dwStartSector - 11100 );
			m_strMCDI += strTmp;
			break;
		}

		strTmp.Format( _W( "+%X" ), m_TrackInfo[i].m_dwStartSector + 150);
		m_strMCDI += strTmp;

		// check for lead-out
		if( LEAD_OUT_NBR == m_TrackInfo[ i ].m_btTrack )
		{
			break;
		}

		// increase number of audio tracks
	    nAudioTracks++;
	}

	// Format (in hex) the total number of audio tracks 
	strTmp.Format( _W( "%X" ), nAudioTracks );

	// Add number of audio tracks to MCDI string
	m_strMCDI = strTmp + m_strMCDI;

	// If this contains only one track, add track length, IN SECTORS!
/*
	if (nTocEntries<2)
	{
		// Add to total
		dwVolID+=m_TrackInfo[1].m_dwStartSector;
	}
*/
	// Assign volume ID
	m_dwVolID=dwVolID;

	// Debug
	LTRACE( _T( "Volume ID is calculated, value is 0x%x" ), dwVolID );

	m_strVolID.Format( _W( "%X" ), m_dwVolID );

	EXIT_TRACE( _T( "CDInfo::ReadToc()" ) );

	return bErr;
}


CUString	CDInfo::GetCDDBCat()
{

	ENTRY_TRACE( _T( "CDInfo::GetCDDBCat()" ) );

	// if this is a new entry, set the CDDB catalog
	// based on the current Genre
	if ( ( GetRevision() < 0 ) || m_strCDDBCat.IsEmpty() )
	{
		int i = 0;
		i = g_GenreTable.SearchGenre( m_strGenre );
		m_strCDDBCat = g_GenreTable.GetCDDBGenre( i );
	}

	EXIT_TRACE( _T( "CDInfo::GetCDDBCat() with value %s" ), m_strCDDBCat );

	return m_strCDDBCat;

}


void CDInfo::SetCDDBCat( const CUString& strCDDBCat )
{
	ENTRY_TRACE( _T( "CDInfo::SetCDDBCat( \"%s\" )" ), strCDDBCat );

	m_strCDDBCat = strCDDBCat;

	EXIT_TRACE( _T( "CDInfo::SetCDDBCat()" ) );

}



void CDInfo::SetGenre( CUString strValue )
{
	ENTRY_TRACE( _T( "CDInfo::SetGenre( \"%s\" )" ), strValue );

	// Check if genre is availble in list
	if ( -1 == g_GenreTable.SearchGenre( strValue ) )
	{
		// Not present, add it!
		g_GenreTable.AddEntry( -1, strValue, _T( "misc" ), true );

		// Save the genre table!
		g_GenreTable.Save( GENRE_TABLE_FILE_NAME );

	}

	// Set string
	m_strGenre = strValue;

	EXIT_TRACE( _T( "CDInfo::SetGenre( ) with m_strGenre value \"%s\" " ), m_strGenre );

}
/*
void CDInfo::SetTrack( BYTE btTrack )
{
	ENTRY_TRACE( _T( "CDInfo::SetTrack( %d )" ), btTrack );

	m_btTrack = btTrack;

	EXIT_TRACE( _T( "CDInfo::SetTrack()" ) );
}
*/
void CDInfo::SetTrackName( const CUString& strValue, int iTrackIndex )
{

	ENTRY_TRACE( _T( "CDInfo::SetTrackName( \"%s\", %d )" ), strValue, iTrackIndex );

	m_TrackInfo[ iTrackIndex ].SetTrackName( strValue );

	EXIT_TRACE( _T( "CDInfo::SetTrackName()" ) );
}

void CDInfo::SetExtTrackName( const CUString& strValue, int iExtTrackIndex )
{
	ENTRY_TRACE( _T( "CDInfo::SetTrackName( \"%s\", %d )" ), strValue, iExtTrackIndex );

	m_TrackInfo[ iExtTrackIndex ].SetExtTrackName( strValue );

	EXIT_TRACE( _T( "CDInfo::SetExtTrackName()" ) );
}


void CDInfo::SetArtist( const CUString& strValue )
{
	ENTRY_TRACE( _T( "CDInfo::SetArtist( \"%s\" )" ), strValue );

	m_strArtist = strValue;

	EXIT_TRACE( _T( "CDInfo::SetArtist()" ) );
}

void CDInfo::SetTitle( const CUString& strValue )
{
	ENTRY_TRACE( _T( "CDInfo::SetTitle( \"%s\" )" ), strValue );

	m_strTitle = strValue;

	EXIT_TRACE( _T( "CDInfo::SetTitle()" ) );
}

