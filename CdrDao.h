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


#if !defined(AFX_CDRDAO_H__5F7BD380_2C64_11D5_A2DB_005004EF8536__INCLUDED_)
#define AFX_CDRDAO_H__5F7BD380_2C64_11D5_A2DB_005004EF8536__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "ChildProcess.h"
#include "HistEdit.h"

// CdrDao.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCdrDao dialog

class CCdrDao : public CDialog
{
// Construction
public:
	CCdrDao(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCdrDao)
	enum { IDD = IDD_CDRDAO };
	CHistEdit	m_OutputWnd;
	CComboBox	m_cdDrives;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCdrDao)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCdrDao)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	HANDLE			m_hInReadPipe;
	HANDLE			m_hInWritePipe;
	HANDLE			m_hOutReadPipe;
	HANDLE			m_hOutWritePipe;
	HANDLE			m_hErrPipe;
	CChildProcess*	m_pChildProcess;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDRDAO_H__5F7BD380_2C64_11D5_A2DB_005004EF8536__INCLUDED_)
