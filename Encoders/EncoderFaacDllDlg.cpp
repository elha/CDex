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
#include "EncoderFaacDllDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int faacBitrates[] = { 32,64,80,96,128 };

CEncoderFaacDllDlg::CEncoderFaacDllDlg() 
	: CEncoderDlg(CEncoderFaacDllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderFaacDllDlg)
	m_bUseMS = FALSE;
	m_bUseLFE = FALSE;
	m_nMpegVersion = -1;
	m_bUseTNS = FALSE;
	//}}AFX_DATA_INIT
}

void CEncoderFaacDllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderFaacDllDlg)
	DDX_Control(pDX, IDC_MPEGVERSION, m_MpegVersion);
	DDX_Control(pDX, IDC_MINBITRATE, m_Bitrate);
	DDX_Control(pDX, IDC_PROFILE, m_Profile);
	DDX_Check(pDX, IDC_USEMS, m_bUseMS);
	DDX_Check(pDX, IDC_USELFE, m_bUseLFE);
	DDX_Radio(pDX, IDC_MPEGVERSION, m_nMpegVersion);
	DDX_Control(pDX, IDC_BANDWIDTH, m_BandWidth);
	DDX_Check(pDX, IDC_ALLOWTNS, m_bUseTNS);
	//}}AFX_DATA_MAP
}

void CEncoderFaacDllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	int nTemp=	( ( m_BandWidth.GetCurSel()  & 0x0F ) << 8 ) +
				( ( m_nMpegVersion           & 0x01 ) << 3 ) + 
				( ( m_bUseLFE                & 0x01 ) << 2 ) + 
				((  m_bUseMS                 & 0x01 ) << 1 ) +
				(   m_bUseTNS                & 0x01 );

	pEncoder->SetUserN1( nTemp );
	pEncoder->SetUserN2( m_Profile.GetCurSel() );
	pEncoder->SetBitrate( faacBitrates[ m_Bitrate.GetCurSel() ] );

	

}

void CEncoderFaacDllDlg::SetControls(CEncoder* pEncoder)
{
	m_Bitrate.SetCurSel(0);

	for (int i=0;i<sizeof(faacBitrates)/sizeof(faacBitrates[0]);i++)
	{
		if (pEncoder->GetBitrate()==faacBitrates[i])
		{
			m_Bitrate.SetCurSel(i);
		}
	}

	m_bUseTNS   	= ( pEncoder->GetUserN1() & 0x01 ) ? TRUE : FALSE;
	m_bUseMS		= ( pEncoder->GetUserN1() & 0x02 ) ? TRUE : FALSE;
	m_bUseLFE		= ( pEncoder->GetUserN1() & 0x04 ) ? TRUE : FALSE;
	m_nMpegVersion  = ( pEncoder->GetUserN1() & 0x08 ) ? 1 : 0;

	if ( m_BandWidth )
	{
		m_BandWidth.SetCurSel( ( pEncoder->GetUserN1() >>8 ) & 0x0F );
	}

	if ( m_Profile.m_hWnd )
	{
		m_Profile.SetCurSel( pEncoder->GetUserN2() );
	}

	// Set data to controls
	UpdateData(FALSE);
}


BEGIN_MESSAGE_MAP(CEncoderFaacDllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderFaacDllDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderFaacDllDlg message handlers

BOOL CEncoderFaacDllDlg::OnInitDialog() 
{

	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	CUString strTmp;
	for (int i=0;i<sizeof(faacBitrates)/sizeof(faacBitrates[0]);i++)
	{
		strTmp.Format( _W( "%d kBits/s" ), faacBitrates[ i ] );
		CUStringConvert strCnv;
        m_Bitrate.AddString( strCnv.ToT( strTmp ) );
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

UINT CEncoderFaacDllDlg::GetIDD()
{
	return IDD;
}
