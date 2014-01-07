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


#ifndef CDDB_H_INCLUDED
#define CDDB_H_INCLUDED

#include <AfxSock.h>
#include "CDdbDlg.h"
#include "TextFile.h"
#include "config.h"
#include "TagData.h"

/** 
 * CDInfo Foreward Class declaration
 */
class CDInfo;


/** 
 * Define the CDDB batch file name
 */
#define CDDB_BATCH_FNAME g_config.GetCDDBPath() + _W( "CDDB_Batch.txt" )
#define CDDB_BATCH_FNAME_TMP	( CDDB_BATCH_FNAME + _W( ".tmp" ) )


typedef enum CDDB_ERROR
{
    CDDB_ERROR_OK,
    CDDB_ERROR_SOCKET,
    CDDB_ERROR_CONNECT,
    CDDB_ERROR_FAILED,
    CDDB_ERROR_QUERY_FAILED,
    CDDB_ERROR_SEND_FAILED,
    CDDB_ERROR_INEXACT_MATCH,
    CDDB_ERROR_MULTIPLE_MATCH,
    CDDB_ERROR_NO_MATCH,
    CDDB_ERROR_INVALID_EMAIL_ADDRESS,
    CDDB_ERROR_RESPONSE,
};


/** 
 * CDdb is a class to obtain (load/get) or store (save/put) CD related information
 */
class CDdb
{

public:

    INT m_lastResponseCode;
    
    INT GetLastResponseCode() const { return m_lastResponseCode; }

    // class constructor
	CDdb(	
			CDInfo* pCDInfo,	//! pointer to a CDInfo object
			CWnd*	pWnd = NULL		//! pointer to Wnd object
		);

	// class destructor
	virtual ~CDdb();

	// Obtain CDDB data from local CDDB database    
    // return TRUE if query is successful, otherwise FALSE
	BOOL	QueryLocal();

    // Writes the current CDDB data to the local CDDB database
	BOOL	WriteCurrentEntry();

    // Obtain CDDB data from remote CDDB database
	BOOL	QueryRemote( );

    // Obtain a server list from the remote site
	BOOL	QuerySites();

	// Writes the current CDDB data to the remote CDDB database
	BOOL	SubmitRemote( );

	// Add or remove the current entry from the CDDB batch list
	BOOL	AddRemoveQueryBatch( //! returns FALSE if successful
									BOOL bAdd //! Add or remove entry
								);				//! returns TRUE if successful

	static CUString GetCDDBAction();
	static CUString GetInfoMsg();
	BOOL RemoveFromCDDBBatch( const CUString& strLine );
	BOOL	ParseData( const CUString& strToParse );


public:
    vector<CHAR>    m_vReadResult;

	CDDB_ERROR  SendProto();
    BOOL        GetMultipleMatches( CUString& strMatches );
	CDDB_ERROR  OpenConnection(CUString strAddress = _T( "" ),int nPort = 80 );
	CDDB_ERROR  CloseConnection();
	CDDB_ERROR  SendHello();
    CDDB_ERROR  SendQuery( CUString& strReadCat );
    CDDB_ERROR  ReadRemote( const CUString& strReadCatagory, CUString& strDiscID, vector<CHAR>& vReadChars );
    CUString     GetDiscIDString() const { return m_strDiscID; }
	void        SetCategory( const CUString& strCategory );

private:
	void		GetRealServerName();
	int			GetString( CUString& strRet);
	BOOL		SendStat();
	CDDB_ERROR  SendRemoteMessage( CUString& strSend, BOOL bQuery );
	int			SendString(CUString strSend, BOOL bAppendLineFeed=TRUE);
	int			SendString_UTF( CHAR* strSend, BOOL bAppendLineFeed=TRUE );
	CUString		CreateHttpSendString( CUString strSend );
	void		EncodeEmailAddress( CUString& strEmailAddress );
	int			SendAuthentication();
	CDDB_ERROR   CheckServerResponse( CUString strCheck, BOOL bQuery );

	BOOL		CheckForTestQuery( DWORD& dwDiscID, CUString& strQuery );
//	void		ReceiveMessage();

	BOOL	GetRemoteSites();

	CUString		GetWindowCDDBFileName( const CUString& strCategoryDir );
	CUString		GetCategoryDir( DWORD dwCategory );
	BOOL StoreToCDDBBatch( const CUString& strLine );

	BOOL	CheckFrameOffsets( CTextFile& cReadFile );
	LONG	SearchEntryInFiles( CUString& strFileName );
	LONG	SearchEntry( CUString& strFileName, CUString& strCategory );
	BOOL	ReadEntry( const CUString& strFileName, long lFilePos );
	BOOL	ReadRarEntry( const CUString& strFileName );

	void	WriteLine( CTextFile& cWriteFile, const CUString& strName, const CUString& strValue );
	BOOL	WriteEntry( const CUString& strFileName, bool bIsWindowsTypeDB, bool bIsDosFileType );
	BOOL	RemoveEntry( CUString& strFileName );
	BOOL	InitWinSock();
	CUString GenerateFileName( const CUString& strCategory );

	BOOL	GetHostName( CUString &szFullyQualifiedHostName );
	BOOL	SendSmtpString( const CUString& strSend, int nNotOkValue );
	BOOL	SmtpOpenConnection( const CUString& strAddr, int nPort );
	CUString EncodeBASE64( CUString szMessage );

	int		SendStringQuotedPrintable( const CUString& strSend );

	BOOL	SendSmtp(
						const CUString& strServer,
						const int nPort,
						const CUString& strMailTo,
						const CUString& strMailFrom,
						const CUString& strMailSubject,
						const CUString& strMailBody 
					);
	CUString GetClientVersion();



	void SetInfoMsg( const CUString& strMsg );



private:
    CUString		m_strReadID;
	CUString		m_strDiscID;
    CUString		m_strLocalPath;
	CDInfo*		m_pCDInfo;

    CUString     m_strLastResponseString;

	// A buffer to hold the receive data
	char		m_lpszBuffer[4*8192];

	// For server
    sockaddr_in sAddrServer;
	CUString		m_strServer;
	CSocket		m_Socket;

	BOOL		m_bConnected;
	CUString		m_strRemoteServer;

	CUString		m_strReadCat;
	CUString		m_strReceive;

	BOOL					m_bInexact;
	BOOL					m_bIsUnixCDDBType;
	BOOL					m_bUTF;

    CUString                 m_lastResponseString;
	static CWnd*			m_pWnd;
	static CMutex			m_mInfoMsg;
	static CUString			m_strInfoMsg;	
	static CMutex			m_mCDDBAction;
	static CUString			m_strCDDBAction;	
	const  CUString			m_strSubmitFileName;
};


#endif