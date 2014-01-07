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
#include "EncoderWinCodec.h"
#include "TaskInfo.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderWinCodec" ) );

// CONSTRUCTOR
CEncoderWinCodec::CEncoderWinCodec()
	:CEncoder()
{
	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding=TRUE;

	m_strExtension = _T( "mp3" );

	m_strEncoderPath = _T( "" );

	m_strUser1 = _T( "Fraunhofer" );

	m_strUser2= _T( "56 kbts" );

	m_nCodecID = WAVE_FORMAT_MPEGLAYER3;

	// All Codec derivates support chunk support
	m_bChunkSupport = TRUE;

	m_bRiffWavSupport = TRUE;
}


// DESTRUCTOR
CEncoderWinCodec::~CEncoderWinCodec()
{
}


const int nCodecChunkSize = 1152 * 2 * 2 * 2;

CDEX_ERR CEncoderWinCodec::InitEncoder( CTaskInfo* pTask )
{
	// No special initialization
	CEncoder::InitEncoder( pTask );

	return CDEX_OK;
}


CDEX_ERR CEncoderWinCodec::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	CDEX_ERR bReturn = CDEX_OK;

	CUString			strLang;

	ENTRY_TRACE( _T( "CEncoderWinCodec::OpenStream( %s, %d %d" ),
				strOutFileName + _W( "." ) + GetExtension(),
				dwSampleRate,
				nChannels );

	m_nInputChannels = nChannels;

	BOOL bHighQuality = GetUserN1();

	// Initialize encoder
	if ( g_AcmCodec.InitStream( GetUser1(), GetUser2(), m_nCodecID, dwSampleRate, nChannels, bHighQuality ) == CDEX_ERROR )
	{
		bReturn = CDEX_FILEOPEN_ERROR;
	}

	if ( CDEX_OK == bReturn )
	{
		m_fpOut = _wfopen(	strOutFileName + _W( "." ) + GetExtension(), _W( "wb+" ) );

		if ( NULL == m_fpOut )
		{
			CUString strErr;

			strLang = g_language.GetString( IDS_ENCODER_ERROR_COULDNOTOPENFILE );

			strErr.Format( strLang, (LPCWSTR)CUString( strOutFileName + _W( "." ) + GetExtension() ) );

            CDexMessageBox( strErr );

			bReturn = CDEX_ERROR;
		}
	}

	if ( CDEX_OK == bReturn )
	{
		// Allocate Output Buffer size
		m_pbOutputStream = new BYTE[ nCodecChunkSize ];

		if ( NULL == m_pbOutputStream )
		{
			bReturn = CDEX_OUTOFMEMORY;
		}

		// Set converter size TODO: Remove hardcoded value
		// INPUT BUFFER SIZE IN SAMPLES, THUS SHORTS
		m_dwInBufferSize= nCodecChunkSize/2;
		//	m_dwInBufferSize=2*1024;
	}

	if ( CDEX_OK == bReturn )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	EXIT_TRACE( _T( "CEncoderWMA8Dll::OpenStream, result %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderWinCodec::CloseStream()
{
	CDEX_ERR bReturn;
	DWORD nBytesToWrite = 0;

	ENTRY_TRACE( _T( "CEncoderWinCodec::CloseStream" ) );

	bReturn = g_AcmCodec.DeInitStream(m_pbOutputStream, &nBytesToWrite);

	if ( CDEX_ERROR == bReturn )
	{
		ASSERT(FALSE);
	}
	else
	{

		// Did we receive some output bytes
		if (nBytesToWrite>0)
		{
			// Guess so, write it to the output file
			if ( 1 != fwrite(m_pbOutputStream,nBytesToWrite,1,m_fpOut) )
			{
				ASSERT(FALSE);
				bReturn = CDEX_FILEWRITE_ERROR;
			}
		}
	}

	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CEncoderWinCodec::CloseStream, return value %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderWinCodec::DeInitEncoder()
{
	(void)CEncoder::DeInitEncoder();

	return CDEX_OK;
}



CDEX_ERR CEncoderWinCodec::EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples)
{
	CDEX_ERR bReturn = CDEX_OK;

	DWORD nBytesToWrite = 0;

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	// Sanity check
	ASSERT( pbsInSamples );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

	if (	g_AcmCodec.EncodeChunk(	dwNumSamples,
									m_psInputStream,
									m_pbOutputStream,
									&nBytesToWrite ) == CDEX_ERROR )
	{
		ASSERT( FALSE );
		bReturn = CDEX_ERROR;
	}

	// Did we receive some output bytes
	if ( nBytesToWrite > 0 )
	{
		// Guess so, write it to the output file
		if ( 1 != fwrite( m_pbOutputStream, nBytesToWrite, 1, m_fpOut ) )
		{
			ASSERT( FALSE );
			bReturn = CDEX_FILEWRITE_ERROR;
		}
	}

	return bReturn;
}


