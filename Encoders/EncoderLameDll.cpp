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
#include "EncoderLameDll.h"
#include "EncoderLameDllDlg.h"
#include "Encode.h"
#include "config.h"
#include "TaskInfo.h"
#include "ID3Tag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderLameDll" ) );

CEncoderLameDll::CEncoderLameDll()
	: CDLLEncoder()
{
	m_strEncoderPath = _T( "lame_enc.dll" );
	m_strExtension= _T( "mp3" );
	m_strEncoderID= _T( "Lame MP3 Encoder " );
	m_nEncoderID = ENCODER_INT;
	SetCRC( 206592 );
	SetMaxBitrate( 320 );
	SetMinBitrate( 32 );
	LoadSettings();

	GetDLLVersionInfo();

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = TRUE;
}

CEncoderLameDll::~CEncoderLameDll()
{
}

CDEX_ERR CEncoderLameDll::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR err;
	err = CDLLEncoder::InitEncoder( pTask );

	return err;
}

CDEX_ERR CEncoderLameDll::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	CDEX_ERR bReturn = CDEX_OK;

	CUString strLang;
    CUStringConvert strCnv;

	ENTRY_TRACE( _T( "CEncoderLameDll::OpenStream( %s, %ld, %d" ),
				strCnv.ToT( strOutFileName + _W( "." ) + GetExtension() ),
				dwSampleRate,
				nChannels );


	DWORD dwInSampleRate = dwSampleRate;

	m_strStreamFileName = strOutFileName;

	// clear all config settings
	memset( &m_beConfig, 0, sizeof(m_beConfig) );

	// set the number of input channels
	m_nInputChannels = nChannels;

	// Divide sample rate by two for MPEG2
	if ( ( GetVersion() >=1 ) && ( dwInSampleRate >= 32000 ) )
	{
		m_dResampleRatio = 2.0;
		dwSampleRate = dwInSampleRate / 2;
	}

	// mask mode, just to be sure (due to an old hack)
	m_nMode &= 0x0F;

	// Do we have to downmix/upmix ?
	if ( BE_MP3_MODE_MONO == m_nMode )
	{
		if ( 2 == nChannels )
		{
			m_bDownMixToMono = TRUE;
		}
		else
		{
			m_bDownMixToMono = FALSE;
		}
	}
	else 
	{
		if ( 1 == nChannels )
		{
			m_bUpMixToStereo = TRUE;
		}
		else
			m_bUpMixToStereo=FALSE;
	}

	int	nCRC		= m_bCRC;
	int nVBR		= (nCRC>>12)&0x0F;
	int nVbrMethod	= (nCRC>>16)&0x0F;

	m_beConfig.dwConfig						= BE_CONFIG_LAME;
	m_beConfig.format.LHV1.dwStructVersion	= 1;
	m_beConfig.format.LHV1.dwStructSize		= sizeof(m_beConfig);
	m_beConfig.format.LHV1.dwSampleRate		= dwSampleRate;
	m_beConfig.format.LHV1.dwReSampleRate	= m_nOutSampleRate;
	m_beConfig.format.LHV1.nMode			= m_nMode; 
	m_beConfig.format.LHV1.dwBitrate		= m_nBitrate;
	m_beConfig.format.LHV1.dwMaxBitrate		= m_nMaxBitrate;
	m_beConfig.format.LHV1.dwMpegVersion	= (dwSampleRate>=32000)?MPEG1:MPEG2;
	m_beConfig.format.LHV1.dwPsyModel		= 0;
	m_beConfig.format.LHV1.dwEmphasis		= 0;
	m_beConfig.format.LHV1.bCRC				= nCRC&0x01;;
	m_beConfig.format.LHV1.bCopyright		= m_bCopyRight;
	m_beConfig.format.LHV1.bPrivate			= m_bPrivate;
	m_beConfig.format.LHV1.bOriginal		= m_bOriginal;

	// allways write the VBR header, even for CBR file
	m_beConfig.format.LHV1.bWriteVBRHeader	= TRUE;

	if ( nVbrMethod > 0 )
	{
		m_beConfig.format.LHV1.bEnableVBR		= TRUE;
		m_beConfig.format.LHV1.nVbrMethod		= (VBRMETHOD)(nVbrMethod-1) ;

		// is this the ABR method ?
		if ( ( VBR_METHOD_ABR + 1 ) == nVbrMethod )
		{
			// get the average bit rate
			m_beConfig.format.LHV1.dwVbrAbr_bps= LOWORD( GetUserN1() )*1000;
			m_beConfig.format.LHV1.nVBRQuality = 0 ;
		}
		else
		{
			// no ABR selected
			m_beConfig.format.LHV1.dwVbrAbr_bps = 0;
			m_beConfig.format.LHV1.nVBRQuality = nVBR ;
		}
	}

	// Get Quality from third nibble
	m_beConfig.format.LHV1.nPreset=( (nCRC >> 8 ) & 0x0F );

	m_fpOut= CID3Tag::SaveInitialV2Tag( strOutFileName + _W( "." ) + GetExtension(), GetId3V2PadSize() );
    //m_fpOut = CDexOpenFile( strOutFileName + _W( "." ) + GetExtension(), _W( "wb+" ) );

	if ( NULL == m_fpOut )
	{
		CUString strErr;

		strLang = g_language.GetString( IDS_ENCODER_ERROR_COULDNOTOPENFILE );

		strErr.Format( strLang, (LPCWSTR)CUString( strOutFileName + _W( "." ) + GetExtension() ) );

        CDexMessageBox( strErr );

		bReturn  = CDEX_FILEOPEN_ERROR;
	}


	if ( CDEX_OK == bReturn )
	{
		int nReturn = m_InitStream(	&m_beConfig,
									&m_dwInBufferSize,
									&m_dwOutBufferSize,
									&m_hbeStream );

		switch ( nReturn )
		{
			case CDEX_OK:
				// do nothing
			break;
			case -1:
				strLang = g_language.GetString( IDS_ENCODER_ERROR_SAMPLERATEBITRATEMISMATCH );
				CDexMessageBox( strLang );
				bReturn = CDEX_ERROR;
			break;
			case -2:
				strLang = g_language.GetString( IDS_ENCODER_ERROR_INVALIDINPUTSTREAM );
				CDexMessageBox( strLang );
				bReturn = CDEX_ERROR;
			break;
			default:
				strLang = g_language.GetString( IDS_ENCODER_ERROR_INVALIDINPUTSTREAM );
				CDexMessageBox( strLang );
				bReturn = CDEX_ERROR;
			break;

		}
	}

	if ( CDEX_OK == bReturn )
	{

		if ( ( GetVersion() >= 1 ) && ( dwInSampleRate >= 32000 ) )
		{
			// Add multiplcation factor for resampling
			m_dwInBufferSize *= 2;
		}

		if ( m_bDownMixToMono )
		{
			m_dwInBufferSize *= 2;
		}


		// Allocate Output Buffer size
		m_pbOutputStream = new BYTE[ m_dwOutBufferSize ];

		// Only expect halve the number of samples
		// in case we have to upsample
		if ( m_bUpMixToStereo )
		{
			// Allocate upsample Buffer size
			m_psInputStream = new SHORT[ m_dwInBufferSize ];

			m_dwInBufferSize /= 2;

		}
	}

	if ( CDEX_OK == bReturn ) 
	{
		// Initialize the input stream
		bReturn = InitInputStream();
	}


	EXIT_TRACE( _T( "CEncoderLameDll::OpenStream, return value %d" ), bReturn );

	return bReturn;
}


CEncoderDlg* CEncoderLameDll::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg = new CEncoderLameDllDlg;
	}
	return m_pSettingsDlg;
}
