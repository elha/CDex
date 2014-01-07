/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2007 Georgy Berdyshev
** Copyright (C) 2007 Albert L. Faber
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
#include <tchar.h>
#include "CDex.h"
#include "CDexFileOpen.h"

#include "CueSheet.h"
#include ".\cuesheet.h"

INITTRACE( _T( "CueSheet" ) );


// AF FIXME
#ifndef _UNICODE
# define tCHAR char
# define tSPRINTF sprintf
# define tFPRINTF fprintf
# define tFOPEN fopen
#else
# define tCHAR wchar_t
# define tSPRINTF wsprintf
# define tFPRINTF fwprintf
# define tFOPEN _wfopen
#endif


// CCueSheet dialog

IMPLEMENT_DYNAMIC(CCueSheet, CDialog)
CCueSheet::CCueSheet(CWnd* pParent /*=NULL*/)
	: CLangDialog(CCueSheet::IDD, pParent)
{
}

CCueSheet::~CCueSheet()
{
}

void CCueSheet::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_editCueSheet);
    DDX_Control(pDX, IDC_CHECK1, m_checkPreGaps);
    //    DDX_GetFileControl(pDX, IDC_CUESHEETFILENAME, m_cueSheetFileName, 0, g_language.GetString( IDS_CUESHEETFILESELECTION ) );
    DDX_Control(pDX, IDC_CHECK2, m_addISRC);
}


BEGIN_MESSAGE_MAP(CCueSheet, CDialog)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
	ON_BN_CLICKED(IDC_CHECK1, OnBnClickedCheck1)
    ON_BN_CLICKED(IDC_CHECK2, OnBnClickedCheck2)
    ON_BN_CLICKED(IDC_COPYTOCLIPBOARD, OnBnClickedCopytoclipboard)
    ON_BN_CLICKED(IDCLOSE, OnBnClickedClose)
END_MESSAGE_MAP()



void CCueSheet::CreateSheet()
{	
	LTRACE2( _T( "CCDexView::OnToolsGeneratecuesheet " ) );

	m_strCueSheet = _W( "" );

    CDInfo& cdInfo = m_pDoc->GetCDInfo();

    DWORD cdID = cdInfo.GetDiscID();

	CUString cueSheetName;
    cueSheetName.Format( _W( "%08X.cue" ), cdID );


    CUString strTmp;
	if (! cdInfo.GetGenre().IsEmpty())
    {
        strTmp.Format( _W("REM GENRE \"%s\"\n"), (LPCWSTR)cdInfo.GetGenre() );
		m_strCueSheet += strTmp;
    }

	if (! cdInfo.GetYear().IsEmpty())
    {
        strTmp.Format( _W("REM DATE %s\n"), (LPCWSTR)cdInfo.GetYear() );
		m_strCueSheet += strTmp;
    }


	strTmp.Format( _W("REM DISCID %08lx\n"), (LPCWSTR)cdInfo.GetDiscID() );
	m_strCueSheet += strTmp; 

	strTmp.Format( _W("REM VOLID %lX\n"), (LPCWSTR)cdInfo.GetVolID() );
	m_strCueSheet += strTmp; 

	strTmp.Format( _W("REM CDDBCAT %s\n"), (LPCWSTR)cdInfo.GetCDDBCat() );
	m_strCueSheet += strTmp; 

	strTmp.Format( _W("REM COMMENT \"CDex %s\"\n"), (LPCWSTR)g_config.GetVersion() );
	m_strCueSheet += strTmp; 

	m_strCueSheet += _W("REM\n");

    if ( m_addISRC.GetCheck() )
    {
        strTmp.Format(  _W("CATALOG %s\n"), (LPCWSTR)cdInfo.GetMCN() );
	    m_strCueSheet += strTmp; 
    }

	strTmp.Format( _W("PERFORMER \"%s\"\n"), (LPCWSTR)cdInfo.GetArtist());
	m_strCueSheet += strTmp; 

	strTmp.Format(  _W("TITLE \"%s\"\n"), (LPCWSTR)cdInfo.GetTitle() );
	m_strCueSheet += strTmp; 

    m_strCueSheet += _W("FILE \"range.wav\" WAVE\n" );

	int	count = 1;
	int offset = 0; // dlg.GetFirstTrack() == 0 ? cdInfo.GetStartSector(cdInfo.GetTrack( 0 )) : 0;

    for (int i = 0; i < cdInfo.GetNumTracks(); ++i, ++count)
	{
		strTmp.Format(  _W("  TRACK %02d AUDIO\n"), count );
		m_strCueSheet += strTmp; 

		strTmp.Format(  _W("    TITLE \"%s\"\n"), (LPCWSTR)cdInfo.GetTrackName( i ) );
		m_strCueSheet += strTmp; 
		
        if (g_config.GetSplitTrackName())
		{
			strTmp.Format(  _W("    PERFORMER \"%s\"\n"), (LPCWSTR)cdInfo.GetArtist());
			m_strCueSheet += strTmp; 
		}
        else
        {
			strTmp.Format(  _W("    PERFORMER \"%s\"\n"), (LPCWSTR)cdInfo.GetArtist());
			m_strCueSheet += strTmp; 
        }

        if ( m_addISRC.GetCheck() )
        {
            strTmp.Format(  _W("    ISRC %s\n"), (LPCWSTR)cdInfo.GetISRC( i ) );
		    m_strCueSheet += strTmp; 
        }

		if ( m_checkPreGaps.GetCheck() )
		{
			int dwGapSize = cdInfo.GetGap( i );

			// Calculate sectors to time
			int nF= (int)( dwGapSize % 75 );
			int nS= (int)( (dwGapSize / 75 ) %60 );
			int nM= (int)( (dwGapSize / 75 ) /60 );

			strTmp.Format(  _W("    PREGAP %02d:%02d:%02d\n"), nM, nS, nF );
			m_strCueSheet += strTmp; 
		}

		DWORD dwStart = offset + cdInfo.GetStartSector( cdInfo.GetTrack( i ) ) /*- dlg.GetFirstSector() */;

		// Calculate sectors to time
		int nF= (int)( dwStart % 75 );
		int nS= (int)( (dwStart / 75 ) %60 );
		int nM= (int)( (dwStart / 75 ) /60 );

		strTmp.Format(  _W("    INDEX 01 %02d:%02d:%02d\n"), nM, nS, nF );
		m_strCueSheet += strTmp; 
	}

    CUString strMsg( m_strCueSheet );

	strMsg.Replace( _W( "\n" ), _W( "\r\n" ) );

    CUStringConvert strCnv;
    m_editCueSheet.SetWindowText( strCnv.ToT( strMsg ) );
}

// CCueSheet message handlers

void CCueSheet::OnBnClickedButton1()
{
	CreateSheet();

    CUStringConvert strCnv;
    CUStringConvert strCnv1;

    CDInfo& cdInfo = m_pDoc->GetCDInfo();
    CUString cueSheetName;
    DWORD cdID = cdInfo.GetDiscID();

    cueSheetName.Format( _W( "%s%08X.cue" ), 
        (LPCWSTR)g_config.GetMP3OutputDir(),
        cdID );

    CUString strFilter = g_language.GetString( IDS_CUESHEETFILESELECTION );

    CFileDialog cueFileDlg(
        FALSE,
        _T( ".cue" ),
        strCnv.ToT( cueSheetName ),
        OFN_OVERWRITEPROMPT,
        strCnv1.ToT( strFilter ) );

    if ( IDOK == cueFileDlg.DoModal() )
    {
        CUString strFileName = CUString( cueFileDlg.GetPathName() );
        FILE* pFile = CDexOpenFile( strFileName, _W( "w" ) );

        if ( pFile )
        {
#ifdef UNICODE
            BYTE btBOM[] = { 0xEF, 0xBB, 0xBF };
            fwrite( &btBOM, sizeof( btBOM ), 1, pFile );
#endif
            LPCSTR lpszUtf = strCnv.ToUTF8( m_strCueSheet );
            fwrite( lpszUtf, 
                    strlen( lpszUtf ), 
                    1, 
                    pFile );
            fclose( pFile );
        }

    }
}

void CCueSheet::OnBnClickedCheck1()
{
	CreateSheet();
}

BOOL CCueSheet::OnInitDialog()
{
	CLangDialog::OnInitDialog();
	
    m_addISRC.SetCheck( TRUE );
    m_checkPreGaps.SetCheck( TRUE );

	CreateSheet();
	return TRUE;  
}

void CCueSheet::OnBnClickedCheck2()
{
	CreateSheet();
}

void CCueSheet::OnBnClickedCopytoclipboard()
{
	CreateSheet();

	if ( OpenClipboard()==0 )  
	{
		CDexMessageBox( g_language.GetString( IDS_ERROR_CANNOTOPENCLIPBOARD ) );    
		return;  
	}
	// Remove the current Clipboard contents  
	if( EmptyClipboard()==0 )  
	{
		CDexMessageBox( g_language.GetString( IDS_ERROR_CANNOTEMPTYCLIPBOARD ) );
	}  

    DWORD dwNumBytes = ( m_strCueSheet.GetLength() + 1 ) * sizeof( TCHAR );

    HANDLE hData = GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, dwNumBytes );

	if ( NULL != hData )
	{
        CUStringConvert strCnv;
    
		LPSTR lpszClip= (LPSTR)GlobalLock( hData );

        // clear while buffer, just to be sure
        memset( lpszClip, 0, dwNumBytes );

        // copy the string data
        memcpy( lpszClip, strCnv.ToT( m_strCueSheet ), dwNumBytes - sizeof( TCHAR ) );  

#ifndef _UNICODE
		if ( ::SetClipboardData( CF_TEXT, hData ) == NULL )  
		{
#else
		if ( ::SetClipboardData( CF_UNICODETEXT, hData ) == NULL )  
		{
#endif
    		CDexMessageBox( g_language.GetString( IDS_UNABLE_COPYTO_CLIPBOARD ) );
        }
	}

	// Close Clipboard
	CloseClipboard();
}

void CCueSheet::OnBnClickedClose()
{
    OnCancel();
}
