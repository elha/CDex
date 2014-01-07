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
#include "CDPlayDigital.h"
#include "config.h"
#include "AsyncEncoder.h"
#include "PlayWavStream.h"
#include "CDInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "CDPlayDigital") );



UINT CCDPlayDigital::RipAndPlay( PVOID pParams )
{
	CDEX_ERR	bReturn = CDEX_OK;
	long		nBufferSize = 0;
	BOOL		bPause = FALSE;

	// tracing info
	ENTRY_TRACE( _T( "CCDPlayDigital::RipAndPlay" ) );
	
	// get pointer to active CCDPlayDigital object
	CCDPlayDigital* pParent= (CCDPlayDigital*)pParams ;

	pParent->m_bAbort = FALSE;

	auto_ptr<PlayWavStream> pPlayStream( new PlayWavStream );

	if ( NULL == pPlayStream.get() )
	{
		bReturn = CDEX_ERROR;
	}
	else
	{
		pPlayStream->SetSampleRate( 44100 );
		pPlayStream->SetChannels( 2 );
		pPlayStream->SetBytesPerSample( sizeof( SHORT ) );
	}

	// Initialize play stream object
	if ( FALSE == pPlayStream->OpenStream( _W("") ) )
	{
		bReturn = CDEX_ERROR;
	}
	else
	{
	}

	DWORD dwSampleBufferSize = pPlayStream->GetBufferSize() / sizeof( SHORT );


	if ( CDEX_OK == bReturn )
	{
		bReturn = CR_OpenRipper(	&nBufferSize,
									pParent->GetBeginSector(),
									pParent->GetEndSector(),
                                    FALSE );
	}

	if ( CDEX_OK == bReturn )
	{
		LONG			nNumBytesRead = 0;
		LONG			nOffset = 0;
		
		// create the stream buffer, allocate on enocder frame additional memory
		auto_ptr<BYTE> pbtBufferStream( new BYTE[ nBufferSize + pPlayStream->GetBufferSize() + 16383 ] );

		// Get a pointer to the buffer
		BYTE* pbtStream = pbtBufferStream.get();

		CDEX_ERR ripErr;

		// Read all chunks
		while (	( CDEX_RIPPING_DONE  != ( ripErr = CR_RipChunk( pbtStream + nOffset, &nNumBytesRead, (BOOL&)pParent->m_bAbort ) ) ) 
				&& !*(volatile BOOL*)&pParent->m_bAbort )
		{
			SHORT*	psEncodeStream=(SHORT*)pbtStream;
			DWORD	dwSamplesToConvert= ( nNumBytesRead + nOffset ) / sizeof( SHORT );

			while ( pParent->m_bPause )
			{
				if ( FALSE == bPause )
				{
					pParent->m_dwCurrentTime = 0;
					pParent->m_dwSeekTimeOffset = pPlayStream->GetCurrentTime();

					bPause = TRUE;
					pPlayStream->Pause();
				}

				::Sleep( 10 );
			}

			if ( bPause )
			{
				pPlayStream->Resume();
				bPause = FALSE;
			}


			// Check for jitter errors
			if ( CDEX_JITTER_ERROR == ripErr )
			{
			}

			// Check if an error did occur
			if ( CDEX_ERROR == ripErr )
			{
				LTRACE( _T( "RipToEncoder::CDEX_ERROR" ) );
				break;
			}

			// Get progress indication
			//nPercent = CR_GetPercentCompleted();

			// Get relative jitter position
			// nJitterPos = CR_GetJitterPosition();

			// Get the number of jitter errors
			// nJitterErrors = CR_GetNumberOfJitterErrors();

			// Get the Peak Value
			// nPeakValue = CR_GetPeakValue();

			// Convert the samples with the encoder
			while ( dwSamplesToConvert >= dwSampleBufferSize )
			{
				if( FALSE == pPlayStream->Write( (BYTE*)psEncodeStream, dwSampleBufferSize * sizeof( SHORT ) ) )
				{
					pParent->m_bAbort = TRUE;
					bReturn = CDEX_ERROR;
				}

				pParent->m_dwCurrentTime = pPlayStream->GetCurrentTime();
				pParent->m_dwBytesWritten += dwSampleBufferSize * sizeof( SHORT );

				// Decrease the number of samples to convert
				dwSamplesToConvert -= dwSampleBufferSize;

				// Increase the sample buffer pointer
				psEncodeStream += dwSampleBufferSize;
			}

			// Copy the remaing bytes up front, if necessary
			if ( dwSamplesToConvert > 0 )
			{
				// Calculate the offset in bytes
				nOffset = dwSamplesToConvert * sizeof( SHORT );

				// Copy up front
				memcpy( pbtStream, psEncodeStream, nOffset );
			}
			else
			{
				nOffset = 0;
			}
		}

		// stop as quickly as possible on when aborting
		if ( pParent->m_bAbort )
		{
			pPlayStream->Pause();
			pPlayStream->Stop();
		}
		else
		{
			if ( nOffset && pParent->m_bAbort )
			{
				if( FALSE == pPlayStream->Write( pbtStream, nOffset ) )
				{
					bReturn = CDEX_ERROR;
				}
				pParent->m_dwBytesWritten += nOffset;
			}

			LTRACE( _T( "Wait for player to Finish" ) );
		}

		// Close the WAV player
		pPlayStream->CloseStream();
	
		LTRACE( _T( "Player finished" ) );

		// Close the Ripper session

		CR_CloseRipper( NULL );

	}
	else
	{
		ASSERT( FALSE );
	}

	pParent->m_pThread = NULL;
	pParent->m_bIsPlaying = FALSE;

	EXIT_TRACE( _T( "RipAndPlay" ) );

	return bReturn;
}


CCDPlayDigital::CCDPlayDigital():
			CObject()
{
	ENTRY_TRACE( _T( "CCDPlayDigital::CCDPlayDigital" ) );

	m_pThread = NULL;
	m_bAbort = FALSE;
	m_dwBeginSector = 0;
	m_dwEndSector = 0;
	m_dwCurrentSector = 0;
	m_bIsPlaying = TRUE;
	m_dwTotalBytes = 0;
	m_dwBytesWritten = 0;
	m_dwCurrentTime = 0;
	m_dwSeekTimeOffset = 0;
	m_bPause = FALSE;

	EXIT_TRACE( _T( "CCDPlayDigital::CCDPlayDigital" ) );

}

CCDPlayDigital::~CCDPlayDigital()
{
	ENTRY_TRACE( _T( "CCDPlayDigital::~CCDPlayDigital" ) );
	EXIT_TRACE( _T( "CCDPlayDigital::~CCDPlayDigital" ) );
}



void CCDPlayDigital::Play( DWORD dwBeginSector, DWORD dwEndSector )
{
	ENTRY_TRACE( _T( "CCDPlayDigital::Play( %d, %d )" ), dwBeginSector, dwEndSector );

	m_bIsPlaying = TRUE;

	ASSERT( dwBeginSector <= dwEndSector );

	SetBeginSector( dwBeginSector );
	SetEndSector( dwEndSector );

	m_dwTotalBytes = (dwEndSector - dwBeginSector ) * CB_CDDASECTORSIZE;
	m_dwBytesWritten = 0;

	// Fire thread
	m_pThread = AfxBeginThread( RipAndPlay, (void*)this, g_config.GetThreadPriority() );

	EXIT_TRACE( _T( "CCDPlay::Play( )" ) );

}

void CCDPlayDigital::Stop( )
{
	ENTRY_TRACE( _T( "CCDPlayDigital::Stop()" ) );

	m_bPause = FALSE;
	m_bAbort = TRUE;

	// wait till thread has been finished
	while ( NULL != (volatile void*)m_pThread )
	{
		::Sleep( 10 );
	}

	EXIT_TRACE( _T( "CCDPlayDigital::Stop()" ) );
}

DWORD CCDPlayDigital::GetCurrentTimeInSecs( )
{
	DWORD dwReturn;

	LTRACE2( _T( "Entering CCDPlayDigital::GetCurrentTimeInSecs()" ) );

	dwReturn = ( m_dwCurrentTime + m_dwSeekTimeOffset ) / 1000;

	LTRACE2( _T( "Leaving CCDPlayDigital::GetCurrentTimeInSecs(), return value %d" ), dwReturn );

	return dwReturn;
}

void CCDPlayDigital::Pause( BOOL bPause )
{
	ENTRY_TRACE( _T( "CCDPlayDigital::Pause( %d )" ), bPause );

	m_bPause = bPause;

	EXIT_TRACE( _T( "CCDPlayDigital::Pause()" ) );
}

