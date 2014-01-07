/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 - 2007 Albert L. Faber
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


#include "StdAfx.h"
#include <afxdisp.h>
#include "ListCtrlSort.h"


////////////////////////////////////////////////////////////////////////  
/////
// CSortClass

CSortClass::CSortClass(CListCtrl * _pWnd, const int _iCol)
{
	pWnd = _pWnd;
	
	ASSERT(pWnd);
	int max = pWnd->GetItemCount();
	DWORD dw;
	
	// replace Item data with pointer to CSortItem structure
	for (int t = 0; t < max; t++)
	{
    	CString txt;
		dw = pWnd->GetItemData(t); // save current data to restore it later
		txt = pWnd->GetItemText(t, _iCol); 
		pWnd->SetItemData(t, (DWORD) new CSortItem(dw, txt));
	}
}

CSortClass::~CSortClass()
{
	ASSERT(pWnd);
	int max = pWnd->GetItemCount();
	CSortItem * pItem;
	for (int t = 0; t < max; t++)
	{
		pItem = (CSortItem *) pWnd->GetItemData(t);
		ASSERT(pItem);
		pWnd->SetItemData(t, pItem->dw);
		delete pItem;
	}
}

void CSortClass::Sort(BOOL _bAsc, EDataType _dtype)
{
	long lParamSort = _dtype;
	
	// if lParamSort positive - ascending sort order, negative - descending
	if (!_bAsc)
		lParamSort *= -1; 
	
	pWnd->SortItems(Compare, lParamSort);
}

int CALLBACK CSortClass::Compare(LPARAM lParam1, LPARAM lParam2, LPARAM 
								 lParamSort)
{
	CSortItem * item1 = (CSortItem *) lParam1;
	CSortItem * item2 = (CSortItem *) lParam2;
	ASSERT(item1 && item2);
	
	// restore data type and sort order from lParamSort
	// if lParamSort positive - ascending sort order, negative - descending
	short   sOrder = lParamSort < 0 ? -1 : 1; 
	EDataType dType  = (EDataType) (lParamSort * sOrder); // get rid of sign
	
	// declare typed buffers
//	COleDateTime t1, t2;
	
	switch (dType)
	{
	case  EDataType::dtINT:
		return ( _ttol(item1->txt) - _ttol(item2->txt) ) * sOrder;
	case  EDataType::dtDEC:
		{
			float fItem1 = 0.0f;
			float fItem2 = 0.0f;

			_stscanf( item1->txt, _T( "%f" ), &fItem1 );
			_stscanf( item2->txt, _T( "%f" ), &fItem2 );

			return ( (fItem1 < fItem2) ? -1 : 1) * sOrder;
		}
	case  EDataType::dtDATETIME:
//		if (t1.ParseDateTime(item1->txt) && t2.ParseDateTime(item2->txt))
//			return (t1 < t2 ? -1 : 1 )*sOrder;
		return ( (time_t)(item1->dw) < (time_t)(item2->dw) ? -1 : 1)*sOrder;

//		else
//			return 0;
	case  EDataType::dtSTRING:
		return item1->txt.CompareNoCase(item2->txt)*sOrder;
		
	default:
		ASSERT("Error: attempt to sort a column without type.");
		return 0;
	}
}


CSortClass::CSortItem::CSortItem(const DWORD _dw, const CString & _txt)
{
	dw  = _dw;
	txt = _txt;
}

/*
void CMyDlg::OnHeaderClicked(NMHDR* pNMHDR, LRESULT* pResult)
{
	static int  nSortedCol = -1;
	static bool bSortAscending = true; 
	
	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	
	if( phdn->iButton == 0 )
	{
		// User clicked on header using left mouse button
		if( phdn->iItem == nSortedCol )
			bSortAscending = !bSortAscending;
		else
			bSortAscending = TRUE;
		
		nSortedCol = phdn->iItem;
		
		CSortClass csc(&m_List, nSortedCol);
		
		csc.Sort(bSortAscending, (CSortClass::EDataType) 
			m_arrColType[nSortedCol]); 
	}
	*pResult = 0;
}
*/
