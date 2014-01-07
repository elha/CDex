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


#ifndef LOCALCDDBPROPPAGE_H_INCLUDED
#define LOCALCDDBPROPPAGE_H_INCLUDED

#include "ddxgetfolder.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CLocalCDDBPropPage dialog

class CLocalCDDBPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CLocalCDDBPropPage)
	CUString m_strTitle;
// Construction
public:
	// CONSTRUCTOR
	CLocalCDDBPropPage();

	// DESTRUCTOR
	~CLocalCDDBPropPage();

	// MUTATORS
	afx_msg BOOL OnApply();
	void UpdateControls();
	void UpdateCtrls();


// Dialog Data
	//{{AFX_DATA(CLocalCDDBPropPage)
	enum { IDD = IDD_LOCALCDDBPROPPAGE };
	CGetFolderControl	m_WinampDB;
	CButton	m_CDDProtocl;
	CGetFolderControl m_CDDBPath;
	int		m_nCDDBType;
	BOOL	m_bStIni;
	BOOL	m_nStLocCDDB;
	BOOL	m_bLongDirNames;
	BOOL	m_bWriteAsDosFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CLocalCDDBPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CLocalCDDBPropPage)
	afx_msg void OnChange();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
