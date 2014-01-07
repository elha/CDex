/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 - 2007 Albert L. Faber
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
#include "cdex.h"
#include "GenreEditDlg.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CGenreEditDlg dialog


CGenreEditDlg::CGenreEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGenreEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenreEditDlg)
	m_strGenre = _T("");
	m_nID3V1ID = 0;
	m_nLastSelection = -1;
	m_nInsertPosition = 0;
	//}}AFX_DATA_INIT
}


void CGenreEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenreEditDlg)
	DDX_Control(pDX, IDC_SPIN1, m_Spin);
	DDX_Control(pDX, IDC_EDIT_CDDBGENRE, m_CDDBGenre);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Text(pDX, IDC_EDIT_GENRE, m_strGenre);
	DDX_Text(pDX, IDC_EDIT_ID3V1ID, m_nID3V1ID);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenreEditDlg, CDialog)
	//{{AFX_MSG_MAP(CGenreEditDlg)
	ON_BN_CLICKED(IDC_INSERTNEW, OnInsertnew)
	ON_NOTIFY(NM_CLICK, IDC_LIST, OnClickList)
	ON_NOTIFY(LVN_KEYDOWN, IDC_LIST, OnKeydownList)
	ON_CBN_SELCHANGE(IDC_EDIT_CDDBGENRE, OnSelchangeEditCddbgenre)
	ON_EN_CHANGE(IDC_EDIT_ID3V1ID, OnChangeEditId3v1id)
	ON_EN_CHANGE(IDC_EDIT_GENRE, OnChangeEditGenre)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_UPDATE_GENRE_LIST, OnUpdateGenreList )

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenreEditDlg message handlers

void CGenreEditDlg::OnOK() 
{
	int i;

	g_GenreTable.DeleteAll();
	g_GenreTable.AddDefaults();

	for ( i =0; i< m_nInsertPosition; i++ )
	{
		CUString strCDDBGenre;

		m_strGenre = CUString( m_List.GetItemText( i, 0 ) );
		strCDDBGenre = CUString( m_List.GetItemText( i, 1 ) );
		m_nID3V1ID = _ttoi( m_List.GetItemText( i, 2 ) );

		if ( m_nID3V1ID <= 0  ) m_nID3V1ID = 0;
		if ( m_nID3V1ID > 255 ) m_nID3V1ID = 255;

		g_GenreTable.AddEntry( m_nID3V1ID, CUString( m_strGenre ), strCDDBGenre, true );
	}

	g_GenreTable.Sort();
	g_GenreTable.Save( GENRE_TABLE_FILE_NAME );

	CDialog::OnOK();
}

void CGenreEditDlg::OnInsertnew() 
{
	CUString strLang;
    CUStringConvert strCnv;

	UpdateList( m_nLastSelection );

	int i = m_nInsertPosition;

	CUString strNmbr;

	strLang = g_language.GetString( IDS_NEW_GENRE_NAME );

	m_List.InsertItem( i, strCnv.ToT( strLang ) );

    m_List.SetItemText( i, 1, strCnv.ToT( g_GenreTable.GetCDDBGenre( 0 ) ) );

	strNmbr.Format( _W( "%d" ) , 255 );

    m_List.SetItemText( i, 2, strCnv.ToT( strNmbr ) );

	m_List.SetCurSel( i );
	SetControls( i );

	m_nInsertPosition++;

}

void CGenreEditDlg::SetControls( int nSelection )
{
	int nNumber = 0;

	m_strGenre = m_List.GetItemText( nSelection, 0 );
	m_CDDBGenre.SelectString( -1, m_List.GetItemText( nSelection, 1 ) );
	m_nID3V1ID = _ttoi( m_List.GetItemText( nSelection, 2 ) );

	m_nLastSelection = nSelection;

	UpdateData( FALSE );

}


void CGenreEditDlg::UpdateList( int nSelection )
{
	if ( nSelection >= 0 )
	{
		CString strTmp;

		UpdateData( TRUE );

		m_List.SetItemText( nSelection, 0, m_strGenre );

		int nSel = m_CDDBGenre.GetCurSel();

		if ( CB_ERR != nSel )
		{
			m_CDDBGenre.GetLBText( nSel, strTmp );
		}
		else
		{
			ASSERT( FALSE );
		}

		m_List.SetItemText( nSelection, 1, strTmp );

		strTmp.Format( _T( "%d" ) , m_nID3V1ID );

		m_List.SetItemText( nSelection, 2, strTmp );
	}	
}


BOOL CGenreEditDlg::OnInitDialog() 
{
	CUString strLang;

	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

    CUStringConvert strCnv;

	strLang = g_language.GetString( IDS_GENRE_NAME );
	m_List.InsertColumn( 0, strCnv.ToT( strLang ), LVCFMT_LEFT, 130, 0 );
	strLang = g_language.GetString( IDS_GENRE_CDDB_NAME );
	m_List.InsertColumn( 1, strCnv.ToT( strLang ), LVCFMT_LEFT, 100, 0 );
	strLang = g_language.GetString( IDS_GENRE_ID3V1ID );
	m_List.InsertColumn( 2, strCnv.ToT( strLang ), LVCFMT_LEFT, 70, 0 );

	m_nInsertPosition = 0;
	
	// Fill Category
	for (WORD i=0; i < g_GenreTable.GetNumGenres(); i++)
	{
		if ( g_GenreTable.CanBeModified( i ) )
		{
			CUString strNmbr;
			strNmbr.Format( _W( "%d" ) , g_GenreTable.GetID3V1ID( i) );
             
            m_List.InsertItem( m_nInsertPosition, strCnv.ToT( g_GenreTable.GetGenre( i ) ) );
			m_List.SetItemText( m_nInsertPosition, 1, strCnv.ToT( g_GenreTable.GetCDDBGenre( i ) ) );
			m_List.SetItemText( m_nInsertPosition, 2, strCnv.ToT( strNmbr ) );
			m_nInsertPosition++;
		}
	}

	m_Spin.SetRange( 0, 255 );

	m_List.SetCurSel( 0 );

	SetControls( 0 );

	return TRUE;
}


void CGenreEditDlg::OnClickList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	PostMessage( WM_UPDATE_GENRE_LIST, 0, 0 );

	*pResult = 0;
}



LONG CGenreEditDlg::OnUpdateGenreList( WPARAM wParam, LPARAM lParam)
{
//		UpdateList( m_nLastSelection );
	int nNewSel = m_List.GetCurSel();
	SetControls( nNewSel );
	return 0;
}

void CGenreEditDlg::OnKeydownList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;

	PostMessage( WM_UPDATE_GENRE_LIST, 0, 0 );

	*pResult = 0;
}

void CGenreEditDlg::OnSelchangeEditCddbgenre() 
{
	UpdateList( m_nLastSelection );
}

void CGenreEditDlg::OnChangeEditId3v1id() 
{
	UpdateList( m_nLastSelection );
}

void CGenreEditDlg::OnChangeEditGenre() 
{
	UpdateList( m_nLastSelection );
}

void CGenreEditDlg::OnCancel() 
{
	CDialog::OnCancel();
}
