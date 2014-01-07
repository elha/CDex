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
 *  StaticFilespec.h
 *
 *  CStaticFilespec interface
 *    A simple class for displaying long filespecs in static text controls
 *    http://www.codeguru.com/controls/cstatic_filespec.shtml
 */


#ifndef _StaticFilespec_h
#define _StaticFilespec_h

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

/////////////////////////////////////////////////////////////////////////////
// CStaticFilespec window

class CStaticFilespec : public CWnd
{
// Construction
public:
	CStaticFilespec
    (DWORD  dwFormat = DT_LEFT | DT_NOPREFIX | DT_VCENTER,
     BOOL   bPathEllipsis = FALSE);

// Attributes
public:

// Operations
public:
  BOOL    IsPath() { return m_bPathEllipsis; }
  void    SetPath (BOOL bPathEllipsis)  { m_bPathEllipsis = bPathEllipsis; } 
  DWORD   GetFormat() { return m_dwFormat; } 
  void    SetFormat (DWORD dwFormat) { m_dwFormat = dwFormat; } 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticFilespec)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticFilespec();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticFilespec)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnSetText (WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

  BOOL    m_bPathEllipsis;    // flag: draw text as path
  DWORD   m_dwFormat;         // text format
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif

// End StaticFilespec.h
