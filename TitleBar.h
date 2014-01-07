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


#if !defined(AFX_TITLEBAR_H__16F47F43_3F63_11D2_8704_A9E3FCB3091B__INCLUDED_)
#define AFX_TITLEBAR_H__16F47F43_3F63_11D2_8704_A9E3FCB3091B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TitleBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTitleBar window

class CTitleBar : public CToolBar
{
	CFont	m_font;
	CStatic	m_Artist;
	CStatic	m_Title;
	CEdit	m_ArtistE;
	CEdit	m_TitleE;

// Construction
public:
	CTitleBar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTitleBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTitleBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CTitleBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TITLEBAR_H__16F47F43_3F63_11D2_8704_A9E3FCB3091B__INCLUDED_)
