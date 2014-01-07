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


#ifndef CDEX_H_INCLUDED
#define CDEX_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

#include "cdrip\cdrip.h"

/////////////////////////////////////////////////////////////////////////////
// CCDexApp:
// See CDex.cpp for the implementation of this class
//

class CCDexApp : public CWinApp
{
//	LExceptionLog	m_clExceptionLog;
    CFile		m_scanning_para_file;
	CFile		m_ana_para_file;
    BOOL		m_scanning_file_open;
	BOOL		m_ana_file_open;
	int			m_LastFilter;
	bool		m_bPlayer;
	bool		m_bAutoRip;
	HINSTANCE	m_hLibSndFileDll;
	HINSTANCE	m_hCDRipDll;
	HINSTANCE	m_hMusicBrainzDll;
	CMutex		m_mtPlayerActive;
	CMutex		m_mtInitializing;
	CUString		m_strCommandLineParams;
public:

public:
	CCDexApp();
	~CCDexApp();
	void SetAutoPlay(BOOL bReStore);
	CDEX_ERR LoadCDRipLibrary();
	bool	GetPlayer() const { return m_bPlayer; }
	bool	GetAutoRip() const { return m_bAutoRip; }
	CUString GetCommandLineParams() const { return m_strCommandLineParams; }
	void	SetInitialized( ) { m_mtInitializing.Unlock(); }
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDexApp)
	public:
	virtual BOOL InitInstance();
	virtual LRESULT ProcessWndProcException( CException* e, const MSG* pMsg );
	virtual int ExitInstance();
    virtual void WinHelp( DWORD dwData, UINT nCmd );
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCDexApp)
	afx_msg void OnAppAbout();
	afx_msg void OnViewerrorlog();
	afx_msg void OnFileNew( );

	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
