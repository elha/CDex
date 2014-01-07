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


#ifndef CDEXVIEW_H_INCLUDED
#define CDEXVIEW_H_INCLUDED

#include "CDPlay.h"
#include "Filename.h"

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "ID3Tag.h"
#include "LineList.h"
#include "HyperLink.h"
#include "RipInfoDB.h"
#include "OpenFileDlg.h"
#include "UtilToolBar.h"
#include "MusicBrainz.h"


class CCopyDialog;
class CMP3ToWavDlg;

class CCDexView : public CFormView
{
private:

	CCopyDialog*	m_pCopyDlg;
	CMP3ToWavDlg*	m_pConvertDlg;
	CUtilToolBar*	m_pUtilToolBar;
	CCDPlay			m_cCDPlay;
	bool			m_noPreGaps;
    bool            m_noIsrc;
public:
	LONG			m_lStatus;

	int				m_nCurPlayTrack;
	int				m_nNumPlayTracks;
	BYTE			m_PlayTracksArray[100];
	BOOL			m_bMultiPlay;
	LONG			m_lOldStatus;
	CFileDialog*	m_pFileDlg;
	CStatusBar*		m_pStatus;
	DWORD			m_dwTimer;
	CRipInfoDB		m_RipInfoDB;
	BOOL			m_bEjected;


protected: // create from serialization only
	CCDexView();
	DECLARE_DYNCREATE(CCDexView)
// Form Data
public:
	//{{AFX_DATA(CCDexView)
	enum { IDD = IDD_TITLEINFO };
	CEdit	m_TrackOffset;
	CEdit	m_Year;
	CEdit	m_Title;
	CEdit	m_Artist;
	CComboBox	m_Genre;
	LListCtrl	m_TrackList;
	CString	m_strYear;
	CString	m_strTitle;
	CString	m_strArtist;
	INT	m_nTrackOffset;
	//}}AFX_DATA
	void EjectCD(BOOL bEject,BOOL bOpenTray);

// Attributes
public:
// Operations
public:
	CCDexDoc*	GetDocument();
	void		OnNewCD();
	void		PlaceControls();
	void		GetPlayTracks();
	void		OnGetTrack2XXX(int nWavOrMp3);
	void		SaveCDInfo();
	void		UpdateCDInfo();
	void		SetStatusBar( LPCTSTR lpszText );
	void		ConvertWav2MP3();
	void		OnSelectConfigure(int nSelectTab);
	void		UpdateTrackStatus();
	
	void		FormatTrackName( ENUM_FILENAME_CONVERTS nType );

	void		AddProfileStrings();
	DWORD		SetRibDBInfo( CTaskInfo& newTask, WORD wTrack ); 


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDexView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint );
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCDexView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	void ShutDown( );
	BOOL SetCurrentPrivilege (LPCTSTR Privilege, BOOL bEnablePrivilege);
	void UpdateGenreSelection( const CString& strSelection );
	void UpdateListControls();

	afx_msg void OnEject();

	afx_msg void OnProfileEditChange();
protected:
	afx_msg LRESULT OnSeekButton( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnWinAmpPlugInFinished( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnSeekButtonUp( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnUpdateTrackStatus( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnEditNextTrack( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnChildDialogClosed( WPARAM wParam,LPARAM lParam );

// Generated message map functions
protected:
	//{{AFX_MSG(CCDexView)
	afx_msg void OnClose();
	afx_msg void OnConfigure();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTracks2wav();
	afx_msg void OnTracks2mp3();
	afx_msg void OnWav2mp3();
	afx_msg void OnSysteminfo();
	afx_msg void OnPtracks2mp3();
	afx_msg void OnNexttrack();
	afx_msg void OnPrevtrack();
	afx_msg void OnStopbutton();
	afx_msg void OnUpdateCDSelBar(CCmdUI* pCmdUI);
	afx_msg void OnUpdateConfigCD(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePtracks2mp3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSkipforward(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSkipback(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePrevtrack(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStopbutton(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTracks2mp3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTracks2wav(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWav2mp3(CCmdUI* pCmdUI);
	afx_msg void OnChange();
	afx_msg void OnRemotecddb();
	afx_msg void OnUpdateCDDB(CCmdUI* pCmdUI);
	afx_msg void OnPlaytrack();
	afx_msg void OnUpdatePlaytrack(CCmdUI* pCmdUI);
	afx_msg void OnPause();
	afx_msg void OnUpdatePause(CCmdUI* pCmdUI);
	afx_msg void OnUpdateNexttrack(CCmdUI* pCmdUI);
	afx_msg void OnRclickTracklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditTracklist(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnUpdateConfigure(CCmdUI* pCmdUI);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnReadcdplayerini();
	afx_msg void OnReadlocalcddb();
	afx_msg void OnDestroy();
	afx_msg void OnMp3toriffwav();
	afx_msg void OnUpdateMp3toriffwav(CCmdUI* pCmdUI);
	afx_msg void OnEditRenametrack();
	afx_msg void OnEditSelectall();
	afx_msg void OnEditRefresh();
	afx_msg void OnEditCopytrackstoclipboard();
	afx_msg void OnCddbSavetocdplayerini();
	afx_msg void OnCddbSavetolocalcddb();
	virtual LRESULT OnChangeCDSelection(WPARAM nSelection,LPARAM lParam);
	virtual LRESULT OnChangeProfileSelection(WPARAM nSelection,LPARAM lParam);
	afx_msg void OnConfigcd();
	afx_msg void OnUpdateViewerrorlog(CCmdUI* pCmdUI);
	afx_msg void OnMp3towav();
	afx_msg void OnUpdateMp3towav(CCmdUI* pCmdUI);
	afx_msg void OnViewripstatus();
	afx_msg void OnComparefiles();
	afx_msg void OnPlayfiles();
	afx_msg void OnUpdatePlayfiles(CCmdUI* pCmdUI);
	afx_msg void OnOptionClearstatus();
	afx_msg void OnRiffwavtomp3();
	afx_msg void OnToolsRecordfiles();
	afx_msg void OnCddbSubmittoremotecddb();
	afx_msg void OnCddbReadcdtext();
	afx_msg void OnCddbBatchQuery();
	afx_msg void OnToolsBurn();
	afx_msg void OnMpegtompeg();
	afx_msg void OnUpdateMpegtompeg(CCmdUI* pCmdUI);
	afx_msg void OnToolsSearchtrackindex();
	afx_msg void OnTracknameModAl();
	afx_msg void OnTracknameModAu();
	afx_msg void OnTracknameModFcc();
	afx_msg void OnTracknameModFcew();
	afx_msg void OnUpdateTracknameModAl(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTracknameModAu(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTracknameModFcc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTracknameModFcew(CCmdUI* pCmdUI);
	afx_msg void OnConfigureWinPlugins();
	afx_msg void OnUpdateConfigureWinPlugins(CCmdUI* pCmdUI);
	afx_msg void OnDblclkGenre();
	afx_msg void OnEditAddeditgenres();
	afx_msg void OnEditModifytracknameExchangeartisttrack();
	afx_msg void OnChangeTrackoffset();
	afx_msg void OnDeleteprofile();
	afx_msg void OnUpdateDeleteprofile(CCmdUI* pCmdUI);
	afx_msg void OnSaveprofile();
	afx_msg void OnUpdateSaveprofile(CCmdUI* pCmdUI);
	afx_msg void OnCddbWinampdb();
	afx_msg void OnUpdateCddbWinampdb(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEject(CCmdUI* pCmdUI);
	afx_msg void OnCddbReadfrommusicbrainz();
	afx_msg void OnUpdateCddbReadfrommusicbrainz(CCmdUI* pCmdUI);
	afx_msg void OnKillfocusTrackoffset();
	//}}AFX_MSG
	afx_msg void OnMbrainz();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnToolsGeneratecuesheet();
    afx_msg void OnToolsDetectpre();
    afx_msg void OnToolsReadupc();
};

#ifndef _DEBUG  // debug version in CDexView.cpp
inline CCDexDoc* CCDexView::GetDocument()
   { return (CCDexDoc*)m_pDocument; }
#endif


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
