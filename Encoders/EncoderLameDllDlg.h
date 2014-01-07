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


#ifndef ENCLAMEOPTS_H_INCLUDED
#define ENCLAMEOPTS_H_INCLUDED

#include "EncoderDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CEncoderLameDllDlg dialog

class CEncoderLameDllDlg : public CEncoderDlg
{
public:
	// CONSTRUCTOR
	CEncoderLameDllDlg();

	// MUTATORS
	virtual void SetControls(CEncoder* m_pEncoder);
	virtual void GetControls(CEncoder* m_pEncoder);

	UINT GetIDD();

// Dialog Data
	//{{AFX_DATA(CEncoderLameDllDlg)
	enum { IDD = IDD_ENCODERLAMEDLLDLG };
	CComboBox	m_Version;
	CComboBox	m_VbrMethod;
	CComboBox	m_VBR;
	CComboBox	m_Quality;
	int			m_nMP3Emphasis;
	int			m_nMP3AcousticModel;
	BOOL		m_bEncDuringRead;
	int			m_nMode;
	BOOL		m_bCopyRight;
	BOOL		m_bCRC;
	BOOL		m_bOriginal;
	BOOL		m_bPrivate;
	BOOL		m_bGPsycho;
	int			m_nABR;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEncoderLameDllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEncoderLameDllDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnVersion();
	afx_msg void OnSetControls();
	afx_msg void OnSelchangePreset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // ENCLAMEOPTS_H_INCLUDED
