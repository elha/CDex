/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 Albert L. Faber
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
#include "config.h"
#include "WinampInConfig.h"
#include "ISndStreamWinAmp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWinampInConfig dialog


CWinampInConfig::CWinampInConfig(CWnd* pParent /*=NULL*/)
	: CDialog(CWinampInConfig::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWinampInConfig)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CWinampInConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWinampInConfig)
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWinampInConfig, CDialog)
	//{{AFX_MSG_MAP(CWinampInConfig)
	ON_BN_CLICKED(IDC_ABOUT, OnAbout)
	ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinampInConfig message handlers

BOOL CWinampInConfig::OnInitDialog() 
{
	CUString strLang;
	int		i = 0;

	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );


    CUStringConvert strCnv;

	strLang = g_language.GetString( IDS_MODULE_NAME );
    m_List.InsertColumn( 0, strCnv.ToT( strLang ), LVCFMT_LEFT, 100, 0 );
	strLang = g_language.GetString( IDS_MODULE_INFO );
	m_List.InsertColumn( 1, strCnv.ToT( strLang ), LVCFMT_LEFT, 500, 0 );

	for ( i=0 ; i < GetNumWinampPlugins() ; i++ )
	{
		m_List.InsertItem( i, strCnv.ToT( ( GetWinampPluginFileName( i ) ) ) );
		m_List.SetItemText( i,1, strCnv.ToT( GetWinampPluginInfo( i ) ) );

	}

	m_List.SetCurSel( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CWinampInConfig::OnAbout() 
{
	int i = 0;
 	
	i = m_List.GetNextItem( -1 ,LVNI_SELECTED);

	if ( i < 0 )
		i = 0;

	WinampPluginAbout( i, GetSafeHwnd() );
	
}

void CWinampInConfig::OnConfigure() 
{
	int i = 0;
 	
	i = m_List.GetNextItem( -1 ,LVNI_SELECTED);

	if ( i < 0 )
		i = 0;

	WinampPluginConfigure( i , GetSafeHwnd());
}
