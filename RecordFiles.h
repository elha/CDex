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


#ifndef RECORD_FILES_H_INCLUDED
#define RECORD_FILES_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ID3Tag.h"
#include "Tasks.h"
#include "DDXGetFolder.h"
#include "Encode.h"

/////////////////////////////////////////////////////////////////////////////
// CRecordFiles dialog

class CRecordFiles : public CDialog
{
// Construction
public:
	CRecordFiles(CWnd* pParent = NULL);
	virtual ~CRecordFiles();

	void SetControls();

// Dialog Data
	//{{AFX_DATA(CRecordFiles)
	enum { IDD = IDD_RECORDFILES };
	CGetFolderControl	m_RecOutputDir;
	CComboBox	m_EncoderType;
	CEdit		m_RecordTime;
	CButton		m_FreeLength;
	CComboBox	m_RecordingDevice;
	CButton		m_Channels;
	CComboBox	m_SampleRate;
	CButton		m_Pause;
	CButton		m_Stop;
	CButton		m_Record;
	CString		m_strBitRate;
	CString		m_strFileName;
	int			m_nSampleRate;
	int			m_nChannels;
	int			m_nTimeLimited;
	CString		m_strRecFileName;
	int			m_nRecordTime;
	CString		m_strRecordStatus;
	int			m_nEncoderType;
	BOOL		m_bAddSequenceNumber;
	UINT		m_nSequenceStart;
	//}}AFX_DATA

	static UINT CRecordFiles::RecordThreadFunc(PVOID pParams);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecordFiles)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRecordFiles)
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnRecord();
	afx_msg void OnStop();
	afx_msg void OnPause();
	afx_msg void OnFreelength();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int				m_nTotalTime;
	int				m_nCurrentTime;
	int				m_nStatus;
	CPropertySheet	m_FileInfoSheet;
	DWORD			m_dwBitRate;
	DWORD			m_nPercentCompleted;
	CWinThread*		m_pThread;
	BOOL			m_bAbort;
	static CTasks	m_Tasks;
	CDEX_ERR		m_bThreadError;
	BOOL			m_bIncrementSeqNr;

	CEncoder*		m_pEncoder[ NUMENCODERS + 1 ];
	int				m_nEncoders;

	void SetEncoderTypes();
	CTasks&			GetTasks() { return m_Tasks; }
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // RECORD_FILES_H_INCLUDED
