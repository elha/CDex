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


#if !defined(AFX_FILECOMPAREDLG_H__D13162E0_04DB_11D3_A2D2_444553540001__INCLUDED_)
#define AFX_FILECOMPAREDLG_H__D13162E0_04DB_11D3_A2D2_444553540001__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FileCompareDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// FileCompareDlg dialog

class FileCompareDlg : public CDialog
{
// Construction
public:
	FileCompareDlg(CWnd* pParent = NULL);   // standard constructor
//	void OnSubtract();

// Dialog Data
	//{{AFX_DATA(FileCompareDlg)
	enum { IDD = IDD_FILECOMPARE };
	CProgressCtrl	m_Progress;
	CString	m_strResult;
	CString	m_strFileNameA;
	CString	m_strFileNameB;
	CString	m_strOffset;
	CString	m_strFileSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FileCompareDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FileCompareDlg)
	afx_msg void OnSelfilea();
	afx_msg void OnSelfileb();
	afx_msg void OnCompare();
	afx_msg void OnCompareCRC(); 

	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILECOMPAREDLG_H__D13162E0_04DB_11D3_A2D2_444553540001__INCLUDED_)
