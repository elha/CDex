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
#include "CDex.h"
#include <direct.h>
#include <locale.h>

#include "Util.h"
#include "MainFrm.h"
#include "PlayerFrame.h"
#include "CDexDoc.h"
#include "CDexView.h"
#include "PlayerView.h"
#include "Encode.h"
#include "Hyperlink.h"
#include "Registry.h"
#include "Util.h"
#include "DIBStatic.h"
#include "RipInfoDB.h"
#include "LangDialog.h"
#include "StatusReporter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "CDex" ) );

//#pragma comment(linker, "/delayload:libsndfile.dll")
//#pragma comment(linker, "/delayload:CDRip.dll")
//#pragma comment(linker, "/delayload:libmusicbrainz.dll")


/////////////////////////////////////////////////////////////////////////////
// CCDexApp

BEGIN_MESSAGE_MAP(CCDexApp, CWinApp)
    //{{AFX_MSG_MAP(CCDexApp)
    ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
    //	ON_COMMAND(ID_VIEWERRORLOG, OnViewerrorlog)
    //}}AFX_MSG_MAP
    // Standard file based document commands
    ON_COMMAND(ID_FILE_NEW, OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
    // Standard print setup command
    ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCDexApp construction

CCDexApp::CCDexApp()
:	m_hLibSndFileDll( NULL ),
m_hCDRipDll( NULL ),
m_hMusicBrainzDll( NULL ),
m_mtPlayerActive( FALSE, _T( "CDEXPLR" ) ),
m_mtInitializing( FALSE, _T( "CDEXPLRI" ) )
{
    
	// EnableHtmlHelp();

    m_bPlayer = false;
    m_bAutoRip = false;
    m_strCommandLineParams = _T("");
}

CCDexApp::~CCDexApp()
{
    ENTRY_TRACE( _T( "CCDexApp::~CCDexApp()" ) );
    EXIT_TRACE( _T( "CCDexApp::~CCDexApp()" ) );
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCDexApp object

CCDexApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CCDexApp initialization

void CCDexApp::SetAutoPlay( BOOL bReStore )
{
    CUStringConvert strCnv;

    if ( GetPrivateProfileInt(	_T( "General" ),
        _T( "DisableAutoCD" ),
        0,
        strCnv.ToT( g_config.GetIniFileName() ) ) ==1 )
    {
        if ( IsWindowsNTOS() )
        {
            static DWORD dwOldValue;

            // Turn of the auto notification
            CRegistry myRegistry(	HKEY_LOCAL_MACHINE,
                _T( "SYSTEM\\CurrentControlSet\\Services\\Cdrom\\Autorun" ) );
            if (bReStore)
            {
                myRegistry.Store( _T( "" ), dwOldValue );
            }
            else
            {
                // Get current value
                myRegistry.Restore( _T( "" ), dwOldValue );

                // Clear value
                myRegistry.Store( _T( "" ), 0 );
            }
        }
        else
        {
            static CUString strOldValue;

            // Turn of the auto notification
            CRegistry myRegistry(	HKEY_LOCAL_MACHINE,
                _T( "\\SOFTWARE\\Classes\\AudioCD\\shell" ) );

            // Restore old value?
            if ( bReStore )
            {
                // Restore old value
                myRegistry.Store( _T( "" ), strOldValue );
            }
            else
            {
                // Get current value
                myRegistry.Restore( _T(  "" ), strOldValue );

                // Clear value
                myRegistry.Store( _T( "" ), _T( "" ) );
            }
        }
    }
}


CDEX_ERR CCDexApp::LoadCDRipLibrary()
{
    ENTRY_TRACE( _T( "CCDexApp::LoadCDRipLibrary()" ) );

    CDEX_ERR bReturn = CDEX_OK;

    if ( m_hCDRipDll )
    {
        g_config.Save();

        CR_DeInit();

        if ( 0 == ::FreeLibrary( m_hCDRipDll ) )
        {
            LTRACE( GetLastErrorString() );

        }
        m_hCDRipDll = NULL;
    }

    if ( FALSE == LoadDLL(  _W( "libmusicbrainz.dll" ),
        _W( "\\MusicBrainz" ),
        m_hMusicBrainzDll,
        TRUE,
        FALSE ) )
    {
        LTRACE( _T( "MusicBrainzDLL not loaded!" ) );
        //		bReturn = CDEX_ERROR;
    }

    //	if ( FALSE == LoadDLL(  _W( "id3lib.dll" ),
    //							_W( "\\id3lib\\prj" ),
    //							m_hCDRipDll,
    //							TRUE,
    //							FALSE ) )
    //	{
    //		LTRACE( _T( "MusicBrainzDLL not loaded!" ) );
    ////		bReturn = CDEX_ERROR;
    //	}

    if ( FALSE == LoadDLL(  _W( "CDRip.dll" ),
        _T( "" ),
        m_hCDRipDll,
        TRUE,
        TRUE ) )
    {
        bReturn = CDEX_ERROR;
    }
    else
    {
        CUString strMsg;
        int debugLevel = 255;



        bReturn = CR_Init( g_config.GetTransportLayer(), g_config.GetCDRipDebugLevel() );


        switch ( bReturn )
        {
        case CDEX_NATIVEEASPINOTSUPPORTED:
            CDexMessageBox( g_language.GetString( IDS_NATIVEASPINOTSUPPORTED ) );
            break;
        case CDEX_FAILEDTOLOADASPIDRIVERS:
            CDexMessageBox( g_language.GetString( IDS_FAILEDTOLOADASPIDRIVERS ) );
            break;
        case CDEX_FAILEDTOGETASPISTATUS:
            CDexMessageBox( g_language.GetString( IDS_FAILEDTOGETASPISTATUS ) );
            break;
        case CDEX_NATIVEEASPISUPPORTEDNOTSELECTED:
            if ( IDYES == CDexMessageBox( g_language.GetString( IDS_ASPIFAILEDSUGGESTNATIVENTSCSI ), 
                MB_YESNO ) )
            {
                // set native SCSI libaray option
                g_config.SetTransportLayer( TRANSPLAYER_NTSCSI );
                bReturn = CR_Init( g_config.GetTransportLayer(), g_config.GetCDRipDebugLevel() );
            }
            break;
        case CDEX_NOCDROMDEVICES:
            if ( IsWindowsNTOS() && ( TRANSPLAYER_NTSCSI != (const int)g_config.GetTransportLayer() ) )
            {

                if ( IDYES == CDexMessageBox( g_language.GetString( IDS_ASPIFAILEDSUGGESTNATIVENTSCSI ), 
                    MB_YESNO ) )
                {
                    // change transport layer
                    g_config.SetTransportLayer( TRANSPLAYER_NTSCSI );
                    bReturn = CR_Init( g_config.GetTransportLayer(), g_config.GetCDRipDebugLevel() );				
                }
            }
            break;

        case CDEX_OK:
            break;
        default:
            ASSERT( FALSE );

        }
    }

    // load CDRom settings
    g_config.LoadCDRomSettings();

    EXIT_TRACE( _T( "CCDexApp::LoadCDRipLibrary(), return value %d" ), bReturn );

    return bReturn;
}


BOOL CCDexApp::InitInstance()
{
    bool bAddPlayerFiles = false;
    BOOL bReturn = TRUE;


    // only one CDex instance can be initialized at once!
    m_mtInitializing.Lock();

    // set log filename
    LOG_SetLogFileName( LOG_DEFAULT_FILENAME );

    ENTRY_TRACE( _T( "CCDexApp::InitInstance()" ) );

    // Parse command line for standard shell commands, DDE, file open
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    m_bPlayer = false;

    for (int i = 1; i < __argc; i++)
    {
        LPCTSTR pszParam = __targv[i];
        BOOL bFlag = FALSE;
        BOOL bLast = ((i + 1) == __argc);

        if (pszParam[0] == _T( '-' ) || pszParam[0] == _T( '/' ) )
        {
            // remove flag specifier
            bFlag = TRUE;
            ++pszParam;
            const CUString strPlayer( _T( "PLAY" ) );
            const CUString strAutoRip( _T( "AUTORIP" ) );
            const CUString strAdd( _T( "ADD" ) );
            const CUString strRegister( _T( "REGISTER" ) );

            CUString strParam = CUString( pszParam );
            if ( 0 == strPlayer.CompareNoCase( strParam ) )
            {
                m_bPlayer = true;
            }
            if ( 0 == strAutoRip.CompareNoCase( strParam ) )
            {
                m_bAutoRip = true;
            }
            if ( 0 == strAdd.CompareNoCase( strParam ) )
            {
                bAddPlayerFiles = true;
            }
            if ( 0 == strRegister.CompareNoCase( strParam ) )
            {
                //				CRegistry myRegistry(	HKEY_LOCAL_MACHINE,
                //										_T( "\\SOFTWARE\\Classes\\AudioCD\\shell" ) );
            }
        }
        else
        {
            if (m_strCommandLineParams.GetLength() > 0 )
            {
                m_strCommandLineParams += _T( '?' );
            }

            m_strCommandLineParams += CUString( pszParam );
        }
    }


    CUString strVersionValue( g_language.GetString( IDS_CDEX_PLAYER_VERSION ) + g_config.GetVersion() );
    CUStringConvert strCnv;

    HWND hWndPlayer = FindWindowEx( NULL, NULL, NULL, strCnv.ToT( strVersionValue ) );

    // check if the CDex player is already active
    if ( m_bPlayer && (0 != hWndPlayer) )
    {
        bReturn = FALSE;

        if ( bAddPlayerFiles )
        {
            CUStringConvert strCnv; 

            COPYDATASTRUCT copyData;

            copyData.dwData = 0;
            copyData.cbData = m_strCommandLineParams.GetLength() + 1;
            copyData.lpData = (PVOID)strCnv.ToUTF8( m_strCommandLineParams );

            int nReturn = ::SendMessage(	hWndPlayer,
                WM_COPYDATA,
                (WPARAM)hWndPlayer,
                (LPARAM)&copyData );
        }
    }
    else
    {
        // Load config settings
        g_config.Load();

        // Initialize util library
        InitUtil();

        // hack the profile settings, so everyting is loaded/stored into the local 
        // configuration file (cdex.ini file)
        free((void*)m_pszProfileName);

        CUStringConvert strCnv;
        m_pszProfileName = _tcsdup( strCnv.ToT( g_config.GetIniFileName() ) );

        // Initialize language library
        g_language.Init( g_config.GetAppPath(), g_config.GetLanguage() );

        // Load libsnd DLL
        if ( FALSE == LoadDLL(  _W( "libsndfile.dll" ),
            _T( "" ),
            m_hLibSndFileDll,
            TRUE,
            TRUE ) )
        {
            return FALSE;
        }

        // Initialize ripper library
        LoadCDRipLibrary();


        AfxEnableControlContainer();

        // enable the error logging/ exception handling mechanisms:
        //	if (AfxGetApp()->GetProfileInt("Version 1.0","DisableExceptionHandling",0)!=1)
        //		TheExceptionLog.InitInstance();

        // Standard initialization
        //#ifdef _AFXDLL
        //	Enable3dControls();			// Call this when using MFC in a shared DLL
        //#else
        //	Enable3dControlsStatic();	// Call this when linking to MFC statically
        //#endif

        // Change the registry key under which our settings are stored.
        // SetRegistryKey( _T( "CDex" ) );

        // LoadStdProfileSettings();  // Load standard INI file options (including MRU)

        // Register document templates
        CSingleDocTemplate* pDocTemplate;


        if ( m_bPlayer )
        {
            pDocTemplate = new CSingleDocTemplate(
                IDR_PLAYERFRAME,
                RUNTIME_CLASS(CCDexDoc),
                RUNTIME_CLASS(CPlayerFrame),
                RUNTIME_CLASS(CPlayerView));
            AddDocTemplate(pDocTemplate);
        }
        else
        {
            pDocTemplate = new CSingleDocTemplate(
                IDR_MAINFRAME,
                RUNTIME_CLASS(CCDexDoc),
                RUNTIME_CLASS(CMainFrame),       // main SDI frame window
                RUNTIME_CLASS(CCDexView));

            AddDocTemplate(pDocTemplate);

            if (g_config.GetWinEncoder())
            {
                TCHAR lpszAcmDriver[ 80 ];

                GetPrivateProfileString(	_T( "Drivers32" ),
                    _T( "msacm.l3codec" ),
                    _T( "" ),
                    lpszAcmDriver,
                    sizeof( lpszAcmDriver ),
                    _T( "System.ini" ) );

                if ( _T( '\0' ) == lpszAcmDriver[ 0 ] )
                {
                    GetPrivateProfileString(	_T( "Drivers32" ),
                        _T( "msacm.l3acm"),
                        _T( "" ),
                        lpszAcmDriver,
                        sizeof( lpszAcmDriver ),
                        _T( "System.ini" ) );
                }

                // Init windows encoder
                if ( lpszAcmDriver[0] != _T( '\0' ) )
                {
                    g_AcmCodec.EnumDrivers();
                }
            }

            // Disable auto play
            SetAutoPlay( FALSE );

        }
        // Dispatch commands specified on the command line
        if ( !ProcessShellCommand( cmdInfo ) )
        {
            return FALSE;
        }

        // Tell the status monitor that we're here
        if ( g_config.GetUseStatusServer() )
        {
            g_statusReporter.Connect();
        }

        m_pMainWnd->ShowWindow( SW_SHOW );
        m_pMainWnd->UpdateWindow();

        // Save INI file
        g_config.Save();
    }

    EXIT_TRACE( _T( "CCDexApp::InitInstance(), return value: %d" ), bReturn );

    return bReturn;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CLangDialog
{
public:
    CAboutDlg();

    // Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    CDIBStatic	m_ActualBMP;
    CHyperLink	m_CDexLink;
    CHyperLink	m_CDexTranslatorLink;
    CString	    m_strCDexVersion;
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CLangDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{

    if ( GetDlgItem( IDC_ABOUT_TRANSLATOR_URL ) )
    {
        CString strURL;
        GetDlgItem( IDC_ABOUT_TRANSLATOR_URL )->GetWindowText( strURL );
        m_CDexTranslatorLink.SetURL( strURL );
    }

    //	m_CDexTranslatorLink.SetURL( g_language.GetString( IDC_ABOUT_TRANSLATOR_URL ) );

    CLangDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    DDX_Control(pDX, IDC_CDEXLOGO, m_ActualBMP);
    DDX_Control(pDX, IDC_CDEXHYPERLINK, m_CDexLink);
    DDX_Control(pDX, IDC_ABOUT_TRANSLATOR_URL, m_CDexTranslatorLink);
    DDX_Text(pDX, IDC_CDEXVERSION, m_strCDexVersion);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CLangDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
    ON_WM_CTLCOLOR()
    ON_WM_PAINT()
    ON_BN_CLICKED(ID_CLOSE, OnClose)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CCDexApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CAboutDlg::OnClose() 
{
    OnOK();	
}


/////////////////////////////////////////////////////////////////////////////
// ProcessWndProcException - catch all for CException derived exceptions 
LRESULT CCDexApp::ProcessWndProcException( CException* e, const MSG* pMsg )
{
    return CWinApp::ProcessWndProcException(e,pMsg);
}



/////////////////////////////////////////////////////////////////////////////
// CCDexApp commands

int CCDexApp::ExitInstance() 
{
    // Tell the status monitor that we're going away
    if ( g_config.GetUseStatusServer() )
    {
        g_statusReporter.Disconnect();
    }

    if ( NULL != m_hMusicBrainzDll ) 
    {
        FreeLibrary( m_hMusicBrainzDll );
        m_hMusicBrainzDll = NULL;
    }

    if ( NULL != m_hCDRipDll ) 
    {
        // Save config info, before we unload the CDRip.
        g_config.Save();

        CR_DeInit();

        FreeLibrary( m_hCDRipDll );
    }

    if ( NULL != m_hLibSndFileDll ) 
    {
        FreeLibrary( m_hLibSndFileDll );
    } 

    if ( ! m_bPlayer )
    {
        // Restore old key
        SetAutoPlay( TRUE );
    }

    m_mtPlayerActive.Unlock();

    return( CWinApp::ExitInstance() );
}


void CCDexApp::OnViewerrorlog() 
{
}


void CCDexApp::OnFileNew( )
{
    CWinApp::OnFileNew();
}


HBRUSH CAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    HBRUSH hbr = CLangDialog::OnCtlColor(pDC, pWnd, nCtlColor);

    // Select transparent drawing mode
    pDC->SetBkMode(TRANSPARENT);

    // Return default brush
    return hbr;
}

BOOL CAboutDlg::OnInitDialog() 
{
    CLangDialog::OnInitDialog();

    // set language string
    CString strLang;
    GetDlgItem( IDC_CDEXVERSION )->GetWindowText( strLang );
    
    CUStringConvert strCnv;
    m_strCDexVersion = strLang + strCnv.ToT( g_config.GetVersion() );

#ifdef _UNICODE
    m_strCDexVersion += _T( " (UNICODE BUILD)" );

#endif

    m_ActualBMP.LoadFromResource( _T( "IDB_CDEXLOGO" ) );

    UpdateData( FALSE );

    return TRUE;
}

static const DWORD aMenuHelpIDs[] =
{
    IDD_WAVFILEDLG,  IDD_WAVFILEDLG ,
    0,    0
};

      //static const DWORD aMenuHelpIDs[] =
      //{
      //   ID_FILE_NEW,  IDH_FILE_NEW ,
      //   ID_FILE_OPEN, IDH_FILE_OPEN ,
      //   ID_FILE_SAVE, IDH_FILE_SAVE ,
      //   ID_EDIT_CUT,  IDH_EDIT_CUT,
      //   ID_EDIT_COPY, IDH_EDIT_COPY,
      //   ID_EDIT_PASTE, IDH_EDIT_PASTE,
      //   ID_FILE_PRINT, IDH_FILE_PRINT,
      //   ID_APP_ABOUT,  IDH_APP_ABOUT,
      //   ID_FILE_PRINT_PREVIEW, IDH_FILE_PRINT_PREVIEW,
      //   0,    0
      //};
// http://support.microsoft.com/kb/191118

int numHelpIDs = 2;

void CCDexApp::WinHelp( DWORD dwData, UINT nCmd )
{
    CWinApp::WinHelp( dwData, nCmd );
    //DWORD i;
    //switch (nCmd)
    //{
    //case HELP_CONTEXT:

    //    // If it is a help context command, search for the
    //    // control ID in the array.
    //    for (i= 0; i < numHelpIDs*2; i+=2)
    //    {
    //        if (aMenuHelpIDs[i] == LOWORD (dwData) )
    //        {
    //            i++;  // pass the help context id to HTMLHelp
    //            HtmlHelp(m_pMainWnd->m_hWnd,"cdex.chm",
    //                HH_HELP_CONTEXT,aMenuHelpIDs[i]);
    //            return;
    //        }
    //    }

    //    // If the control ID cannot be found,
    //    // display the default topic.
    //    if (i == numHelpIDs*2)
    //        HtmlHelp(m_pMainWnd->m_hWnd,"sample.chm",HH_DISPLAY_TOPIC,0);
    //    break;
    //}
}
