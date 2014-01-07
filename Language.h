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


#ifndef LANGUAGE_H_INCLUDED
#define LANGUAGE_H_INCLUDED

#include <vector>
using namespace std;


class CLangElement
{
public:
	int		nID;
	CUString	strValue;
};


class CLanguage : public CObject  
{
public:
	/**
	 * Language constructor 
	 */
	CLanguage( );

	/**
	 * Language destructor 
	 */
	virtual ~CLanguage();

	BOOL	SetLanguage( const CUString& strLanguage );

	CUString	GetLanguageString( const DWORD dwIndex ) const;
	DWORD	GetNumLanguageStrings() const { return m_vLangFiles.size(); }

	void Init( const CUString& strLanguageDir, const CUString& strLanguage );
//	BOOL SetLanguage( const int nLanguageIdx );

	CUString	GetMainMenuItem( const DWORD nID, const DWORD nItem );
	CUString	GetSubMenuItem( const DWORD nID );
	CUString	GetDialogString( const DWORD nID );
	CUString	GetString( const DWORD nID ) const;

	BOOL TranslateMenu( CMenu* pMenu, const int nMenuID, BOOL bTopLevel = TRUE );
	BOOL InitDialogStrings( CDialog* pDialog, long lSection );

	CUString	GetLanguage() const;

	DWORD	GetCodePage() const {return m_dwCodePageID;}
	CUString GetRevisionLevel( const CUString& strFileName );
	
private:
	void SearchLanguageFiles( );
	void ParseLanguageFile( const CUString& strFileName );
	void TranslateTab( CUString& strModify );
	void ReportMissingID( int nID, const CUString& strEnglish, int nType ) const;
    void ReportChangedID( int nID, const CUString& strResource, const CUString& strEnglish, int nType ) const;
    void EncodeTab( CUString& strModify ) const;

	void SetFont( CWnd* pWnd );

	CUString					m_strCurrentLanguage;
	vector<CUString>			m_vLangFiles;
	vector<CLangElement>	m_vMenus;
	vector<CLangElement>	m_vStrings;
	vector<CLangElement>	m_vDialogs;
	CUString					m_strLanguageDir;
	DWORD					m_dwCodePageID;
	DWORD					m_dwSubLangID;
	DWORD					m_dwLangID;

	static const CUString	DEFAULT_LANG_SELECTION;
	static const CUString    CODEPAGETAG;
	static const CUString    LANGTAG;
	static const CUString    SUBLANGTAG;
	static const CUString    REVISIONTAG;
	static const CUString	m_strLangFileExt;
};

#endif

