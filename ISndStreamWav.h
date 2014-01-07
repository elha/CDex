/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 - 2007 Albert L. Faber
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


#ifndef ISNDSTREAMWAV_H_INCLUDED
#define ISNDSTREAMWAV_H_INCLUDED

#include "ISndStream.h"

extern "C"
{
	#include "sndfile.h"	
}


class ISndStreamWAV : public ISndStream
{
	SNDFILE*	m_pSndFile;
	DWORD		m_dwSamples;
public:
	ISndStreamWAV();
	virtual ~ISndStreamWAV();
	virtual BOOL	OpenStream( const CUString& strFileName);
	virtual DWORD	Read(PBYTE pbData,DWORD dwNumBytes);
	virtual BOOL	CloseStream();
	virtual __int64	Seek( __int64 ddwOffset, UINT nFrom );
	virtual void	Flush();
	virtual void	Pause();
	virtual VOID	InfoBox( HWND hWnd );
	
	virtual DWORD	GetTotalTime();
	virtual DWORD	GetCurrentTime();

private:
	__int64			m_ddwCurrentFilePos;
	__int64			m_ddwTotalFileSize;
};




#endif