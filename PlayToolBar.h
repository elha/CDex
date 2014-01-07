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


#ifndef PLAYTOOLBAR_H_INCLUDED
#define PLAYTOOLBAR_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PlayToolBar.h : header file
//

#define ID_CDSELECTIONBAR_ID	( AFX_IDW_TOOLBAR + 10 )

typedef enum DIRECTION_TAG
{
	BACK=0,
	FORWARD
} DIRECTION;


/////////////////////////////////////////////////////////////////////////////
// CPlayToolBar window

class CPlayToolBar : public CToolBar
{
private:
	CComboBox	m_CDSelBox;
	CFont		m_font;
// Construction
public:
	CPlayToolBar();

// Attributes
public:

// Operations
public:
	void UpdateCDSelection();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPlayToolBar();
	void RepeatButton(DIRECTION direction);

	// Generated message map functions
protected:
	//{{AFX_MSG(CPlayToolBar)
	afx_msg int OnCreate(LPCREATESTRUCT);
	afx_msg void OnSelectCD();
	afx_msg void OnSkipBack();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYTOOLBAR_H__DF9D478F_D0CF_11D2_A2D2_444553540001__INCLUDED_)
