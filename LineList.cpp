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


#include "Stdafx.h"
#include "LineList.h"
#include ".\linelist.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// LListCtrl


/////////////////////////////////////////////////////////////////////////////
// GetCurSel -
//		an obvious CListCtrl deficiency, tell us index of current selection.
/////////////////////////////////////////////////////////////////////////////
int LListCtrl::GetCurSel() const
{
    return GetNextItem(-1,LVNI_ALL|LVNI_SELECTED);
}

/////////////////////////////////////////////////////////////////////////////
// SetCurSel -
//		an obvious CListCtrl deficiency, specify and make visible a new selection.
/////////////////////////////////////////////////////////////////////////////
int LListCtrl::SetCurSel(int nSelect) 
{
    if(!SetItemState(nSelect,LVIS_SELECTED,LVIS_SELECTED))
    {
        return LB_ERR;
    }
    if(!EnsureVisible(nSelect,FALSE))
    {
        return LB_ERR;
    }
    return TRUE;
}

BEGIN_MESSAGE_MAP(LListCtrl, CListCtrl)
    //{{AFX_MSG_MAP(LListCtrl)
    ON_NOTIFY_REFLECT(LVN_INSERTITEM, OnInsertitem)
    ON_WM_GETDLGCODE()
    ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
    //}}AFX_MSG_MAP
    //	ON_NOTIFY_REFLECT(NM_CLICK, OnClick)
    //	ON_NOTIFY_REFLECT(LVN_BEGINLABELEDIT, OnBeginlabeledit)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// OnGetDialogCode - 
//		allow our control to recieve crack at parent dialog's key codes
//		this allows the control to catch RETURN rather than the dialog's
//		default push button and hence to support up arrow/down arrow/return 
//		item selection (must also place a NM_RETURN message handler in the 
//		dialog's message map). 
/////////////////////////////////////////////////////////////////////////////
//UINT LListCtrl::OnGetDlgCode()
//{
//return CListCtrl::OnGetDlgCode() | DLGC_WANTALLKEYS;
//}

/////////////////////////////////////////////////////////////////////////////
// OnInsertItem - 
//		make item selectable and hi-lighted by entire row, not just first 
//		column sub-item (an undocumented tid-bit of the Net). Yes there must
//		be a better place for this but I'll be damned if I can find it. 
//		OnCreate(), Create(), etc, as added by ClassWizard do not hook, 
//		place it here for now.
/////////////////////////////////////////////////////////////////////////////
void LListCtrl::OnInsertitem(NMHDR* pNMHDR, LRESULT* pResult) 
{
    ListView_SetExtendedListViewStyle(m_hWnd,LVS_EX_FULLROWSELECT);
}


// hard coded margins in inches and pixels:
#define PRINTMARGIN_Y 0.50		
#define PRINTMARGIN_X 0.50
#define PRINTMARGINY ((int)((double)pDC->GetDeviceCaps(LOGPIXELSY)*PRINTMARGIN_Y))
#define PRINTMARGINX ((int)((double)pDC->GetDeviceCaps(LOGPIXELSX)*PRINTMARGIN_X))
#define LINESPACING	 ((csCharSize.cy * 6)/5)
int LListCtrl::SelectAll() 
{
    for (int i=0;i<GetItemCount();i++)
    {
        SetCurSel(i);
    }

    // Scroll up to the first selected
    SetCurSel(0);

    SetItemState(-1,LVIS_FOCUSED,LVIS_FOCUSED);

    return TRUE;
}


#define VK_C		67
#define VK_V		86
#define VK_X		88
#define VK_Z		90
BOOL LListCtrl::PreTranslateMessage(MSG* pMsg)
{
    // If edit control is visible in tree view control, sending a
    // WM_KEYDOWN message to the edit control will dismiss the edit
    // control.  When ENTER key was sent to the edit control, the parent
    // window of the tree view control is responsible for updating the
    // item's label in TVN_ENDLABELEDIT notification code.
    if ( pMsg->message == WM_KEYDOWN )
    {
        CHAR ckey=toupper( pMsg->wParam &0xFF );

        if ( VK_RETURN == pMsg->wParam )
            m_bEditNext = TRUE;
        if ( VK_ESCAPE == pMsg->wParam )
            m_bEditNext = FALSE;


        if( GetKeyState( VK_CONTROL )<-1 && (ckey== 'A') )
        {
            SelectAll();
            return TRUE;
        }
        if( GetKeyState( VK_CONTROL )<-1 && (ckey== 'N') )
        {
            SelectNone();
            return TRUE;
        }


        CEdit* edit = GetEditControl();
        if (edit)
        {
            //			LTRACE("Control key status = %d %d\n",LOBYTE(GetKeyState( VK_CONTROL )),HIWORD(GetKeyState( VK_CONTROL )));

            if( GetKeyState( VK_CONTROL )<-1 && (ckey== _T( 'C' ) ) )
            {
                edit->Copy();
                return TRUE;
            }
            if( GetKeyState( VK_CONTROL )<-1 && (ckey== _T( 'V' ) ) )
            {
                edit->Paste();
                return TRUE;
            }
            if( GetKeyState( VK_CONTROL )<-1 && (ckey== _T( 'X' ) ) )
            {
                edit->Cut();
                return TRUE;
            }
            if( GetKeyState( VK_CONTROL )<-1 && (ckey== _T( 'Z' ) ) )
            {
                edit->Undo();
                return TRUE;
            }
            if( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CONTROL || pMsg->wParam == VK_INSERT || pMsg->wParam == VK_SHIFT )
            {
                edit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
                return TRUE;
            }
        }
    }
    return CListCtrl::PreTranslateMessage(pMsg);
}

int LListCtrl::SelectNone() 
{
    // -1 changes the state of all items in the list.
    return SetItemState(-1, ~LVIS_SELECTED, LVIS_SELECTED);

    //	return SetCurSel(-1);
}

void LListCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
    /*
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM	*pItem = &pDispInfo->item;

    if ( pItem->pszText != NULL )
    SetItemText( pItem->iItem, pItem->iSubItem, pItem->pszText );

    */	
    int nFocused = GetNextItem(-1,LVNI_FOCUSED);

    // select next item
    nFocused++;

    //	PostMessage( WM_KEYDOWN, 28, 0x1500001 );
    //	PostMessage( WM_KEYUP, 28, 0x1500001 );
    //	PostMessage( WM_KEYDOWN, 71, 0x3c0001 );
    //	PostMessage( WM_KEYUP, 71, 0x3c0001 );
    /*
    int	nCount = GetItemCount();

    if ( nFocused < GetItemCount() )
    {
    //		SetFocus( nFocused );

    EditLabel( nFocused );
    }
    */
    *pResult = TRUE;
}

CMutex myLockListCtrl;

void LListCtrl::SetHeaderControlText( DWORD dwPos, const CUString& strValue )
{
    myLockListCtrl.Lock();
    if ( m_hWnd )
    {
        CHeaderCtrl* pHeaderCtrl = GetHeaderCtrl();

        ASSERT( (int)dwPos < pHeaderCtrl->GetItemCount() );

        HDITEM hdi;

        memset( &hdi, 0, sizeof( HDITEM ) );

        // Get header item data
        pHeaderCtrl->GetItem( dwPos, &hdi );

        // modify item data
        CUStringConvert strCnv;
        hdi.pszText = strCnv.ToT( strValue );
        hdi.mask = HDI_TEXT;

        // Set item data
        pHeaderCtrl->SetItem( dwPos, &hdi );

    }
    myLockListCtrl.Unlock();
}

void LListCtrl::OnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    *pResult = 0;
}

bool LListCtrl::IsColumnHidden( int column ) 
{
    bool hidden = false;
    for ( unsigned int index =0; index <  m_hiddenCols.size(); index++ )
    {
        if (  m_hiddenCols[ index ] == column )
        {
            hidden = true;
        }
    }
    return hidden;
}

void LListCtrl::UnHideColumn( int column ) 
{
    vector<int>::iterator itemToDelete = m_hiddenCols.begin();

    for ( unsigned int index =0; index <  m_hiddenCols.size(); index++ )
    {
        if (  *itemToDelete == column )
        {
            m_hiddenCols.erase( itemToDelete );

        }
        itemToDelete++;
    }
}

void LListCtrl::HideColumn( int column ) 
{
    bool hidden = false;
    for ( unsigned int index =0; index <  m_hiddenCols.size(); index++ )
    {
        if (  m_hiddenCols[ index ] == column )
        {
            hidden = true;
        }
    }
    if ( !hidden )
    {
        m_hiddenCols.push_back( column );
    }
}


BOOL LListCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    switch( ((NMHDR*)lParam)->code)
    {
        // prevent resizing of the 'hidden' column
    case HDN_BEGINTRACKA:
    case HDN_BEGINTRACKW:
        int col = ((HD_NOTIFY FAR *) lParam)->iItem;

        for ( unsigned int i=0;i< m_hiddenCols.size(); i++ )
        {
            if ( col == m_hiddenCols[ i ] )
            {
                *pResult = TRUE;
                return TRUE;
            }
        }
        break;
    }
    return CListCtrl::OnNotify(wParam, lParam, pResult);
}
