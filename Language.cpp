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
#include "Language.h"
#include "resource.h"
#include "Util.h"
#include "TextFile.h"
#include <locale.h>
#include <algorithm>


// static int g_nReportMissing = QueryModuleDebugLevel( _T( "ReportMissingLangIDs" ) );
static int g_nReportMissing = TRUE;
// QueryModuleDebugLevel( _T( "ReportMissingLangIDs" ) );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const CUString CLanguage::m_strLangFileExt( _T( ".cdex.lang" ) );
const CUString CLanguage::DEFAULT_LANG_SELECTION( _T( "english" ) );
const CUString CLanguage::CODEPAGETAG( _T( "CODEPAGE=" ) );
const CUString CLanguage::LANGTAG( _T( "LANGUAGE=" ) );
const CUString CLanguage::SUBLANGTAG( _T( "SUBLANG=" ) );
const CUString CLanguage::REVISIONTAG( _T( "REVISION=" ) );

CLanguage::CLanguage()
{
	m_strCurrentLanguage	= DEFAULT_LANG_SELECTION;
	m_dwCodePageID			= 1252;		// STANDARD CODE PAGE ANSI INTERNATIONAL
	m_dwSubLangID			= SUBLANG_DEFAULT;
	m_dwLangID				= LANG_ENGLISH;
}

CLanguage::~CLanguage()
{
}


void CLanguage::ReportMissingID( int nID, const CUString& strEnglish, int nType ) const
{
	if ( g_nReportMissing )
	{
		BOOL		bAdd = TRUE;
		CUString    strWrite;
		CString		strRead;
		CStdioFile	cFile;
        CUStringConvert strCnv;

		switch ( nType )
		{
			case 0:
				strWrite.Format( _W( "#%04X# \"%s\"" ), nID, (LPCWSTR)strEnglish );
			break;
			case 1:
				strWrite.Format( _W( "#M%04X# \"%s\"" ), nID, (LPCWSTR)strEnglish );
			break;
			case 2:
				strWrite.Format( _W( "#D%08X# \"%s\"" ), nID, (LPCWSTR)strEnglish );
			break;
		}

        EncodeTab( strWrite );

		if ( cFile.Open(	_T( "c:\\temp\\missing.txt" ),
							CFile::modeReadWrite | CFile::modeNoTruncate | CFile::modeCreate ) )
		{
            while ( cFile.ReadString( strRead ) )
			{
				if ( strWrite.Find( CUString( strRead ) ) >=0 )
				{
					bAdd = FALSE;
					break;
				}
			}
		    if ( bAdd )
		    {
                cFile.WriteString( strCnv.ToT( strWrite + _W( "\n" ) ) );
		    }
    		cFile.Close();
		}
	}
}

void CLanguage::ReportChangedID( int nID, const CUString& strResource, const CUString& strEnglish, int nType ) const
{
	if ( g_nReportMissing )
	{
		BOOL		bAdd = TRUE;
		CUString    strWrite;
		CUString    strTmp;
		CString		strRead;
		CStdioFile	cFile;
        CUStringConvert strCnv;

		switch ( nType )
		{
			case 0:
				strWrite.Format( _W( "#%04X# \"%s\"" ), nID, (LPCWSTR)strEnglish );
				strTmp.Format( _W( "#    # \"%s\"" ), (LPCWSTR)strResource );
                strWrite += _W( "\n" ) + strTmp;
			break;
			case 1:
				strWrite.Format( _W( "#M%04X# \"%s\"" ), nID, (LPCWSTR)strEnglish );
				strTmp.Format( _W( "#M    # \"%s\"" ), (LPCWSTR)strResource );
                strWrite += _W( "\n" ) + strTmp;
			break;
			case 2:
				strWrite.Format( _W( "#D%08X# \"%s\"" ), nID, (LPCWSTR)strEnglish );
				strTmp.Format( _W( "#D        # \"%s\"" ), (LPCWSTR)strResource );
                strWrite += _W( "\n" ) + strTmp;
			break;
		}

        EncodeTab( strWrite );

		if ( cFile.Open(	_T( "c:\\temp\\missing.txt" ),
							CFile::modeReadWrite | CFile::modeNoTruncate | CFile::modeCreate ) )
		{
            while ( cFile.ReadString( strRead ) )
			{
				if ( strWrite.Find( CUString( strRead ) ) >=0 )
				{
					bAdd = FALSE;
					break;
				}
			}
		}

		if ( bAdd )
		{
            cFile.WriteString( strCnv.ToT( strWrite + _W( "\n" ) ) );
		}

		cFile.Close();
	}
}

CUString	CLanguage::GetMainMenuItem( const DWORD nID, const DWORD nItem )
{
	CUString strRet;

	strRet = GetSubMenuItem( nID );

	DWORD	dwIndex = 0;
	int		nStart  = 0;
	int		nStop   = strRet.GetLength();

	for ( dwIndex = 0; dwIndex < nItem; dwIndex++ )
	{
		nStart = strRet.Find( '#', nStart + 1 );

		if ( -1 == nStart )
		{
			break;
		}
	}

	if ( -1 != nStart )
	{
		nStop = strRet.Find( '#', nStart + 1 );

		if ( -1 == nStop )
		{
			nStop = strRet.GetLength();
		}
	}

	if ( nStop - nStart > 2 )
	{
		strRet = strRet.Mid( nStart + 1, nStop - nStart - 1 );
	}

	return strRet;
}

CUString	CLanguage::GetSubMenuItem( const DWORD nID )
{
	CUString strRet;
	DWORD	dwIndex;

	for ( dwIndex = 0; dwIndex < m_vMenus.size(); dwIndex++ )
	{
		if ( m_vMenus[ dwIndex ].nID == nID )
		{
			strRet = m_vMenus[ dwIndex ].strValue;
			break;
		}
	}

	return strRet;
}

CUString	CLanguage::GetDialogString( const DWORD nID )
{
	CUString strRet;

	DWORD	dwIndex;

	for ( dwIndex = 0; dwIndex < m_vDialogs.size(); dwIndex++ )
	{
		if ( m_vDialogs[ dwIndex ].nID == nID )
		{
			strRet = m_vDialogs[ dwIndex ].strValue;
			break;
		}
	}

	return strRet;
}

CUString	CLanguage::GetString( const DWORD nID ) const
{
	CUString strRet;

	DWORD	dwIndex;

	for ( dwIndex = 0; dwIndex < m_vStrings.size(); dwIndex++ )
	{
		if ( m_vStrings[ dwIndex ].nID == nID )
		{
			strRet = m_vStrings[ dwIndex ].strValue;
			break;
		}
	}
	if ( strRet.IsEmpty() )
	{
        CString strResource;
		strResource.LoadString( nID );
        strRet = CUString( strResource );
#ifdef _DEBUG
		ReportMissingID( nID, strRet, 0 );
#endif
	} 
    else
    {
#ifdef _DEBUG
        CString strResource;
	    strResource.LoadString( nID );
        
        if ( 0 != strRet.Compare( CUString( strResource ) ) )
        {
    		ReportChangedID( nID, CUString( strResource ), strRet, 0 );
        }
#endif
    }


	return strRet;
}

const int MENU_SEPARARTOR =	0;	// identifies the separator id
const int MENU_POPUP = -1;		// identifies the popup id
const int COMMON_IDMIN = 0;
const int COMMON_IDMAX = 99;

void CLanguage::TranslateTab( CUString& strModify )
{
	int nPos;
	
	while ( ( nPos = strModify.Find( _W( "\\t" ) ) ) >= 0 )
	{
		strModify = strModify.Left( nPos ) + _W( "\t" ) + strModify.Right( strModify.GetLength() - nPos -2 );
	}
	while ( ( nPos = strModify.Find( _W( "\\n" ) ) ) >= 0 )
	{
		strModify = strModify.Left( nPos ) + _W( "\n" ) + strModify.Right( strModify.GetLength() - nPos -2 );
	}
	while ( ( nPos = strModify.Find( _W( "\\r" ) ) ) >= 0 )
	{
		strModify = strModify.Left( nPos ) + _W( "\r" ) + strModify.Right( strModify.GetLength() - nPos -2 );
	}

}

void CLanguage::EncodeTab( CUString& strModify ) const
{
	int nPos;
	
	while ( ( nPos = strModify.Find( _W( "\t" ) ) ) >= 0 )
	{
		strModify = strModify.Left( nPos ) + _W( "\\t" ) + strModify.Right( strModify.GetLength() - nPos -1 );
	}
	while ( ( nPos = strModify.Find( _W( "\n" ) ) ) >= 0 )
	{
		strModify = strModify.Left( nPos ) + _W( "\\n" ) + strModify.Right( strModify.GetLength() - nPos -1 );
	}
	while ( ( nPos = strModify.Find( _W( "\r" ) ) ) >= 0 )
	{
		strModify = strModify.Left( nPos ) + _W( "\\r" ) + strModify.Right( strModify.GetLength() - nPos -1 );
	}
}

void CLanguage::ParseLanguageFile( const CUString& strFileName )
{
	BOOL bResult		= TRUE;
	DWORD dwRevision	= 0;
	FILE* pFile			= NULL;

    CUString strFullFileName = m_strLanguageDir + _W("\\lang\\") + strFileName + m_strLangFileExt;
    CUStringConvert strCnv;

	pFile = CDexOpenFile( strFullFileName, _W( "r" ) );

	if ( NULL != pFile )
	{
		CHAR lpszRead[ 1024 ];
		int nStart = 0;
		int nStop  = 0;

		CUString strRead;

		memset( lpszRead, 0, sizeof( lpszRead ) );

		while ( fgets( lpszRead, sizeof( lpszRead ), pFile  ) )
		{
			bool         bHasNumber = false;
			bool         bIsMenu    = false;
			bool         bIsDialog  = false;
			bool	     bHasString = false;
			DWORD        nIndex = 0;
			CLangElement newElement;

			strRead = _T( "" );

			for ( nIndex = 0; nIndex< strlen( lpszRead ); nIndex++ )
			{
				if (	( lpszRead[ nIndex ] != 0x0A ) &&
						( lpszRead[ nIndex ] != 0x0D ) )
				{
					strRead +=  (TCHAR)lpszRead[ nIndex ];
				}
			}

			TranslateTab( strRead );

			nStart = strRead.Find( _T( '#' ) );

			if ( nStart >= 0 )
			{
				if ( ( nStart + 1 ) == strRead.Find( _T( 'M' ) ) )
				{
					nStart++;
					bIsMenu = TRUE;
				}
				else
				if ( ( nStart + 1 ) == strRead.Find( _T( 'D' ) ) )
				{
					nStart++;
					bIsDialog = TRUE;
				}


				nStop = strRead.Find( _T( '#' ), nStart + 1 );

				if ( nStop > 2 )
				{
					CUString strID ( strRead.Mid( nStart + 1, nStop - nStart -1 ) );
					swscanf( strID, _W( "%x" ), &newElement.nID );
					bHasNumber = true;
				}

			}
			else
			{
				if ( strRead.Find( CODEPAGETAG ) == 0 )
				{
					m_dwCodePageID = _wtoi( strRead.Mid( CODEPAGETAG.GetLength() ) );
				}
				else if ( strRead.Find( LANGTAG ) == 0 )
				{
					m_dwLangID = _wtoi( strRead.Mid( LANGTAG.GetLength() ) );
				}
				else if ( strRead.Find( SUBLANGTAG ) == 0 )
				{
					m_dwSubLangID = _wtoi( strRead.Mid( SUBLANGTAG.GetLength() ) );
				}
				else if ( strRead.Find( REVISIONTAG ) == 0 )
				{
					int nPos;
					nPos = strRead.Find( _W( ",v " ) );
					if ( nPos >= 0 )
					{
						float fVersion = 1.0f;
						swscanf( strRead.Mid( nPos+3,4 ), _W( "%f" ), &fVersion );
						dwRevision =   (DWORD)( 100 * ( fVersion + 0.005 ) );
					}

				}
			}

			nStart = strRead.Find( '"' );

			if ( nStart >= 0 )
			{
				nStop = strRead.Find( '"', nStart + 1 );

				if ( nStop > 2 )
				{
					lpszRead[ 0 ] = '\0';

					for ( nIndex = nStart + 1; nIndex < (DWORD)nStop; nIndex++)
					{
						lpszRead[ nIndex - nStart - 1] = (CHAR)strRead.GetAt( nIndex );
						lpszRead[ nIndex - nStart ] = '\0';
					}

					newElement.strValue = CUString( lpszRead, m_dwCodePageID );
					bHasString = true;
				}
			}

			if ( bHasString && bHasNumber )
			{
				if ( bIsMenu )
				{
					m_vMenus.push_back( newElement );
				}
				else if ( bIsDialog )
				{
					m_vDialogs.push_back( newElement );
				}
				else
				{
					m_vStrings.push_back( newElement );
				}
			}
			memset( lpszRead, 0, sizeof( lpszRead ) );	
		}

		fclose( pFile );
	}


#ifdef UNICODE
	TCHAR* pTest = _wsetlocale( LC_ALL, GetString( 99 ) );
#else
	TCHAR* pTest = setlocale( LC_ALL, strCnv.ToT( GetString( 99 ) ) );

#endif
	SetThreadLocale( MAKELCID( m_dwLangID, m_dwSubLangID ) );
	setlocale( LC_NUMERIC, "English" );
}

CUString CLanguage::GetRevisionLevel( const CUString& strFileName )
{
	BOOL		bResult     = TRUE;
	CUString		strRet = _T( "0.0" );

    // AF TODO
    //CStdioFile	cLangFile;

	//bResult = cLangFile.Open(	m_strLanguageDir + _T("\\lang\\") + strFileName + m_strLangFileExt,
	//							CFile::modeRead );

	//if ( bResult )
	//{
	//	int nStart = 0;
	//	int nStop  = 0;

	//	CUString strRead;

	//	while ( cLangFile.ReadString( strRead ) )
	//	{
	//		TranslateTab( strRead );

	//		if ( 0 == strRead.Find( REVISIONTAG ) )
	//		{
	//			int nPos = strRead.Find( _T( ",v " ) );
	//			int nEndPos = strRead.Find( _T( " " ), nPos + 4 );

	//			if ( ( nPos >= 0 ) && ( nEndPos >= 0 ) )
	//			{
	//				strRet =strRead.Mid( nPos + 3, nEndPos - nPos - 3 );
	//				strRet.TrimLeft();
	//				strRet.TrimRight();
	//			}
	//			break;
	//		}	
	//	}
	//	cLangFile.Close();
	//}

	return strRet;;
}

void CLanguage::SearchLanguageFiles( )
{
	CUString	strSearchName;
	CFileFind	fileFind;
	BOOL		bFound = FALSE;

//    PVOID pTest = (PVOID)m_strLanguageDir;
//CString str;
//CString str1 =_T( "CSTRING1" );
//CString str2 =_T( "CSTRING2" );
//CUString str3( _W("CUSTRING" ));
//    str.Format(	_W( "%s" ),str1 );
//    str.Format(	_W( "%s" ),(LPCWSTR)str3 );
//    str.Format(	_W( "%s\\lang\\*%s" ),
//							str1,
//							str3);


//    CUString* p1 = &str3;
//    CString* p2 = &str1;

	// build the filename search string
    strSearchName.Format( _W( "%s\\lang\\*%s" ), (LPCWSTR)m_strLanguageDir, (LPCWSTR)m_strLangFileExt );

    CUStringConvert strCnv;

    bFound = fileFind.FindFile( strCnv.ToT( strSearchName ) );

	while ( bFound )
	{
		bFound = fileFind.FindNextFile();

		CUString strFileName = fileFind.GetFileName();

		CUString strLanguage = strFileName.Left( strFileName.GetLength() - m_strLangFileExt.GetLength() );

		m_vLangFiles.push_back( strLanguage );
	}
	sort( m_vLangFiles.begin(), m_vLangFiles.end() );
}


void CLanguage::Init( const CUString& strLanguageDir, const CUString& strLanguage )
{
	m_strLanguageDir = strLanguageDir;

	// remove all vector elements
	m_vLangFiles.clear();

	SearchLanguageFiles( );

	SetLanguage( strLanguage );
}


BOOL CLanguage::SetLanguage( const CUString& strLanguage )
{
	BOOL	bReturn = TRUE;
	DWORD   nIndex = 0;

	m_vMenus.clear();
	m_vStrings.clear();
	m_vDialogs.clear();

	m_strCurrentLanguage = DEFAULT_LANG_SELECTION;

	for ( nIndex = 0; nIndex < m_vLangFiles.size(); nIndex++ )
	{
		if ( 0 == m_vLangFiles[ nIndex ].CompareNoCase( strLanguage ) )
		{
			m_strCurrentLanguage = strLanguage;
			break;
		}
	}
	
	ParseLanguageFile( strLanguage );

	return bReturn;
}


CUString CLanguage::GetLanguageString( const DWORD dwIndex ) const
{
	ASSERT( dwIndex < m_vLangFiles.size() );

	return m_vLangFiles[ dwIndex ];
}



BOOL CLanguage::TranslateMenu( CMenu* pMenu, const int nMenuID, BOOL bTopLevel )
{
	CMenu*			pChildMenu = NULL;
	static CUString	strMissing;
	BOOL			bReturn = TRUE;
	static int		nPopUpIdx = 0;

	if ( bTopLevel )
	{
		nPopUpIdx = MAKELPARAM( 0, nMenuID );
		strMissing = _T( "" );
	}

	DWORD i = pMenu->GetMenuItemCount();
	for( i = 0; i < pMenu->GetMenuItemCount(); i++ )
	{
		pChildMenu = pMenu->GetSubMenu( i );

		int nItemID = pMenu->GetMenuItemID( i );

		if ( MENU_SEPARARTOR != nItemID )
		{
			CUString strLang;

			if ( bTopLevel )
			{
				strLang = GetMainMenuItem( nMenuID, i );
			}
			else
			{
				if ( MENU_POPUP != nItemID )
				{
					strLang = GetSubMenuItem( nItemID );
				}
				else
				{
					strLang = GetSubMenuItem( nPopUpIdx );
					nPopUpIdx++;
				}
			}
		
			if ( MENU_POPUP != nItemID )
			{
                CUStringConvert strCnv;
                
				if ( !strLang.IsEmpty() )
				{
					pMenu->ModifyMenu( nItemID, MF_STRING, nItemID, strCnv.ToT( strLang ) );
				}
				else
				{
					CString strTmp;
					pMenu->GetMenuString( nItemID, strTmp, MF_BYCOMMAND );
					ReportMissingID( nItemID, CUString( strTmp ), 1 );
				}
			}
			else
			{
				int nPosition = UINT( pChildMenu->m_hMenu );

				if ( !strLang.IsEmpty() )
				{
					int nPosition = UINT( pChildMenu->m_hMenu );

                    CUStringConvert strCnv;

					pMenu->ModifyMenu(	UINT( pChildMenu->m_hMenu ),
										MF_STRING | MF_POPUP,
										UINT( pChildMenu->m_hMenu ),
										strCnv.ToT( strLang ) );
				}
				else
				{

					if ( bTopLevel )
					{
						CString strTmp;
						pMenu->GetMenuString( i, strTmp, MF_BYPOSITION );
						strMissing += _W( "#" ) + CUString( strTmp );
					}
					else
					{
						CString strTmp;
						pMenu->GetMenuString( 0, strTmp, MF_BYPOSITION );
						ReportMissingID( nItemID, CUString( strTmp ), 1 );
					}
				}

				TranslateMenu( pMenu->GetSubMenu(i), -1, FALSE );
			}
		}
	}

	if ( bTopLevel )
	{
		if ( !strMissing.IsEmpty() )
		{
			ReportMissingID( nMenuID, strMissing + _W( "#" ), 1 );
		}	
	}

	return bReturn;
}

BOOL CLanguage::InitDialogStrings( CDialog* pDialog, long lSection )
{
	short nCtrlID;
	CUString sText;
	CWnd* pWndCtrl;
	DWORD	nIndex = 0;
    CUStringConvert strCnv;

	CUString strLang = GetDialogString( MAKELPARAM( 0, lSection ) );

	if( !strLang.IsEmpty() )
	{
		pDialog->SetWindowText( strCnv.ToT( strLang ) );
	}
	else
	{
		CString strMissing;
		pDialog->GetWindowText( strMissing );
		ReportMissingID( MAKELPARAM( 0, lSection ), CUString( strMissing ), 2 );
	}

	// test if the dialog has child windows
	if( pWndCtrl = pDialog->GetWindow( GW_CHILD | GW_HWNDFIRST ) )
	{
		// lool through all controls (windows) and replace text
		do
		{
			nCtrlID = pWndCtrl->GetDlgCtrlID();

 
			if (	!pWndCtrl->IsKindOf( RUNTIME_CLASS( CEdit ) ) &&
					!pWndCtrl->IsKindOf( RUNTIME_CLASS( CListCtrl ) ) &&
					!pWndCtrl->IsKindOf( RUNTIME_CLASS( CSpinButtonCtrl) )
				)
			{

				if( nCtrlID > 0 )
				{
					// check if the Id is in the range of common strings
					if( (nCtrlID >= COMMON_IDMIN) && (nCtrlID <= COMMON_IDMAX) )
					{
						strLang = GetString( nCtrlID );
					}
					else
					{
						strLang = GetDialogString( MAKELPARAM( nCtrlID, lSection ) );
					}

					if( !strLang.IsEmpty() )
					{

						if ( pWndCtrl->IsKindOf( RUNTIME_CLASS( CComboBox ) ) )
						{
							CUString strAdd;
							int		nPos = -1;

							// remove old strings
							((CComboBox*)pWndCtrl)->ResetContent();

							while ( ( nPos = strLang.Find( '\n' ) ) >= 0 )
							{
								((CComboBox*)pWndCtrl)->AddString( strCnv.ToT( strLang.Left( nPos ) ) );
								strLang = strLang.Mid( nPos + 1 );
								nPos++;
							}

							if ( strLang.GetLength() > nPos )
							{
								((CComboBox*)pWndCtrl)->AddString( strCnv.ToT( strLang.Mid( nPos ) ) );
							}
						}
						else
						{
							pDialog->SetDlgItemText( nCtrlID, strCnv.ToT( strLang ) );
						}
					}
					else
					{
						CString strMissing;

						if ( pWndCtrl->IsKindOf( RUNTIME_CLASS( CComboBox ) ) )
						{
							int nIndex = 0;
							int	nItems = ((CComboBox*)pWndCtrl)->GetCount();

							for ( nIndex =0; nIndex < nItems; nIndex++ )
							{
								CString strTmp;
								
								((CComboBox*)pWndCtrl)->GetLBText( nIndex, strTmp );
								strMissing += strTmp;

								if ( nIndex < nItems - 1 )
								{
									strMissing += _T( "\n" );
								}
							}
						}
						else
						{
							pDialog->GetDlgItemText( nCtrlID, strMissing );
						}
						if ( !strMissing.IsEmpty() )
						{
							ReportMissingID( MAKELPARAM( nCtrlID, lSection ), CUString( strMissing ), 2 );
						}
					}
				}
			}

			pWndCtrl = pWndCtrl->GetWindow( GW_HWNDNEXT );

		} while( (pWndCtrl != NULL) && (pWndCtrl != pDialog->GetWindow( GW_CHILD | GW_HWNDFIRST )) );
	}

	return TRUE;
}

CUString CLanguage::GetLanguage( ) const
{
	return m_strCurrentLanguage;
}




// code page table
//Identifier Name 
//037 IBM EBCDIC - U.S./Canada 
//437 OEM - United States 
//500 IBM EBCDIC - International  
//708 Arabic - ASMO 708 
//709 Arabic - ASMO 449+, BCON V4 
//710 Arabic - Transparent Arabic 
//720 Arabic - Transparent ASMO 
//737 OEM - Greek (formerly 437G) 
//775 OEM - Baltic 
//850 OEM - Multilingual Latin I 
//852 OEM - Latin II 
//855 OEM - Cyrillic (primarily Russian) 
//857 OEM - Turkish 
//858 OEM - Multlingual Latin I + Euro symbol 
//860 OEM - Portuguese 
//861 OEM - Icelandic 
//862 OEM - Hebrew 
//863 OEM - Canadian-French 
//864 OEM - Arabic 
//865 OEM - Nordic 
//866 OEM - Russian 
//869 OEM - Modern Greek 
//870 IBM EBCDIC - Multilingual/ROECE (Latin-2) 
//874 ANSI/OEM - Thai (same as 28605, ISO 8859-15) 
//875 IBM EBCDIC - Modern Greek 
//932 ANSI/OEM - Japanese, Shift-JIS 
//936 ANSI/OEM - Simplified Chinese (PRC, Singapore) 
//949 ANSI/OEM - Korean (Unified Hangeul Code) 
//950 ANSI/OEM - Traditional Chinese (Taiwan; Hong Kong SAR, PRC)  
//1026 IBM EBCDIC - Turkish (Latin-5) 
//1047 IBM EBCDIC - Latin 1/Open System 
//1140 IBM EBCDIC - U.S./Canada (037 + Euro symbol) 
//1141 IBM EBCDIC - Germany (20273 + Euro symbol) 
//1142 IBM EBCDIC - Denmark/Norway (20277 + Euro symbol) 
//1143 IBM EBCDIC - Finland/Sweden (20278 + Euro symbol) 
//1144 IBM EBCDIC - Italy (20280 + Euro symbol) 
//1145 IBM EBCDIC - Latin America/Spain (20284 + Euro symbol) 
//1146 IBM EBCDIC - United Kingdom (20285 + Euro symbol) 
//1147 IBM EBCDIC - France (20297 + Euro symbol) 
//1148 IBM EBCDIC - International (500 + Euro symbol) 
//1149 IBM EBCDIC - Icelandic (20871 + Euro symbol) 
//1200 Unicode UCS-2 Little-Endian (BMP of ISO 10646) 
//1201 Unicode UCS-2 Big-Endian  
//1250 ANSI - Central European  
//1251 ANSI - Cyrillic 
//1252 ANSI - Latin I  
//1253 ANSI - Greek 
//1254 ANSI - Turkish 
//1255 ANSI - Hebrew 
//1256 ANSI - Arabic 
//1257 ANSI - Baltic 
//1258 ANSI/OEM - Vietnamese 
//1361 Korean (Johab) 
//10000 MAC - Roman 
//10001 MAC - Japanese 
//10002 MAC - Traditional Chinese (Big5) 
//10003 MAC - Korean 
//10004 MAC - Arabic 
//10005 MAC - Hebrew 
//10006 MAC - Greek I 
//10007 MAC - Cyrillic 
//10008 MAC - Simplified Chinese (GB 2312) 
//10010 MAC - Romania 
//10017 MAC - Ukraine 
//10021 MAC - Thai 
//10029 MAC - Latin II 
//10079 MAC - Icelandic 
//10081 MAC - Turkish 
//10082 MAC - Croatia 
//12000 Unicode UCS-4 Little-Endian 
//12001 Unicode UCS-4 Big-Endian 
//20000 CNS - Taiwan  
//20001 TCA - Taiwan  
//20002 Eten - Taiwan  
//20003 IBM5550 - Taiwan  
//20004 TeleText - Taiwan  
//20005 Wang - Taiwan  
//20105 IA5 IRV International Alphabet No. 5 (7-bit) 
//20106 IA5 German (7-bit) 
//20107 IA5 Swedish (7-bit) 
//20108 IA5 Norwegian (7-bit) 
//20127 US-ASCII (7-bit) 
//20261 T.61 
//20269 ISO 6937 Non-Spacing Accent 
//20273 IBM EBCDIC - Germany 
//20277 IBM EBCDIC - Denmark/Norway 
//20278 IBM EBCDIC - Finland/Sweden 
//20280 IBM EBCDIC - Italy 
//20284 IBM EBCDIC - Latin America/Spain 
//20285 IBM EBCDIC - United Kingdom 
//20290 IBM EBCDIC - Japanese Katakana Extended 
//20297 IBM EBCDIC - France 
//20420 IBM EBCDIC - Arabic 
//20423 IBM EBCDIC - Greek 
//20424 IBM EBCDIC - Hebrew 
//20833 IBM EBCDIC - Korean Extended 
//20838 IBM EBCDIC - Thai 
//20866 Russian - KOI8-R 
//20871 IBM EBCDIC - Icelandic 
//20880 IBM EBCDIC - Cyrillic (Russian) 
//20905 IBM EBCDIC - Turkish 
//20924 IBM EBCDIC - Latin-1/Open System (1047 + Euro symbol) 
//20932 JIS X 0208-1990 & 0121-1990 
//20936 Simplified Chinese (GB2312) 
//21025 IBM EBCDIC - Cyrillic (Serbian, Bulgarian) 
//21027 Extended Alpha Lowercase 
//21866 Ukrainian (KOI8-U) 
//28591 ISO 8859-1 Latin I 
//28592 ISO 8859-2 Central Europe 
//28593 ISO 8859-3 Latin 3  
//28594 ISO 8859-4 Baltic 
//28595 ISO 8859-5 Cyrillic 
//28596 ISO 8859-6 Arabic 
//28597 ISO 8859-7 Greek 
//28598 ISO 8859-8 Hebrew 
//28599 ISO 8859-9 Latin 5 
//28605 ISO 8859-15 Latin 9 
//29001 Europa 3 
//38598 ISO 8859-8 Hebrew 
//50220 ISO 2022 Japanese with no halfwidth Katakana 
//50221 ISO 2022 Japanese with halfwidth Katakana 
//50222 ISO 2022 Japanese JIS X 0201-1989 
//50225 ISO 2022 Korean  
//50227 ISO 2022 Simplified Chinese 
//50229 ISO 2022 Traditional Chinese 
//50930 Japanese (Katakana) Extended 
//50931 US/Canada and Japanese 
//50933 Korean Extended and Korean 
//50935 Simplified Chinese Extended and Simplified Chinese 
//50936 Simplified Chinese 
//50937 US/Canada and Traditional Chinese 
//50939 Japanese (Latin) Extended and Japanese 
//51932 EUC - Japanese 
//51936 EUC - Simplified Chinese 
//51949 EUC - Korean 
//51950 EUC - Traditional Chinese 
//52936 HZ-GB2312 Simplified Chinese  
//54936 Windows XP: GB18030 Simplified Chinese (4 Byte)  
//57002 ISCII Devanagari 
//57003 ISCII Bengali 
//57004 ISCII Tamil 
//57005 ISCII Telugu 
//57006 ISCII Assamese 
//57007 ISCII Oriya 
//57008 ISCII Kannada 
//57009 ISCII Malayalam 
//57010 ISCII Gujarati 
//57011 ISCII Punjabi 
//65000 Unicode UTF-7 
//65001 Unicode UTF-8 
