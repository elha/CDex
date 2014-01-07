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


/* OSndStream.cpp */

#include "StdAfx.h"
#include "OSndStreamWAV.h"


// CONSTRUCTOR
OSndStreamWAV::OSndStreamWAV()
	:OSndStream()
{
	SetStreamType( SNDSTREAM_WAV );
	m_pSndFile=NULL;
	SetFileExtention( _T( "wav" ) );
	m_OutputFormat = SF_FORMAT_PCM_16;
	SetSampleRate( 44100 );
	SetChannels( 2 );
}

// DESRUCTOR
OSndStreamWAV::~OSndStreamWAV()
{
	CloseStream();
}


//BOOL OSndStreamWAV::Write(PBYTE pbData,DWORD dwNumBytes, double dUpSampleRatio, double dScale )
BOOL OSndStreamWAV::Write(PBYTE pbData,DWORD dwNumBytes )
{

	if ( sf_write_short( m_pSndFile, (short*)pbData, dwNumBytes / 2 ) == dwNumBytes )
		return TRUE;
	return FALSE;
}

BOOL OSndStreamWAV::OpenStream( const CUString& strFileName )
{
	SF_INFO wfInfo;

    memset(&wfInfo,0,sizeof(SF_INFO));
	wfInfo.samplerate  = GetSampleRate();
	wfInfo.frames      = -1;
	wfInfo.sections	   = 1;
	wfInfo.channels    = GetChannels();
	wfInfo.format      = (SF_FORMAT_WAV | m_OutputFormat) ;

	// Set file name
	SetFileName(strFileName);
	
    CUStringConvert strCnv;

	// Open stream
    #ifdef _UNICODE
    if (! (m_pSndFile = sf_open(	(const tchar*)strCnv.ToT( GetFileName() + _W( "." ) + GetFileExtention() ),
									SFM_WRITE,
									&wfInfo ) ) )
    #else
	if (! (m_pSndFile = sf_open(	strCnv.ToT( GetFileName() + _W( "." ) + GetFileExtention() ),
									SFM_WRITE,
									&wfInfo ) ) )
    #endif
	{
		ASSERT( FALSE );
		return FALSE;
	}

	// return Success
	return TRUE;
}

BOOL OSndStreamWAV::CloseStream()
{
	if (m_pSndFile)
	{
		if ( sf_close(m_pSndFile) !=0 )
		{
			ASSERT( FALSE );
			return FALSE;
		}
		m_pSndFile=NULL;
	}
	return TRUE;
}

