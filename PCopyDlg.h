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


#ifndef PCOPYDLG_H_INCLUDED
#define PCOPYDLG_H_INCLUDED

#include "Tasks.h"

/////////////////////////////////////////////////////////////////////////////
// PCopyDlg dialog

class PCopyDlg : public CDialog
{
public:
	CTasks	m_Tasks;
private:
	WORD		m_wFirstTrack;
	WORD		m_wLastTrack;
	DWORD		m_dwFirstSector;
	DWORD		m_dwLastSector;
	int			m_nStartT;
	int			m_nEndT;
	BOOL		m_bInit;
// Construction
public:
	// ACCESSORS
	DWORD	GetLastSector() const {return m_dwLastSector;}
	DWORD	GetFirstSector() const {return m_dwFirstSector;}

	CUString GetFileName() const {return CUString( m_strFileName );}
	int		GetOutputType() const {return m_nOutputType;}
	WORD	GetLastTrack() const {return m_wLastTrack;}
	WORD	GetFirstTrack() const {return m_wFirstTrack;}

	// MUTATORS
	void	SetOutputType(int nValue)		{m_nOutputType=nValue;}
	void	SetFileName(CUString& strValue)	{m_strFileName=strValue;}
	void	SetFirstTrack(WORD nValue)		{m_wFirstTrack=nValue;}
	void	SetLastTrack(WORD nValue)		{m_wLastTrack=nValue;}


	PCopyDlg(CWnd* pParent = NULL);   // standard constructor
	void	CalcSectors();
	DWORD	GetStartTrack() const {return m_nStartT;}

	CTasks&		GetTasks() { return m_Tasks;}

// Dialog Data
	//{{AFX_DATA(PCopyDlg)
	enum { IDD = IDD_PCOPYDIALOG };
	CButton	m_MP3Type;
	CSpinButtonCtrl	m_SpinSS;
	CSpinButtonCtrl	m_SpinSM;
	CSpinButtonCtrl	m_SpinSF;
	CSpinButtonCtrl	m_SpinES;
	CSpinButtonCtrl	m_SpinEM;
	CSpinButtonCtrl	m_SpinEF;
	CComboBox	m_StartTrack;
	CComboBox	m_EndTrack;
	CString	m_strFileName;
	int		m_nOutputType;
	int		m_nStartF;
	int		m_nStartM;
	int		m_nStartS;
	int		m_nEndF;
	int		m_nEndM;
	int		m_nEndS;
	CString	m_strInfo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PCopyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PCopyDlg)
	afx_msg void ValidateEndRange();
	afx_msg void ValidateStartRange();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeStarttrack();
	afx_msg void OnSelchangeEndtrack();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PCOPYDLG_H__E8382940_1E74_11D2_8703_B18A8AD96214__INCLUDED_)
