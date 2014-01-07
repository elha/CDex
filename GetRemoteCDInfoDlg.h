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


#ifndef GETREMOTECDINFODLG_H_INCLUDED
#define GETREMOTECDINFODLG_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// CDdbDlg.h : header file
//

#include "Resource.h"
#include "CDinfo.h"
#include "LangDialog.h"
#include "afxwin.h"
#include "cddb.h"
#include "afxcmn.h"
#include "LineList.h"


extern BOOL	g_bAbortCDDB;	

typedef UINT (*CDDBTHREADPROC) (VOID*, CWnd*, volatile BOOL& bAbort );


/////////////////////////////////////////////////////////////////////////////
// CGetRemoteCDInfoDlg dialog

class CGetRemoteCDInfoDlg : public CLangDialog
{
private:
    UINT            m_threadAction;
	CWinThread*		m_pThread;
	void*			m_pThreadParam;
	CDDBTHREADPROC	m_pThreadProc;
	CEvent			m_eThreadFinished;
	BOOL			m_bAbort;
	CDdb            m_localCDDB;
    bool            m_bAbortThread;
    CUString         m_strQueryResult;
    CUString         m_strReadResult;
    CUString         m_strCatagory;
    vector<CHAR>    m_vReadResult;
    CUString          m_strDiskID;
    UINT            m_codePage;
    bool            m_bShowMultiMatch;
    UINT            m_state;
    CDInfo*         m_pCDInfo;
    bool            m_isBatch;
public:
    CDDB_ERROR OpenCDDBConnection();
    CDDB_ERROR CloseCDDBConnection();
    CDDB_ERROR QueryFromCDDB();
    CDDB_ERROR ReadFromCDDB();

    void DisplayMultiMatch();

	// Construction
	CGetRemoteCDInfoDlg( CDInfo* pCDInfo, bool isBatch, CWnd* pParent = NULL);

    void AddText( const CUString& strAddText );
	void CloseDialog() {OnCancel();}
	void SetAction( const CUString& strAction );
	static UINT RemoteThread( void* pParams );
    // void SetTagData( const CTagData& value ) { m_localCDDB.SetTagData( value ); }
    CUString GetCDDBResponseString( BOOL bQuery, UINT responseCode );
    void SetResponse( const CUString& strResponse ); 
    void SetCDDBResponse( CDDB_ERROR returnCode );
    void ParseAndWriteData();
    BOOL QueryNewEntry(); 

    bool SetPreviewData();
	CUString	m_strAction;
	CUString	m_strResponse;
    vector<CUString> m_vBatchLines;
    DWORD       m_currentBatchIndex;
// Dialog Data
	//{{AFX_DATA(CGetRemoteCDInfoDlg)
	enum { IDD = IDD_GETREMOTECDINFO };
	CEdit	m_Output;
	CUString	m_strHist;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGetRemoteCDInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
    INT SetupNextBatchEntry(); 
    void ReadBatch(); 

	// Generated message map functions
	//{{AFX_MSG(CGetRemoteCDInfoDlg)
	virtual void OnCancel();
	afx_msg void OnAbort();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg	LONG OnInfoMsg( WPARAM wParam, LPARAM lParam );
	afx_msg	LONG OnActionMsg( WPARAM wParam, LPARAM lParam );
	afx_msg LONG OnFinished( WPARAM wParam, LPARAM lParam );
	afx_msg	LONG OnCloseDialog( WPARAM wParam,LPARAM lParam );
	afx_msg void OnCddbResubmit();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEnChangePreview();
    CComboBox m_comboCodePage;
    LListCtrl m_listControl;
    CButton m_getButton;
    afx_msg void OnBnClickedCddbRead();
    afx_msg void OnCbnSelchangeCodepageselection();
    afx_msg void OnBnClickedOk();
    afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
    CButton m_resubmitCheck;
    

    afx_msg void OnBnClickedCancel();
};


//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.



#endif // !defined(AFX_CDDBDLG_H__5DCC5BA2_499E_11D2_8705_AB15F5F39A1A__INCLUDED_)
