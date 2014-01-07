/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2001 Albert L. Faber
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


#ifndef CDEXUSERMSG_H_INCLUDED
#define CDEXUSERMSG_H_INCLUDED

#define CDEX_MSG_BASE				( WM_USER + 800 )

#define ID_STARTDETECTION			( CDEX_MSG_BASE +  1 )
#define ID_INITDETECTION			( CDEX_MSG_BASE +  2 )
#define WM_PLAYERCLOSED				( CDEX_MSG_BASE +  3 )
#define WM_EDITNEXTTRACK			( CDEX_MSG_BASE +  4 )
#define WM_UPDATETRACKSTATUS		( CDEX_MSG_BASE +  5 )
#define WM_CTRLCHANGED				( CDEX_MSG_BASE +  6 )
#define WM_UPDATE_GENRE_LIST		( CDEX_MSG_BASE +  7 )
#define WM_PLAYTAGCHANGED			( CDEX_MSG_BASE +  8 )
#define WM_PLAYTAGUPDCONF			( CDEX_MSG_BASE +  9 )

#define WM_SEEKBUTTONVALUECHANGED	( CDEX_MSG_BASE + 10 )
#define WM_SEEKBUTTONUP				( CDEX_MSG_BASE + 11 )

#define WM_SEEKSLIDERDOWN			( CDEX_MSG_BASE + 14 )
#define WM_SEEKSLIDERUP 			( CDEX_MSG_BASE + 15 )

#define WM_UPDATE_CDSELECTION		( CDEX_MSG_BASE + 16 )
#define WM_DIALOG_CLOSED			( CDEX_MSG_BASE + 17 )

#define WM_UPDATE_PROFILESELECTION	( CDEX_MSG_BASE + 18 )

#define WM_SCAN_ISRC                ( CDEX_MSG_BASE + 31 )


#define WM_CDDB_INFO_MSG			( CDEX_MSG_BASE + 50 )
#define WM_CDDB_ACTION_MSG			( CDEX_MSG_BASE + 51 )
#define WM_CDDB_REMOTE_FINISHED		( CDEX_MSG_BASE + 52 )
#define WM_CDDB_CLOSE_DIALOG		( CDEX_MSG_BASE + 53 )
#define WM_CDDB_RESPONSE_MSG        ( CDEX_MSG_BASE + 54 )

#define WM_CDEX_UPDATE_TRACKSTATUS	( CDEX_MSG_BASE + 100 )
#define WM_CDEX_INITIAL_UPDATE		( CDEX_MSG_BASE + 101 )
#define WM_CDEX_SAVE_CD_INFO		( CDEX_MSG_BASE + 102 )


#define WM_PLAYER_ADD_FILE			( CDEX_MSG_BASE + 200 )

#endif
