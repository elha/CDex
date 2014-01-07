/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2006 Albert L. Faber
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


#ifndef RIPINFODB_INCLUDED
#define RIPINFODB_INCLUDED

#include "Config.h"

class CRipInfoDB : public CObject
{
private:
	FILE*	m_pFile;
	int		m_nTrack;
	CUString	m_strFileName;

public:
	CRipInfoDB();
	~CRipInfoDB();
	void SetFileName( CUString strFileName );
	void DeleteTrack( int nTrack );
	void SetCurrentTrack( int nTrack );
	CDEX_ERR GetStatusInfo( int nTrack, DWORD& dwJitterErrors );
	void SetRipInfo(CUString strRipInfo);
	void SetRipInfoFinshedOK(CUString strRipInfo, ULONG crc);
	CUString GetRipInfo( int nTrack );
	void SetJitterError( int nSectorStart, int nSectorEnd, int nTrackSectorBegin );
	void SetAbortError( CUString strAbortInfo );
	void RemoveAll();
	void Init();
	BOOL GetCRC(int nTrack, ULONG& crc);
};


#endif