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
#include "cdex.h"
#include "logging\logging.h"
#include "GetRemoteCDInfoDlg.h"
#include "cddb.h"
#include "CDInfo.h"
#include ".\cddbdlg.h"
#include ".\getremotecdinfodlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


INITTRACE( _T( "CDDBDlg" ) );

const UINT RTA_INIT    = 0x0000;
const UINT RTA_OPEN    = 0x0001;
const UINT RTA_QUERY   = 0x0002;
const UINT RTA_READ    = 0x0003; 
const UINT RTA_READ_FINISHED    = 0x0004; 
const UINT RTA_FINISH  = 0x0005; 

//const UINT CDS_READING  = 0x0001; 

/////////////////////////////////////////////////////////////////////////////
// CGetRemoteCDInfoDlg dialog

typedef struct
{
    TCHAR   name[ 255];
    DWORD   codePage;
} CODEPAGE_MAP;

CODEPAGE_MAP codePageMap[]=
{
    {_T("UTF-8"),CP_UTF8},{_T("Afrikaans"),1252}, {_T("Albania"),1250}, {_T("Arabic (Saudi Arabia)"),1256},
    {_T("Arabic (Iraq)"),1256}, {_T("Arabic (Egypt)"),1256}, {_T("Arabic (Libya)"),1256},
    {_T("Arabic (Algeria)"),1256}, {_T("Arabic (Morocco)"),1256}, {_T("Arabic (Tunisia)"),1256},
    {_T("Arabic (Oman)"),1256}, {_T("Arabic (Yemen)"),1256}, {_T("Arabic (Syria)"),1256},
    {_T("Arabic (Jordan)"),1256}, {_T("Arabic (Lebanon)"),1256}, {_T("Arabic (Kuwait)"),1256},
    {_T("Arabic (United Arab Emirates)"),1256}, {_T("Arabic (Bahrain)"),1256}, {_T("Arabic (Qatar)"),1256},
    {_T("Basque"),1252}, {_T("Byelorussian"),1251}, {_T("Bulgarian"),1251},
    {_T("Catalan"),1252}, {_T("Chinese (Taiwan)"),950}, {_T("Chinese (People's Republic of China)"),936}, {_T("Chinese (Hong Kong)"),950}, {_T("Chinese (Singapore)"),936}, {_T("Croatia"),1250},
    {_T("Czech"),1250}, {_T("Danish"),1252}, {_T("Dutch"),1252}, {_T("English"),1252}, 
    {_T("Estonian"),1257}, {_T("Faeroese"),1250},{_T("Farsi"),1256}, {_T("Finnish"),1252}, {_T("French"),1252},
    {_T("German"),1252}, {_T("Greek"),1253}, {_T("Hebrew"),1255}, {_T("Hungarian"),1250},
    {_T("Icelandic"),1252}, {_T("Indonesian"),1252}, {_T("Italian"),1252},{_T("Japanese"),932},
    {_T("Korean"),949}, {_T("Korean (Johab)"),1361}, {_T("Latvian"),1257}, {_T("Lithuanian"),1257},
    {_T("Norwegian (Bokm�l)"),1252}, {_T("Norwegian (Nynorsk)"),1252}, {_T("Polish"),1250},
    {_T("Portuguese"),1252}, {_T("Romanian"),1250}, {_T("Russian"),1251}, {_T("Serbian (Latin)"),1250},
    {_T("Slovak"),1250}, {_T("Slovenian"),1250}, {_T("Spanish"),1252}, {_T("Swedish"),1252}, {_T("Thai"),874}, {_T("Turkish"),1254}, {_T("Ukrainian"),1251}
};

CGetRemoteCDInfoDlg::CGetRemoteCDInfoDlg( CDInfo* pCDInfo, bool isBatch, CWnd* pParent )
: CLangDialog( CGetRemoteCDInfoDlg::IDD, pParent ),
m_localCDDB( pCDInfo )
{
    m_isBatch = isBatch;
    m_bShowMultiMatch = false;
    m_pCDInfo = pCDInfo;

    //	m_pThreadProc = pThreadProc;
    m_pThread = NULL;
    m_eThreadFinished.ResetEvent();
    //	m_pThreadParam = pThreadParam;
    m_bAbort = FALSE;
    m_bAbortThread = FALSE;
    m_strHist = _T( "" );
    //{{AFX_DATA_INIT(CGetRemoteCDInfoDlg)
    m_strAction = _T("");
    //}}AFX_DATA_INIT
    m_state = RTA_INIT;

    m_codePage = CP_UTF8;

}


void CGetRemoteCDInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CLangDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGetRemoteCDInfoDlg)
    DDX_Control(pDX, IDC_OUTPUT, m_Output);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_CODEPAGESELECTION, m_comboCodePage);
    DDX_Control(pDX, IDC_LIST1, m_listControl);
    DDX_Control(pDX, IDC_CDDB_READ, m_getButton);
    DDX_Control(pDX, IDC_CHECK1, m_resubmitCheck);
}


BEGIN_MESSAGE_MAP(CGetRemoteCDInfoDlg, CLangDialog)
    //{{AFX_MSG_MAP(CGetRemoteCDInfoDlg)
    ON_BN_CLICKED(IDC_ABORT, OnAbort)
    ON_BN_CLICKED(IDC_CDDB_RESUBMIT, OnCddbResubmit)
    //}}AFX_MSG_MAP

    ON_MESSAGE( WM_CDDB_INFO_MSG, OnInfoMsg )
    ON_MESSAGE( WM_CDDB_ACTION_MSG, OnActionMsg )
    ON_MESSAGE( WM_CDDB_REMOTE_FINISHED, OnFinished )
    ON_MESSAGE( WM_CDDB_CLOSE_DIALOG, OnCloseDialog )
    ON_EN_CHANGE(IDC_PREVIEW, OnEnChangePreview)
    ON_BN_CLICKED(IDC_CDDB_READ, OnBnClickedCddbRead)
    ON_CBN_SELCHANGE(IDC_CODEPAGESELECTION, OnCbnSelchangeCodepageselection)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnLvnItemchangedList1)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetRemoteCDInfoDlg message handlers


UINT CGetRemoteCDInfoDlg::RemoteThread( void* pParams )
{
    UINT returnValue = CDDB_ERROR_OK;

    ASSERT( pParams );

    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::RemoteThread( %p )" ), pParams );

    CGetRemoteCDInfoDlg* pDlg = (CGetRemoteCDInfoDlg*)pParams;

    returnValue = pDlg->OpenCDDBConnection();

    if ( returnValue == CDDB_ERROR_OK )
    {
        returnValue = pDlg->QueryFromCDDB();
    }

    if ( returnValue == CDDB_ERROR_OK )
    {
        returnValue = pDlg->ReadFromCDDB();
    }

    pDlg->CloseCDDBConnection();

    pDlg->PostMessage( WM_CDDB_REMOTE_FINISHED, returnValue );

    EXIT_TRACE( _T( "CGetRemoteCDInfoDlg::QueryRemote( ), return value %d" ), returnValue );

    pDlg->m_eThreadFinished.SetEvent();

    return returnValue;

}

CDDB_ERROR CGetRemoteCDInfoDlg::OpenCDDBConnection()
{
    CDDB_ERROR  returnValue = CDDB_ERROR_OK;

    m_state = RTA_OPEN;

    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::OpenCDDBConnection()" ) );

    SetAction( g_language.GetString( IDS_CDDB_ACTION_OPENCONNECTION ) );

    // Try to open remote connection
    returnValue = m_localCDDB.OpenConnection();

    // Send initial hand shake
    if ( returnValue == CDDB_ERROR_OK )
    {
        returnValue = m_localCDDB.SendHello();
    }

    if ( returnValue == CDDB_ERROR_OK )
    {
        returnValue = m_localCDDB.SendProto();
    }

    SetCDDBResponse( returnValue );

    EXIT_TRACE( _T( "CGetRemoteCDInfoDlg::OpenCDDBConnection(), return value %d" ), returnValue );

    return returnValue;
}

CDDB_ERROR CGetRemoteCDInfoDlg::CloseCDDBConnection()
{
    CDDB_ERROR returnValue = CDDB_ERROR_OK;

    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::CloseCDDBConnection()" ) );

    m_localCDDB.CloseConnection();

    EXIT_TRACE( _T( "CGetRemoteCDInfoDlg::CloseCDDBConnection(), return value %d" ), returnValue );

    return returnValue;
}


CUString CGetRemoteCDInfoDlg::GetCDDBResponseString( BOOL bQuery, UINT responseCode )
{
    CUString strLang;

    // Check response
    switch ( responseCode )
    {
    case 200: 
        strLang = g_language.GetString( IDS_CDDB_READWRITE_ALLOWED );
        break;
    case 201: 
        strLang = g_language.GetString( IDS_CDDB_OK_READONLY );
        break;
    case 202: 
        strLang = g_language.GetString( IDS_CDDB_NO_MATCH_FOUND );
        break;
    case 210:
        if ( bQuery == FALSE )
        {
            strLang = g_language.GetString( IDS_CDDB_OK_RESPONSE );
        }
        else 
        {
            strLang = g_language.GetString( IDS_CDDB_FOUND_MULTIPLE_EXACT_MATCHES );
        }
        break;
    case 211:
        strLang = g_language.GetString( IDS_CDDB_FOUND_MULTIPLE_INEXACT_MATCHES );
        break;
    case 401:
        strLang = g_language.GetString( IDS_CDDB_ENTRY_NOT_FOUND );
        break;
    case 402: 
        strLang = g_language.GetString( IDS_CDDB_ALREADY_SHOOK_HANDS );
        break;
    case 403:
        strLang = g_language.GetString( IDS_CDDB_DB_ENTRY_CORRUPT );
        break;
    case 409: 
        strLang = g_language.GetString( IDS_CDDB_NO_HANDSHAKE );
        break;
    case 431: 
        strLang = g_language.GetString( IDS_CDDB_INVALID_HANDSHAKE );
        break;
    case 432: 		
        strLang = g_language.GetString( IDS_NO_CONNECTION_ALLOWED );
        break;
    case 433: 
        strLang = g_language.GetString( IDS_NO_CONNECTION_ALLOWED );
        break;
    case 434: 
        strLang = g_language.GetString( IDS_NO_CONNECTION_LOAD_TOO_HIGH );
        break;
    case 501: 
        strLang = g_language.GetString( IDS_CDDB_ILLEGAL_PROTO_LEVEL );
        break;
    case 502: 
        strLang = g_language.GetString( IDS_CDDB_PROTOCOL_IS_CURRENT_LEVEL );
        break;
    default:
        strLang = g_language.GetString( IDS_CDDB_SERVER_ERROR );
    }
    return strLang;
}

CDDB_ERROR CGetRemoteCDInfoDlg::QueryFromCDDB()
{
    CDDB_ERROR returnValue = CDDB_ERROR_OK;

    m_state = RTA_QUERY;

    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::QueryFromCDDB()" ) );

    SetAction( g_language.GetString( IDS_CDDB_ACTION_SENDQUERY ) );

    m_strQueryResult = _W("" );
    returnValue = m_localCDDB.SendQuery( m_strQueryResult );

    if ( returnValue == CDDB_ERROR_OK )
    {
        m_strCatagory = m_strQueryResult;
        m_strDiskID   = m_localCDDB.GetDiscIDString();
    }

    SetCDDBResponse( returnValue );

    EXIT_TRACE( _T( "CDdb::QueryFromCDDB( ), return value %d" ), returnValue );

    return returnValue;
}

CDDB_ERROR CGetRemoteCDInfoDlg::ReadFromCDDB()
{
    CDDB_ERROR returnValue = CDDB_ERROR_OK;

    m_state = RTA_READ;

    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::ReadFromCDDB()" ) );

    m_vReadResult.clear();
    m_localCDDB.ReadRemote( m_strCatagory, m_strDiskID, m_vReadResult );


    SetCDDBResponse( returnValue );

    EXIT_TRACE( _T( "CDdb::QueryRemote( ), return value %d" ), returnValue );

    return returnValue;
}

LONG CGetRemoteCDInfoDlg::OnCloseDialog( WPARAM wParam,LPARAM lParam )
{
    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::OnCloseDialog( %d, %d )"), wParam, lParam );

    WaitForSingleObject( m_eThreadFinished, -1 );

    CLangDialog::OnCancel();

    EXIT_TRACE( _T( "CGetRemoteCDInfoDlg::OnCloseDialog( )" ) );

    return 0;
}


void CGetRemoteCDInfoDlg::OnCancel() 
{
    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::OnCancel()") );

    WaitForSingleObject( m_eThreadFinished, -1 );

    CLangDialog::OnCancel();

    EXIT_TRACE( _T( "CGetRemoteCDInfoDlg::OnCancel()") );
}


void CGetRemoteCDInfoDlg::AddText( const CUString& strAddText ) 
{
    CUStringConvert strCnv;
    
    m_strHist += strAddText;
    m_strHist += _W( "\r\n" );

    m_Output.SetWindowText( strCnv.ToT( m_strHist) );
    m_Output.LineScroll( m_Output.GetLineCount() - 1, 0 );
}

void CGetRemoteCDInfoDlg::SetAction( const CUString& strAction ) 
{
    m_strAction = strAction;
    PostMessage( WM_CDDB_ACTION_MSG, 0 );
}

void CGetRemoteCDInfoDlg::SetCDDBResponse( CDDB_ERROR returnCode ) 
{
    CUString strResponse;

    switch ( returnCode )
    {
    case CDDB_ERROR_OK:
        strResponse = g_language.GetString( IDS_CDDB_OK_RESPONSE );
        break;
    case CDDB_ERROR_SOCKET:
        strResponse = g_language.GetString( IDS_CDDB_CONNECTION_FAILED_FOR_REASON );
        strResponse += CUString( GetLastErrorString() );
        break;
    case CDDB_ERROR_CONNECT:
        strResponse = g_language.GetString( IDS_CDDB_ERROR_FAILED_TO_CONNECT );
        break;
    case CDDB_ERROR_FAILED:
        break;
    case CDDB_ERROR_QUERY_FAILED:
        break;
    case CDDB_ERROR_SEND_FAILED:
        break;
    case CDDB_ERROR_INEXACT_MATCH:
        strResponse = g_language.GetString( IDS_CDDB_INEXACT_MATCH );
        break;
    case CDDB_ERROR_MULTIPLE_MATCH:
        strResponse = g_language.GetString( IDS_CDDB_MULTIPLE_MATCHES );
        break;
    case CDDB_ERROR_NO_MATCH:
        strResponse = g_language.GetString( IDS_CDDB_NO_MATCH_FOUND );
        break;
    case CDDB_ERROR_INVALID_EMAIL_ADDRESS:
        strResponse = g_language.GetString( IDS_INVALID_EMAIL_ADDRESS );
        break;
    case CDDB_ERROR_RESPONSE:
        break;
    }

    // GetCDDBResponseString( FALSE, m_localCDDB.GetLastResponseCode() ) );

    SetResponse( strResponse );
}

void CGetRemoteCDInfoDlg::SetResponse( const CUString& strResponse ) 
{
    m_strResponse = strResponse;
    PostMessage( WM_CDDB_ACTION_MSG, 1 );
}

void CGetRemoteCDInfoDlg::OnAbort() 
{
    // Kill Thread

}

const int CDDB_BATCH_ERR_OK = 0;
const int CDDB_BATCH_ERR_INVALID_LINE = 1;
const int CDDB_BATCH_ERR_EMPTY = 2;


void CGetRemoteCDInfoDlg::ReadBatch() 
{
	TCHAR	lpszLine[ 2048 ] = { _T( '\0'),};
    TCHAR*  linePtr = NULL;

    m_vBatchLines.clear();
    FILE* fp = CDexOpenFile( CDDB_BATCH_FNAME, _W( "r" ) );

    if( fp != NULL )
    {
        do
        {
    	    linePtr = _fgetts( lpszLine, sizeof( lpszLine ), fp );
            if ( linePtr ) 
            {
                CUString strLine( linePtr, CP_ACP );
                strLine.Replace( _W( "\r" ), _W( "" ) );
                strLine.Replace( _W( "\n" ), _W( "" ) );
                m_vBatchLines.push_back( strLine );
            }
        } while ( linePtr != NULL );

        fclose( fp );
    }

}


INT CGetRemoteCDInfoDlg::SetupNextBatchEntry() 
{
	INT     returnValue = CDDB_BATCH_ERR_INVALID_LINE;

    TCHAR* linePtr = NULL;

    while ( returnValue == CDDB_BATCH_ERR_INVALID_LINE )
    {
        CUString strLine;

        if ( m_currentBatchIndex < m_vBatchLines.size() )
        {
            CUString strLine = m_vBatchLines[ m_currentBatchIndex ];

            CUStringConvert strCnv;

            linePtr = strCnv.ToT( strLine );

            if ( linePtr != NULL )
            {
	            TCHAR*	pStrTok = NULL;
	            INT		nTracks=0;
	            INT		i=0;

	            pStrTok= _tcstok( linePtr, _T( " " ) );

	            // Get CDDB disc ID
	            if ( pStrTok )
	            {
		            int nID=0;
		            _stscanf( pStrTok, _T( "%x" ), &nID );
		            m_pCDInfo->SetDiscID( nID );
		            pStrTok = _tcstok( NULL, _T( " " ) );
	            }

	            // Get number of tracks
	            if ( pStrTok )
	            {
		            nTracks = _ttoi( pStrTok );
		            m_pCDInfo->SetNumTracks( nTracks );
		            pStrTok = _tcstok( NULL, _T( " " ) );
	            }

	            if ( pStrTok )
	            {
		            // Get track offset
		            for (i=0;i<nTracks;i++)
		            {
			            DWORD dwSec=_ttoi( pStrTok );

			            m_pCDInfo->SetTrack( i, i + 1 );
			            m_pCDInfo->SetStartSector( i + 1, dwSec );

			            pStrTok = _tcstok( NULL, _T( " " ) );

                        if ( pStrTok == NULL )
                        {
                            break;
                        }
		            }
	            }

	            if ( pStrTok )
	            {
		            DWORD dwSec=_ttoi( pStrTok );
                    m_pCDInfo->SetTotalSecs( dwSec );

	                if ( m_pCDInfo->GetNumTracks() < 99 )
	                {
                        returnValue = CDDB_BATCH_ERR_OK;
	                }        
                }

                if ( returnValue != CDDB_BATCH_ERR_OK )
                {
                    // remove false entry
                    m_localCDDB.RemoveFromCDDBBatch( strLine );
                    m_currentBatchIndex++;
                }

            }
        }
        else
        {
            returnValue = CDDB_BATCH_ERR_EMPTY;
        }
    }
    return returnValue;
}


BOOL CGetRemoteCDInfoDlg::QueryNewEntry() 
{
    CUStringConvert strCnv;

    GetDlgItem( IDC_LIST1 )->EnableWindow( FALSE );
    GetDlgItem( IDC_CDDB_READ )->EnableWindow( FALSE );
    GetDlgItem( IDC_CDDB_READ )->SetWindowText( strCnv.ToT( g_language.GetString( IDS_CDDBDLG_NEXT ) ) );

    // try to do both actions at once
    m_threadAction = RTA_QUERY | RTA_READ;

    for ( int idx = 0; idx < sizeof( codePageMap ) /sizeof( codePageMap[0] ); idx++ )
    {
        m_comboCodePage.AddString( codePageMap[ idx ].name );
    }
    m_comboCodePage.SetCurSel( 0 );


    m_eThreadFinished.ResetEvent();

    m_bAbortThread = FALSE;

    // Start the remote query thread
    m_pThread = AfxBeginThread( RemoteThread, this );
    
    return TRUE;
}

BOOL CGetRemoteCDInfoDlg::OnInitDialog() 
{
    CLangDialog::OnInitDialog();

    INT returnValue = CDDB_BATCH_ERR_OK;


    if ( m_isBatch )
    {
        ReadBatch(); 
        m_currentBatchIndex = 0;
        returnValue = SetupNextBatchEntry();
        m_codePage = CP_UTF8;

    }

    if ( returnValue == CDDB_BATCH_ERR_OK )
    {
        QueryNewEntry(); 
    }

    return TRUE;  
}


void CGetRemoteCDInfoDlg::OnOK() 
{
    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::OnOK") );

    WaitForSingleObject( m_eThreadFinished, -1 );

    // Write data
    ParseAndWriteData();

    if ( m_resubmitCheck.GetCheck() )
    {
        m_localCDDB.SubmitRemote();
    }
    

    bool closeDialog = true;

    if ( m_isBatch )
    {
        m_localCDDB.RemoveFromCDDBBatch( m_vBatchLines[ m_currentBatchIndex ] );

        m_currentBatchIndex++;

        INT returnValue = SetupNextBatchEntry();
        if ( returnValue == CDDB_BATCH_ERR_OK )
        {
            m_codePage = CP_UTF8;
            closeDialog = false;
            QueryNewEntry(); 
        }
    }

    if ( closeDialog )
    {
        // Call standard OnOK
        CLangDialog::OnOK();
    }

    EXIT_TRACE( _T( "CGetRemoteCDInfoDlg::OnOK()" ) );

}


LONG CGetRemoteCDInfoDlg::OnInfoMsg( WPARAM wParam,LPARAM lParam )
{

    CUString strNew = CDdb::GetInfoMsg();

    if ( strNew.GetLength() )
    {
        AddText( strNew );
    }
    return 0;
}

LONG CGetRemoteCDInfoDlg::OnActionMsg( WPARAM wParam,LPARAM lParam )
{
    switch ( wParam )
    {
    case 0:
        AddText( m_strAction );
        break;
    case 1:
        AddText( m_strResponse );
        break;
    }
    return 0;
}


CUString RemoveTokenWithSeparators( CUString& string, LPCWSTR charset) {

    CUString token = string.SpanExcluding( charset );
    string = string.Mid( token.GetLength() + 1 );
    return token;
}

void CGetRemoteCDInfoDlg::DisplayMultiMatch()
{
    m_bShowMultiMatch = true;

    int nColumnCount = m_listControl.GetHeaderCtrl()->GetItemCount();

    // Delete all of the columns.
    for (int i=0;i < nColumnCount;i++)
    {
        m_listControl.DeleteColumn(0);
    }
    CRect	rcList;

    // Get size position of Track List
    m_listControl.GetWindowRect( rcList );

    CUString strLang = g_language.GetString(IDS_GENRE);

    m_listControl.DeleteAllItems();

    CUStringConvert strCnv;
 
    m_listControl.InsertColumn( 0, strCnv.ToT( strLang ), LVCFMT_LEFT, rcList.Width()/6 -1, 0 );

    strLang = g_language.GetString(IDS_CDDBID);
    m_listControl.InsertColumn( 1, strCnv.ToT( strLang ), LVCFMT_LEFT, rcList.Width()/6 -1, 0 );

    strLang = g_language.GetString(IDS_ALBUM);
    m_listControl.InsertColumn( 2, strCnv.ToT( strLang ), LVCFMT_LEFT, rcList.Width()*2/3 -1, 0 );

    int item = 0;
    CUString s( m_strQueryResult );
    while (! s.IsEmpty()) {

        CUString token = RemoveTokenWithSeparators( s, _W( "\n" ) );
        if  (token.GetLength() == 0 )
        {
            break;
        }


        int nPrevPos=0;
        int nPos = token.Find( _T( ' ' ) );
        if (nPos>=0)
        {
            CUStringConvert strCnv;
            m_listControl.InsertItem( item, strCnv.ToT( token.Left( nPos-nPrevPos ) ) );
            // m_listControl.SetItemText( item, 0, token.Left( nPos-nPrevPos );
            token= token.Right( token.GetLength() - nPos - 1 );

            nPos= token.Find( _T( ' ' ) );
            if (nPos>=0)
            {
                m_listControl.SetItemText( item, 1, strCnv.ToT( token.Left( nPos-nPrevPos ) ) );
                m_listControl.SetItemText( item, 2, strCnv.ToT( token.Right(token.GetLength() - nPos - 1 ) ) );
            }
            item++;
        }
    }

    GetDlgItem( IDC_CDDB_READ )->EnableWindow( TRUE );
    m_listControl.SetCurSel( 0 );

    GetDlgItem( IDC_LIST1 )->SetFocus();
}

void CGetRemoteCDInfoDlg::ParseAndWriteData()
{
    CUString strLang = g_language.GetString( IDS_UNKNOWN );

    m_pCDInfo->SetGenre( strLang );

    m_pCDInfo->SetYear( _T( "" ) );

    // set proper category
    m_localCDDB.SetCategory( m_strCatagory );

    // Parse data and store it into the CD Info structure
    m_localCDDB.ParseData( m_strReadResult );

    // Save data to data base
    if ( g_config.GetSaveToLocCDDB() )
    {
	    m_localCDDB.WriteCurrentEntry();
    }
}

LONG CGetRemoteCDInfoDlg::OnFinished(WPARAM wParam,LPARAM lParam)
{
    ENTRY_TRACE( _T( "CGetRemoteCDInfoDlg::OnFinished( %d, %d )"), wParam, lParam );

    m_bShowMultiMatch = false;

    GetDlgItem( IDC_LIST1 )->EnableWindow( TRUE );

    if ( (wParam == CDDB_ERROR_INEXACT_MATCH ) || (wParam == CDDB_ERROR_MULTIPLE_MATCH ) )
    {
        m_bShowMultiMatch = true;
        DisplayMultiMatch();
    }
    int nColumnCount = m_listControl.GetHeaderCtrl()->GetItemCount();

    // Delete all of the columns.
    for (int i=0;i < nColumnCount;i++)
    {
        m_listControl.DeleteColumn(0);
    }
    CRect	rcList;

    // Get size position of Track List
    m_listControl.GetWindowRect( rcList );

    CUString strLang = g_language.GetString(IDS_GENRE);
    m_listControl.DeleteAllItems();

    CUStringConvert strCnv;

    m_listControl.InsertColumn( 0, strCnv.ToT( strLang ), LVCFMT_LEFT, rcList.Width()/6 -1, 0 );

    strLang = g_language.GetString(IDS_CDDBID);
    m_listControl.InsertColumn( 1, strCnv.ToT( strLang ), LVCFMT_LEFT, rcList.Width()/6 -1, 0 );

    strLang = g_language.GetString(IDS_ALBUM);
    m_listControl.InsertColumn( 2, strCnv.ToT( strLang ), LVCFMT_LEFT, rcList.Width()*2/3 -1, 0 );


    int item = 0;
    CUString s( m_strQueryResult );
    while (! s.IsEmpty()) {
        CUString token = RemoveTokenWithSeparators( s, _W( "\n" ) );
        if  (token.GetLength() == 0 )
        {
            break;
        }

        int nPrevPos=0;
        int nPos = token.Find( _T( ' ' ) );
        if (nPos>=0)
        {
            CUStringConvert strCnv;
            
            m_listControl.InsertItem( item, strCnv.ToT( token.Left( nPos-nPrevPos ) ) );
            // m_listControl.SetItemText( item, 0, token.Left( nPos-nPrevPos );
            token= token.Right( token.GetLength() - nPos - 1 );

            nPos= token.Find( _T( ' ' ) );
            if (nPos>=0)
            {
                m_listControl.SetItemText( item, 1, strCnv.ToT( token.Left( nPos-nPrevPos ) ) );
                m_listControl.SetItemText( item, 2, strCnv.ToT( token.Right(token.GetLength() - nPos - 1 ) ) );
            }
            m_listControl.SetCurSel( item );
            item++;
        }



    }
    if ( CDDB_ERROR_OK == wParam )
    {
        
        // Enable OK button
        // GetDlgItem( IDOK )->EnableWindow( TRUE );

        if ( !SetPreviewData() )
        {
            SetResponse( g_language.GetString( IDS_CDDB_TEXT_IS_NOT_ASCII ) );
            GetDlgItem( IDC_CDDB_READ )->EnableWindow( TRUE );
        }
        else
        {            
            // close dialog
            OnOK();        
        }

        if (  m_currentBatchIndex == m_vBatchLines.size() - 1 )
        {
            GetDlgItem( IDC_CDDB_READ )->SetWindowText( strCnv.ToT( g_language.GetString( IDS_CDDBDLG_FINISH ) ));
            GetDlgItem( IDC_CDDB_READ )->EnableWindow( TRUE );
        }

    }
    else
    {
        // No errors have occured, just close dialog box
        //		OnOK();
    }

    m_listControl.SetCurSel( 0 );

    EXIT_TRACE( _T( "CGetRemoteCDInfoDlg::OnFinished()") );
    return 0;
}

void CGetRemoteCDInfoDlg::OnCddbResubmit() 
{
}

void CGetRemoteCDInfoDlg::OnEnChangePreview()
{
}

void CGetRemoteCDInfoDlg::OnBnClickedCddbRead()
{
    UINT returnValue = CDDB_ERROR_OK;

    GetDlgItem( IDC_CDDB_READ )->EnableWindow( FALSE );

    switch ( m_state )
    {
    case RTA_READ:
        OnOK();
        break;
    case RTA_QUERY:
        {
            GetDlgItem( IDC_LIST1 )->EnableWindow( FALSE );


            m_bShowMultiMatch = false;

            int curSel = m_listControl.GetCurSel( );

            if ( curSel >= 0 )
            {
                m_strCatagory = CUString( m_listControl.GetItemText( curSel, 0 ) );
                m_strDiskID = CUString( m_listControl.GetItemText( curSel, 1 ) ) ;
            }

            returnValue = OpenCDDBConnection();


            if ( returnValue == CDDB_ERROR_OK )
            {
                returnValue = ReadFromCDDB();
            }

            CloseCDDBConnection();

            if ( !SetPreviewData() )
            {
                SetResponse( g_language.GetString( IDS_CDDB_TEXT_IS_NOT_ASCII ) );
            }
            else
            {            
                OnOK();        
            }
        }
        break;
    }
}

bool IsValidUTF8( CHAR* pszData )
{
    int vLength;

    for (; *pszData; ++pszData)
    {
        if ((*pszData & 0xFE) == 0xFC)
            vLength = 5;
        else if ((*pszData & 0xFC) == 0xF8)
            vLength = 4;
        else if ((*pszData & 0xF8) == 0xF0)
            vLength = 3;
        else if ((*pszData & 0xF0) == 0xE0)
            vLength = 2;
        else if ((*pszData & 0xE0) == 0xC0)
            vLength = 1;
        else if ((*pszData & 0x80) == 0x00)
            vLength = 0;
        else
            return false;

        if (vLength > 0)
        {
            // vEncoding = kUtf8;
        }
        while (0 < vLength--)
            if ((*++pszData & 0xC0) != 0x80)
                return false;
    }
    return true;
}

bool CGetRemoteCDInfoDlg::SetPreviewData()
{
    CUStringConvert strCnv;

    bool isPlainASCII  = true;

    int nColumnCount = m_listControl.GetHeaderCtrl()->GetItemCount();

    GetDlgItem( IDC_LIST1 )->EnableWindow( TRUE );

    // Delete all of the columns.
    for (int i=0;i < nColumnCount;i++)
    {
        m_listControl.DeleteColumn(0);
    }

    CRect	rcList;

    // Get size position of Track List
    m_listControl.GetWindowRect( rcList );

    // AF TODO
    
    CUString strLang = g_language.GetString( IDS_CDDB_PREVIEW );
    m_listControl.DeleteAllItems();
    m_listControl.InsertColumn( 0, strCnv.ToT( strLang ), LVCFMT_LEFT, rcList.Width(), 0 );

    int size = m_vReadResult.size();

    LPCSTR lpszInData = &m_vReadResult[0];
    vector<WCHAR> vOutData( m_vReadResult.size() + 1 );

    // convert from ANSI code page to wide string
	int nResultChars = MultiByteToWideChar(m_codePage, 0, lpszInData, -1, &vOutData[0], m_vReadResult.size());
	if (nResultChars == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION)
		isPlainASCII = false;

	for (int i = 0; i < nResultChars; i++)
	{
		if (vOutData[i] == 0xFFFD)
			isPlainASCII = false;
	}

    //if ( m_codePage == CP_UTF8 )
    //{
    //    LPWSTR tmpWideString[1024];

    //    MultiByteToWideChar( CP_UTF8, 0, lpszInData, -1, tmpWideString, 1023 );
    //    // convert from wide string back to ANSI Current Code Page
    // WideCharToMultiByte( m_codePage , 0, wideCharString, -1, destString, destStringSizeTChars, NULL, NULL );
    //
    //}

    LPWSTR lpwTest = &vOutData[0];

    CUString s( &vOutData[0] );
    m_strReadResult = s;

    int sizeTest = vOutData.size();

    int item = 0;
    while (! s.IsEmpty()) {
        CUString token = RemoveTokenWithSeparators( s, _W( "\n" ) );
        if  (token.GetLength() == 0 )
        {
            break;
        }
        if ( token.Find( _W( "TITLE" ) )  > 0  )
        {
            m_listControl.InsertItem( item, strCnv.ToT( token ) );
            item++;
        }
    }

    m_listControl.SetScrollRange( SB_VERT, 0,item-1 );
    m_listControl.SetScrollPos( SB_VERT, item-1 );


    return isPlainASCII;
    // return IsValidUTF8( (CHAR*)&vOutData[0] );
}

void CGetRemoteCDInfoDlg::OnCbnSelchangeCodepageselection()
{
    int selection = m_comboCodePage.GetCurSel();
    bool isNotUTF = ( selection > 0 );
    
    m_resubmitCheck.SetCheck( isNotUTF );

    if ( selection >= 0 )
    {
        m_codePage = codePageMap[ selection ].codePage;
        if ( m_bShowMultiMatch )
        {
            DisplayMultiMatch();
        }
        else
        {
            SetPreviewData();
        }
    }

    SetResponse( g_language.GetString( IDS_CDDB_CODEPAGE_RESUBMIT_GUIDANCE ) );

}

void CGetRemoteCDInfoDlg::OnBnClickedOk()
{
}

void CGetRemoteCDInfoDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

    //GetDlgItem( IDC_CDDB_READ )->EnableWindow( TRUE );
    //GetDlgItem( IDC_LIST1 )->SetFocus();

    *pResult = 0;
}

void CGetRemoteCDInfoDlg::OnBnClickedCancel()
{
    OnCancel();
}
