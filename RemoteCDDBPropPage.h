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


#ifndef REMOTECDDBPROPPAGE_H_INCLUDED
#define REMOTECDDBPROPPAGE_H_INCLUDED

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CRemoteCDDBPropPage dialog

class CRemoteCDDBPropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CRemoteCDDBPropPage)
private:
	int		m_nLastServerSelection;
	CUString	m_strTitle;

// Construction
public:
	// CONSTRUCTOR
	CRemoteCDDBPropPage();

	// DESTRUCTOR
	~CRemoteCDDBPropPage();

	// MUTATORS
	afx_msg BOOL OnApply();
	void UpdateControls();
	void UpdateCtrls();
	void GetRemoteServers();
	UINT QuerySites( CWnd* pWnd, volatile BOOL& bAbort );

// Dialog Data
	//{{AFX_DATA(CRemoteCDDBPropPage)
	enum { IDD = IDD_CDDBSETTINGS };
	CComboBox	m_cddbProtocol;
	CEdit	m_ProxyUser;
	CEdit	m_ProxyPassword;
	CButton	m_UseProxy;
	CButton	m_CDDProtocl;
	CEdit	m_ProxyAddress;
	CEdit	m_ProxyPort;
	CComboBox	m_ServerList;
	CString	m_strEmail;
	CString	m_strProxyAddress;
	int		m_nProxyPort;
	BOOL	m_bUseProxy;
	BOOL	m_bUseAuthentication;
	CString	m_strProxyUser;
	CString	m_strProxyPassword;
	int		m_nTimeOut;
	BOOL	m_bAutoConnect;
	CString	m_strPath;
	CString	m_strAddress;
	int		m_nCDDBPort;
	CString	m_strLocation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CRemoteCDDBPropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CRemoteCDDBPropPage)
	afx_msg void OnChange();
	virtual BOOL OnInitDialog();
	afx_msg void OnUseauthentication();
	afx_msg void OnQuerysites();
	afx_msg void OnSubmitopts();
	afx_msg void OnResetbutton();
	afx_msg void OnAddsite();
	afx_msg void OnChangeLocation();
	afx_msg void OnChangeHttppath();
	afx_msg void OnSelchangeProtocol();
	afx_msg void OnSelchangeServerlist();
	afx_msg void OnChangeAddress();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
