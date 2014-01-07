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
#include "EncoderLameDllDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEncoderLameDllDlg dialog

CEncoderLameDllDlg::CEncoderLameDllDlg() 
	: CEncoderDlg(CEncoderLameDllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderLameDllDlg)
	m_bEncDuringRead = FALSE;
	m_nMode = -1;
	m_bCopyRight = FALSE;
	m_bCRC = FALSE;
	m_bOriginal = FALSE;
	m_bPrivate = FALSE;
	m_nABR = 8;
	//}}AFX_DATA_INIT
}


void CEncoderLameDllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderLameDllDlg)
	DDX_Control(pDX, IDC_OUTSAMPLERATE, m_OutSampleRate);
	DDX_Control(pDX, IDC_VBRMETHOD, m_VbrMethod);
	DDX_Control(pDX, IDC_VBR, m_VBR);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nMode);
	DDX_Check(pDX, IDC_COPYRIGHT, m_bCopyRight);
	DDX_Check(pDX, IDC_CRC, m_bCRC);
	DDX_Check(pDX, IDC_ORIGINAL, m_bOriginal);
	DDX_Check(pDX, IDC_PRIVATE, m_bPrivate);
	DDX_Control(pDX, IDC_QUALITY, m_Quality);
	DDX_Text(pDX, IDC_ABR, m_nABR);
	DDV_MinMaxInt(pDX, m_nABR, 0, 320);
	DDX_Control(pDX, IDC_MINBITRATE, m_MinBitrate);
	DDX_Control(pDX, IDC_MAXBITRATE, m_MaxBitrate);
	DDX_Control(pDX, IDC_MPEGVERSION, m_Version);
	//}}AFX_DATA_MAP

}




BEGIN_MESSAGE_MAP(CEncoderLameDllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderLameDllDlg)
	ON_CBN_SELCHANGE(IDC_QUALITY, OnSelchangePreset )
	ON_CBN_SELCHANGE(IDC_VBR, OnSetControls )
	ON_CBN_SELCHANGE(IDC_VBRMETHOD, OnSetControls )
	ON_CBN_SELCHANGE(IDC_MPEGVERSION, OnVersion )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderLameDllDlg message handlers

BOOL CEncoderLameDllDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEncoderLameDllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData( TRUE );

	// Set items
	pEncoder->SetOnTheFlyEncoding( m_bEncDuringRead );
	pEncoder->SetPrivate( m_bPrivate );
	pEncoder->SetOriginal( m_bOriginal );
	pEncoder->SetCopyRight( m_bCopyRight );
	pEncoder->SetMode( m_nMode );

	pEncoder->SetVersion( m_Version.GetCurSel() );

	// USE CRC TO PACK ADDITIONAL INFO
	// First 4 bits are reserved for mode,
	// 2nd 4 bits for g_Psycho,
	// third nible for Quality
	// fifth nible for VBR Method

	int nVBR = m_VBR.GetCurSel();
	int nVbrMethod = m_VbrMethod.GetCurSel();

	int nCRC=(m_bCRC&0x01)+((m_Quality.GetCurSel()&0x0F)<<8)+((nVBR&0x0F)<<12)+ ((nVbrMethod&0x0F)<<16);
	pEncoder->SetCRC(nCRC);

	// Set bitrate selection
	pEncoder->SetBitrate( GetMinBitrate() );

	// Set MAX bitrate selection
	pEncoder->SetMaxBitrate( GetMaxBitrate() );

	// Set ABR setting
	pEncoder->SetUserN1( m_nABR );

	// Set OutputSampleRate 
	if ( 0 == m_OutSampleRate.GetCurSel() )
	{
		pEncoder->SetOutSampleRate( m_OutSampleRate.GetCurSel() );
	}
	else
	{
		pEncoder->SetOutSampleRate( MP3SampleRates[ m_Version.GetCurSel() ][ m_OutSampleRate.GetCurSel() - 1 ] );
	}
}

void CEncoderLameDllDlg::SetControls(CEncoder* pEncoder)
{
	// Set control items based in pEncoder information
	m_bEncDuringRead = pEncoder->GetOnTheFlyEncoding();
	m_bPrivate = pEncoder->GetPrivate();
	m_bOriginal = pEncoder->GetOriginal();
	m_bCopyRight = pEncoder->GetCopyRight();
	m_nMode = pEncoder->GetMode();

	if ( NULL != m_Version.m_hWnd )
	{
		m_Version.SetCurSel( pEncoder->GetVersion() );
	}

	int nCRC=pEncoder->GetCRC();
	m_bCRC = nCRC & 0x01;

	if ( m_Quality.m_hWnd )
	{
		m_Quality.SetCurSel( ( nCRC >> 8 ) & 0x0F );
	}
	int nVBR = ( ( nCRC >> 12 ) & 0x0F );
	int nVbrMethod= ( ( nCRC >> 16 ) & 0x0F );

	m_VBR.SetCurSel( nVBR );
	m_VbrMethod.SetCurSel( nVbrMethod );

	// Get ABR setting
	m_nABR=LOWORD(pEncoder->GetUserN1());

	m_nVersion = m_Version.GetCurSel( );

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Fill Max bitrate table
	FillMaxBitrateTable();

	// Set the proper bitrate
	SetMinBitrate( pEncoder->GetBitrate() );

	// Set the proper MAX bitrate
	SetMaxBitrate( pEncoder->GetMaxBitrate() );

	FillOutSampleRateCtrl();

	int i = 0;
	m_OutSampleRate.SetCurSel( 0 );

	for ( i=0; i< sizeof( MP3SampleRates[m_Version.GetCurSel()] ) / sizeof( MP3SampleRates[m_Version.GetCurSel()][0] ); i++ )
	{
		if ( MP3SampleRates[m_Version.GetCurSel()][i] == pEncoder->GetOutSampleRate() )
		{
			m_OutSampleRate.SetCurSel( i + 1 );
		}
	}

	// Set data to controls
	UpdateData(FALSE);

	// Make sure the correct items are enabled/disabled
	OnSetControls();

}

void CEncoderLameDllDlg::OnVersion() 
{
	int i = 0;

	UpdateData( TRUE );

	m_nVersion = m_Version.GetCurSel( );

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Fill Max bitrate table
	FillMaxBitrateTable();

	// Fill OutputSample rate control
	FillOutSampleRateCtrl();

	if ( m_Version.GetCurSel() == 0 )
	{
		SetMinBitrate( 128 );
		SetMaxBitrate( 320 );
	}
	else
	{
		SetMinBitrate( 64 );
		SetMaxBitrate( 160 );
	}

	
	m_MaxBitrate.EnableWindow( m_VbrMethod.GetCurSel() > 0 );

}


void CEncoderLameDllDlg::OnSetControls() 
{
	UpdateData(TRUE);

	switch ( m_Quality.GetCurSel() )
	{

		case LQP_R3MIX: /* check for R3MIX setting */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;

		case LQP_STANDARD: /* check for alt-preset standard */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;
		case LQP_FAST_STANDARD: /* check for alt-preset fast standard  */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;

		case LQP_EXTREME: /* check for alt-preset extreme  */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;

		case LQP_FAST_EXTREME: /* check for alt-preset fast extreme */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;
		case LQP_INSANE: /* check for alt-preset fast insane  */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;
		case LQP_ABR: /* check for alt-preset fast insane  */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;
		case LQP_CBR: /* check for alt-preset fast insane  */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( TRUE );
			m_VbrMethod.EnableWindow( FALSE );
		break;
		case LQP_MEDIUM: /* check for alt-preset medium  */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;

		case LQP_FAST_MEDIUM: /* check for alt-preset fast medium */
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( FALSE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( FALSE );
			m_VbrMethod.EnableWindow( FALSE );
		break;

		default:
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 0 )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 1 )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 2 )->EnableWindow( TRUE );
			GetDlgItem( IDC_CHANNELSELECTION + 3 )->EnableWindow( TRUE );
			m_VbrMethod.EnableWindow( TRUE );
	}


	if ( ( m_VbrMethod.GetCurSel() > 0 )  )
	{
		m_MaxBitrate.EnableWindow( TRUE );
		if ( 4 != m_Quality.GetCurSel() )
			m_VBR.EnableWindow( TRUE );
		else
			m_VBR.EnableWindow( FALSE );

	}
	else
	{
		m_MaxBitrate.EnableWindow( FALSE );
		m_VBR.EnableWindow( FALSE );
	}

	if (	( m_VbrMethod.GetCurSel() == 5 )	||
			( LQP_ABR == m_Quality.GetCurSel() ) )
	{
		// Only enable ABR when VBR label is set to ABR
		GetDlgItem( IDC_ABR )->EnableWindow( TRUE );
	}
	else
	{
		GetDlgItem( IDC_ABR )->EnableWindow( FALSE );
	}


	UpdateData( FALSE );
}


void CEncoderLameDllDlg::OnSelchangePreset() 
{
	UpdateData(TRUE);

	switch ( m_Quality.GetCurSel() )
	{

		case LQP_R3MIX: /* check for R3MIX setting */
			m_VbrMethod.SetCurSel( 3 ); // set to VBR MTRH method
			SetMinBitrate( 96 );		// set minimum bit rate
			SetMaxBitrate( 224 );
			m_nMode = 1;				// set joint stereo
			m_Version.SetCurSel( 0 );	// set MPEG 1
			m_VBR.SetCurSel( 1 );		// set VBR Quality to 1
			GetDlgItem( IDC_MPEGVERSION )->EnableWindow( FALSE );
		break;

		case LQP_STANDARD: /* check for alt-preset standard */
			m_VbrMethod.SetCurSel( 2 ); // set to VBR RH method
			SetMinBitrate( 128 );		// set minimum bit rate
			m_nMode = 1;				// set joint stereo
			SetMaxBitrate( 320 );
			m_Version.SetCurSel( 0 );	// set MPEG 1
			m_VBR.SetCurSel( 2 );		// set VBR Quality to 2
		break;
		case LQP_FAST_STANDARD: /* check for alt-preset fast standard  */
			m_VbrMethod.SetCurSel( 3 ); // set to VBR MTRH method
			SetMinBitrate( 128 );		// set minimum bit rate
			SetMaxBitrate( 320 );
			m_nMode = 1;				// set joint stereo
			m_Version.SetCurSel( 0 );	// set MPEG 1
			m_VBR.SetCurSel( 2 );		// set VBR Quality to 4
		break;

		case LQP_EXTREME: /* check for alt-preset extreme  */
			m_VbrMethod.SetCurSel( 2 ); // set to VBR RH method
			SetMinBitrate( 128 );		// set minimum bit rate
			SetMaxBitrate( 320 );
			m_nMode = 1;				// set joint stereo
			m_Version.SetCurSel( 0 );	// set MPEG 1
			m_VBR.SetCurSel( 2 );		// set VBR Quality to 4
		break;

		case LQP_FAST_EXTREME: /* check for alt-preset fast extreme */
			m_VbrMethod.SetCurSel( 3 ); // set to VBR MTRH method
			SetMinBitrate( 128 );		// set minimum bit rate
			SetMaxBitrate( 320 );
			m_nMode = 1;				// set joint stereo
			m_Version.SetCurSel( 0 );	// set MPEG 1
			m_VBR.SetCurSel( 2 );		// set VBR Quality to 4
		break;
		case LQP_INSANE: /* check for alt-preset fast insane  */
			m_VbrMethod.SetCurSel( 0 ); // set to none
			SetMinBitrate( 320 );		// set minimum bit rate
			SetMaxBitrate( 320 );
			m_nMode = 1;				// set joint stereo
			m_Version.SetCurSel( 0 );	// set MPEG 1
		break;
		case LQP_ABR: /* check for alt-preset fast insane  */
			m_VBR.SetCurSel( 2 );		// set VBR Quality to 4
			m_VbrMethod.SetCurSel( VBR_METHOD_ABR + 1 );	// set to VBR ABR
			SetMinBitrate( 64 );		// set minimum bit rate
			SetMaxBitrate( 320 );
			m_nMode = 1;				// set joint stereo
			m_Version.SetCurSel( 0 );	// set MPEG 1
		break;
		case LQP_CBR: /* check for alt-preset fast insane  */
			m_VbrMethod.SetCurSel( 0 ); // set to none
			m_nMode = 1;				// set joint stereo
			m_Version.SetCurSel( 0 );	// set MPEG 1
		break;
	}

	// update controls
	UpdateData( FALSE );

	// enable/disable proper controls
	OnSetControls();
}



UINT CEncoderLameDllDlg::GetIDD()
{
	return IDD;
}
