/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006, 2007, 2008 Georgy Berdyshev
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
#include "ISndStreamWav.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



INITTRACE( _T( "ISndStreamWav" ) );


// CONSTRUCTOR
ISndStreamWAV::ISndStreamWAV()
	:ISndStream()
{
	m_pSndFile=NULL;
	SetFileExtention( _W( "wav" ));
	m_dwSamples=0;
	m_ddwTotalFileSize = 0;
	m_ddwCurrentFilePos = 0;
}

// DESRUCTOR
ISndStreamWAV::~ISndStreamWAV()
{
	CloseStream();
}


BOOL ISndStreamWAV::OpenStream( const CUString& strFileName )
{
	SF_INFO wfInfo;

    LTRACE( _T( "ISndStreamWAV::OpenStream( %s )" ), strFileName );

	// Set the buffer size
	SetBufferSize(12000);

	memset(&wfInfo,0,sizeof(SF_INFO));

	SetFileName(strFileName);

	// Open stream
    CUStringConvert strCnv; 

    #ifdef _UNICODE
    m_pSndFile = sf_open((const tchar*)strCnv.ToT( GetFileName() ),
							SFM_READ,
							&wfInfo );
    #else
	m_pSndFile = sf_open(	strCnv.ToT( GetFileName() ),
							SFM_READ,
							&wfInfo );
    #endif
	
    if ( NULL == m_pSndFile )
	{
		LTRACE( _T( "ISndStreamWAV::OpenStream( %s ) failed" ), strFileName );
		ASSERT(FALSE);
		return FALSE;
	}

	SetChannels( wfInfo.channels );
	SetSampleRate(wfInfo.samplerate);


	WORD wBitsPerSample = 0;
	switch ( wfInfo.format & SF_FORMAT_SUBMASK )
	{
		case SF_FORMAT_PCM_S8:
			wBitsPerSample = 8;
		break;
		case SF_FORMAT_PCM_16:
			wBitsPerSample = 16;
		break;
		case SF_FORMAT_PCM_24:
			wBitsPerSample = 24;
		break;
		case SF_FORMAT_PCM_32:
			wBitsPerSample = 32;
		break;
		default:
			wBitsPerSample = 16;
			ASSERT( FALSE );
	}

	SetBitRate( wfInfo.samplerate * wfInfo.channels * wBitsPerSample );
	
	m_dwSamples = (DWORD)wfInfo.frames;

	m_ddwTotalFileSize = sf_seek( m_pSndFile, 0, SEEK_END ) * wfInfo.channels * ( wBitsPerSample / 8 );

	m_ddwCurrentFilePos = 0;

	sf_seek( m_pSndFile, 0, SEEK_SET );

	// return Success
	return TRUE;
}


DWORD ISndStreamWAV::GetTotalTime()
{
	// return total time in ms
	if (GetSampleRate()>0)
		return m_dwSamples/GetSampleRate()*1000;
	return 0;
}

DWORD ISndStreamWAV::GetCurrentTime()
{
	// return total time in ms
	if (GetSampleRate()>0)
		return (DWORD)sf_seek( m_pSndFile, 0, SEEK_CUR )/GetSampleRate()*1000;
	return 0;
}


DWORD ISndStreamWAV::Read(PBYTE pbData,DWORD dwNumBytes)
{
	ASSERT(m_pSndFile);

LTRACE( _T( "ISndStreamWAV::Read dwNumBytes %d  CurrentFilePos %ld TotalFileSize %ld " ),
	   dwNumBytes, 
	   m_ddwCurrentFilePos, 
	   m_ddwTotalFileSize );

	m_ddwCurrentFilePos+= dwNumBytes;

	if ( m_ddwTotalFileSize )
	{
		SetPercent( (INT)( (double) m_ddwCurrentFilePos * 100.0 / m_ddwTotalFileSize ) );
	}
	else
	{
		SetPercent( 0 );
	}

	DWORD dwReturn = (DWORD)sf_read_short( m_pSndFile,(short*)pbData, dwNumBytes / sizeof( SHORT ) ) * sizeof( SHORT );
	
	LTRACE( _T( "ISndStreamWAV::Read returns %d " ),dwReturn );

	return dwReturn;
}


BOOL ISndStreamWAV::CloseStream()
{
	if (m_pSndFile)
	{
		if ( sf_close( m_pSndFile ) !=0 )
		{
			ASSERT( FALSE );
			return FALSE;
		}
		m_pSndFile = NULL;
	}
	return TRUE;
}

__int64 ISndStreamWAV::Seek( __int64 ddwOffset, UINT nFrom )
{
	__int64	ddwFilePosition = 0;
	int		nPercent = (int)ddwOffset;
	
	// Flush the stream
	Flush();

	switch ( nFrom )
	{
		case SEEK_PERCENT:
			if ( nPercent<0  ) nPercent= 0;
			if ( nPercent>99 ) nPercent=99;
			ddwFilePosition= (__int64)( nPercent * m_ddwTotalFileSize / 100.0 );
		break;

		case SEEK_TIME:
			ddwFilePosition= (__int64)( ddwOffset / 1000.0 * GetSampleRate() );
			break;
		default:
			ASSERT( FALSE );
		break;
	}

	ddwFilePosition = ddwFilePosition / 4 * 4;

	// Seek to the desired position
	sf_seek( m_pSndFile, ddwFilePosition, SEEK_SET );

	return ddwFilePosition;
}


void ISndStreamWAV::Flush()
{
}

void ISndStreamWAV::Pause()
{
}

void ISndStreamWAV::InfoBox( HWND hWnd )
{
}
