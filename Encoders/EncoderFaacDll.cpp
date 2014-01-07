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
#include "EncoderFaacDll.h"
#include "EncoderFaacDllDlg.h"
#include "Encode.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderFaacDll" ) );

//#pragma comment(linker, "/delayload:libfaac.dll")



// CONSTRUCTOR
CEncoderFaacDll::CEncoderFaacDll() 
	: CEncoder(),
	m_handle( NULL ),
	m_hDLL( NULL ),
	m_pConfig( NULL )
{
	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding = TRUE;

	// All DLL derivates support chunk support
	m_bChunkSupport = TRUE;
	m_bRiffWavSupport = FALSE;
	m_dResampleRatio = 1.0;
	m_strEncoderPath = _T( "libfaac.dll" );
	m_strExtension = _T( "aac" );
	m_strEncoderID = _T( "FAAC DLL Encoder " );
	m_nEncoderID = ENCODER_FAAC;

	LoadSettings();

	// Get encoder ID to get proper version info
	GetDLLVersionInfo();
}

// DESTRUCTOR
CEncoderFaacDll::~CEncoderFaacDll()
{
	if ( m_hDLL )
	{
		ASSERT( FALSE );
		FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}
	if ( m_fpOut )
	{
		ASSERT( FALSE );
		fclose( m_fpOut );
		m_fpOut = NULL;
	}
}


CEncoderDlg* CEncoderFaacDll::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg = new CEncoderFaacDllDlg;
	}
	return m_pSettingsDlg;
}


void CEncoderFaacDll::GetDLLVersionInfo()
{
	CUString strVersion;

	m_bAvailable = FALSE;

	if ( LoadDLL(	m_strEncoderPath,
					_W( "\\encoders\\libraries\\libfaac\\Dll" ),
					m_hDLL,
					FALSE,
					FALSE ) )
	{
		m_bAvailable = TRUE;

		char *faac_id_string = NULL;
		char *faac_copyright_string = NULL;

		int result = faacEncGetVersion( &faac_id_string, &faac_copyright_string );

		strVersion = _T("");

		if ( NULL != faac_id_string ) 
		{
			for ( DWORD charIndex = 0; charIndex < strlen( faac_id_string ); charIndex++ ) {
				strVersion += faac_id_string[ charIndex ];
			}

		}

		// Free the library
		FreeLibrary( m_hDLL );
	}
	else
	{
		strVersion = g_language.GetString( IDS_ENCODER_VERSIONNOTAVAILABLE );
	}

	m_hDLL=NULL;

	m_strEncoderID += strVersion;
}




CDEX_ERR CEncoderFaacDll::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR bReturn = CDEX_OK;

	CUString	strLang;

	ENTRY_TRACE( _T( "CEncoderFaacDll::InitEncoder" ) );

	CEncoder::InitEncoder( pTask );

	if ( !LoadDLL(	m_strEncoderPath,
					_W( "\\encoders\\libraries\\libfaac\\Dll" ),
					m_hDLL,
					FALSE,
					FALSE ) )
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CEncoderFaacDll::InitEncoder, return value %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderFaacDll::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	CDEX_ERR	bReturn		= CDEX_OK;
	bool		bStereo		= true;
	CUString		strLang;

    CUStringConvert strCnv;

	ENTRY_TRACE( _T( "CEncoderFaacDll::OpenStream( %s, %d %d" ),
                strCnv.ToT( strOutFileName + _W( "." ) + GetExtension() ),
				dwSampleRate,
				nChannels );


	// setup number of input channels
	m_nInputChannels = nChannels;

	// setup number of output channels
	if ( ( m_nMode & 0xFFFF ) == BE_MP3_MODE_MONO )
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

		
	// try to get a handle
	m_handle = faacEncOpen( dwSampleRate,
							(bStereo == true )? 2 : 1,
							&m_dwInBufferSize,
							&m_dwOutBufferSize );


	if ( NULL == m_handle )
	{
		CUString strMsg;

		strLang = g_language.GetString( IDS_LOADLIBRARY_FAILED );

		strMsg.Format(	strLang , (LPCWSTR)CUString( ( g_config.GetAppPath() + _W( "\\" ) + m_strEncoderPath ) ) );

		CDexMessageBox( strMsg );
		
		LTRACE( _T( "CEncoderFaacDll::OpenStream Failed to get handle" ) );

		bReturn = CDEX_ERROR;
	}

	if ( CDEX_OK == bReturn )
	{
		// get current config
		m_pConfig = faacEncGetCurrentConfiguration( m_handle );

		// set settings
		if ( GetUserN1() & 0x08 )
		{
			m_pConfig->mpegVersion = FAAC_MPEG4;
		}
		else
		{
			m_pConfig->mpegVersion = FAAC_MPEG2;
		}

		m_pConfig->bitRate = m_nBitrate * 1000;

		m_pConfig->allowMidside = ( GetUserN1() & 0x02 ) ? TRUE : FALSE;

		m_pConfig->useLfe = ( GetUserN1() & 0x04 ) ? TRUE : FALSE;; /* ? */
		m_pConfig->useTns = ( GetUserN1() & 0x01 ) ? TRUE : FALSE;
		m_pConfig->inputFormat = FAAC_INPUT_16BIT;
		m_pConfig->aacObjectType = ( GetUserN2() & 0x07 );
		m_pConfig->shortctl = SHORTCTL_NORMAL;

		switch ( ( GetUserN1() >> 8 ) & 0x0F )
		{
			case 0: m_pConfig->bandWidth = 16000; break;
			case 1: m_pConfig->bandWidth = 18000; break;
			case 2: m_pConfig->bandWidth = 19500; break;
			default: m_pConfig->bandWidth = 19500; break;
		}

		// set new config
		if ( FALSE == faacEncSetConfiguration( m_handle, m_pConfig ) )
		{
			CUString strMsg;

			strLang = g_language.GetString( IDS_LOADLIBRARY_FAILED );

			strMsg.Format(	strLang , (LPCWSTR)CUString( g_config.GetAppPath() + _W( "\\" ) + m_strEncoderPath ) );

			CDexMessageBox( strMsg );
			
			LTRACE( _T( "CEncoderFaacDll::OpenStream Failed to set config" ) );

			bReturn = CDEX_ERROR;
		}

	}

   if ( CDEX_OK == bReturn )
   {
		// Open output file
		m_fpOut = CDexOpenFile(	strOutFileName + _W( "." ) + GetExtension(), _W( "wb+" ) );

		// Check if output file has been opened correctly
		if ( NULL == m_fpOut )
		{
			CUString strMsg;

			strMsg = g_language.GetString( IDS_ENCODER_ERROR_INVALIDINPUTSTREAM );

			CDexMessageBox( strMsg );

			bReturn = CDEX_FILEOPEN_ERROR;
		}
   }


	if ( CDEX_OK == bReturn )
	{
		// Allocate Output Buffer size
		m_pbOutputStream = new BYTE[ m_dwOutBufferSize ];

		if ( NULL == m_pbOutputStream )
		{
			bReturn = CDEX_OUTOFMEMORY;
		}
	}

	if ( CDEX_OK == bReturn )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	EXIT_TRACE( _T( "CEncoderFaacDll::OpenStream, return value %d"), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderFaacDll::CloseStream()
{
	CDEX_ERR bReturn = CDEX_OK;

	int nBytesToWrite = 0;

	ENTRY_TRACE( _T( "CEncoderFaacDll::CloseStream" ) );

	// finish encoding and write the last aac frames
	nBytesToWrite = faacEncEncode(	m_handle,
									NULL,
									0,
									m_pbOutputStream,
									m_dwOutBufferSize );

	if ( nBytesToWrite < 0 )
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
	}

	// Did we receive some output bytes
	if ( nBytesToWrite > 0 )
	{
		// Guess so, write it to the output file
		if ( 1 != fwrite( m_pbOutputStream,nBytesToWrite, 1, m_fpOut ) )
		{
			ASSERT( FALSE );
			return CDEX_FILEWRITE_ERROR;
		}
	}

	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CEncoderFaacDll::CloseStream, return value %d"), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderFaacDll::DeInitEncoder()
{
	// Release DLL if necessary
	if ( NULL != m_hDLL)
	{
		FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}

	(void)CEncoder::DeInitEncoder();

	return CDEX_OK;
}



CDEX_ERR CEncoderFaacDll::EncodeChunk( PSHORT pbsInSamples, DWORD dwNumSamples )
{
	CDEX_ERR bReturn = CDEX_OK;

	int nBytesToWrite = 0;

	LTRACE2( _T( "Entering CEncoderFaacDll::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	// Sanity check
	ASSERT( pbsInSamples );

	// process incoming data
	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );


	// feed sample to the encoder library
	nBytesToWrite = faacEncEncode(	m_handle,
									(INT*)pbsInSamples,
									dwNumSamples,
									m_pbOutputStream,
									m_dwOutBufferSize );

	LTRACE2( _T( "CEncoderFaacDll::EncodeChunk() Fed Encoder %d samples, returned %d bytes" ),
					dwNumSamples,
					nBytesToWrite );


	if ( nBytesToWrite < 0 )
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
	}

	// Did we receive some output bytes
	if ( nBytesToWrite > 0 )
	{
		// Guess so, write it to the output file
		if ( 1 != fwrite( m_pbOutputStream,nBytesToWrite, 1, m_fpOut ) )
		{
			ASSERT( FALSE );
			return CDEX_FILEWRITE_ERROR;
		}
	}

	LTRACE2( _T( "Leaving CEncoderFaacDll::EncodeChunk(), return status %d" ),
					bReturn );
	return bReturn;
}


