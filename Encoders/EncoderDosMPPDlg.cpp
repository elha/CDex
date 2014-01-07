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
#include "EncoderDosMPPDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEncoderDosMPPDlg::CEncoderDosMPPDlg() 
	: CEncoderDlg(CEncoderDosMPPDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderDosMPPDlg)
	m_strExtEncOpts = "";
	m_bHideDosBox = FALSE;
	m_bEncDuringRead = FALSE;
	//}}AFX_DATA_INIT
}

void CEncoderDosMPPDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderDosMPPDlg)
	DDX_Text(pDX, IDC_EXTENCOPTS, m_strExtEncOpts);
	DDX_Check(pDX, IDC_HIDE_DOSBOX, m_bHideDosBox);
	DDX_Check(pDX, IDC_ON_THE_FLY, m_bEncDuringRead);
	//}}AFX_DATA_MAP
	DDX_GetFileControl(pDX, IDC_EXTENC, m_cExtEncPath, 0, g_language.GetString( IDS_EXTENCFILESELECTION ) );
}


void CEncoderDosMPPDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	CString strPath;
	m_cExtEncPath.GetWindowText( strPath );
	pEncoder->SetUser1( CUString( strPath ) );
	pEncoder->SetUser2( CUString( m_strExtEncOpts ) );
	pEncoder->SetUserN1( m_bHideDosBox );
	pEncoder->SetOnTheFlyEncoding( m_bEncDuringRead );
	pEncoder->SetChunkSupport( m_bEncDuringRead );
}

void CEncoderDosMPPDlg::SetControls(CEncoder* pEncoder)
{
    CUStringConvert strCnv;

	// Set control items based in pEncoder information
	m_cExtEncPath.SetWindowText( strCnv.ToT( pEncoder->GetUser1() ) );
	m_strExtEncOpts=pEncoder->GetUser2();
	m_bHideDosBox = pEncoder->GetUserN1( );
	m_bEncDuringRead = pEncoder->GetOnTheFlyEncoding();

	// Set data to controls
	UpdateData( FALSE );
}


BEGIN_MESSAGE_MAP(CEncoderDosMPPDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderDosMPPDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderDosMPPDlg message handlers

BOOL CEncoderDosMPPDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

UINT CEncoderDosMPPDlg::GetIDD()
{
	return IDD;
}
