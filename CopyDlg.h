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


#ifndef COPYDLG_INCLUDED
#define COPYDLG_INCLUDED

#include "CTimeTrack.h"
#include "Tasks.h"
#include "TextProgressCtrl.h"
#include "afxwin.h"

#define DO_READ			0x01
#define DO_NORMALZE		0x02
#define DO_CONVERT		0x04
#define DO_WRITETAG		0x08


// Forward class delcaration
class CCDexView;
class CRipInfoDB;



/////////////////////////////////////////////////////////////////////////////
// CCopyDialog dialog

class CCopyDialog : public CDialog
{
private:
	INT					m_iStatus;
	CTimeTrack			m_TimeTrack;
	CTimeTrack			m_totalTimeTrack;
	DWORD				m_dwTimer;
    INT                 m_nTotalTimeOnStatus;
	CMutex				m_mLockControls;
	CWinThread*			m_pThread;
	CEvent				m_eThreadFinished;
	CRipInfoDB*			m_pRipInfoDB;
	BOOL				m_bYesToAll;
	BOOL				m_bResetTimeTrack;
	volatile WORD		m_wCurrentTrack;
	BOOL				m_bRetainWavFile;
	volatile INT		m_nPercent;
	volatile BOOL		m_bAbortThread;
	volatile INT		m_nJitterErrors;
	volatile INT		m_nJitterPos;
	volatile BOOL		m_bCancelled;
	static CTasks		m_Tasks;
	static CTaskInfo	m_CurrentTask;
    ULONG               m_lCRC;

public:
	CCDexView*		m_pView;
public:
	// CONSTRUCTOR
	CCopyDialog( CCDexView* pView );

	// DESTRUCTOR
	~CCopyDialog();

	void		UpdateControls();
	void		SetupControls(int nStatus);

	static UINT RipFunc(PVOID pParams);
	static CTasks&		GetTasks() { return m_Tasks;}

// Dialog Data
	//{{AFX_DATA(CCopyDialog)
	enum { IDD = IDD_COPYDIALOG };
	CTextProgressCtrl m_TrackCtrl;
	CTextProgressCtrl m_JitterCtrl;
	CString	m_strInfo1;
	CString	m_strInfo2;
	CString	m_strPeakValue;
	CString	m_strInfo3;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCopyDialog)
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	static CDEX_ERR CalculateNormalizationFactor( CCopyDialog*	pDlg );

	static CDEX_ERR RipToEncoder(	CCopyDialog*	pDlg,
									ENCODER_TYPES	nEncoderType,
									BOOL			bIsTempFile,
									BOOL&			bNoToAll );

	static CDEX_ERR WavToMpeg(	CCopyDialog*	pDlg,
								INT&			nSampleRate,
								INT&			nChannels,
								BOOL			bWriteId3V2Tag,
								BOOL&			bNoToAll );

	static CDEX_ERR MP3ToRiffWav(	CCopyDialog*	pDlg,
									DWORD			dwBitRate,
									DWORD			dwSampleRate,
									INT				nNumChannels );

	static CDEX_ERR WriteId3Tag(	CCopyDialog*	pDlg,
									BOOL			bCanWriteTagV1,
									BOOL			bCanWriteTagV2 );

	static CDEX_ERR AddToPlayList(	CCopyDialog*	pDlg );

	static CDEX_ERR NormWav(		CCopyDialog*	pDlg,
									CUString			strWavFileName,
									BOOL			bReplace );

	static CDEX_ERR GetMaxWaveValue(	CCopyDialog*	pDlg );

	static CTaskInfo&	GetCurrentTask() { return m_CurrentTask;}
	static void SetCurrentTask( CTaskInfo& newTask ) { m_CurrentTask = 	newTask;}
public:
    CStatic m_strInfo4;
    CStatic m_strInfo5;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
