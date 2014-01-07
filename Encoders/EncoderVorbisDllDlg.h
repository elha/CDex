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


#ifndef ENCODERVORBISDLLDLG_H_INCLUDED
#define ENCODERVORBISDLLDLG_H_INCLUDED

#include "EncoderDlg.h"

class CEncoderVorbidDllDlg : public CEncoderDlg
{
public:
	// CONSTRUCTOR
	CEncoderVorbidDllDlg();

	// MUTATORS
	virtual void SetControls(CEncoder* m_pEncoder);
	virtual void GetControls(CEncoder* m_pEncoder);
	virtual void SetBitrates(int min_br,int nom_br, int max_br);
	virtual void FillBitrateTables();

	UINT GetIDD();

// Dialog Data
	//{{AFX_DATA(CEncoderVorbidDllDlg)
	enum { IDD = IDD_ENCODERVORBISDLLDLG };
	CScrollBar	m_QualityScroll;
	CComboBox	m_MinBitrate;
	CComboBox	m_Bitrate;
	CComboBox	m_MaxBitrate;
	BOOL		m_bEncDuringRead;
	int		m_nMode;
	int		m_nChannels;
	BOOL	m_bUseQualitySetting;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEncoderVorbidDllDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UpdateQualityControlValue();

	// Generated message map functions
	//{{AFX_MSG(CEncoderVorbidDllDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUsequalitysetting();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



#endif // ENCODERVORBISDLLDLG_H_INCLUDED
