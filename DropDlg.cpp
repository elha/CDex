/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
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


// DropDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cdex.h"
#include "DropDlg.h"
#include "config.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDropDlg dialog


CDropDlg::CDropDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDropDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDropDlg)
	//}}AFX_DATA_INIT
	m_bDeleteWAV = g_config.GetDeleteWAV();
	m_bRiffWAV = g_config.GetRiffWav();
	m_bNormalize=FALSE;
	m_bEncode=FALSE;
}


void CDropDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDropDlg)
	DDX_Check(pDX, IDC_DELETEWAV, m_bDeleteWAV);
	DDX_Check(pDX, IDC_RIFFWAV, m_bRiffWAV);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDropDlg, CDialog)
	//{{AFX_MSG_MAP(CDropDlg)
	ON_BN_CLICKED(IDC_ENCODE, OnEncode)
	ON_BN_CLICKED(IDC_NORMALIZE, OnNormalize)
	ON_BN_CLICKED(IDC_NORMANDENC, OnNormandenc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDropDlg message handlers

void CDropDlg::OnEncode() 
{
	UpdateData(TRUE);
	g_config.SetRiffWav( m_bRiffWAV );
	g_config.SetDeleteWAV(m_bDeleteWAV);
	m_bEncode=TRUE;
	CDialog::OnOK();
}

void CDropDlg::OnNormalize() 
{
	UpdateData(TRUE);
	g_config.SetRiffWav( FALSE );
	g_config.SetDeleteWAV(FALSE);
	m_bNormalize=TRUE;
	CDialog::OnOK();
	
}

void CDropDlg::OnNormandenc() 
{
	UpdateData(TRUE);
	g_config.SetRiffWav( m_bRiffWAV );
	g_config.SetDeleteWAV(m_bDeleteWAV);
	m_bNormalize=TRUE;
	m_bEncode=TRUE;
	CDialog::OnOK();
}

void CDropDlg::OnCancel() 
{
	CDialog::OnCancel();
}

BOOL CDropDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// translate dialog controls
	g_language.InitDialogStrings( this, IDD_DROPASK );

	return TRUE;
}
