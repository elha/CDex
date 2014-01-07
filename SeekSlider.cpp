/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
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


// SeekSlider.cpp : implementation file

#include "stdafx.h"
#include "cdex.h"
#include "SeekSlider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSeekSlider

CSeekSlider::CSeekSlider()
{
}

CSeekSlider::~CSeekSlider()
{
}


BEGIN_MESSAGE_MAP(CSeekSlider, CSliderCtrl)
	//{{AFX_MSG_MAP(CSeekSlider)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSeekSlider message handlers

void CSeekSlider::OnLButtonDown(UINT nFlags, CPoint point) 
{
	GetParent()->SendMessage( WM_SEEKSLIDERDOWN );
	CSliderCtrl::OnLButtonDown( nFlags, point );
}

void CSeekSlider::OnLButtonUp( UINT nFlags, CPoint point ) 
{
	GetParent()->SendMessage( WM_SEEKSLIDERUP, 0, GetPos() );	
	CSliderCtrl::OnLButtonUp( nFlags, point );
}
