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


#ifndef ISNDSTREAMMPEG_H_INCLUDED
#define ISNDSTREAMMPEG_H_INCLUDED

#include <math.h>
#include "ISndStream.h"

#include "mpglibdll/mpglibdll.h"
#include "MPEGHeader.h"


#define MPEG_WAV_BUFFERSIZE 4602


class ISndStreamMPEG : public ISndStream
{
public:
	// CONSTRUCTOR
	ISndStreamMPEG();

	// DESTRUCTOR
	virtual ~ISndStreamMPEG();

	//MUTATORS
	virtual BOOL	OpenStream(CString strFileName);
	virtual DWORD	Read(PBYTE pbData,DWORD dwNumBytes);
	virtual BOOL	CloseStream();
	virtual LONG	Seek( LONG lOff, UINT nFrom );
	virtual void	Flush();
	virtual VOID	InfoBox( HWND hWnd );

	// ACCESSORS
	virtual DWORD	GetTotalTime();
	virtual DWORD	GetCurrentTime();

private:
	FILE*		m_pFile;
	BYTE		m_pbDataInBuffer[MPEG_WAV_BUFFERSIZE];
	MPEGHeader	m_Header;
	HINSTANCE	m_hDLL;
	int			m_dwTotalFileSize;
	int			m_dwCurrentFilePos;
	DWORD		m_dwBytesToDo;
	DWORD		m_dwSkipBytes;

	MPGLIB_OPENSTREAM	m_pOpenStream;
	MPGLIB_DECODECHUNK	m_pDecode;
	MPGLIB_CLOSESTREAM	m_pCloseStream;
	MPGLIB_VERSION		m_pVersion;
	MPGLIB_GETBITRATE	m_pGetBitRate;
	MPGLIB_FLUSH		m_pFlush;
	H_STREAM			m_hStream;
};




#endif