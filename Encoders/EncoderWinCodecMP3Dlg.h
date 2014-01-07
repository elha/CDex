/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
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


#ifndef ENCODERWINCODECMP3DLG_H_INCLUDED
#define ENCODERWINCODECMP3DLG_H_INCLUDED

#include "EncoderDlg.h"

class CEncoderWinCodecMP3Dlg : public CEncoderDlg
{
private:
	BOOL		m_bInit;
	CEncoder*	m_pEncoder;
protected:
	INT			m_nCodecID;
public:
	// CONSTRUCTOR
	CEncoderWinCodecMP3Dlg();

	// MUTATORS
	virtual void SetControls(CEncoder* pEncoder);
	virtual void GetControls(CEncoder* pEncoder);
	UINT GetIDD();

// Dialog Data
	//{{AFX_DATA(CEncoderWinCodecMP3Dlg)
	enum { IDD = IDD_ENCODERWINCODECMP3DLG };
	CComboBox	m_Formats;
	CComboBox	m_Codecs;
	BOOL	m_bEncDuringRead;
	BOOL	m_bHighQuality;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEncoderWinCodecMP3Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEncoderWinCodecMP3Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnCodecoptions();
	afx_msg void OnSelchangeWincodec();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


class CWinEncOptsULaw : public CEncoderWinCodecMP3Dlg
{
public:
	CWinEncOptsULaw();	
};


class CWinEncOptsScx : public CEncoderWinCodecMP3Dlg
{
public:
	CWinEncOptsScx();	
};


#endif // ENCODERWINCODECMP3DLG_H_INCLUDED
