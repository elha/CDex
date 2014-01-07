/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2004 Bradey Honsinger
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


#ifndef STATUSREPORTER_H_INCLUDED
#define STATUSREPORTER_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// CSocket
#include <Afxsock.h>

/////////////////////////////////////////////////////////////////////////////
// CStatusReporter:
// See StatusReporter.cpp for the implementation of this class
//

class CStatusReporter
{

public:
	CStatusReporter();
	~CStatusReporter();

	void Connect();
	void Disconnect();

	void AlbumStart( const CTaskInfo& taskInfo );
	void AlbumFinish( CDEX_ERR status, BOOL bCancelled );

	void TrackStart( const CTaskInfo& taskInfo );
	void TrackFinish( CDEX_ERR status, BOOL bCancelled, int njitterErrors );
	
	enum STALLREASON 
	{
        STALLREASON_CDDB_NO_HITS,
		STALLREASON_CDDB_MULTIPLE_HITS,
		STALLREASON_FILE_EXISTS
	};

	void Stalled( STALLREASON reason );

private:
	// Private methods
	CDEX_ERR GetHostname();
	
	CDEX_ERR OpenConnection();
	void CloseConnection();
	
	CDEX_ERR SendMessageType( CUString strMsgType );
	CDEX_ERR SendMessageValue( CUString strName, CUString strValue  );
	CDEX_ERR SendMessageEnd( );
	
	CDEX_ERR SendString( CUString strSend  );

//	CDEX_ERR SendStatusMessage( CUString strMessage );

	// Private data
	bool m_bConnected;
	CUString m_strHostname;
	CSocket m_Socket;

	CUString m_strArtist;
	CUString m_strAlbum;
	int m_nTracks;

	CUString m_strTrack;
	int m_nTrack;
	
	
/*
Album and Track Start/Finish Ripping message hooks in CCopyDialog::RipFunc 

Connect();
  @ CCDexApp::InitInstance 
  -> send machine name to status server 
Disconnect() 
  @ CCDexApp::ExitInstance (add entry/exit traces) 
  -> say goodbye, disconnect socket 
AlbumStart( RecordItems ) 
  @ CCopyDlg::RipFunc entry 
  -> album name, no. tracks 
TrackStart( RecordItems ) 
  @ CCopyDlg::RipFunc, top of while loop 
  -> track no, track name 
TrackDone( ) 
  @ CCopyDlg::RipFunc, bottom of while loop 
  -> jitter errors (if any) from m_nJitterErrors, other error (if any) from bErr, cancelled 
AlbumDone( ) 
  @ CCopyDlg::RipFunc exit 
  -> status (success, cancelled, other error) 
Stalled( ) 
  @ CDdb::NoHits (or whatever it's called)
  @ CDdb::HandleMultiple 
  @ Util:CheckNoFileOverwrite 
  @ others? 
  -> "stalled" message, reason for stall 
*/  
};

// Global status reporter instance
extern CStatusReporter g_statusReporter;

#endif
