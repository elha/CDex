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
#include "AsyncEncoder.h"
#include "Config.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "AsyncEncoder" ) );


CFifoBuffer::CFifoBuffer()
:	m_pDataBuffer(NULL),
	m_pSizeBuffer(NULL),
	m_nReadIdx(0),
	m_nWriteIdx(0),
	m_nBufferSize(0),
	m_nBuffers(0),
	m_nBuffersAvailable(0)
{
}

CFifoBuffer::~CFifoBuffer()
{
	DeInit();
}

bool CFifoBuffer::Init( int nBufferSize, int nBuffers )
{
	ENTRY_TRACE( _T( "CFifoBuffer::Init(%d,%d)" ), nBufferSize, nBuffers );

	m_nBufferSize		= nBufferSize;
	m_nBuffers			= nBuffers;
	m_nBuffersAvailable = nBuffers;
	
	// Allocate buffer memory
	m_pSizeBuffer = new INT [ nBuffers ];
	m_pDataBuffer = new SHORT[ nBufferSize * nBuffers ];

	// clear buffers
	memset( m_pSizeBuffer, 0, nBuffers * sizeof( INT ) );
	memset( m_pDataBuffer, 0, nBufferSize * nBuffers * sizeof( SHORT ) );

	EXIT_TRACE( _T( "CFifoBuffer::Init()" ) );

	return true;

}

void CFifoBuffer::DeInit(  )
{
	ENTRY_TRACE( _T( "CFifoBuffer::DeInit()" ) );

	delete [] m_pSizeBuffer;
	delete [] m_pDataBuffer;

	m_pSizeBuffer = NULL;
	m_pDataBuffer = NULL;

	EXIT_TRACE( _T( "CFifoBuffer::DeInit()" ) );
}

INT CFifoBuffer::BuffersAvailable()
{
	LTRACE3( _T( "CFifoBuffer::BuffersAvailable() returns %d" ), m_nBuffersAvailable );

	return m_nBuffersAvailable;
}

INT CFifoBuffer::Write( PSHORT pData, int nSize )
{
	LTRACE2( _T( "Entering CFifoBuffer::Write(%d,%d)" ), pData, nSize );

	if ( m_nBuffersAvailable == 0 )
	{
		return 0;
	}

	memcpy( 
			&m_pDataBuffer[m_nWriteIdx * m_nBufferSize],
			pData,
			nSize * sizeof(SHORT)
			);

	m_pSizeBuffer[ m_nWriteIdx ] = nSize;

	m_nWriteIdx= (m_nWriteIdx+1) % m_nBuffers;

	m_section.Lock();

	m_nBuffersAvailable--;

	m_section.Unlock();

	LTRACE( _T( "CFifoBuffer::Write()" ) );

	return  nSize;
}


INT CFifoBuffer::Read( PSHORT pData )
{
	LTRACE2( _T( "Entering CFifoBuffer::Read(%d)" ), pData );

	if ( m_nBuffersAvailable == m_nBuffers )
	{
		return 0;
	}

	INT	nSize = m_pSizeBuffer[ m_nReadIdx ];

	memcpy( 
			pData,
			&m_pDataBuffer[m_nReadIdx * m_nBufferSize],
			nSize * sizeof( SHORT )
			);

	m_nReadIdx = (m_nReadIdx + 1) % m_nBuffers;

	m_section.Lock();

	m_nBuffersAvailable++;

	m_section.Unlock();

	LTRACE( _T( "CFifoBuffer::Read() : %d" ), nSize );

	return nSize;
}


CAsyncEncoder::CAsyncEncoder(CEncoder *pEnc, BOOL& bStopExternal, int nBufferSize, int nBuffers)
	: m_bStopExternal( bStopExternal ),
	m_pEncoder( pEnc )

{
	nErr			= CDEX_OK;
	m_bLastBlock	= false;
	m_bRunning		= FALSE;
	m_pThread		= NULL;
	m_nBuffers		= nBuffers;
	m_nBufferSize	= nBufferSize;

	m_FifoBuffer.Init( nBufferSize, nBuffers );

	m_pThread = AfxBeginThread(CAsyncEncoderFeederProc, this);

	// yield
	::Sleep(0);
}

CAsyncEncoder::~CAsyncEncoder()
{
	if(m_bRunning)
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
}



void CAsyncEncoder::ThreadProc( void )
{
	// indicate thread is active
	m_bRunning			= TRUE;
	SHORT*		pData	= NULL;
	INT			nSize	= 0;


	ENTRY_TRACE( _T( "CAsyncEncoder::ThreadProc()" ) );

	pData = new SHORT[ m_nBufferSize ];

	while( !m_bStopExternal )
	{
		// if the fifo has more than 3/4 of its buffers available then wake the ripping thread
		if(m_FifoBuffer.BuffersAvailable() > (3 * m_nBuffers) / 4)
			m_RipEvent.SetEvent();

		// are there any entries in the buffer ?
		if( m_FifoBuffer.BuffersAvailable() < m_nBuffers )
		{
			// get top most array entry
			nSize= m_FifoBuffer.Read( pData );

			if ( nSize )
			{
				nErr = m_pEncoder->EncodeChunk( pData, nSize ); 

				// Abort the encoding loop when we get an encoding error
				if ( nErr != CDEX_OK )
				{
					break;
				}
			}
		}
		else
		{
			if ( m_bLastBlock )
			{
				break;
			}

			// this thread sleeps
			m_EncodeEvent.Lock(50);
		}
	}

	delete [] pData;

	EXIT_TRACE( _T( "CAsyncEncoder::ThreadProc()" ) );

	// indicate thread has been finished
	m_bRunning = FALSE;
}


void CAsyncEncoder::WaitForFinished()
{
	int nSize = -1;

	ENTRY_TRACE( _T( "CAsyncEncoder::WaitForFinished()" ) );

	DWORD dwStillAlive=0;

	m_bLastBlock = true;

	dwStillAlive=WaitForSingleObject(m_pThread->m_hThread,100000);
	
	if (dwStillAlive==WAIT_TIMEOUT)
	{
		LTRACE( _T( "CAsyncEncoder TimeOut!" ) );

		// Kill Thread
		::TerminateThread( m_pThread->m_hThread, 0 );
	}

	EXIT_TRACE( _T( "CAsyncEncoder::WaitForFinished()" ) );
};


int CAsyncEncoder::Add(short *pData, DWORD nLen)
{
	if(!m_bRunning || m_bStopExternal)
	{
		return nErr;
	}

	if( nLen )
	{
		// if the fifo has less than 1/4 of its buffers available then this thread sleeps
		while ( m_FifoBuffer.BuffersAvailable() < (1 * m_nBuffers) / 4 ) 
		{
			m_RipEvent.Lock( 1000 );
			if(!m_bRunning || m_bStopExternal)
			{
				return nErr;
			}
		}


		m_FifoBuffer.Write( pData, nLen );

		// wake up the encoding thread
		m_EncodeEvent.SetEvent();
	}
	return nErr;
}


UINT CAsyncEncoder::CAsyncEncoderFeederProc(LPVOID param)
{
	((CAsyncEncoder*)param)->ThreadProc();
	return 0;
}

