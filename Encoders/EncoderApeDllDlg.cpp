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
#include "EncoderApeDllDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEncApeOpts dialog


CEncApeOpts::CEncApeOpts(CWnd* pParent /*=NULL*/)
	: CEncoderDlg(CEncApeOpts::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEncApeOpts)
	m_nMode = -1;
	//}}AFX_DATA_INIT
}


void CEncApeOpts::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncApeOpts)
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nMode);
	DDX_Control(pDX, IDC_APECOMPRESSIONLEVEL, m_CompressionLevel);
	DDX_Control(pDX, IDC_APETAGFORMAT, m_TagFormat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEncApeOpts, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncApeOpts)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


UINT CEncApeOpts::GetIDD()
{
	return IDD;
}

void CEncApeOpts::GetControls(CEncoder* pEncoder)
{
	UpdateData( TRUE );

	pEncoder->SetMode( m_nMode );

	pEncoder->SetUserN1( m_CompressionLevel.GetCurSel() );

	pEncoder->SetUserN2( m_TagFormat.GetCurSel() );

}

void CEncApeOpts::SetControls(CEncoder* pEncoder)
{
	m_nMode = pEncoder->GetMode();

	m_CompressionLevel.SetCurSel( LOBYTE( LOWORD( pEncoder->GetUserN1() ) ) );

	m_TagFormat.SetCurSel( LOBYTE( LOWORD( pEncoder->GetUserN2() ) ) );

	// Set data to controls
	UpdateData(FALSE);

}

/////////////////////////////////////////////////////////////////////////////
// CEncApeOpts message handlers

BOOL CEncApeOpts::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
