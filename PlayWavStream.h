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


#ifndef PLAYWAVSTREAM_H_INCLUDED
#define PLAYWAVSTREAM_H_INCLUDED

#include "OSndStream.h"
#include "mmsystem.h"
#include <afxmt.h>

#define NUMSTREAMBUFFERS 8
#define STREAMBUFFERSIZE 16384



//class CWaveBuffer;

class CWaveBuffer 
{
public:
    CWaveBuffer();
    ~CWaveBuffer();
    BOOL Init( HWAVEOUT hWave, int Size );
    void Done();
    BOOL Write( PBYTE pData, int nBytes, int& BytesWritten );
    void Flush();
private:
    WAVEHDR      m_Hdr;
    HWAVEOUT     m_hWave;
    int          m_nBytes;
};

class CWaveOut 
{
public:
    CWaveOut( LPCWAVEFORMATEX Format, int nBuffers, int BufferSize );
    ~CWaveOut();
    void Write( PBYTE Data, int nBytes );
    void Flush();
	void WaitToFinish();
    void Reset();
	void Abort() { m_bAbort = TRUE; }
	void Pause();
	void Resume();

	HWAVEOUT GetHwave() const { return m_hWave; }

private:
    const HANDLE	m_hSem;
    const int		m_nBuffers;
    int				m_CurrentBuffer;
    BOOL			m_NoBuffer;
    CWaveBuffer*	m_WavBuffer;
    HWAVEOUT		m_hWave;
	BOOL			m_bAbort;
	BOOL			m_bPaused;
};



class PlayWavStream:public OSndStream
{
public:
	// CONSTRUCTOR
	PlayWavStream();

	// DESTRUCTOR
	~PlayWavStream();

	// OPERATIONS
	virtual BOOL	OpenStream( const CUString& strFileName );
	virtual BOOL	CloseStream();
	virtual BOOL	Write(PBYTE pbData,DWORD dwBytes);
	virtual VOID	Stop();
	virtual VOID	Resume();
	virtual VOID	Pause();
	virtual DWORD	GetCurrentTime();

private:
	// ATTRIBUTES
	WAVEFORMATEX	m_WavFormat;
	CWaveOut*		m_pWavOut;
	CMutex			m_mtWavOut;
};



#endif
