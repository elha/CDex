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
#include "CDex.h"
#include "TitleBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTitleBar

CTitleBar::CTitleBar()
{
}

CTitleBar::~CTitleBar()
{
}


BEGIN_MESSAGE_MAP(CTitleBar, CToolBar)
	//{{AFX_MSG_MAP(CTitleBar)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTitleBar message handlers

int CTitleBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CToolBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Load Toolbar
	if (!LoadToolBar(IDR_TITLEBAR))
		return -1;

	CClientDC dc(this);

	int nHeight=-((dc.GetDeviceCaps(LOGPIXELSY)*8)/72);

	// Create an 8 point sans serief font for the combo boxes
	m_font.CreateFont(nHeight,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,OUT_CHARACTER_PRECIS,
		CLIP_CHARACTER_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH | FF_DONTCARE, _T( "MS Sans Serif" ));

	CFont* pOldFont =dc.SelectObject(&m_font);


	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);

	int cxChar = tm.tmAveCharWidth;
	int cyChar = tm.tmHeight + tm.tmExternalLeading;

	dc.SelectObject(pOldFont);

	// Add "Artist" field to toolbar
	SetButtonInfo(0,IDC_TBARTIST,TBBS_SEPARATOR,cxChar*16);

	CRect rcClient;

	GetItemRect(0,&rcClient);

	rcClient.bottom=rcClient.top+ (cyChar*16);

	if (!m_Artist.Create( _T( "Artist" ),WS_CHILD|WS_VISIBLE,rcClient,this,IDC_TBARTIST))
		return -1;

	// Add "Artist" field to toolbar
	SetButtonInfo(2,IDC_TBARTISTE,TBBS_SEPARATOR,cxChar*80);

	rcClient;

	GetItemRect(2,&rcClient);

	rcClient.bottom=rcClient.top+ (cyChar*16);

	if (!m_ArtistE.Create(WS_CHILD|WS_VISIBLE,rcClient,this,IDC_TBARTISTE))
		return -1;

	
	return 0;
}
