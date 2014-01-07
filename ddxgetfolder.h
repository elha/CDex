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


////////////////////////////////// Macros ///////////////////////////

#ifndef __DDXGETFOLDER_H__
#define __DDXGETFOLDER_H__


////////////////////////////////// Consts /////////////////////////////////////

//flags used to control how the DDX_GetFolderControl routine works

const DWORD GFLDR_OLD_STYLE_DIALOG = 0x0001;   //Use the old style file open dialog instead of the
                                               //style as used in Windows Explorer



////////////////////// foward declaration ///////////////////////////
class CGetFolderControl;


/////////////////////////// Classes /////////////////////////////////
class CModifyButton : public CButton
{
public:
  CModifyButton();
  void SetBuddy(CGetFolderControl* pBuddy);

protected:
  //{{AFX_VIRTUAL(CModifyButton)
  public:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  //{{AFX_MSG(CModifyButton)
  afx_msg void OnClicked();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  CGetFolderControl* m_pBuddy;
  BOOL m_bFirstCall;
  CToolTipCtrl m_ToolTip;
};



class CGetFolderControl : public CStatic
{
public:
  CGetFolderControl();

  BOOL SubclassEdit(HWND hEdit);
  void SetFlags(DWORD dwFlags, const CString& sTitle);
  void Edit();

protected:
  //{{AFX_VIRTUAL(CGetFolderControl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

  //{{AFX_MSG(CGetFolderControl)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  BOOL AddEditButton();

  CModifyButton m_Edit;
  DWORD         m_dwFlags;
  CString       m_sTitle;
};





/////////////// MFC Data exchange routines //////////////////////////

void DDX_GetFolderControl(CDataExchange* pDX, int nIDC, CGetFolderControl& rCGetFolderControl, DWORD dwFlags, const CString& sTitle);


#endif //_DDXGETFOLDER_H__