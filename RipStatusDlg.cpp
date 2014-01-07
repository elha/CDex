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
#include "RipStatusDlg.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRipStatusDlg dialog


CRipStatusDlg::CRipStatusDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRipStatusDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRipStatusDlg)
	m_strRipStatus = _T("");
	//}}AFX_DATA_INIT
}


void CRipStatusDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CRipStatusDlg)
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_RIPSTATUS, m_ripStatus);
}


BEGIN_MESSAGE_MAP(CRipStatusDlg, CDialog)
	//{{AFX_MSG_MAP(CRipStatusDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRipStatusDlg message handlers

BOOL CRipStatusDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );
	
    CUStringConvert strCnv;
    m_ripStatus.SetWindowText( strCnv.ToT( m_strRipStatus ) );
	return TRUE;
}
