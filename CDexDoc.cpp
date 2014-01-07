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
#include "CDex.h"

#include "CDexDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "CDexView" ) );


/////////////////////////////////////////////////////////////////////////////
// CCDexDoc

IMPLEMENT_DYNCREATE(CCDexDoc, CDocument)

BEGIN_MESSAGE_MAP(CCDexDoc, CDocument)
	//{{AFX_MSG_MAP(CCDexDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCDexDoc construction/destruction

CCDexDoc::CCDexDoc()
{
	ENTRY_TRACE( _T( "CCDexDoc::CCDexDoc()" ) );
	m_bSaveToIni=FALSE;
	EXIT_TRACE( _T( "CCDexDoc::CCDexDoc()" ) );
}

CCDexDoc::~CCDexDoc()
{
	ENTRY_TRACE( _T( "CCDexDoc::~CCDexDoc()" ) );
	EXIT_TRACE( _T( "CCDexDoc::~CCDexDoc()" ) );
}

BOOL CCDexDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CCDexDoc serialization

void CCDexDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CCDexDoc diagnostics

#ifdef _DEBUG
void CCDexDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CCDexDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CCDexDoc commands


void CCDexDoc::InitCDRom()
{
}

void CCDexDoc::ReadCDInfo()
{
	ENTRY_TRACE( _T( "CCDexDoc::ReadCDInfo()" ) );

	// Read the table of contents
	m_CDInfo.ReadToc();

	// Get CDInfo
	m_CDInfo.ReadCDInfo();

	EXIT_TRACE( _T( "CCDexDoc::ReadCDInfo()" ) );

}

void CCDexDoc::OnCloseDocument() 
{
	ENTRY_TRACE( _T( "CCDexDoc::OnCloseDocument()" ) );

	UpdateAllViews(NULL,WM_CDEX_SAVE_CD_INFO,NULL);
	
	CDocument::OnCloseDocument();

	EXIT_TRACE( _T( "CCDexDoc::OnCloseDocument()" ) );
}

BOOL CCDexDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	return CDocument::OnSaveDocument(lpszPathName);
}

BOOL CCDexDoc::SaveModified() 
{
	return TRUE;
}
