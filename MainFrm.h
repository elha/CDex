/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
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


#ifndef MAINFRM_H_INCLUDED
#define MAINFRM_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "PlayToolBar.h"
#include "UtilToolBar.h"

class CMainFrame : public CFrameWnd
{
private:
	CFont	m_font;

protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:
// Operations
	CPlayToolBar* GetPlayToolBar()	{return &m_wndPlayBar;}
	CToolBar* GetRecToolBar() {return &m_wndToolBar;}
	CUtilToolBar* GetUtilToolBar() {return &m_wndUtilBar;}
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
	void SetStatusInfo(CUString& strInfo) {
        CUStringConvert strCnv;
        m_wndStatusBar.SetPaneText(1, strCnv.ToT( strInfo ));
    }
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual void GetMessageString(UINT nID, CString& rMessage) const;

protected:  // control bar embedded members
	CStatusBar		m_wndStatusBar;
	CToolBar		m_wndToolBar;
	CPlayToolBar	m_wndPlayBar;
	CBitmap			m_bmToolbarHi;
	CUtilToolBar	m_wndUtilBar;
	CMenu*			m_pPopupMenu;
	int				m_nLangOptionMenuPos;
	int				m_nLangLangMenuPos;
	UINT			m_nMenuID;

	virtual bool SetLanguageMenu(  );

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnViewplaybar();
	afx_msg void OnUpdateViewplaybar(CCmdUI* pCmdUI);
	afx_msg void OnViewrecordbar();
	afx_msg void OnUpdateViewrecordbar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewutiltoolbar(CCmdUI* pCmdUI);
	afx_msg void OnViewutiltoolbar();
	//}}AFX_MSG

	virtual afx_msg void OnLanguageSelection( UINT nID );
	virtual afx_msg BOOL OnMyToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__8F4F6B59_3F3C_11D2_8704_A9E3FCB3091B__INCLUDED_)
