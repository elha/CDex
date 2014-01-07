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


#include "StdAfx.h"
#include <AfxWin.h>
#include "CDex.h"
#include "HyperLink.h"

#include "MainFrm.h"
#include "resource.h"
#include "config.h"
#include "cdrip\CdRip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LANG_OPTION_MENU_POS	(5)
#define LANG_LANGUAGE_MENU_POS	(7)

#ifndef _countof
	#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif


/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_VIEWPLAYBAR, OnViewplaybar)
	ON_UPDATE_COMMAND_UI(ID_VIEWPLAYBAR, OnUpdateViewplaybar)
	ON_COMMAND(ID_VIEWRECORDBAR, OnViewrecordbar)
	ON_UPDATE_COMMAND_UI(ID_VIEWRECORDBAR, OnUpdateViewrecordbar)
	ON_WM_ENTERIDLE()
	ON_UPDATE_COMMAND_UI(ID_VIEWUTILTOOLBAR, OnUpdateViewutiltoolbar)
	ON_COMMAND(ID_VIEWUTILTOOLBAR, OnViewutiltoolbar)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
	ON_COMMAND_RANGE( ID_LANGUAGE_START, ID_LANGUAGE_END, OnLanguageSelection)
	// toolbar "tooltip" notification
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnMyToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnMyToolTipText)

END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR,           // status line indicator
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_pPopupMenu = NULL;
	m_nLangOptionMenuPos = LANG_OPTION_MENU_POS;
	m_nLangLangMenuPos   = LANG_LANGUAGE_MENU_POS;
	m_nMenuID			 = IDR_MAINFRAME;
}

CMainFrame::~CMainFrame()
{
	delete m_pPopupMenu;
// ===========================================================
    m_bmToolbarHi.DeleteObject();
// ===========================================================

}

bool CMainFrame::SetLanguageMenu()
{
	CMenu* pParentMenu = GetMenu();
	CMenu* pMenu = GetMenu()->GetSubMenu( m_nLangOptionMenuPos );
    
    CUStringConvert strCnv;

	ASSERT( pMenu );

	if ( NULL == m_pPopupMenu )
	{
		m_pPopupMenu = new CMenu;

		m_pPopupMenu->CreatePopupMenu();

		CUString strAdd( g_language.GetString( IDS_CHECKFORADDITIONAL_LANGUAGE_FILES ) );

        CUStringConvert strCnv;

		// Add Check for Updates
        m_pPopupMenu->AppendMenu( MF_STRING | MF_ENABLED, ID_LANGUAGE_START, strCnv.ToT( strAdd ) );

		// Add language menu items
		for ( DWORD idx = 0; idx < g_language.GetNumLanguageStrings() ; idx++ )
		{

			CUString strAdd( g_language.GetLanguageString( idx ) );
			m_pPopupMenu->AppendMenu( MF_STRING | MF_ENABLED, ID_LANGUAGE_START + idx + 1, strCnv.ToT( strAdd ) );
		}
	}

	CString strCurrent;
	pMenu->GetMenuString( m_nLangLangMenuPos, strCurrent, MF_BYPOSITION );

	// Add language menu items
	for ( DWORD idx = 0; idx < m_pPopupMenu->GetMenuItemCount(); idx++ )
	{
		CString strMenu;

		if ( 0 == idx )
		{
			m_pPopupMenu->ModifyMenu(	idx, 
										MF_BYPOSITION | MF_STRING,
										ID_LANGUAGE_START,
										strCnv.ToT( g_language.GetString( IDS_CHECKFORADDITIONAL_LANGUAGE_FILES ) ));

		}

		// get menu string
		m_pPopupMenu->GetMenuString( idx, strMenu, MF_BYPOSITION );

		// check if this is the selected language
		if ( 0 == g_language.GetLanguage().CompareNoCase( CUString( strMenu ) ) )
		{
			m_pPopupMenu->CheckMenuItem( idx, MF_BYPOSITION | MF_CHECKED  );
		}
		else
		{
			m_pPopupMenu->CheckMenuItem( idx, MF_BYPOSITION | MF_UNCHECKED  );
		}
	}

	// remove exisiting menu item
	pMenu->RemoveMenu( m_nLangLangMenuPos, MF_BYPOSITION );

	// insert new popup menu
	pMenu->InsertMenu(	m_nLangLangMenuPos,
						MF_BYPOSITION | MF_POPUP, 
						(UINT)m_pPopupMenu->m_hMenu, 
						strCurrent );


	// peform translation
	g_language.TranslateMenu( GetMenu(), m_nMenuID );

	// refresh frame windows
	ActivateFrame( SW_HIDE );
	ActivateFrame( SW_SHOW );

	return true;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect	myRect=CRect(
						g_config.GetIni().GetValue( _W( "RipperFrame"), _W( "FrameWndLeft"), 0 ),
						g_config.GetIni().GetValue( _W( "RipperFrame"), _W( "FrameWndTop"), 0 ),
						g_config.GetIni().GetValue( _W( "RipperFrame"), _W( "FrameWndRight"), 580 ),
						g_config.GetIni().GetValue( _W( "RipperFrame"), _W( "FrameWndBottom"), 390 )
					);

	// Move the window
	if ( myRect.Width() < 100 || myRect.Height() < 100 )
	{
		myRect=CRect( 0, 0, 640, 400 );
	}

	MoveWindow(myRect);


	// Create status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	CRect rcClient;
	GetClientRect(rcClient);

	m_wndStatusBar.SetPaneInfo(1,ID_SEPARATOR,SBPS_NORMAL,320);
	m_wndStatusBar.SetPaneInfo(0,ID_SEPARATOR,SBPS_STRETCH,100);

	// Docking is OK
	EnableDocking(CBRS_ALIGN_ANY);

	// Create main toolbar
	if (!m_wndToolBar.Create(this,WS_CHILD|WS_VISIBLE|CBRS_TOP,AFX_IDW_TOOLBAR) || !m_wndToolBar.LoadToolBar(IDW_RECBAR) )
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

//	m_wndToolBar.GetToolBarCtrl().AddBitmap(7,IDB_RECBAR256);
// ===========================================================
#ifdef OLDICONS
	if (::GetDeviceCaps(GetDC()->m_hDC,BITSPIXEL)>8) 
	{
		m_bmToolbarHi.LoadMappedBitmap(IDB_RECBAR256);
		m_wndToolBar.SetBitmap( (HBITMAP)m_bmToolbarHi );
	}
#else
// ===========================================================

	if ( UseFlatLookToolBar() )
	{
		CImageList	imageList;
		CBitmap		bitmap;

//		ModifyStyle(0, TBSTYLE_FLAT);

		// Set up hot bar image lists.
		// Create and set the normal toolbar image list.
		bitmap.LoadMappedBitmap( IDB_RECBAR256 );
		imageList.Create(32, 32, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		m_wndToolBar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();

		// Create and set the hot toolbar image list.
//		bitmap.LoadBitmap( IDB_CDPLAYTOOLBAR_H );
//		imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1);
//		imageList.Add(&bitmap, RGB(255,0,255));
//		SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
//		imageList.Detach();
//		bitmap.Detach();


		// Create and set the disabled toolbar image list.
		bitmap.LoadMappedBitmap( IDB_RECBAR256D );
		imageList.Create( 32, 32, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		m_wndToolBar.SendMessage( TB_SETDISABLEDIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();

		// END HIGH RES BUTTONS
	}

#endif

	// Set toolbar style
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC );

	// Allow docking everywhere
	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );

	// Dock the contol bar at the right side of the frame window
	DockControlBar(&m_wndToolBar,AFX_IDW_DOCKBAR_RIGHT);


	// Create the play tool bar
	if (!m_wndPlayBar.Create(this,WS_CHILD|WS_VISIBLE|CBRS_TOP, ID_CDSELECTIONBAR_ID ) )
	{
		TRACE0("Failed to create play toolbar\n");
		return -1;      // fail to create
	}

	// Set the style of the toolbar
	m_wndPlayBar.SetBarStyle(m_wndPlayBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// Allow docking everywhere
	m_wndPlayBar.EnableDocking(CBRS_ALIGN_ANY);

	// Dock the control at the top of the frame window
	DockControlBar(&m_wndPlayBar,AFX_IDW_DOCKBAR_TOP);


	// Create the utility tool bar
	if ( !m_wndUtilBar.Create( this, WS_CHILD | WS_VISIBLE | CBRS_TOP, ID_UTILOOLBAR ) )
	{
		TRACE0("Failed to create play toolbar\n");
		return -1;      // fail to create
	}

	// Set the style of the toolbar
	m_wndUtilBar.SetBarStyle(m_wndUtilBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// Allow docking everywhere
	m_wndUtilBar.EnableDocking(CBRS_ALIGN_ANY);

	// Dock the control at the top of the frame window
	DockControlBar(&m_wndUtilBar,AFX_IDW_DOCKBAR_TOP);

	// Load the state of the Status Bar
	LoadBarState( _T( "MainFrmBarSettings" ) );

	SetLanguageMenu( );

//	ShowControlBar(	&m_wndPlayBar,
//					g_config.GetIni().GetValue( _T( "PlayToolBar" ), _T( "Show" ), 1 ),
//					FALSE );
//	ShowControlBar(	&m_wndToolBar,
//					g_config.GetIni().GetValue( _T( "RecordToolBar" ), _T( "Show" ), 1 ),
//					FALSE );

	return 0;
}


BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~FWS_ADDTOTITLE;
	return CFrameWnd::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnClose() 
{
	// Save the state of the Status Bar
	SaveBarState( _T( "MainFrmBarSettings" ) );
	
	CRect	myRect;

	// Get the size of the frame window
	GetWindowRect(myRect);

	// Save the frame settings to the INI file
	g_config.GetIni().SetValue( _T( "RipperFrame" ), _T( "FrameWndLeft" ), myRect.left );
	g_config.GetIni().SetValue( _T( "RipperFrame" ), _T( "FrameWndTop" ), myRect.top );
	g_config.GetIni().SetValue( _T( "RipperFrame" ), _T( "FrameWndRight" ), myRect.right );
	g_config.GetIni().SetValue( _T( "RipperFrame" ), _T( "FrameWndBottom" ), myRect.bottom );

//	g_config.GetIni().SetValue( _T( "PlayToolBar" ), 
//								_T( "Show" ), 
//								(m_wndPlayBar.GetStyle() & WS_VISIBLE) );

//	g_config.GetIni().SetValue( _T( "RecordToolBar" ), 
//								_T( "Show" ), 
//								(m_wndToolBar.GetStyle() & WS_VISIBLE) );

	CFrameWnd::OnClose();
}

void CMainFrame::OnViewplaybar() 
{
	ShowControlBar(&m_wndPlayBar, (m_wndPlayBar.GetStyle() & WS_VISIBLE) == 0,FALSE);
}

void CMainFrame::OnUpdateViewplaybar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( (m_wndPlayBar.GetStyle() & WS_VISIBLE)?1:0);
}

void CMainFrame::OnViewrecordbar() 
{
	ShowControlBar(&m_wndToolBar, (m_wndToolBar.GetStyle() & WS_VISIBLE) == 0,FALSE);
}

void CMainFrame::OnUpdateViewrecordbar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( (m_wndToolBar.GetStyle() & WS_VISIBLE)?1:0);
}


void CMainFrame::OnLanguageSelection( UINT nID ) 
{
    CDocTemplate*	pTemplate = NULL;
    POSITION		pos;
    CDocument*		pDocument = NULL;

	int nLanguageSelection = nID - ID_LANGUAGE_START;


	if ( 0 == nLanguageSelection )
	{
		WORD nLanguage;

		CHyperLink myLink;

		CUString strVersions;
		// FIXME
		strVersions.Format( _W("cdexversion=1.70&beta=8&cdexdir=%s" ), (LPCWSTR)g_config.GetAppPath() );

		for ( nLanguage=0; nLanguage < g_language.GetNumLanguageStrings(); nLanguage++)
		{
			CUString strLangName( g_language.GetLanguageString( nLanguage ) );
			CUString strLangVersion;
			strLangVersion.Format( _W("%s=%s"), (LPCWSTR)strLangName, (LPCWSTR)g_language.GetRevisionLevel( strLangName ) );
			strVersions += _W( "&" ) + strLangVersion;
		}

//		myLink.GotoURL( _T("www.cdex.n3.net/lang/langcheck.php?") + strVersions , SW_SHOW );
		myLink.GotoURL( _T("cdexos.sourceforge.net/lang/cdex_v1.70"), SW_SHOW );
	}
	else
	{
//		nLanguageSelection--;

		// Switch to selected language
		CString strMenu;

		m_pPopupMenu->GetMenuString( nLanguageSelection, strMenu, MF_BYPOSITION );

		g_language.SetLanguage( CUString( strMenu ) );

		// get document template
		pos = AfxGetApp()->GetFirstDocTemplatePosition();

		pTemplate = AfxGetApp()->GetNextDocTemplate( pos );

		POSITION docpos = pTemplate->GetFirstDocPosition( );

		// Loop through documents in this template
		while ( docpos )
		{
			VERIFY( pDocument = pTemplate->GetNextDoc(docpos) );

			// update all view attached to this document
			pDocument->UpdateAllViews( NULL,WM_CDEX_INITIAL_UPDATE, NULL );
		}

		g_config.SetLanguage( g_language.GetLanguage() );
		g_config.Save();

		SetLanguageMenu( );
	}
}

BOOL CMainFrame::OnMyToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;

	UINT nID = pNMHDR->idFrom;

	if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
		pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
	{
		// idFrom is actually the HWND of the tool
		nID = ((UINT)(WORD)::GetDlgCtrlID( (HWND)nID ) );
	}

	if (nID != 0) // will be zero on a separator
	{
		// don't handle the message if no string resource found
        CUStringConvert strCnv;
        CUString strLangTipText = g_language.GetString( nID );

		if ( strLangTipText.IsEmpty() )
		{
			return FALSE;
		}	

		// this is the command id, not the button index
        AfxExtractSubString( strTipText, strCnv.ToT( strLangTipText ), 1, _T( '\n' ));
	}
#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, strTipText, _countof(pTTTA->szText) );
	else
		_mbstowcsz(pTTTW->szText, strTipText, _countof(pTTTW->szText) );
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, _countof(pTTTA->szText));
	else
		lstrcpyn(pTTTW->szText, strTipText, _countof(pTTTW->szText));
#endif
	*pResult = 0;

	// bring the tooltip window above other popup windows
	::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,
		SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);

	return TRUE;    // message was handled
}


void CMainFrame::GetMessageString(UINT nID, CString& rMessage) const
{
    CUStringConvert strCnv;

	// load appropriate string
	CUString strLang = g_language.GetString( nID );

	// extract first portion of string
    AfxExtractSubString( rMessage, strCnv.ToT( strLang ), 0, '\n');

	if ( rMessage.IsEmpty() )
	{
		// not found
		TRACE1("Warning: no message line prompt for ID 0x%04X.\n", nID);
	}
}

void CMainFrame::OnUpdateViewutiltoolbar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( (m_wndUtilBar.GetStyle() & WS_VISIBLE)?1:0);
}

void CMainFrame::OnViewutiltoolbar() 
{
	ShowControlBar(&m_wndUtilBar, (m_wndUtilBar.GetStyle() & WS_VISIBLE) == 0,FALSE);
}
