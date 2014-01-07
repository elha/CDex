/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1998 Jorge Lodos (lodos@cigb.edu.cu)
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


#if !defined(AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_)
#define AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// DIBStatic.h : header file
//

#include "Dib.h"

/////////////////////////////////////////////////////////////////////////////
// CDIBStatic window

class CDIBStatic : public CStatic
{
// Construction
public:
	CDIBStatic();

// Attributes
public:
	CDib m_DIB;

public:
	BOOL IsValidDib() const { return (m_DIB.IsValid()); }

// Operations
public:
	BOOL LoadDib(LPCTSTR lpszFileName);
	BOOL LoadDib(CFile& file);
	void UpdateDib();
	void RemoveDib() { m_DIB.Invalidate(); UpdateDib(); }
	
	BOOL DoRealizePalette(BOOL bForceBackGround);
	BOOL LoadFromResource(LPCTSTR lpszResourceName);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDIBStatic)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDIBStatic();

protected:
	void ClearDib();
	void PaintDib(BOOL bDibValid);

	// Generated message map functions
protected:
	//{{AFX_MSG(CDIBStatic)
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnQueryNewPalette();
	afx_msg void OnPaletteChanged(CWnd* pFocusWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBSTATIC_H__40E4BDE3_BD3A_11D1_89F4_0000E81D3D27__INCLUDED_)
