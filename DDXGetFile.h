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


#ifndef DDXGETFILE_H_INCLUDED
#define DDXGETFILE_H_INCLUDED


////////////////////////////////// Consts /////////////////////////////////////

//flags used to control how the DDX_GetFolderControl routine works



////////////////////// foward declaration ///////////////////////////
class CGetFileControl;


/////////////////////////// Classes /////////////////////////////////
class CFModifyButton : public CButton
{
public:
  CFModifyButton();
  void SetBuddy(CGetFileControl* pBuddy);

protected:
  //{{AFX_VIRTUAL(CFModifyButton)
  public:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

  //{{AFX_MSG(CFModifyButton)
  afx_msg void OnClicked();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  CGetFileControl* m_pBuddy;
  BOOL m_bFirstCall;
  CToolTipCtrl m_ToolTip;
};




class CGetFileControl : public CStatic
{
public:
  CGetFileControl();

  BOOL SubclassEdit(HWND hEdit);
  void SetFlags(DWORD dwFlags, const CUString& strFilter );
  void Edit();

protected:
  //{{AFX_VIRTUAL(CGetFileControl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

  //{{AFX_MSG(CGetFileControl)
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()

  BOOL AddEditButton();

  CFModifyButton	m_Edit;
  DWORD		        m_dwFlags;
  CUString		    m_strFilter;
};





/////////////// MFC Data exchange routines //////////////////////////

void DDX_GetFileControl(CDataExchange* pDX, int nIDC, CGetFileControl& rCGetFileControl, DWORD dwFlags, const CUString& strFilter );


#endif // DDXGETFILE_H_INCLUDED