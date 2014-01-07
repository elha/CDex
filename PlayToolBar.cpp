/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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
#include "PlayToolBar.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CDSKIPBACKPOSITION 2
#define CDSKIPFORWARDPOSITION 3
#define CDSELPOSITION 13


/////////////////////////////////////////////////////////////////////////////
// CPlayToolBar

CPlayToolBar::CPlayToolBar()
{
}

CPlayToolBar::~CPlayToolBar()
{
}


BEGIN_MESSAGE_MAP(CPlayToolBar, CToolBar)
	//{{AFX_MSG_MAP(CPlayToolBar)
	ON_WM_CREATE()
	ON_CBN_SELENDOK(ID_CDSELBAR,OnSelectCD)
	ON_BN_CLICKED( ID_SKIPBACK, OnSkipBack )
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPlayToolBar message handlers


void CPlayToolBar::OnSelectCD()
{
	// Notify Active View
	((CFrameWnd*)AfxGetMainWnd())->GetActiveView()->SendMessage(WM_UPDATE_CDSELECTION,m_CDSelBox.GetCurSel(),0L);
}


void CPlayToolBar::OnSkipBack()
{
}


void CPlayToolBar::UpdateCDSelection()
{
	// Clear Selection
	m_CDSelBox.ResetContent( );

	CDROMPARAMS cdParams;

	memset(&cdParams,0x00,sizeof(CDROMPARAMS));

	// Set selection in CD-ROM list
	int nSelCD=CR_GetActiveCDROM();

	// Fill list with CD-ROM devices
	for (int i=0;i<CR_GetNumCDROM();i++)
	{
		// Set the active CD ROM in order to get the information
		CR_SetActiveCDROM(i);

		// Get cdParames of current selected CD-ROM
		CR_GetCDROMParameters(&cdParams);

		// Add the CD-ROM ID to the combo box
        CUString strTmp( cdParams.lpszCDROMID, CP_ACP );
        CUStringConvert strCnv;
 		m_CDSelBox.AddString( strCnv.ToT( strTmp ) );
	}
	
	// Select the original CD-ROM again
	CR_SetActiveCDROM(nSelCD);

	// Set selection in CD-ROM list
	m_CDSelBox.SetCurSel(nSelCD);
}





int CPlayToolBar::OnCreate(LPCREATESTRUCT lpcs)
{
	CImageList	imageList;
	CBitmap		bitmap;

	if (CToolBar::OnCreate(lpcs) ==-1)
		return -1;

	// Load Toolbar
	if (!LoadToolBar(IDW_PLAYBAR))
		return -1;

	if ( UseFlatLookToolBar() )
	{
		ModifyStyle(0, TBSTYLE_FLAT);

		// Set up hot bar image lists.
		// Create and set the normal toolbar image list.
		bitmap.LoadBitmap( IDB_CDPLAYTOOLBAR_C );
		imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		SendMessage(TB_SETIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();

		// Create and set the hot toolbar image list.
		bitmap.LoadBitmap( IDB_CDPLAYTOOLBAR_H );
		imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();


		// Create and set the disabled toolbar image list.
		bitmap.LoadBitmap( IDB_CDPLAYTOOLBAR_D );
		imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		SendMessage(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();


		// END HIGH RES BUTTONS
	}



	// Create an 8-point MS Sans Serif font for the combo box
	CClientDC dc(this);

	int nHeight=-((dc.GetDeviceCaps(LOGPIXELSY)*8)/72);

	m_font.CreateFont(nHeight,0,0,0,FW_NORMAL,0,0,0,
		DEFAULT_CHARSET,OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,
		DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, _T( "MS Sans Serif" ));

	CFont* pOldFont=dc.SelectObject( &m_font );

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int cxChar=tm.tmAveCharWidth;
	int cyChar=tm.tmHeight + tm.tmExternalLeading;

	dc.SelectObject(pOldFont);

	// Add CDSelection to toolbar
	SetButtonInfo(CDSELPOSITION, ID_CDSELBAR, TBBS_SEPARATOR, cxChar * 50 );

	CRect rect;
	GetItemRect(CDSELPOSITION,&rect);

	rect.bottom=rect.top+(cyChar*16);

	if (!m_CDSelBox.Create(WS_CHILD | WS_VISIBLE |WS_VSCROLL | CBS_DROPDOWNLIST, rect, this, ID_CDSELBAR ) )
		return -1;

	m_CDSelBox.SetFont(&m_font);
	return 0;
}



void CPlayToolBar::RepeatButton(DIRECTION direction) 
{
	int		nbiter = 0;
	MSG		msg;
	int		id  = GetDlgCtrlID( );
	int		mCurrentValue=0;
	int		mIncrement=1;
	WORD	wFirst=1;
	HWND	hWnd = GetSafeHwnd();
	BOOL	bDone = FALSE;

//	if( mhWnd == NULL )
//		mhWnd = GetParent()->GetSafeHwnd();

	while( bDone==FALSE)
	{

		while (PeekMessage( &msg, hWnd, 0, 0, PM_REMOVE ) )
		{
			if (msg.message==WM_LBUTTONUP)
			{
				bDone=TRUE;
				// Repost message
				::SendMessage(msg.hwnd,msg.message,msg.wParam,msg.lParam);
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if( nbiter < 10 )
			Sleep( 200 );
		else if( nbiter < 20 )
			Sleep( 100 );
		else if( nbiter < 30 )
			Sleep( 50 );
		else 
			Sleep( 25 );
		
		nbiter++;

		mCurrentValue += ( direction == FORWARD )? mIncrement : -mIncrement;
/*		
		if( mCurrentValue < mMin )
		{
			mCurrentValue = mMax;
		}
		if( mCurrentValue >  mMax)
		{
			mCurrentValue = mMin;
		}
*/
//		::SendMessage( GetParent()->GetSafeHwnd(), WM_SEEKBUTTONVALUECHANGED, id, mCurrentValue );
//		TRACE("Value = %d\n", mCurrentValue );
		((CFrameWnd*)AfxGetMainWnd())->GetActiveView()->SendMessage( WM_SEEKBUTTONVALUECHANGED, wFirst, mCurrentValue );
		wFirst=0;
	}
	((CFrameWnd*)AfxGetMainWnd())->GetActiveView()->SendMessage( WM_SEEKBUTTONUP, 0, 0 );
}


void CPlayToolBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CRect rcSkip;
	CRect rcSkipBack;
	GetItemRect(CDSKIPBACKPOSITION,&rcSkipBack);
	GetItemRect(CDSKIPFORWARDPOSITION,&rcSkip);

	CToolBar::OnLButtonDown(nFlags, point);

	if ( (rcSkipBack.PtInRect( point )) && ((nFlags&MK_LBUTTON)==MK_LBUTTON)  )
	{
		int nOldStat=GetToolBarCtrl().GetState( ID_SKIPBACK);
		if ( (nOldStat&TBSTATE_ENABLED) == TBSTATE_ENABLED)
		{
			RepeatButton(BACK);
		}
		return;
	}

	if ( (rcSkip.PtInRect( point )) && ((nFlags&MK_LBUTTON)==MK_LBUTTON)  )
	{
		int nOldStat=GetToolBarCtrl().GetState( ID_SKIPFORWARD);
		if ( (nOldStat&TBSTATE_ENABLED) == TBSTATE_ENABLED)
		{
			RepeatButton(FORWARD);
		}
		return;
	}
}

void CPlayToolBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CToolBar::OnLButtonUp(nFlags, point);
}
