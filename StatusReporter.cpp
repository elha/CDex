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


#include "stdafx.h"
#include "Util.h"
#include "TaskInfo.h"
#include "StatusReporter.h"

INITTRACE( _T( "StatusReporter" ) );

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Global status reporter instance
CStatusReporter g_statusReporter;

CStatusReporter::CStatusReporter():
	m_bConnected( FALSE ),
	m_strHostname(),
	m_Socket(),
	m_strArtist(),
	m_strAlbum(),
	m_nTracks( 0 ),
	m_strTrack(),
	m_nTrack( 0 )
{
};
	
CStatusReporter::~CStatusReporter()
{
	if ( m_bConnected )
	{
		LTRACE( _T( "CStatusReporter: unexpected destruction!" ) );
		ASSERT( FALSE );
		Disconnect();
	}
};

void CStatusReporter::Connect()
{
	if ( CDEX_OK != GetHostname() )
	{
		CDexMessageBox( _W( "Failed to get local host name!(CStatusReporter:Connect)" ) );
	};

	if ( CDEX_OK == OpenConnection() )
	{
 		m_bConnected = TRUE;
		SendMessageType( _W( "Connect" ) );
		SendMessageValue( _W( "Hostname" ), m_strHostname );
		SendMessageEnd();
	}
	else
	{
		CDexMessageBox( _W( "Failed to connect to status server!(CStatusReporter:Connect)" ) );
	}
};
void CStatusReporter::Disconnect()
{
	SendMessageType( _W( "Disconnect" ) );
	SendMessageEnd();

	CloseConnection();
	
	m_bConnected = FALSE;
};

void CStatusReporter::AlbumStart( const CTaskInfo& taskInfo )
{
    CTagData& tagData( ((CTaskInfo&)taskInfo).GetTagData() );

	m_strArtist = tagData.GetArtist( );
	m_strAlbum = tagData.GetAlbum( );
	m_nTracks = tagData.GetTotalTracks( );

	CUString strTracks;
	strTracks.Format( _W( "%d" ), m_nTracks );

	SendMessageType( _W( "AlbumStart" ) );
	SendMessageValue( _W( "Artist" ), m_strArtist );
	SendMessageValue( _W( "Album" ), m_strAlbum );
	SendMessageValue( _W( "Tracks" ), strTracks );
	SendMessageEnd();
};
void CStatusReporter::AlbumFinish( CDEX_ERR status, BOOL bCancelled )
{
	CUString strCompletion = _W( "OK" );
	if ( bCancelled )
	{
		strCompletion = _W( "Cancelled" );
	} 
	else if ( CDEX_OK != status ) 
	{
		strCompletion = _W( "Aborted" );
	}
	
	SendMessageType( _W( "AlbumFinish" ) );
	SendMessageValue( _W( "Completion" ), strCompletion );
	SendMessageEnd();

	m_strArtist = _W( "" );
	m_strAlbum = _W( "" );
	m_nTracks = 0;
};

void CStatusReporter::TrackStart( const CTaskInfo& taskInfo )
{
    CTagData& tagData( ((CTaskInfo&)taskInfo).GetTagData() );

	m_strTrack = tagData.GetTitle();

	m_nTrack = tagData.GetTrackNumber( );
	
    CUString strTrackNo;
	strTrackNo.Format( _W( "%d" ), m_nTrack );

	SendMessageType( _W( "TrackStart" ) );
	SendMessageValue( _W( "Name" ), m_strTrack );
	SendMessageValue( _W( "Number" ), strTrackNo );
	SendMessageEnd();
};
void CStatusReporter::TrackFinish( CDEX_ERR status, BOOL bCancelled, int njitterErrors )
{
	CUString strJitterErrors;
	strJitterErrors.Format( _W( "%d" ), njitterErrors );
	
	SendMessageType( _W( "TrackFinish" ) );
	SendMessageValue( _W( "JitterErrors" ), strJitterErrors );
	SendMessageEnd();

	m_strTrack = _W( "" );
	m_nTrack = 0;
};

void CStatusReporter::Stalled( STALLREASON reason )
{
	CUString strReason;
	switch ( reason )
	{
		case STALLREASON_CDDB_NO_HITS:
			strReason += _W("cddb no hits" );
			break;

		case STALLREASON_CDDB_MULTIPLE_HITS:
			strReason += _W( "cddb multiple hits" );
			break;

		case STALLREASON_FILE_EXISTS:
			strReason += _W( "file exists" );
			break;

		default:
			strReason += _W( "unknown reason" );
			ASSERT( FALSE );
			break;
	}

	SendMessageType( _W( "Stalled" ) );
	SendMessageValue( _W( "Reason" ), strReason );
	SendMessageEnd();
};


/*
	Private method implementations
*/

CDEX_ERR CStatusReporter::GetHostname()
{
	CDEX_ERR bReturn =	CDEX_OK;

	ENTRY_TRACE( _T( "CStatusReporter::GetHostname()" ) );

	TCHAR hostname[ MAX_COMPUTERNAME_LENGTH + 1 ];
	DWORD hostnameLength = sizeof( hostname ) / sizeof( TCHAR );

	if ( ::GetComputerName( hostname, &hostnameLength ) )
	{
		m_strHostname = hostname;
	}
	else
	{
		CUString strError = GetLastErrorString();
		LTRACE( _T( "Failed to get local hostname: %s" ), strError );
							
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CStatusReporter::GetHostname(), return value %d" ), bReturn );

	return bReturn;
}

CDEX_ERR CStatusReporter::OpenConnection()
{
	CDEX_ERR bReturn =	CDEX_OK;
	
	ENTRY_TRACE( _T( "CStatusReporter::OpenConnection()" ) );

	// Init the WINSOCK library
	if ( !AfxSocketInit() )
	{
		LTRACE( _T( "Failed to initialize WinSock library" ) );
		bReturn = CDEX_ERROR;
	}
	else
	{
		// Clear all errors
		WSASetLastError( 0 );
		
		// Set host and port
		CUString strServer = g_config.GetStatusServer();
		int nPort = g_config.GetStatusServerPort();
		
		// Create the CSocket
		if ( 0 == m_Socket.Create( ) )
		{
			LTRACE( _T( "Failed to create socket " ) );
			bReturn = CDEX_ERROR;
		}
		else
		{
			LTRACE( _T( "Open connection =\"%s\" Port=%d" ), strServer, nPort );

			// Try to connect
            CUStringConvert strCnv;
			if ( 0 == m_Socket.Connect( strCnv.ToT( strServer ), nPort )  )
			{
				CUString strError = GetLastErrorString();
				LTRACE( _T( "Failed to connect to host: %s" ), strError );
									
				bReturn  = CDEX_ERROR;
			}
		}
	}	
	
	EXIT_TRACE( _T( "CStatusReporter::OpenConnection(), return value %d" ), bReturn );

	return bReturn;
}

void CStatusReporter::CloseConnection()
{
	ENTRY_TRACE( _T( "CStatusReporter::CloseConnection()" ) );

	// Close socket
	m_Socket.Close();
	
	EXIT_TRACE( _T( "CStatusReporter::CloseConnection()" ) );
}

CDEX_ERR CStatusReporter::SendMessageType( CUString strMsgType  )
{
	return SendString( strMsgType + CUString( _W( "\r\n" ) ) );
};

CDEX_ERR CStatusReporter::SendMessageValue( CUString strName, CUString strValue  )
{
	return SendString( strName + CUString( _W( ": " ) ) + strValue + CUString( _W( "\r\n" ) ) );
};

CDEX_ERR CStatusReporter::SendMessageEnd( )
{
	return SendString( _W( "\r\n" ) );
};

CDEX_ERR CStatusReporter::SendString( CUString strSend  )
{
	CDEX_ERR bReturn = CDEX_OK;

	LTRACE( _T( "Sending: %s" ), strSend );

    CUStringConvert strCnv;

    if ( SOCKET_ERROR == m_Socket.Send( strCnv.ToT( strSend ), strSend.GetLength() ) )
	{
		CUString strError = GetLastErrorString();
		LTRACE( _T( "Failed to send string: %s" ), strError );

		bReturn = CDEX_ERROR;
	}

	return bReturn;
};

/*
CDEX_ERR CStatusReporter::SendStatusMessage( CUString strMessage )
{
	CDEX_ERR bReturn = CDEX_OK;

	if ( m_bConnected )
	{
		// host: artist - album - track (num/total): message
		CUString status;
		status.Format( _T( "%s: %s - %s -%s (%d/%d): %s\r\n" ), 
				(LPCWSTR)m_strHostname, 
				(LPCWSTR)m_strArtist, 
                (LPCWSTR)m_strAlbum, 
                (LPCWSTR)m_strTrack, 
				m_nTrack, m_nTracks,
			strMessage 
			);

		bReturn = SendString( status );
	}
	else
	{
		LTRACE( _T( "Tried to send string when not connected: %s" ), strMessage );

		bReturn = CDEX_ERROR;
	}

	return bReturn;
};
*/
