/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
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


#ifndef CDDBDLG_H_INCLUDED
#define CDDBDLG_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CDdbDlg.h : header file
//

#include "Resource.h"
#include "CDinfo.h"
#include "LangDialog.h"
#include "afxwin.h"

extern BOOL	g_bAbortCDDB;	

typedef UINT (*CDDBTHREADPROC) (VOID*, CWnd*, volatile BOOL& bAbort );


/////////////////////////////////////////////////////////////////////////////
// CCDdbDlg dialog

class CCDdbDlg : public CLangDialog
{
private:
	CWinThread*		m_pThread;
	void*			m_pThreadParam;
	CDDBTHREADPROC	m_pThreadProc;
	CEvent			m_eThreadFinished;
	BOOL			m_bAbort;
public:

	// Construction
	CCDdbDlg( CDDBTHREADPROC pThreadProc,void* pThreadParam,CWnd* pParent = NULL);   // standard constructor
	void AddText( const CUString& strAddText );
	void CloseDialog() {OnCancel();}
	void SetAction( const CUString& strAction );
	static UINT RemoteThread( void* pParams );

// Dialog Data
	//{{AFX_DATA(CCDdbDlg)
	enum { IDD = IDD_CDDBDIALOG };
	CStatic	m_Action;
	CEdit	m_Output;
	CString	m_strHist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDdbDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCDdbDlg)
	virtual void OnCancel();
	afx_msg void OnAbort();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg	LONG OnInfoMsg( WPARAM wParam, LPARAM lParam );
	afx_msg	LONG OnActionMsg( WPARAM wParam, LPARAM lParam );
	afx_msg LONG OnFinished( WPARAM wParam, LPARAM lParam );
	afx_msg	LONG OnCloseDialog( WPARAM wParam,LPARAM lParam );
	afx_msg void OnCddbResubmit();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEnChangePreview();
    afx_msg void OnCbnSelchangeCddbDlgCat();
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_CDDBDLG_H__5DCC5BA2_499E_11D2_8705_AB15F5F39A1A__INCLUDED_)
