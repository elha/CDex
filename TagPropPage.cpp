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
#include "TagPropPage.h"
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
// CTagPropPage property page

IMPLEMENT_DYNCREATE(CTagPropPage, CPropertyPage)

CTagPropPage::CTagPropPage()
	: CPropertyPage( CTagPropPage::IDD )
{
    static CUStringConvert strCnv;

	m_strTitle  = g_language.GetString( IDS_TABPROPPAGE );
	m_psp.pszTitle = strCnv.ToT( m_strTitle );
	m_psp.dwFlags |= PSP_USETITLE;

    
	m_strComment =  g_config.GetID3Comment();
	m_strEncodedBy =  g_config.GetID3EncodedBy();
	m_nID3V2TrkNrType = g_config.GetID3V2TrackNumber();

	//{{AFX_DATA_INIT(CTagPropPage)
	//}}AFX_DATA_INIT
}

CTagPropPage::~CTagPropPage()
{
}

void CTagPropPage::DoDataExchange(CDataExchange* pDX)
{
    CUString strLang;

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTagPropPage)
    DDX_Text(pDX, IDC_ID3COMMENT, m_strComment);
    DDX_Text(pDX, IDC_ID3V2ENCODEDBY, m_strEncodedBy); // Hydra
    DDX_CBIndex(pDX, IDC_TRACKNUMBERSETTING, m_nID3V2TrkNrType);
    //}}AFX_DATA_MAP
    // Set Folder selection 
    DDX_Control(pDX, IDC_ID3V1, m_cbID3V1);
    DDX_Control(pDX, IDC_ID3V23, m_cbID3V23);
    DDX_Control(pDX, IDC_ID3V24, m_cbID3V24);
    DDX_GetFileControl(pDX, IDC_TAGOPTSPICTURE, m_tagPicture, 0, g_language.GetString( IDS_TAGPICTURECFILESELECTION ) );

    DDX_Control(pDX, IDC_ID3V2ENCODEDBY3, m_pictureComment);
    DDX_Control(pDX, IDC_ADDPICTURE, m_addPicture);
}



BOOL CTagPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

    int version = g_config.GetID3Version();

    m_cbID3V1.SetCheck( version & 0x01  );
    m_cbID3V23.SetCheck( version & 0x02 );
    m_cbID3V24.SetCheck( version & 0x04 );

    CUStringConvert strCnv;
    m_pictureComment.SetWindowText( strCnv.ToT( g_config.GetTagPictureComment( ) ) );
    m_tagPicture.SetWindowText( strCnv.ToT( g_config.GetTagPictureFile() ) );
    
    m_addPicture.SetCheck( g_config.GetAddTagPicture() == 1 );

	UpdateControls();

	return TRUE;
}

void CTagPropPage::UpdateControls()
{
	CString strTmp;
	g_config.SetID3EncodedBy( CUString( m_strEncodedBy ) );

    int version =   m_cbID3V1.GetCheck() +
                    (m_cbID3V23.GetCheck() << 1) +
                    (m_cbID3V24.GetCheck() << 2);

	g_config.SetID3Version( version );
	g_config.SetID3Comment( CUString( m_strComment ) );
	g_config.SetID3V2TrackNumber( CConfig::ID3TRKNRTTYPE( m_nID3V2TrkNrType ) );

    TCHAR lpszTmpString[ MAX_PATH * 2 ] = { _T( '\0' ),};

    m_pictureComment.GetWindowText( lpszTmpString, sizeof( lpszTmpString ) );
    g_config.SetTagPictureComment( CUString( lpszTmpString, CP_ACP ) );

    m_tagPicture.GetWindowText( lpszTmpString, sizeof( lpszTmpString ) );
    g_config.SetTagPictureFile( CUString( lpszTmpString, CP_ACP ) );
    
    g_config.SetAddTagPicture( ( m_addPicture.GetCheck() > 0 ) );

}


BOOL CTagPropPage::OnApply()
{
	// Call parent to update all property sheets
	((ConfigSheet*)GetParent())->OnApply();

	// Do default stuff
	return CPropertyPage::OnApply();
}

void CTagPropPage::OnChange() 
{
  SetModified(TRUE);
}


BEGIN_MESSAGE_MAP(CTagPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTagPropPage)
	ON_BN_CLICKED(IDC_NORMTRACK, OnChange)
	ON_EN_CHANGE(IDC_LNORMLEVEL, OnChange)
	ON_EN_CHANGE(IDC_NORMVALUE, OnChange)
	ON_EN_CHANGE(IDC_UNORMLEVEL, OnChange)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_ID3V23, OnBnClickedId3v23)
    ON_BN_CLICKED(IDC_ID3V24, OnBnClickedId3v24)
END_MESSAGE_MAP()

void CTagPropPage::OnBnClickedId3v23()
{
    m_cbID3V24.SetCheck( FALSE );
}

void CTagPropPage::OnBnClickedId3v24()
{
    m_cbID3V23.SetCheck( FALSE );
}
