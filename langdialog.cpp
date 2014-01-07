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


// LangDialog.cpp : implementation file
//

#include "stdafx.h"
#include "cdex.h"
#include "LangDialog.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLangDialog dialog


CLangDialog::CLangDialog(UINT nIDTemplate, CWnd* pParent )
	: CDialog( nIDTemplate, pParent )
{
	//{{AFX_DATA_INIT(CLangDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nIDTemplate = nIDTemplate;
}

CLangDialog::~CLangDialog( )
{
}


void CLangDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLangDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLangDialog, CDialog)
	//{{AFX_MSG_MAP(CLangDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLangDialog message handlers

BOOL CLangDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, (long)m_nIDTemplate );
		
	return TRUE;
}

