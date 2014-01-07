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


#ifndef ENCODERPROPPAGE_H_INCLUDED
#define ENCODERPROPPAGE_H_INCLUDED


#include "resource.h"
#include "Encode.h"


/////////////////////////////////////////////////////////////////////////////
// CEncoderPropPage dialog

class CEncoderPropPage : public CPropertyPage
{
private:
	int		m_nOldSel;
	CUString	m_strTitle;
	DECLARE_DYNCREATE(CEncoderPropPage)

// Construction
public:

	int				m_nMP3Channels;
	CEncoder*		m_pEncoder[NUMENCODERS];
	int				m_nEncoders;

	// CONSTRUCTOR
	CEncoderPropPage();

	// DESRUCTOR
	~CEncoderPropPage();

	// MUTATORS
	void SetNormControl();
	afx_msg BOOL OnApply();
	void UpdateControls();
	void UpdateCtrls();
	void SelectEncOptionDlg();

	// Dialog Data
	//{{AFX_DATA(CEncoderPropPage)
	enum { IDD = IDD_ENCODERPROPPAGE };
	CComboBox	m_EncoderSel;
	CComboBox	m_ThreadPriority;
	BOOL		m_bRiffWav;
	int			m_nEncoderType;
	BOOL	m_bRetainWAVFile;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CEncoderPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CEncoderPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelcodec();
	afx_msg void OnRtencoding();
	afx_msg void OnChange();
	afx_msg void OnEnctype();
	afx_msg void OnSelchangeEncodersel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
