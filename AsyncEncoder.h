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


#ifndef ASYNCENCODER_H_INCLUDED
#define ASYNCENCODER_H_INCLUDED

#include "Encode.h"
#include "AfxTempl.h"

class CFifoBuffer
{
public:
	CFifoBuffer();
	~CFifoBuffer( );

	bool Init( int nBufferSize, int nBuffers );
	void DeInit();


	INT		Write( PSHORT pData, int nSize );
	INT		Read( PSHORT pData );
	INT		BuffersAvailable();

private:
	CCriticalSection m_section;
	PSHORT	m_pDataBuffer;
	PINT	m_pSizeBuffer;
	INT		m_nReadIdx;
	INT		m_nWriteIdx;
	INT		m_nBuffers;
	INT		m_nBufferSize;
	INT		m_nBuffersAvailable;
};

class CAsyncEncoder : public CObject
{
public:
	CAsyncEncoder(CEncoder *pEnc, BOOL &_bStopExternal,int nBufferSize, int nBuffers);
	~CAsyncEncoder();

	void		WaitForFinished();
	int			Add(short *pData, DWORD nLen);
	static UINT CAsyncEncoderFeederProc(LPVOID param);


private:

	void ThreadProc( );

	CEvent m_RipEvent;
	CEvent m_EncodeEvent;

	CFifoBuffer			m_FifoBuffer;
	int					nErr;
	volatile BOOL&		m_bStopExternal;
	bool				m_bLastBlock;
	volatile bool		m_bRunning;
	CEncoder*			m_pEncoder;
	CWinThread*			m_pThread;
	int					m_nBufferSize;
	int					m_nBuffers;
};


#endif