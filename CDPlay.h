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


#ifndef CDPLAY_H__INCLUDED
#define CDPLAY_H__INCLUDED

#include "CDPlayDigital.h"

class CDInfo;

/////////////////////////////////////////////////////////////////////////////
// CCDPlay dialog

class CCDPlay : public CObject
{
private:
	BOOL			m_bPlayDigital;
	CCDPlayDigital	m_cPlayDigital;
	CDInfo*			m_pCDInfo;
	BOOL			m_bPausing;
	DWORD			m_dwBeginSector;
	DWORD			m_dwEndSector;
	DWORD			m_dwSeekOffset;

public:
	// CONSTRUCTOR
	CCDPlay();

	// DESTRUCTOR
	~CCDPlay();

	void		Play( DWORD dwBeginSector, DWORD dwEndSector );
	void		Stop();
	DWORD		GetCurrentTimeInSecs();
	BOOL		IsPlaying();
	void		Pause( BOOL bPause );
	BOOL		IsPausing() const { return 	m_bPausing;}
	DWORD		GetBeginSector() const { return m_dwBeginSector; }
	DWORD		GetEndSector() const { return m_dwEndSector; }
	void		SeekToSector( DWORD dwRelOffset );
	DWORD		GetPlaySectors() const {return m_dwEndSector - m_dwBeginSector;}
private:
};

#endif
