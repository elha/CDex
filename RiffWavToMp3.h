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


#ifndef RIFFWAVTOMP3_H_INCLUDED
#define RIFFWAVTOMP3_H_INCLUDED

#include "resource.h"
#include "OpenFileDlg.h"
#include "afxwin.h"

/////////////////////////////////////////////////////////////////////////////
// CRiffWavToMP3 dialog

class CRiffWavToMP3 : public CDialog
{
// Construction
public:
	COpenFileDlg*	m_pFileDlg;
private:
	POSITION		m_Pos;
	int				m_nCurFile;
	int				m_nTotalFiles;

public:
	// CONSTRUCTOR
	CRiffWavToMP3(CWnd* pParent = NULL);

	// DESTRUCTOR
	virtual ~CRiffWavToMP3();

// Dialog Data
	//{{AFX_DATA(CRiffWavToMP3)
	enum { IDD = IDD_RIFFWAVTOMP3 };
	CProgressCtrl	m_Progress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRiffWavToMP3)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRiffWavToMP3)
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnAbort();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    CStatic m_processingFile;
};

#endif
