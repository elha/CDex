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


#ifndef CDEXTRACT_INCLUDED
#define CDEXTRACT_INCLUDED

#include "AspiCD.h"

extern "C" {
	#include "./interface/cdda_interface.h"
	#include "./paranoia/cdda_paranoia.h"
}

class CCDExtract:public CAspiCD
{
private:
	int				m_nPeakValue;
	PBYTE			m_pbtReadBuffer;
	PBYTE			m_pbtOverlapBuffer;
	PBYTE			m_pbtReadBufferCompare;
	LONG			m_lSector;
    ULONG           m_ulCRC;

	int				m_nBeginSector;
	int				m_nEndSector;
	int				m_nJitterPos;
	DWORD			m_nJitterErrors;
	BOOL			m_bFirstRead;
	INT				m_nPercentCompleted;
	DWORD			m_dwBytesToDo;
	DWORD			m_dwReadSectors;
	BOOL			m_bC2ErrorDetectionEnabled;
	DWORD			m_dwReadBufferSize;
	BOOL			m_bFuaBit;
	cdrom_paranoia*	m_pParanoia;
	cdrom_drive*	m_pParanoiaDrive;
	INT				m_nCurrentSpeed;
	INT				m_nLastSpeedAdjust;
	volatile BOOL*	m_pbAbort;
	HANDLE			m_rippingThreadHandle;
	DWORD			m_dwFastModeInBuffer;
    BOOL            m_bExcludeNullSamplesInCRC;
public:
	// CONSRUCTOR
	CCDExtract();

	// DESTRUCTOR
	~CCDExtract();

	// The Thread Function
	static	UINT ThreadFunc(LPVOID pVoid);
	DWORD	CorrectJitter( BYTE* pbtOverlapPtr,BYTE* pbtReadPtr,DWORD dwBytesRead,BOOL& bJitterError );
	BOOL	SetupTrackExtract( int nBeginSector, int nEndSector );
	BOOL	SetupTrackExtractParanoia( int nBeginSector, int nEndSector );
	BOOL	EndTrackExtract();
	UINT	SetupConvertThread( void* pThreadParams );
	int		GetPeakValue() const {return m_nPeakValue;}
	DWORD	GetJitterErrors() const {return m_nJitterErrors;}
	WORD	GetJitterPosition() const {return m_nJitterPos;}
	CDEX_ERR RipChunk(BYTE* pbtStream,LONG* pNumBytes, BOOL& bAbort );
	CDEX_ERR RipChunkParanoia( BYTE* pbtStream, LONG* pNumBytes, BOOL& bAbort );
	CDEX_ERR RipChunkFastMode( BYTE* pbtStream, LONG* pNumBytes, BOOL& bAbort );
	BOOL	SetupTrackExtractFastMode(int nBeginSector,int nEndSector);

	INT		GetPercentCompleted() const {return m_nPercentCompleted;}
	void	SetPercentCompleted(int nValue) {m_nPercentCompleted=nValue;}
	void	GetLastJitterErrorPosition(DWORD& dwStartSector,DWORD& dwEndSector);

	void	FlushCache( DWORD dwCurrentSector );

	void	ReadChunk(	const long	nSectorsToRead,
						const long	lOverlapSamples,
						const BOOL	bJitterCorr,
						const DWORD	dwBytesRead,
						BOOL&		bJitterError,
						DWORD&		dwSkipBytes,
						PBYTE		pbtReadBuffer,
						DWORD		dwReadBufferSize );

	INT		GetCurrentSpeed() const { return m_nCurrentSpeed; }
	void	SetCurrentSpeed( INT nValue ) { m_nCurrentSpeed = nValue; }

	INT		GetLastSectorSpeedAdjusted( ) const { return m_nLastSpeedAdjust; }
	void	GetLastSectorSpeedAdjusted( INT nValue ) { m_nLastSpeedAdjust = nValue; }

	LONG	GetCurrentSector() const { return m_lSector;}
	int		GetBeginSector() const {return m_nBeginSector;}
	int		GetEndSector() const {return m_nEndSector;}

	BOOL	GetAbort() const { return ( m_pbAbort ) ? *m_pbAbort: FALSE; }

	DWORD	GetReadBufferSize() const { return m_dwReadBufferSize;}
	// AF STUBBED
//	BOOL	IsC2ErrorDetectionEnabled() const { return m_bC2ErrorDetectionEnabled; }
	BOOL	IsC2ErrorDetectionEnabled() const { return FALSE; }
	virtual CDEX_ERR Init();
	void	CheckFuaBit();

    ULONG   GetCRC() const;

    void    InitCRC( BOOL bExcludeNullSamplesInCRC );

	static void paranoiaCallback( long, int );
    CDEX_ERR ReadBlockDirect( BYTE* pbtStream,DWORD numBytes, LONG sector, DWORD numSectors );


};



#endif

