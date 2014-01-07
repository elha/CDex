/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1998 - 1999 Juan Franklin Peña
** Copyright (C) 2000 Albert L. Faber
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


#ifndef CUSTOMOPENFILEDLG_H
#define CUSTOMOPENFILEDLG_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


#include "vector"
using namespace std ;

#include "Resource.h"
#include "ddxgetfolder.h"
#include "LineList.h"

class COpenFileDlg : public CDialog
{
// Construction
public:
	// standard constructor
	COpenFileDlg(CUString directory, CUString fileType, int iconId, BOOL	bOpenDlg = TRUE, CWnd* pParent  = NULL );
	virtual ~COpenFileDlg();

	CUString GetFilePath();  // Returns full path
//	CUString GetFileName();  // Returns only the file name without path
	CUString GetDir() const {return m_strDir;}

	CUString	GetOpenButtonText() const { return m_strOpenText;}
	void	SetOpenButtonText( CUString strValue ) { m_strOpenText = strValue;}

	virtual	CUString GetNextPathName(POSITION& pos );
	virtual CUString GetSubPath( POSITION pos );
	virtual CUString GetFileName( POSITION pos ); 


	virtual POSITION GetStartPosition( ) const;
	virtual void ShowNormalizeCtrl( BOOL bValue );
	virtual void ShowDelOrigCtrl( BOOL bValue );
	virtual void ShowRiffWavCtrl( BOOL bValue );
	virtual void ShowKeepDirLayoutCtrl( BOOL bValue );
	void EnableSettingsDlg( BOOL bValue ) { m_bSettingsDlg = bValue; }

private:
	enum FILE_OPEN_COLUMN_TYPES { 
		FILE_OPEN_NAME = 0,
		FILE_OPEN_TYPE,
		FILE_OPEN_DATE,
		FILE_OPEN_PATH,
		FILE_OPEN_SIZE,
		FILE_OPEN_MAX_COLUMN_TYPES
	};

	int			m_nIconId;
	CUString		m_strDir;
	CUString		m_strFileType;
	CUString		m_strFileName;
	BOOL		m_bIsOpenFileDlg;
	BOOL		m_bSortAscending;
	int			m_nSortedColumn;
	BOOL		m_bInit;
	CUString*	m_strSelectedFiles;
	int			m_nSelectedFiles;
	vector<CUString> m_vStrExt;
	BOOL		m_bShowNorm;
	BOOL		m_bShowDelOrig;
	BOOL		m_bShowRiffWav;
	BOOL		m_bShowKeepDirLayout;
	BOOL		m_bSettingsDlg;
	CUString		m_strOpenText;
	int			m_nColWidth[ FILE_OPEN_MAX_COLUMN_TYPES ];

	virtual BOOL FillFileList();
	virtual void AddRecursiveFiles( const CUString& strDir, int nItem = 0 );
	virtual void MakeExtString();
	virtual BOOL CompareExt( const CUString& strExt );
	virtual void ReSortColumns();
	virtual void SaveSettings();

	
//	virtual int SetCurSel(int nSelect);
// Dialog Data
public:
	//{{AFX_DATA(COpenFileDlg)
	enum { IDD = IDD_OPEN_FILE };
	CGetFolderControl m_EditDir;
	LListCtrl	m_ctrlRequestedFiles;
	BOOL	m_bRecursiveDir;
	BOOL	m_bDeleteOriginal;
	BOOL	m_bNormalize;
	BOOL	m_bRiffWav;
	BOOL	m_bKeepDirLayout;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenFileDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(COpenFileDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnColumnclickListRequestedFiles(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditDir();
	afx_msg void OnSelectAll();
	afx_msg void OnRecursiveDir();
	afx_msg void OnOpen();
	//}}AFX_MSG
	virtual afx_msg void OnSettings();

	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // CUSTOMOPENFILEDLG_H
