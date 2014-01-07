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
#include "EncoderBladeDllDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBldEncOpts dialog

CBldEncOpts::CBldEncOpts() 
	: CEncoderDlg(CBldEncOpts::IDD)
{
	//{{AFX_DATA_INIT(CBldEncOpts)
	m_nMode = -1;
	m_bCopyRight = FALSE;
	m_bCRC = FALSE;
	m_bOriginal = FALSE;
	m_bPrivate = FALSE;
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
}


void CBldEncOpts::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBldEncOpts)
	DDX_Control(pDX, IDC_BITRATE, m_MinBitrate);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nMode);
	DDX_Check(pDX, IDC_COPYRIGHT, m_bCopyRight);
	DDX_Check(pDX, IDC_CRC, m_bCRC);
	DDX_Check(pDX, IDC_ORIGINAL, m_bOriginal);
	DDX_Check(pDX, IDC_PRIVATE, m_bPrivate);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CBldEncOpts, CEncoderDlg)
	//{{AFX_MSG_MAP(CBldEncOpts)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBldEncOpts message handlers

BOOL CBldEncOpts::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CBldEncOpts::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	pEncoder->SetOnTheFlyEncoding(m_bEncDuringRead);
	pEncoder->SetPrivate(m_bPrivate);
	pEncoder->SetCRC(m_bCRC);
	pEncoder->SetOriginal(m_bOriginal);
	pEncoder->SetCopyRight(m_bCopyRight);
	pEncoder->SetMode(m_nMode);

	// Get bitrate selection
	pEncoder->SetBitrate(GetMinBitrate());
}

void CBldEncOpts::SetControls(CEncoder* pEncoder)
{
	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();
	m_bPrivate=pEncoder->GetPrivate();
	m_bCRC=pEncoder->GetCRC();
	m_bOriginal=pEncoder->GetOriginal();
	m_bCopyRight=pEncoder->GetCopyRight();
	m_nMode=pEncoder->GetMode();

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Set the proper bitrate
	SetMinBitrate(pEncoder->GetBitrate());

	// Set data to controls
	UpdateData(FALSE);
}




UINT CBldEncOpts::GetIDD()
{
	return IDD;
}
