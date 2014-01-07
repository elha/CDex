/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2007 Georgy Berdyshev
** Copyright (C) 2007 Albert L. Faber
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
#pragma once

#include "langdialog.h"
#include "CdexDoc.h"
#include "afxwin.h"

class CCueSheet : public CLangDialog
{
	DECLARE_DYNAMIC(CCueSheet)

public:
	CCDexDoc* m_pDoc;
	CCueSheet(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCueSheet();

// Dialog Data
	enum { IDD = IDD_CREATECUESHEET };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedCheck1();

private:
	void CreateSheet();
    CUString m_strCueSheet;
public:
	CEdit m_editCueSheet;
	virtual BOOL OnInitDialog();
	CButton m_checkPreGaps;
    afx_msg void OnBnClickedCheck2();
    afx_msg void OnBnClickedCopytoclipboard();
    afx_msg void OnBnClickedClose();
    CButton m_addISRC;
};
