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


#ifndef DETECTGAPS_H_INCLUDED
#define DETECTGAPS_H_INCLUDED
#include "afxwin.h"


/////////////////////////////////////////////////////////////////////////////
// CCDexMsgBox dialog

class CCDexMsgBox : public CDialog
{
// Construction
public:
	CCDexMsgBox(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCDexMsgBox)
	enum { IDD = IDD_CDEXMSGBOX };
	CString	m_strProfileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDexMsgBox)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCDexMsgBox)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    void SetMessageBoxText( const CUString& msgBoxText )
    {
        CUStringConvert strCnv;
        m_msgBoxText.SetWindowText( strCnv.ToT( msgBoxText ));
    }
    void SetMessageBoxCaption( const CUString& value )
    {
        CUStringConvert strCnv;
        SetWindowText( strCnv.ToT( value ));
    }
    bool AbortPressed() const { return m_bAbort; }

private:
    CStatic m_msgBoxText;
    bool    m_bAbort;

public:
    afx_msg void OnStnClickedDetectgaps();
protected:
    virtual void PostNcDestroy();
public:
    afx_msg void OnBnClickedAbort();
};

#endif DETECTGAPS_H_INCLUDED