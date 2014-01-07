/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2002 Johan Pouwelse
** Copyright (C) 2002 Albert L. Faber
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


#ifndef CDEX_MBRAINZ_H_INCLUDED
#define CDEX_MBRAINZ_H_INCLUDED

#include "MusicBrainz/mb_c.h"

// Foreward class declaration of CDInfo
class CDInfo;


// CMusicBrainz is a class to load/get or save/put CD related information
// It is the second generation of the CDDB/FreeDB music Encyclopedia,
// Uses the distributed http://MusicBrainz.org server network.


class CMusicBrainz
{
public:

	// CONSTRUCTOR
	CMusicBrainz( CDInfo* pCDInfo );

	// DESCTRUCTOR
	~CMusicBrainz();

	// Method to get server information
	BOOL CMusicBrainz::GetInfoFromServer();

private:
	//member function;
	BOOL CMusicBrainz::GenerateId(char DiscId[33]);
	BOOL CMusicBrainz::CopyAllResults();

	// The local pointer to the CDInfo class
	CDInfo*		      m_pCDInfo;

	// The main musicbrainz 'object'
	musicbrainz_t   m_pMusicBrainz;

	// URL
	char m_url[1025]; 
};

#endif
