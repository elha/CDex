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


#include "StdAfx.h"
#include "FilenamePropPage.h"
#include "config.h"
#include "ConfigDlg.h"
#include "CDex.h"
#include "FileFormatInfoDlg.h"
#include "ReplaceIllCharsDlg.h"
#include "Filename.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FILENAME PROPERTY DIALOG
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CFilenamePropPage, CPropertyPage)

CFilenamePropPage::CFilenamePropPage()
	: CPropertyPage(CFilenamePropPage::IDD )
{
    static CUStringConvert strCnv;

	m_strTitle  = g_language.GetString( IDS_FILENAMEPROPPAGE );
	m_psp.pszTitle = strCnv.ToT( m_strTitle );
	m_psp.dwFlags |= PSP_USETITLE;

	m_bPlsPlayList	= g_config.GetPLSPlayList();
	m_bM3UPlayList	= g_config.GetM3UPlayList();
	m_bInit=FALSE;

	//{{AFX_DATA_INIT(CFilenamePropPage)
	m_strFileNameDemo = _T("");
	m_strPlsFileNameDemo = _T("");
	//}}AFX_DATA_INIT
	m_strFileFormat = g_config.GetFileFormat();
	m_strPlaylistFormat = g_config.GetPlsFileFormat();
	m_bSplitTrack		= g_config.GetSplitTrackName();
	m_strSplitChar		= g_config.GetSplitTrackChar();
}

CFilenamePropPage::~CFilenamePropPage()
{
}

void CFilenamePropPage::DoDataExchange(CDataExchange* pDX)
{
    CUStringConvert strCnv;
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFilenamePropPage)
	DDX_Text(pDX, IDC_FNAMEDEMO, m_strFileNameDemo);
	DDX_Check(pDX, IDC_PLSPLAYLIST, m_bPlsPlayList);
	DDX_Check(pDX, IDC_M3UPLAYLIST, m_bM3UPlayList);
    DDX_Text(pDX, IDC_FILEFORMAT, m_strFileFormat );
	DDX_Text(pDX, IDC_PLAYLISTFORMAT, m_strPlaylistFormat);
	DDX_Text(pDX, IDC_PLAYLISTDEMO, m_strPlsFileNameDemo);
	DDX_Text(pDX, IDC_SPLITCHAR, m_strSplitChar);
	DDX_Check(pDX, IDC_SPLITTRACKNAME, m_bSplitTrack);
	//}}AFX_DATA_MAP
	// Set Folder selection 
	CUString strLang( g_language.GetString( IDS_PLEASE_SELECTFOLDER ) );

	DDX_GetFolderControl(pDX, IDC_MP3OUDIR, m_MP3OutputDir, 0, strCnv.ToT( strLang ) );
	DDX_GetFolderControl(pDX, IDC_CNVOUTDIR, m_CnvOutputDir, 0, strCnv.ToT( strLang ) );
}

BOOL CFilenamePropPage::PreTranslateMessage( MSG *pMsg )
{

	if( pMsg->message == WM_MOUSEMOVE )
	{
		POINT pt = pMsg->pt;    
		ScreenToClient( &pt );

		// this is the only function to call
		m_Tooltip.ShowToolTip( (CPoint)pt );
	}

	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CFilenamePropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

    CUStringConvert strCnv;

	// Set MP3 output directory	
	m_MP3OutputDir.SetWindowText( strCnv.ToT( g_config.GetMP3OutputDir() ) );

	// Set wav conversion output directory
	m_CnvOutputDir.SetWindowText( strCnv.ToT( g_config.GetCnvOutputDir() ) );

	// Update controls
	UpdateData(FALSE);

	m_bInit=TRUE;

	// Create the CToolTipCtrl control.
	m_Tooltip.Create( this );

	m_Tooltip.AddControlInfo( IDC_PLAYLISTFORMAT, IDS_FILEFORMAT_TT );
	m_Tooltip.AddControlInfo( IDC_FILEFORMAT, IDS_FILEFORMAT_TT );

	// Force an OnChange to update seprate character
	OnChange();

	return TRUE;
}

void CFilenamePropPage::UpdateControls()
{
	UpdateData(TRUE);

	// Get file format string
	g_config.SetFileFormat( CUString( m_strFileFormat ));

	// Get playlist file format string
	g_config.SetPlsFileFormat( CUString( m_strPlaylistFormat ) );

	// Set MP3 output directory
	CString strDir;
	m_MP3OutputDir.GetWindowText(strDir);
	if (!strDir.IsEmpty())
	{
		// Add trailing back space
		if (strDir[strDir.GetLength()-1]!= _T( '\\' ) )
		{
			strDir += _T( "\\" );
		}
	}
	// And actually set it now
	g_config.SetMP3OutputDir( CUString( strDir ));

	// DO the same for the WAV conversion output directory
	m_CnvOutputDir.GetWindowText(strDir);
	if (!strDir.IsEmpty())
	{
		// Add trailing back space
		if (strDir[strDir.GetLength()-1]!='\\')
		{
			strDir+="\\";
		}
	}
	// And actually set it now
	g_config.SetCnvOutputDir( CUString( strDir ));

	// Set playlist stuff
	g_config.SetPLSPlayList(m_bPlsPlayList);
	g_config.SetM3UPlayList(m_bM3UPlayList);

	// for fools who are deleting the m_strSplitChar
	if (!m_strSplitChar.IsEmpty() && m_strSplitChar.GetAt(0)>0)
		g_config.SetSplitTrackChar( (CHAR)m_strSplitChar.GetAt(0));

	// Set the BOOL if we need to split the track name
	g_config.SetSplitTrackName(m_bSplitTrack);


}


BOOL CFilenamePropPage::OnApply()
{
	// Call parent to update all property sheets
	((ConfigSheet*)GetParent())->OnApply();

	// Do default stuff
	return CPropertyPage::OnApply();
}

void CFilenamePropPage::OnChange() 
{
	CUString	strLang;

	if (m_bInit)
	{
		SetModified(TRUE);

		UpdateData(TRUE);

		CUString strDemo[ NUMFILENAME_IDS ];
        
        strLang = g_language.GetString(IDS_ARTIST)			;strDemo[0]=strLang;
        strLang = g_language.GetString(IDS_ALBUM)			;strDemo[1]=strLang;
        strLang = g_language.GetString(IDS_TRACKNUMBER)		;strDemo[2]=strLang;
        strLang = g_language.GetString(IDS_TRACKNAME)		;strDemo[3]=strLang;
        strLang = g_language.GetString(IDS_VOLUMEID)		;strDemo[4]=strLang;
        strLang = g_language.GetString(IDS_CDDBID)			;strDemo[5]=strLang;
        strLang = g_language.GetString(IDS_TRACKNUMBERLZ)	;strDemo[6]=strLang;
        strLang = g_language.GetString(IDS_NUMBEROFTRACKS)	;strDemo[7]=strLang;
        strLang = g_language.GetString(IDS_YEAR)			;strDemo[8]=strLang;
        strLang = g_language.GetString(IDS_GENRE)			;strDemo[9]=strLang;
        strLang = g_language.GetString(IDS_ARTIST)			;strDemo[10]=strLang;

		CUString strDir,strName;

		BuildFileName( CUString( m_strFileFormat ),strDemo,strName,strDir);

		m_strFileNameDemo=strDir+strName;

		BuildFileName( CUString( m_strPlaylistFormat ),strDemo,strName,strDir);

		m_strPlsFileNameDemo=strDir+strName;

		SetModified(TRUE);

		UpdateData(FALSE);
	}
}


void CFilenamePropPage::OnChangeFileFormat() 
{
	// Get updated value out of controls
	UpdateData(TRUE);

	// Update controls (i.e. build new file name)
	OnChange();

	// re-update the tooltip
	m_Tooltip.ShowToolTip( IDC_FILEFORMAT );
}

void CFilenamePropPage::OnChangePlaylistFormat() 
{
	// Get updated value out of controls
	UpdateData(TRUE);

	// Update controls (i.e. build new file name)
	OnChange();

	// re-update the tooltip
	m_Tooltip.ShowToolTip( IDC_PLAYLISTFORMAT );
}

void CFilenamePropPage::OnFnameformatinfo() 
{
	CFileFormatInfoDlg dlg;
	dlg.DoModal();
}

void CFilenamePropPage::OnFilename() 
{
//	CFileNameDlg dlg;
/*
	if (dlg.DoModal()==IDOK)
	{
		m_nMask=dlg.m_nMask;
		m_nOrderID=dlg.m_nOrderID;
		OnChange();

	}
*/
}


BEGIN_MESSAGE_MAP(CFilenamePropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CFilenamePropPage)
	ON_EN_CHANGE(IDC_MP3OUDIR, OnChange)
	ON_BN_CLICKED(IDC_FILENAME, OnFilename)
	ON_EN_CHANGE(IDC_FILEFORMAT, OnChangeFileFormat)
	ON_BN_CLICKED(IDC_FNAMEFORMATINFO, OnFnameformatinfo)
	ON_BN_CLICKED(IDC_ILLEGAL_CHARS, OnIllegalChars)
	ON_EN_CHANGE(IDC_PLAYLISTFORMAT, OnChangePlaylistFormat)
	ON_EN_CHANGE(IDC_CNVOUTDIR, OnChange)
	ON_BN_CLICKED(IDC_M3UPLAYLIST, OnChange)
	ON_BN_CLICKED(IDC_PLSPLAYLIST, OnChange)
	ON_BN_CLICKED(IDC_PLAYLISTFMTINFO, OnPlaylistfmtinfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



void CFilenamePropPage::OnIllegalChars() 
{
	CReplaceIllCharsDlg dlg;
	dlg.DoModal();
}

void CFilenamePropPage::OnPlaylistfmtinfo() 
{
	CFileFormatInfoDlg dlg;
	dlg.DoModal();
}
