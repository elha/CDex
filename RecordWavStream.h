/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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


#ifndef RECORDWAVSTREAM_H_INCLUDED
#define RECORDWAVSTREAM_H_INCLUDED

#include "ISndStream.h"
#include "mmsystem.h"
#include <afxmt.h>

#define RECNUMSTREAMBUFFERS 32
#define RECSTREAMBUFFERSIZE 16384


class RecordWavStream:public ISndStream
{
public:
	// CONSTRUCTOR
	RecordWavStream();

	// DESTRUCTOR
	~RecordWavStream();

	// OPERATIONS
	virtual BOOL	OpenStream( CUString strFileName = _W( "" ) );
	virtual BOOL	CloseStream();
	virtual BOOL	Start();
	virtual void	Flush();
//	virtual LONG	Seek( LONG lOff, UINT nFrom );
	virtual DWORD	Read( PBYTE pbData, DWORD dwBytes );
	virtual void	SetRecordingDevice(INT nValue) {m_nWavInDevice=nValue;}
	virtual INT		GetRecordingDevice() const {return m_nWavInDevice;}

//	static  UINT	RecordWavStream::RecordThread(PVOID pParams);

private:
	// ATTRIBUTES
	WAVEFORMATEX	m_WavFormat;
	HWAVEIN			m_hWavIn;
	WAVEHDR			m_vWavHeader[ RECNUMSTREAMBUFFERS ];
	PBYTE			m_pStreamBuffer;
	INT				m_nRecordBuffer;
	INT				m_nReadBuffer;
	CSemaphore*		m_pBuffersAvailable;
	CSemaphore*		m_pBuffersFilled;
//	BOOL			m_bAbortThread;
	BOOL			m_bRecPaused;
	INT				m_nWavInDevice;
	BOOL			m_bStopped;
	
	// OPERATIONS
	static void CALLBACK RecordCallBack(HWAVEIN hwi, UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);
};



#endif
