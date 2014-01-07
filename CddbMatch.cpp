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


#include "stdafx.h"
#include "Cdex.h"
#include "CddbMatch.h"
#include "Util.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCddbMatch dialog


CCddbMatch::CCddbMatch(CWnd* pParent /*=NULL*/)
	: CDialog(CCddbMatch::IDD, pParent)
{
	m_nMatch = 0;
	m_nMatches = 0;

	//{{AFX_DATA_INIT(CCddbMatch)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CCddbMatch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCddbMatch)
	DDX_Control(pDX, IDOK, m_OK);
	DDX_Control(pDX, IDC_MATCHLIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCddbMatch, CDialog)
	//{{AFX_MSG_MAP(CCddbMatch)
	ON_NOTIFY(NM_CLICK, IDC_MATCHLIST, OnClickMatchlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCddbMatch message handlers

void CCddbMatch::AddMatch(CUString strMatches)
{
	int nPrevPos=0;
	int	nPos=0;

	if ( m_nMatches < MAX_MATCHES )
	{
		m_strMatch[ m_nMatches ] = strMatches;

		nPos=strMatches.Find( _T( ' ' ) );
		if (nPos>=0)
		{
			m_strArrayCat[ m_nMatches ] = strMatches.Mid( nPrevPos, nPos-nPrevPos );
			strMatches=strMatches.Right(strMatches.GetLength()-nPos-1);

			nPos=strMatches.Find( _T( ' ' ) );
			if (nPos>=0)
			{
				m_strArrayDid[ m_nMatches ]=strMatches.Mid(nPrevPos,nPos-nPrevPos);
				m_strArrayTit[ m_nMatches ]=strMatches.Right(strMatches.GetLength()-nPos-1);

				m_nMatches++;
			}
		}
	}
}


BOOL CCddbMatch::OnInitDialog() 
{
	CUString		strLang;
	CRect		rcList;

	CDialog::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// Get size position of Track List
	m_List.GetWindowRect(rcList);

	// Convert from screen to client (this parent window of course)
	ScreenToClient(rcList);
	int iWidth=rcList.Width();

    CUStringConvert strCnv; 

	// Insert header columns
    strLang = g_language.GetString(IDS_GENRE);
	m_List.InsertColumn(0,strCnv.ToT( strLang ),LVCFMT_LEFT,iWidth/4);
    strLang = g_language.GetString(IDS_ALBUM);
	m_List.InsertColumn(1,strCnv.ToT( strLang ),LVCFMT_LEFT,iWidth/2);
    strLang = g_language.GetString(IDS_CDDBID);
	m_List.InsertColumn(2,strCnv.ToT( strLang ),LVCFMT_RIGHT,iWidth/5);

	for (int i=0;i<m_nMatches;i++)
	{
        CUStringConvert strCnv;
        
		// Add matches to the dialog
		m_List.InsertItem( i, strCnv.ToT( m_strArrayCat[ i ] ) );
		m_List.SetItemText( i, 1, strCnv.ToT( m_strArrayTit[ i ] ) );
		m_List.SetItemText( i, 2, strCnv.ToT( m_strArrayDid[ i ] ) );
	}

	m_List.SetFocus();

	// By default, select the first one
	if ( m_nMatches > 0 )
	{
		// Enable the OK button
		m_OK.EnableWindow( TRUE );

		// Select first entry in list
		m_List.SetCurSel( 0 );
	}
	else
	{
		// Do not enable the OK button, wait for User to select a match
		m_OK.EnableWindow( FALSE );
	}

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCddbMatch::OnOK() 
{
	// Get the item that's selected
	m_nMatch = m_List.GetCurSel();

	// Check range of selection to avoid GPF
	if ( m_nMatch < 0 || m_nMatch > m_nMatches )
	{
		m_nMatch = 0;
	}

	CDialog::OnOK();
}

void CCddbMatch::OnCancel() 
{
	m_nMatch = 0;

	CDialog::OnCancel();
}


void CCddbMatch::OnClickMatchlist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// User has selected a match, enable OK button
	m_OK.EnableWindow(TRUE);
	*pResult = 0;
}
