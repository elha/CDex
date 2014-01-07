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


#ifndef CDPROPPAGE_INCLUDED
#define CDPROPPAGE_INCLUDED

#include "Resource.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CDPropPage dialog

class CDPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CDPropPage)
	CUString		m_strTitle;
// Construction
public:
	int			m_nReadBlkSize;
	int			m_nReadOverlap;
	CComboBox	m_ReadMethod;
	CComboBox	m_CDRomList;
	UINT		m_nCDSpeed;

	// CONSRUCTOR
	CDPropPage();

	// DESTRUCTOR
	~CDPropPage();

	// MUTATORS
	afx_msg BOOL OnApply();
	void UpdateControls();

	void	GetCDParams();
	void	SetCDParams();

	void	FillCDRiveSelection();


// Dialog Data
	//{{AFX_DATA(CDPropPage)
	enum { IDD = IDD_CDPROPPAGE };
	CButton	m_UseNtScsi;
	BOOL	m_bSwapChannels;
	BOOL	m_bJitter;
	int		m_nStartOffset;
	int		m_nEndOffset;
	int		m_nSpinUpTime;
	BOOL	m_bAspiPosting;
	int		m_nRetries;
	BOOL	m_bEjectWhenFinished;
	BOOL	m_bSelectAll;
	BOOL	m_bUseNtScsi;
	BOOL	m_bFirstBlockOnly;
	BOOL	m_bMultipleRead;
	int		m_nMultipleReadCount;
	BOOL	m_bLockDuringRead;
	CComboBox	m_RippingMethod;
	BOOL	m_bUseCDText;
	int		m_nSampleOffset;
	//}}AFX_DATA


	void	EnDisAbleControls();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CDPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDPropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnDetect();
	afx_msg void OnSelchangeCdromlist();
	afx_msg void OnJitter();
	afx_msg void OnSelchangeRippingmethod();
	afx_msg void OnUsentscsi();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
};


#endif
 