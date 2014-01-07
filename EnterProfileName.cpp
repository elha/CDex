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
#include "Util.h"
#include "EnterProfileName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEnterProfileName dialog


CEnterProfileName::CEnterProfileName(CWnd* pParent /*=NULL*/)
	: CDialog(CEnterProfileName::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEnterProfileName)
	m_strProfileName = _T("");
	//}}AFX_DATA_INIT
}


void CEnterProfileName::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEnterProfileName)
	DDX_Text(pDX, IDC_PROFILENAME, m_strProfileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEnterProfileName, CDialog)
	//{{AFX_MSG_MAP(CEnterProfileName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEnterProfileName message handlers



BOOL CEnterProfileName::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );
	
	return TRUE;
}
