/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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
#include "DDXGetFile.h"
#include "Util.h"

///////////////////////////////// defines /////////////////////////////////////

#define GETFOLDER_EDIT_ID 0x0C8F

BEGIN_MESSAGE_MAP(CFModifyButton, CButton)
  //{{AFX_MSG_MAP(CFModifyButton)
  ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

        
CFModifyButton::CFModifyButton()
{
	m_pBuddy = NULL;
	m_bFirstCall = TRUE;
}


void CFModifyButton::SetBuddy(CGetFileControl* pBuddy)
{
	m_pBuddy = pBuddy;
}


BOOL CFModifyButton::PreTranslateMessage(MSG* pMsg) 
{                     
	//create the tooltip
	if (m_bFirstCall)
	{
		CUString strLang( g_language.GetString( IDS_DDX_GETFILE_SELECTFILE ) );

		m_ToolTip.Create( this );
		m_ToolTip.Activate( TRUE );
        CUStringConvert strCnv;
		m_ToolTip.AddTool( this, strCnv.ToT( strLang ) );
		m_bFirstCall = FALSE;
	}
  

	//give the tooltip a chance to handle the message
	m_ToolTip.RelayEvent(pMsg);

	return CButton::PreTranslateMessage(pMsg);
}


void CFModifyButton::OnClicked() 
{
	if (m_pBuddy)
		m_pBuddy->Edit();
	else
		TRACE0("CFModifyButton: No auto buddy defined\n");
}                        



BEGIN_MESSAGE_MAP(CGetFileControl, CStatic)
  //{{AFX_MSG_MAP(CGetFileControl)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


CGetFileControl::CGetFileControl()
{
}


BOOL CGetFileControl::SubclassEdit(HWND hEdit)
{   
	// Test our inputs
	ASSERT(this);
	if (!IsWindow(hEdit))
	{
		ASSERT(FALSE);
		TRACE0("CGetFileControl::SubclassEdit -- window handle is invalid!\n");

		return FALSE;
	}                
  
	// Subclass the control
	if (SubclassWindow(hEdit))
	{
		//This control can only be used with a read only edit control
		LONG lStyle = GetWindowLong(GetSafeHwnd(), GWL_STYLE);
		if (!(lStyle & ES_READONLY))
		{
			TRACE0("CGetFileControl::SubclassEdit -- ES_READONLY style should be set for the edit control\n");
			ASSERT(FALSE);
			return FALSE;
		}

	    return AddEditButton();
	}
	else
	{
		TRACE0("CGetFileControl::SubclassEdit -- Could not subclass edit control!\n");
		ASSERT(FALSE);
		return FALSE;
	}
}


BOOL CGetFileControl::AddEditButton()
{
	CRect Rect;
	GetWindowRect(Rect);
	GetParent()->ScreenToClient(Rect);
	Rect.left = Rect.right;
	Rect.right = Rect.left + (Rect.Height()*8/10);  //width is 8/10 of height

	// Dynamically create the edit button control
	CUString sEditControlText;
	sEditControlText = g_language.GetString(IDS_DDX_GFLDR_EDIT_TEXT);

    CUStringConvert strCnv;
	BOOL bSuccess = m_Edit.Create( strCnv.ToT( sEditControlText ), WS_VISIBLE | WS_CHILD | WS_GROUP, Rect, GetParent(), GETFOLDER_EDIT_ID);

	// Tell the button to call this class when it is clicked
	m_Edit.SetBuddy(this);

	// Ensure it is using the same font as the parent
	m_Edit.SetFont(GetParent()->GetFont());

	return bSuccess;
}


void CGetFileControl::Edit()
{

    CUStringConvert strCnv;

	// Create file open dialog
    CFileDialog dlg( TRUE, _T( "" ) , NULL, OFN_HIDEREADONLY, strCnv.ToT( m_strFilter ) );

	if ( IDOK == dlg.DoModal() )
	{
		POSITION pos = dlg.GetStartPosition();
		SetWindowText( dlg.GetNextPathName( pos ) );
		UpdateData( FALSE );
	}

}


void CGetFileControl::SetFlags(DWORD dwFlags, const CUString& strExt) 
{ 
	m_dwFlags = dwFlags; 
	m_strFilter = strExt;
}



void DDX_GetFileControl(CDataExchange* pDX, int nIDC, CGetFileControl& rCGetFileControl, DWORD dwFlags, const CUString& strExt)
{
	HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
	if (!pDX->m_bSaveAndValidate && rCGetFileControl.m_hWnd == NULL)    // not subclassed yet
	{
		if (!rCGetFileControl.SubclassEdit(hWndCtrl))
		{
			ASSERT(FALSE);      // possibly trying to subclass twice ?
			AfxThrowNotSupportedException();
		}
	}

	rCGetFileControl.SetFlags(dwFlags, strExt );
}

BOOL CGetFileControl::PreTranslateMessage(MSG* pMsg) 
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
