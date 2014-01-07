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
#include "config.h"
#include "cdex.h"
#include "ConfigDlg.h"
#include "TreePropSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define WIN_ENCODER_ENABLED		
#ifdef _DEBUG
#endif


BOOL g_bEncPresent=FALSE;
BOOL g_bEncDllPresent=FALSE;
BOOL g_bHasBladeMP3=FALSE;




/////////////////////////////////////////////////////////////////////////////
// ConfigSheet

IMPLEMENT_DYNAMIC(ConfigSheet, CTreePropSheet)


ConfigSheet::ConfigSheet( int nSelectTab,CWnd* pParentWnd )
	: CTreePropSheet( IDS_PROPSHEET_CDEXCONFIG, pParentWnd )
{
    CUStringConvert strCnv;

	// translate property sheet title
    SetTitle( strCnv.ToT( g_language.GetString( IDS_PROPSHEET_CDEXCONFIG ) ) );

	m_psh.dwFlags |= PSH_NOAPPLYNOW;	// Construct pages

    SetTreeViewMode( TRUE, TRUE, 0 );

    SetEmptyPageText( strCnv.ToT( g_language.GetString( IDS_PROPPAGE_SELECT_CHILD_ITEM  ) ));

	AddPage( &m_GenericPropPage );
	AddPage( &m_FilenamePropPage );
	AddPage( &m_CDPropPage );
	AddPage( &m_EncoderPropPage );
	AddPage( &m_LocalCDDBPropPage );
	AddPage( &m_RemoteCDDBPropPage );
	AddPage( &m_TagPropPage );

	ASSERT( nSelectTab>=0 && nSelectTab<4);

	m_bIsInSync = FALSE;

	// Select Active Page
	SetActivePage( nSelectTab );
}


ConfigSheet::~ConfigSheet()
{
}

BOOL ConfigSheet::OnApply()
{
	if ( FALSE == m_bIsInSync )
	{
		if (m_CDPropPage.m_hWnd!=NULL) m_CDPropPage.UpdateControls();
		if (m_EncoderPropPage.m_hWnd!=NULL) m_EncoderPropPage.UpdateControls();
		if (m_GenericPropPage.m_hWnd!=NULL) m_GenericPropPage.UpdateControls();
		if (m_TagPropPage.m_hWnd!=NULL) m_TagPropPage.UpdateControls();
		if (m_RemoteCDDBPropPage.m_hWnd!=NULL) m_RemoteCDDBPropPage.UpdateControls();
		if (m_FilenamePropPage.m_hWnd!=NULL) m_FilenamePropPage.UpdateControls();
		if (m_LocalCDDBPropPage.m_hWnd!=NULL) m_LocalCDDBPropPage.UpdateControls();

		g_config.Save();

		m_bIsInSync = TRUE;
	}

	return 0;
}

BEGIN_MESSAGE_MAP(ConfigSheet, CTreePropSheet)
	//{{AFX_MSG_MAP(ConfigSheet)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ConfigSheet message handlers
