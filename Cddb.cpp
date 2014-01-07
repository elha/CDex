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


#include "stdafx.h"
#include "Cddb.h"
#include "CdInfo.h"
#include "CddbMatch.h"
#include "resource.h"
#include <AfxSock.h>
#include "ID3Tag.h"

//#include "urarfilelib\urarlib.h"

/** 
 * define the maximum CDDB line length
 */
const int MAX_CDDB_LINE = 256;
const int HTTP_DEFAULT_PORT = 80;

CWnd*	CDdb::m_pWnd = NULL;
CUString	CDdb::m_strCDDBAction = _T( "" );
CUString	CDdb::m_strInfoMsg = _T( "" );
CMutex	CDdb::m_mCDDBAction;
CMutex	CDdb::m_mInfoMsg;


INITTRACE( _T( "CDDB" ) );

static const CUString FILENAMETAG( _T( "#FILENAME=" ) );
static const CUString DTITLETAG( _T( "DTITLE=" ) );
static const CUString EXTDTAG( _T( "EXTD=" ) );
static const CUString TITLETAG( _T( "TTITLE" ) );
static const CUString EXTTTAG( _T( "EXTT" ) );
static const CUString DGENRETAG( _T( "DGENRE" ) );
static const CUString DYEARTAG( _T( "DYEAR" ) );

static const CUString REVISIONTAG( _T( "# Revision:" ) );
static const CUString SUBMITVIATAG( _T( "# Submitted via:" ) );


#define HTTP_SUBMIT_CGI	_T( "/~cddb/submit.cgi" )

#ifdef _DEBUG
	// #define DEBUG_LOCAL
	// #define NO_CONNECT
	// #define HTTP_SUBMIT_CGI 			"/cgi-bin/newsubmit.cgi"
#endif

static int gs_bProtoLevel = 6;

const CUString strCDDBCatagories[] =
{
	_T( "blues" ),
	_T( "classical" ),
	_T( "country" ),
	_T( "folk" ),
	_T( "jazz" ),
	_T( "newage" ),
	_T( "reggae" ),
	_T( "rock" ),
	_T( "soundtrack" ),
	_T( "data" ),
	_T( "misc" )
};

const CUString CDDBCatToID3[] =
{
	_T( "Blues" ),
	_T( "Classical" ),
	_T( "Country" ),
	_T( "Folk" ),
	_T( "Jazz" ),
	_T( "New Age" ),
	_T( "Reggae" ),
	_T( "Rock" ),
	_T( "Soundtrack" ),
	_T( "Data" ),
	_T( "Other" )
};


#define NumCategories (sizeof(strCDDBCatagories)/sizeof(strCDDBCatagories[0]))


// CONSTRUCTOR
CDdb::CDdb( CDInfo*	pCDInfo, CWnd* pWnd ) 
: m_strSubmitFileName( g_config.GetCDDBPath() +  _W( "\\SUBMIT.TXT" ))
{
	m_pWnd = pWnd;
    m_lastResponseCode = -1;

    //	m_pSockFile = NULL;
	m_pCDInfo = pCDInfo;

	if (pCDInfo)
	{
		m_pCDInfo=pCDInfo;
		m_strDiscID.Format( _W( "%08lx" ), m_pCDInfo->GetDiscID() );
		m_strLocalPath=g_config.GetCDDBPath();
	}

    CUStringConvert strCnv;

	m_bInexact = FALSE;

	m_bIsUnixCDDBType = ( UNIXTYPECDDB == g_config.GetLocalCDDBType() ) ;

	m_bUTF = FALSE;
}


// DESTRUCTOR
CDdb::~CDdb()
{
	// Close connection if necessary
	// CloseConnection();
}


/////////////////////////////////////////////////////////////
//
// Convert a string to HTTP CGI format
//
/////////////////////////////////////////////////////////////
void ConvertSpace(CUString& strConvert)
{
    for (int i=0;i<strConvert.GetLength();i++)
	{
        if ( strConvert.GetAt( i ) == _T( ' ' ) )
		{
            strConvert.SetAt( i, _T( '+' ) );
		}
    }
}


int CDdb::GetString( CUString& strRet)
{
	CUString strLang;
	int		nTimeOut=g_config.GetCDDBTimeOut()*1000;
	int		nTime=0;
	DWORD	nRead=0;

	vector<CHAR> vChars;

	// Clear return value
	strRet = _T( "" );

	while( nTime < nTimeOut )
	{
		if( m_Socket.IOCtl( FIONREAD, &nRead ) && nRead )
		{
			while(nRead --)
			{
				char ch;
				m_Socket.Receive( &ch, 1 );

				if ( ch == _T( '\n' ) )
				{
                    m_vReadResult.push_back( '\n' );

					vChars.push_back( _T( '\0' ) );

					if ( m_bUTF )
					{
						strRet = CUString( &vChars[0], CP_UTF8);
					}
					else
					{
						strRet = CUString( &vChars[0], CP_ACP );
					}

					LTRACE( _T( "GetString \"%s\"" ), strRet );
                    return 0;
				}
				else
				{
					if ( ch != '\r' )
					{
						vChars.push_back( ch );
                        m_vReadResult.push_back( ch );
					}
				}
			}
		}
		else 
		{
			::Sleep( 100 );
			nTime+=100;
		}
    }

	strLang = g_language.GetString( IDS_CDDB_REMOTE_TIMEOUT );
	CDexMessageBox( strLang );
	return SOCKET_ERROR;
}



BOOL CDdb::InitWinSock()
{
	if ( !AfxSocketInit() )
	{
		CDexMessageBox( IDP_SOCKETS_INIT_FAILED );
		return FALSE;
	}
	return TRUE;
}



BOOL CDdb::GetMultipleMatches( CUString& strMatches )
{
	BOOL bReturn = FALSE;
	int nNumMatches=0;

    m_strReceive = _T( "" );

	m_bInexact=TRUE;

	// CCddbMatch	dlg;
	// Get all match strings
	do
	{
		// Get next string
		if ( SOCKET_ERROR == GetString( m_strReceive ) )
			return FALSE;

		// Add Match to dialog
		// dlg.AddMatch(m_strReceive);
        strMatches += m_strReceive + _W( "\n" );

    	bReturn = FALSE;

		nNumMatches++;

	} while ( m_strReceive[0] != _T( '.' ) );


	//// Only pop dialog when there are multiple matches
	//if ( nNumMatches > 1 )
	//{
	//	// Popup dialog and make selection
	//	if ( IDOK == dlg.DoModal() )
	//	{
	//		// Get selected entry
	//		m_strReadCat = dlg.GetMatchCat();
	//		m_strReadID = dlg.GetMatchDiskID();
	//	}
	//	else
	//	{
	//		bStop = TRUE;
	//	}
	//}
    // strMatches =
	return bReturn;
}


CDDB_ERROR CDdb::CheckServerResponse( CUString strCheck, BOOL bQuery )
{
    CDDB_ERROR returnValue = CDDB_ERROR_OK;
    m_lastResponseCode = _wtoi( strCheck.Left( 3 ) );
	
    m_lastResponseString = _T( "" );

    // Check response
	switch ( m_lastResponseCode )
	{
		case 200: 
            m_strLastResponseString = g_language.GetString( IDS_CDDB_READWRITE_ALLOWED );
		break;
		case 201: 
			m_strLastResponseString = g_language.GetString( IDS_CDDB_OK_READONLY );
		break;
		case 202: 
			m_strLastResponseString = g_language.GetString( IDS_CDDB_NO_MATCH_FOUND );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 210:
			if ( bQuery == FALSE )
			{
				m_strLastResponseString = g_language.GetString( IDS_CDDB_OK_RESPONSE );
			}
			else 
			{
				m_strLastResponseString = g_language.GetString( IDS_CDDB_FOUND_MULTIPLE_EXACT_MATCHES );
			}
			break;
		case 211:
			m_strLastResponseString = g_language.GetString( IDS_CDDB_FOUND_MULTIPLE_INEXACT_MATCHES );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 401:
			m_strLastResponseString = g_language.GetString( IDS_CDDB_ENTRY_NOT_FOUND );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 402: 
			m_strLastResponseString = g_language.GetString( IDS_CDDB_ALREADY_SHOOK_HANDS );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 403:
			m_strLastResponseString = g_language.GetString( IDS_CDDB_DB_ENTRY_CORRUPT );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 409: 
			m_strLastResponseString = g_language.GetString( IDS_CDDB_NO_HANDSHAKE );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 431: 
			m_strLastResponseString = g_language.GetString( IDS_CDDB_INVALID_HANDSHAKE );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 432: 		
			m_strLastResponseString = g_language.GetString( IDS_NO_CONNECTION_ALLOWED );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 433: 
			m_strLastResponseString = g_language.GetString( IDS_NO_CONNECTION_ALLOWED );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 434: 
			m_strLastResponseString = g_language.GetString( IDS_NO_CONNECTION_LOAD_TOO_HIGH );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 501: 
			m_strLastResponseString = g_language.GetString( IDS_CDDB_ILLEGAL_PROTO_LEVEL );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;
		case 502: 
			m_strLastResponseString = g_language.GetString( IDS_CDDB_PROTOCOL_IS_CURRENT_LEVEL );
        	returnValue = CDDB_ERROR_RESPONSE;
		break;

		default:
        	returnValue = CDDB_ERROR_RESPONSE;
			m_strLastResponseString = g_language.GetString( IDS_CDDB_SERVER_ERROR );
	}

	return returnValue;
}


CDDB_ERROR CDdb::OpenConnection(CUString strAddress,int nPort)
{
	CDDB_ERROR returnValue = CDDB_ERROR_OK;
    CUString strURL( _T( "" ) );
    int nSocketPort = 0;

	ENTRY_TRACE( _T( "CDdb::OpenConnection( %s, %d )" ), strAddress, nPort );

	// Init the WINSOCK library
	if ( !InitWinSock() )
    {
        returnValue = CDDB_ERROR_SOCKET;
    }
    
    if  ( returnValue == CDDB_ERROR_OK )
    {
	    // Clear all errors
	    WSASetLastError( 0 );

	    // Set URL string
	    strURL = g_RemoteSites.GetAddress();

	    // Set port number
	    nSocketPort = g_RemoteSites.GetPort();

	    if ( !strAddress.IsEmpty() )
	    {
		    strURL = strAddress;
		    nSocketPort = nPort;
	    }

	    if ( g_RemoteSites.GetProtocol() == OPTIONS_CDDB_USEHTTP )
	    {
		    // Get user port, default HTTP port
		    nSocketPort = HTTP_DEFAULT_PORT;

		    // Do we have to go trough a proxy server ?
		    if ( g_config.GetCDDBUseProxy() )
		    {
			    // Get proxy port
			    nSocketPort= g_config.GetCDDBProxyPort();

			    // Set URL to proxy address instead of remote server address
			    strURL = g_config.GetProxyAddress();
		    }
	    }

	    strURL.TrimLeft();
	    strURL.TrimRight();

	    // Create the CSocket
	    if (  0 == m_Socket.Create( ) )
	    {
		    LTRACE( _T( "Failed to create socket " ) );
            returnValue = CDDB_ERROR_SOCKET;
	    }
    }

    if  ( returnValue == CDDB_ERROR_OK )
	{
		// Debug output
		LTRACE( _T( "Open connection =\"%s\" Port=%d" ), strURL,nSocketPort );

        CUStringConvert strCnv;

		// Try to conect (non blocking )
		if ( 0 == m_Socket.Connect( strCnv.ToT( strURL ), nSocketPort )  )
		{
			CUString strTmp;
			CUString strError;

			strError = GetLastErrorString();

			CUString strLang = g_language.GetString( IDS_CDDB_CONNECTION_FAILED_FOR_REASON );
			strTmp.Format( _W( "%s\r\n\"%s\"\r\n" ), (LPCWSTR)strLang, (LPCWSTR)strError );
			SetInfoMsg( strTmp );
			returnValue  = CDDB_ERROR_CONNECT;
		}
        else
        {
			// Keep connection alive, value =0 is not keeping connection alive
			int iAlive=1;		 
			m_Socket.SetSockOpt( SO_KEEPALIVE,&iAlive, sizeof( BOOL ) );

			int iRecBufSize=sizeof( m_lpszBuffer );

			// Set receive buffer size to 8K
			m_Socket.SetSockOpt( SO_RCVBUF, &iRecBufSize, sizeof( int ) );
		}		
	}


	if ( CDDB_ERROR_OK == returnValue )
    {
	    if ( g_RemoteSites.GetProtocol() != OPTIONS_CDDB_USEHTTP )
	    {
		    // Get first response of server
		    //ReceiveMessage();
		    if ( GetString( m_strReceive ) == SOCKET_ERROR )
            {
			    returnValue = CDDB_ERROR_SOCKET;
            }

		    // Check response
		    returnValue = CheckServerResponse( m_strReceive, FALSE );
	    }

	}

	EXIT_TRACE( _T( "CDdb::OpenConnection(), return value %d" ), returnValue );

	return returnValue;
}


CDDB_ERROR CDdb::CloseConnection()
{
	// Debug output
	LTRACE( _T( "Close connection" ) );
#ifndef NO_CONNECT 

	// Close socket
    m_Socket.Close();
#endif
	// It is okay
	return CDDB_ERROR_OK;
}


void CDdb::EncodeEmailAddress( CUString& strEmailAddress )
{
/*
	int nIndex = 0;
	CUString strEscapeChars( _T( ";/?:&=+$," ) );

	while ( nIndex < strEmailAddress.GetLength() )
	{
		if ( strEscapeChars.Find( strEmailAddress.GetAt( nIndex ) ) >= 0 )
		{
			strEmailAddress.Format( _T("%s%%%02x%s"), strEmailAddress.Left( nIndex ), (int)strEmailAddress.GetAt( nIndex ), strEmailAddress.Mid( nIndex + 1) );
		}
		nIndex++;
	}
*/
}

CUString CDdb::CreateHttpSendString( CUString strSend )
{
	CUString strRet;

	CUString strEmailAddress( g_config.GetEmailAddress() );

	ENTRY_TRACE( _T( "CDdb::CreateHttpSendString( %s )" ), strSend );

	EncodeEmailAddress( strEmailAddress );

	// Search for @
	int nPos = strEmailAddress.Find( _T( '@' ) );

	if (nPos>0)
	{
		strEmailAddress.SetAt( nPos, _T( '+' ) );
	}

	strRet.Format( _W( "%s?cmd=%s&hello=%s+cdex+%s&%s%d" ), (LPCWSTR)g_RemoteSites.GetPath(), (LPCWSTR)strSend, (LPCWSTR)strEmailAddress, (LPCWSTR)GetClientVersion(), _W( "proto=" ), gs_bProtoLevel );

	ENTRY_TRACE( _T( "CDdb::CreateHttpSendString( ) returns \"%s\"" ), strRet );

	return strRet;
}

CDDB_ERROR CDdb::SendHello()
{
	CUString strEmailAddress( g_config.GetEmailAddress() );

	// Search for @
	int nPos = strEmailAddress.Find( _T( '@' ) );
	if (nPos>0)
	{
		if ( OPTIONS_CDDB_USEHTTP == g_RemoteSites.GetProtocol() )
		{
			strEmailAddress.SetAt( nPos, _T( '+' ) );
		}
		else
		{
			strEmailAddress.SetAt( nPos, _T( ' ' ) );
		}
	}
	else
	{
		return CDDB_ERROR_INVALID_EMAIL_ADDRESS;
	}

	if ( OPTIONS_CDDB_USEHTTP == g_RemoteSites.GetProtocol() )
	{
		// For HTTP protocol, don't send hello string, just create hello string
		return CDDB_ERROR_OK;
	}
	else
	{
		// Prepare hello string
		CUString strSend;
		strSend.Format( _W( "cddb hello %s cdex %s" ), (LPCWSTR)strEmailAddress, (LPCWSTR)GetClientVersion() );

		// Send string to remote server
		SendRemoteMessage( strSend, FALSE );
	}
	return CDDB_ERROR_OK;
}


CDDB_ERROR CDdb::SendProto()
{
    CDDB_ERROR returnValue = CDDB_ERROR_OK;

	CUString strLang;

	CUString strEmailAddress(g_config.GetEmailAddress());

	EncodeEmailAddress( strEmailAddress );

	if (g_RemoteSites.GetProtocol() != OPTIONS_CDDB_USEHTTP)
	{
		CUString strSend;
		strSend.Format( _W( "proto %d" ), gs_bProtoLevel );
		returnValue = SendRemoteMessage( strSend, FALSE );
	}

	return returnValue;
}


BOOL CDdb::SendStat()
{
	CUString strLang;

	CUString strEmailAddress( g_config.GetEmailAddress() );

	EncodeEmailAddress( strEmailAddress );

	CUString strSend( _T( "stat" ) );

	// Send string to remote server
	if ( SendRemoteMessage( strSend, FALSE ) == SOCKET_ERROR )
		return FALSE;

	// Get the query result until termination character
	int nError =0 ;
	do
	{
		// Get repsonse
		nError=GetString(m_strReceive);

		// If OK, add to receive buffer
		if (nError!=SOCKET_ERROR)
		{
			SetInfoMsg( m_strReceive + _W( "\r\n" ) );
		}

	} while (nError!=SOCKET_ERROR && m_strReceive[0] != _T( '.' ) );


	return TRUE;
}


int CDdb::SendString( CUString strSend, BOOL bAppendLineFeed )
{
	int bReturn = CDEX_OK;

	if ( bAppendLineFeed )
	{
		strSend += _W( "\r\n" );
	}

//    OutputDebugString( (LPCWSTR)strSend );

	LTRACE( _T( "Sending: %s" ), strSend );

	// Send the data
    CUStringConvert strCnv;
    LPCSTR lpszUTF = strCnv.ToUTF8( strSend );
	if (m_Socket.Send( lpszUTF, strlen( lpszUTF ) )==SOCKET_ERROR )
	{
		bReturn = SOCKET_ERROR;
	}

	return bReturn;
}

int CDdb::SendString_UTF( CHAR* strSend, BOOL bAppendLineFeed )
{
	int bReturn = CDEX_OK;

    CUStringConvert strCnv; 

	ENTRY_TRACE( _T( "SendString_UTF( \"%s\", %d" ), strSend, bAppendLineFeed );

	if ( bAppendLineFeed )
	{
		strcat( strSend, "\r\n"  );
	}

	// Send the data
	if (m_Socket.Send( strSend, strlen( strSend ) )==SOCKET_ERROR )
	{
		bReturn  = SOCKET_ERROR;
	}

	EXIT_TRACE( _T( "SendString_UTF() returns %d" ), bReturn );

	return bReturn;
}

CDDB_ERROR CDdb::SendRemoteMessage( CUString& strSend, BOOL bQuery )
{
    CDDB_ERROR returnValue = CDDB_ERROR_OK;
	CUString strLang;

	m_bUTF = FALSE;

	if ( g_RemoteSites.GetProtocol() == OPTIONS_CDDB_USEHTTP )
	{
		// Close previous connection
		CloseConnection();

		// Open new connection
        returnValue = OpenConnection();

		if ( CDDB_ERROR_OK == returnValue )
		{
			int i=0;
			// encode spaces and todo
			// replace spaces with + for string to send
			for (i=0;i<strSend.GetLength();i++)
			{
				if ( _T( ' ' ) == strSend.GetAt(i) )
				{
					strSend.SetAt( i, _T( '+' ) );
				}
			}

			for (i=0;i<strSend.GetLength();i++)
			{
				if ( _T( ' ' ) == strSend.GetAt(i) )
				{
					strSend.SetAt( i, _T( '+' ) );
				}
			}

			CUString strHTTPSend;

			// create send string
			strHTTPSend = CreateHttpSendString( strSend );

			if (g_config.GetCDDBUseProxy())
			{
				strHTTPSend =	_W( "GET http://" ) + 
								g_RemoteSites.GetAddress() +
								strHTTPSend;
			}
			else
			{
				strHTTPSend= _W( "GET " ) + strHTTPSend;
			}


			// Add HTTP protocol string
            strHTTPSend += _W( " HTTP/1.0" );

			// And send HTTP command
			if ( SendString(strHTTPSend) == SOCKET_ERROR )
            {
                returnValue = CDDB_ERROR_SOCKET;
            }

            if ( CDDB_ERROR_OK == returnValue )
            {
			    // Send host header to support virtual hosts
			    if ( SOCKET_ERROR == SendString( _W( "Host: " ) + g_RemoteSites.GetAddress() ) )
                {
                    returnValue = CDDB_ERROR_SOCKET;
                }
            }

            if ( CDDB_ERROR_OK == returnValue )
            {
			    // send HTTP-User Agent (part of HTTP Header, see RFC822 spec
			    if ( SOCKET_ERROR == SendString( _W( "User-Agent: CDex/" ) + GetClientVersion() ) )
                {
                    returnValue = CDDB_ERROR_SOCKET;
                }
            }

            if ( CDDB_ERROR_OK == returnValue )
            {
			    // Send authentication if necessary
			    if (g_config.GetCDDBUseAuthentication())
                {
				    if (SendAuthentication()==SOCKET_ERROR)
                    {
                        returnValue = CDDB_ERROR_SOCKET;
                    }
                }
            }

            if ( CDDB_ERROR_OK == returnValue )
            {
			    // Flush data 
			    if (SendString( _W("") )==SOCKET_ERROR)
                {
                    returnValue = CDDB_ERROR_SOCKET;
                }
            }


            if ( CDDB_ERROR_OK == returnValue )
            {
			    // Get Server response
			    if (GetString(m_strReceive)==SOCKET_ERROR)
                {
                    returnValue = CDDB_ERROR_SOCKET;
                }
            }

            if ( CDDB_ERROR_OK == returnValue )
            {
			    // Check if HTTP server is responding correctly
			    if ( ( m_strReceive.Find( _W( "HTTP/" ) ) !=0 ) || 
					    ( m_strReceive.Find( _W( "200" ) ) != 9 ) )
			    {
				    // Guess not
				    strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );
				    SetInfoMsg( strLang + m_strReceive+ _W( "\r\n" ) );
				    return CDDB_ERROR_SOCKET;
			    }
			    else
			    {
				    // Get all response from HTTP server
				    do
				    {
					    int nPos = -1;

					    if ( SOCKET_ERROR ==GetString(m_strReceive) )
					    {
						    return CDDB_ERROR_SOCKET;
					    }

					    nPos = m_strReceive.Find( _W( "Content-Type:" ) );

					    // Check for UTF-8 character set
					    if ( nPos >= 0  )
					    {
						    if ( m_strReceive.Find( _W( "UTF-8" ) ) >= 0 )
						    {
							    m_bUTF = TRUE;
						    }
					    }

					    SetInfoMsg( m_strReceive + _W( "\r\n" ) );

				    } while(m_strReceive.GetLength()> 0);
			    }
            }
		}
	}
	else
	{
		// Use CDDB protocol, it is easy, just send the string
		if (SendString(strSend)==SOCKET_ERROR)
			return CDDB_ERROR_SOCKET;
	}

	// Get the CDDB server response
	if (GetString(m_strReceive)==SOCKET_ERROR)
		return CDDB_ERROR_SOCKET;


	// Check if server responce is OK
	returnValue = CheckServerResponse( m_strReceive, bQuery );

	// Everything went well, indicate so
	return returnValue;
}



CDDB_ERROR CDdb::SendQuery( CUString& strReadCat )
{
    CDDB_ERROR returnValue = CDDB_ERROR_FAILED;
	CUString strSend;
	CUString strTmp;

    // be sure the correct DiscID is used
    m_strDiscID.Format( _W( "%08lx" ), m_pCDInfo->GetDiscID() );

	strSend.Format( _W( "cddb query %08lx %d" ), m_pCDInfo->GetDiscID(), m_pCDInfo->GetNumTracks() );

	//	Fill Query string with start sectors
	for ( int i=0; i < m_pCDInfo->GetNumTracks(); i++ )
	{
		strTmp.Format( _W( " %d" ), m_pCDInfo->GetStartSector( i + 1 ) + 150 );
		strSend += strTmp;
	}

	// Add the total number of seconds to it
	strTmp.Format( _W( " %d" ), m_pCDInfo->GetTotalSecs() );
	strSend+=strTmp;


#ifdef _DEBUG
	{
		DWORD dwDiscID;
		CUString strQuery;
		if ( CheckForTestQuery( dwDiscID, strQuery ) )
		{
			strSend = strQuery;
		}
	}
#endif

	// Send Query string
	if ( SendRemoteMessage( strSend, TRUE ) == SOCKET_ERROR )
    {
        switch ( m_lastResponseCode )
        {
            case 210: 
            case 211: 
                break;
            default:
                returnValue = CDDB_ERROR_FAILED;

        }

    }

    if ( m_lastResponseCode == 210 || m_lastResponseCode == 211 )
    {
        returnValue = CDDB_ERROR_INEXACT_MATCH;

        if  ( 0 != GetMultipleMatches( strReadCat ) )
        {
            returnValue = CDDB_ERROR_NO_MATCH;
        }
    }
    else if ( m_lastResponseCode == 202 )
    {
        returnValue = CDDB_ERROR_NO_MATCH;
    }
    else
    {
		int nPos = -1;

		// Get category string
		strReadCat = CUString( m_strReceive );

        // Now get the category, strip return code + space
		strReadCat= strReadCat.Mid( 4 );

		// get category string
		nPos = strReadCat.Find( _W( " " ) );

		if ( nPos > 0 )
		{
			strReadCat=strReadCat.Left( nPos );
		}

		strReadCat.MakeLower();
		strReadCat.TrimLeft();
		strReadCat.TrimRight();

		LTRACE( _T( "Category in query set to \"%s\"" ), strReadCat );

        returnValue = CDDB_ERROR_OK;
	}

	strSend.Format( _W( "cddb query %08lx %d" ), m_pCDInfo->GetDiscID(), m_pCDInfo->GetNumTracks() );

	return returnValue;
}


BOOL CDdb::CheckForTestQuery( DWORD& dwDiscID, CUString& strQuery )
{
	BOOL bReturn = FALSE;

#ifdef _DEBUG
	FILE* pFileTest = CDexOpenFile( _W( "c:\\temp\\cddb_test.txt" ), _W( "r" ) );

	if ( pFileTest )
	{
		TCHAR tmp[ 255];
		int numTracks = 0;


		_fgetts( tmp, sizeof( tmp ), pFileTest );

		_stscanf( tmp, _T( "%lx" ), &dwDiscID );

		_fgetts( tmp, sizeof( tmp ), pFileTest );
		_stscanf( tmp, _T( "%d" ), &numTracks );

		strQuery.Format( _W( "cddb query %08lx %d" ), dwDiscID , numTracks );

		// time stamps
		_fgetts( tmp, sizeof( tmp ), pFileTest );

		if ( '\n' == tmp[ _tcslen( tmp ) -1 ] )
		{
			tmp[ _tcslen( tmp ) -1 ] = '\0';
		}
		
		
		strQuery += _W( " " );
        strQuery += CUString( tmp );

		// total time
		_fgetts( tmp, sizeof( tmp ), pFileTest );
		if ( '\n' == tmp[ _tcslen( tmp ) -1 ] )
		{
			tmp[ _tcslen( tmp ) -1 ] = '\0';
		}		
		
		strQuery += _W( " " );
        strQuery += CUString( tmp );

        fclose( pFileTest );

		bReturn = TRUE;
	}
#endif
	return bReturn;
}



BOOL CDdb::StoreToCDDBBatch( const CUString& strLine )
{
    CUStringConvert strCnv;
	BOOL bReturn = FALSE;

	ENTRY_TRACE( _T( "CDdb::StoreToCDDBBatch( %s )" ), strCnv.ToT( strLine ) );

	// Open entry file
	FILE* fp = CDexOpenFile( CDDB_BATCH_FNAME , _W( "a+" ) );

	if ( NULL != fp )
	{
        fprintf( fp, "%s\n", strCnv.ToUTF8( strLine ) );
		fclose( fp );
	}
	else
	{
		bReturn = FALSE;
	}

	EXIT_TRACE( _T( "CDdb::StoreToCDDBBatch( ), return value %d" ), bReturn );

	return bReturn;
}

BOOL CDdb::RemoveFromCDDBBatch( const CUString& strLine )
{
	BOOL	bReturn = TRUE;
	TCHAR	lpszQuery[ 2048 ] = { _T( '\0' ),};
	FILE*	fp_tmp = NULL;
	FILE*	fp = NULL;

	ENTRY_TRACE( _T( "CDdb::RemoveFromCDDBBatch( %s )" ), strLine );

	// Open entry file for write
	fp = CDexOpenFile( CDDB_BATCH_FNAME , _W( "r" ) );

	if ( NULL == fp )
	{
		bReturn = FALSE;
	}

	if ( TRUE == bReturn )
	{
		fp_tmp = CDexOpenFile( CDDB_BATCH_FNAME_TMP , _W( "w" ) );

		if ( NULL == fp_tmp )
		{
			bReturn = FALSE;
		}
	}

	if ( TRUE == bReturn )
	{
		while ( _fgetts( lpszQuery, sizeof( lpszQuery ), fp ) )
		{
            CUString strLineInBatch( lpszQuery, CP_UTF8 );
            strLineInBatch.Replace( _W( "\r" ), _W( "" ) );
            strLineInBatch.Replace( _W( "\n" ), _W( "" ) );

			// check if this is the string we have to remove
			if ( 0 != strLine.Compare( strLineInBatch ) )
			{
                CUStringConvert strCnv;
                fprintf( fp_tmp, "%s\n", strCnv.ToUTF8( strLineInBatch ) );
			}
		}
	}

	if ( NULL != fp )
	{
		fclose( fp );
	}

	if ( NULL != fp_tmp )
	{
		fclose( fp_tmp );
	}

	if ( TRUE == bReturn )
	{
		LTRACE( _T( "CDdb::RemoveFromCDDBBatch( ), renaming \"%s\" to \"%s\"" ), 
					CDDB_BATCH_FNAME_TMP,
					CDDB_BATCH_FNAME );

		CDexDeleteFile( CDDB_BATCH_FNAME );
		CDexMoveFile( CDDB_BATCH_FNAME_TMP, CDDB_BATCH_FNAME );
		CDexDeleteFile( CDDB_BATCH_FNAME_TMP );
	}
	
	EXIT_TRACE( _T( "CDdb::RemoveFromCDDBBatch( ), return value %d" ), bReturn );

	return bReturn;
}


BOOL CDdb::AddRemoveQueryBatch( BOOL bAdd )
{

	BOOL bReturn = FALSE;
	CUString strSend;
	CUString strTmp;
	strSend.Format( _W( "%08lx %d" ), m_pCDInfo->GetDiscID(), m_pCDInfo->GetNumTracks() );

	ENTRY_TRACE( _T( "CDdb::AddRemoveQueryBatch( %d )" ), bAdd );

	//	Fill Query string with start sectors
	for ( int i = 0; i < m_pCDInfo->GetNumTracks() ; i++ )
	{
		strTmp.Format( _W( " %d" ), m_pCDInfo->GetStartSector( i + 1 ) );
		strSend += strTmp;
	}

	// Add the total number of seconds to it
	strTmp.Format( _W( " %d" ), m_pCDInfo->GetTotalSecs() );

	strSend += strTmp;

	// allways remove the existing entry first ( avoid doubles )
	bReturn = RemoveFromCDDBBatch( strSend );

	if ( bAdd )
	{
		bReturn = StoreToCDDBBatch( strSend );
	}

	EXIT_TRACE( _T( "CDdb::AddRemoveQueryBatch( ), return value %d" ), bReturn );

	return bReturn;
}


BOOL CDdb::QueryRemote( )
{
	CUString		strLang;
	CUString		strRemote;
	BOOL		bStop		= FALSE;
	BOOL		bFound		= FALSE;
	INT			nCategory	= 0;
	BOOL		bReturn		= TRUE;

	ENTRY_TRACE( _T( "CDdb::QueryRemote( )" ) );

    m_strDiscID.Format( _W( "%08lx" ), m_pCDInfo->GetDiscID() );

#ifdef _DEBUG
    // AddRemoveQueryBatch( true );
#endif

#ifdef TEST_MATCH
	CCddbMatch	dlg;
	CUString strTest="rock 02044d01 Oasis / Oasis Interviews\nrock 02044a01 Laurent Voulzy / Rockollection live\nmisc 02044901 Various Artists / Terrordrome Disk 3\n.";
	dlg.AddMatch(strTest);

	if ( IDOK == dlg.DoModal()  )
	{
		m_strReadCat = dlg.GetMatchCat();
		m_strReadID = dlg.GetMatchDiskID();
	}
	else
	{
		bStop=TRUE;
	}

	return FALSE;
#endif

	// Get Disc ID
	m_strReadID.Format( _W( "%08lx" ), m_pCDInfo->GetDiscID() );

	// Try to open remote connection
	if ( CDDB_ERROR_OK != OpenConnection() )
	{
		bReturn = FALSE;
	}

	// Send initial hand shake
	if (	( TRUE == bReturn ) 
			&& ( SendHello() == FALSE ) )
	{
		bReturn = FALSE;
	}

	if (	( TRUE == bReturn ) 
			&& ( SendProto() == FALSE ) )
	{
		bReturn = FALSE;
	}

	if (	( TRUE == bReturn ) 
			&& ( SendQuery(m_strReadCat) == FALSE ) )
	{
		bReturn = FALSE;
	}

	// Create the request command string
	CUString strRead( _W( "cddb read " ) + m_strReadCat + _W( " " ) + m_strReadID );



#ifdef _DEBUG
	{
		DWORD dwDiscID;
		CUString strQuery;
		if ( CheckForTestQuery( dwDiscID, strQuery ) )
		{
			strRead.Format( _W( "cddb read %s %08lx" ), (LPCWSTR)m_strReadCat, dwDiscID );
		}
	}
#endif

	// Now send it the reques command
	if (	( TRUE == bReturn ) 
			&& ( SOCKET_ERROR == SendRemoteMessage( strRead, FALSE ) ) )
	{
		bReturn = FALSE;
	}


	// Get the query result until termination character
	if (	( TRUE == bReturn ) )
	{
        m_vReadResult.clear();

        int nError = 0;
		do
		{
			// Get repsonse
			nError = GetString( m_strReceive );

			// If OK, add to receive buffer
			if ( SOCKET_ERROR != nError )
			{
				SetInfoMsg( m_strReceive + _W( "\r\n" ) );
				strRemote += m_strReceive + _W( "\n" );
			}
			else
			{
				bReturn = FALSE;
			}

		} while ( nError != SOCKET_ERROR && m_strReceive[ 0 ] != _W( '.' ) );
	}

	if ( ( TRUE == bReturn ) )
	{
		strLang = g_language.GetString( IDS_UNKNOWN );
		m_pCDInfo->SetGenre( strLang );
		m_pCDInfo->SetYear( _T( "" ) );

		// set proper category
		SetCategory( m_strReadCat );

		// Parse data and store it into the CD Info structure
		ParseData( strRemote );

		// Save data to data base
		if ( g_config.GetSaveToLocCDDB() )
		{
			WriteCurrentEntry();
		}
	}

	// And close connection
	CloseConnection();

	if ( ( TRUE == bReturn ) )
	{
		// Remove the existing entry from the CDDB Batch query file
		AddRemoveQueryBatch( FALSE );
	}

	EXIT_TRACE( _T( "CDdb::QueryRemote( ), return value %d" ), bReturn );

	return bReturn;
}



CDDB_ERROR CDdb::ReadRemote( const CUString& strReadCatagory, CUString& strDiscID, vector<CHAR>& vReadChars )
{
    CDDB_ERROR returnValue = CDDB_ERROR_OK;

	// Create the request command string
	CUString strRead;
    
    strRead.Format( _W( "cddb read %s %s" ), (LPCWSTR)strReadCatagory, (LPCWSTR)strDiscID );

#ifdef _DEBUG
	{
		DWORD dwDiscID;
		CUString strQuery;
		if ( CheckForTestQuery( dwDiscID, strQuery ) )
		{
			strRead.Format( _W( "cddb read %s %08lx" ), (LPCWSTR)strReadCatagory, dwDiscID );
		}
	}
#endif

	// Now send it the reques command
	returnValue = SendRemoteMessage( strRead, FALSE );


	// Get the query result until termination character
	if ( returnValue == CDDB_ERROR_OK )
	{
        m_vReadResult.clear();

        int nError = 0;
		do
		{
			// Get repsonse
			nError = GetString( m_strReceive );

			// If OK, add to receive buffer
			if ( SOCKET_ERROR == nError )
			{
				returnValue = CDDB_ERROR_FAILED;
			}
		} while ( nError != SOCKET_ERROR && m_strReceive[ 0 ] != _T( '.' ) );
	}

    vReadChars = m_vReadResult;
    vReadChars.push_back( '\0' );

	if ( CDDB_ERROR_OK == returnValue )
	{
		// Remove the existing entry from the CDDB Batch query file
		AddRemoveQueryBatch( FALSE );
	}

	EXIT_TRACE( _T( "CDdb::ReadRemote( ), return value %d" ), returnValue );

	return returnValue;
}


BOOL CDdb::GetRemoteSites()
{
	ENTRY_TRACE( _T( "CDdb::GetRemoteSites( )" ) );

	CUString strSend( _W( "sites" ) );

	// Send site request to remote server
	SendRemoteMessage( strSend, FALSE );

	int nError;
	bool bParsePath = true;


	CUString strPath;


	// Clear all entries
	g_RemoteSites.ClearAll();

	// Add Default Entries
	g_RemoteSites.AddDefaultSites();


	// Get the query result until termination character
	while (TRUE)
	{
		// Get repsonse
		nError=GetString(m_strReceive);

		// If OK, add to receive buffer
		if ( SOCKET_ERROR == nError )
		{
			return FALSE;
		}

		if ( m_strReceive[0] == _T( '.' ) )
		{
			break;
		}

		//    Examples:
		//	ca.us.cddb.com cddbp 888 - N037.23 W122.01 Fremont, CA USA
		//	ca.us.cddb.com http 80 /~cddb/cddb.cgi N037.23 W122.01 Fremont, CA USA
		//  OR FOR CDDBP Query Site (note, not protocol specification):
		//  freedb.freedb.org 888 N037.21 W121.55 San Jose, CA USA

		int nPos;

		nPos = m_strReceive.Find( _T( ' ' ) );

		if ( nPos < 0 ) 
		{
			return FALSE;
		}

		CUString strAddress = m_strReceive.Left( nPos );
		m_strReceive = m_strReceive.Right( m_strReceive.GetLength() - nPos - 1 );

		nPos = m_strReceive.Find( _W( "http" ) );

		int nProtocol = OPTIONS_CDDB_USETCPIP;

		// not HTTP protocol, so it must be CDDBP protocol
		if ( nPos < 0 )
		{
			nProtocol = OPTIONS_CDDB_USETCPIP;

			nPos = m_strReceive.Find( _W( "cddbp" ) );

			// check if protocol string is present
			if (nPos<0) 
			{
				// not, set to zero
				nPos = 0;
				bParsePath = false;
			}
			else
			{
				// present, set to length of cddbp
				nPos = 5;
			}
		}
		else
		{
			// strip http protocol string
			nPos = 4;
			nProtocol = OPTIONS_CDDB_USEHTTP;
		}


		// strip protocol string if present
		if (nPos)
		{
			m_strReceive = m_strReceive.Right(m_strReceive.GetLength()-nPos-1);
		}

		nPos=m_strReceive.Find( _T( ' ' ) );
		if (nPos<0) return FALSE;
		int nPort=_wtoi(m_strReceive.Left(nPos));

		m_strReceive=m_strReceive.Right(m_strReceive.GetLength()-nPos-1);

		if ( bParsePath ) 
		{
			nPos=m_strReceive.Find( _T( ' ' ) );
			if (nPos<0) 
				return FALSE;
			strPath=m_strReceive.Left(nPos);
			m_strReceive=m_strReceive.Right(m_strReceive.GetLength()-nPos-1);
		}
		else
		{
			strPath = _T( "-" );
		}

		nPos=m_strReceive.Find( _T( ' ' ) );
		if (nPos<0) return FALSE;
		CUString strPosition1=m_strReceive.Left(nPos);
		m_strReceive=m_strReceive.Right(m_strReceive.GetLength()-nPos-1);

		nPos=m_strReceive.Find( _T( ' ' ) );
		if (nPos<0) return FALSE;
		CUString strPosition2=m_strReceive.Left( nPos );
		m_strReceive=m_strReceive.Right(m_strReceive.GetLength()-nPos-1);
		
		g_RemoteSites.AddSite(strAddress,m_strReceive,strPath,nPort,nProtocol);

		SetInfoMsg( m_strReceive );

	}

	g_RemoteSites.SetActiveSite(0);

	EXIT_TRACE( _T( "CDdb::GetRemoteSites( )" ) );

	return TRUE;
}


BOOL CDdb::QuerySites()
{
	BOOL bReturn = TRUE;

	ENTRY_TRACE( _T( "CDdb::QuerySites( )" ) );

	// Try to open remote connection
	if ( ( TRUE == bReturn ) && ( CDDB_ERROR_OK != OpenConnection() ) )
	{
		bReturn = FALSE;
	}

	// Send initial hand shake
	if ( ( TRUE == bReturn ) && ( FALSE == SendHello() ) )
	{
		bReturn = FALSE;
	}

	if ( ( TRUE == bReturn ) && ( FALSE == GetRemoteSites() ) )
	{
		bReturn = FALSE;
	}

	// And close connection
	CloseConnection();


	EXIT_TRACE( _T( "CDdb::QuerySites( ), return value %d" ), bReturn );

	return bReturn;
}


BOOL CDdb::WriteCurrentEntry()
{
	// Find entry in CDDB
	CUString strFileName;
	CUString strCategory;
	int		nCategory=0;

	LONG	lFilePos = SearchEntry( strFileName, strCategory );

	// Delete it
	if ( -1 != lFilePos )
	{
		RemoveEntry( strFileName );
	}

	strCategory = m_pCDInfo->GetCDDBCat();

	// Create file name
	strFileName= GenerateFileName( strCategory );

	// Make sure that the directories exist
	for (nCategory = 0 ; nCategory < NumCategories ; nCategory ++) 
	{
		CUString strCategory( GetCategoryDir( nCategory )  );

		// Create fill dir path
		CUString strTestDir( m_strLocalPath + strCategory );

		// And test it, no dialog please
		DoesDirExist( strTestDir, FALSE );
	}

	// And write entry to disk
	WriteEntry( strFileName, 
				!m_bIsUnixCDDBType,
				( g_config.GetCDDBWriteAsDosFile( ) > 0 ) );

	return TRUE;
}


BOOL CDdb::QueryLocal()
{
	CUString strLang;
	CUString strFileName;
	CUString	strCategory;
	LONG	lFilePos = -1;
	BOOL	bReturn = FALSE;

	ENTRY_TRACE( _T( "CDdb::QueryLocal" ) );

	if ( -1 != ( lFilePos = SearchEntry( strFileName, strCategory ) ) )
	{
		strLang = g_language.GetString( IDS_UNKNOWN );
		m_pCDInfo->SetGenre( strLang);
		m_pCDInfo->SetYear( _W( "" ) );

		// set proper category
		SetCategory( m_strReadCat );

		bReturn = ReadEntry( strFileName, lFilePos );
	}

	if ( FALSE == bReturn )
	{
		bReturn = ReadRarEntry( strFileName );

	}

	EXIT_TRACE( _T( "CDdb::QueryLocal with value %d" ), bReturn );

	return bReturn;
}


// Convert the incoming string into the CDInfo structure
BOOL CDdb::ParseData( const CUString& strToParse )
{
	CUString	strDTitle;
	CUString strEDTitle;
	BOOL	bDYear = FALSE;
	BOOL	bDGenre = FALSE;
	CUString	aStrTitle[ 100 ];
	CUString	aStrEXTT[ 100 ];
	int		nTrackIndex = 0;
	LPSTR	lpRet = NULL;
	CUString	strParse;
	CUString	strLine;
	CUString strTitle;
	int		nStart = -1;
	int		nStop = 0;
	
	// remove \r 
	strParse = strToParse.SpanExcluding( _W( "\r" ) );

	nStop = strParse.Find( _T( '\n' ) );

	if ( nStop == -1 )
	{
		nStop = strParse.GetLength();
	}

	while ( nStart < strParse.GetLength() )
	{
		strLine = strParse.Mid( nStart, nStop - nStart );
		
		ASSERT( strLine.GetLength() <= MAX_CDDB_LINE );

		// Get DTITLE field
		if ( strLine.Find( DTITLETAG ) >= 0 )
		{
			// Copy title to tmp string
			strDTitle += strLine.Mid(  DTITLETAG.GetLength() );
		}

		// Get Extra EXTD field
		if ( strLine.Find( EXTDTAG ) >= 0 )
		{
			// Copy title to tmp string
			strEDTitle+= strLine.Mid(  EXTDTAG.GetLength() );
		}

		// Get TTITLE field
        if ( strLine.Find( TITLETAG ) >= 0 ) 
		{
			CUString strDigit;

			// Get TTITLE digit
			if ( _T( '=' ) == strLine[ TITLETAG.GetLength() + 1 ] )
			{
				// ONE DIGIT
				strDigit = strLine.Mid( TITLETAG.GetLength(), 1   );
				strTitle = strLine.Mid( TITLETAG.GetLength() + 2  );
			}
			else
			{
				// TWO DIGITS
				strDigit = strLine.Mid( TITLETAG.GetLength() , 2 );
				strTitle = strLine.Mid( TITLETAG.GetLength() + 3 );
			}

			// Get digit
			nTrackIndex = _wtoi( strDigit );

			// Add new string to title
			if ( nTrackIndex >= 0 && nTrackIndex <= 99 )
			{
				aStrTitle[ nTrackIndex ]+= strTitle;
			}

		}

		// Get EXTT field
        if ( strLine.Find( EXTTTAG ) >= 0 ) 
		{
			CUString strDigit;

			// Get EXTT digit
			if ( _T( '=' ) == strLine[ EXTTTAG.GetLength() + 1 ] )
			{
				// ONE DIGIT
				strDigit = strLine.Mid( EXTTTAG.GetLength(), 1 );
				strTitle = strLine.Mid( EXTTTAG.GetLength() + 2 );
			}
			else
			{
				// TWO DIGITS
				strDigit = strLine.Mid( EXTTTAG.GetLength(), 2 );
				strTitle = strLine.Mid( EXTTTAG.GetLength() + 3 );
			}

			// Get digit
			nTrackIndex = _wtoi( strDigit );

			if ( nTrackIndex >= 0 && nTrackIndex <= 99 )
			{
				aStrEXTT[ nTrackIndex ] += strTitle;
			}
		}

        if ( strLine.Find( REVISIONTAG ) >= 0 ) 
		{
			int nRev=_wtoi( strLine.Mid( REVISIONTAG.GetLength() + 1 ) );
			m_pCDInfo->SetRevision( nRev );
		}

        if ( strLine.Find( SUBMITVIATAG ) >= 0 ) 
		{
			m_pCDInfo->SetSubmission( strLine.Mid( SUBMITVIATAG.GetLength() + 1 ) );
		}

        if ( strLine.Find( DYEARTAG ) >= 0 ) 
		{
			int nPos;

			nPos = strLine.Find( _T( '=' ) );

			if ( nPos > 0 )
			{
				strLine = strLine.Mid( nPos + 1 );

				strLine.TrimLeft();
				strLine.TrimRight();

				LTRACE( _T( "DYEAR value parsed, value \"%s\"" ), strLine );

				m_pCDInfo->SetYear( strLine );

				bDYear = TRUE;
			}
		}

        if ( strLine.Find( DGENRETAG ) >= 0 ) 
		{
			int nPos;

			nPos = strLine.Find( _T( '=' ) );

			if ( nPos > 0 )
			{
				strLine = strLine.Mid( nPos + 1 );

				strLine.TrimLeft();
				strLine.TrimRight();

                if ( strLine.GetLength() >  0 )
                {
				    m_pCDInfo->SetGenre( strLine );
                }

				LTRACE( _T( "DGENRE value parsed, value \"%s\"" ), strLine );

				bDGenre = TRUE;
			}
		}

		nStart = nStop + 1;

		nStop = strParse.Find( _T( '\n' ), nStart );

		if ( nStop == -1 )
		{
			nStop = strParse.GetLength();
		}

	}



	int nPos= strEDTitle.Find( _W( "ID3G:" ) );
	if ( nPos >= 0 )
	{
		if ( FALSE == bDGenre )
		{
			CUString strGenreID = strEDTitle.Mid( nPos + 5, 4 );
            CUStringConvert strCnv; 

			int nGenreID = _ttoi( strCnv.ToT( strGenreID ) );

			LTRACE( _T( "ID3G value parsed, value (%d) \"%s\"" ),
						nGenreID,
						g_GenreTable.GetID3V1GenreString( nGenreID ) );

			// be backward compatible !
			m_pCDInfo->SetGenre( g_GenreTable.GetID3V1GenreString( nGenreID ) );
		}
		
		// Delete the TAG stuff
		strEDTitle = strEDTitle.Left( nPos );

	}

	// Get ID3 Tag info from ExtTitle
	nPos = strEDTitle.Find( _W( "YEAR:" ) );

	if ( nPos >= 0 )
	{
		// Set CDInfo
		if ( FALSE == bDGenre )
		{
			LTRACE( _T( "YEAR: value parsed, value \"%s\"" ),
						strEDTitle.Mid( nPos + 6, 4 ) );

			m_pCDInfo->SetYear( strEDTitle.Mid( nPos + 6, 4 ) );
		}

		// Delete the TAG stuff
		strEDTitle = strEDTitle.Left( nPos );
	}

	// Trim strEDTitle
	strEDTitle.TrimLeft();
	strEDTitle.TrimRight();

	while ( ( nPos = strEDTitle.Find( _W( "\n" ) ) ) > 0 )
	{
		strEDTitle = strEDTitle.Mid( nPos + 1 );
	}

	// Set extra DTitle
	m_pCDInfo->SetExtTitle( strEDTitle );

	for ( nTrackIndex = 0; nTrackIndex< 100; nTrackIndex++)
	{
		m_pCDInfo->SetTrackName( aStrTitle[ nTrackIndex ], nTrackIndex );
		m_pCDInfo->SetExtTrackName( aStrEXTT[ nTrackIndex ], nTrackIndex );
	}

	// Split D Title into Artist / Album name
	nPos = strDTitle.Find( _W( " / " ) );

	if ( nPos >= 1 )
	{
		CUString strTmp;

		strTmp = strDTitle.Left( nPos );
		strTmp.TrimLeft();
		strTmp.TrimRight();

		m_pCDInfo->SetArtist( strTmp );

		strTmp = strDTitle.Mid( nPos + 2  );
		strTmp.TrimLeft();
		strTmp.TrimRight();

		m_pCDInfo->SetTitle( strTmp );

	}
	else
	{
		m_pCDInfo->SetArtist( _T( "" ) );
		m_pCDInfo->SetTitle( strDTitle );
	}

	return TRUE;
}

BOOL CDdb::ReadEntry( const CUString& strFileName, long lFilePos )
{
	CTextFile	cReadFile;
	CUString		strRead;
	CUString		strBuffer;
	CUString		strDTitle;
	BOOL		bDone = FALSE;
	BOOL		bReturn = FALSE;

	// Clear buffer
	memset( m_lpszBuffer, 0x00, sizeof( m_lpszBuffer ) );

    CUStringConvert strCnv; 

	// open file, check result
	if ( cReadFile.Open( strCnv.ToT( strFileName ), CTextFile::modeRead ) )
	{
		int nFileNameTags = 0;

		// seek to position
		cReadFile.Seek( lFilePos, CTextFile::seekBegin );
		
		// get entry data
		while( cReadFile.ReadString( strRead ) && nFileNameTags < 2 )
		{
			CUString strUpper( strRead );

			// make all uppercase
			strUpper.MakeUpper();

			// Are we already hit the next entry ?
			if ( strUpper.Find( FILENAMETAG ) >= 0 )
			{
				nFileNameTags++;
			}
			else
			{
				strBuffer += strRead + _W( "\n" );
			}
		}

		// close the file
		cReadFile.Close();

		// Parse data into CD Info structure
		ParseData( strBuffer );

		bReturn = TRUE;
	}
	else
	{
		bReturn = FALSE;
	}

	// return result
	return bReturn;
}

BOOL CDdb::ReadRarEntry( const CUString& strFileName )
{
	BOOL		bReturn = FALSE;

#if 0
#endif

	// return result
	return bReturn;
}


BOOL CDdb::CheckFrameOffsets( CTextFile& cReadFile )
{
	BOOL	bReturn = FALSE;
	BOOL	bEntry = FALSE;
	WORD	wEntry = 0;
	BOOL	bEntriesOK = FALSE;
	BOOL	bSecsOK = FALSE;
	BOOL	bDone = FALSE;
	CUString	strRead;

	ENTRY_TRACE( _T( "CDdb::CheckFrameOffsets()" ) );

	// Read line from file
	while( cReadFile.ReadString( strRead ) && !bDone )
	{
		CUString strUpper( strRead );
		strUpper.MakeUpper();

        if ( strUpper.Find( FILENAMETAG ) >= 0 ) 
		{
			bDone = TRUE;
		}

		// Search for the # Track frame offsets:
		if ( strRead.Find( _W( "# Track frame offsets:") ) >= 0 ) 
		{
			bEntry = TRUE;
			LTRACE( _T( "CheckFrameOffsets # Track frame offsets: Found" ) );
		}

		if ( bEntry )
		{
			CUString strOffset[2];

			strOffset[0].Format( _W( "%d" ), m_pCDInfo->GetStartSector( wEntry + 1 ) + 150 );
			strOffset[1].Format( _W( "%d" ), m_pCDInfo->GetStartSector( wEntry + 1 ) - m_pCDInfo->GetStartSector( 1 ) + 150 );

			// Check offset
			if ( wEntry < m_pCDInfo->GetNumTracks() )
			{
				if ( strRead.Find( strOffset[0] ) >= 0 ||
					 strRead.Find( strOffset[1] ) >= 0 ) 
				{
					bEntriesOK = TRUE;
					LTRACE( _T( "CheckFrameOffsets: Entry Track[%d] Offset OK " ) , wEntry );
					wEntry++;
				}
				else
				{
					bEntriesOK = FALSE;

					LTRACE( _T( "CheckFrameOffsets: Entry Track[%d] Offset NOT OK (is %s should be %s or %s" ),
											wEntry,
											strRead,
											strOffset[ 0 ],
											strOffset[ 1 ] );
				}
			}
		}

		if ( strRead.Find( _W( "# Disc length:" ) ) >= 0 ) 
		{
			CUString strSecs;

			// Ok this is all we need
			bDone = TRUE;

			// Create a string of the offset
			strSecs.Format( _W( "%d" ), m_pCDInfo->GetTotalSecs() );

			// Check secs
			if ( strRead.Find( strSecs ) >= 0 ) 
			{
				bSecsOK = TRUE;

				LTRACE( _T( "CheckFrameOffsets: Secs OK %d" ), m_pCDInfo->GetTotalSecs() );
			}
			else
			{
				// WORK AROUND FOR OLD BUG
				strSecs.Format( _W( "%d" ), m_pCDInfo->GetTotalSecs() - 1 );

				if ( strRead.Find( strSecs ) >= 0 ) 
				{
					bSecsOK = TRUE;

					//WriteCurrentEntry();
					LTRACE( _T( "Bug Workaround in total secs %d" ), m_pCDInfo->GetTotalSecs() );
				}
				else
				{
					bSecsOK = FALSE;
					LTRACE( _T( "CheckFrameOffsets: Secs NOT OK is %d should be %s" ),
								m_pCDInfo->GetTotalSecs(), 
								strRead );
				}
			}

		}
	}


	// Check if all conditions are met
	if ( bSecsOK && bEntriesOK )
	{
		bReturn = TRUE;
	}

	EXIT_TRACE( _T( "CDdb::CheckFrameOffsets, return value %d" ), bReturn );

	// Not OK
	return bReturn;
}



// Try to locate find the disc in the strFileName file
// If found, return proper index where in file entry exists
LONG CDdb::SearchEntryInFiles( CUString& strFileName )
{
	LONG		lFileNameIndex=0;
	CTextFile	cReadFile;
	CUString		strRead;

    CUStringConvert strCnv;

	LTRACE( _T( "SearchEntryInFiles \"%s\"" ), strCnv.ToT( strFileName ) );


	// try to open the file
	if ( !cReadFile.Open( strCnv.ToT( strFileName ), CTextFile::modeRead ) )
	{
		LTRACE( _T( "SearchEntryInFiles failed to open file" ) );
		return -1;
	}
	else
	{
		while( cReadFile.ReadString( strRead ) )
		{
			CUString strUpper( strRead );
			strRead.MakeUpper();


			// Search for the #FILENAME
			if ( ( strUpper.Find( FILENAMETAG ) >= 0 ) ||  m_bIsUnixCDDBType ) 
			{
				// Check DISC ID
				if ( strRead.Find( m_strDiscID ) ||  m_bIsUnixCDDBType )
				{
					LTRACE( _T( "SearchEntryInFiles DiscID \"%s\" Found" ),m_strDiscID);

					// Check if Frame Offsets are OK
					if ( CheckFrameOffsets( cReadFile ) )
					{
						LTRACE( _T( "SearchEntryInFiles returns %d" ), lFileNameIndex );

						// close the file
						cReadFile.Close();

						// And were all done, return start of entry
						return lFileNameIndex;
					}
				}
			}

			// Get current file position
			lFileNameIndex= (LONG)cReadFile.GetPosition( );
		}

		// close the file
		cReadFile.Close( );
	}

	LTRACE( _T( "SearchEntryInFiles returns %d" ), -1 );

	// Entry not found
	return -1;
}


// Try to locate find the disc in the strFileName file
// If found, return proper index where in file entry exists
// Also it returns the proper strCat and the strFileName

LONG CDdb::SearchEntry( CUString& strFileName, CUString& strCategory )
{
	LONG	lFilePos = -1;
	DWORD	dwCategory = 0;
	BOOL	bFound = FALSE;

	ENTRY_TRACE( _T( "CDdb::SearchEntry(%s,%s)" ), strFileName, strCategory );

	// loop through the categories
    for ( dwCategory = 0 ; dwCategory < NumCategories ; dwCategory++ ) 
	{
		// set category
		strCategory =  strCDDBCatagories[ dwCategory ];

		// get the proper file name
		strFileName = GenerateFileName( strCategory );

		// Get the location of the entry in the file
		lFilePos = SearchEntryInFiles( strFileName );

		if ( lFilePos >= 0 )
		{
			bFound = TRUE;
			break;
		}
	}

	if ( FALSE == bFound )
	{
		lFilePos = -1;
	}

	EXIT_TRACE( _T( "CDdb::SearchEntry( %s, %s), return value %d" ),
				strFileName, 
				strCategory,
				lFilePos );

	return lFilePos;
}


BOOL CDdb::WriteEntry( const CUString& strFileName,bool bIsWindowsTypeDB, bool bIsDosFileType )
{
	CTextFile	cWriteFile;
	CUString		strWrite;
	CUString		strTmp;
	BOOL		bReturn = TRUE;

	ENTRY_TRACE( _T( "CDdb::WriteEntry(%s, %d, %d)" ), strFileName, bIsWindowsTypeDB, bIsDosFileType );

	// Set File type
	cWriteFile.SetWriteAsDosFile( bIsDosFileType );

    CUStringConvert strCnv;

	if ( ! cWriteFile.Open( strCnv.ToT( strFileName ), CTextFile::modeAppend ) ) 
	{
		ASSERT( FALSE );
		bReturn = FALSE;
	}		
	else
	{
		if ( bIsWindowsTypeDB )
		{
			LTRACE( _T( "WriteEntry: Write #FILENAME tag for Windows CDDB" ) );

			// Write #FILENAME tag for Windows Type CDDB
			cWriteFile.WriteString( FILENAMETAG + m_strDiscID );
		}

		// Write track start information
		cWriteFile.WriteString( _T( "# xmcd CD database file" ) );

		cWriteFile.WriteString( _T( "#" ) );

		// Write track offsets
		cWriteFile.WriteString( _T( "# Track frame offsets:" ) );

		// Write track start information
		for ( int i= 0 ; i< m_pCDInfo->GetNumTracks() ; i++) 
		{
			strWrite.Format( _W( "#\t%d" ), m_pCDInfo->GetStartSector( i + 1 ) + 150 );
			cWriteFile.WriteString( strWrite );
		}

		// Write disc length
		cWriteFile.WriteString( _T( "#" ) );

		strWrite.Format( _W( "# Disc length: %d seconds"), m_pCDInfo->GetTotalSecs() );
		cWriteFile.WriteString( strWrite );

		// Write revision and submitted
		cWriteFile.WriteString( _T( "#" ) );

		// Bug fix, in pre CDex 1.30, there is an error regarding the revision number
		// Check if revision is invalid, if so, change it to 0
		if ( m_pCDInfo->GetRevision() > 100 )
		{
			strWrite.Format( _W( "%s %d" ), (LPCWSTR)REVISIONTAG, 0 );
			cWriteFile.WriteString( strWrite );
		}
		else
		{
			strWrite.Format( _W( "%s %d"), (LPCWSTR)REVISIONTAG, m_pCDInfo->GetRevision() );
			cWriteFile.WriteString( strWrite );
		}

		strWrite.Format( _W( "%s %s"), (LPCWSTR)SUBMITVIATAG, (LPCWSTR)m_pCDInfo->GetSubmission() );

		cWriteFile.WriteString( strWrite );

		// Write DISCID to file
		cWriteFile.WriteString( _T( "#" ) );
		WriteLine( cWriteFile, _T( "DISCID" ), m_strDiscID );

		// DTITLE
		CUString strDTitle( m_pCDInfo->GetArtist() + _W( " / " ) + m_pCDInfo->GetTitle() );
		
		// Write DTITLE to file
		WriteLine( cWriteFile, _T( "DTITLE" ), strDTitle);

		// DYEAR Field
		WriteLine( cWriteFile, DYEARTAG, m_pCDInfo->GetYear() );

		// DGENRE Field
		WriteLine( cWriteFile, DGENRETAG, m_pCDInfo->GetGenre() );

		// TRACKS
		for (int nLoop = 0 ; nLoop < m_pCDInfo->GetNumTracks() ; nLoop ++) 
		{
			CUString strTitleTag;
			strTitleTag.Format( _W( "TTITLE%d") , nLoop );
			WriteLine( cWriteFile, strTitleTag, m_pCDInfo->GetTrackName( nLoop ) );
		}

		CUString strExtD( m_pCDInfo->GetExtTitle() );

		if ( !m_pCDInfo->GetYear().IsEmpty() )
		{
			CUString strTmp( strExtD );
			strExtD.Format( _W( "%s YEAR: %s" ), (LPCWSTR)strTmp, (LPCWSTR)m_pCDInfo->GetYear() );
		}

		// EXTD Field
		WriteLine( cWriteFile, _T( "EXTD" ), strExtD );

		// EXTT
		for (int nLoop = 0 ; nLoop < m_pCDInfo->GetNumTracks() ; nLoop ++) 
		{
			CUString strTitleTag;
			strTitleTag.Format( _W( "EXTT%d" ), nLoop);
			WriteLine( cWriteFile, strTitleTag,m_pCDInfo->GetExtTrackName(nLoop) );
		}

		cWriteFile.WriteString( _T( "PLAYORDER=") );

		// Close the file
		cWriteFile.Close(  );
	}

	EXIT_TRACE( _T( "CDdb::WriteEntry(), return value: %d" ), bReturn );

	return bReturn;
}

void CDdb::WriteLine( CTextFile& cWriteFile, const CUString& strName, const CUString& strValue )
{
	CUString strWriteValue( strValue );
	bool	bDone = false;

	while ( !bDone )
	{
		CUString strTag( strName );
        
        strTag += _W( "=" );

		CUString strOut = strTag + strWriteValue;

		if ( strOut.GetLength() > ( MAX_CDDB_LINE - 2 ) )
		{
			strWriteValue = strWriteValue.Mid( ( MAX_CDDB_LINE - 2 ) - strTag.GetLength() );
			strOut = strOut.Left( ( MAX_CDDB_LINE - 2 ) );
		}
		else
		{
			bDone = true;
		}

		cWriteFile.WriteString( strOut );
	}
}

BOOL CDdb::RemoveEntry( CUString& strFileName )
{
	BOOL	bReturn = TRUE;
	CUString strRead;
	CUString strSearch = FILENAMETAG + m_strDiscID;
	
	ENTRY_TRACE( _T( "CDdb::RemoveEntry(%s)" ), strFileName );

	// make upcase
	strSearch.MakeUpper();

	if ( m_bIsUnixCDDBType ) 
	{
		// Simply delete the file for Unix CDDBs
		CDexDeleteFile( strFileName );
	}
	else
	{
		// Windows CDDB is a little more complicated,
		// Open the file, seek to nFilePos, search for next #FILENAME entry
		// Move everything behind #FILENAME entry to nFilePos

		CTextFile cReadFile;
		CTextFile cWriteFile;


		FILE* fp = NULL;
		FILE* fpOut = NULL;

		CUString strNewFileName = CUString( strFileName + _W( ".new" ) );

        CUStringConvert strCnv;

		cWriteFile.SetWriteAsDosFile( g_config.GetCDDBWriteAsDosFile() > 0 );

		if ( !cReadFile.Open( strCnv.ToT( strFileName ), CTextFile::modeRead ) )
		{
			bReturn = FALSE;
		}


		if ( !cWriteFile.Open( strCnv.ToT( strNewFileName ), CTextFile::modeWrite ) )
		{
			bReturn = FALSE;
		}

		if ( TRUE == bReturn )
		{
			bool bDeleting = false;

			// copy all the lines
			while ( cReadFile.ReadString( strRead ) )
			{
				CUString strUpper( strRead );
				strUpper.MakeUpper();

				if ( false == bDeleting  )
				{
					// OK, search for next #FILENAME Tag
					if ( strUpper.Find( strSearch ) >= 0 )
					{
						bDeleting = true;
					}
					else
					{
						cWriteFile.WriteString( strRead );
					}
				}
				else
				{
					if ( strUpper.Find( FILENAMETAG ) >= 0 )
					{
						bDeleting = false;
						cWriteFile.WriteString( strRead );
					}
				}
			}

			cReadFile.Close( );
			cWriteFile.Close( );

			// And rename file
			CDexCopyFile( strNewFileName, strFileName, FALSE );

			// delete the temp file
			CDexDeleteFile( strNewFileName );
		}
	}

	EXIT_TRACE( _T( "CDdb::RemoveEntry(%s), return value: %d" ), strFileName, bReturn );

	return bReturn;
}

CUString  CDdb::GetCategoryDir( DWORD dwCategory )
{
	ENTRY_TRACE( _T( "CDdb::GetCategoryDir( %d )" ), dwCategory );

	ASSERT( dwCategory < NumCategories );

	CUString strCategory( strCDDBCatagories[ dwCategory ] );

	// trim directory file name to a maximum of 8 characters?
	if ( FALSE == g_config.GetCDDBLongDirNames() )
	{
		strCategory = strCategory.Left( 8 );
	}

	EXIT_TRACE( _T( "CDdb::GetCategoryDir, with value: %s" ), strCategory );

	return strCategory;
}

CUString  CDdb::GetWindowCDDBFileName( const CUString& strCategoryDir )
{
	int			nFileID	= 0;
	CUString		strFileName;
	CFileFind	cFileFind;
	BOOL		bFileFound = FALSE;
	BOOL		bEntryFound = FALSE;

	ENTRY_TRACE( _T( "Entering CDdb::GetWindowCDDBFileName( %s )" ),
				strCategoryDir );

	// extract ID from first two digitis of th CDDB ID
	swscanf( m_strDiscID.Left( 2 ), _W( "%X" ), &nFileID );

	// build search pattern
	strFileName.Format( _W( "%s%s\\*.*" ), (LPCWSTR)m_strLocalPath, (LPCWSTR)strCategoryDir );

	LTRACE( _T( "CDdb::GetWindowCDDBFileName, search pattern %s" ), strFileName );

	// do a file find
    CUStringConvert strCnv;

    bFileFound = cFileFind.FindFile( strCnv.ToT( strFileName ) );

	// loop through the found file names
	while ( ( TRUE == bFileFound ) &&
			( FALSE == bEntryFound ) )
	{
		bFileFound = cFileFind.FindNextFile();

		// get the file name
		strFileName = CUString( cFileFind.GetFileName() );

		// check the filename length
		if ( 6 == strFileName.GetLength() )
		{
			int nFileIDStart = 0;
			int nFileIDEnd =0;

			// extract range from file name
            CUStringConvert strCnv;
			_stscanf( strCnv.ToT( strFileName.Mid( 0, 2 ) ), _T( "%X" ), &nFileIDStart );
			_stscanf( strCnv.ToT( strFileName.Mid( 4, 2 ) ), _T( "%X" ), &nFileIDEnd );

			// is ID wihtin the range were looking for
            if ( ( nFileID >= nFileIDStart ) &&
				 ( nFileID <= nFileIDEnd ) ) 
			{
				// get full file path
				strFileName = CUString( cFileFind.GetFilePath() );

				LTRACE( _T( "CDdb::GetWindowCDDBFileName, found entry (%s)" ), strFileName );

				bEntryFound = TRUE;
			}
		}
	}

	// if not found, build it ourself
	if ( FALSE == bEntryFound )
	{
	    strFileName.Format( _W( "%s%s\\%02xto%02x" ),  (LPCWSTR)m_strLocalPath, (LPCWSTR)strCategoryDir, nFileID, nFileID );
	}

	EXIT_TRACE( _T( "CDdb::GetWindowCDDBFileName, return value: %s" ), strFileName );

	return strFileName;
}


CUString CDdb::GenerateFileName( const CUString& strCategory )
{
	CUString		strFileName;
	CUString		strCategoryDir( strCategory );

	ENTRY_TRACE( _T( "CDdb::GenerateFileName( %s )"), strCategory );

	// trim directory file name to a maximum of 8 characters?
	if ( FALSE == g_config.GetCDDBLongDirNames() )
	{
		strCategoryDir = strCategoryDir.Left( 8 );
	}

	// Unix Data Base, simple
	if ( m_bIsUnixCDDBType ) 
	{
		// For UNIX type CDDB, just create file name string from CDDB ID
		strFileName.Format( _W( "%s%s\\%s" ), (LPCWSTR)m_strLocalPath, (LPCWSTR)strCategoryDir, (LPCWSTR)m_strDiscID );
	}
	else
	{
		// Create windows file name string
		strFileName = GetWindowCDDBFileName( strCategory );
	}

	EXIT_TRACE( _T( "CDdb::GenerateFileName, return value :%s:"), strFileName );

	return strFileName;
}


CUString CDdb::EncodeBASE64( CUString szMessage )
{
    CHAR bIndex[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	CUString strEncoded;
    int i,n;

    n = szMessage.GetLength();

    strEncoded = _T( "" );

    for( i = 0; i < n  ; i += 3 )
	{
        LONG d = 0;

		CHAR ch[4] = {'\0',};

		ch [ 0 ] = (CHAR)szMessage.GetAt( i + 0 );

		if ( ( i + 1 ) < n )
		{
			ch [ 1 ] = (CHAR)szMessage.GetAt( i + 1 );
		}
		if ( ( i + 2 ) < n )
		{
			ch [ 2 ] = (CHAR)szMessage.GetAt( i + 2 );
		}

		d  = ( ( ch[ 0 ] & 0xFF ) >> 2 ); 
        strEncoded += bIndex[ d & 0x3f ];
		
		d  = ( ( ch[ 0 ] & 0x03 ) << 4 ) + ( ( ch[ 1 ] & 0xF0 ) >> 4 ); 
		strEncoded += bIndex[( d ) & 0x3f ];

		if ( ( i + 1 ) < n )
		{
			d  = ( ( ch[ 1 ] & 0x0F ) << 2 ) + ( ( ch[ 2 ] & 0xFF ) >> 6 ); 
			strEncoded += bIndex[( d ) & 0x3f ];
		}
		else
		{
			strEncoded += _W( "=" );
		}

		if ( ( i + 2 ) < n )
		{
			d  = ( ( ch[ 2 ] & 0x3F ) ); 
			strEncoded += bIndex[( d ) & 0x3f ];
		}
		else
		{
			strEncoded += _W( "=" );
		}

    }

    return strEncoded;
}

int CDdb::SendAuthentication()
{
	CUString strSend;
    CUString strEncoded;
	CUString strAuth;

    if (	g_config.GetCDDBUseAuthentication() &&
			g_config.GetCDDBProxyUser().GetLength()>0 &&
			g_config.GetCDDBProxyPassword().GetLength()>0 )
	{

		strAuth.Format( _W( "%s:%s" ),  (LPCWSTR)g_config.GetCDDBProxyUser(),  (LPCWSTR)g_config.GetCDDBProxyPassword() );

		// Encode authentication string
		strEncoded = EncodeBASE64( strAuth );

		// Format it once again
		strSend.Format( _W( "Proxy-Authorization: Basic %s" ), (LPCWSTR)strEncoded );

		return SendString( strSend );
	}
	return 0;
}


BOOL CDdb::SubmitRemote()
{
	BOOL		bUseMailTo=(g_config.GetCDDBSubmitVia()==0);
	BOOL		bSubmitError=FALSE;
	CUString		strOut;
	CUString		strLang;
	CUString		strMailSubject;
	CUString		strMailBody;

	// Get corresponding CDDB genre string
	CUString		strCat=	m_pCDInfo->GetCDDBCat();

	// Write entry to disk ( to save in proper Directory )
	WriteCurrentEntry();

	// Set submission string to to CDex 
	strOut.Format( _W("CDex %s"), (LPCWSTR)GetClientVersion() );

	m_pCDInfo->SetSubmission( strOut );

	// Increase revision number
	m_pCDInfo->SetRevision(m_pCDInfo->GetRevision() + 1 );
//	m_pCDInfo->SetRevision( 0 );

    CUStringConvert strCnv; 

	// Check if test mode is turned on
	BOOL bTestMode= GetPrivateProfileInt(	_T( "Debug" ), 
											_T( "SubmitTestMode" ), 
											0,
											strCnv.ToT( g_config.GetIniFileName() ) );

	if ( FALSE == bUseMailTo )
	{
		// Try to open remote connection
		if ( CDDB_ERROR_OK != OpenConnection( g_config.GetCDDBHSubmitAddr(), HTTP_DEFAULT_PORT  ) )
		{
			return FALSE;
		}

		if ( g_config.GetCDDBUseProxy() )
		{
			strOut.Format( _W( "POST http://%s:%d%s HTTP/1.0" ), (LPCWSTR)g_config.GetCDDBHSubmitAddr(), HTTP_DEFAULT_PORT, HTTP_SUBMIT_CGI );
		}
		else
		{
			strOut.Format( _W( "POST %s HTTP/1.0" ), HTTP_SUBMIT_CGI );
		}
   
		if ( SOCKET_ERROR == SendString( strOut ) )
		{
			return FALSE;
		}

		// send HTTP-User Agent (part of HTTP Header, see RFC822 spec
		if ( SOCKET_ERROR == SendString( _W( "UserAgent:  CDex/" ) + GetClientVersion() ) )
		{
			return FALSE;
		}

		// Send authentication if necessary
		if ( g_config.GetCDDBUseAuthentication() )
		{
			if ( SOCKET_ERROR  == SendAuthentication() )
			{
				return FALSE;
			}
		}

   		strOut.Format( _W( "Category: %s" ) ,(LPCWSTR)strCat );

		if ( SOCKET_ERROR == SendString( strOut ) )
			return FALSE;
		
		strOut.Format( _W( "Discid: %08lx" ), m_pCDInfo->GetDiscID() );

		if ( SOCKET_ERROR == SendString( strOut ) )
			return FALSE;
		
		strOut.Format( _W( "User-Email: %s" ), (LPCWSTR)g_config.GetEmailAddress() );
		if ( SOCKET_ERROR == SendString( strOut ) )
			return FALSE;
		
		strOut.Format( _W( "Submit-Mode: %s" ), (bTestMode==0) ? _W( "submit" ) : _W( "test" ) );

		if ( SOCKET_ERROR == SendString( strOut ) )
			return FALSE;

		if ( 6 == gs_bProtoLevel )
		{
			strOut.Format( _W( "Charset: %s" ), _W( "utf-8" ) );
			SendString( _T( "Content-Type: text/plain; charset=utf-8" ) );
		}
		else
		{
			SendString( _W( "Content-Type: text/plain; charset=iso-8859-1" ) );
			strOut.Format( _W( "Charset: %s" ), _W( "iso-8859-1" ) );
		}

		if ( SOCKET_ERROR == SendString( strOut ) )
		{
			return FALSE;
		}

		strOut.Format( _W( "X-Cddbd-Note: Submission problems?  E-mail %s" ), _W( "freedb@freedb.org" ) );
		if ( SOCKET_ERROR == SendString( strOut )  )
			return FALSE;

	}
	else
	{
		strMailSubject.Format( _W( "cddb %s %08lx" ), (LPCWSTR)strCat, m_pCDInfo->GetDiscID() );
	}

	CDexDeleteFile( m_strSubmitFileName );

	// Write entry to temp file
	WriteEntry( m_strSubmitFileName, false, true );

	// Open entry file
	FILE* fp = CDexOpenFile( m_strSubmitFileName, _W( "rt" ) );

	if ( NULL == fp )
	{
		ASSERT( FALSE );
		return FALSE;
	}

	// seek to end of file
	fseek( fp, 0, SEEK_END );

	int nFileSize = ftell( fp );

	// seek to start of file
	fseek( fp, 0, SEEK_SET );


	char lpszTmpBuffer[512];
	CUString strContent;

	
	while(!feof(fp))
	{
		lpszTmpBuffer[0] = '\0';

		fgets(	lpszTmpBuffer,
				sizeof(lpszTmpBuffer) - 1,
				fp );

		lpszTmpBuffer[ sizeof(lpszTmpBuffer)-1 ] = '\0';

		if (strlen(lpszTmpBuffer)>0)
		{
			if ( lpszTmpBuffer[ strlen(lpszTmpBuffer) - 1 ] == 0x0A )
			{
				lpszTmpBuffer[ strlen(lpszTmpBuffer) - 1 ] = '\0';
			}

            strContent += CUString( lpszTmpBuffer, CP_UTF8 );
            strContent += _W( "\r\n" );

			if ( TRUE == bUseMailTo )
			{
				DWORD nIdx;
				for ( nIdx = 0; nIdx < strlen( lpszTmpBuffer ); nIdx++ )
				{
					// straight conversion
					strMailBody+= lpszTmpBuffer[ nIdx ];

				}

				int nTest = strMailBody.GetLength();
				strMailBody+= _W( "\r\n" );
			}
			else
			{
				// SetInfoMsg( _W( "\r\n" ) + CUString( lpszTmpBuffer, CP_UTF8 ) );
				// SendString_UTF( lpszTmpBuffer );
			}
		}
	}

	// close file
	if ( NULL != fp )
	{
		fclose(fp);
	}


	if ( FALSE == bUseMailTo )
	{

        int contentLength = strContent.GetLength();

		// Determine length of entry
		strOut.Format( _W( "Content-Length: %d" ), contentLength );
		
        SendString( strOut );
        SetInfoMsg( _W( ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n" ) );

        SetInfoMsg( strOut + _W( "\r\n" ) );

		// One additional line feed
		SendString( _W ( "\r\n" ) );

        SendString( strContent );
        SetInfoMsg( strContent );

		// One additional line feed
		SendString( CUString( _T( "\r\n" ) ) );

		// One additional line feed
		SendString( CUString( _T( "\r\n" ) ) );

		// Get Server response
		if (GetString(m_strReceive)==SOCKET_ERROR)
		{
			return FALSE;
		}


		// Check if HTTP server is responding correctly
		if (m_strReceive.Find( _W( "HTTP/" ) ) != 0 || m_strReceive.Find( _W( "200" ) ) != 9 )
		{
			// Guess not
			strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );
            SetInfoMsg( _W( "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n" ) );
			SetInfoMsg( strLang + m_strReceive + _W( "\r\n" ) );
			bSubmitError=TRUE;
		}

		LTRACE( _T( "Submit header: \"%s\"" ), m_strReceive );

		// Get header response from HTTP server
		while(m_strReceive.GetLength()> 0)
		{
			if ( GetString( m_strReceive ) == SOCKET_ERROR )
			{
				return FALSE;
			}
			LTRACE( _T( "Submit header: \"%s\"" ), m_strReceive );
			SetInfoMsg( _W( "\r\n" ) + m_strReceive );
		}


		// Get body response from HTTP server
		if ( GetString( m_strReceive ) == SOCKET_ERROR )
		{
			return FALSE;
		}

		if ( m_strReceive.GetLength() > 0 )
		{
			// check result, when ok response is "200 OK"
			if ( m_strReceive.Find( _W( "200 OK" ) ) == 0  )
			{
				SetInfoMsg( m_strReceive + _W( "\r\n" ) );
				bSubmitError = FALSE;
			}
			else
			{
				strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );
				SetInfoMsg( strLang + m_strReceive + _W( "\r\n" ) );
				bSubmitError = TRUE;
			}
		}
		else
		{
			strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );
			SetInfoMsg( strLang +  _W( "\r\n" ) );
			return FALSE;
		}


		if ( bSubmitError )
		{
			return FALSE;
		}

		if ( CDEX_OK != CloseConnection() )
		{
			return FALSE;
		}
	}
	else
	{
		CUString strServer( g_config.GetCDDBESubmitServer() );
		CUString strMailFrom( g_config.GetEmailAddress() );
		CUString strMailTo( g_config.GetCDDBESubmitAddr() );
		
		int nPort = 25;

		if ( strServer.IsEmpty() )
		{
			strLang = g_language.GetString( IDS_CDDB_ERROR_UNKNOWNSMTPSERVER );

			SetInfoMsg( strLang );
			return FALSE;
		}
		if ( strMailFrom.IsEmpty() || ( strMailFrom.Find( _T( '@' ) ) < 0 ) )
		{
			strLang = g_language.GetString( IDS_INVALID_EMAIL_ADDRESS );
		
			SetInfoMsg( strLang );

			return FALSE;
		}

		if ( bTestMode )
		{
			strMailTo = _T( "test-submit@freedb.org" );
		}

		strServer.TrimLeft();
		strServer.TrimRight();

		return SendSmtp( strServer, nPort, strMailTo, strMailFrom, strMailSubject, strMailBody );

	}

	// Write the version with the update information ( like revision number )
	WriteCurrentEntry();

	return TRUE;
}


void CDdb::SetCategory( const CUString& strCategory )
{
	CUString		strLang;
	int			nCategory = 0;

// AF TODO	LTRACE( CUString( _T( "SetCategory to \"" ) + strCategory +"\"" ) );

	for ( nCategory = 0 ; nCategory < NumCategories ; nCategory ++ ) 
	{
		if ( strCategory == strCDDBCatagories[ nCategory ] )
		{
			m_pCDInfo->SetCDDBCat( strCategory );

			strLang = g_language.GetString( IDS_UNKNOWN );

			// Only set Genre when it is set to Unknown
			if (	0 == strLang.CompareNoCase( m_pCDInfo->GetGenre() ) ||
					m_pCDInfo->GetGenre().IsEmpty() )
			{
				LTRACE( _T( "m_pCDInfo->SetGenre to %s" ), CDDBCatToID3[ nCategory ] );
				m_pCDInfo->SetGenre( CDDBCatToID3[ nCategory ] );

			}
		}
	}
}


BOOL CDdb::GetHostName(CUString &szFullyQualifiedHostName)
{
    BOOL bReturnCode = FALSE;

    CHAR lpszComputerName[ MAX_COMPUTERNAME_LENGTH + 1 ];

    memset( lpszComputerName, 0, sizeof( lpszComputerName ) );

    if ( 0 == gethostname(	lpszComputerName, 
							sizeof( lpszComputerName ) ) )
	{
        // Get a fully qualified name
        hostent *pHostEnt = gethostbyname(lpszComputerName);

        if ( pHostEnt )
		{
            szFullyQualifiedHostName = CUString( pHostEnt->h_name, CP_UTF8 );
            bReturnCode = TRUE;
        }
		else
		{
            bReturnCode = FALSE;
        }
    } else
	{
        bReturnCode = FALSE;
    }

    return( bReturnCode );
}

BOOL CDdb::SendSmtpString( const CUString& strSend, int nOkValue )
{
	CUString	strLang;
	CUString strIn;
	CUString strOk;

	strOk.Format( _W( "%d" ), nOkValue );

	// Send string
	if ( SOCKET_ERROR == SendString( strSend ) )
	{
		LTRACE( _T( "Could not send string \"%s\"" ), strSend );

		strLang = g_language.GetString( IDS_CDDB_ERROR_FAILEDTOSENDSTRING );

		CDexMessageBox( strLang , MB_OK | MB_ICONERROR );

		return FALSE;
	}

	// Get response
	if ( SOCKET_ERROR == GetString( strIn ) )
	{
		LTRACE( _T( "No server response \"%s\"" ), strIn );

		strLang = g_language.GetString( IDS_CDDB_ERROR_NOSERVERRESPONSE );
		CDexMessageBox( strLang , MB_OK | MB_ICONERROR );

		return FALSE;
	}

	// Check Response
	if ( strIn.Find( strOk ) < 0 )
	{
		LTRACE( _T( "Invalid response string \"%s\"" ), strIn );

		strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );

		CDexMessageBox( strLang , MB_OK | MB_ICONERROR );

		return FALSE;
	}

	return TRUE;
}


BOOL CDdb::SmtpOpenConnection( const CUString& strAddr, int nPort )
{
	CUString strLang;
    CUStringConvert strCnv; 

	// Init the WINSOCK library
	InitWinSock();

	// Clear all errors
	WSASetLastError(0);

	// Create the socket
	BOOL bRet=m_Socket.Create();

	// Debug output
	LTRACE( _T( "Open SMTP connection =\"%s\" Port=%d" ), strCnv.ToT( strAddr ), nPort );

	// Open the socket connection
	if ( m_Socket.Connect( strCnv.ToT( strAddr ), nPort ) !=0 )
	{
		// Keep connection alive, value =0 is not keeping connection alive
		int iAlive=1;		 

		m_Socket.SetSockOpt( SO_KEEPALIVE, &iAlive, sizeof( BOOL ) );

		int iRecBufSize=sizeof( m_lpszBuffer );

		// Set receive buffer size to 8K
		m_Socket.SetSockOpt( SO_RCVBUF, &iRecBufSize, sizeof( int ) );
		
		return TRUE;
	}
	else
	{
		CUString strTmp;

		CUString strError( GetLastErrorString() );

		strLang = g_language.GetString( IDS_CDDB_CONNECTION_FAILED_FOR_REASON );

		strTmp.Format( _W( "%s\r\n%s\r\n" ), (LPCWSTR)strLang, (LPCWSTR)strError );

		SetInfoMsg( strTmp );

	}
	return FALSE;
}


int CDdb::SendStringQuotedPrintable( const CUString& strSend ) 
{
	CUString strLang;
    int		nLastSoftbreak = 0;
	int		nInIdx =0;
	int		nStrLen = 0;
	CUString strOut;

	for ( nInIdx =0; nInIdx< strSend.GetLength(); nInIdx++)
	{
		WCHAR ch = strSend[ nInIdx ];

        if ((ch >= 32 && ch <= 60) || (ch >= 62 && ch<= 126) || ( ch== 9 ) )
		{
            strOut+= ch;
		}
        else
		{
			if ( ch != 0x0A && ch != 0x0D )
			{
				char cHigh, cLow;

				cHigh = (char)( ( ch & 0xF0 ) >> 4 );
				cLow = (char)( ch & 0x0F );

				strOut+= '=';

				if (cHigh > 9)
					strOut+= (char)( 'A' + cHigh - 10 );
				else
					strOut+= (char)( '0' + cHigh );

				if (cLow > 9)
					strOut+= (char)( 'A' + cLow - 10 );
				else
					strOut+= (char)( '0' + cLow );
			}
        }

//        if ( strOut.GetLength() > 70 )
		if ( ch == 0x0A )

		{
			//strOut+= '=';

			SetInfoMsg( CUString( _T( "\r\n") ) + strOut );

			if ( SendString( strOut,TRUE ) == SOCKET_ERROR ) 
			{
				LTRACE( _T( "SendStringQuotedPrintable, error code %d" ), WSAGetLastError() );

				strLang = g_language.GetString( IDS_CDDB_ERROR_FAILEDTOSENDSTRING );

				SetInfoMsg( _W( "\r\n") + strLang + _W( " " ) + strOut );

				return SOCKET_ERROR;
			}

			strOut=_W("");
        }
    }

	SetInfoMsg( CUString( _T( "\r\n" ) ) + strOut );

	if ( 0 != strOut.CompareNoCase( _W( "=00" ) ) )
	{
		if ( SOCKET_ERROR == SendString( strOut ) ) 
		{
			LTRACE( _T( "SendStringQuotedPrintable, error code %d" ) , WSAGetLastError() );

			strLang = g_language.GetString( IDS_CDDB_ERROR_FAILEDTOSENDSTRING );

			SetInfoMsg( _W( "\r\n" ) + strLang + _W( " " ) + strOut );

			return SOCKET_ERROR;
		}
	}

	return 0;
}


BOOL CDdb::SendSmtp(
						const CUString& strServer,
						const int nPort,	
						const CUString& strMailTo,
						const CUString& strMailFrom,
						const CUString& strMailSubject,
						const CUString& strMailBody 
					)
{
	BOOL bUseMime = TRUE;

	// get the SMTP server address
	CUString strFROM;
	CUString strRCPT;
	CUString strIn;
	CUString strTo;
	CUString strFrom;
	CUString strSubject;
	CUString strBody;
	CUString strHelo( _T( "HELO " ) );
	CUString strHostName;
	CUString	strLang;

	strFROM.Format( _W( "MAIL FROM: <%s>" ), (LPCWSTR)strMailFrom );
	strRCPT.Format( _W( "RCPT TO: <%s>" ), (LPCWSTR)strMailTo );

	strFrom.Format( _W( "From: <%s>" ), (LPCWSTR)strMailFrom );
	strTo.Format( _W( "To: <%s>" ), (LPCWSTR)strMailTo );

	strSubject.Format( _W( "Subject: %s" ), (LPCWSTR)strMailSubject );
	strBody.Format( _W( "%s" ), (LPCWSTR)strMailBody );


	// Setup helo string
    if ( GetHostName( strHostName ) ) 
	{
        strHelo += strHostName;
    }

	strLang = g_language.GetString( IDS_CDDB_ACTION_CONNECTINGTOSERVER );
	SetInfoMsg( _W( "\r\n" ) + strLang + m_strServer );

	// Open the socket connection
	if ( SmtpOpenConnection( strServer, 25 ) )
	{

		LTRACE( _T( "Opened SMTP socket to %s" ), strServer );

		if ( SOCKET_ERROR == GetString( strIn ) )
		{
			LTRACE( _T( "Invalid response \"%s\"" ), strIn );

			strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );

			SetInfoMsg( _W( "\r\n" ) + strLang );

			CloseConnection();

			return FALSE;
		}

		// Check for valid sign-on status      
		if ( strIn.Left(3) != _T( "220" ) )
		{
			LTRACE( _T( "Server not ready" ) );

			strLang = g_language.GetString( IDS_CDDB_ACTION_SERVERNOTREADY );

			SetInfoMsg( _W( "\r\n" ) + strLang );

			CloseConnection();

			return FALSE;
		}

		SetInfoMsg( _W( "\r\nSending " ) + strHelo );

		// Send HELO string
		if ( FALSE == SendSmtpString( strHelo, 250 ) )
		{
			strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );

			SetInfoMsg( _W( "\r\n" ) + strLang );

			CloseConnection();

			return FALSE;
		}

		SetInfoMsg( _W( "\r\nSending " ) + strFROM );

		// Send FROM string
		if ( FALSE == SendSmtpString( strFROM, 250 ) )
		{
			strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );

			SetInfoMsg( _W( "\r\n" ) + strLang );

			CloseConnection();

			return FALSE;
		}

		strLang = g_language.GetString( IDS_CDDB_ACTION_SENDING );

		SetInfoMsg( _W( "\r\n" ) + strLang + _W( " " ) + strRCPT );

		// Send RCPT string
		if ( FALSE == SendSmtpString( strRCPT, 250 ) )
		{
			strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );


            SetInfoMsg( _W( "\r\n" ) + strLang );

			CloseConnection();

			return FALSE;
		}

		strLang = g_language.GetString( IDS_CDDB_ACTION_SENDINGDATA );
		SetInfoMsg( _W( "\r\n" ) + strLang );

		// Send DATA string
		if ( FALSE == SendSmtpString( CUString( _T( "DATA" ) ), 354 ) )
		{
			strLang = g_language.GetString( IDS_CDDB_INVALIDSERVERRESPONSE );

			SetInfoMsg( _W( "\r\n" ) + strLang );

			CloseConnection();

			return FALSE;
		}

		// ===================
		// START RFC822 HEADER
		// ===================

		strLang = g_language.GetString( IDS_CDDB_ACTION_SENDING );

		SetInfoMsg( _W( "\r\n" ) + strLang + _W( " " ) + strFrom );
		SendString( strFrom );

		SetInfoMsg( _W( "\r\n" ) + strLang + _W( " " ) + strTo );
		SendString( strTo );

		SetInfoMsg( _W( "\r\n" ) + strLang + _W( " " ) + strSubject );
		SendString( strSubject );

		SendString( CUString( _T( "X-Cddbd-Note: Sent by CDex " ) ) +
					GetClientVersion() +
					CUString( _T( "- Questions: cdex@hccnet.nl" ) ) );

		if ( 6 == gs_bProtoLevel )
		{
			SendString( _T( "Content-Type: text/plain; charset=utf-8" ) );
		}
		else
		{
			SendString( _T( "Content-Type: text/plain; charset=iso-8859-1" ) );
		}

	    if ( bUseMime )
		{
			SendString( _T( "MIME-Version: 1.0" ) );
			SendString( _T( "Content-Transfer-Encoding: quoted-printable" ) );
		}

		// Add empty line after header
		SendString( _T( "" ) );

		// END RFC822 HEADER - BEGIN BODY
		// ==============================
		// Send the termination line
		// Send DATA string

		strLang = g_language.GetString( IDS_CDDB_ACTION_SENDINGBODYTEXT );

		SetInfoMsg( strLang );

	    if ( bUseMime )
		{
			SendStringQuotedPrintable( strBody );
		}
		else
		{
			SendString( strBody );
		}

		if ( FALSE == SendSmtpString( CUString( _T( "\r\n." ) ), 250 ) )
		{
			SetInfoMsg( _W( "\r\nInvalid response" ) );
			CloseConnection();
			return FALSE;
		}

		// And were done
		SendString( _T( "QUIT" ) );

	}
	else
	{
		LTRACE( _T( "Failed to open SMTP socket to %s" ), strServer );
	}

	return TRUE;
}



CUString CDdb::GetClientVersion()
{
	CUString strVersion( g_config.GetVersion());
	CUString strRet;

	// Remove white spaces
	for ( int i=0; i<strVersion.GetLength(); i++)
	{
		if ( strVersion[i] != _T( ' ' ) )
			strRet += strVersion[ i ];
	}

	return strRet;
}


void CDdb::SetInfoMsg( const CUString& strMsg )
{

	if ( m_pWnd )
	{
		m_mInfoMsg.Lock();

		m_strInfoMsg += strMsg;

		m_mInfoMsg.Unlock();

		m_pWnd->PostMessage( WM_CDDB_INFO_MSG, 0, 0L );
	}
}


CUString	CDdb::GetCDDBAction()
{
	CUString strRet;

	m_mCDDBAction.Lock();

	strRet = m_strCDDBAction;

	m_mCDDBAction.Unlock();

	return strRet;
}

CUString	CDdb::GetInfoMsg()
{
	CUString strRet;

	m_mInfoMsg.Lock();

	strRet = m_strInfoMsg;

	m_strInfoMsg = _T( "" );

	m_mInfoMsg.Unlock();

	return strRet;
}

