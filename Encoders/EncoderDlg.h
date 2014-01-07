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


#ifndef ENCOPTSDLG_H_INCLUDED
#define ENCOPTSDLG_H_INCLUDED

// EncOptsDlg.h : header file
//
#include "resource.h"

extern const int MP3SampleRates[3][3];


// Foreward class declaration
class CEncoder;

class CEncoderDlg : public CDialog
{
protected:
	int			m_nVersion;
	int			m_nLayer;
	CComboBox	m_MinBitrate;
	CComboBox	m_MaxBitrate;
	CComboBox	m_OutSampleRate;
public:
	CEncoderDlg(UINT nIDTemplate, CWnd* pParentWnd = NULL ) ;
	virtual void OnOk() {};
	virtual void OnCancel() {};

	// ACCESSORS
	virtual UINT GetIDD()=0;
	// MUTATORS
	virtual void SetControls(CEncoder* m_pEncoder)=0;
	virtual void GetControls(CEncoder* m_pEncoder)=0;
	virtual void FillMaxBitrateTable();
	virtual void FillMinBitrateTable();
	virtual void FillOutSampleRateCtrl();
	virtual int  GetMinBitrate();
	virtual int  GetMaxBitrate();
	virtual void SetMinBitrate(int nBitrate);
	virtual void SetMaxBitrate(int nBitrate);
};









//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif

