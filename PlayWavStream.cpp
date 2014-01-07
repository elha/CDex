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
#include "PlayWavStream.h"
#include "Config.h"
#include <mmsystem.h>


INITTRACE( _T( "PlayWavStream" ) );

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static LONG glLastCount=0;

// CONSTRUCTOR
PlayWavStream::PlayWavStream()
	: OSndStream()
{

	SetBufferSize( STREAMBUFFERSIZE );

	m_mtWavOut.Lock();

	m_pWavOut = NULL;

	m_mtWavOut.Unlock();

}

// DESCTRUCTOR
PlayWavStream::~PlayWavStream()
{
	CloseStream();
}


VOID PlayWavStream::Stop()
{
	m_mtWavOut.Lock();

	if (m_pWavOut)
	{
		m_pWavOut->Abort();
		m_pWavOut->Reset();
	}

	m_mtWavOut.Unlock();
}


VOID PlayWavStream::Pause()
{
	m_mtWavOut.Lock();

	if (m_pWavOut)
	{
		m_pWavOut->Pause();
		m_pWavOut->Reset();
	}

	m_mtWavOut.Unlock();
}

VOID PlayWavStream::Resume()
{
	m_mtWavOut.Lock();

	if ( m_pWavOut )
	{
		m_pWavOut->Resume();
	}

	m_mtWavOut.Unlock();
}

BOOL PlayWavStream::CloseStream()
{
	m_mtWavOut.Lock();

	if ( m_pWavOut )
	{
		m_pWavOut->WaitToFinish();
		delete m_pWavOut;
		m_pWavOut=NULL;
	}
	m_mtWavOut.Unlock();

	return TRUE;
}



BOOL PlayWavStream::OpenStream(const CUString& strFileName)
{
	memset(&m_WavFormat,0,sizeof(m_WavFormat));

	m_WavFormat.wBitsPerSample  = 16;
	m_WavFormat.wFormatTag      = WAVE_FORMAT_PCM;
	m_WavFormat.nChannels       = GetChannels();
	m_WavFormat.nSamplesPerSec  = GetSampleRate();
	m_WavFormat.nAvgBytesPerSec = (DWORD) m_WavFormat.nChannels * m_WavFormat.nSamplesPerSec * 2;
	m_WavFormat.nBlockAlign     = (WORD) (m_WavFormat.nChannels << 1);
	m_WavFormat.cbSize          = 0;

	m_mtWavOut.Lock();

	m_pWavOut=new CWaveOut(&m_WavFormat,NUMSTREAMBUFFERS,STREAMBUFFERSIZE);

	m_mtWavOut.Unlock();

	if ( NULL == m_pWavOut )
		return FALSE;


	return TRUE;
}

BOOL PlayWavStream::Write(PBYTE pbData,DWORD dwBytes)
{
	BOOL bReturn = FALSE;

	m_mtWavOut.Lock();

	if ( m_pWavOut )
	{
		m_pWavOut->Write( pbData,dwBytes );
		bReturn = TRUE;
	}
	m_mtWavOut.Unlock();

	return bReturn;
}

DWORD PlayWavStream::GetCurrentTime()
{
	DWORD dwResult = 0;

	m_mtWavOut.Lock();

	if (	m_pWavOut &&
			NULL != m_pWavOut->GetHwave() )
	{
		MMTIME		mmt;
		MMRESULT	mmr;

		mmt.wType = TIME_BYTES;

		mmr = waveOutGetPosition( m_pWavOut->GetHwave(), &mmt, sizeof( mmt ) ); 

		if ( m_WavFormat.nAvgBytesPerSec )
		{
			dwResult = (DWORD)( mmt.u.cb * 1000.0 / m_WavFormat.nAvgBytesPerSec );
		}
	}

	m_mtWavOut.Unlock();

	return dwResult;
} 



#include <windows.h>
#include <mmsystem.h>
//#include <amstream.h>

/********************************************************************

   Trivial wave player stuff

 ********************************************************************/


/*
    CWaveBuffer
*/

CWaveBuffer::CWaveBuffer()
{
	glLastCount=0;

}

BOOL CWaveBuffer::Init(HWAVEOUT hWave, int Size)
{
    m_hWave  = hWave;
    m_nBytes = 0;

    /*  Allocate a buffer and initialize the header. */
    m_Hdr.lpData = (LPSTR)LocalAlloc( LMEM_FIXED, Size );
    
	if ( NULL == m_Hdr.lpData )
	{
        return FALSE;
    }

    m_Hdr.dwBufferLength  = Size;
    m_Hdr.dwBytesRecorded = 0;
    m_Hdr.dwUser = 0;
    m_Hdr.dwFlags = 0;
    m_Hdr.dwLoops = 0;
    m_Hdr.lpNext = 0;
    m_Hdr.reserved = 0;

    /*  Prepare it. */
    waveOutPrepareHeader( hWave, &m_Hdr, sizeof( WAVEHDR ) );

    return TRUE;
}

CWaveBuffer::~CWaveBuffer()
{
    if ( m_Hdr.lpData )
	{
        waveOutUnprepareHeader(m_hWave, &m_Hdr, sizeof(WAVEHDR));
        LocalFree( m_Hdr.lpData );
    }
}

void CWaveBuffer::Flush()
{
    // ASSERT(m_nBytes != 0);
    m_nBytes = 0;
    waveOutWrite(	m_hWave, 
					&m_Hdr,
					sizeof( WAVEHDR ) );
}

BOOL CWaveBuffer::Write(PBYTE pData, int nBytes, int& BytesWritten)
{
    // ASSERT((DWORD)m_nBytes != m_Hdr.dwBufferLength);
    BytesWritten = min((int)m_Hdr.dwBufferLength - m_nBytes, nBytes);
    CopyMemory((PVOID)(m_Hdr.lpData + m_nBytes), (PVOID)pData, BytesWritten);
    m_nBytes += BytesWritten;
    if (m_nBytes == (int)m_Hdr.dwBufferLength) {
        /*  Write it! */
        m_nBytes = 0;
        waveOutWrite(m_hWave, &m_Hdr, sizeof(WAVEHDR));
        return TRUE;
    }
    return FALSE;
}

void CALLBACK WaveCallback(HWAVEOUT hWave, UINT uMsg, DWORD dwUser, 
                           DWORD dw1, DWORD dw2)
{
    if (uMsg == WOM_DONE)
	{
        ReleaseSemaphore((HANDLE)dwUser, 1, &glLastCount);
    }
}

/*
    CWaveOut
*/

CWaveOut::CWaveOut(LPCWAVEFORMATEX Format, int nBuffers, int BufferSize) :
    m_nBuffers( nBuffers ),
    m_CurrentBuffer( 0 ),
    m_NoBuffer( TRUE ),
    m_hSem( CreateSemaphore( NULL, nBuffers, nBuffers, NULL ) ),
    m_WavBuffer( new CWaveBuffer[ nBuffers ] ),
    m_hWave( NULL ),
	m_bAbort( FALSE ),
	m_bPaused( FALSE )
{
	glLastCount= m_nBuffers-1;

    /*  Create wave device. */
    waveOutOpen(	&m_hWave,
					WAVE_MAPPER,
					Format,
					(DWORD)WaveCallback,
					(DWORD)m_hSem,
					CALLBACK_FUNCTION );

    /*  Initialize the wave buffers. */
    for (int i = 0; i < nBuffers; i++)
	{
        m_WavBuffer[i].Init( m_hWave, BufferSize );
    }

}


CWaveOut::~CWaveOut()
{
    /*  First, get the buffers back. */
    waveOutReset( m_hWave );

    /*  Free the buffers. */
    delete [] m_WavBuffer;

    /*  Reset the device, just to be sure */
    waveOutReset( m_hWave );

    /*  Close the wave device. */
    waveOutClose(m_hWave);

    /*  Free the semaphore. */
    CloseHandle( m_hSem );
}

void CWaveOut::Flush()
{
    if (!m_NoBuffer)
	{
        m_WavBuffer[m_CurrentBuffer].Flush();

        m_NoBuffer = TRUE;

        m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;

    }
}

void CWaveOut::Reset()
{
    waveOutReset( m_hWave );

	// clear all pending buffers

}

void CWaveOut::Pause( )
{
    m_bPaused = TRUE;
}

void CWaveOut::Resume( )
{
    m_bPaused = FALSE;
}


void CWaveOut::Write( PBYTE pData, int nBytes )
{
    while ( nBytes != 0 )
	{

        /*  Get a buffer if necessary. */
        if ( m_NoBuffer )
		{
            WaitForSingleObject( m_hSem, INFINITE );
            m_NoBuffer = FALSE;
        }

        /*  Write into a buffer. */
        int nWritten;

		if ( !m_bPaused )
		{
			if ( m_WavBuffer[ m_CurrentBuffer ].Write( pData, nBytes, nWritten ) )
			{
				m_NoBuffer = TRUE;
				m_CurrentBuffer = (m_CurrentBuffer + 1) % m_nBuffers;
				nBytes -= nWritten;
				pData += nWritten;
			} else {
				// ASSERT(nWritten == nBytes);
				break;
			}
		}
		else
		{
			// when paused, through everything in the byte bucket
			// we have to re-seek anyways due to the reset
			nBytes = 0;
		}
    }
}


void CWaveOut::WaitToFinish()
{
	if ( FALSE == m_bAbort )
	{
		// Wait till all data has been played
		while ( glLastCount< (m_nBuffers-1) )
		{
			::Sleep(10);
		}
	}

	while ( waveOutClose(m_hWave) == WAVERR_STILLPLAYING )
	{
		::Sleep(10);
	}

}

