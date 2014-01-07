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
#include "EncoderNttVqfDllDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int nttVqfBitrates[] = { 24, 32, 48 };


CEncoderNttVqfDllDlg::CEncoderNttVqfDllDlg() 
	: CEncoderDlg(CEncoderNttVqfDllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderNttVqfDllDlg)
	m_nMode = -1;
	m_bEncDuringRead = FALSE;
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
}


void CEncoderNttVqfDllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderNttVqfDllDlg)
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nMode);
	DDX_Control(pDX, IDC_MINBITRATE, m_MinBitrate);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEncoderNttVqfDllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderNttVqfDllDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderNttVqfDllDlg message handlers

BOOL CEncoderNttVqfDllDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEncoderNttVqfDllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	pEncoder->SetOnTheFlyEncoding( m_bEncDuringRead );
	pEncoder->SetMode( m_nMode );
	pEncoder->SetVersion( m_nVersion );
	pEncoder->SetUserN1( 0 );

	// Set bitrate selection
	pEncoder->SetBitrate( GetMinBitrate() );
}

void CEncoderNttVqfDllDlg::SetControls(CEncoder* pEncoder)
{
	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();
	m_nMode=pEncoder->GetMode();
	m_nVersion=pEncoder->GetVersion();

	int nTemp=pEncoder->GetUserN1();

	// Fill Min bitrate table
	FillMinBitrateTable();

	// Set the proper bitrate
	SetMinBitrate(pEncoder->GetBitrate());

	// Set data to controls
	UpdateData(FALSE);
}

void CEncoderNttVqfDllDlg::SetMinBitrate(int nBitrate)
{
	int nItems=sizeof(nttVqfBitrates)/sizeof(nttVqfBitrates[0] );
	
	int i;

	for (i=0;i<nItems;i++)
	{
		if (nBitrate==nttVqfBitrates[i] )
		{
			m_MinBitrate.SetCurSel( i );
			return;
		}
	}

	// default return if not found
	m_MinBitrate.SetCurSel( nItems - 1 );
}

void CEncoderNttVqfDllDlg::FillMinBitrateTable()
{
	// Depending on the settings, fill the bit-rate tables
	m_MinBitrate.ResetContent();

	int nItems=sizeof(nttVqfBitrates)/sizeof(nttVqfBitrates[0] );
	
	int i;

	for ( i = 0; i < nItems; i++ )
	{
		CUString strItem;
		strItem.Format( _W( "%d kbps" ), nttVqfBitrates[ i ] );
        CUStringConvert strCnv;
        m_MinBitrate.AddString( strCnv.ToT( strItem ) );
	}
}

int CEncoderNttVqfDllDlg::GetMinBitrate()
{
	int nItems=sizeof(nttVqfBitrates)/sizeof(nttVqfBitrates[0] );
	return nttVqfBitrates[ m_MinBitrate.GetCurSel() ];
}

UINT CEncoderNttVqfDllDlg::GetIDD()
{
	return IDD;
}
