/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1997 Ravi Bhavnani
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


/*
 *  StaticFilespec.cpp
 *
 *  CStaticFilespec implementation
 *    A simple class for displaying long filespecs in static text controls
 *    http://www.codeguru.com/controls/cstatic_filespec.shtml
 */

#include "stdafx.h"
#include "StaticFilespec.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticFilespec

CStaticFilespec::CStaticFilespec
  (DWORD  dwFormat      /* = DT_LEFT | DT_NOPREFIX | DT_VCENTER */,
   BOOL   bPathEllipsis /* = FALSE */ )
{
  m_bPathEllipsis = bPathEllipsis;
  m_dwFormat = dwFormat;
}

CStaticFilespec::~CStaticFilespec()
{
}

BEGIN_MESSAGE_MAP(CStaticFilespec, CWnd)
	//{{AFX_MSG_MAP(CStaticFilespec)
  ON_MESSAGE( WM_SETTEXT, OnSetText )
 	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticFilespec message handlers

BOOL CStaticFilespec::OnEraseBkgnd(CDC* pDC) 
{
RECT  rectWnd;    // window rectangle
	
  // Erase background
  GetClientRect (&rectWnd);
  pDC->FillSolidRect (&rectWnd, ::GetSysColor (COLOR_3DFACE));
	return (TRUE);
}

void CStaticFilespec::OnPaint() 
{
CPaintDC	dc (this);            // device context for painting
DWORD       dwFormat;             // text format
RECT		rectWnd;						  // window rectangle
CString     strText;              // window text
CWnd*       pWndParent = NULL;    // parent window

	// Set default font
  pWndParent = GetParent();

  if (pWndParent)
     dc.SelectObject (pWndParent->GetFont());

	// Draw text
  GetWindowText(strText);
  GetClientRect(&rectWnd);
  dwFormat = m_dwFormat | (m_bPathEllipsis ? DT_PATH_ELLIPSIS : DT_END_ELLIPSIS);
  ::DrawTextEx (dc.m_hDC,
                strText.GetBuffer (0),
                strText.GetLength(),
                &rectWnd,
                dwFormat,
                NULL);
  strText.ReleaseBuffer();
	
	// Do not call CWnd::OnPaint() for painting messages
}

LRESULT CStaticFilespec::OnSetText( WPARAM wParam, LPARAM lParam )
{
  DefWindowProc (WM_SETTEXT, wParam, lParam);
  Invalidate();
  return (TRUE);
}
   
// End StaticFilespec.cpp
