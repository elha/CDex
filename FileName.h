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


#if !defined(AFX_FILENAME_H__BA325EC3_3D10_11D2_8704_D8C97688C21E__INCLUDED_)
#define AFX_FILENAME_H__BA325EC3_3D10_11D2_8704_D8C97688C21E__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// FileName.h : header file
//

enum ENUM_FILENAME_CONVERTS {
	FILENAME_CONVERT_NONE = 0,
	FILENAME_CONVERT_LOWER,
	FILENAME_CONVERT_UPPER,
	FILENAME_CONVERT_FC_UPPER,
	FILENAME_CONVERT_FCEW_UPPER,
	FILENAME_CONVERT_SWAP_ARTIST_TRACK
};


/////////////////////////////////////////////////////////////////////////////
// CFNButton window

class CFNButton : public CButton
{
private:
	COLORREF	m_clrText;
	COLORREF	m_clrBkgnd;
	CBrush 		m_brBkgnd;
	BOOL		m_bActivated;
	int			m_nOrderID;
	BOOL		m_bSet;
// Construction
public:
	CFNButton();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFNButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFNButton();
	BOOL	IsActive() const {return m_bActivated;}
	void	IsActive(BOOL bValue) {m_bActivated=bValue;Invalidate();}
	int		GetOrderID() const {return m_nOrderID;}
	void	SetOrderID(int nValue) {m_nOrderID=nValue;}

	// Generated message map functions
protected:
	//{{AFX_MSG(CFNButton)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnClicked();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#define NUMFILENAME_IDS 11

extern void	BuildFileName(CUString strFormat,CUString strArray[NUMFILENAME_IDS],CUString& strName,CUString& strDir);
extern CUString FixFileNameString(const CUString& strIn);
extern const CUString ILLEGAL_FNAME_CHARS;

extern DWORD GetNumReplaceCharacters();

extern CUString FormatTrackNameString( CUString strFormat, ENUM_FILENAME_CONVERTS nType );

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILENAME_H__BA325EC3_3D10_11D2_8704_D8C97688C21E__INCLUDED_)
