/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2002 Albert L. Faber
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
#include "UtilToolBar.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define PROFILE_SEL_POSITION 0


/////////////////////////////////////////////////////////////////////////////
// CUtilToolBar

CUtilToolBar::CUtilToolBar()
{
}

CUtilToolBar::~CUtilToolBar()
{
}


BEGIN_MESSAGE_MAP(CUtilToolBar, CToolBar)
	//{{AFX_MSG_MAP(CUtilToolBar)
	ON_WM_CREATE()
	ON_CBN_SELENDOK(ID_PROFILESELECTION,OnSelectProfile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUtilToolBar message handlers


void CUtilToolBar::OnSelectProfile()
{
	// Notify Active View
	((CFrameWnd*)AfxGetMainWnd())->GetActiveView()->SendMessage( WM_UPDATE_PROFILESELECTION, m_ProfileSelCtrl.GetCurSel(), 0L );
}


void CUtilToolBar::SetProfileSelection( DWORD dwSelection )
{
	m_ProfileSelCtrl.SetCurSel( dwSelection );
}

void CUtilToolBar::SetProfileSelection( const CUString& strSelection )
{
    CUStringConvert strCnv;

	int nPos = m_ProfileSelCtrl.FindStringExact( -1, strCnv.ToT( strSelection ) );
	if ( nPos >=0 )
	{
		m_ProfileSelCtrl.SetCurSel( nPos );
	}
}


INT CUtilToolBar::GetProfileSelection( )
{
	return m_ProfileSelCtrl.GetCurSel();
}

CUString CUtilToolBar::GetProfileSelectionString( )
{
	CString strRet;
	int nIndex = m_ProfileSelCtrl.GetCurSel();
	if ( nIndex >= 0 )
	{
		m_ProfileSelCtrl.GetLBText( m_ProfileSelCtrl.GetCurSel(), strRet );
	}
	return CUString( strRet );
}

void CUtilToolBar::AddProfileSelection( const CUString& strAdd )
{
    CUStringConvert strCnv;
    m_ProfileSelCtrl.AddString( strCnv.ToT( strAdd ) );
}

void CUtilToolBar::DeleteProfileSelection( const CUString& strDelete )
{
    CUStringConvert strCnv;

    int nPos = m_ProfileSelCtrl.FindStringExact( -1, strCnv.ToT( strDelete ) );
	if ( nPos >=0 )
	{
		m_ProfileSelCtrl.DeleteString( nPos );
	}
}

void CUtilToolBar::DeleteAllProfileSelections( )
{
	m_ProfileSelCtrl.ResetContent( );
}

int CUtilToolBar::OnCreate(LPCREATESTRUCT lpcs)
{
	CImageList	imageList;
	CBitmap		bitmap;

	if (CToolBar::OnCreate(lpcs) ==-1)
		return -1;

	// Load Toolbar
	if ( !LoadToolBar( IDW_UTILBAR ) )
		return -1;

	if ( TRUE == UseFlatLookToolBar() )
	{
		ModifyStyle(0, TBSTYLE_FLAT);
/*
		// Set up hot bar image lists.
		// Create and set the normal toolbar image list.
		bitmap.LoadBitmap( IDB_UTILTOOLBAR_C );
		imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		SendMessage(TB_SETIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();

		// Create and set the hot toolbar image list.
		bitmap.LoadBitmap( IDB_UTILTOOLBAR_H );
		imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		SendMessage(TB_SETHOTIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();


		// Create and set the disabled toolbar image list.
		bitmap.LoadBitmap( IDB_UTILTOOLBAR_D );
		imageList.Create(16, 16, ILC_COLORDDB|ILC_MASK, 13, 1);
		imageList.Add(&bitmap, RGB(255,0,255));
		SendMessage(TB_SETDISABLEDIMAGELIST, 0, (LPARAM)imageList.m_hImageList);
		imageList.Detach();
		bitmap.Detach();

*/
		// END HIGH RES BUTTONS
	}


	// Create an 8-point MS Sans Serif font for the combo box
	CClientDC dc( this );

	int nHeight=-((dc.GetDeviceCaps(LOGPIXELSY)*8)/72);

	m_font.CreateFont(nHeight,0,0,0,FW_NORMAL,0,0,0,
		DEFAULT_CHARSET,OUT_CHARACTER_PRECIS,CLIP_CHARACTER_PRECIS,
		DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE, _T( "MS Sans Serif" ) );

	CFont* pOldFont = dc.SelectObject( &m_font );

	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int cxChar=tm.tmAveCharWidth;
	int cyChar=tm.tmHeight + tm.tmExternalLeading;

	dc.SelectObject( pOldFont );

	// Add Profile Selection to toolbar
	SetButtonInfo( PROFILE_SEL_POSITION, ID_PROFILESELECTION, TBBS_SEPARATOR, cxChar * 50 );

	CRect rect;
	GetItemRect( PROFILE_SEL_POSITION, &rect );

	rect.bottom=rect.top + ( cyChar * 16 );

	if (!m_ProfileSelCtrl.Create(WS_CHILD | WS_VISIBLE |WS_VSCROLL | CBS_DROPDOWNLIST, rect,this,ID_PROFILESELECTION))
		return -1;

	m_ProfileSelCtrl.SetFont(&m_font);

	return 0;
}

