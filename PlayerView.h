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


#ifndef PLAYERVIEW_H_INCLUDED
#define PLAYERVIEW_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// PlayerView.h : header file
//

#include "TransBmp.h"


#include "PlayWavStream.h"
#include "ISndStream.h"
#include "MixerFader.h"
#include "ID3Tag.h"
#include "SeekSlider.h"


/////////////////////////////////////////////////////////////////////////////
// CPlayerView form view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CPlayerView : public CFormView
{
private:
	vector<CString> m_vFileNames;

	CMixerFader		m_Volume;

	void			SetControls();
	LONG			OnWinAmpPlugWmUserMessage(WPARAM wParam,LPARAM lParam);
	static UINT		PlayThreadFunc(PVOID pParams);
	CEvent			m_eThreadFinished;
	CWinThread*		m_pThread;
	BOOL			m_bAbortCurrent;
	BOOL			m_bAbortThread;
	DWORD			m_dwSeekOffset;
	CTransBmp		m_cVolumeBitmap;
public:
	ISndStream*		m_pInStream;
	PlayWavStream*	m_pPlayStream;
	int				m_nTotalTime;
	int				m_nCurrentTime;
	int				m_nStatus;
	DWORD			m_dwBitRate;
	CEvent			m_ePaused;
	CEvent			m_eSeeking;
	BOOL			m_bPlayRandom;

protected:
	CPlayerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPlayerView)

// Form Data
public:
	typedef enum {	THREAD_PLAY_NEXT_TRACK,
					THREAD_PLAY_PREV_TRACK,
					THREAD_PLAY_FINISH,
					THREAD_PLAY_TRACK } THREAD_COMMAND;

	THREAD_COMMAND	m_ThreadCommand;

	THREAD_COMMAND	GetThreadCommand() const { return m_ThreadCommand;}
	void SetThreadCommand( THREAD_COMMAND nCmd ) { m_ThreadCommand = nCmd;}

	BOOL	GetPlayRandom() const { return m_bPlayRandom; }
	void	SetPlayRandom( BOOL bValue ) { m_bPlayRandom = bValue; }

	void	AddToPlayList( const CString& strFileName );
	void	PlaceControls();
	void	AddMultiPlayerFiles( const CString& strAddFileNames );

	//{{AFX_DATA(CPlayerView)
	enum { IDD = IDD_PLAYERVIEW };
	CListBox	m_PlayList;
	CStatic		m_TimeCtrl;
	CSeekSlider		m_Position;
	CString			m_strFileName;
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	afx_msg LRESULT	OnStartSeek (WPARAM wParam, LPARAM lParam ) ;
	afx_msg LRESULT OnStopSeek( WPARAM wParam, LPARAM lParam ) ;
	afx_msg	LRESULT OnWinAmpPlugInFinished( WPARAM wParam, LPARAM lParam );
	afx_msg	LONG	OnWinAmpPlugWmUser0( WPARAM wParam, LPARAM lParam );
	void			AddStringToPlayList( const CString& strAdd ) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlayerView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnDropFiles(HDROP hDropInfo);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPlayerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CPlayerView)
	afx_msg void OnDetails();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFileOpen();
	afx_msg void OnPrev();
	afx_msg void OnNext();
	afx_msg void OnPlay();
	afx_msg void OnPause();
	afx_msg void OnStop();
	afx_msg void OnUpdateNext(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePlay(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrev(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStop(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileopen(CCmdUI* pCmdUI);
	afx_msg void OnDestroy();
	afx_msg void OnConfigureWinPlugins();
	afx_msg void OnSelchangePlaylist();
	afx_msg void OnDblclkPlaylist();
	afx_msg void OnPlaylistClear();
	afx_msg void OnPlaylistSave();
	afx_msg void OnOptionsPlayrandom();
	afx_msg void OnUpdateOptionsPlayrandom(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG
//	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PLAYERVIEW_H__D47AF043_7BC8_11D5_A2DB_005004EF8536__INCLUDED_)
