/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 - 2002 Albert L. Faber
** Copyright (C) 1998 - 1999 Juan Franklin Peña
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


#include "stdafx.h"
#include "OpenFileDlg.h"
#include "ListCtrlSort.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void COpenFileDlg::ShowNormalizeCtrl(BOOL bValue)
{
	m_bShowNorm = bValue;
}
void COpenFileDlg::ShowDelOrigCtrl(BOOL bValue)
{
	m_bShowDelOrig = bValue;
}
void COpenFileDlg::ShowRiffWavCtrl(BOOL bValue)
{
	m_bShowRiffWav = bValue;
}
void COpenFileDlg::ShowKeepDirLayoutCtrl(BOOL bValue)
{
	m_bShowKeepDirLayout = bValue;
}


COpenFileDlg::~COpenFileDlg()
{
	delete [] m_strSelectedFiles;
}

/////////////////////////////////////////////////////////////////////////////
// COpenFileDlg dialog

COpenFileDlg::COpenFileDlg(CUString directory
								, CUString fileType
								, int	  iconId
								, BOOL	  bOpenDlg /*= TRUE*/
								, CWnd*   pParent  /*= NULL*/ )
	: CDialog(COpenFileDlg::IDD, pParent)
	, m_strDir(directory)
	, m_strFileType(fileType)
	, m_nIconId(iconId)
	, m_bIsOpenFileDlg(bOpenDlg)
	, m_strFileName( _T( "" ) )
	, m_bSettingsDlg( FALSE )

{
	m_bSortAscending=TRUE;
	m_nSortedColumn=0;
	m_bInit=FALSE;
	m_strSelectedFiles=NULL;
	m_nSelectedFiles=0;
	m_bShowNorm=TRUE;
	m_bShowDelOrig=TRUE;
	m_bShowRiffWav=FALSE;
	m_bShowKeepDirLayout = TRUE;

	m_nColWidth[ FILE_OPEN_NAME ] = 123;
	m_nColWidth[ FILE_OPEN_TYPE ] = 49;
	m_nColWidth[ FILE_OPEN_DATE ] = 104;
	m_nColWidth[ FILE_OPEN_PATH ] = 152;
	m_nColWidth[ FILE_OPEN_SIZE ] = 61;

	m_bRecursiveDir = g_config.GetFileOpenRecursive();
	m_bKeepDirLayout= g_config.GetKeepDirLayout();

	//{{AFX_DATA_INIT(COpenFileDlg)
	m_bDeleteOriginal = FALSE;
	m_bNormalize = FALSE;
	m_bRiffWav = FALSE;
	//}}AFX_DATA_INIT
}


void COpenFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CUString strLang;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenFileDlg)
	DDX_Control(pDX, IDC_LIST_REQUESTED_FILES, m_ctrlRequestedFiles);
	DDX_Check(pDX, IDC_RECURSIVE_DIR, m_bRecursiveDir);
	DDX_Check(pDX, IDC_DELETE_ORIGINAL, m_bDeleteOriginal);
	DDX_Check(pDX, IDC_NORMALIZE, m_bNormalize);
	DDX_Check(pDX, IDC_RIFF_WAV, m_bRiffWav);
	DDX_Check(pDX, IDC_KEEPDIRLAYOUT, m_bKeepDirLayout);
	//}}AFX_DATA_MAP

	strLang = g_language.GetString( IDS_PLEASE_SELECTFOLDER );
    CUStringConvert strCnv;
    DDX_GetFolderControl(pDX, IDC_EDIT_DIR,m_EditDir, 0, strCnv.ToT( strLang ) );

}


BEGIN_MESSAGE_MAP(COpenFileDlg, CDialog)
	//{{AFX_MSG_MAP(COpenFileDlg)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_REQUESTED_FILES, OnColumnclickListRequestedFiles)
	ON_EN_CHANGE(IDC_EDIT_DIR, OnChangeEditDir)
	ON_BN_CLICKED(IDC_SELECT_ALL, OnSelectAll)
	ON_BN_CLICKED(IDC_RECURSIVE_DIR, OnRecursiveDir)
	ON_BN_CLICKED(IDC_FILE_OPEN, OnOpen)
	ON_BN_CLICKED(IDC_SETTINGS, OnSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenFileDlg message handlers

BOOL COpenFileDlg::CompareExt(const CUString& strExt)
{
	unsigned int n;

	for (n=0;n<m_vStrExt.size();n++)
	{
		if (m_vStrExt[n] == strExt)
			return TRUE;
	}

	return FALSE;
}

void COpenFileDlg::MakeExtString()
{
	int nPos=0;
	int n=0;
	CUString strTmp(m_strFileType);

	// compare with caps
	strTmp.MakeUpper();

	// clean the array
	m_vStrExt.clear();

	// Divvy up the file name extention string into seperate strings
	while ( ( nPos = strTmp.Find( _T( ';' ) ) ) > 0 )
	{
		// add string to array
		CUString strNewExt = strTmp.Left( nPos );

		m_vStrExt.push_back( strNewExt );

		// trim of extention, including ';' separator
		strTmp = strTmp.Right( strTmp.GetLength() - nPos - 1 );
	}

	// add the last one
	m_vStrExt.push_back( strTmp );
}


void COpenFileDlg::AddRecursiveFiles( const CUString& strDir,int nItem )
{
	CFileFind	fileFind;
	CUString		pathAndFileType = strDir + CUString( _W( "\\*.*" ));
	BOOL		bStop=FALSE;

    CUStringConvert strCnv;

	// Iterate through the items in the listbox and populate the listconrol.
	if (fileFind.FindFile( strCnv.ToT( pathAndFileType ) ) )
	{
		do
		{
			bStop=(fileFind.FindNextFile()==0);
			CUString strFileName = CUString( fileFind.GetFileName() );
			CUString strFileRoot = CUString( fileFind.GetRoot() );
			CUString strFileType;

			if (	TRUE == fileFind.IsDirectory() && 
					FALSE == fileFind.IsDots() &&
					TRUE == m_bRecursiveDir )
			{
				AddRecursiveFiles( strDir + CUString( _W( "\\" ) ) + strFileName, nItem );
			}

			if (fileFind.IsDirectory()==FALSE && fileFind.IsDots()==FALSE)
			{
				int nPos=strFileName.ReverseFind( _T( '.' ) );
				

				if (nPos>0)
				{
					CUString strExt;
					strExt = strFileName.Right(strFileName.GetLength()-nPos-1);

					strFileType = strExt;

					strExt.MakeUpper();

					if ( CompareExt( strExt ) == TRUE )
					{
						CUString strFileSize;
						CUString strFileDate;
						CTime	fileTime;

						// Get the data/time stamp of this file
						fileFind.GetLastWriteTime( fileTime );

						// Format date time string
						strFileDate.Format( _W( "%4d/%02d/%02d %02d:%02d" ), fileTime.GetYear(), fileTime.GetMonth(), fileTime.GetDay(), fileTime.GetHour(), fileTime.GetMinute() );

						strFileSize.Format( _W( "%10.2f" ), fileFind.GetLength() / ( 1024.0 * 1024.0 ) );

                        CUStringConvert strCnv;

						m_ctrlRequestedFiles.InsertItem( nItem, strCnv.ToT( strFileName ));

						m_ctrlRequestedFiles.SetItemText( nItem, FILE_OPEN_TYPE, strCnv.ToT( strFileType ) );

						m_ctrlRequestedFiles.SetItemText( nItem, FILE_OPEN_DATE ,strCnv.ToT( strFileDate ) );

						m_ctrlRequestedFiles.SetItemText( nItem, FILE_OPEN_PATH, strCnv.ToT( strFileRoot ) );

						m_ctrlRequestedFiles.SetItemText( nItem, FILE_OPEN_SIZE, strCnv.ToT( strFileSize ) );

						m_ctrlRequestedFiles.SetItemData( nItem, (DWORD)fileTime.GetTime() );

						nItem++;
					}
				}
			}
		} while (bStop!=TRUE);
	}	


	m_bSortAscending=TRUE;
}


BOOL COpenFileDlg::FillFileList() 
{
    CUStringConvert strCnv;

	// clear list
	m_ctrlRequestedFiles.DeleteAllItems();

	CFileFind fileFind;

	if ((m_strDir[m_strDir.GetLength()-1]=='\\') )
		m_strDir=m_strDir.Left(m_strDir.GetLength()-1);

	// If directory does not exist, do not waste time
	if (!fileFind.FindFile( strCnv.ToT( m_strDir )) && (m_strDir.GetLength()>3) )
	{
		m_strDir=g_config.GetAppPath();
	}

	// create the extions strings
	MakeExtString();

	// do a recursive search for all the files with the proper extention
	AddRecursiveFiles( m_strDir, 0 );

	CSortClass csc( &m_ctrlRequestedFiles, m_nSortedColumn );
	ReSortColumns( );

	// Select All Items
	m_ctrlRequestedFiles.SelectAll();

	return TRUE;
}


BOOL COpenFileDlg::OnInitDialog() 
{
	CUString strLang;
	CUString strWndTitle;

	CDialog::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );


	if (!m_bShowNorm)
		GetDlgItem(IDC_NORMALIZE)->ShowWindow(SW_HIDE);
	if (!m_bShowDelOrig)
		GetDlgItem(IDC_DELETE_ORIGINAL)->ShowWindow(SW_HIDE);
	if (!m_bShowRiffWav)
		GetDlgItem(IDC_RIFF_WAV)->ShowWindow(SW_HIDE);
	if (!m_bShowKeepDirLayout)
		GetDlgItem(IDC_KEEPDIRLAYOUT)->ShowWindow(SW_HIDE);

	if ( ! m_strOpenText.IsEmpty() )
	{
        CUStringConvert strCnv;
        GetDlgItem(IDC_FILE_OPEN)->SetWindowText( strCnv.ToT( m_strOpenText ) );
	}
	
	if ( !m_bSettingsDlg )
	{
		GetDlgItem(IDC_SETTINGS)->ShowWindow( SW_HIDE );
	}
    CUStringConvert strCnv;

	if (!m_bIsOpenFileDlg)
	{

		strWndTitle = g_language.GetString( IDS_SAVE_FILE_AS );

        SetWindowText( strCnv.ToT( strWndTitle ) );

		strLang = g_language.GetString( IDS_OPEN_SAVEBUTTON );

		SetDlgItemText(IDOK, strCnv.ToT( strLang ) );
	}
	else
	{
		strWndTitle= g_language.GetString( IDS_OPEN_FILE );
		SetWindowText( strCnv.ToT( strWndTitle ) );
	}


	m_nColWidth[ FILE_OPEN_NAME ] = g_config.GetIni().GetValue( _T( "FileOpenDlg" ), _T( "CLW_0" ), m_nColWidth[ FILE_OPEN_NAME ] );
	m_nColWidth[ FILE_OPEN_TYPE ] = g_config.GetIni().GetValue( _T( "FileOpenDlg" ), _T( "CLW_1" ), m_nColWidth[ FILE_OPEN_TYPE ] );
	m_nColWidth[ FILE_OPEN_DATE ] = g_config.GetIni().GetValue( _T( "FileOpenDlg" ), _T( "CLW_2" ), m_nColWidth[ FILE_OPEN_DATE ] );
	m_nColWidth[ FILE_OPEN_PATH ] = g_config.GetIni().GetValue( _T( "FileOpenDlg" ), _T( "CLW_3" ), m_nColWidth[ FILE_OPEN_PATH ] );
	m_nColWidth[ FILE_OPEN_SIZE ] = g_config.GetIni().GetValue( _T( "FileOpenDlg" ), _T( "CLW_4" ), m_nColWidth[ FILE_OPEN_SIZE ] );

	m_nSortedColumn = g_config.GetIni().GetValue( _T( "FileOpenDlg" ), _T( "CLW_S" ), m_nSortedColumn );

	// Create columns of track list
	strLang = g_language.GetString( IDS_OPEN_FILENAME );

    m_ctrlRequestedFiles.InsertColumn( FILE_OPEN_NAME, strCnv.ToT( strLang ), LVCFMT_LEFT, m_nColWidth[ FILE_OPEN_NAME ], -1 );

	strLang = g_language.GetString( IDS_OPEN_TYPE );
	m_ctrlRequestedFiles.InsertColumn( FILE_OPEN_TYPE, strCnv.ToT( strLang ), LVCFMT_LEFT, m_nColWidth[ FILE_OPEN_TYPE ], -1 );

	strLang = g_language.GetString( IDS_OPEN_DATE );
	m_ctrlRequestedFiles.InsertColumn( FILE_OPEN_DATE, strCnv.ToT( strLang ), LVCFMT_LEFT, m_nColWidth[ FILE_OPEN_DATE ], -1 );

	strLang = g_language.GetString( IDS_OPEN_PATH );
	m_ctrlRequestedFiles.InsertColumn( FILE_OPEN_PATH, strCnv.ToT( strLang ), LVCFMT_LEFT, m_nColWidth[ FILE_OPEN_PATH ], -1 );

	strLang = g_language.GetString( IDS_OPEN_SIZEINMB );
	m_ctrlRequestedFiles.InsertColumn( FILE_OPEN_SIZE, strCnv.ToT( strLang ) , LVCFMT_RIGHT, m_nColWidth[ FILE_OPEN_SIZE ], -1 );

	m_nSortedColumn = g_config.GetIni().GetValue( _T( "FileOpenDlg" ), _T( "CLW_S" ), 0 );

	// Fill the list with the content of the current directory
	FillFileList();

	m_EditDir.SetWindowText( strCnv.ToT( m_strDir ) );

	m_bInit=TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpenFileDlg::OnOK() 
{

	UpdateData( TRUE );

	m_strSelectedFiles = NULL;
	m_nSelectedFiles = 0;

	int index = m_ctrlRequestedFiles.GetNextItem( -1, LVNI_SELECTED );

	while ( index >= 0 )
	{
		m_nSelectedFiles++;
		index = m_ctrlRequestedFiles.GetNextItem( index, LVNI_SELECTED );
	}

	m_strSelectedFiles=new CUString[ m_nSelectedFiles ];

	index = m_ctrlRequestedFiles.GetNextItem( -1, LVNI_SELECTED );
	m_nSelectedFiles = 0;

	while ( index >= 0 )
	{
		m_strSelectedFiles[ m_nSelectedFiles++ ] = CUString( m_ctrlRequestedFiles.GetItemText( index, FILE_OPEN_PATH ) + _T( "\\" ) + m_ctrlRequestedFiles.GetItemText( index, FILE_OPEN_NAME ) );
		index=m_ctrlRequestedFiles.GetNextItem( index, LVNI_SELECTED );
	}

	g_config.SetFileOpenRecursive( m_bRecursiveDir );
	g_config.SetKeepDirLayout( m_bKeepDirLayout );

	CDialog::OnOK();
}


CUString COpenFileDlg::GetFilePath()
{
	return m_strDir + _W( "\\" ) + m_strFileName;
}


void COpenFileDlg::ReSortColumns( ) 
{
	CSortClass csc(&m_ctrlRequestedFiles, m_nSortedColumn);

	CSortClass::EDataType eSortType( CSortClass::dtSTRING );

	switch  ( m_nSortedColumn )
	{
		case FILE_OPEN_NAME: eSortType = CSortClass::dtSTRING; break;
		case FILE_OPEN_TYPE: eSortType = CSortClass::dtSTRING; break;
		case FILE_OPEN_DATE: eSortType = CSortClass::dtDATETIME; break;
		case FILE_OPEN_PATH: eSortType = CSortClass::dtSTRING; break;
		case FILE_OPEN_SIZE: eSortType = CSortClass::dtINT; break;
		default:
			ASSERT( FALSE );
	}

	csc.Sort( m_bSortAscending, eSortType );
}

void COpenFileDlg::OnColumnclickListRequestedFiles(NMHDR* pNMHDR, LRESULT* pResult) 
{

	HD_NOTIFY *phdn = (HD_NOTIFY *) pNMHDR;
	
	if( phdn->iButton >= 0 )
	{
		SaveSettings();

		// User clicked on header using left mouse button
		if( phdn->iButton == m_nSortedColumn )
		{
			m_bSortAscending = !m_bSortAscending;
		}		
		else
		{
			m_bSortAscending = TRUE;
		}
		
		m_nSortedColumn = phdn->iButton;
		
		ReSortColumns();
	}

	*pResult = 0;
}

void COpenFileDlg::OnChangeEditDir() 
{
	if (m_bInit)
	{
        CString strTmp;
		m_EditDir.GetWindowText( strTmp );
        m_strDir = CUString( strTmp );
		FillFileList();

		// Update controls
		UpdateData(FALSE);
	}
}

void COpenFileDlg::OnSelectAll() 
{
	m_ctrlRequestedFiles.SelectAll();
	m_ctrlRequestedFiles.SetFocus();
}

//void COpenFileDlg::OnSelectNone() 
//{
//	m_ctrlRequestedFiles.SelectNone();
//}


void COpenFileDlg::OnRecursiveDir() 
{
	UpdateData(TRUE);
	FillFileList();
}



POSITION COpenFileDlg::GetStartPosition( ) const
{
	if (m_nSelectedFiles==0)
		return (POSITION)NULL;
	else
		return (POSITION)1;
}


CUString COpenFileDlg::GetNextPathName( POSITION& pos ) 
{
	CUString strRet;

	if ((int)pos>0 && (int)pos<=m_nSelectedFiles)
	{
		strRet=m_strSelectedFiles[(int)pos-1];
	}

	pos=(POSITION)((int)pos+1);

	if ((int)pos<0 || (int)pos> m_nSelectedFiles)
	{
		pos=(POSITION)NULL;
	}

	return strRet;
}

CUString COpenFileDlg::GetFileName( POSITION pos ) 
{
	CUString strRet = GetNextPathName( pos );

	int nPos = strRet.ReverseFind('\\');

	if ( nPos >= 0 )
	{
		strRet = strRet.Mid( nPos + 1 );
	}


	return strRet;
}

CUString COpenFileDlg::GetSubPath( POSITION pos ) 
{

	if ( !m_bKeepDirLayout )
	{
		return _T( "" );
	}

	CUString strRet = GetNextPathName( pos );

	int nStartPos = strRet.Find( m_strDir );
	int nLength = strRet.ReverseFind( _T( '\\' ) ) + 1;

	if ( nStartPos >= 0 )
	{
		nStartPos += 1 + m_strDir.GetLength();
	}

	nLength -= nStartPos;

	if ( nLength < 0 )
	{
		nLength = 0;
	}

	strRet = strRet.Mid( nStartPos, nLength );

	return strRet;
}


void COpenFileDlg::SaveSettings() 
{
	g_config.GetIni().SetValue( _T( "FileOpenDlg" ), _T( "CLW_0" ), m_ctrlRequestedFiles.GetColumnWidth( FILE_OPEN_NAME ) );
	g_config.GetIni().SetValue( _T( "FileOpenDlg" ), _T( "CLW_1" ), m_ctrlRequestedFiles.GetColumnWidth( FILE_OPEN_TYPE ) );
	g_config.GetIni().SetValue( _T( "FileOpenDlg" ), _T( "CLW_2" ), m_ctrlRequestedFiles.GetColumnWidth( FILE_OPEN_DATE ) );
	g_config.GetIni().SetValue( _T( "FileOpenDlg" ), _T( "CLW_3" ), m_ctrlRequestedFiles.GetColumnWidth( FILE_OPEN_PATH ) );
	g_config.GetIni().SetValue( _T( "FileOpenDlg" ), _T( "CLW_4" ), m_ctrlRequestedFiles.GetColumnWidth( FILE_OPEN_SIZE ) );
	g_config.GetIni().SetValue( _T( "FileOpenDlg" ), _T( "CLW_S" ), m_nSortedColumn );
}

void COpenFileDlg::OnOpen() 
{
	SaveSettings();

	OnOK();
}

BOOL COpenFileDlg::PreTranslateMessage(MSG* pMsg) 
{
	return ::IsDialogMessage( m_hWnd, pMsg );
}


void COpenFileDlg::OnSettings() 
{
}
