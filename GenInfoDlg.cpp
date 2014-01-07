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


#include "stdafx.h"
#include "cdex.h"
#include "CDexDoc.h"
#include "GenInfoDlg.h"
#include "FileVersion.h"
//#include <Winver.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "Windows.h"


/////////////////////////////////////////////////////////////////////////////
// CGenInfoDlg dialog
CGenInfoDlg::CGenInfoDlg(CCDexDoc* pDoc,CWnd* pParent /*=NULL*/)
	: CDialog(CGenInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGenInfoDlg)
	m_strDiscID = _T("");
	m_strVolID = _T("");
	m_strOsVersion = _T("");
	m_strAspiVersion = _T("");
	m_strAspiVersion1 = _T("");
	//}}AFX_DATA_INIT

	m_pDoc = pDoc;

}


void CGenInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGenInfoDlg)
	DDX_Control(pDX, IDC_DETAILEDDRIVEINFO, m_cDetailedDriveInfo);
	DDX_Text(pDX, IDC_DISCID, m_strDiscID);
	DDX_Text(pDX, IDC_VOLID, m_strVolID);
	DDX_Text(pDX, IDC_OSVERSION, m_strOsVersion);
	DDX_Text(pDX, IDC_ASPIVERSION, m_strAspiVersion);
	DDX_Text(pDX, IDC_ASPIVERSION1, m_strAspiVersion1);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGenInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CGenInfoDlg)
	ON_BN_CLICKED(IDC_COPYTOCLIP, OnCopytoclip)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGenInfoDlg message handlers

void CGenInfoDlg::OnCopytoclip() 
{
	CUString strLang;
	CString strDriveInfo;

	if ( OpenClipboard()==0 )  
	{
		strLang = g_language.GetString( IDS_ERROR_CANNOTOPENCLIPBOARD );
		CDexMessageBox( strLang );
		return;  
	}
	
	// Remove the current Clipboard contents  
	if( EmptyClipboard()==0 )  
	{
		strLang = g_language.GetString( IDS_ERROR_CANNOTEMPTYCLIPBOARD );
		CDexMessageBox( strLang );
	}  

	// Create srting with all the data we want to put in the clip board

	CUString strClip;
	strClip += CUString( _W( "CDex version  :" )) + g_config.GetVersion() + _W( "\r\n" );
	strClip += _W( "Disk ID       :" ) + CUString( m_strDiscID ) + _W( "\r\n" );
	strClip += _W( "Volume ID     :" ) + CUString( m_strVolID ) + _W( "\r\n" );
	strClip += _W( "OS Version    :" ) + CUString( m_strOsVersion ) + _W( "\r\n" );
	strClip += _W( "Aspi Manager  :" ) + CUString( m_strAspiVersion ) + _W( " " ) + CUString( m_strAspiVersion1 ) + _W( "\r\n" );

	m_cDetailedDriveInfo.GetWindowText( strDriveInfo );

	strClip += _W("\r\n") + CUString( strDriveInfo );

	HANDLE hData=GlobalAlloc(	GMEM_MOVEABLE | GMEM_DDESHARE,
								strClip.GetLength() + 1 );

	if ( NULL != hData )
	{
        CUStringConvert strCnv;

		LPSTR lpszClip = (LPSTR)GlobalLock( hData );
		strcpy( lpszClip, strCnv.ToACP( strClip ) );

		if ( NULL == ::SetClipboardData( CF_TEXT, hData ) )  
		{
			strLang = g_language.GetString( IDS_UNABLE_COPYTO_CLIPBOARD );
    		CDexMessageBox( strLang );
		}  
	}

	// Close Clipboard
	CloseClipboard();
}

BOOL CGenInfoDlg::OnInitDialog() 
{
	int nDrive = 0;

	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );
	
	CHAR lpszInfo[ 4096 ];

	// Construct CDROMPARAMS
	CDROMPARAMS cdParams;

	// Get volume ID and put it into local string
	m_strVolID.Format( _T( "%X" ), m_pDoc->GetCDInfo().GetVolID() );

	// Get Disc ID and put it into local string
	m_strDiscID.Format( _T( "%08X" ), m_pDoc->GetCDInfo().GetDiscID() );

	int nActiveCDROM = CR_GetActiveCDROM();

	for ( nDrive = 0; nDrive < CR_GetNumCDROM(); nDrive++ )
	{
		CR_SetActiveCDROM( nDrive );

		// Get cdParames of current selected CD-ROM
		CR_GetCDROMParameters( &cdParams );

		CR_GetDetailedDriveInfo( lpszInfo, sizeof( lpszInfo ) );

		m_cDetailedDriveInfo.AddString( CUString( lpszInfo, CP_UTF8 ) );
	}

	// Scroll the edit control
	m_cDetailedDriveInfo.LineScroll( -m_cDetailedDriveInfo.GetFirstVisibleLine(), 0 );

	CR_SetActiveCDROM( nActiveCDROM );

	OSVERSIONINFO myOsVersion;

	// Set size of structure
	myOsVersion.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );

	::GetVersionEx( &myOsVersion);

	CUString strOsType( _T( "Unknown" ) );	
	
/*
              dwMajorVersion  dwMinorVersion  dwPlatformID
Win3x+Win32s      ?                ?          VER_PLATFORM_WIN32s
       Win95      4                0          VER_PLATFORM_WIN32_WINDOWS
       Win98      4                10         VER_PLATFORM_WIN32_WINDOWS
       WinME      4                90         VER_PLATFORM_WIN32_WINDOWS
      NT3.51      3                51         VER_PLATFORM_WIN32_NT
       NT4.0      4                0          VER_PLATFORM_WIN32_NT
       Win2k      5                0          VER_PLATFORM_WIN32_NT
       WINXP      5                1          VER_PLATFORM_WIN32_NT
     WIN.NET      5                1          VER_PLATFORM_WIN32_NT
*/

	switch ( myOsVersion.dwPlatformId )
	{
		case VER_PLATFORM_WIN32s:
			strOsType = _W( "Win32s" );
		break;
		case VER_PLATFORM_WIN32_WINDOWS:

			strOsType = _W( "Windows 9x" );

			if (	( 4 == myOsVersion.dwMajorVersion ) &&
					( 0 == myOsVersion.dwMinorVersion ) )
			{
				strOsType = _W( "Windows 95" );
			}
			if (	( 4 == myOsVersion.dwMajorVersion ) &&
					( 10 == myOsVersion.dwMinorVersion ) )
			{
				strOsType = _W( "Windows 98" );
			}
			if (	( 4 == myOsVersion.dwMajorVersion ) &&
					( 90 == myOsVersion.dwMinorVersion ) )
			{
				strOsType = _W( "Windows ME" );
			}
		break;
		case VER_PLATFORM_WIN32_NT:
			strOsType = _T( "Windows NT" );

			if (	( 3 == myOsVersion.dwMajorVersion ) &&
					( 51 == myOsVersion.dwMinorVersion ) )
			{
				strOsType = _W( "Windows NT 3.51" );
			}
			if (	( 4 == myOsVersion.dwMajorVersion ) &&
					( 0 == myOsVersion.dwMinorVersion ) )
			{
				strOsType = _W( "Windows NT 4.0" );
			}
			if (	( 5 == myOsVersion.dwMajorVersion ) &&
					( 0 == myOsVersion.dwMinorVersion ) )
			{
				strOsType = _W( "Windows 2000" );
			}
			if (	( 5 == myOsVersion.dwMajorVersion ) &&
					( 1 == myOsVersion.dwMinorVersion ) )
			{
				strOsType = _W( "Windows XP" );
			}
		break;
	}

    CUStringConvert strCnv;
	m_strOsVersion.Format(	_T( "%s (Version %d.%d build %d %s)" ),
							strCnv.ToT( strOsType ),
							myOsVersion.dwMajorVersion,
							myOsVersion.dwMinorVersion,
							LOWORD(myOsVersion.dwBuildNumber),
							myOsVersion.szCSDVersion );

	// Get ASPI version
	CFileVersion myVersion;
	TCHAR lpszModuleName[ MAX_PATH + 1];

	// Get ASPI manager through ASPI DLL
//	pCdda->GetHaInquiry(m_strAspiVersion);

	// Get ASPI manager information through FileVersion
	GetSystemDirectory( lpszModuleName, MAX_PATH );
	
	_tcscat( lpszModuleName, _T( "\\wnaspi32.dll" ) );
	myVersion.Open( lpszModuleName );

	CUString strCompany = myVersion.GetCompanyName();
	CUString strVersion = myVersion.GetProductVersion();
	CUString strProductName = myVersion.GetProductName();

	if ( strCompany.GetLength() > 0 )
	{
        CUStringConvert strCnv;
        CUStringConvert strCnv1;
        CUStringConvert strCnv2;

        m_strAspiVersion.Format( _T( "%s ; %s ; %s " ), strCnv.ToT( strCompany ), strCnv1.ToT( strProductName ), strCnv2.ToT( strVersion ) );
	}
	else
	{
		m_strAspiVersion = _T( "?" );
	}

	UpdateData( FALSE );

	return TRUE;
}
