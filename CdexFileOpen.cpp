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


// CdexFileOpen.cpp : implementation file
//

#include "stdafx.h"
#include "cdex.h"
#include "CdexFileOpen.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCdexFileOpen

IMPLEMENT_DYNAMIC(CCdexFileOpen, CFileDialog)

CCdexFileOpen::CCdexFileOpen(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
	m_bInit=FALSE;
}


BEGIN_MESSAGE_MAP(CCdexFileOpen, CFileDialog)
	//{{AFX_MSG_MAP(CCdexFileOpen)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CCdexFileOpen::OnInitDialog() 
{

//  GetDlgItem(stc2)->ShowWindow(SW_HIDE);
//  GetDlgItem(stc3)->ShowWindow(SW_HIDE);
//  GetDlgItem(edt1)->ShowWindow(SW_HIDE);
//  GetDlgItem(lst1)->ShowWindow(SW_HIDE);
  GetDlgItem(cmb1)->ShowWindow(SW_HIDE);
    
  //We must put something in this field, even though it is hidden.  This is
  //because if this field is empty, or has something like "*.txt" in it,
  //and the user hits OK, the dlg will NOT close.  We'll jam something in
  //there (like "Junk") so when the user hits OK, the dlg terminates.
  //Note that we'll deal with the "Junk" during return processing (see below)

  SetDlgItemText(edt1, _T("Junk"));

  //Now set the focus to the directories listbox.  Due to some painting
  //problems, we *must* also process the first WM_PAINT that comes through
  //and set the current selection at that point.  Setting the selection
  //here will NOT work.  See comment below in the on paint handler.
            
//  GetDlgItem(lst2)->SetFocus();

	m_bInit=TRUE;

	CFileDialog::OnInitDialog();

	// translate dialog resources
//	g_language.InitDialogStrings( this, IDD );

  
	return FALSE;
}

void CCdexFileOpen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (m_bInit==TRUE)
	{
		m_bInit=FALSE;
		SendDlgItemMessage(lst2, LB_SETCURSEL, 0, 0L);
	}	
}
