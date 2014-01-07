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
#include "EncoderWAVFixed.h"
#include "Encode.h"
#include "config.h"
#include "EncoderWavDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderWAVFixed" ) );

// CWAVEncoder CONSTRUCTOR
CEncoderWAVFixed::CEncoderWAVFixed()
	:CEncoderWAV()
{
}


// CEncoderWAVFixed DESTRUCTOR
CEncoderWAVFixed::~CEncoderWAVFixed()
{
}



CDEX_ERR CEncoderWAVFixed::OpenStream( CUString strOutFileName,DWORD dwSampleRate,WORD nChannels )
{
	int nFormat=0;
	int	nCompression=0;
	int nTest=-1;

	ENTRY_TRACE( _T( "CEncoderWAVFixed::OpenStream" ) );

//	pTest=fopen("f:\\cdextest\\test.raw","wb+");

	memset(&m_wfInfo,0,sizeof(SF_INFO));

	// Retain number of input channels
	m_nInputChannels=nChannels;


	m_dResampleRatio = 1;

	m_wfInfo.samplerate  = dwSampleRate;
	m_wfInfo.frames      = -1;
	m_wfInfo.sections	 = 1;
	m_wfInfo.channels    = nChannels;

	nCompression = SF_FORMAT_PCM_16;

	m_strExtension = _T( "wav" );

	nFormat = SF_FORMAT_WAV;

	m_wfInfo.format = ( nFormat | nCompression );

	// Open stream
    CUStringConvert strCnv;
    #ifdef _UNICODE
    if ( !( m_pSndFile = sf_open(	(const tchar*)strCnv.ToT( strOutFileName +  _W( "." ) + GetExtension() ), 
									SFM_WRITE,
									&m_wfInfo ) ) )
    #else
    if ( !( m_pSndFile = sf_open(	strCnv.ToT( strOutFileName +  _W( "." ) + GetExtension() ), 
									SFM_WRITE,
									&m_wfInfo ) ) )
    #endif
	{
		CUString strError( g_language.GetString( IDS_FAILEDTOOPENFILE ) );
		SetLastCDexErrorString( strError + strOutFileName +  _W( "." ) + GetExtension() );
		ASSERT( FALSE );
		return CDEX_ERROR;
	}

	m_bDownMixToMono = FALSE;
	m_bUpMixToStereo = FALSE;


	// Set converter size TODO: Remove hardcoded value
	// INPUT BUFFER SIZE IN SAMPLES, THUS SHORTS
	m_dwInBufferSize = 8192;

	// Allocate Output Buffer size
	m_pbOutputStream= new BYTE[ m_dwInBufferSize ];

	// Initialize input stream
	InitInputStream();

	m_strEncoderID = _T( "PCM WAV file" );

	EXIT_TRACE( _T( "CEncoderWAVFixed::OpenStream" ) );

	// Everything went OK
	return CDEX_OK;
}
