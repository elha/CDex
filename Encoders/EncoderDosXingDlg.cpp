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
#include "EncoderDosXingDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEncoderDosXingDlg::CEncoderDosXingDlg() 
	: CEncoderDlg(CEncoderDosXingDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderDosXingDlg)
	m_bCbrOrVbr = -1;
	m_bCopyright = FALSE;
	m_bDownmixToMono = FALSE;
	m_bFilterHighFreq = FALSE;
	m_bOriginal = FALSE;
	m_bSimpleStereo = FALSE;
	m_nVbrScale = 0;
	m_bEncChoose = FALSE;
	//}}AFX_DATA_INIT
}



void CEncoderDosXingDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderDosXingDlg)
	DDX_Control(pDX, IDC_BITRATE, m_MinBitrate);
	DDX_Radio(pDX, IDC_CBRORVBR, m_bCbrOrVbr);
	DDX_Check(pDX, IDC_COPYRIGHT, m_bCopyright);
	DDX_Check(pDX, IDC_DOWNMIX, m_bDownmixToMono);
	DDX_Check(pDX, IDC_FILTERHIGHFREQ, m_bFilterHighFreq);
	DDX_Check(pDX, IDC_ORIGINAL, m_bOriginal);
	DDX_Check(pDX, IDC_SIMPLESTEREO, m_bSimpleStereo);
	DDX_Text(pDX, IDC_VBRSCALE, m_nVbrScale);
	DDV_MinMaxInt(pDX, m_nVbrScale, 0, 100);
	DDX_Check(pDX, IDC_ENCSAMPLERATE, m_bEncChoose);
	//}}AFX_DATA_MAP
	DDX_GetFileControl(pDX, IDC_EXTENC, m_cExtEncPath, 0, g_language.GetString( IDS_EXTENCFILESELECTION ) );
}


void CEncoderDosXingDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	CString strTmp;

	m_cExtEncPath.GetWindowText( strTmp );

	pEncoder->SetUser1( CUString( strTmp ) );

	// Set bitrate selection
	pEncoder->SetBitrate(GetMinBitrate());

	pEncoder->SetOriginal( m_bOriginal );
	pEncoder->SetCopyRight( m_bCopyright );
	pEncoder->SetMode( m_bCbrOrVbr );
	
	pEncoder->SetUserN1(	( m_bDownmixToMono << 0 )	+
							( m_bFilterHighFreq << 1)	+
							( m_bSimpleStereo << 2 )	+
							( m_bEncChoose << 3 )
						);

	pEncoder->SetUserN2( m_nVbrScale );

}

void CEncoderDosXingDlg::SetControls(CEncoder* pEncoder)
{
    CUStringConvert strCnv;
	
    // Set control items based in pEncoder information
	m_cExtEncPath.SetWindowText(  strCnv.ToT( pEncoder->GetUser1() ) );

	m_bOriginal = pEncoder->GetOriginal();
	m_bCopyright = pEncoder->GetCopyRight();
	m_bCbrOrVbr = pEncoder->GetMode();
	
	m_bDownmixToMono	=(pEncoder->GetUserN1() >>0 ) & 0x01;
	m_bFilterHighFreq	=(pEncoder->GetUserN1() >>1 ) & 0x01;
	m_bSimpleStereo		=(pEncoder->GetUserN1() >>2 ) & 0x01;
	m_bEncChoose		=(pEncoder->GetUserN1() >>3 ) & 0x01;

	m_nVbrScale=pEncoder->GetUserN2();

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Set the proper bitrate
	SetMinBitrate(pEncoder->GetBitrate());

	// Set data to controls
	UpdateData( FALSE );
}


BEGIN_MESSAGE_MAP(CEncoderDosXingDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderDosXingDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderDosXingDlg message handlers

BOOL CEncoderDosXingDlg::OnInitDialog() 
{

	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;
}

UINT CEncoderDosXingDlg::GetIDD()
{
	return IDD;
}

