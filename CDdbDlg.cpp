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
#include "CDdbDlg.h"
#include "cddb.h"
#include ".\cddbdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "CDDBDlg" ) );

/////////////////////////////////////////////////////////////////////////////
// CCDdbDlg dialog


UINT CCDdbDlg::RemoteThread( void* pParams )
{
	BOOL	bSuccess = FALSE;

	ENTRY_TRACE( _T( "CCDdbDlg::RemoteThread( %p )" ), pParams );

	ASSERT( pParams );

	CCDdbDlg* pDlg = (CCDdbDlg*)pParams;

	// Call the desired function
	bSuccess = pDlg->m_pThreadProc( pDlg->m_pThreadParam, pDlg, pDlg->m_bAbort );

	LTRACE( _T( "CCDdbDlg::RemoteThread( ), threadproc returns %d " ), bSuccess );
	
	LTRACE( _T( "CCDdbDlg::RemoteThread( ), PostMessage( %d, 0 )" ), bSuccess );

	// Notifiy dialog that were done
	pDlg->PostMessage( WM_CDDB_REMOTE_FINISHED, bSuccess , 0 );
	
	LTRACE( _T( "CCDdbDlg::RemoteThread( ), SetEvent()" ) );

	// Set event
	pDlg->m_eThreadFinished.SetEvent();

	EXIT_TRACE( _T( "CCDdbDlg::RemoteThread( )" ) );
	return 0;
}


CCDdbDlg::CCDdbDlg( CDDBTHREADPROC  pThreadProc,void* pThreadParam,CWnd* pParent /*=NULL*/)
	: CLangDialog( CCDdbDlg::IDD, pParent )
{
	m_pThreadProc = pThreadProc;
	m_pThread = NULL;
	m_eThreadFinished.ResetEvent();
	m_pThreadParam = pThreadParam;
	m_bAbort = FALSE;
	m_strHist = _T( "" );
}


void CCDdbDlg::DoDataExchange(CDataExchange* pDX)
{
    CLangDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CCDdbDlg)
    DDX_Control(pDX, IDC_CURACTION, m_Action);
    DDX_Control(pDX, IDC_OUTPUT, m_Output);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCDdbDlg, CLangDialog)
	//{{AFX_MSG_MAP(CCDdbDlg)
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	ON_BN_CLICKED(IDC_CDDB_RESUBMIT, OnCddbResubmit)
	//}}AFX_MSG_MAP

	ON_MESSAGE( WM_CDDB_INFO_MSG, OnInfoMsg )
	ON_MESSAGE( WM_CDDB_ACTION_MSG, OnActionMsg )
	ON_MESSAGE( WM_CDDB_REMOTE_FINISHED, OnFinished )
	ON_MESSAGE( WM_CDDB_CLOSE_DIALOG, OnCloseDialog )
    ON_EN_CHANGE(IDC_PREVIEW, OnEnChangePreview)
    ON_CBN_SELCHANGE(IDC_CDDB_DLG_CAT, OnCbnSelchangeCddbDlgCat)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCDdbDlg message handlers

LONG CCDdbDlg::OnCloseDialog( WPARAM wParam,LPARAM lParam )
{
	ENTRY_TRACE( _T( "CCDdbDlg::OnCloseDialog( %d, %d )"), wParam, lParam );

	if ( WAIT_OBJECT_0 != WaitForSingleObject( m_eThreadFinished, 2000 ) )
	{
		LTRACE( _T( "CCDdbDlg::OnCloseDialog( ), timeout" ) );

		if ( m_pThread )
		{
			LTRACE( _T( "CCDdbDlg::OnCloseDialog( ), terminate thread" ) );
			TerminateThread( m_pThread->m_hThread, -999 );		
		}
	}

	CLangDialog::OnCancel();

	EXIT_TRACE( _T( "CCDdbDlg::OnCloseDialog( )" ) );

	return 0;
}


void CCDdbDlg::OnCancel() 
{
	ENTRY_TRACE( _T( "CCDdbDlg::OnCancel()") );

	m_bAbort = TRUE;
	PostMessage( WM_CDDB_CLOSE_DIALOG );

	EXIT_TRACE( _T( "CCDdbDlg::OnCancel()") );
}


void CCDdbDlg::AddText( const CUString& strAddText ) 
{
//	m_Output.ReplaceSel( strAddText );

	int nDeleteLines = m_Output.GetLineCount() - 500;

	while ( nDeleteLines > 0 )
	{
		int nPos = m_strHist.Find( _T( "\r\n" ) );
		if ( nPos >= 0 )
		{
			m_strHist = m_strHist.Right( m_strHist.GetLength() - nPos - 2 );
		}
		nDeleteLines--;
	}

	m_strHist += strAddText;

	m_Output.SetWindowText( m_strHist );

	m_Output.LineScroll( m_Output.GetLineCount() - 1, 0 );

}

void CCDdbDlg::SetAction( const CUString& strAction ) 
{
    CUStringConvert strCnv;
    m_Action.SetWindowText( strCnv.ToT( strAction ) );
  // AF TODO  GetWindow( IDC_CURACTION )->SetWindowText( strCnv.ToT( strAction ) );
	UpdateData( FALSE );
}

void CCDdbDlg::OnAbort() 
{
	// Kill Thread

}

BOOL CCDdbDlg::OnInitDialog() 
{
	CLangDialog::OnInitDialog();

	m_eThreadFinished.ResetEvent();

	// Start the remote query thread
	m_pThread = AfxBeginThread( RemoteThread, this );
	
	return TRUE;  
}


void CCDdbDlg::OnOK() 
{
	// Call standard OnOK
	CLangDialog::OnOK();
}


LONG CCDdbDlg::OnInfoMsg( WPARAM wParam,LPARAM lParam )
{

	CUString strNew = CDdb::GetInfoMsg();

	if ( strNew.GetLength() )
	{
		AddText( strNew );
	}
	return 0;
}

LONG CCDdbDlg::OnActionMsg( WPARAM wParam,LPARAM lParam )
{
	SetAction( CDdb::GetCDDBAction() );
	return 0;
}

LONG CCDdbDlg::OnFinished(WPARAM wParam,LPARAM lParam)
{
//	DWORD dwExitCode = 999;

	::Sleep( 10 );

	ENTRY_TRACE( _T( "CCDdbDlg::OnFinished( %d, %d )"), wParam, lParam );

/*	if ( m_pThread )
	{
		GetExitCodeThread( m_pThread->m_hThread, &dwExitCode );
	}
*/
	// Wait for thread to Finish
	if ( WAIT_OBJECT_0 != WaitForSingleObject( m_eThreadFinished, 2000 ) )
	{
		if ( m_pThread )
		{
			LTRACE( _T( "CCDdbDlg::OnFinished(), kill thread") );
			TerminateThread( m_pThread->m_hThread, 999 );
		}
	}

	if ( 0 == wParam )
	{
		CUString strLang;

		strLang = g_language.GetString( IDS_CDDB_ERROR_SEE_MSGXBOX );

		// Enable OK button
		GetDlgItem( IDOK )->EnableWindow( TRUE );

		// Set Status
		SetAction( strLang );
	}
	else
	{
		// No errors have occured, just close dialog box
		OnOK();
	}

	EXIT_TRACE( _T( "CCDdbDlg::OnFinished()") );
	return 0;
}

void CCDdbDlg::OnCddbResubmit() 
{
}

void CCDdbDlg::OnEnChangePreview()
{
}

void CCDdbDlg::OnCbnSelchangeCddbDlgCat()
{
    // TODO: Add your control notification handler code here
}
