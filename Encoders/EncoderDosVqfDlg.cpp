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
#include "EncoderDosVqfDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CEncoderDosVqfDlg::CEncoderDosVqfDlg( CWnd* pParent )
	: CEncoderDlg( CEncoderDosVqfDlg::IDD, pParent )
{
	//{{AFX_DATA_INIT(CEncoderDosVqfDlg)
	//}}AFX_DATA_INIT
}


void CEncoderDosVqfDlg::DoDataExchange(CDataExchange* pDX)
{

	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderDosVqfDlg)
	DDX_Control(pDX, IDC_VQFMODE, m_VQFMode);
	//}}AFX_DATA_MAP
	DDX_GetFileControl(pDX, IDC_EXTENC, m_cExtEncPath, 0, g_language.GetString( IDS_EXTENCFILESELECTION ) );

}


BEGIN_MESSAGE_MAP(CEncoderDosVqfDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderDosVqfDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderDosVqfDlg message handlers

BOOL CEncoderDosVqfDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;
}



void CEncoderDosVqfDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	CString strTmp;
	
	m_cExtEncPath.GetWindowText( strTmp );

	pEncoder->SetUser1( CUString( strTmp ) );

	CString strSelected;
	m_VQFMode.GetLBText( m_VQFMode.GetCurSel(), strSelected );
	pEncoder->SetUser2( CUString( strSelected ) );
}

void CEncoderDosVqfDlg::SetControls(CEncoder* pEncoder)
{
    CUStringConvert strCnv;

	// Set control items based in pEncoder information
	m_cExtEncPath.SetWindowText( strCnv.ToT( pEncoder->GetUser1() ) );

	// Get string that matches the saved setting
	int nPos=m_VQFMode.FindString( -1, strCnv.ToT( pEncoder->GetUser2() ) );
	
	// Set selection
	m_VQFMode.SetCurSel( min( 0, nPos ) );

	// Set data to controls
	UpdateData( FALSE );
}

UINT CEncoderDosVqfDlg::GetIDD()
{
	return IDD;
}
