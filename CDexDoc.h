/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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


#if !defined(AFX_CDEXDOC_H__8F4F6B5B_3F3C_11D2_8704_A9E3FCB3091B__INCLUDED_)
#define AFX_CDEXDOC_H__8F4F6B5B_3F3C_11D2_8704_A9E3FCB3091B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Config.h"
#include "ID3Tag.h"
#include "CopyDlg.h"
#include "CDInfo.h"


typedef enum CDROMSTATUS {	CD_DISABLED,
							CD_IDLE,
							CD_EJECT,
							CD_NEWCD,
							CD_NONE,
							CD_PLAYSINGLE,
							CD_PLAYMULTI,
							CD_RECORDING,
							CD_PREPRECORDING,
							CD_RECFINISHED,
							CD_PLAYFINISHED
						};


class CCDexDoc : public CDocument
{
public:
	DWORD			m_dwVolID;
	DWORD			m_dwDiscID;
	CDROMSTATUS		m_Status;
	HACCEL			m_hAccel;
	CCopyDialog*	m_pCopyDlg;
//	CWaveCnvDlg*	m_pCnvDlg;
//	ID3Tag			m_ID3Tag;
	BOOL			m_bEjected;
	BOOL			m_bSaveToIni;
	CDInfo			m_CDInfo;

protected: // create from serialization only
	CCDexDoc();
	DECLARE_DYNCREATE(CCDexDoc)


// Attributes
public:
	CDInfo&			GetCDInfo() {return m_CDInfo;}
	void			InitCDRom();
	void			ReadCDInfo();


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDexDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL SaveModified();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCDexDoc();
	BOOL	SaveToIni() const {return m_bSaveToIni;}
	void 	SaveToIni(BOOL bValue) {m_bSaveToIni=bValue;}

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CCDexDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDEXDOC_H__8F4F6B5B_3F3C_11D2_8704_A9E3FCB3091B__INCLUDED_)
