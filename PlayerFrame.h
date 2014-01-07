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


#ifndef PLAYERFRAME_H_INCLUDED
#define PLAYERFRAME_H_INCLUDED

#include "MainFrm.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PlayerFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPlayerFrame frame

class CPlayerFrame : public CMainFrame
{
	DECLARE_DYNCREATE(CPlayerFrame)
protected:
	CPlayerFrame();           // protected constructor used by dynamic creation

private:
//	CMenu*			m_pPopupMenu;

// Attributes
public:

// Operations
public:
	afx_msg LRESULT OnPlayerAddFile(WPARAM wParam, LPARAM lParam);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerFrame)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:  // control bar embedded members
	virtual ~CPlayerFrame();

//	bool SetLanguageMenu();

	// Generated message map functions
	//{{AFX_MSG(CPlayerFrame)
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewplaybar();
	afx_msg void OnUpdateViewplaybar(CCmdUI* pCmdUI);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	//}}AFX_MSG

//	virtual afx_msg void OnLanguageSelection( UINT nID );
//	virtual afx_msg BOOL OnMyToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
	
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYERFRAME_H__D47AF044_7BC8_11D5_A2DB_005004EF8536__INCLUDED_)
