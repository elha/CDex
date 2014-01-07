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
#include "EncoderApeDll.h"
#include "Encode.h"
#include "config.h"
#include "TaskInfo.h"
#include "EncoderApeDllDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderApeDll" ) );

//#pragma comment(linker, "/delayload:MACDll.dll" )


// CONSTRUCTOR
CEncoderApeDll::CEncoderApeDll() : CEncoder()
{
	m_hDLL = NULL;
	m_hAPECompress = NULL;

	m_strStreamFileName = _T( "" );

	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding = TRUE;

	m_strEncoderPath = _T( "MACDll.dll" );
	m_strEncoderID = _T( "Monkey's Audio Encoder DLL " );

	m_bChunkSupport = TRUE;

	m_bRiffWavSupport = FALSE;

	m_dResampleRatio = 1.0;

	m_nEncoderID = ENCODER_APE;

	// Default to COMPRESSION_LEVEL_EXTRA_HIGH
	SetUserN1( 3 );

	// Set tag type to APE
	SetUserN2( 0 );

	// Default to stereo
	SetMode( 1);

	m_bWriteTag = FALSE;

	// get current settings from ini file
	LoadSettings();

	// Get encoder ID to get proper version info
	GetDLLVersionInfo();

	m_strExtension = _T( "ape" );

	m_bCanWriteTagV1 = FALSE;
	m_bCanWriteTagV2 = FALSE;

}


// DESTRUCTOR
CEncoderApeDll::~CEncoderApeDll()
{
	if ( m_hDLL )
	{
		ASSERT( FALSE );
		::FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}
	if ( m_fpOut )
	{
		ASSERT( FALSE );
		fclose( m_fpOut );
		m_fpOut = NULL;
	}
}


void CEncoderApeDll::GetDLLVersionInfo()
{
	CUString strVersion;

	m_bAvailable = FALSE;

#ifdef _UNICODE

	if ( LoadDLL(  m_strEncoderPath, _W( "\\encoders\\libraries\\libape" ), m_hDLL, FALSE, FALSE ) )
	{
		CUString strLang;

		// Get version number
		int nVersion = GetVersionNumber();


		strLang = g_language.GetString( IDS_ENCODER_VERSIONSTRING );

		// Create version string
		strVersion.Format( _W( "(%s %4.3f)" ), strLang, nVersion / 1000.0f );

		// Free the library
		::FreeLibrary( m_hDLL );

		// Indicate that DLL has been freed
		m_hDLL = NULL;

		m_bAvailable = TRUE;

	}
	else
#endif
	{
		strVersion = g_language.GetString( IDS_ENCODER_VERSIONNOTAVAILABLE );
	}

	// Add version info the encoder ID
	m_strEncoderID += strVersion;
}



CDEX_ERR CEncoderApeDll::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR	bReturn = CDEX_OK;
	CUString		strLang;

	LTRACE( _T( "CEncoderApeDll::InitEncoder" ) );

	CEncoder::InitEncoder( pTask );

	m_hAPECompress = NULL;

#ifdef _UNICODE

	if ( LoadDLL( m_strEncoderPath, _W( "\\encoders\\libraries\\libape" ), m_hDLL, TRUE, FALSE ) )
	{
		int nRetVal = 0;

		nRetVal = GetInterfaceCompatibility( MAC_VERSION_NUMBER, TRUE, NULL );

		if ( 0 != nRetVal )
		{
			bReturn = CDEX_ERROR;
		}
		else
		{
			// create the encoder interface
			m_hAPECompress = c_APECompress_Create( &nRetVal );

			if ( NULL == m_hAPECompress )
			{
				ASSERT( FALSE );
				bReturn = CDEX_ERROR;
			}
		}
	}
	else
#endif
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CEncoderApeDll::InitEncoder, return value %d"), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderApeDll::OpenStream( CUString strOutFileName, DWORD dwSampleRate, WORD nChannels )
{
	CDEX_ERR		bReturn = CDEX_OK;
	WAVEFORMATEX	wfeAudioFormat = { 0,};
	int				nVBR = ( GetCRC() >> 12 ) & 0x0F;
	int				nRetVal = 0;
	int				nLevel = 0;
	bool			bStereo = true;

	ASSERT( NULL != m_hAPECompress );

    CUStringConvert strCnv;

	ENTRY_TRACE( _T( "CEncoderApeDll::OpenStream( %s, %d %d" ),
                strCnv.ToT( strOutFileName + _W( "." ) + GetExtension() ),
				dwSampleRate,
				nChannels );

	m_dwInBufferSize = 1024;

	// setup number of input channels
	m_nInputChannels = nChannels;

	// setup number of output channels
	if ( 0 == GetMode() )
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


	wfeAudioFormat.cbSize = 0;
	wfeAudioFormat.nSamplesPerSec = dwSampleRate;
	wfeAudioFormat.wBitsPerSample = 16;
	wfeAudioFormat.nChannels = bStereo ? 2:1;
	wfeAudioFormat.wFormatTag = 1;
	wfeAudioFormat.nBlockAlign = (wfeAudioFormat.wBitsPerSample / 8) * wfeAudioFormat.nChannels;
	wfeAudioFormat.nAvgBytesPerSec = wfeAudioFormat.nBlockAlign * wfeAudioFormat.nSamplesPerSec;

	m_strStreamFileName = strOutFileName + _W( "." ) + GetExtension();

	switch ( LOBYTE( LOWORD(  GetUserN1() ) ) )
	{
		case 0: nLevel = COMPRESSION_LEVEL_FAST;		break;
		case 1: nLevel = COMPRESSION_LEVEL_NORMAL;		break;
		case 2: nLevel = COMPRESSION_LEVEL_HIGH;		break;
		case 3: nLevel = COMPRESSION_LEVEL_EXTRA_HIGH;	break;
		default:
			ASSERT( FALSE );
	}

	// start the encoder
	nRetVal = c_APECompress_Start(	m_hAPECompress,
                                    GetDosFileName( m_strStreamFileName ),
									&wfeAudioFormat,
									0x7FFFFFFF,
									nLevel,
									NULL,
									CREATE_WAV_HEADER_ON_DECOMPRESSION );
	if ( 0 != nRetVal )
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
	}

	if ( m_bDownMixToMono )
	{
		m_dwInBufferSize *= 2;
	}

	if ( CDEX_OK == bReturn )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	EXIT_TRACE( _T( "CEncoderApeDll::OpenStream, return value %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderApeDll::EncodeChunk( PSHORT pbsInSamples, DWORD dwNumSamples )
{
	CDEX_ERR bReturn = CDEX_OK;
	DWORD nBytesToAdd = 0;
	int nRetVal = 0;

	LTRACE2( _T( "Entering CEncoderApeDll::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

	nBytesToAdd = dwNumSamples * sizeof( SHORT );

	nRetVal = c_APECompress_AddData( m_hAPECompress, 
									(BYTE*)m_psInputStream,
									nBytesToAdd );

	if ( nRetVal != ERROR_SUCCESS )
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
	}
	

	EXIT_TRACE( _T( "CEncoderApeDll::EncodeChunk(), return status %d" ),
					bReturn );

	return bReturn;

}

CDEX_ERR CEncoderApeDll::CloseStream()
{
	CDEX_ERR bReturn = CDEX_ERROR;

	ENTRY_TRACE( _T( "CEncoderApeDll::CloseStream" ) );

	///////////////////////////////////////////////////////////////////////////////
	// finalize the file (could append a tag, or WAV terminating data)
	///////////////////////////////////////////////////////////////////////////////
	if ( 0 != c_APECompress_Finish( m_hAPECompress, 
									NULL,
									0,
									0 ) )
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
	}
	else
	{
		m_bWriteTag = TRUE;
	}


	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CEncoderApeDll::CloseStream, return value %d" ), bReturn );



	return bReturn;

}


CDEX_ERR CEncoderApeDll::DeInitEncoder()
{
	if ( NULL != m_hAPECompress )
	{
		c_APECompress_Destroy( m_hAPECompress );
		m_hAPECompress = NULL;
	}
	else
	{
		ASSERT( FALSE );
	}

	if ( m_bWriteTag )
	{
		CUString strTrackNumber;

        CTagData& tagData( m_pTask->GetTagData() );

        strTrackNumber.Format( _W( "%d" ), tagData.GetTrackNumber() + tagData.GetTrackOffset() );

		int nRetVal = 0;

		// Add tag
        CUStringConvert strCnv0;
        CUStringConvert strCnv1;
        CUStringConvert strCnv2;
        CUStringConvert strCnv3;
        CUStringConvert strCnv4;
        CUStringConvert strCnv5;
        CUStringConvert strCnv6;

		nRetVal = TagFileSimple(	GetDosFileName( m_strStreamFileName ),
                                    strCnv0.ToUTF8( tagData.GetArtist() ), 
									strCnv1.ToUTF8( tagData.GetAlbum() ), 
                                    strCnv2.ToUTF8( tagData.GetTitle() ), 
                                    strCnv3.ToUTF8( tagData.GetComment() ),
									strCnv4.ToUTF8( tagData.GetGenre() ), 
									strCnv5.ToUTF8( tagData.GetYear() ),
									strCnv6.ToUTF8( strTrackNumber ), 
									TRUE, 
									( GetUserN2() == 1 ) );
	}

	if ( m_hDLL )
	{
		FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}

	return CDEX_OK;
}


CEncoderDlg* CEncoderApeDll::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg = new CEncApeOpts;
	}
	return m_pSettingsDlg;
}


