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


#include "StdAfx.h"

#include "CTimeTrack.h"

void CTimeTrack::ReInit(DWORD dwTrackLength)
{
	m_dwTrackLength=dwTrackLength;
	m_dwStartTime=GetTickCount();
	m_dwElapsed=0;
	m_dwEstimated=0;
	m_dwRemaining=0;
	m_nPrevPercent=0;
}


CUString CTimeTrack::GetSpeedString()
{
	CUString strTmp;
	strTmp.Format( _W( "%4.1f" ), m_fSpeed);
	return strTmp;
}

CUString CTimeTrack::GetElapsedString()
{
	CUString strTmp;
	strTmp.Format( _W( "%02d:%02d" ), m_dwElapsed / 60, m_dwElapsed % 60 );
	return strTmp;
}

CUString CTimeTrack::GetEstimateString()
{
	CUString strTmp;
	strTmp.Format( _W( "%02d:%02d" ), m_dwEstimated / 60, m_dwEstimated % 60 );
	return strTmp;
}

CUString CTimeTrack::GetRemainingString()
{
	CUString strTmp;
	strTmp.Format( _W( "%02d:%02d" ), m_dwRemaining / 60, m_dwRemaining % 60 );
	return strTmp;
}

/*
void CTimeTrack::Calculate(int nPercent)
{
	m_dwElapsed=(GetTickCount()-m_dwStartTime)/1000;

	// Update every two percent
	if ((nPercent-m_nPrevPercent)>=3)
	{
		if (nPercent)
		{
			m_dwEstimated=(DWORD)max(0,m_dwElapsed*100.0/nPercent);
			m_nPrevPercent=nPercent;
		}
	}
	m_dwRemaining=(DWORD)max(0,(double)m_dwEstimated-(double)m_dwElapsed);
}
*/


void CTimeTrack::Calculate(int nPercent)
{
 	DWORD dwElapsed = GetTickCount() - m_dwStartTime;
 	m_dwElapsed = dwElapsed / 1000;

    if ( ( nPercent > 0 ) && ( dwElapsed > 0 )  ) 
    {
 	    if ( ( nPercent != m_nPrevPercent ) && ( m_dwElapsed >= 1 ) && ( nPercent > 0 ))
	    {
 		    m_dwEstimated = m_dwElapsed * 100 / nPercent;
 		    m_fSpeed = (float)( m_dwTrackLength * nPercent / (dwElapsed * 100.0) );
		    m_nPrevPercent = nPercent;
	    }
        if (m_dwEstimated != 0)
        {
            if ( ( (int)m_dwEstimated - (int)m_dwElapsed ) > 0 )
            {
    	        m_dwRemaining = m_dwEstimated - m_dwElapsed;
            }
            else
            {
    	        m_dwRemaining = 0;
            }
        }
    }

}
