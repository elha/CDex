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
#include "FileExistWarning.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileExistWarning dialog


CFileExistWarning::CFileExistWarning(CWnd* pParent /*=NULL*/)
	: CDialog(CFileExistWarning::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileExistWarning)
	m_strFileName = _T("");
	//}}AFX_DATA_INIT
}


void CFileExistWarning::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileExistWarning)
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileExistWarning, CDialog)
	//{{AFX_MSG_MAP(CFileExistWarning)
	ON_BN_CLICKED(IDC_TOALL, OnToall)
	ON_BN_CLICKED(IDC_NO, OnNo)
	ON_BN_CLICKED(IDC_YES, OnYes)
	ON_BN_CLICKED(IDC_MYNO, OnMyno)
	ON_BN_CLICKED(IDC_NOTOALL, OnNotoall)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileExistWarning message handlers

void CFileExistWarning::OnToall() 
{
	m_bYesToAll = TRUE;
	CDialog::OnOK();
	
}

void CFileExistWarning::OnCancel() 
{
	CDialog::OnCancel();
}

void CFileExistWarning::OnOK() 
{
	CDialog::OnOK();
}

BOOL CFileExistWarning::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	if ( m_strFileName.GetLength() > 55 )
	{
		int nPos = m_strFileName.Find( _T( ":" ) );

		if ( nPos < 0 )
			nPos = 0;

		m_strFileName = m_strFileName.Left( nPos + 2 ) +
						_T( " ... " ) +
						m_strFileName.Right( 50 ) ;

	}

	UpdateData( FALSE );

	m_bYesToAll = FALSE;
	m_bNoToAll  = FALSE;
	
	return TRUE;
}

void CFileExistWarning::OnNo() 
{
	OnCancel();	
}

void CFileExistWarning::OnYes() 
{
	OnOK();	
}

void CFileExistWarning::OnMyno() 
{
	CDialog::OnCancel();

}

void CFileExistWarning::OnNotoall() 
{
	m_bNoToAll = TRUE;
	CDialog::OnCancel();
}
