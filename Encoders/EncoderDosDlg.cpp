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
#include "CDex.h"
#include "EncoderDosDlg.h"
#include "EncoderDos.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// EXTERN ENCODER OPTION 
CEncoderDosDlg::CEncoderDosDlg() 
	: CEncoderDlg(CEncoderDosDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderDosDlg)
	m_strExtEncOpts = _T("");
	m_bEncDuringRead = FALSE;
	m_bHideDosBox = FALSE;
	m_strExtention = _T("");
	m_bSendWavHeader = FALSE;
	//}}AFX_DATA_INIT
}



void CEncoderDosDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderDosDlg)
	DDX_Text(pDX, IDC_EXTENCOPTS, m_strExtEncOpts);
	DDX_GetFileControl(pDX, IDC_EXTENC, m_cExtEncPath, 0, g_language.GetString( IDS_EXTENCFILESELECTION ) );   
	DDX_Check(pDX, IDC_ON_THE_FLY, m_bEncDuringRead);
	DDX_Check(pDX, IDC_HIDE_DOSBOX, m_bHideDosBox);
	DDX_Text(pDX, IDC_EXTENTION, m_strExtention);
	DDX_Control(pDX, IDC_BITRATE, m_MinBitrate);
	DDX_Check(pDX, IDC_SENDWAVTOSTDIN, m_bSendWavHeader);
	//}}AFX_DATA_MAP
}


void CEncoderDosDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	CString strTmp;
	m_cExtEncPath.GetWindowText( strTmp );
	pEncoder->SetUser1( CUString( strTmp ) );
	pEncoder->SetUser2( CUString( m_strExtEncOpts ));
	pEncoder->SetOnTheFlyEncoding( m_bEncDuringRead );
	pEncoder->SetChunkSupport( m_bEncDuringRead );
	pEncoder->SetUserN1( m_bHideDosBox );
	pEncoder->SetExtention( CUString( m_strExtention ) );

	((CEncoderDos*)pEncoder)->SetAddStdinWavHeader( m_bSendWavHeader );

	// Get bitrate selection
	pEncoder->SetBitrate(GetMinBitrate());
}

void CEncoderDosDlg::SetControls(CEncoder* pEncoder)
{
	// Set control items based in pEncoder information
    CUStringConvert strCnv;

    m_cExtEncPath.SetWindowText( strCnv.ToT( pEncoder->GetUser1() ) );

	m_strExtEncOpts=pEncoder->GetUser2();
	m_strExtention= pEncoder->GetExtension();

	m_bEncDuringRead = pEncoder->GetOnTheFlyEncoding();
	m_bHideDosBox = pEncoder->GetUserN1( );

	m_bSendWavHeader = ((CEncoderDos*)pEncoder)->GetAddStdinWavHeader();

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Set the proper bitrate
	SetMinBitrate(pEncoder->GetBitrate());

	// Set data to controls
	UpdateData(FALSE);
}


BEGIN_MESSAGE_MAP(CEncoderDosDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderDosDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderDosDlg message handlers

BOOL CEncoderDosDlg::OnInitDialog() 
{

	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


UINT CEncoderDosDlg::GetIDD()
{
	return IDD;
}
