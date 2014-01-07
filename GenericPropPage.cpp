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


#include "StdAfx.h"
#include "GenericPropPage.h"
#include "config.h"
#include "ConfigDlg.h"
#include "CDex.h"
#include ".\genericproppage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGenericPropPage property page

IMPLEMENT_DYNCREATE(CGenericPropPage, CPropertyPage)

CGenericPropPage::CGenericPropPage()
	: CPropertyPage( CGenericPropPage::IDD )
{
    static CUStringConvert strCnv;

	m_strTitle  = g_language.GetString( IDS_GENERICPROPPAGE );
	m_psp.pszTitle = strCnv.ToT( m_strTitle );
	m_psp.dwFlags |= PSP_USETITLE;


	m_nLNormFactor= g_config.GetLNormFactor();
	m_nHNormFactor= g_config.GetHNormFactor();
	m_bNormTrack  = g_config.GetNormTrack();
	m_nLNormLevel = g_config.GetLowNormLevel();
	m_nUNormLevel = g_config.GetHighNormLevel();
    
	m_bAutoShutDown = g_config.GetAutoShutDown();
	m_bDigitalCDPlay = g_config.GetCDPlayDigital();

	//{{AFX_DATA_INIT(CGenericPropPage)
	//}}AFX_DATA_INIT
}

CGenericPropPage::~CGenericPropPage()
{
}

void CGenericPropPage::DoDataExchange(CDataExchange* pDX)
{
    CUString strLang;

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CGenericPropPage)
    DDX_Check(pDX, IDC_NORMTRACK, m_bNormTrack);
    DDX_Text(pDX, IDC_LNORMLEVEL, m_nLNormLevel);
    DDX_Text(pDX, IDC_UNORMLEVEL, m_nUNormLevel);
    DDX_Text(pDX, IDC_LNORMVALUE, m_nLNormFactor);
    DDV_MinMaxInt(pDX, m_nLNormFactor, 1, 100);
    DDX_Text(pDX, IDC_HNORMVALUE, m_nHNormFactor);
    DDV_MinMaxInt(pDX, m_nHNormFactor, 1, 100);
    DDX_Check(pDX, IDC_AUTOSHUTDOWN, m_bAutoShutDown);
    DDX_Check(pDX, IDC_DIGITALCDPLAYBACK, m_bDigitalCDPlay);
    //}}AFX_DATA_MAP
    // Set Folder selection 
    strLang = g_language.GetString( IDS_PLEASE_SELECTFOLDER );
    CUStringConvert strCnv;
    DDX_GetFolderControl(pDX, IDC_TEMPDIR, m_tmpDir, 0, strCnv.ToT( strLang ) );
}



BOOL CGenericPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN1))->SetRange( 1, 100 );
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN2))->SetRange( 0, 100 );
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN3))->SetRange( 0, 100 );
	((CSpinButtonCtrl*)GetDlgItem(IDC_SPIN4))->SetRange( 1, 100 );

    CUStringConvert strCnv;
	m_tmpDir.SetWindowText( strCnv.ToT( g_config.GetTempDir() ) );

	UpdateControls();

	return TRUE;
}

void CGenericPropPage::UpdateControls()
{
	CString strTmp;
	g_config.SetLNormFactor( ( m_nLNormFactor<1 )?1:m_nLNormFactor );
	g_config.SetHNormFactor( ( m_nHNormFactor<1 )?1:m_nHNormFactor );
	g_config.SetNormTrack( m_bNormTrack );
	g_config.SetLowNormLevel( m_nLNormLevel );
	g_config.SetHighNormLevel( m_nUNormLevel );

	m_tmpDir.GetWindowText( strTmp );

	if ( !strTmp.IsEmpty() )
	{
		// Add trailing back space
		if ( strTmp[ strTmp.GetLength()-1 ] != _T( '\\' ) )
		{
			strTmp +=  _T( "\\" );
		}
	}

	g_config.SetTempDir( CUString( strTmp ) );
	g_config.SetAutoShutDown( m_bAutoShutDown );
	g_config.SetCDPlayDigital( m_bDigitalCDPlay );

}


BOOL CGenericPropPage::OnApply()
{
	// Call parent to update all property sheets
	((ConfigSheet*)GetParent())->OnApply();

	// Do default stuff
	return CPropertyPage::OnApply();
}

void CGenericPropPage::OnChange() 
{
  SetModified(TRUE);
}


BEGIN_MESSAGE_MAP(CGenericPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CGenericPropPage)
	ON_BN_CLICKED(IDC_NORMTRACK, OnChange)
	ON_EN_CHANGE(IDC_LNORMLEVEL, OnChange)
	ON_EN_CHANGE(IDC_NORMVALUE, OnChange)
	ON_EN_CHANGE(IDC_UNORMLEVEL, OnChange)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ID3V23, OnBnClickedId3v23)
    ON_BN_CLICKED(IDC_ID3V24, OnBnClickedId3v24)
END_MESSAGE_MAP()

void CGenericPropPage::OnBnClickedId3v23()
{
    m_cbID3V24.SetCheck( FALSE );
}

void CGenericPropPage::OnBnClickedId3v24()
{
    m_cbID3V23.SetCheck( FALSE );
}
