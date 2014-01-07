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


#ifndef TIMETRACK_INCLUDED
#define TIMETRACK_INCLUDED


class CTimeTrack
{
 	float   m_fSpeed;
 	DWORD   m_dwTrackLength;

	DWORD	m_dwStartTime;
	DWORD	m_dwElapsed;
	DWORD	m_dwEstimated;
	DWORD	m_dwRemaining;
	int		m_nPrevPercent;
public:
 	CTimeTrack(DWORD dwTrackLength=0)	{m_fSpeed=0.0f; ReInit(dwTrackLength);}
 	void ReInit(DWORD dwTrackLength);

 	CUString GetSpeedString();
	CUString GetElapsedString();
	CUString GetEstimateString();
	CUString GetRemainingString();
	void	Calculate(int nPercent);

};

#endif
