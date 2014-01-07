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


#ifndef CONFIGDLG_INCLUDED
#define CONFIGDLG_INCLUDED

#include "Config.h"
#include "Resource.h"

#include "CDPropPage.h"
#include "LocalCDDBPropPage.h"
#include "RemoteCDDBPropPage.h"
#include "GenericPropPage.h"
#include "FilenamePropPage.h"
#include "EncoderPropPage.h"
#include "TagPropPage.h"

#include "TreePropSheet.h"

using namespace TreePropSheet;

// ConfigSheet

class ConfigSheet : public CTreePropSheet
{
	DECLARE_DYNAMIC(ConfigSheet)

private:
	BOOL				m_bIsInSync;
// Construction
public:
	CDPropPage			m_CDPropPage;
	CEncoderPropPage	m_EncoderPropPage;
	CGenericPropPage	m_GenericPropPage;
	CRemoteCDDBPropPage	m_RemoteCDDBPropPage;
	CFilenamePropPage	m_FilenamePropPage;
	CLocalCDDBPropPage	m_LocalCDDBPropPage;
	CTagPropPage	    m_TagPropPage;
	// CONSTRUCTORS
	ConfigSheet( int nSelectTab,CWnd* pParentWnd=NULL);

	// DESTRUCTOR
	virtual ~ConfigSheet();

	// MUTATORS
	virtual afx_msg BOOL OnApply();

protected:
	// And hook in the message map
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.







#endif



