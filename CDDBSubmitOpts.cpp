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
#include "config.h"
#include "CDDBSubmitOpts.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCDDBSubmitOpts dialog


CCDDBSubmitOpts::CCDDBSubmitOpts(CWnd* pParent /*=NULL*/)
	: CDialog(CCDDBSubmitOpts::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCDDBSubmitOpts)
	m_strEmailAddress = g_config.GetCDDBESubmitAddr();
	m_strHTMLAddress = g_config.GetCDDBHSubmitAddr();
	m_nSubmitVia = g_config.GetCDDBSubmitVia();
	m_strSMTPServer = g_config.GetCDDBESubmitServer();
	//}}AFX_DATA_INIT
}


void CCDDBSubmitOpts::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCDDBSubmitOpts)
	DDX_Text(pDX, IDC_EMAILADDRESS, m_strEmailAddress);
	DDX_Text(pDX, IDC_HTMLADDRESS, m_strHTMLAddress);
	DDX_Radio(pDX, IDC_SUBMITVIA, m_nSubmitVia);
	DDX_Text(pDX, IDC_SMTPSERVER, m_strSMTPServer);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCDDBSubmitOpts, CDialog)
	//{{AFX_MSG_MAP(CCDDBSubmitOpts)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCDDBSubmitOpts message handlers

void CCDDBSubmitOpts::OnOK() 
{
	UpdateData(TRUE);

	g_config.SetCDDBESubmitAddr( CUString( m_strEmailAddress ) );
	g_config.SetCDDBHSubmitAddr( CUString( m_strHTMLAddress ) );
	g_config.SetCDDBSubmitVia( m_nSubmitVia);
	g_config.SetCDDBESubmitServer( CUString( m_strSMTPServer ) );

	CDialog::OnOK();
}

BOOL CCDDBSubmitOpts::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );
	
	return TRUE;
}
