/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2007 Georgy Berdyshev
** Copyright (C) 2007 Albert L. Faber
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

#include "TagData.h"
#include "config.h"
#include "resource.h"



INITTRACE( _T( "TagData" ) );


// CONSRUCTOR
CTagData::CTagData()
{
	ENTRY_TRACE( _T( "CTagData::CTagData()" ) );

    // Initialize the variables
	Init();

	EXIT_TRACE( _T( "CTagData::CTagData()" ) );
}

// Init CTagData variables
void CTagData::Init()
{
	ENTRY_TRACE( _T( "CTagData::Init()" ) );

	// Initialize all strings and variables
    m_strTitle		= _T( "" );
    m_strArtist		= _T( "" );
    m_strAlbum		= _T( "" );
    m_strYear		= _T( "" );
    m_strComment	= _T( "" );
	m_strEncodedBy  = _T( "" );
    m_strGenre		= _T( "" );
	m_strLengthInMs = _T( "" );
	m_dwCDBID		= 0;
	m_dwVOLID		= 0;
	m_dwCDBID       = 0;
	m_dwVOLID       = 0;
	m_dwTotalTracks = 0;
    m_lTrackOffset  = 0;
	memset( m_btaRawToc, 0, sizeof( m_btaRawToc ) );

    EXIT_TRACE( _T( "CTagData::Init()" ) );
}


