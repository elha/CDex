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


#include "StdAfx.h"
#include "EncoderMP2DllDlg.h"
#include "Encode.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CEncoderMP2DllDlg::CEncoderMP2DllDlg() 
	: CEncoderDlg(CEncoderMP2DllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderMP2DllDlg)
	m_nMode = -1;
	m_bCopyRight = FALSE;
	m_bCRC = FALSE;
	m_bOriginal = FALSE;
	m_bPrivate = FALSE;
	m_bNoPadding = FALSE;
	m_bWriteAncil = FALSE;
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
	m_nLayer=1;
}


void CEncoderMP2DllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderMP2DllDlg)
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nMode);
	DDX_Check(pDX, IDC_COPYRIGHT, m_bCopyRight);
	DDX_Check(pDX, IDC_CRC, m_bCRC);
	DDX_Check(pDX, IDC_ORIGINAL, m_bOriginal);
	DDX_Check(pDX, IDC_PRIVATE, m_bPrivate);
	DDX_Check(pDX, IDC_NO_PADDING, m_bNoPadding);
	DDX_Check(pDX, IDC_WRITE_ANCIL, m_bWriteAncil);
	DDX_Control(pDX, IDC_BITRATE, m_MinBitrate);
	DDX_Control(pDX, IDC_VBR, m_VBR);
	DDX_Radio(pDX, IDC_VERSION, m_nVersion);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEncoderMP2DllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderMP2DllDlg)
	ON_BN_CLICKED(IDC_VERSION, OnVersion)
	ON_BN_CLICKED(IDC_VERSION1, OnVersion2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderMP2DllDlg message handlers

BOOL CEncoderMP2DllDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEncoderMP2DllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	int nCRC=m_bCRC;
	nCRC+=(m_bWriteAncil==TRUE)?2:0;
	nCRC+=(m_bNoPadding==TRUE)?4:0;
	nCRC+=( ( m_VBR.GetCurSel() & 0x0F ) << 12 );

	// Set items
	pEncoder->SetOnTheFlyEncoding(m_bEncDuringRead);
	pEncoder->SetPrivate(m_bPrivate);
	pEncoder->SetCRC(nCRC);
	pEncoder->SetOriginal(m_bOriginal);
	pEncoder->SetCopyRight(m_bCopyRight);
	pEncoder->SetMode(m_nMode);

	pEncoder->SetCRC(nCRC);
	pEncoder->SetVersion(m_nVersion);

	// Get bitrate selection
	pEncoder->SetBitrate(GetMinBitrate());
}

void CEncoderMP2DllDlg::SetControls(CEncoder* pEncoder)
{
	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();
	m_bPrivate=pEncoder->GetPrivate();
	m_bCRC=(pEncoder->GetCRC()&0x01)?1:0;
	m_bOriginal=pEncoder->GetOriginal();
	m_bCopyRight=pEncoder->GetCopyRight();
	m_nMode=pEncoder->GetMode();
	m_bWriteAncil = (pEncoder->GetCRC()&0x02)?1:0;
	m_bNoPadding = (pEncoder->GetCRC()&0x04)?1:0;

	if ( m_VBR.m_hWnd )
	{
		m_VBR.SetCurSel( (pEncoder->GetCRC()>>12) );
	}

	// Get layer version
	m_nVersion=pEncoder->GetVersion();

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Set the proper bitrate
	SetMinBitrate(pEncoder->GetBitrate());

	// Set data to controls
	UpdateData(FALSE);
}

void CEncoderMP2DllDlg::OnVersion() 
{
	m_nVersion=0;

	// Fill Min bitrate table
	FillMinBitrateTable();

	SetMinBitrate(192);
}

void CEncoderMP2DllDlg::OnVersion2() 
{
	m_nVersion=1;

	// Fill Min bitrate table
	FillMinBitrateTable();

	SetMinBitrate( 96 );
}

UINT CEncoderMP2DllDlg::GetIDD()
{
	return IDD;
}
