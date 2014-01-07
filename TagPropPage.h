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


#ifndef TAGPROPPAGE_H_INCLUDED
#define TAGPROPPAGE_H_INCLUDED

#include "ddxgetfolder.h"
#include "resource.h"
#include "afxwin.h"
#include "ddxgetfile.h"


/////////////////////////////////////////////////////////////////////////////
// CTagPropPage dialog

class CTagPropPage : public CPropertyPage
{
private:
	DECLARE_DYNCREATE(CTagPropPage)
	CUString	m_strTitle;
public:
	// CONSTRUCTOR
	CTagPropPage();

	// DESTRUCTOR
	~CTagPropPage();

	// MUTATORS
	afx_msg BOOL OnApply();
	void UpdateControls();

	CGetFolderControl m_tmpDir;

// Dialog Data
	//{{AFX_DATA(CTagPropPage)
	enum { IDD = IDD_TAGPROPPAGE };
	CString	m_strComment;
	CString	m_strEncodedBy;
	int		m_nID3V2TrkNrType;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTagPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTagPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedId3v23();
    afx_msg void OnBnClickedId3v24();
    CButton m_cbID3V1;
    CButton m_cbID3V23;
    CButton m_cbID3V24;
    CGetFileControl m_tagPicture;
    CEdit m_pictureComment;
    CButton m_addPicture;
};


#endif
