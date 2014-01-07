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


#ifndef FILENAMEPROPPAGE_H_INCLUDED
#define FILENAMEPROPPAGE_H_INCLUDED


#include "MFECToolTip.h"
#include "resource.h"
#include "ddxgetfolder.h"

/////////////////////////////////////////////////////////////////////////////
// FILENAME PROPERTY dialog
/////////////////////////////////////////////////////////////////////////////

class CFilenamePropPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CFilenamePropPage)
private:
	BOOL			m_bInit;
	CMFECToolTip	m_Tooltip;
	CToolTipCtrl	m_FormatInfoTooltip;
	CUString			m_strTitle;

// Construction
public:
	CGetFolderControl m_MP3OutputDir;
	CGetFolderControl m_CnvOutputDir;

	// CONSTRUCTOR
	CFilenamePropPage();

	// DESTRUCTOR
	~CFilenamePropPage();

	// MUTATORS
	afx_msg BOOL OnApply();
	void UpdateControls();

	virtual BOOL PreTranslateMessage( MSG* pMsg );


// Dialog Data
	//{{AFX_DATA(CFilenamePropPage)
	enum { IDD = IDD_FILENAMEPROPPAGE };
	CString	m_strFileNameDemo;
	BOOL	m_bPlsPlayList;
	BOOL	m_bM3UPlayList;
	CString	m_strFileFormat;
	CString	m_strPlaylistFormat;
	CString	m_strPlsFileNameDemo;
	CString	m_strSplitChar;
	BOOL	m_bSplitTrack;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFilenamePropPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CFilenamePropPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnChange();
	afx_msg void OnTrackbutton();
	afx_msg void OnFilename();
	afx_msg void OnChangeFileFormat();
	afx_msg void OnChangePlaylistFormat();
	afx_msg void OnFnameformatinfo();
	afx_msg void OnIllegalChars();
	afx_msg void OnPlaylistfmtinfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
