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


#include "StdAfx.h"
#include "LocalCDDBPropPage.h"
#include "config.h"
#include "ConfigDlg.h"
#include "AutoDetect.h"
#include "CDex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CLocalCDDBPropPage property page

IMPLEMENT_DYNCREATE(CLocalCDDBPropPage, CPropertyPage)

CLocalCDDBPropPage::CLocalCDDBPropPage() :
	CPropertyPage( CLocalCDDBPropPage::IDD )
{
    static CUStringConvert strCnv; 

	m_strTitle  = g_language.GetString( IDS_LOCALCDDBPROPPAGE );
	m_psp.pszTitle = strCnv.ToT( m_strTitle );
	m_psp.dwFlags |= PSP_USETITLE;

	//{{AFX_DATA_INIT(CLocalCDDBPropPage)
	//}}AFX_DATA_INIT
	m_nCDDBType =g_config.GetLocalCDDBType();
	m_bStIni = g_config.GetSaveToCDPlayer();
	m_nStLocCDDB = g_config.GetSaveToLocCDDB();
	m_bLongDirNames=g_config.GetCDDBLongDirNames();
	m_bWriteAsDosFile = g_config.GetCDDBWriteAsDosFile( );
	// Get Long or short CDDB directory names
}

CLocalCDDBPropPage::~CLocalCDDBPropPage()
{
}

void CLocalCDDBPropPage::DoDataExchange(CDataExchange* pDX)
{
	CUString strLang;

	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocalCDDBPropPage)
	DDX_Radio(pDX, IDC_CDDBTYPE, m_nCDDBType);
	DDX_Check(pDX, IDC_STCDINI, m_bStIni);
	DDX_Check(pDX, IDC_STLOCALCDDB, m_nStLocCDDB);
	DDX_Check(pDX, IDC_LONGDIRNAMES, m_bLongDirNames);
	DDX_Check(pDX, IDC_DOSFILES, m_bWriteAsDosFile);
	//}}AFX_DATA_MAP

	// Set Folder selection 
	strLang = g_language.GetString( IDS_PLEASE_SELECTFOLDER );
    CUStringConvert strCnv;
	DDX_GetFolderControl(pDX, IDC_CDDBPATH, m_CDDBPath, 0, strCnv.ToT( strLang ) );
	DDX_GetFolderControl(pDX, IDC_WINAMPDBPATH, m_WinampDB, 0, strCnv.ToT( strLang ) );
}


BEGIN_MESSAGE_MAP(CLocalCDDBPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CLocalCDDBPropPage)
	ON_EN_CHANGE(IDC_CDDBPATH, OnChange)
	ON_BN_CLICKED(IDC_CDDBTYPE, OnChange)
	ON_BN_CLICKED(IDC_STCDINI, OnChange)
	ON_BN_CLICKED(IDC_STLOCALCDDB, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CLocalCDDBPropPage::UpdateControls()
{
	UpdateData(TRUE);

	// Set CDDB directory
	CString strDir;

	m_CDDBPath.GetWindowText( strDir );

	// And actually set it now
	g_config.SetCDDBPath( CUString( strDir ));

	m_WinampDB.GetWindowText( strDir );

	// And actually set it now
	g_config.SetWinampDBPath( CUString( strDir ) );

	// Get CDDB Type
	g_config.SetLocalCDDBType( m_nCDDBType );

	// Store to CDPlayer.ini
	g_config.SetSaveToCDPlayer(m_bStIni );

	// Store to Local CDDB
	g_config.SetSaveToLocCDDB( m_nStLocCDDB );

	// Get Long or short CDDB directory names
	g_config.SetCDDBLongDirNames( m_bLongDirNames );

	g_config.SetCDDBWriteAsDosFile( m_bWriteAsDosFile );
}


void CLocalCDDBPropPage::OnChange() 
{
	SetModified(TRUE);
	UpdateCtrls();
}

BOOL CLocalCDDBPropPage::OnApply()
{
	// Call parent to update all property sheets
	((ConfigSheet*)GetParent())->OnApply();

	// Do default stuff
	return CPropertyPage::OnApply();
}


void CLocalCDDBPropPage::UpdateCtrls()
{
	// Get data out of controls
	UpdateData(TRUE);

	// And update controls again
	UpdateData(FALSE);
}

BOOL CLocalCDDBPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

    CUStringConvert strCnv;

	m_CDDBPath.SetWindowText( strCnv.ToT( g_config.GetCDDBPath() ));

	m_WinampDB.SetWindowText( strCnv.ToT( g_config.GetWinampDBPath() ) );

	// Do update the data controls
	UpdateCtrls();

	return TRUE;
}

