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


#include "stdafx.h"
#include "cdex.h"
#include "EncoderVorbisDllDlg.h"
#include "Encode.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int oggBitrates[] = { 0,32,40,48,64,80,96,128,160,196,256,350};


static int QualityNominalBitrateRelation[][2] = {
	{ -1, 45 },
 	{  0, 64 },
 	{  1, 80 },
 	{  2, 96 },
 	{  3, 112 },
 	{  4, 128 },
 	{  5, 160 },
 	{  6, 192 },
 	{  7, 224 },
 	{  8, 256 },
 	{  9, 320 },
 	{  9, 500 }
};
 


CEncoderVorbidDllDlg::CEncoderVorbidDllDlg() 
	: CEncoderDlg(CEncoderVorbidDllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderVorbidDllDlg)
	m_nMode = -1;
	m_nChannels = -1;
	m_bUseQualitySetting = FALSE;
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
}


void CEncoderVorbidDllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderVorbidDllDlg)
	DDX_Control(pDX, IDC_QUALITYSETTINGSCROLL, m_QualityScroll);
	DDX_Control(pDX, IDC_MINBITRATE, m_MinBitrate);
	DDX_Control(pDX, IDC_BITRATE, m_Bitrate);
	DDX_Control(pDX, IDC_MAXBITRATE, m_MaxBitrate);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nChannels);
	DDX_Check(pDX, IDC_USEQUALITYSETTING, m_bUseQualitySetting);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEncoderVorbidDllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderVorbidDllDlg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_USEQUALITYSETTING, OnUsequalitysetting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderVorbidDllDlg message handlers

BOOL CEncoderVorbidDllDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// setup scroll range, 0.01 precission CScrollBar
	m_QualityScroll.SetScrollRange( -100, 1000 );

	return TRUE;
}


void CEncoderVorbidDllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	pEncoder->SetOnTheFlyEncoding(m_bEncDuringRead);

	int nMode = ( m_nChannels  == 1 ) ? BE_MP3_MODE_MONO : BE_MP3_MODE_STEREO;

	pEncoder->SetMode( nMode );

	// Get bitrate selection
	pEncoder->SetMinBitrate( oggBitrates[ m_MinBitrate.GetCurSel() ] );
	pEncoder->SetBitrate( oggBitrates[ m_Bitrate.GetCurSel() ] );
	pEncoder->SetMaxBitrate( oggBitrates[ m_MaxBitrate.GetCurSel() ] );

	pEncoder->SetUserN1( MAKELPARAM( m_QualityScroll.GetScrollPos(), m_bUseQualitySetting ) );

}



void CEncoderVorbidDllDlg::FillBitrateTables()
{
	CUString strLang;

	// Depending on the settings, fill the bit-rate tables
	m_MinBitrate.ResetContent();

	int nItems = sizeof( oggBitrates ) / sizeof( oggBitrates[ 0 ] );

	strLang = g_language.GetString( IDS_DEFAULT );

    CUStringConvert strCnv;

	m_Bitrate.AddString( strCnv.ToT( strLang ) );
	m_MaxBitrate.AddString( strCnv.ToT( strLang ) );
	m_MinBitrate.AddString( strCnv.ToT( strLang ) );

	// DO NOT DISPLAY ITEM 0
	for (int i=1;i<nItems;i++)
	{
		CUString strItem;
        CUStringConvert strCnv;
		
        strItem.Format( _W( "%d kbps" ), oggBitrates[ i ] );

		m_MinBitrate.AddString( strCnv.ToT( strItem ) );
		m_Bitrate.AddString( strCnv.ToT( strItem ) );
		m_MaxBitrate.AddString( strCnv.ToT( strItem ) );
	}
}

void CEncoderVorbidDllDlg::SetBitrates(int min_br,int nom_br, int max_br)
{
	int nItems=sizeof(oggBitrates)/sizeof(oggBitrates[0]);
	
	m_MinBitrate.SetCurSel(0);
	m_Bitrate.SetCurSel(0);
	m_MaxBitrate.SetCurSel(0);

	for (int i=1;i<nItems;i++)
	{
		if (min_br==oggBitrates[i])
		{
			m_MinBitrate.SetCurSel(i);
		}
		if (nom_br==oggBitrates[i])
		{
			m_Bitrate.SetCurSel(i);
		}
		if (max_br==oggBitrates[i])
		{
			m_MaxBitrate.SetCurSel(i);
		}
	}

}


void CEncoderVorbidDllDlg::SetControls(CEncoder* pEncoder)
{
	INT nMode = pEncoder->GetMode();

	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();

	m_nChannels = ( ( nMode & 0xFFFF ) == BE_MP3_MODE_MONO )? 1 : 0;

	// Fill Min bitrate table
	FillBitrateTables();

	// Set the proper bitrate
	SetBitrates(pEncoder->GetMinBitrate(),pEncoder->GetBitrate(),pEncoder->GetMaxBitrate());

	m_QualityScroll.SetScrollPos( (SHORT)(LOWORD( pEncoder->GetUserN1() ) ) );

	m_bUseQualitySetting = HIWORD( pEncoder->GetUserN1() );

	m_MinBitrate.EnableWindow( m_bUseQualitySetting == 0 );
	m_Bitrate.EnableWindow( m_bUseQualitySetting == 0 );
	m_MaxBitrate.EnableWindow( m_bUseQualitySetting == 0 );

	m_QualityScroll.EnableWindow( m_bUseQualitySetting );
	m_QualityScroll.EnableScrollBar( m_bUseQualitySetting ? ESB_ENABLE_BOTH : ESB_DISABLE_RTDN );

	UpdateQualityControlValue();

	// Set data to controls
	UpdateData(FALSE);
}

void CEncoderVorbidDllDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// Get data from controls
	UpdateData( TRUE );

	CEncoderDlg::OnHScroll( nSBCode, nPos, pScrollBar );

	switch ( nSBCode )
	{
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			pScrollBar->SetScrollPos( nPos );
		break;
		case SB_LINELEFT:
			pScrollBar->SetScrollPos( m_QualityScroll.GetScrollPos() - 1 );
		break;
		case SB_LINERIGHT:
			pScrollBar->SetScrollPos( m_QualityScroll.GetScrollPos() + 1 );
		break;
	}

	UpdateQualityControlValue();

	UpdateData( FALSE );

}

void CEncoderVorbidDllDlg::UpdateQualityControlValue()
{
	DOUBLE dQuality = (DOUBLE)m_QualityScroll.GetScrollPos() / 100.0;
	DOUBLE dEstBitrate = 0.0;
	DOUBLE dMultA = 0.0;
	DOUBLE dMultB = 0.0;
	INT		nQualityA = 0;
	INT		nQualityB = 0;
	
	nQualityA = (INT)floor( dQuality );
	nQualityB = nQualityA + 1;

	if ( nQualityB > 10 )
	{
		nQualityB = 10;
	}

	if ( dQuality > 0 )
	{
		dMultB = dQuality - nQualityA;
	}
	else
	{
		dMultB = dQuality - nQualityA;
	}

	dMultA = ( 1.0 - dMultB );

	dEstBitrate =	dMultA * (DOUBLE)QualityNominalBitrateRelation[ nQualityA + 1 ][1] + 
					dMultB * (DOUBLE)QualityNominalBitrateRelation[ nQualityB + 1 ][1];

	// update info
    CString strQuality;
    strQuality.Format( _T( "%5.2f (~%3d kbps)" ), dQuality, (int)( dEstBitrate + 0.5 ) );
    GetDlgItem( IDC_QUALITYSETTING )->SetWindowText( strQuality );
}


void CEncoderVorbidDllDlg::OnUsequalitysetting() 
{
	// Get data from controls
	UpdateData( TRUE );

	m_MinBitrate.EnableWindow( m_bUseQualitySetting == 0 );
	m_Bitrate.EnableWindow( m_bUseQualitySetting == 0 );
	m_MaxBitrate.EnableWindow( m_bUseQualitySetting == 0 );

	m_QualityScroll.EnableWindow( m_bUseQualitySetting );
	m_QualityScroll.EnableScrollBar( m_bUseQualitySetting ? ESB_ENABLE_BOTH : ESB_DISABLE_RTDN );

	// Set data to controls
	UpdateData(FALSE);

}

UINT CEncoderVorbidDllDlg::GetIDD()
{
	return IDD;
}

