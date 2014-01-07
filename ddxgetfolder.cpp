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


/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "ddxgetfolder.h"
#include "Util.h"
#include "ChooseDirDlg.h"


///////////////////////////////// defines /////////////////////////////////////

#define GETFOLDER_EDIT_ID 0x0C8F


BEGIN_MESSAGE_MAP(CModifyButton, CButton)
  //{{AFX_MSG_MAP(CModifyButton)
  ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

        
CModifyButton::CModifyButton()
{
	m_pBuddy = NULL;
	m_bFirstCall = TRUE;
}


void CModifyButton::SetBuddy(CGetFolderControl* pBuddy)
{
	m_pBuddy = pBuddy;
}


BOOL CModifyButton::PreTranslateMessage(MSG* pMsg) 
{                     
	//create the tooltip
	if (m_bFirstCall)
	{
		CString strLang( g_language.GetString( IDS_DDX_GFLDR_TT_MODIFY ) );

		m_ToolTip.Create( this );
		m_ToolTip.Activate( TRUE );
		m_ToolTip.AddTool( this, strLang );
		m_bFirstCall = FALSE;
	}
  

	//give the tooltip a chance to handle the message
	m_ToolTip.RelayEvent(pMsg);

	return CButton::PreTranslateMessage(pMsg);
}


void CModifyButton::OnClicked() 
{
	if (m_pBuddy)
		m_pBuddy->Edit();
	else
		TRACE0("CModifyButton: No auto buddy defined\n");
}                        



BEGIN_MESSAGE_MAP(CGetFolderControl, CStatic)
  //{{AFX_MSG_MAP(CGetFolderControl)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


CGetFolderControl::CGetFolderControl()
{
}


BOOL CGetFolderControl::SubclassEdit(HWND hEdit)
{   
	// Test our inputs
	ASSERT(this);
	if (!IsWindow(hEdit))
	{
		ASSERT(FALSE);
		TRACE0("CGetFolderControl::SubclassEdit -- window handle is invalid!\n");

		return FALSE;
	}                
  
	// Subclass the control
	if (SubclassWindow(hEdit))
	{
		//This control can only be used with a read only edit control
		LONG lStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
		if (!(lStyle & ES_READONLY))
		{
			TRACE0("CGetFolderControl::SubclassEdit -- ES_READONLY style should be set for the edit control\n");
			ASSERT(FALSE);
			return FALSE;
		}

	    return AddEditButton();
	}
	else
	{
		TRACE0("CGetFolderControl::SubclassEdit -- Could not subclass edit control!\n");
		ASSERT(FALSE);
		return FALSE;
	}
}


BOOL CGetFolderControl::AddEditButton()
{
	CRect Rect;
	GetWindowRect(Rect);
	GetParent()->ScreenToClient(Rect);
	Rect.left = Rect.right;
	Rect.right = Rect.left + (Rect.Height()*8/10);  //width is 8/10 of height

	// Dynamically create the edit button control
	CString sEditControlText;
	sEditControlText = g_language.GetString(IDS_DDX_GFLDR_EDIT_TEXT);


	BOOL bSuccess = m_Edit.Create(sEditControlText, WS_VISIBLE | WS_CHILD | WS_GROUP, Rect, GetParent(), GETFOLDER_EDIT_ID);

	// Tell the button to call this class when it is clicked
	m_Edit.SetBuddy(this);

	// Ensure it is using the same font as the parent
	m_Edit.SetFont(GetParent()->GetFont());

	return bSuccess;
}


void CGetFolderControl::Edit()
{
	CChooseDirDlg dlg;
	CString sDir;
	GetWindowText( sDir );
    CUString strNewDir( sDir );

	if (dlg.GetDirectory( strNewDir, this, (m_dwFlags ), CUString( m_sTitle )))
    {
        CUStringConvert strCnv;
        SetWindowText( strCnv.ToT( strNewDir ) );
    }
}


void CGetFolderControl::SetFlags(DWORD dwFlags, const CString& sTitle) 
{ 
	m_dwFlags = dwFlags; 
	m_sTitle = sTitle;
}



void DDX_GetFolderControl(CDataExchange* pDX, int nIDC, CGetFolderControl& rCGetFolderControl, DWORD dwFlags, const CString& sTitle)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (!pDX->m_bSaveAndValidate && rCGetFolderControl.m_hWnd == NULL)    // not subclassed yet
	{
		if (!rCGetFolderControl.SubclassEdit(hWndCtrl))
		{
			ASSERT(FALSE);      // possibly trying to subclass twice ?
			AfxThrowNotSupportedException();
		}
	}
	rCGetFolderControl.SetFlags(dwFlags, sTitle);
}

BOOL CGetFolderControl::PreTranslateMessage(MSG* pMsg) 
{
	if ( pMsg->message == WM_KEYDOWN )
	{
		if ( VK_RETURN == pMsg->wParam )
		{
			m_Edit.OnClicked();
			return TRUE;
		}
	}
	
	return CStatic::PreTranslateMessage(pMsg);
}
