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


#ifndef ISNDSTREAMOGG_H_INCLUDED
#define ISNDSTREAMOGG_H_INCLUDED

#include "ISndStream.h"

extern "C"
{
	#include "Vorbis\Dll\VorbisDLL.h"
}

#define OGG_WAV_BUFFERSIZE 16384

class ISndStreamOGG : public ISndStream
{
private:
	DWORD				m_dwSamples;
	FILE*				m_pFile;
	HINSTANCE			m_hDLL;

	VBDECOPEN			m_DecOpen;
	VBDECREAD			m_DecRead;
	VBDECSEEK			m_DecSeek;
	VBDECCLOSE			m_DecClose;

	VB_DEC_INFO			m_decInfo;


public:
	ISndStreamOGG();
	virtual ~ISndStreamOGG();
	virtual BOOL	OpenStream(CString strFileName);
	virtual DWORD	Read(PBYTE pbData,DWORD dwNumBytes);
	virtual BOOL	CloseStream();
	virtual LONG	Seek( LONG lOff, UINT nFrom );
	virtual void	Flush();
	virtual VOID	InfoBox( HWND hWnd );
	
	virtual DWORD	GetTotalTime();
	virtual DWORD	GetCurrentTime();


	
};




#endif


