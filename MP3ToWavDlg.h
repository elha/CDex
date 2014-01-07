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


#ifndef MP3TOWAVDLG_H_INCLUDED
#define MP3TOWAVDLG_H_INCLUDED

#include "OpenFileDlg.h"
#include "CTimeTrack.h"

#include "SndStreamFactory.h"


class CMpegToWavFileOpen : public COpenFileDlg
{
public:
	CMpegToWavFileOpen( CUString directory, CUString fileType, int iconId, BOOL	bOpenDlg = TRUE, CWnd* pParent  = NULL );

	virtual ~CMpegToWavFileOpen();
	virtual void OnSettings();

	DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// CMP3ToWavDlg dialog

class CMP3ToWavDlg : public CDialog
{
private:
	CEvent	m_eThreadFinished;
	CWnd*	m_pParent;
	WORD	m_wCurrentTrack;
	BOOL	m_bAbortThread;
	CString m_strMP3FileName;
	CString m_strWavFileName;
	BOOL	m_bYesToAll;

public:
	// CONSTRUCTOR
	CMP3ToWavDlg(CWnd* pParent = NULL);

	// DESTRUCTOR
	~CMP3ToWavDlg();

	COpenFileDlg*		m_pFileDlg;
	POSITION			m_Pos;
	int					m_nCurFile;
	int					m_nTotalFiles;
	CTimeTrack			m_TimeTrack;
	CWinThread*			m_pThread;
	int					m_nPercentCompleted;
	ISndStream*			m_pInStream;

	static UINT ConvertFunc(PVOID pParams);


// Dialog Data
	//{{AFX_DATA(CMP3ToWavDlg)
	enum { IDD = IDD_MP3TOWAV };
	CProgressCtrl	m_Progress;
	CString	m_strFileName;
	CString	m_strTimeInfo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMP3ToWavDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMP3ToWavDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MP3TOWAVDLG_H__2E69F3A5_FDB1_11D2_A2D2_444553540001__INCLUDED_)
