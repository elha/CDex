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


/*
Module : CHOOSEDIRDLG.CPP
Purpose: Implementation for an MFC class to get a directory/folder. 
         Uses the file open/save as common dialogs or the newer Shell API SHBrowseForFolder
*/


/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "choosedirdlg.h"
#include "shlobj.h"
#include <dlgs.h>



///////////////////////////////// Implementation /////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


class COldFileDirDialog : public CFileDialog
{
public:
// Public data members
  BOOL m_bDlgJustCameUp;
    
// Constructors
  COldFileDirDialog(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
                 LPCTSTR lpszDefExt = NULL,
                 LPCTSTR lpszFileName = NULL,
                 DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                 LPCTSTR lpszFilter = NULL,
                 CWnd* pParentWnd = NULL);
                                          
// Implementation
protected:
  //{{AFX_MSG(COldFileDirDialog)
  virtual BOOL OnInitDialog();
  afx_msg void OnPaint();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};





COldFileDirDialog::COldFileDirDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, 
                                     LPCTSTR lpszFileName, DWORD dwFlags, 
                                     LPCTSTR lpszFilter, CWnd* pParentWnd) 
  : CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
  //{{AFX_DATA_INIT(COldFileDirDialog)
  //}}AFX_DATA_INIT
}


BEGIN_MESSAGE_MAP(COldFileDirDialog, CFileDialog)
  //{{AFX_MSG_MAP(COldFileDirDialog)
  ON_WM_PAINT()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL COldFileDirDialog::OnInitDialog()
{  
  //CenterWindow();

  //Let's hide these windows so the user cannot tab to them.  Note that in
  //the private template the coordinates for these guys are
  //*outside* the coordinates of the dlg window itself.  Without the following
  //ShowWindow()'s you would not see them, but could still tab to them.
    
  GetDlgItem(stc2)->ShowWindow(SW_HIDE);
  GetDlgItem(stc3)->ShowWindow(SW_HIDE);
  GetDlgItem(edt1)->ShowWindow(SW_HIDE);
  GetDlgItem(lst1)->ShowWindow(SW_HIDE);
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
            
  GetDlgItem(lst2)->SetFocus();
            
  m_bDlgJustCameUp=TRUE;
               
  CFileDialog::OnInitDialog();
     
  return(FALSE);
}
  
  
void COldFileDirDialog::OnPaint()
{
  CPaintDC dc(this); // device context for painting
    
  //This code makes the directory listbox "highlight" an entry when it first
  //comes up.  W/O this code, the focus is on the directory listbox, but no
  //focus rectangle is drawn and no entries are selected.  Ho hum.

  if (m_bDlgJustCameUp)
  {
    m_bDlgJustCameUp=FALSE;
    SendDlgItemMessage(lst2, LB_SETCURSEL, 0, 0L);
  }
    
  // Do not call CFileDialog::OnPaint() for painting messages
}


int CALLBACK CChooseDirDlg::SetSelProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
  if (uMsg == BFFM_INITIALIZED)
    ::SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);

  return 0;
}


BOOL CChooseDirDlg::GetDirectory(CUString& sDir, CWnd* pWndParent, BOOL bOldStyleDialog, const CUString& sTitle)
{
  BOOL bSuccess = FALSE;

  BYTE WinMajorVersion = LOBYTE(LOWORD(GetVersion()));
  if ((WinMajorVersion >= 4) && !bOldStyleDialog) //Running on Windows 95 shell and new style requested
  {
    CUStringConvert strCnv;
    CUStringConvert strCnv1;

    TCHAR sDisplayName[_MAX_PATH];
    BROWSEINFO bi;

    bi.hwndOwner = pWndParent->GetSafeHwnd();
    bi.pidlRoot = NULL;
    bi.lpszTitle =  strCnv.ToT( sTitle );
    bi.pszDisplayName = sDisplayName;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = SetSelProc;
    bi.lParam = (LPARAM)strCnv1.ToT( sDir );

	LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bi);

    if (pItemIDList)
    {
      TCHAR sPath[_MAX_PATH];
      if (SHGetPathFromIDList(pItemIDList, sPath))
      {
        bSuccess = TRUE;
        sDir = sPath;
      }

			//avoid memory leaks by deleting the PIDL
			//using the shells task allocator
			IMalloc* pMalloc;
			if (SHGetMalloc(&pMalloc) != NOERROR)
			{
				TRACE( _T( "Failed to get pointer to shells task allocator" ) );
				bSuccess = FALSE;
			}
			pMalloc->Free(pItemIDList);
			if (pMalloc)
			{
				pMalloc->Release();
			}
		}
	}
	else  //Use old style if requested or when running on NT 3.51 where we have no choice
	{
        CUStringConvert strCnv;
        CUStringConvert strCnv1;
        
		COldFileDirDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ENABLETEMPLATE, NULL, pWndParent);
		dlg.m_ofn.Flags &= ~OFN_EXPLORER; //Turn of the explorer style customisation
		dlg.m_ofn.hInstance = AfxGetInstanceHandle();
		dlg.m_ofn.lpTemplateName = MAKEINTRESOURCE(FILEOPENORD);
        dlg.m_ofn.lpstrInitialDir = strCnv1.ToT( sDir );
		dlg.m_ofn.lpstrTitle = strCnv.ToT( sTitle );
		if (dlg.DoModal() == IDOK)
		{
			//Nuke the "Junk" text filename
			dlg.m_ofn.lpstrFile[ dlg.m_ofn.nFileOffset - 1 ] = _T( '\0' );
			bSuccess = TRUE;
			sDir = dlg.m_ofn.lpstrFile;

		}
	}

	if ( !sDir.IsEmpty() )
	{
		if ( sDir.GetAt( sDir.GetLength() - 1 ) != _T( '\\' ) )
		{
			sDir += _T( '\\' );
		}
	}
	  return bSuccess;
}



