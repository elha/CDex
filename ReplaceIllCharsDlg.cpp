/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 Albert L. Faber
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
#include "ReplaceIllCharsDlg.h"
#include "config.h"
#include "FileName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const int nNumReplaceChars = GetNumReplaceCharacters();

/////////////////////////////////////////////////////////////////////////////
// CReplaceIllCharsDlg dialog


CReplaceIllCharsDlg::CReplaceIllCharsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CReplaceIllCharsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CReplaceIllCharsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CReplaceIllCharsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplaceIllCharsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReplaceIllCharsDlg, CDialog)
	//{{AFX_MSG_MAP(CReplaceIllCharsDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplaceIllCharsDlg message handlers

BOOL CReplaceIllCharsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// get string replacement character string
	CUString strReplace=g_config.GetRILC();

	// Fill in the dialog box's Edit controls 
	for (int i=0; i < nNumReplaceChars; i++ )
	{
		int nChar;

        CUStringConvert strCnv;

        int nRes = _stscanf( strCnv.ToT( strReplace.Mid( i * 3, 2 ) ),_T( "%2X" ), &nChar );

		if ( 0 == nRes || EOF == nRes )
		{
			GetDlgItem( IDC_RPLC_1 + i )->SetWindowText( _T( " " ) );
		}		
		else
		{
			GetDlgItem( IDC_RPLC_1 + i )->SetWindowText( CString( (CHAR)( nChar&0xFF ) ) );
		}
	}

	return TRUE;
}

void CReplaceIllCharsDlg::OnOK() 
{
	// get string replacement character string
	CUString strReplace;

	// Get the Control items, and create the replacement string
	for ( int i=0; i < nNumReplaceChars; i++ )
	{
		CString strTmp;
		GetDlgItem( IDC_RPLC_1 + i )->GetWindowText( strTmp );
		CHAR nChar = '\0';


		if ( !strTmp.IsEmpty() )
		{
			// get the last entered character
			nChar = (CHAR)strTmp.GetAt( strTmp.GetLength() - 1 );
		}

		strTmp.Format( _T( "%02X " ), (BYTE)(nChar & 0xFF ) );

		strReplace += CUString( (LPCTSTR)strTmp );
	}

	// Set the string into the config file
	g_config.SetRILC( strReplace );

	CDialog::OnOK();
}
