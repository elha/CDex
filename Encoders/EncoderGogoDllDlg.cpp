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


#include "stdafx.h"
#include "cdex.h"
#include "EncoderGogoDllDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEncoderGogoDllDlg::CEncoderGogoDllDlg() 
	: CEncoderDlg(CEncoderGogoDllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderGogoDllDlg)
	m_nMode = -1;
	m_bUseSIMD = FALSE;
	m_bUsePsy = FALSE;
	m_nVersion = 0;
	m_bUseLPF = FALSE;
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
}


void CEncoderGogoDllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderGogoDllDlg)
	DDX_Control(pDX, IDC_MINBITRATE, m_MinBitrate);
	DDX_Control(pDX, IDC_VBR, m_VBR);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nMode);
	DDX_Check(pDX, IDC_USE_SIMD, m_bUseSIMD);
	DDX_Check(pDX, IDC_USE_PSY, m_bUsePsy);
	DDX_Check(pDX, IDC_USE_LPF, m_bUseLPF);
	DDX_Radio(pDX, IDC_VERSION, m_nVersion);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEncoderGogoDllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderGogoDllDlg)
	ON_BN_CLICKED(IDC_VERSION, OnVersion)
	ON_BN_CLICKED(IDC_VERSION1, OnVersion2)
	ON_CBN_SELCHANGE(IDC_VBR, OnSelchangeVbr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderGogoDllDlg message handlers

BOOL CEncoderGogoDllDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEncoderGogoDllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	pEncoder->SetOnTheFlyEncoding(m_bEncDuringRead);
	pEncoder->SetMode(m_nMode);
	pEncoder->SetVersion(m_nVersion);
	pEncoder->SetUserN1(m_bUseSIMD + (m_bUsePsy<<1) + (m_bUseLPF<<2) );

	// USE CRC TO PACK ADDITIONAL INFO
	// First 4 bits are reserved for mode, 2nd for bits for g_Psycho, third nible for Quality
	int nVBR=m_VBR.GetCurSel();

	int nCRC=((nVBR&0x0F)<<12);
	pEncoder->SetCRC(nCRC);


	// Set bitrate selection
	pEncoder->SetBitrate(GetMinBitrate());
}

void CEncoderGogoDllDlg::SetControls(CEncoder* pEncoder)
{
	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();
	m_nMode=pEncoder->GetMode();
	m_nVersion=pEncoder->GetVersion();

	int nTemp=pEncoder->GetUserN1();
	
	m_bUseSIMD=(nTemp&0x01)?TRUE:FALSE;
	m_bUsePsy=(nTemp&0x02)?TRUE:FALSE;
	m_bUseLPF=(nTemp&0x04)?TRUE:FALSE;

	int nCRC=pEncoder->GetCRC();
	int nVBR=(nCRC>>12)&0x0F;
	m_VBR.SetCurSel(nVBR);

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Set the proper bitrate
	SetMinBitrate(pEncoder->GetBitrate());

	// Set data to controls
	UpdateData(FALSE);
}

void CEncoderGogoDllDlg::OnVersion() 
{
	m_nVersion=0;

	// Fill Min bitrate table
	FillMinBitrateTable();

	SetMinBitrate(128);
}


void CEncoderGogoDllDlg::OnVersion2() 
{
	m_nVersion=1;

	// Fill Min bitrate table
	FillMinBitrateTable();

	SetMinBitrate(64);
}

void CEncoderGogoDllDlg::OnSelchangeVbr() 
{
	UpdateData(TRUE);
}

UINT CEncoderGogoDllDlg::GetIDD()
{
	return IDD;
}
