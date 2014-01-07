/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1997 PJ Naughter
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


////////////////////////////////// Macros ///////////////////////////

#ifndef __CHOOSEDIRDLG_H__
#define __CHOOSEDIRDLG_H__


/////////////////////////// Classes /////////////////////////////////

class CChooseDirDlg : public CObject
{
public:
//Get a directory
  BOOL GetDirectory(CUString& sDir, CWnd* pWndParent, BOOL bOldStyleDialog, const CUString& sTitle);

protected:
  static int CALLBACK SetSelProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
};



#endif //_CHOOSEDIRDLG_H__