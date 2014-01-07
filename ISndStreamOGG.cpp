/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 Albert L. Faber
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
#include "ISndStreamOGG.h"
#include "Config.h"


// CONSTRUCTOR
ISndStreamOGG::ISndStreamOGG()
	:ISndStream()
{
	SetFileExtention("ogg");
	SetStreamType( SNDSTREAM_OGG );

	m_dwSamples=0;
	m_hDLL=NULL;

	m_DecOpen	=NULL;
	m_DecRead	=NULL;
	m_DecSeek	=NULL;
	m_DecClose	=NULL;

	memset(&m_decInfo,0,sizeof(m_decInfo));
	m_decInfo.hStream=-1;
}

// DESRUCTOR
ISndStreamOGG::~ISndStreamOGG()
{
	CloseStream();
}


BOOL ISndStreamOGG::OpenStream(CString strFileName)
{
	DWORD		dwRead=0;

	SetFileName(strFileName);


	// Set the buffer size
	SetBufferSize(12000);

	// Open stream
	//if (! (m_pSndFile = sf_open_read(GetFileName(),&wfInfo)))


	// try to load the Vorbis DLL
	m_hDLL=LoadLibrary(g_config.GetAppPath()+"\\vorbis.dll");

#ifdef _DEBUG
	if (m_hDLL==NULL)
		m_hDLL=LoadLibrary(g_config.GetAppPath()+"\\Vorbis\\Dll\\Debug\\vorbis.dll");
#else
	if (m_hDLL==NULL)
		m_hDLL=LoadLibrary(g_config.GetAppPath()+"\\Vorbis\\Dll\\Release\\vorbis.dll");
#endif

	if (m_hDLL==NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Get the DLL functions
	m_DecOpen	= (VBDECOPEN)	GetProcAddress(m_hDLL, TEXT_VBDECOPEN);
	m_DecRead	= (VBDECREAD)	GetProcAddress(m_hDLL, TEXT_VBDECREAD);
	m_DecClose	= (VBDECCLOSE)	GetProcAddress(m_hDLL, TEXT_VBDECCLOSE);
	m_DecSeek	= (VBDECSEEK) 	GetProcAddress(m_hDLL, TEXT_VBDECSEEK);

	// try to open the input file
	if ( m_DecOpen(&m_decInfo,NULL,strFileName)!=VB_ERR_OK)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	if (m_decInfo.nBitRate>0)
		SetBitRate(m_decInfo.nBitRate);

	SetChannels(m_decInfo.dwChannels);
	SetSampleRate(m_decInfo.dwSampleRate);
	
	// return Success
	return TRUE;
}


DWORD ISndStreamOGG::GetTotalTime()
{
	return m_decInfo.dwTotTime_in_ms;
}

DWORD ISndStreamOGG::GetCurrentTime()
{
	return m_decInfo.dwCurTime_in_ms;
}

SHORT pOutBuf[4096];

DWORD ISndStreamOGG::Read(PBYTE pbData,DWORD dwNumBytes)
{
	DWORD dwWrite=0;
	m_DecRead(&m_decInfo,pbData,&dwWrite);

	SetPercent( (INT) ( (double)m_decInfo.dwCurTime_in_ms * 100.0 / m_decInfo.dwTotTime_in_ms ) );

	if 	(m_decInfo.nBitRate>0)
		SetBitRate(m_decInfo.nBitRate);

	return dwWrite;
}


BOOL ISndStreamOGG::CloseStream()
{
	if (m_hDLL)
	{
		m_DecClose(&m_decInfo);

		FreeLibrary(m_hDLL);

		m_hDLL=NULL;
	}

	return TRUE;
}

LONG ISndStreamOGG::Seek( LONG lOff, UINT nFrom )
{
	LONG lSeekTo=0;

	int nPercent=lOff;

	// Limit percentage number
	if (nPercent<0 ) nPercent= 0;
	if (nPercent>99) nPercent=99;

	// Calculate seek to time, in ms
	if (nFrom==SEEK_PERCENT)
	{
		lSeekTo=nPercent*m_decInfo.dwTotTime_in_ms/100;
	}

	// Do it
	m_DecSeek(&m_decInfo,lSeekTo);

	// We're done, return seek position
	return lSeekTo;
}


void ISndStreamOGG::Flush()
{
}

void ISndStreamOGG::InfoBox( HWND hWnd )
{
}