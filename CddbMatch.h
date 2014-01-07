/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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


#if !defined(AFX_CDDBMATCH_H__39131300_54B9_11D2_8706_C83366A80109__INCLUDED_)
#define AFX_CDDBMATCH_H__39131300_54B9_11D2_8706_C83366A80109__INCLUDED_


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CddbMatch.h : header file
//

#include "LineList.h"


/////////////////////////////////////////////////////////////////////////////
// CCddbMatch dialog

#define MAX_MATCHES 25

class CCddbMatch : public CDialog
{
	int		m_nMatch;
	int		m_nMatches;

	CUString	m_strArrayCat[MAX_MATCHES];
	CUString	m_strArrayTit[MAX_MATCHES];
	CUString	m_strArrayDid[MAX_MATCHES];
	CUString	m_strMatch[MAX_MATCHES];

// Construction
public:
	CCddbMatch(CWnd* pParent = NULL);   // standard constructor

	CUString GetMatch() const {return m_strMatch[m_nMatch];}
	CUString GetMatchCat() const {return m_strArrayCat[m_nMatch];}
	CUString GetMatchTitle() const {return m_strArrayTit[m_nMatch];}
	CUString GetMatchDiskID() const {return m_strArrayDid[m_nMatch];}

	void	AddMatch(CUString strMatches);

// Dialog Data
	//{{AFX_DATA(CCddbMatch)
	enum { IDD = IDD_CDDBMATCH };
	CButton	m_OK;
	LListCtrl	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCddbMatch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCddbMatch)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClickMatchlist(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDDBMATCH_H__39131300_54B9_11D2_8706_C83366A80109__INCLUDED_)
