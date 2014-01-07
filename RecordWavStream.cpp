/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 - 2002 Albert L. Faber
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
#include "RecordWavStream.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


INITTRACE( _T( "RecordWavStream" ) );


// CONSTRUCTOR
RecordWavStream::RecordWavStream()
	: ISndStream()
{
	ENTRY_TRACE( _T( "RecordWavStream::RecordWavStream()" ) );

	memset( &m_WavFormat, 0x00, sizeof( m_WavFormat ) );
	memset( &m_vWavHeader, 0x00, RECNUMSTREAMBUFFERS * sizeof( WAVEHDR ) );
	m_hWavIn = NULL;
	m_pStreamBuffer = NULL;
	m_pBuffersAvailable = NULL;
	m_pBuffersFilled = NULL;
	m_bRecPaused = TRUE;
	m_nRecordBuffer = 0;
	m_nReadBuffer = 0;
	SetChannels( 2 );
	SetSampleRate( 44100 );
	m_bStopped = FALSE;
	m_nWavInDevice = WAVE_MAPPER;

	EXIT_TRACE( _T( "RecordWavStream::RecordWavStream()" ) );

}

// DESCTRUCTOR
RecordWavStream::~RecordWavStream()
{
	ENTRY_TRACE( _T( "RecordWavStream::~RecordWavStream()" ) );

	CloseStream();

	EXIT_TRACE( _T( "RecordWavStream::~RecordWavStream()" ) );
}

// MCI Callback function
void CALLBACK RecordWavStream::RecordCallBack(	HWAVEIN hwi,
												UINT	uMsg,
												DWORD	dwInstance,
												DWORD	dwParam1,
												DWORD	dwParam2 )
{
	long lCount = 0;

	LTRACE2( _T( "Entering RecordWavStream::RecordCallBack()" ) );

	RecordWavStream* pRecordWavStream=(RecordWavStream*)dwInstance;

	if ( FALSE == pRecordWavStream->m_bStopped )
	{
		if (MM_WIM_DATA == uMsg )
		{
			LPWAVEHDR lpWaveHdr = (LPWAVEHDR) dwParam1;
			MMRESULT mmr;

			// Unprepare the header, to get access to the buffer
			mmr = waveInUnprepareHeader(	hwi, 
											lpWaveHdr, 
											sizeof( WAVEHDR ) );

			// Decrease available number of buffers semaphore
			pRecordWavStream->m_pBuffersAvailable->Lock();

			// Increase the number of filled buffers
			pRecordWavStream->m_pBuffersFilled->Unlock( 1, &lCount );

			if ( NULL != pRecordWavStream )
			{
				LTRACE3( _T( "RecordWavStream::RecordCallBack(), Number of filled buffers=%d"), lCount );
			}
		}
	}
	else
	{
		// Increase the number of filled buffers
		pRecordWavStream->m_pBuffersFilled->Unlock( 1, &lCount );
	}
	LTRACE2( _T( "Leaving RecordWavStream::RecordCallBack(), lCount = %d" ), lCount );

}



BOOL RecordWavStream::CloseStream()
{
	ENTRY_TRACE( _T( "RecordWavStream::CloseStream()" ) );

	// Indicate that were are stopping
	m_bStopped = TRUE;

//	m_bAbortThread=TRUE;

	// Reset the input device
	if ( NULL != m_hWavIn )
	{
		waveInReset( m_hWavIn );

		// close input device
		waveInClose( m_hWavIn );

		m_hWavIn = NULL;
	}


	// Delete stream buffer
	delete [] m_pStreamBuffer;
	m_pStreamBuffer=NULL;

	delete m_pBuffersAvailable;
	m_pBuffersAvailable=NULL;

	delete m_pBuffersFilled;
	m_pBuffersFilled=NULL;

	EXIT_TRACE( _T( "RecordWavStream::CloseStream()" ) );

	return TRUE;
}



BOOL RecordWavStream::OpenStream(CUString strFileName)
{
	BOOL bReturn = TRUE;

	ENTRY_TRACE( _T( "RecordWavStream::OpenStream( %s )" ), strFileName );

	MMRESULT mmr;

	m_WavFormat.wBitsPerSample  = 16;
	m_WavFormat.wFormatTag      = WAVE_FORMAT_PCM;
	m_WavFormat.nChannels       = GetChannels();
	m_WavFormat.nSamplesPerSec  = GetSampleRate();
	m_WavFormat.nAvgBytesPerSec = (DWORD) m_WavFormat.nChannels * m_WavFormat.nSamplesPerSec * 2;
	m_WavFormat.nBlockAlign     = (WORD) (m_WavFormat.nChannels << 1);
	m_WavFormat.cbSize          = 0;

//	DWORD pFunc=(DWORD)PlayCallBack;

	if ( waveInOpen(	&m_hWavIn,
						m_nWavInDevice,
						&m_WavFormat,
						(DWORD)RecordCallBack,
						(DWORD)this,
						CALLBACK_FUNCTION) != MMSYSERR_NOERROR)
	{
		CDexMessageBox( _W( "Could not open wave device." ) );
		bReturn = FALSE;
	}

	if ( TRUE == bReturn )
	{
		// Allocate the stream buffer
		m_pStreamBuffer = new BYTE[ RECNUMSTREAMBUFFERS * RECSTREAMBUFFERSIZE ];

		if ( NULL == m_pStreamBuffer )
		{
			bReturn = FALSE;
		}
	}

	if ( TRUE == bReturn )
	{
		// Initialize all stream buffer WavHeader elements
		for (int i=0;i<RECNUMSTREAMBUFFERS;i++)
		{
			m_vWavHeader[i].lpData= (LPSTR)&m_pStreamBuffer[ i * RECSTREAMBUFFERSIZE ]; 
			m_vWavHeader[i].dwBufferLength = RECSTREAMBUFFERSIZE; 
			mmr = waveInPrepareHeader( m_hWavIn, &m_vWavHeader[ i ], sizeof( WAVEHDR ) );
			mmr = waveInAddBuffer( m_hWavIn, &m_vWavHeader[ i ], sizeof( WAVEHDR ) );
		}

		// Create new semphore, set count to RECNUMSTREAMBUFFERS
		m_pBuffersAvailable = new CSemaphore(RECNUMSTREAMBUFFERS,RECNUMSTREAMBUFFERS);

		if ( NULL == m_pBuffersAvailable )
		{
			bReturn = FALSE;
		}
		else
		{
			// Null Buffer Filled right now
			m_pBuffersFilled = new CSemaphore(0,RECNUMSTREAMBUFFERS);

			if ( NULL == m_pBuffersFilled )
			{
				bReturn = FALSE;
			}
		}
	}

	EXIT_TRACE( _T( "RecordWavStream::OpenStream(), return value %d" ), bReturn );

	return bReturn;
}


BOOL RecordWavStream::Start()
{
	ENTRY_TRACE( _T( "RecordWavStream::Start()" ) );

	m_bStopped = FALSE;

	waveInStart( m_hWavIn );
	m_bRecPaused = FALSE;

	EXIT_TRACE( _T( "RecordWavStream::Start()" ) );

	return TRUE;
}

DWORD RecordWavStream::Read( PBYTE pbData, DWORD dwBytes )
{
	MMRESULT	mmr;
	LONG		lCount;

	LTRACE2( _T( "Entering RecordWavStream::Read( %p, %d )" ), pbData, dwBytes );

	// check if m_pBuffersAvailable does exist
	ASSERT( m_pBuffersAvailable );

	//	Wait till at least one buffer is filled
	m_pBuffersFilled->Lock();

	if ( FALSE == m_bStopped )
	{

		// Get data from buffer
		memcpy( pbData, &m_pStreamBuffer[ m_nReadBuffer * RECSTREAMBUFFERSIZE ], dwBytes );

		dwBytes = m_vWavHeader[ m_nReadBuffer ].dwBytesRecorded;

		// Prepare new header
		mmr = waveInPrepareHeader( m_hWavIn, &m_vWavHeader[ m_nReadBuffer ], sizeof( WAVEHDR ) );

		// And add it back to the buffer again
		mmr = waveInAddBuffer( m_hWavIn, &m_vWavHeader[ m_nReadBuffer ],sizeof( WAVEHDR ) );

		// Skip to next buffer
		m_nReadBuffer = ( m_nReadBuffer + 1 )%RECNUMSTREAMBUFFERS;


		// Release buffer, thus increase available buffer
		m_pBuffersAvailable->Unlock( 1, &lCount );
	}
	else
	{
		// Release buffer, thus increase available buffer
		m_pBuffersAvailable->Unlock( 1, &lCount );
	}
	
	LTRACE2( _T( "Leaving RecordWavStream::Read Number of Available buffers=%d\n" ), lCount );

	return dwBytes;
}




void RecordWavStream::Flush()
{
	long lCount = 0;
	MMRESULT mmr;

	//	Wait till at least one buffer is filled
	m_pBuffersFilled->Lock();

	// Prepare new header
	mmr = waveInPrepareHeader( m_hWavIn, &m_vWavHeader[ m_nReadBuffer ], sizeof( WAVEHDR ) );

	// And add it back to the buffer again
	mmr = waveInAddBuffer( m_hWavIn, &m_vWavHeader[ m_nReadBuffer ],sizeof( WAVEHDR ) );

	// Skip to next buffer
	m_nReadBuffer = ( m_nReadBuffer + 1 )%RECNUMSTREAMBUFFERS;


	// Release buffer, thus increase available buffer
	m_pBuffersAvailable->Unlock( 1, &lCount );

}
