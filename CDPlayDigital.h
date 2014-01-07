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


#ifndef CDPLAYDIGITAL_H__INCLUDED
#define CDPLAYDIGITAL_H__INCLUDED


/////////////////////////////////////////////////////////////////////////////
// CCDPlayDigital dialog

class CCDPlayDigital : public CObject
{
private:
	volatile BOOL	m_bAbort;
	CWinThread*		m_pThread;
	DWORD			m_dwBeginSector;		;
	DWORD			m_dwEndSector;
	DWORD			m_dwCurrentSector;
	BOOL			m_bIsPlaying;

	DWORD			m_dwTotalBytes;
	DWORD			m_dwBytesWritten;
	DWORD			m_dwCurrentTime;
	DWORD			m_dwSeekTimeOffset;
	volatile BOOL	m_bPause;
public:
	// CONSTRUCTOR
	CCDPlayDigital();

	// DESTRUCTOR
	~CCDPlayDigital();

	BOOL		GetCurrentSector() const { return m_dwCurrentSector; }
	void		SetCurrentSector( DWORD dwValue ) { m_dwCurrentSector = dwValue; }

	BOOL		GetBeginSector() const { return m_dwBeginSector; }
	void		SetBeginSector( DWORD dwValue ) { m_dwBeginSector = dwValue; }

	BOOL		GetEndSector() const { return m_dwEndSector; }
	void		SetEndSector( DWORD dwValue ) { m_dwEndSector = dwValue; }
	
	void		Play( DWORD dwBeginSector, DWORD dwEndSector );
	void		Stop();
	void		Pause( BOOL bPause );
	DWORD		GetCurrentTimeInSecs( );

	BOOL		IsPlaying() const {return m_bIsPlaying;}
	static		UINT RipAndPlay(PVOID pParams);

private:
};

#endif
