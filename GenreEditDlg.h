/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 - 2007 Albert L. Faber
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


#ifndef GENREEDITDLG_H_INCLUDED
#define GENREEDITDLG_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GenreEditDlg.h : header file
//

#include "LineList.h"


/////////////////////////////////////////////////////////////////////////////
// CGenreEditDlg dialog

class CGenreEditDlg : public CDialog
{
// Construction
	int		m_nLastSelection;
	int		m_nInsertPosition;
public:
	CGenreEditDlg(CWnd* pParent = NULL);   // standard constructor
	void SetControls( int nSelection );
	void UpdateList( int nSelection );

// Dialog Data
	//{{AFX_DATA(CGenreEditDlg)
	enum { IDD = IDD_EDIT_GENRES };
	CSpinButtonCtrl	m_Spin;
	CComboBox	m_CDDBGenre;
	LListCtrl	m_List;
	CString     m_strGenre;
	int		    m_nID3V1ID;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenreEditDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGenreEditDlg)
	virtual void OnOK();
	afx_msg void OnInsertnew();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangeEditCddbgenre();
	afx_msg void OnChangeEditId3v1id();
	afx_msg void OnChangeEditGenre();
	virtual void OnCancel();
	//}}AFX_MSG
	afx_msg LONG OnUpdateGenreList( WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // GENREEDITDLG_H_INCLUDED
