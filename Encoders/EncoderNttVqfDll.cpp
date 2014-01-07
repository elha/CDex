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
#include "EncoderNttVqfDll.h"
#include "EncoderNttVqfDllDlg.h"
#include "Encode.h"
#include "config.h"
#include "utf8.h"
#include "TaskInfo.h"
#include "Chunk.h"
#include "bfile.h"
#include "bstream_e.h"
#include "ChunkHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#pragma comment(linker, "/delayload:tvqenc.dll")

INITTRACE( _T( "EncoderNttVqfDll" ) );

// CONSTRUCTOR
CEncoderNttVqfDll::CEncoderNttVqfDll() : CEncoder()
{

	m_hDLL			= NULL;
	m_pfBuffer		= NULL;

	m_pTwinChunk	= NULL;
	m_pbFile		= NULL;

	memset( &m_setupInfo, 0, sizeof( m_setupInfo ) );
	memset( &m_encInfo, 0, sizeof( m_encInfo ) );

	m_strStreamFileName = _T( "" );

	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding = TRUE;

	m_strEncoderPath= _T( "tvqenc.dll" ) ;
	m_strEncoderID= _T( "NTT VQF (tvqenc.dll) Encoder " );
	m_strExtension= _T( "vqf" );

	m_bChunkSupport = TRUE;

	m_bRiffWavSupport = FALSE;

	m_dResampleRatio = 1.0;

	m_nEncoderID = ENCODER_NTTVQF;

	m_nMode = 1;

	m_nBitrate = 48;

	// get current settings from ini file
	LoadSettings();

	// Get encoder ID to get proper version info
	GetDLLVersionInfo();

	m_bCanWriteTagV1 = FALSE;
	m_bCanWriteTagV2 = FALSE;

}


// DESTRUCTOR
CEncoderNttVqfDll::~CEncoderNttVqfDll()
{
	if ( m_hDLL )
	{
		ASSERT( FALSE );
		::FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}

	if ( m_pbFile )
	{
		ASSERT( FALSE );
		bclose( m_pbFile );
		m_pbFile = NULL;
	}

	delete [] m_pfBuffer;
	m_pfBuffer = NULL;
}

void CEncoderNttVqfDll::GetDLLVersionInfo()
{
	CDEX_ERR bReturn = CDEX_OK;

	CUString strVersion;

	strVersion = g_language.GetString( IDS_ENCODER_VERSIONNOTAVAILABLE );

	ENTRY_TRACE( _T( "CEncoderNttVqfDll::GetDLLVersionInfo" ) );

	m_bAvailable = FALSE;

	if ( !LoadDLL(  m_strEncoderPath,
					_W( "\\encoders\\libraries\\libnttvqf\\Dll" ),
					m_hDLL,
					FALSE,
					FALSE ) )
	{
		bReturn = CDEX_ERROR;
	}


	if ( CDEX_OK == bReturn )
	{
		int nRet = TvqGetVersionID( V2, m_setupInfo.ID );

		if ( 0 == nRet )
		{
			strVersion = CUString( m_setupInfo.ID, CP_UTF8 );

			m_bAvailable = TRUE;
		}
	}

	if ( m_hDLL )
	{
		::FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}

	m_strEncoderID += strVersion;

	EXIT_TRACE( _T( "CEncoderNttVqfDll::GetDLLVersionInfo, status %d" ), bReturn );
}



CDEX_ERR CEncoderNttVqfDll::InitEncoder( CTaskInfo* pTask )
{

	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoderNttVqfDll::InitEncoder" ) );

	CEncoder::InitEncoder( pTask );

	if ( !LoadDLL( m_strEncoderPath, _W( "\\encoders\\libraries\\libnttvqf\\Dll" ), m_hDLL, TRUE, FALSE ) )
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CEncoderNttVqfDll::InitEncoder, result %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderNttVqfDll::OpenStream( CUString strOutFileName, DWORD dwSampleRate, WORD nChannels )
{
	CDEX_ERR	bReturn = CDEX_OK;
	bool		bStereo	= true;
	int			nRet = 0;

	m_strStreamFileName = strOutFileName + _W( "." ) + GetExtension();

    CUStringConvert strCnv;

	ENTRY_TRACE( _T( "CEncoderNttVqfDll::OpenStream( %s, %d %d" ),
				strCnv.ToT( strOutFileName + _W( "." ) + GetExtension() ),
				dwSampleRate,
				nChannels );


	// setup number of input channels
	m_nInputChannels = nChannels;

	// setup number of output channels
	if ( ( LOWORD( m_nMode ) ) == 0 )
	{
		bStereo = false;
	}


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

	// setup number of output channels
	m_setupInfo.channelMode = bStereo ? 1 : 0;

	// setup sampling rate of the output file
	m_setupInfo.samplingRate = dwSampleRate / 1000;

	// setup proper bitrate
	m_setupInfo.bitRate = GetBitrate() * (bStereo ? 2 : 1) ;

	// number of VQ pre-selection candidates
	m_encInfo.N_CAN_GLOBAL = 16;

    CTagData& tagData( m_pTask->GetTagData() );

    strncpy( m_setupInfo.Name, strCnv.ToACP( tagData.GetTitle() ), BUFSIZ );
    strncpy( m_setupInfo.Comt, strCnv.ToACP( tagData.GetComment() ), BUFSIZ );
	strncpy( m_setupInfo.Auth, strCnv.ToACP( tagData.GetArtist() ), BUFSIZ );
	strncpy( m_setupInfo.Cpyr, strCnv.ToACP( tagData.GetAlbum() ), BUFSIZ );
	strncpy( m_setupInfo.File, strCnv.ToACP( strOutFileName ), BUFSIZ );

	// Initialize the encoder
	nRet = TvqEncInitialize(	&m_setupInfo, 
								&m_encInfo,
								&m_index,
								1 );



	if ( 0 != nRet )
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
	}
	else
	{
		int nFrameSize;
		int nChannels;


		if ( NULL == ( m_pbFile = bopen( strCnv.ToACP( m_strStreamFileName ), "wb" ) ) )
		{
			LTRACE( _T( "%s: File open error." ), m_strStreamFileName );
			ASSERT( FALSE );
			bReturn = CDEX_ERROR;
		}
		else
		{
			CChunkChunk* pTwinChunk = NULL;

			TvqInitBsWriter();

		
			// get the number of samples per frame per channel
			nFrameSize = TvqEncGetFrameSize();

			// get the number of encoding channels
			nChannels = TvqEncGetNumChannels();

			m_dwInBufferSize  = nFrameSize * nChannels;

			if ( m_bDownMixToMono )
			{
				m_dwInBufferSize *= 2;
			}

			m_pfBuffer = new float[ m_dwInBufferSize ];

			if ( NULL == m_pfBuffer )
			{
				ASSERT( FALSE );
				bReturn = CDEX_ERROR;
			}

			pTwinChunk = TvqCreateHeaderChunk( &m_setupInfo, m_lpszHeaderInfoDir );

			if ( NULL == pTwinChunk )
			{
				ASSERT( FALSE );
				bReturn = CDEX_ERROR;
			}
			else
			{
				TvqPutBsHeaderInfo( m_pbFile, *pTwinChunk );
				delete pTwinChunk;
			}
		}
	}

	if ( CDEX_OK == bReturn )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	EXIT_TRACE( _T( "CEncoderNttVqfDll::OpenStream, return value %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderNttVqfDll::CloseStream()
{
	ENTRY_TRACE( _T( "CEncoderNttVqfDll::CloseStream" ) );

	/* flush encoder twice */
	EncodeChunk( NULL, 0 );
	EncodeChunk( NULL, 0 );

	TvqEncTerminate( &m_index );

	bclose( m_pbFile );
	m_pbFile = NULL;

	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CEncoderNttVqfDll::CloseStream" ) );

	// No Errors
	return CDEX_OK;

}


CDEX_ERR CEncoderNttVqfDll::DeInitEncoder()
{
	ENTRY_TRACE( _T( "CEncoderNttVqfDll::DeInitEncoder" ) );

	delete [] m_pfBuffer;
	m_pfBuffer = NULL;

	if ( m_hDLL )
	{
		::FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}

	(void)CEncoder::DeInitEncoder();

	EXIT_TRACE( _T( "CEncoderNttVqfDll::DeInitEncoder" ) );

	return CDEX_OK;
}



CDEX_ERR CEncoderNttVqfDll::EncodeChunk( PSHORT pbsInSamples,DWORD dwNumSamples )
{
	CDEX_ERR	bReturn = CDEX_OK;
	PSHORT		pSamples = NULL;
	DWORD		dwSample = 0;
	int			nChannels = TvqEncGetNumChannels();

	LTRACE2( _T( "Entering CEncoderVorbisDll::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

	pSamples = m_psInputStream;


	if ( 2 == nChannels )			
	{
		DWORD dwSamplesPerChannel = dwNumSamples / 2;

		for ( dwSample = 0; dwSample < dwSamplesPerChannel; dwSample ++ )
		{
			m_pfBuffer[ dwSample + 0 ] = pSamples[ 2 * dwSample + 0 ];
			m_pfBuffer[ dwSamplesPerChannel + dwSample ] = pSamples[ 2* dwSample + 1 ];
		}

		// pad with zeros for last frame
		for ( dwSample = dwNumSamples; dwSample < m_dwInBufferSize; dwSample++ )
		{
			m_pfBuffer[ dwSample ] = 0;
		}
	}
	else
	{
		DWORD dwSamplesPerChannel = dwNumSamples;

		for ( dwSample = 0; dwSample < dwSamplesPerChannel; dwSample ++ )
		{
			m_pfBuffer[ dwSample ] = pSamples[ dwSample ];
		}

		// pad with zeros for last frame
		for ( dwSample = dwNumSamples; dwSample < m_dwInBufferSize; dwSample++ )
		{
			m_pfBuffer[ dwSample ] = 0;
		}
	}


	// encode frame
	TvqEncodeFrame( m_pfBuffer, &m_index );

	TvqWriteBsFrame( &m_index, m_pbFile );


	LTRACE2( _T( "Leaving CEncoderNttVqfDll::EncodeChunk(), return status %d" ),
					bReturn );

	return bReturn;
}


CEncoderDlg* CEncoderNttVqfDll::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg = new CEncoderNttVqfDllDlg;
	}
	return m_pSettingsDlg;
}
