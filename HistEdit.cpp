/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
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
#include "HistEdit.h"
#include "Util.h"

static const int DEF_NUM_HIST_LINES=250;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CHistEdit::CHistEdit()
	:CEdit()
{
	m_nLines=0;
	m_nHistLines=DEF_NUM_HIST_LINES;
}

CHistEdit::~CHistEdit()
{
}

void CHistEdit::AddString(const CUString& strAdd, bool bAddReturn)
{
	m_nLines++;

	if (GetLineCount()>m_nHistLines)
	{
		int nPos=m_strHist.Find(_W("\n"));
		if (nPos>=0)
		{
			m_strHist=m_strHist.Right(m_strHist.GetLength()-nPos-2);
		}
	}

	// Add the new string

	CUString strNew;

	strNew.Format(_W("%s%s"), (LPCWSTR)m_strHist, (LPCWSTR)strAdd );
//	m_strHist= m_strHist + strAdd;

	m_strHist = strNew;


	// Add return if this is not the first string
	if (!m_strHist.IsEmpty() )
	{
		if ( bAddReturn )
			m_strHist+=_W( "\r\n" );
	}

    CUStringConvert strCnv;

	// Put the string into CEdit control
	SetWindowText( strCnv.ToT( m_strHist ) );

	// Scroll the edit control
	LineScroll( GetLineCount()-1, 0 );

}


void CHistEdit::SetLogging( const CUString& strFileName, bool bEnable )
{
	m_strLogFileName= strFileName;
	m_bLogEnabled= bEnable;
}