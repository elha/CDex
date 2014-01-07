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
#include "CDexMsgBox.h"
#include ".\cdexmsgbox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCDexMsgBox dialog


CCDexMsgBox::CCDexMsgBox(CWnd* pParent /*=NULL*/)
	: CDialog(CCDexMsgBox::IDD, pParent)
{
}


void CCDexMsgBox::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MSGBOXTEXT, m_msgBoxText);
}


BEGIN_MESSAGE_MAP(CCDexMsgBox, CDialog)
    ON_STN_CLICKED(IDC_DETECTGAPS, OnStnClickedDetectgaps)
    ON_BN_CLICKED(IDC_ABORT, OnBnClickedAbort)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCDexMsgBox message handlers



BOOL CCDexMsgBox::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_bAbort = false;
    // translate dialog resources
	g_language.InitDialogStrings( this, IDD );
	
	return TRUE;
}

void CCDexMsgBox::OnStnClickedDetectgaps()
{
}

void CCDexMsgBox::PostNcDestroy()
{
    CDialog::PostNcDestroy();
    delete this;
}

void CCDexMsgBox::OnBnClickedAbort()
{
    m_bAbort = true;
}
