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


#ifndef CLISTCTRLSORT_H_INCLUDED
#define CLISTCTRLSORT_H_INCLUDED

class CSortClass
{
public:
	enum EDataType {dtNULL, dtINT, dtSTRING, dtDATETIME, dtDEC};
	
	CSortClass(CListCtrl * _pWnd, const int _iCol);
	virtual ~CSortClass();
	void Sort(BOOL bAsc, EDataType _dtype);
	
protected:
	CListCtrl * pWnd;
	
	static int CALLBACK Compare(LPARAM lParam1, LPARAM lParam2, LPARAM 
		lParamSort);
	
	struct CSortItem
	{
		CSortItem(const DWORD _dw, const CString &_txt);
		DWORD dw; 
		CString txt;
	};
};



#endif