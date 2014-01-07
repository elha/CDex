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
#include "EncoderWMA8Dll.h"
#include "EncoderWMA8DllDlg.h"
#include "TaskInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderWMA8Dll" ) );


// CONSTRUCTOR

CEncoderWMA8Dll::CEncoderWMA8Dll() : 
	CEncoder(),
	m_pCODEC( NULL ),
	m_pCreateNewCompressionObject( NULL ),
	m_pDeleteCompressionObject( NULL ),
	m_pANewDataBlockArrived( NULL ),
	m_pShowConfigBit( NULL ),
	m_pRemoveConfigBit( NULL ),
	m_pGetExtensionYouCreate( NULL ),
	m_pSetIDTagElement( NULL ),
	m_pSetSettings( NULL ),
	m_hDLL( NULL )

{
	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding=TRUE;

	m_strEncoderPath = _T( "NoFileNeeded" );
	m_strEncoderID = _T( "Microsoft WMA Encoder" );
	m_strExtension = _T( "wma" );

	// All DLL derivates support chunk support
	m_bChunkSupport = TRUE;

	m_bCanWriteTagV1 = FALSE;
	m_bCanWriteTagV2 = FALSE;

	m_nEncoderID = ENCODER_WMA;

	LoadSettings();

	m_dwInBufferSize = 1024;

	// only stereo seems to work
	m_nMode = 1;

}


// DESTRUCTOR
CEncoderWMA8Dll::~CEncoderWMA8Dll()
{
    m_Encoder.Cleanup();
}

CEncoderDlg* CEncoderWMA8Dll::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg = new CEncoderWMA8DllDlg;
	}
	return m_pSettingsDlg;
}


CDEX_ERR CEncoderWMA8Dll::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR bReturn = CDEX_OK;

	CEncoder::InitEncoder( pTask );

#ifdef WMA8
	OSVERSIONINFO osversioninfo;
	bool osisnt = false;

	m_bAvailable = FALSE;


    try {
	    osversioninfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx( &osversioninfo );

		if ( VER_PLATFORM_WIN32_NT == osversioninfo.dwPlatformId )
		{
	    	osisnt = true; //NT or 2000
	    }
		
		if ( osisnt )
		{
			m_hDLL = LoadLibrary( _T( "wmvcore.dll" ) );
    	    if (!m_hDLL) return false;
	        FreeLibrary(m_hDLL);
        }
	    m_hDLL = NULL;
		m_hDLL = LoadLibrary( _T( "WMA8Connect.dll" ) );

    	if ( NULL == m_hDLL) 
			return false;

		m_pCreateNewCompressionObject = (LPCreateNewCompressionObject)GetProcAddress( m_hDLL, "CreateNewCompressionObject" );
		m_pDeleteCompressionObject = (LPDeleteCompressionObject)GetProcAddress( m_hDLL, "DeleteCompressionObject" );
		m_pANewDataBlockArrived = (LPANewDataBlockArrived)GetProcAddress( m_hDLL, "ANewDataBlockArrived" );
		m_pShowConfigBit = (LPShowConfigBit)GetProcAddress( m_hDLL, "ShowConfigBit" );
		m_pRemoveConfigBit = (LPRemoveConfigBit)GetProcAddress( m_hDLL, "RemoveConfigBit" );
		m_pGetExtensionYouCreate = (LPGetExtensionYouCreate)GetProcAddress( m_hDLL, "GetExtensionYouCreate" );
		m_pSetSettings = (LPSetSettings)GetProcAddress( m_hDLL, "SetSettings" );
		m_pSetIDTagElement = (LPSetIDTagElement)GetProcAddress( m_hDLL, "SetIDTagElement" );

		if (	NULL == m_pCreateNewCompressionObject ||
				NULL == m_pDeleteCompressionObject ||
				NULL == m_pANewDataBlockArrived || 
				NULL == m_pGetExtensionYouCreate || 
				NULL == m_pSetSettings || 
				NULL == m_pSetIDTagElement )
		{
			bReturn = CDEX_ERROR;
			ASSERT( FALSE );
	    }
		else
		{
			m_bAvailable = TRUE;
		}
    } catch (...)
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
    }
#endif

	return bReturn;
}

CDEX_ERR CEncoderWMA8Dll::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	bool bStereo = LOWORD( GetMode() == 1 );

	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoderWMA8Dll::OpenStream( %s, %d %d" ),
				strOutFileName + CUString( _W( "." ) ) + GetExtension(),
				dwSampleRate,
				nChannels );

	// setup number of input channels
	m_nInputChannels = nChannels;

	m_strStreamFileName = strOutFileName;


	// mixer setup
	if ( ( false == bStereo ) && ( 2 == nChannels ) )
	{
		m_bDownMixToMono  = TRUE;
	}
	else
	{
		m_bDownMixToMono = FALSE;
	}

	if (  ( true == bStereo ) && ( 1 == nChannels ) )
	{
		m_bUpMixToStereo = TRUE;
	}
	else
	{
		m_bUpMixToStereo = FALSE;
	}

#ifndef WMA8
	if ( ! m_Encoder.Init(strOutFileName+ _T( "." ) + GetExtension(),m_nBitrate*1000,dwSampleRate,nChannels,m_dwInBufferSize) == S_OK )
	{
		bReturn = CDEX_ERROR;
	}

	m_Encoder.SetDRM( m_bCopyRight );

#else
	WAVEFORMATEX WFXInFormat;
	ENCompressionOpenResult errCode = COROk;

    WFXInFormat.wFormatTag = WAVE_FORMAT_PCM;
	WFXInFormat.nChannels = bStereo ? 2 : 1;
    WFXInFormat.nSamplesPerSec = dwSampleRate;
    WFXInFormat.nAvgBytesPerSec = dwSampleRate * sizeof( SHORT) * WFXInFormat.nChannels;
    WFXInFormat.nBlockAlign = 4;
    WFXInFormat.wBitsPerSample = 8 * sizeof( SHORT );
    WFXInFormat.cbSize = 0;


    DWORD stereo = 0;
	DWORD wmasecure = 0;

	if ( false == bStereo  )
	{
		stereo = 0;
	}
	else
	{
		stereo = 0xFFFFFFFF;
	}

	if ( m_bCopyRight )
	{
		wmasecure = 0xFFFFFFFF;
	}
	else
	{
		wmasecure = 0;
	}


	m_pSetSettings( m_nBitrate, 
					stereo, 
					dwSampleRate,
					wmasecure );

    CUStringConvert strCnv;

    m_pCODEC = m_pCreateNewCompressionObject(	strCnv.ToUTF8( CUString( strOutFileName + CUString( _W( ".wma") )  ) ),
												errCode,
												&WFXInFormat );
	
	switch ( errCode )
	{
		case CORFileError:
			LTRACE( _T( "The output file %s could not be opened." ) ,strOutFileName);
            bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;

		case CORMemoryError:
			LTRACE( _T( "A memory error occured." ) );
            bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;

		case CORCodecError:
			LTRACE( _T( "The CODEC required to compress %s could not be opened.\n(Probably some problem with the pSetSettings call)"), strOutFileName );
			ASSERT( FALSE );
            bReturn = CDEX_ERROR;
		break;
    }
 
#endif
	if ( CDEX_OK == bReturn )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	EXIT_TRACE( _T( "CEncoderWMA8Dll::OpenStream, result %d" ), bReturn );

	return bReturn;
}

CDEX_ERR CEncoderWMA8Dll::EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples)
{
	CDEX_ERR bReturn = CDEX_OK;

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	// Sanity check
	ASSERT( pbsInSamples );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

#ifndef WMA8
	if ( m_Encoder.ConvertChunk(	(BYTE*)m_pFifoBuffers[ m_nFifoRead ].psData,
									m_pFifoBuffers[ m_nFifoRead ].dwSamples*2 ) != S_OK )
	{
		bReturn = CDEX_ERROR;
		ASSERT( FALSE );
	}

#else
	ENCompressBlockResult cResult;

	m_pANewDataBlockArrived (	m_pCODEC,
								(CHAR*)m_psInputStream,
								dwNumSamples * sizeof( SHORT ),
								FALSE,
								cResult );
	switch ( cResult )
	{
		case COBFileError:
			LTRACE( _T( "Unable to write to %s, disk full?" ) );
			bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;
		
		case COBMemoryError:
			LTRACE( _T( "A memory error occured." ) );
			bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;
		
		case COBCodecError:
            LTRACE( _T( "CODEC compression error." ) );
			bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;
	}

#endif


	return bReturn;
}

CDEX_ERR CEncoderWMA8Dll::CloseStream()
{
	CDEX_ERR bReturn = CDEX_OK;

#ifndef WMA8
	if (m_Encoder.Cleanup()==S_OK)
	{
		bReturn = CDEX_ERROR;
		ASSERT( FALSE );
	}
#else
	ENCompressBlockResult cResult;

	m_pANewDataBlockArrived (	m_pCODEC,
								NULL,
								0,
								TRUE,
								cResult );
	switch ( cResult )
	{
		case COBFileError:
			LTRACE( _T( "Unable to write to %s, disk full?" ) );
			bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;
		
		case COBMemoryError:
			LTRACE( _T( "A memory error occured." ) );
			bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;
		
		case COBCodecError:
            LTRACE( _T( "CODEC compression error." ) );
			bReturn = CDEX_ERROR;
			ASSERT( FALSE );
		break;
	}

	m_pDeleteCompressionObject( m_pCODEC );

	if ( m_pTask && m_pTask->GetValidTagData() )
	{
        CTagData& tagData( m_pTask->GetTagData() );

		CUString strFName = m_strStreamFileName + CUString( _W( "." ) ) + GetExtension();
		CUString strArtist = tagData.GetArtist();
		CUString strAlbum = tagData.GetAlbum();
        CUString strTitle = tagData.GetTitle();
		CUString strComment = tagData.GetComment();
        CUString strEncodedBy = tagData.GetEncodedBy();;
        CUString	strDateTime = tagData.GetYear();;
        CUString	strGenre = tagData.GetGenre();
        CUString	strTrack;

		CUString strMCDI = tagData.GetMCDI( );
		strTrack.Format( _W( "%d" ), (int)tagData.GetTrackNumber() + tagData.GetTrackOffset() - 1 );

        CUStringConvert strCnv1;
        LPSTR lpszFileName = strCnv1.ToUTF8( strFName );
        CUStringConvert strCnv;
        
        m_pSetIDTagElement( lpszFileName, "ARTIST", strCnv.ToACP( strArtist ) );
        m_pSetIDTagElement( lpszFileName, "TITLE", strCnv.ToACP( strTitle ) );
		m_pSetIDTagElement( lpszFileName, "ALBUM", strCnv.ToACP( strAlbum ) );
		m_pSetIDTagElement( lpszFileName, "Comment", strCnv.ToACP( strComment ) );
		m_pSetIDTagElement( lpszFileName, "YEAR", strCnv.ToACP( strDateTime ) );
		m_pSetIDTagElement( lpszFileName, "GENRE", strCnv.ToACP( strGenre ));
		m_pSetIDTagElement( lpszFileName, "TRACK", strCnv.ToACP( strTrack ) );
		m_pSetIDTagElement( lpszFileName, "WM/AlbumCoverURL", "http://www.albumcoverurl.com" );
		m_pSetIDTagElement( lpszFileName, "ENCODEDBY", strCnv.ToACP( strEncodedBy ) );
 		m_pSetIDTagElement( lpszFileName, "WM/MCDI", strCnv.ToACP( strMCDI ) );
	}

#endif
	(void)CEncoder::CloseStream();

	return bReturn;
}

CDEX_ERR CEncoderWMA8Dll::DeInitEncoder()
{
	return CDEX_OK;
}

