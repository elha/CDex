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


#ifndef AUTODETECT_H_INCLUDED
#define AUTODETECT_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "config.h"

class CCdda;

/////////////////////////////////////////////////////////////////////////////
// CAutoDetect dialog

class CAutoDetect : public CDialog
{
private:
	BOOL		m_bAbort;
public:
	// CONSTRUCTOR
	CAutoDetect(CWnd* pParent = NULL);   // standard constructor
	// DESTRUCTOR
	~CAutoDetect();
	CWaitCursor* m_pWaitCursor;
	static CDEX_ERR TestDrive(CDROMPARAMS& cdTestParams);

// Dialog Data
	//{{AFX_DATA(CAutoDetect)
	enum { IDD = IDD_AUTODETECT };
	CListCtrl	m_List;
	// CString	m_strMMC;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAutoDetect)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	static UINT OnStartDetection(void *lpParam);
    static UINT CAutoDetect::OnStartDetectionCache(void *lpParam);

	afx_msg void OnInitDetection();

	// Generated message map functions
	//{{AFX_MSG(CAutoDetect)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnDestroy();
	virtual void OnCancel();
	afx_msg void OnAbort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
