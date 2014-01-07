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


#include "StdAfx.h"
#include "CDPlay.h"
#include "config.h"
#include "CDPlayDigital.h"
#include "CDInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "CDPlay") );

CCDPlay::CCDPlay():
			CObject()
{
	ENTRY_TRACE( _T( "CCDPlay::CCDPlay" ) );

	m_bPlayDigital = TRUE;
	m_bPausing = FALSE;
	m_dwBeginSector = 0;
	m_dwEndSector = 0;
	m_dwSeekOffset = 0;

	EXIT_TRACE( _T( "CCDPlay::CCDPlay" ) );

}

CCDPlay::~CCDPlay()
{
	ENTRY_TRACE( _T( "CCDPlay::~CCDPlay" ) );
	EXIT_TRACE( _T( "CCDPlay::~CCDPlay" ) );
}



void CCDPlay::Play( DWORD dwBeginSector, DWORD dwEndSector )
{
	ENTRY_TRACE( _T( "CCDPlay::Play( %d, %d )" ), dwBeginSector, dwEndSector );

	m_bPlayDigital = g_config.GetCDPlayDigital();

	LTRACE( _T( "CCDPlay::Play( ) Play digital is %d" ), m_bPlayDigital );

	m_bPausing = FALSE;

	m_dwBeginSector = dwBeginSector;
	m_dwEndSector = dwEndSector;
	m_dwSeekOffset = 0;

	if ( m_bPlayDigital )
	{
		m_cPlayDigital.Play(	m_dwBeginSector,
								m_dwEndSector );
	}
	else
	{
		CR_PlaySection(	m_dwBeginSector,
						m_dwEndSector );
	}

	EXIT_TRACE( _T( "CCDPlay::Play( )" ) );
}

void CCDPlay::Stop( )
{
	ENTRY_TRACE( _T( "CCDPlay::Stop()" ) );

	if ( m_bPlayDigital )
	{
		m_cPlayDigital.Stop();
	}
	else
	{
		CR_PauseCD( FALSE );
		CR_StopPlayTrack();
	}

	EXIT_TRACE( _T( "CCDPlay::Stop()" ) );
}

BOOL CCDPlay::IsPlaying()
{
	BOOL bReturn = TRUE;

	LTRACE2( _T( "Entering CCDPlay::IsPlaying()" ) );

	if ( m_bPlayDigital )
	{
		bReturn = m_cPlayDigital.IsPlaying();
	}
	else
	{
		BYTE btAudioPlaying = CR_IsAudioPlaying();

		// Are we still playing
		if (( btAudioPlaying==0x13 || btAudioPlaying==0x14 || btAudioPlaying==0x15 ))
		{
			bReturn = FALSE;
		}
	}

	LTRACE2( _T( "Leaving CCDPlay::IsPlaying(), return value is %d" ), bReturn );

	return bReturn;
}

DWORD CCDPlay::GetCurrentTimeInSecs( )
{
	DWORD dwReturn = 0;

	LTRACE2( _T( "Entering CCDPlay::GetCurrentTimeInSecs()" ) );

	if ( m_bPlayDigital )
	{
		dwReturn = m_cPlayDigital.GetCurrentTimeInSecs();
	}
	else
	{
		// Get track position
		DWORD dwRelPos;
		DWORD dwAbsPos;

		CR_GetPlayPosition( dwRelPos, dwAbsPos );

		// Convert to MSF
		dwReturn = dwRelPos / TRACKSPERSEC;

	}

	dwReturn += m_dwSeekOffset / TRACKSPERSEC;

	LTRACE2( _T( "Leaving CCDPlay::GetCurrentTimeInSecs(), return value %d" ), dwReturn );

	return dwReturn;
}

void CCDPlay::Pause( BOOL bPause )
{
	ENTRY_TRACE( _T( "CCDPlay::Pause( %d )" ), bPause );

	m_bPausing = bPause;

	if ( m_bPlayDigital )
	{
		m_cPlayDigital.Pause( bPause );
	}
	else
	{

		CR_PauseCD( bPause );
	}

	EXIT_TRACE( _T( "CCDPlay::Pause()" ) );
}

void CCDPlay::SeekToSector( DWORD dwSeekSector )
{
	ENTRY_TRACE( _T( "CCDPlay::SeekToSector( %d )" ), dwSeekSector );

	if ( m_bPlayDigital )
	{
		m_dwSeekOffset = dwSeekSector;
		m_cPlayDigital.Stop();
		m_cPlayDigital.Play(	m_dwBeginSector + dwSeekSector,
								m_dwEndSector );
	}
	else
	{
		m_dwSeekOffset = 0;
		
//		CR_StopPlayTrack();
		CR_PlaySection(	m_dwBeginSector + dwSeekSector,
						m_dwEndSector );
	}

	EXIT_TRACE( _T( "CCDPlay::SeekToSector()" ) );
}

