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


#include "stdafx.h"
#include "cdex.h"
#include "PlayerFrame.h"
#include "config.h"
#include "Util.h"
#include "Language.h"
#include "HyperLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define LANG_OPTION_MENU_POS	(1)
#define LANG_LANGUAGE_MENU_POS	(3)

#ifndef _countof
	#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif


static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_SEPARATOR           // status line indicator
};


/////////////////////////////////////////////////////////////////////////////
// CPlayerFrame

IMPLEMENT_DYNCREATE(CPlayerFrame, CMainFrame)

CPlayerFrame::CPlayerFrame()
{
	m_pPopupMenu = NULL;
	m_nLangOptionMenuPos = LANG_OPTION_MENU_POS;
	m_nLangLangMenuPos   = LANG_LANGUAGE_MENU_POS;
	m_nMenuID			 = IDR_PLAYERFRAME;
}

CPlayerFrame::~CPlayerFrame()
{
}


BEGIN_MESSAGE_MAP(CPlayerFrame, CMainFrame)
	//{{AFX_MSG_MAP(CPlayerFrame)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEWPLAYBAR, OnViewplaybar)
	ON_UPDATE_COMMAND_UI(ID_VIEWPLAYBAR, OnUpdateViewplaybar)
	ON_WM_CREATE()
	ON_WM_COPYDATA()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
	// toolbar "tooltip" notification
//	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnMyToolTipText)
//	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnMyToolTipText)

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayerFrame message handlers

int CPlayerFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	CImageList	imageList;
	CBitmap		bitmap;

	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	

	CRect	myRect=CRect(
					g_config.GetIni().GetValue( _W( "PlayerFrame" ), _W( "FrameWndLeft" ), 100 ),
					g_config.GetIni().GetValue( _W( "PlayerFrame" ), _W( "FrameWndTop" ), 100 ),
					g_config.GetIni().GetValue( _W( "PlayerFrame" ), _W( "FrameWndRight"), 350 ),
					g_config.GetIni().GetValue( _W( "PlayerFrame" ), _W( "FrameWndBottom"), 250 )
					);

	// Move the window
	if ( myRect.Width() <100 || myRect.Height() < 100 )
	{
		myRect=CRect( 0, 0, 350, 250 );
	}

	MoveWindow( myRect );

	// Create status bar
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	m_wndStatusBar.SetPaneInfo( 1, ID_SEPARATOR,SBPS_NORMAL, 150 );
	m_wndStatusBar.SetPaneInfo( 0, ID_SEPARATOR,SBPS_STRETCH, 100 );

	// Create main toolbar
	if (!m_wndToolBar.Create( this, WS_CHILD | WS_VISIBLE | CBRS_TOP, AFX_IDW_TOOLBAR) || !m_wndToolBar.LoadToolBar( IDW_PLAYFILEBAR ) )
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if ( UseFlatLookToolBar() )
	{
		m_wndToolBar.ModifyStyle( 0, TBSTYLE_FLAT );

		// Set up hot bar image lists.
		// Create and set the normal toolbar image list.
		if ( bitmap.LoadBitmap( IDB_FILEPLAYTOOLBAR_C ) )
		{
			if ( imageList.Create( 16, 16, ILC_COLORDDB|ILC_MASK, 13, 1 ) )
			{

				if ( -1 != imageList.Add( &bitmap, RGB(255,0,255) ) )
				{
					m_wndToolBar.SendMessage(TB_SETIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
				}
				imageList.Detach();
			}
			bitmap.Detach();
		}

		// Create and set the hot toolbar image list.
		if ( bitmap.LoadBitmap( IDB_FILEPLAYTOOLBAR_H ) )
		{
			if ( imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1) )
			{
				if ( -1 != imageList.Add(&bitmap, RGB(255,0,255)) )
				{
					m_wndToolBar.SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
				}
				imageList.Detach();
			}
			bitmap.Detach();
		}


		// Create and set the disabled toolbar image list.
		if ( bitmap.LoadBitmap( IDB_FILEPLAYTOOLBAR_D ) )
		{
			if ( imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1) )
			{
				if ( -1 != imageList.Add(&bitmap, RGB(255,0,255)) )
				{
					m_wndToolBar.SendMessage(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
				}
				imageList.Detach();
			}
			bitmap.Detach();
		}
	}

	// Docking is OK
	EnableDocking(CBRS_ALIGN_ANY);

	// Set the style of the toolbar
	m_wndToolBar.SetBarStyle( m_wndToolBar.GetBarStyle() | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC );

	// Allow docking everywhere
	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );

	// Dock the contol bar at the right side of the frame window
	DockControlBar(&m_wndToolBar,AFX_IDW_DOCKBAR_TOP);

	LoadBarState( _T( "PlayerFrmBarSettings" ) );

	SetLanguageMenu( );

	return 0;
}

void CPlayerFrame::OnClose() 
{

	CRect	myRect;

	// Get the size of the frame window
	GetWindowRect(myRect);

	// Save the frame settings to the INI file
	g_config.GetIni().SetValue( _W( "PlayerFrame"), _W( "FrameWndLeft"), myRect.left );
	g_config.GetIni().SetValue( _W( "PlayerFrame"), _W( "FrameWndTop"), myRect.top );
	g_config.GetIni().SetValue( _W( "PlayerFrame"), _W( "FrameWndRight"), myRect.right );
	g_config.GetIni().SetValue( _W( "PlayerFrame"), _W( "FrameWndBottom"), myRect.bottom );

	SaveBarState( _T( "PlayerFrmBarSettings" ) );

	CFrameWnd::OnClose();
}

void CPlayerFrame::OnSize(UINT nType, int cx, int cy) 
{
	CFrameWnd::OnSize(nType, cx, cy);
}

BOOL CPlayerFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style &= ~FWS_ADDTOTITLE;
	return CFrameWnd::PreCreateWindow(cs);
}


void CPlayerFrame::OnViewplaybar() 
{
	ShowControlBar(&m_wndToolBar, (m_wndToolBar.GetStyle() & WS_VISIBLE) == 0,FALSE);
}

void CPlayerFrame::OnUpdateViewplaybar(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( (m_wndToolBar.GetStyle() & WS_VISIBLE)?1:0);
}

BOOL CPlayerFrame::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
    CDocTemplate*	pTemplate = NULL;
    POSITION		pos;
    CDocument*		pDocument = NULL;

	pos = AfxGetApp()->GetFirstDocTemplatePosition();

	pTemplate = AfxGetApp()->GetNextDocTemplate( pos );

	if ( pCopyDataStruct->dwData == 0 && pCopyDataStruct->lpData )
	{
		if ( pTemplate )
		{

			POSITION docpos = pTemplate->GetFirstDocPosition( );

			// Loop through documents in this template
			while ( docpos )
			{
				VERIFY( pDocument = pTemplate->GetNextDoc(docpos) );

				// update all view attached to this document
				pDocument->UpdateAllViews( NULL, WM_PLAYER_ADD_FILE, (CObject*)pCopyDataStruct->lpData );
			}
		}
	}
	
	return CMainFrame::OnCopyData(pWnd, pCopyDataStruct);
}

