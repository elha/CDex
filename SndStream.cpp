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
#include "SndStream.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CONSTRUCTOR
SndStream::SndStream()
{
	m_nBufferSize=SNDSTREAM_BUFFER_SIZE;			// Set default buffer size
	m_nChannels = 0;									// init number of channels
	m_nSampleRate = 0;								// init sample rate
	m_nPercent = 0;									// init percent completed
	m_pModifyBuffer = NULL;
	m_dwModifyBufferSize = 0;
	m_nPeakValue = -1;
	m_nBytesPerSample = 2;
}

// COPY CONSTRUCTOR
SndStream::SndStream(const SndStream& rhs)
{
	m_nBufferSize = rhs.m_nBufferSize;
	m_nChannels = rhs.m_nChannels;
	m_nSampleRate = rhs.m_nSampleRate;
	m_nPercent = rhs.m_nPercent;
	m_nPeakValue = rhs.m_nPeakValue;
	m_nBytesPerSample = rhs.m_nBytesPerSample;
}

// COPY STREAM INFORMATION
void SndStream::CopyStreamInfo(const SndStream& rhs)
{
	m_nChannels = rhs.m_nChannels;
	m_nSampleRate = rhs.m_nSampleRate;
	m_nPeakValue = rhs.m_nPeakValue;
	m_nBytesPerSample = rhs.m_nBytesPerSample;
}


// DESRUCTOR
SndStream::~SndStream()
{
	if ( m_pModifyBuffer )
	{
		free ( m_pModifyBuffer );
	}
}



BOOL SndStream::OpenStream( const CUString& strFileName )
{
	ASSERT( FALSE );
	return FALSE;
}

BOOL SndStream::CloseStream()
{
	ASSERT( FALSE );
	return FALSE;
}

LONG SndStream::Seek( LONG lOff, UINT nFrom )
{
	return -1;
}

DWORD SndStream::ModifyStreamData( BYTE* pData, DWORD dwBytes, double dUpSampleRatio, double dScale )
{
	DWORD dwISample =0;
	DWORD dwOSample =0;

	SHORT* psIData = (SHORT*) pData;
	SHORT* psOData = (SHORT*) m_pModifyBuffer;

	// allocate initial buffer 
	if ( m_pModifyBuffer == NULL )
	{
		m_dwModifyBufferSize = 8192;
		m_pModifyBuffer = (BYTE*)calloc( m_dwModifyBufferSize, 1 );
		psOData = (SHORT*) m_pModifyBuffer;
	}


	if ( dUpSampleRatio > 1.0 )
	{
		for ( dwISample = 0; dwISample < dwBytes/2 ; dwISample++ )
		{
			
		}
	}
	else
	{

	}
	return dwOSample;
}


void SndStream::CalcPeakValue( BYTE* pbtData, DWORD dwBytes )
{
	DWORD	dwSample = 0;
	PSHORT	psData = (PSHORT)pbtData;

	switch ( m_nBytesPerSample )
	{
		case 1:
			ASSERT( pbtData );

			for ( dwSample = 0; dwSample < dwBytes; dwSample ++ )
			{
				m_nPeakValue = abs( pbtData[ dwSample ] );
			}

		break;
		case 2:
			ASSERT( psData );

			for ( dwSample = 0; dwSample < dwBytes / sizeof (SHORT ); dwSample ++ )
			{
				m_nPeakValue = abs( psData[ dwSample ] );
			}
		break;
		default:
			ASSERT( FALSE );
		break;

	}
}

