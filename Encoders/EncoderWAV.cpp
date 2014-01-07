/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006, 2007, 2008 Georgy Berdyshev
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
#include "EncoderWAV.h"
#include "EncoderWAVDlg.h"
#include "Encode.h"
#include "config.h"
#include "TaskInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderWAV" ) );


// CEncoderWAV CONSTRUCTOR
CEncoderWAV::CEncoderWAV()
	:CEncoder()
{
	// DLL encoders do support ChunkEncoding
	m_strEncoderID = _T( "WAV Output Encoder" );
	m_bOnTheFlyEncoding = TRUE;
	m_strExtension = _T( "wav" );
	m_strEncoderPath = _T( "" );
	m_strUser1 = _T( "" );
	m_strUser2 = _T( "" );
	// All Codec derivates support chunk support
	m_bChunkSupport = TRUE;
	m_bRiffWavSupport = FALSE;
	m_pSndFile = NULL;
	m_nEncoderID = ENCODER_WAV;
	LoadSettings();
}


// CEncoderWAV DESTRUCTOR
CEncoderWAV::~CEncoderWAV()
{
	DeInitEncoder();
	CloseStream();
}


CDEX_ERR CEncoderWAV::InitEncoder( CTaskInfo* pTask )
{
	return CEncoder::InitEncoder( pTask );
}


CDEX_ERR CEncoderWAV::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	int nFormat=0;
	int	nCompression=0;
	int nTest=-1;

	ENTRY_TRACE( _T( "CEncoderWAV::OpenStream" ) );

	memset( &m_wfInfo, 0, sizeof( SF_INFO ) );

	// Retain number of input channels
	m_nInputChannels = nChannels;


	m_dResampleRatio = dwSampleRate / m_nSampleRate;

	m_wfInfo.samplerate  = m_nSampleRate;
	m_wfInfo.frames      = -1;
	m_wfInfo.sections    = 1;
	m_wfInfo.channels    = ( GetMode() == 1 )?1:2;

	switch ( GetUserN1() )
	{
		case 0: nCompression = SF_FORMAT_PCM_16;	break;
		case 1: nCompression = SF_FORMAT_ULAW;		break;
		case 2: nCompression = SF_FORMAT_ALAW;		break;
		case 3: nCompression = SF_FORMAT_IMA_ADPCM;	break;
		case 4: nCompression = SF_FORMAT_MS_ADPCM;	break;
		default:
			ASSERT( FALSE );
	}

	switch ( GetUserN2() )
	{
		case 0:
			m_strExtension = _T( "wav" );
			nFormat=SF_FORMAT_WAV;
		break;

		case 1:
			m_strExtension = _T( "aiff" );
			nFormat=SF_FORMAT_AIFF; 
		break;

		case 2:
			m_strExtension = _T( "au" );
			nFormat=SF_FORMAT_AU | SF_ENDIAN_BIG;
		break;
		case 3:
			m_strExtension = _T( "au" );
			nFormat = SF_FORMAT_AU | SF_ENDIAN_LITTLE;
		break;
		case 4:
			nCompression=0;
			m_strExtension = _T( "raw" );
			nFormat = SF_FORMAT_RAW | SF_ENDIAN_BIG;
		break;
		case 5:
			nCompression=0;
			m_strExtension = _T( "raw" );
			nFormat = SF_FORMAT_RAW | SF_ENDIAN_LITTLE;
		break;

		default:
			ASSERT( FALSE );
	}
	m_wfInfo.format= (nFormat|nCompression);

	// Open stream
    CUStringConvert strCnv;
    #ifdef _UNICODE
    if ( !( m_pSndFile = sf_open((const tchar*)strCnv.ToT( strOutFileName+ _W( "." ) + GetExtension() ),
									SFM_WRITE,
									&m_wfInfo ) ) )
    #else
	if ( !( m_pSndFile = sf_open(	strCnv.ToT( strOutFileName+ _W( "." ) + GetExtension() ),
									SFM_WRITE,
									&m_wfInfo ) ) )
    #endif
	{
		ASSERT( FALSE );
		return FALSE;
	}

	// Set converter size TODO: Remove hardcoded value
	// INPUT BUFFER SIZE IN SAMPLES, THUS SHORTS
	m_dwInBufferSize = 8192;

	if ( !IsOutputStereo() && IsInputStereo() )
	{
		m_bDownMixToMono=TRUE;
	}
	else
	{
		m_bDownMixToMono=FALSE;
	}

	if ( IsOutputStereo()  && !IsInputStereo() )
	{
		m_bUpMixToStereo = TRUE;
	}
	else
	{
		m_bUpMixToStereo = FALSE;
	}

	if ( m_bDownMixToMono )
	{
		m_dwInBufferSize *= 2;
	}


	// Allocate Output Buffer size
	m_pbOutputStream= new BYTE[ m_dwInBufferSize ];


	// Only expect half the number of samples
	// in case we have to upsample
	InitInputStream();

	EXIT_TRACE( _T( "CEncoderWAV::OpenStream" ) );

	// Everything went OK
	return CDEX_OK;
}


CDEX_ERR CEncoderWAV::CloseStream()
{
	if ( m_pSndFile )
	{
		if ( sf_close( m_pSndFile ) !=0 )
		{
			ASSERT( FALSE );
			return FALSE;
		}
		m_pSndFile = NULL;
	}

	// No Errors
	return CDEX_OK;
}


CDEX_ERR CEncoderWAV::DeInitEncoder()
{
	return CDEX_OK;
}


CDEX_ERR CEncoderWAV::EncodeChunk( PSHORT pbsInSamples,DWORD dwNumSamples )
{
	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	// Sanity check
	ASSERT( pbsInSamples );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

	// Write chunk to disk
	if ( sf_write_short(	m_pSndFile,
							m_psInputStream,
							dwNumSamples )
							!= dwNumSamples )
	{
		return CDEX_ERROR;
	}

	return CDEX_OK;
}


CEncoderDlg* CEncoderWAV::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg = new CEncoderWAVDlg;
	}
	return m_pSettingsDlg;
}


