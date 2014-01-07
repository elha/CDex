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


#ifndef LINELISTINCLUDED
#define LINELISTINCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include <list>
using namespace std;

// LineList.h : header file
//
/////////////////////////////////////////////////////////////////////////////
// LListCtrl window

class LListCtrl : public CListCtrl
{
private:
// Construction
	BOOL m_bEditNext;
public:
    vector<int> m_hiddenCols;

    LListCtrl():CListCtrl() { m_bEditNext = FALSE;};
    virtual ~LListCtrl() {};
	BOOL GetEditNext() const { return m_bEditNext; }
	VOID SetEditNext( BOOL bValue ) { m_bEditNext = bValue;}
//    VOID HideColumn( int colToHide) { for (int i=0;i<m_hiddenCols.size();i++) { if (m_hiddenCols[i] ==} }
//    VOID NumHideColumn( int colToHide) { for (int i=0;i<m_hiddenCols.size();i++) { if (m_hiddenCols[i] ==} }
    bool IsColumnHidden( int column );
    void HideColumn( int column );
    void UnHideColumn( int column );

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(LListCtrl)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	int		GetCurSel() const;
	int		SetCurSel(int nSelect);
	int		SelectAll();
	int		SelectNone();

	void	SetHeaderControlText( DWORD dwPos, const CUString& strValue );

private:
	void print_header(CDC *pDC, CRect crPrintedPage, CSize csCharSize, CPoint& cpCurPos, CUString& strHeader, DWORD dwFlags);
	void print_line(CDC *pDC, CRect crPrintedPage, CSize csCharSize, CPoint& cpCurpos, DWORD dwLine);
	void print_footer(CDC *pDC, CRect crPrintedPage, CSize csCharSize, CUString& strFooter, DWORD dwPage, DWORD dwPagesTotal);

// Generated message map functions
protected:
    //{{AFX_MSG(LListCtrl)
	afx_msg void OnInsertitem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
      DECLARE_MESSAGE_MAP()

public:
// DoPrint flags:
	enum Flags
	{
		LLC_LANDSCAPE	= 0x0001,
		LLC_HDRNODATE	= 0x0002,
		LLC_HDRNOTIME	= 0x0004,
	};
protected:
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
