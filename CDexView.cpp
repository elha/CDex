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
#include "CDexDoc.h"
#include "CDexView.h"
#include <direct.h>
#include <limits.h>

#include "ConfigDlg.h"
#include "Filename.h"
#include "PCopyDlg.h"
#include "GenInfoDlg.h"
#include "MP3toRiffWavDlg.h"
#include "RiffWavToMP3.h"
#include "MP3ToWavDlg.h"
#include "CddbDlg.h"
#include "AudioFile.h"
#include "DropDlg.h"
#include "CDDB.h"
#include "mainfrm.h"
#include "RipStatusDlg.h"
#include "FileCompareDlg.h"
#include "RecordFiles.h"
#include "CdrDao.h"

#include "CDexFileOpen.h"
#include "ISndStreamWinAmp.h"
#include "WinampInConfig.h"
#include "GenreEditDlg.h"
#include "EnterProfileName.h"
#include "MusicBrainz.h"
#include ".\cdexview.h"
#include "GetRemoteCDInfoDlg.h"
#include "CDexMsgBox.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BOOL g_bEncoderPresent=TRUE;

const int TL_ARTISTNAME   = 0;
const int TL_TRACKNAME    = 0;
const int TL_TRACKNUMBER  = 1;
const int TL_STARTTIME    = 2;
const int TL_PLAYTIME     = 3;
const int TL_TRACKSIZE    = 4;
const int TL_TRACKSTATUS  = 5;
const int TL_CHECKSUM     = 6;
const int TL_GAP          = 7;
const int TL_ISRC         = 8;

typedef struct 
{
    int ID;
    int IDLang;
    DWORD textAlignment;
    double width;
} TITLE_HEADER_TABLE;

TITLE_HEADER_TABLE titleHeaderTable[] = 
{
    { TL_TRACKNAME,     IDS_NAME,       LVCFMT_LEFT, 0.25  },
    { TL_TRACKNUMBER,   IDS_TRACK,      LVCFMT_RIGHT, 0.075 },
    { TL_STARTTIME,     IDS_START_TIME, LVCFMT_RIGHT, 0.075 },
    { TL_PLAYTIME,      IDS_PLAY_TIME,  LVCFMT_RIGHT, 0.075 },
    { TL_TRACKSIZE,     IDS_SIZE,       LVCFMT_RIGHT, 0.075 },
    { TL_TRACKSTATUS,   IDS_STATUS,     LVCFMT_RIGHT, 0.075 },
    { TL_CHECKSUM,      IDS_CHECKSUM,   LVCFMT_RIGHT, 0.075 },
    { TL_GAP,           IDS_PREGAP,     LVCFMT_RIGHT, 0.075 },
    { TL_ISRC,          IDS_ISRC,       LVCFMT_RIGHT, 0.135 },
};

// Timer stuff
#define TIMERID 1
#define TIMERSLOT 500

static const int	INIT  = 0;
static const int	IDLE  = 1;
static const int	NEWCD=2;
static const int	EJECTCD=3;
static const int	PLAYING=4;
static const int	RECORDING=5;
static const int	NONE=6;
static const int	CONFIGURE=7;

#define PLAYOPERATION_INPROGRESS	17
#define PLAYOPERATION_PAUSED		18
#define PLAYOPERATION_SUCCESSCOMPL	13
#define PLAYOPERATION_STOPERROR		14


INITTRACE( _T( "CDexView" ) );


/////////////////////////////////////////////////////////////////////////////
// CCDexView

IMPLEMENT_DYNCREATE(CCDexView, CFormView)

BEGIN_MESSAGE_MAP(CCDexView, CFormView)
	//{{AFX_MSG_MAP(CCDexView)
	ON_WM_CLOSE()
	ON_COMMAND(ID_CONFIGURE, OnConfigure)
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_COMMAND(ID_TRACKS2WAV, OnTracks2wav)
	ON_COMMAND(ID_TRACKS2MP3, OnTracks2mp3)
	ON_COMMAND(ID_WAV2MP3, OnWav2mp3)
	ON_COMMAND(ID_SYSTEMINFO, OnSysteminfo)
	ON_COMMAND(ID_PTRACKS2MP3, OnPtracks2mp3)
	ON_COMMAND(ID_NEXTTRACK, OnNexttrack)
	ON_COMMAND(ID_PREVTRACK, OnPrevtrack)
	ON_COMMAND(ID_STOPBUTTON, OnStopbutton)
	ON_UPDATE_COMMAND_UI(ID_CDSELBAR, OnUpdateCDSelBar)
	ON_UPDATE_COMMAND_UI(ID_CONFIGCD, OnUpdateConfigCD)
	ON_UPDATE_COMMAND_UI(ID_PTRACKS2MP3, OnUpdatePtracks2mp3)
	ON_UPDATE_COMMAND_UI(ID_SKIPFORWARD, OnUpdateSkipforward)
	ON_UPDATE_COMMAND_UI(ID_SKIPBACK, OnUpdateSkipback)
	ON_UPDATE_COMMAND_UI(ID_PREVTRACK, OnUpdatePrevtrack)
	ON_UPDATE_COMMAND_UI(ID_STOPBUTTON, OnUpdateStopbutton)
	ON_UPDATE_COMMAND_UI(ID_TRACKS2MP3, OnUpdateTracks2mp3)
	ON_UPDATE_COMMAND_UI(ID_TRACKS2WAV, OnUpdateTracks2wav)
	ON_UPDATE_COMMAND_UI(ID_WAV2MP3, OnUpdateWav2mp3)
	ON_EN_CHANGE(IDC_ARTIST, OnChange)
	ON_COMMAND(ID_REMOTECDDB, OnRemotecddb)
	ON_UPDATE_COMMAND_UI(ID_READLOCALCDDB, OnUpdateCDDB)
	ON_COMMAND(ID_PLAYTRACK, OnPlaytrack)
	ON_UPDATE_COMMAND_UI(ID_PLAYTRACK, OnUpdatePlaytrack)
	ON_COMMAND(ID_PLAYBARPAUSE, OnPause)
	ON_UPDATE_COMMAND_UI(ID_PLAYBARPAUSE, OnUpdatePause)
	ON_UPDATE_COMMAND_UI(ID_NEXTTRACK, OnUpdateNexttrack)
	ON_NOTIFY(NM_RCLICK, IDC_TRACKLIST, OnRclickTracklist)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_TRACKLIST, OnEndlabeleditTracklist)
	ON_UPDATE_COMMAND_UI(ID_CONFIGURE, OnUpdateConfigure)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_READCDPLAYERINI, OnReadcdplayerini)
	ON_COMMAND(ID_READLOCALCDDB, OnReadlocalcddb)
	ON_WM_DESTROY()
	ON_COMMAND(ID_MP3TORIFFWAV, OnMp3toriffwav)
	ON_UPDATE_COMMAND_UI(ID_MP3TORIFFWAV, OnUpdateMp3toriffwav)
	ON_COMMAND(ID_EDIT_RENAMETRACK, OnEditRenametrack)
	ON_COMMAND(ID_EDIT_SELECTALL, OnEditSelectall)
	ON_COMMAND(ID_EDIT_REFRESH, OnEditRefresh)
	ON_COMMAND(ID_EDIT_COPYTRACKSTOCLIPBOARD, OnEditCopytrackstoclipboard)
	ON_COMMAND(ID_CDDB_SAVETOCDPLAYERINI, OnCddbSavetocdplayerini)
	ON_COMMAND(ID_CDDB_SAVETOLOCALCDDB, OnCddbSavetolocalcddb)
	ON_COMMAND(ID_CONFIGCD, OnConfigcd)
	ON_UPDATE_COMMAND_UI(ID_VIEWERRORLOG, OnUpdateViewerrorlog)
	ON_COMMAND(ID_MP3TOWAV, OnMp3towav)
	ON_UPDATE_COMMAND_UI(ID_MP3TOWAV, OnUpdateMp3towav)
	ON_COMMAND(ID_VIEWRIPSTATUS, OnViewripstatus)
	ON_COMMAND(ID_COMPAREFILES, OnComparefiles)
	ON_COMMAND(ID_PLAYFILES, OnPlayfiles)
	ON_UPDATE_COMMAND_UI(ID_PLAYFILES, OnUpdatePlayfiles)
	ON_COMMAND(ID_OPTION_CLEARSTATUS, OnOptionClearstatus)
	ON_COMMAND(ID_RIFFWAVTOMP3, OnRiffwavtomp3)
	ON_COMMAND(ID_TOOLS_RECORDFILES, OnToolsRecordfiles)
	ON_COMMAND(ID_CDDB_SUBMITTOREMOTECDDB, OnCddbSubmittoremotecddb)
	ON_COMMAND(ID_CDDB_READCDTEXT, OnCddbReadcdtext)
	ON_COMMAND(ID_CDDB_BATCH_QUERY, OnCddbBatchQuery)
	ON_COMMAND(ID_MPEGTOMPEG, OnMpegtompeg)
	ON_UPDATE_COMMAND_UI(ID_MPEGTOMPEG, OnUpdateMpegtompeg)
	ON_COMMAND(ID_CONFIGURE_WIN_PLUGINS, OnConfigureWinPlugins)
	ON_UPDATE_COMMAND_UI(ID_CONFIGURE_WIN_PLUGINS, OnUpdateConfigureWinPlugins )
	ON_CBN_DBLCLK(IDC_GENRE, OnDblclkGenre)
	ON_COMMAND(ID_EDIT_ADDEDITGENRES, OnEditAddeditgenres)
	ON_COMMAND(ID_EDIT_MODIFYTRACKNAME_EXCHANGEARTISTTRACK, OnEditModifytracknameExchangeartisttrack)
	ON_EN_CHANGE(IDC_TRACKOFFSET, OnChangeTrackoffset)
	ON_COMMAND(ID_DELETEPROFILE, OnDeleteprofile)
	ON_UPDATE_COMMAND_UI(ID_DELETEPROFILE, OnUpdateDeleteprofile)
	ON_COMMAND(ID_SAVEPROFILE, OnSaveprofile)
	ON_UPDATE_COMMAND_UI(ID_SAVEPROFILE, OnUpdateSaveprofile)
	ON_COMMAND(ID_CDDB_WINAMPDB, OnCddbWinampdb)
	ON_UPDATE_COMMAND_UI(ID_CDDB_WINAMPDB, OnUpdateCddbWinampdb)
	ON_UPDATE_COMMAND_UI(ID_EJECT, OnUpdateEject)
	ON_COMMAND(ID_EJECT, OnEject)
	ON_CBN_SELCHANGE(IDC_GENRE, OnChange)
	ON_EN_CHANGE(IDC_TITLE, OnChange)
	ON_EN_CHANGE(IDC_YEAR, OnChange)
	ON_UPDATE_COMMAND_UI(ID_REMOTECDDB, OnUpdateCDDB)
	ON_UPDATE_COMMAND_UI(ID_READCDPLAYERINI, OnUpdateCDDB)
	ON_UPDATE_COMMAND_UI(ID_CDDB_READCDTEXT, OnUpdateCDDB)
	ON_UPDATE_COMMAND_UI(ID_CDDB_SAVETOLOCALCDDB, OnUpdateCDDB)
	ON_UPDATE_COMMAND_UI(ID_CDDB_SUBMITTOREMOTECDDB, OnUpdateCDDB)
	ON_EN_KILLFOCUS(IDC_TRACKOFFSET, OnKillfocusTrackoffset)
	//}}AFX_MSG_MAP

	ON_COMMAND(ID_CDDB_READFROMMUSICBRAINZ, OnMbrainz)

	ON_MESSAGE(WM_UPDATE_CDSELECTION, OnChangeCDSelection )
	ON_MESSAGE(WM_UPDATE_PROFILESELECTION, OnChangeProfileSelection )

	ON_MESSAGE(WM_SEEKBUTTONVALUECHANGED, OnSeekButton )

	ON_MESSAGE(WM_USER+2, OnWinAmpPlugInFinished )

	ON_MESSAGE(WM_EDITNEXTTRACK, OnEditNextTrack )
	ON_MESSAGE(WM_SEEKBUTTONUP, OnSeekButtonUp)
	ON_MESSAGE(WM_UPDATETRACKSTATUS, OnUpdateTrackStatus)

	ON_MESSAGE(WM_DIALOG_CLOSED, OnChildDialogClosed)
	ON_COMMAND(ID_TRACKNAME_MOD_AL, OnTracknameModAl)
	ON_COMMAND(ID_TRACKNAME_MOD_AU, OnTracknameModAu)
	ON_COMMAND(ID_TRACKNAME_MOD_FCC, OnTracknameModFcc)
	ON_COMMAND(ID_TRACKNAME_MOD_FCEW, OnTracknameModFcew)
	ON_UPDATE_COMMAND_UI(ID_TRACKNAME_MOD_AL, OnUpdateTracknameModAl)
	ON_UPDATE_COMMAND_UI(ID_TRACKNAME_MOD_AU, OnUpdateTracknameModAu)
	ON_UPDATE_COMMAND_UI(ID_TRACKNAME_MOD_FCC, OnUpdateTracknameModFcc)
	ON_UPDATE_COMMAND_UI(ID_TRACKNAME_MOD_FCEW, OnUpdateTracknameModFcew)

	ON_CBN_SELENDOK( ID_PROFILESELECTION, OnProfileEditChange )

    ON_COMMAND(ID_TOOLS_GENERATECUESHEET, OnToolsGeneratecuesheet)
    ON_COMMAND(ID_TOOLS_DETECTPRE, OnToolsDetectpre)
    ON_COMMAND(ID_TOOLS_READUPC, OnToolsReadupc)
    END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CCDexView construction/destruction

CCDexView::CCDexView()
	: CFormView(CCDexView::IDD)
{
	ENTRY_TRACE( _T( "CCDexView::CCDexView" ) );

	m_pStatus = NULL;

	//{{AFX_DATA_INIT(CCDexView)
	m_strYear = _T("");
	m_strTitle = _T("");
	m_strArtist = _T("");
	m_nTrackOffset = 0;
	//}}AFX_DATA_INIT

	m_nCurPlayTrack=0;
	m_nNumPlayTracks=0;
	memset(m_PlayTracksArray,0x00,sizeof(m_PlayTracksArray));
	m_lOldStatus=INIT;
	m_pFileDlg=NULL;

	m_noPreGaps = true;
    m_noIsrc = true;
	EXIT_TRACE( _T( "CCDexView::CCDexView" ) );
}


CCDexView::~CCDexView()
{
	delete m_pFileDlg;
}

void CCDexView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCDexView)
	DDX_Control(pDX, IDC_YEAR, m_Year);
	DDX_Control(pDX, IDC_TITLE, m_Title);
	DDX_Control(pDX, IDC_ARTIST, m_Artist);
	DDX_Control(pDX, IDC_GENRE, m_Genre);
	DDX_Control(pDX, IDC_TRACKLIST, m_TrackList);
	DDX_Text(pDX, IDC_YEAR, m_strYear);
	DDX_Text(pDX, IDC_TITLE, m_strTitle);
	DDX_Text(pDX, IDC_ARTIST, m_strArtist);
	DDX_Text(pDX, IDC_TRACKOFFSET, m_nTrackOffset);
	DDV_MinMaxInt(pDX, m_nTrackOffset, -9999, 9999);
	DDX_Control(pDX, IDC_TRACKOFFSET, m_TrackOffset);
	//}}AFX_DATA_MAP
}



BOOL CCDexView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CFormView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CCDexView drawing

void CCDexView::OnDraw(CDC* pDC)
{
	CCDexDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
}

/////////////////////////////////////////////////////////////////////////////
// CCDexView printing

BOOL CCDexView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CCDexView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CCDexView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

/////////////////////////////////////////////////////////////////////////////
// CCDexView diagnostics

#ifdef _DEBUG
void CCDexView::AssertValid() const
{
	CFormView::AssertValid();
}

void CCDexView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CCDexDoc* CCDexView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CCDexDoc)));
	return (CCDexDoc*)m_pDocument;
}
#endif //_DEBUG


void CCDexView::OnConfigure() 
{
	ENTRY_TRACE( _T( "CCDexView::OnConfigure" ) );

	// By default select the encoder TAB
	OnSelectConfigure( 3 );

	EXIT_TRACE( _T( "CCDexView::OnConfigure" ) );

}

void CCDexView::OnSelectConfigure( int nSelectTab ) 
{
	ENTRY_TRACE( _T( "CCDexView::OnSelectConfigure( %d )" ), nSelectTab );

	// Save the CD Info
	SaveCDInfo();

	// Make a copy of the current configuration
	CConfig oldConfig( g_config );

	// Create dialog box
	ConfigSheet dlg(nSelectTab);

	// Change status to recording
	m_lOldStatus=m_lStatus;
	m_lStatus=CONFIGURE;


	// And show it
	if ( IDOK == dlg.DoModal() )
	{
		OnChangeCDSelection( -1, 0 );
	}
	else
	{
		// Revert to old settings
		g_config = oldConfig;
	}

	// Set status to old status
	m_lStatus = m_lOldStatus;

	EXIT_TRACE( _T( "CCDexView::OnSelectConfigure( )" ) );

//	UpdateData(FALSE);
}

void CCDexView::OnInitialUpdate() 
{
	ENTRY_TRACE( _T( "CCDexView::OnInitialUpdate()" ) );

	CUString		strLang;

	m_pCopyDlg = NULL;
	m_pConvertDlg = NULL;

	// init controls (depending on language setting)
	OnUpdate( this, WM_CDEX_INITIAL_UPDATE, NULL );

	// Load the genre table
	(void)g_GenreTable.Load( GENRE_TABLE_FILE_NAME );

	// Close door if necessary
	CR_EjectCD( FALSE );
	m_bEjected=FALSE;

	m_dwTimer=GetTickCount();
	if ( m_dwTimer> ( UINT_MAX -2000 ) )
		m_dwTimer=1000;

	m_pStatus=(CStatusBar *)AfxGetApp()->m_pMainWnd->GetDescendantWindow(AFX_IDW_STATUS_BAR);

	// Allow Drag and dropping of files
	DragAcceptFiles();

	GetDocument()->SetModifiedFlag(TRUE);

	m_lStatus=INIT;

	CFormView::OnInitialUpdate();

	CString strGenre( g_language.GetString( IDS_UNKNOWN ) );

	UpdateGenreSelection( strGenre );

    int numCols = sizeof( titleHeaderTable ) / sizeof( titleHeaderTable[0] );

    // determine multiply factor
    for ( int tableIdx = 0; tableIdx < numCols; tableIdx++ )
    {
        CUStringConvert strCnv;

        // Create columns of track list
	    strLang = g_language.GetString( titleHeaderTable[ tableIdx ].IDLang );
	    m_TrackList.InsertColumn( titleHeaderTable[ tableIdx ].ID, strCnv.ToT( strLang ), titleHeaderTable[ tableIdx ].textAlignment, 0, 0 );
    }


    // default hide the GAP and ISRC column
    m_TrackList.HideColumn( TL_GAP );
    m_TrackList.HideColumn( TL_ISRC );
    
    // Align the stuff
	PlaceControls();

	// Setup the timer
	SetTimer( TIMERID, TIMERSLOT, NULL );

	InitWinAmpPlugins( GetSafeHwnd() );

	m_pUtilToolBar = ((CMainFrame*)AfxGetMainWnd())->GetUtilToolBar();

	AddProfileStrings();


	OnUpdate(this,0,NULL);

	((CCDexApp*)AfxGetApp())->SetInitialized();

	EXIT_TRACE( _T( "CCDexView::OnInitialUpdate()" ) );
}

void CCDexView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint) 
{
	ENTRY_TRACE( _T( "CCDexView::OnUpdate, hint=%d" ), lHint );

	CUString strValue( g_language.GetString( IDS_CDEX_VERSION ) );

    CUStringConvert strCnv;

	// Set proper window title
	AfxGetApp()->m_pMainWnd->SetWindowText( strCnv.ToT( strValue + g_config.GetVersion() ) );

	switch ( lHint )
	{
		case WM_CDEX_UPDATE_TRACKSTATUS:
			// Update Track Status Information
			UpdateTrackStatus();
		break;
		case WM_CDEX_INITIAL_UPDATE:
			g_language.InitDialogStrings( (CDialog*)this, IDD_TITLEINFO );
			UpdateListControls();
		break;

		case WM_CDEX_SAVE_CD_INFO:
			// Save the stuff
			SaveCDInfo();
		break;

		case INIT:

			// Start with "fresh" document
			GetDocument()->SaveToIni( FALSE );
			GetDocument()->InitCDRom();
			{
			    CMainFrame* pMyMain = (CMainFrame*)AfxGetMainWnd();

			    CPlayToolBar* pCDInfoBar = ((CMainFrame*)AfxGetMainWnd())->GetPlayToolBar();

			    if ( NULL != pCDInfoBar )
			    {
				    pCDInfoBar->UpdateCDSelection();
			    }

			    // INIT done, switch status to NONE
			    m_lStatus = NONE;
			}
		break;

		case IDLE:
		break;

		case NEWCD:
			m_lStatus=IDLE;
			m_bEjected=FALSE;
			OnNewCD();
		break;

		case EJECTCD:
			// m_lStatus=NONE;
			// m_TrackList.DeleteAllItems();
		break;

		default:
			ASSERT( FALSE );
		break;
	}

	EXIT_TRACE( _T( "CCDexView::OnUpdate" ) );
}

void CCDexView::OnNewCD()
{
	ENTRY_TRACE( _T( "CCDexView::OnNewCD" ) );

	CCDexDoc* pDoc=GetDocument();

	m_noPreGaps = true;
	m_noIsrc = true;

	BOOL bUnitReady = CR_IsUnitReady();

	LTRACE2( _T( "CCDexView::OnNewCD, bUnitReady = %d " ), bUnitReady );

	DWORD oldVolID = pDoc->GetCDInfo().GetVolID();

	if ( bUnitReady )
	{
		// Set atatus to IDLE
		m_lStatus=IDLE;

		CWaitCursor Wait;

		// Get TOC + info from CDDB
		pDoc->ReadCDInfo();

        // default hide the GAP and ISRC column
        m_TrackList.HideColumn( TL_GAP );
        m_TrackList.HideColumn( TL_ISRC );

        PlaceControls();

		// Enable the Tracklist
		m_TrackList.EnableWindow( TRUE );



	}

	if ( oldVolID != pDoc->GetCDInfo().GetVolID() ) {
		UpdateCDInfo();
	}

	// Start RIP?
	if ( ((CCDexApp*)AfxGetApp())->GetAutoRip() || g_config.GetAutoRip() )
	{
		LTRACE( _T( "CCDexView::UpdateCDInfo() start AUTO rip" ) );
		PostMessage( WM_COMMAND, ID_TRACKS2MP3, 0 );
	}

	EXIT_TRACE( _T( "CCDexView::OnNewCD" ) );
}


void CCDexView::UpdateTrackStatus()
{
	int nTrack = 0;
	CCDexDoc* pDoc=GetDocument();

	ENTRY_TRACE( _T( "CCDexView::UpdateTrackStatus()" ) );

	// How many tracks do we have?
	int nNumTracks=pDoc->GetCDInfo().GetNumTracks();

	// Loop trough the tracks and fill in the data
	for ( nTrack = 0; nTrack < nNumTracks; nTrack++ )
	{
		CUString strTmp;
        CUString strCRC;

		DWORD	dwJitterErrors = 0;

        strCRC = _T( "-" );
        ULONG ulCRC = 0;

		if ( m_RipInfoDB.GetCRC( nTrack + 1, ulCRC ) )
        {
            strCRC.Format( _W( "%08x" ), ulCRC );
        }

		CDEX_ERR nStatusInfo = m_RipInfoDB.GetStatusInfo( nTrack + 1, dwJitterErrors );

		switch ( nStatusInfo )
		{
			case CDEX_RIPPING_DONE:
				strTmp = g_language.GetString( IDS_STATUS_OK );
			break;
			case CDEX_ERROR:
				strTmp.Format( _W( "A" ) );
			break;
			case CDEX_JITTER_ERROR:
				strTmp.Format( _W( "X %d" ), dwJitterErrors );
			break;
			default:
				strTmp= _W( "-" );
			break;
		}

        CUStringConvert strCnv;

        m_TrackList.SetItemText( nTrack, TL_TRACKSTATUS, strCnv.ToT( strTmp ) );
        m_TrackList.SetItemText( nTrack, TL_CHECKSUM, strCnv.ToT( strCRC ) );
        
	}

	EXIT_TRACE( _T( "CCDexView::UpdateTrackStatus()" ) );

}

void CCDexView::UpdateCDInfo()
{
	int nTrack;
	CCDexDoc* pDoc = GetDocument();

	ENTRY_TRACE( _T( "CCDexView::UpdateCDInfo()" ) );

	// First Remove All entries
	m_TrackList.DeleteAllItems();

	// How many tracks do we have?
	int nNumTracks = pDoc->GetCDInfo().GetNumTracks();

	CUString strFileName;

	strFileName.Format( _W( "%08X" ), pDoc->GetCDInfo().GetDiscID() );
	// Set ripping file name
	m_RipInfoDB.SetFileName( strFileName );

    CDInfo& cdInfo = pDoc->GetCDInfo();

	// Loop trough the tracks and fill in the data
	for ( nTrack = 1; nTrack <= nNumTracks; nTrack++ )
	{
		CUString strTmp;

		// Which item has to be inserted ?
		int nInsertItem = nTrack - 1;

        CUStringConvert strCnv;

		// Okay, fill in the stuff
		m_TrackList.InsertItem( nInsertItem, strCnv.ToT( cdInfo.GetTrackName( nTrack-1 ) ) );

		strTmp.Format( _W( "%02d" ), cdInfo.GetTrack( nTrack-1 ) );
		m_TrackList.SetItemText( nInsertItem, TL_TRACKNUMBER, strCnv.ToT( strTmp ) );

		m_TrackList.SetItemText( nInsertItem, TL_STARTTIME, strCnv.ToT( cdInfo.GetStartTime( nTrack ) ) );

		m_TrackList.SetItemText( nInsertItem, TL_PLAYTIME, strCnv.ToT( cdInfo.GetTrackDuration( nTrack ) ) );

		strTmp.Format( _W( "%4.2f MB "), cdInfo.GetSize( nTrack ) / 1024 / 1024.0 );
		m_TrackList.SetItemText( nInsertItem, TL_TRACKSIZE, strCnv.ToT( strTmp ) );

		m_TrackList.SetItemText( nInsertItem, TL_TRACKSTATUS, _T( "-" ) );

        m_TrackList.SetItemText( nInsertItem, TL_ISRC, strCnv.ToT( cdInfo.GetISRC( nTrack - 1 ) ) );

        CUString strGap;
        DWORD dwGapSize = cdInfo.GetGap( nTrack - 1 );
        DWORD dwMins   = ( dwGapSize / 60 / TRACKSPERSEC );
        DWORD dwSecs   = ( dwGapSize / TRACKSPERSEC ) % 60;
        DWORD dwFrames = ( dwGapSize % TRACKSPERSEC );

	////DWORD dwSec = ( ( nSeekPos ) / TRACKSPERSEC ) % 60;
	////DWORD dwMin = ( ( nSeekPos ) / TRACKSPERSEC / 60 ) % 60;

        strGap.Format( _W( "%02d:%02d:%02d" ), dwMins, dwSecs, dwFrames );
        m_TrackList.SetItemText( nInsertItem, TL_GAP, strCnv.ToT( strGap ) );
    
    }
	// Update Track Status Information
	UpdateTrackStatus();

	// Get Artist name
	m_strArtist = cdInfo.GetArtist();

	// Get Title
	m_strTitle = cdInfo.GetTitle();

	// Get Year
	m_strYear = cdInfo.GetYear();

	// Get Category
    CUStringConvert strCnv;
	UpdateGenreSelection( strCnv.ToT( cdInfo.GetGenre() ) );

	if (g_config.GetSelectAllCDTracks())
	{
		// Select all tracks by default
		m_TrackList.SelectAll();
	}

	// reset track offset number
	m_nTrackOffset = 0;

	// Update controls
	UpdateData( FALSE );

	EXIT_TRACE( _T( "CCDexView::UpdateCDInfo()" ) );
}

void CCDexView::OnTimer( UINT nIDEvent ) 
{
	CUString	strLang;
	CUString strInfo;

	if ( RECORDING != m_lStatus && CONFIGURE != m_lStatus )
	{
		// poll the current status
		CDMEDIASTATUS cdStatus;
		
		if ( CDEX_OK == CR_IsMediaLoaded( cdStatus ) )
		{
			LTRACE2( _T( "CCDexView::CR_IsMediaLoaded() returned value %d" ), cdStatus );

			switch( m_lStatus )
			{
				case NONE:
					if ( CDMEDIA_PRESENT == cdStatus )
					{
						LTRACE2( _T( "New Media present!\n" ) );
						OnUpdate( this, NEWCD, NULL );
					}
				break;
				default:
					if ( CDMEDIA_PRESENT != cdStatus )
					{
						LTRACE2( _T( "Media not present!\n" ) );
						EjectCD( TRUE, FALSE );			
					}
			}
		}
		else
		{
			EjectCD( TRUE, FALSE );			
		}

		int nSelected=-1;
		int nItems=0;
		DOUBLE dSize=0;
		if ( NULL != m_TrackList.m_hWnd )
		{
			while( (nSelected=m_TrackList.GetNextItem(nSelected,LVNI_SELECTED))!=-1)
			{
				float fSize;

				nItems++;
				CUString strTmp;

				strTmp = CUString( m_TrackList.GetItemText( nSelected, 4 ) );

				swscanf( strTmp, _W( "%f" ), &fSize );

				dSize += (double)fSize;
			}
			DOUBLE dFreeDiskSpace=GetMyFreeDiskSpace(g_config.GetMP3OutputDir())/(1024.0*1024.0);

			strLang = g_language.GetString( IDS_FREEDISKSPACE );

			strInfo.Format( strLang, nItems, dSize, (INT)dFreeDiskSpace );
		}
	}

	if (m_lStatus==PLAYING)
	{
		if ( !m_cCDPlay.IsPlaying() )
		{
			// Guess not, go to next track if available
			if (m_nCurPlayTrack<m_nNumPlayTracks && m_nCurPlayTrack>=0)
			{
				DWORD dwTrack = m_PlayTracksArray[ m_nCurPlayTrack++ ];

				m_cCDPlay.Play( GetDocument()->GetCDInfo().GetStartSector( dwTrack ),
								GetDocument()->GetCDInfo().GetSaveEndSector( dwTrack ) );
			}
			else
			{
				// Finished playing, update status
				m_lStatus = m_lOldStatus;
			}
		}
		else
		{
			DWORD dwSecs = m_cCDPlay.GetCurrentTimeInSecs();

			strLang = g_language.GetString( IDS_PLAYINGTRACKPOSITION );

			strInfo.Format( strLang, m_PlayTracksArray[ max( m_nCurPlayTrack - 1, 0 ) ], ( ( dwSecs / 60 / 60 ) % 60 ), ( ( dwSecs / 60 ) % 60 ), ( dwSecs % 60 ) );
		}
	}


	// Update Status Bar

//	if (!strInfo.IsEmpty())
	{
        CUStringConvert strCnv;
		SetStatusBar( strCnv.ToT( strInfo ) );
	}

	CFormView::OnTimer( nIDEvent );
}

void CCDexView::PlaceControls() 
{
	ENTRY_TRACE( _T( "CCDexView::PlaceControls()" ) );

	if ( m_TrackList.m_hWnd )
	{

		CRect	rcList;
		CRect	rcParent;

		// Get size of parent window
		GetClientRect( rcParent );

		// Get size position of Track List
		m_TrackList.GetWindowRect( rcList );

		// Convert from screen to client (this parent window of course)
		ScreenToClient( rcList );

		rcList.right = rcParent.right - 10;
		rcList.bottom = rcParent.bottom;

		// And adjust the size of the track list
		m_TrackList.MoveWindow( rcList );

        double totalWidth = 0.0;

        int numCols = sizeof( titleHeaderTable ) / sizeof( titleHeaderTable[0] );

        // determine multiply factor
        for ( int tableIdx = 0; tableIdx < numCols; tableIdx++ )
        {
            if ( !m_TrackList.IsColumnHidden( tableIdx ) )
            {
                totalWidth += titleHeaderTable[ tableIdx ].width;
            }
        }

        double toPixFactor = ((double)rcList.Width() - 24 ) / totalWidth;
       
        // if window is not wide enough, scale it up to avoid unreadable columns
        if ( rcList.Width() < 600 )
        {
            double scale = 600.0 / (double)rcList.Width();
            toPixFactor *= scale;
        }

        // assign column widhts
        for ( int index = 0; index < numCols; index++ )
        {
            if ( !m_TrackList.IsColumnHidden( index ) )
            {
                int width = (int)( toPixFactor * titleHeaderTable[ index ].width  );
        		m_TrackList.SetColumnWidth( index, width );
            }
            else
            {
        		m_TrackList.SetColumnWidth( index, 0  );
            }
        }
	}

	EXIT_TRACE( _T( "CCDexView::PlaceControls()" ) );
}


void CCDexView::OnSize( UINT nType, int cx, int cy ) 
{
	CFormView::OnSize( nType, cx, cy );
	PlaceControls();
	
}

void CCDexView::OnTracks2wav() 
{
	OnGetTrack2XXX( WAVETYPE );
}

void CCDexView::OnTracks2mp3() 
{
	OnGetTrack2XXX( MP3TYPE );
}

void CCDexView::OnWav2mp3() 
{
	CUString	strLang;

	DWORD dwTrack = 1;

	ENTRY_TRACE( _T( "CCDexView::OnWav2mp3()" ) );

	// create custom open file dialog
	COpenFileDlg fileDlg( g_config.GetWAVInputDir(), _T( "WAV" ), IDI_FILE_ICON );

	fileDlg.m_bNormalize = g_config.GetNormTrack();
	fileDlg.m_bDeleteOriginal = g_config.GetDeleteWAV();
	fileDlg.m_bRiffWav = g_config.GetRiffWav();

	fileDlg.ShowDelOrigCtrl( TRUE );
	fileDlg.ShowNormalizeCtrl( TRUE );
	fileDlg.ShowRiffWavCtrl( TRUE );
	fileDlg.ShowKeepDirLayoutCtrl( TRUE );

	strLang = g_language.GetString( IDS_CONVERT_BUTTON_TEXT );
	fileDlg.SetOpenButtonText( strLang );

	// check if OK button has been pressed
    if ( IDOK == fileDlg.DoModal() ) 
    {
		g_config.SetNormTrack(fileDlg.m_bNormalize);
		g_config.SetDeleteWAV(fileDlg.m_bDeleteOriginal);
		g_config.SetRiffWav(fileDlg.m_bRiffWav);

		// Get values out of controls
		UpdateData(TRUE);

		// Create new modeless dialog
		m_pCopyDlg=new CCopyDialog(this);

		// Save to the config file
		g_config.SetWAVInputDir(fileDlg.GetDir());

		// Save status
		m_lOldStatus=m_lStatus;

		// Indicate start of recording
		m_lStatus=RECORDING;

		// Get input directory
		POSITION pos=fileDlg.GetStartPosition();

		while (pos!=NULL)
		{
			CTaskInfo newTask;
            CTagData tagData;

			POSITION nFilePos = pos;
			// Add new record item

			CUString strSubPath( fileDlg.GetSubPath( nFilePos ) );

			// set full file name
			newTask.SetFullFileName( fileDlg.GetNextPathName( pos ));

			// set output dir
			newTask.SetOutDir( g_config.GetCnvOutputDir() + strSubPath );

			newTask.SetEncoderType( ENCODER_TYPES( g_config.GetEncoderType() ) );
			newTask.SetRip( FALSE );
			newTask.SetNormalize( fileDlg.m_bNormalize );
			newTask.SetConvert( TRUE );
			newTask.SetDeleteWav( fileDlg.m_bDeleteOriginal );
			newTask.SetRiffWav( g_config.GetRiffWav() );

			tagData.SetTrackNumber( dwTrack );
			tagData.SetTrackOffset( 0 );

            newTask.SetTagData( tagData );
			m_pCopyDlg->GetTasks().AddTaskInfo( newTask );

			dwTrack++;
		}

		// Start the Wave File conversion dialog
		ConvertWav2MP3();
    }

	EXIT_TRACE( _T( "CCDexView::OnWav2mp3()" ) );

}

void CCDexView::OnSysteminfo() 
{
	CGenInfoDlg dlg( GetDocument() );
	dlg.DoModal();
}


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

void SaveCueSheet(CCDexView& view, const PCopyDlg& dlg, CDInfo& cdInfo)
{
	CUString cueSheetName = g_config.GetMP3OutputDir();
    cueSheetName += dlg.GetFileName();
    cueSheetName += _W( ".cue" );

	FILE* cueSheet = CDexOpenFile( cueSheetName, _T( "w" ) );
	if (cueSheet == NULL) return;

	auto_ptr<CEncoder> pEncoder( EncoderObjectFactory( ENCODER_TYPES(g_config.GetEncoderType()) ) );
	bool isMP3Enc = (pEncoder->GetExtension() == _T( "mp3" ));

	if (! cdInfo.GetGenre().IsEmpty())
		tFPRINTF(cueSheet, _T("REM GENRE \"%s\"\n"), cdInfo.GetGenre());
	if (! cdInfo.GetYear().IsEmpty())
		tFPRINTF(cueSheet, _T("REM DATE %s\n"), cdInfo.GetYear());
	tFPRINTF(cueSheet, _T("REM DISCID %08lx\n"), cdInfo.GetDiscID());
	tFPRINTF(cueSheet, _T("REM VOLID %lX\n"), cdInfo.GetVolID());
	tFPRINTF(cueSheet, _T("REM CDDBCAT %s\n"), cdInfo.GetCDDBCat());
    tFPRINTF(cueSheet, _T("REM COMMENT \"CDex %s\"\n"), g_config.GetVersion());
	tFPRINTF(cueSheet, _T("REM\n"));
    tFPRINTF(cueSheet, _T("PERFORMER \"%s\"\n"), cdInfo.GetArtist());
	tFPRINTF(cueSheet, _T("TITLE \"%s\"\n"), cdInfo.GetTitle());
	if (isMP3Enc && dlg.GetOutputType() == MP3TYPE)
		tFPRINTF(cueSheet, _T("FILE \"%s.mp3\" MP3\n"), dlg.GetFileName());
	else
		tFPRINTF(cueSheet, _T("FILE \"%s.wav\" WAVE\n"), dlg.GetFileName());

	CTaskInfo newTask;
	int	count = 1;
	int offset = 0; // dlg.GetFirstTrack() == 0 ? cdInfo.GetStartSector(cdInfo.GetTrack( 0 )) : 0;

	for (int i = dlg.GetFirstTrack(); i <= dlg.GetLastTrack(); ++i, ++count)
	{
		int n = view.SetRibDBInfo( newTask, i );

		tFPRINTF(cueSheet, _T("  TRACK %02d AUDIO\n"), count);
        tFPRINTF(cueSheet, _T("    TITLE \"%s\"\n"), newTask.GetTagData().GetTitle());
		if (g_config.GetSplitTrackName())
            tFPRINTF(cueSheet, _T("    PERFORMER \"%s\"\n"), newTask.GetTagData().GetArtist());

		DWORD dwStart = offset + cdInfo.GetStartSector(cdInfo.GetTrack( i )) - dlg.GetFirstSector();

		// Calculate sectors to time
		int nF= (int)( dwStart % 75 );
		int nS= (int)( (dwStart / 75 ) %60 );
		int nM= (int)( (dwStart / 75 ) /60 );

		tFPRINTF(cueSheet, _T("    INDEX 01 %02d:%02d:%02d\n"), nM, nS, nF);
	}
	fclose(cueSheet);
}


void CCDexView::OnPtracks2mp3() 
{
	INT			nFirstSelected = -1;
	INT			nLastSelected = -1;
	BOOL		bHaveAudioTracks = FALSE;
	PCopyDlg	dlg;
	int			nNumTracks = GetDocument()->GetCDInfo().GetNumTracks();
	CUString		strLang;

	ENTRY_TRACE( _T( "CCDexView::OnPtracks2mp3()" ) );

	// Get latest values
	UpdateData( TRUE );

	// get a pointer to the document
	CCDexDoc* pDoc = GetDocument();
	ASSERT_VALID( pDoc );

	// Get latest values
	SaveCDInfo();

	// helper, just to make life a little easier
	CDInfo& cdInfo=pDoc->GetCDInfo();

	// Fill the track list for the selection dialog box
	for (int i = 0; i < nNumTracks; i++ )
	{
		WORD wTrack=cdInfo.GetTrack( i );

		// Check if there is at least one audio track
		if (cdInfo.IsAudioTrack( wTrack ) )
		{
			CTaskInfo newTask;

			// indicate that we have at least one track
			bHaveAudioTracks = TRUE;

			
			CUString strEncExt;

            CTagData& tagData( newTask.GetTagData() );

			newTask.SetFullFileName( cdInfo.GetTrackName(i) + _W( ".mp3" ) );
            newTask.GetTagData().SetTitle( cdInfo.GetTrackName( i ) );
			newTask.SetStartSector( cdInfo.GetStartSector( wTrack ) );
			newTask.SetEndSector( cdInfo.GetSaveEndSector( wTrack ) );
			
            tagData.SetTrackNumber( wTrack );
			tagData.SetTotalTracks( nNumTracks );
			tagData.SetTrackOffset( m_nTrackOffset );
            tagData.SetComment( g_config.GetID3Comment() );
            tagData.SetEncodedBy( g_config.GetID3EncodedBy() );

			// Check also which tracks are currently selected
			if ( m_TrackList.GetItemState( i, LVIF_STATE | LVIS_SELECTED ) == LVIS_SELECTED )
			{
				if ( nFirstSelected == -1 )
					nFirstSelected = i;
				nLastSelected = i;
			}

			dlg.GetTasks().AddTaskInfo( newTask );

		}
	}

	if ( nFirstSelected == -1 )
		nFirstSelected = 0;

	if ( nLastSelected == -1 )
		nLastSelected = 0;

	// Exit when no audio trackes are present
	if ( !bHaveAudioTracks )
	{
		return;
	}

	// set first selected track
	dlg.SetFirstTrack( nFirstSelected );

	// set last selected track
	dlg.SetLastTrack( nLastSelected );


	// Set the output type ( WAV of MP3 )
	dlg.SetOutputType( g_config.GetPCopyEncType() );

	// set the file name
//	strLang = g_language.GetString( IDS_PCOPY_RANGE );

//	dlg.SetFileName( dlg.GetTasks().GetTaskInfo( nFirstSelected ).GetTrackName( ) + strLang );

 	tCHAR buf[256];
 	if (nLastSelected - nFirstSelected == nNumTracks - 1)
 		tSPRINTF(buf, _T("%s - %s"), cdInfo.GetArtist(), cdInfo.GetTitle());
 	else
 		tSPRINTF(buf, _T("%s - %s (trk %d-%d)"), cdInfo.GetArtist(), cdInfo.GetTitle(), cdInfo.GetTrack(nFirstSelected), cdInfo.GetTrack(nLastSelected));
 	dlg.SetFileName( CUString(buf) );

    dlg.SetFileName( dlg.GetTasks().GetTaskInfo( nFirstSelected ).GetTagData().GetTitle( ) + strLang );

	// show the dialog
	if ( IDOK == dlg.DoModal() )
	{
		CTaskInfo newTask;

		SaveCueSheet(*this, dlg, cdInfo);

		// Get proper output type (WAV or MP3)
		int nWavOrMp3 = dlg.GetOutputType();

		// Create new modeless dialog
		m_pCopyDlg = new CCopyDialog(this);

		// Set track info
		DWORD n = SetRibDBInfo( newTask, 0 );

        CTagData& tagData( newTask.GetTagData() );


		// Fill out the record items
		newTask.SetFullFileName( g_config.GetMP3OutputDir() + FixFileNameString( dlg.GetFileName() ) );
		newTask.SetOutDir( g_config.GetMP3OutputDir() );
		newTask.SetStartSector( dlg.GetFirstSector() );
		newTask.SetEndSector( dlg.GetLastSector() );
		tagData.SetTrackNumber( dlg.GetStartTrack() );
		tagData.SetTrackOffset( m_nTrackOffset );

        // set track name
        tagData.SetTitle( dlg.GetFileName() );

		// Make the output type persistant
		g_config.SetPCopyEncType( dlg.GetOutputType() );

		// type specific
		if ( WAVETYPE == nWavOrMp3 )
		{
			newTask.SetEncoderType( ENCODER_FIXED_WAV );

			// set file extention
			newTask.SetFileExt( _T( "wav" ) );

			// No conversion required
			newTask.SetConvert( FALSE );

			// Do NOT Delete the WAV file
			newTask.SetDeleteWav( FALSE );
		}
		else
		{
			ENCODER_TYPES nEncoderType = ENCODER_TYPES( g_config.GetEncoderType() );

			auto_ptr<CEncoder> pEncoder( EncoderObjectFactory( nEncoderType ) );

			newTask.SetEncoderType( ENCODER_TYPES( nEncoderType ) );

			// set file extention
			if ( NULL != pEncoder.get() )
			{
				newTask.SetFileExt( pEncoder->GetExtension() );
			}

			// Conversion required
			newTask.SetConvert( TRUE );
			// Delete the WAV file
			newTask.SetDeleteWav( TRUE );
		}


		// Save status
		m_lOldStatus = m_lStatus;

		// Indicate start of recording
		m_lStatus = RECORDING;

		// Yes, the files have to be ripped form CD
		newTask.SetRip( TRUE );

		// Normalize the file?
		newTask.SetNormalize( g_config.GetNormTrack() );

		// Convert to a Riff Wav file
		newTask.SetRiffWav( g_config.GetRiffWav() );

		m_pCopyDlg->GetTasks().AddTaskInfo( newTask );

		// Create the modeless dialog box
		m_pCopyDlg->Create( IDD_COPYDIALOG, this );

		// Show the dialog
		m_pCopyDlg->ShowWindow( SW_SHOW );

		// Center the window
		m_pCopyDlg->CenterWindow();

		// Update the window
		m_pCopyDlg->UpdateWindow();
	}

	EXIT_TRACE( _T( "CCDexView::OnPtracks2mp3()" ) );
}


void CCDexView::OnStopbutton() 
{
	// And stop the playing
	m_cCDPlay.Stop();

	// Update Status
	m_lStatus=IDLE;
}

void CCDexView::EjectCD( BOOL bEject,BOOL bOpenTray ) 
{
	ENTRY_TRACE( _T( "CCDexView::EjectCD( %d , %d )" ), bEject, bOpenTray );

	if ( TRUE == bEject )
	{
		// CD has been ejected, set status to NONE
		m_lStatus = NONE;

		// Save the stuff
		SaveCDInfo();

		if ( bOpenTray && CR_IsUnitReady() )
		{
			// Just in case
			m_cCDPlay.Stop();
		}

		// Clear Artist and Title information
		UpdateData( FALSE );

		// Disable the Tracklist
		m_TrackList.EnableWindow( FALSE );

		m_bEjected = TRUE;

		// clear CD Information
		GetDocument()->GetCDInfo().Init();
	}
	else
	{
		m_bEjected = FALSE;
	}


	if ( TRUE == bOpenTray )
	{
		// Open or close CD tray
		CR_EjectCD( bEject );
	}

	EXIT_TRACE( _T( "CCDexView::EjectCD()" ) );
}


void CCDexView::OnEject() 
{
	ENTRY_TRACE( _T( "CCDexView::OnEject" ) );

	// Eject or load CD
	EjectCD(!m_bEjected,TRUE);

	EXIT_TRACE( _T( "CCDexView::OnEject" ) );
}


void CCDexView::OnUpdatePtracks2mp3(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ( m_lStatus==IDLE ) && ( GetDocument()->GetCDInfo().GetNumTracks() > 0 ) );
}

void CCDexView::OnUpdateSkipforward(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_lStatus==PLAYING);
}

void CCDexView::OnUpdateSkipback(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_lStatus==PLAYING);
}


void CCDexView::OnUpdateStopbutton(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_lStatus==PLAYING);
}

void CCDexView::OnUpdateTracks2mp3(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ( m_lStatus==IDLE ) && ( GetDocument()->GetCDInfo().GetNumTracks() > 0 ) );
}

void CCDexView::OnUpdateTracks2wav(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ( m_lStatus==IDLE ) && ( GetDocument()->GetCDInfo().GetNumTracks() > 0 ) );
}

void CCDexView::OnUpdateWav2mp3(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_lStatus==IDLE || m_lStatus==NONE);

	// Borrow this guy to update the edit controls as well
	if ( IDLE == m_lStatus )
	{
		m_Artist.EnableWindow( TRUE );
		m_Title.EnableWindow( TRUE );
		m_Year.EnableWindow( TRUE );
		m_Genre.EnableWindow( TRUE );
		m_TrackOffset.EnableWindow( TRUE );
	}
	else
	{
		m_Artist.EnableWindow( FALSE );
		m_Title.EnableWindow( FALSE );
		m_Year.EnableWindow( FALSE );
		m_Genre.EnableWindow( FALSE );
		m_TrackOffset.EnableWindow( FALSE );
	}
}

void CCDexView::OnChange() 
{
	GetDocument()->SaveToIni(TRUE);
}

void CCDexView::OnRemotecddb() 
{
	ENTRY_TRACE( _T( "CCDexView::OnRemotecddb()" ) );
	
	// Can take a whileCWaitCursor Wait;

	// Create Remote CDDB dialog box
//	CCDdbDlg dlg(ReadRemoteCDDBThreadFunc,&(GetDocument()->GetCDInfo()));

    CTagData tagData;
    tagData.SetCDBID( GetDocument()->GetCDInfo().GetDiscID() );
    tagData.SetTotalTracks( GetDocument()->GetCDInfo().GetNumTracks() );

    CGetRemoteCDInfoDlg dlg( &GetDocument()->GetCDInfo(), false );

	dlg.DoModal();

	// Update Data
	UpdateCDInfo();

	EXIT_TRACE( _T( "CCDexView::OnRemotecddb()" ) );
}

void CCDexView::OnUpdateCDDB(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ( m_lStatus==IDLE ) && ( GetDocument()->GetCDInfo().GetNumTracks() > 0 ) );
}


void CCDexView::OnUpdatePlaytrack(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_lStatus == IDLE);
}

void CCDexView::OnPause() 
{
	ENTRY_TRACE( _T( "CCDexView::OnPause()") );

	m_cCDPlay.Pause( !m_cCDPlay.IsPausing() );

	EXIT_TRACE( _T( "CCDexView::OnPause()" ) );
}

void CCDexView::OnUpdatePause( CCmdUI* pCmdUI ) 
{
	if ( m_lStatus == PLAYING )
	{
		pCmdUI->Enable( TRUE );

		pCmdUI->SetCheck( m_cCDPlay.IsPausing() );
	}
	else
	{
		pCmdUI->Enable( FALSE );
		pCmdUI->SetCheck( FALSE );
	}
}


// PLAYING STUFF
void CCDexView::OnPlaytrack() 
{
	m_lOldStatus=m_lStatus;

	GetPlayTracks();

	m_lStatus = PLAYING;

	DWORD dwTrack = m_PlayTracksArray[ m_nCurPlayTrack++ ];

	m_cCDPlay.Play( GetDocument()->GetCDInfo().GetStartSector( dwTrack ),
					GetDocument()->GetCDInfo().GetSaveEndSector( dwTrack ) );
}

void CCDexView::GetPlayTracks() 
{

	// Get the number of play tracks
	int nNumTracks = GetDocument()->GetCDInfo().GetNumTracks();

	// Clear current array.
	m_nNumPlayTracks = 0;
	m_nCurPlayTrack = 0;


	// add all files
	for ( int i = 0; i < nNumTracks; i++ )
	{
		m_PlayTracksArray[ m_nNumPlayTracks ] = i+1;
		m_nNumPlayTracks++;
	}

	// Loop through list and see which items are selected
	for (int i = 0; i < nNumTracks; i++ )
	{
		// get first selected track
		if ( LVIS_SELECTED == m_TrackList.GetItemState( i,LVIF_STATE|LVIS_SELECTED) )
		{
			m_nCurPlayTrack = i;
			break;
		}
	}
}


void CCDexView::OnNexttrack() 
{
	m_cCDPlay.Stop();

	DWORD dwTrack = m_PlayTracksArray[ m_nCurPlayTrack++ ];

	m_cCDPlay.Play( GetDocument()->GetCDInfo().GetStartSector( dwTrack ),
					GetDocument()->GetCDInfo().GetSaveEndSector( dwTrack ) );
}

void CCDexView::OnPrevtrack() 
{
	m_cCDPlay.Stop();

	m_nCurPlayTrack = max( m_nCurPlayTrack - 1, 0 );

	m_cCDPlay.Play( GetDocument()->GetCDInfo().GetStartSector( m_nCurPlayTrack ),
					GetDocument()->GetCDInfo().GetSaveEndSector( m_nCurPlayTrack ) );
}


void CCDexView::OnUpdateNexttrack(CCmdUI* pCmdUI) 
{
	if ( PLAYING == m_lStatus )
	{
		pCmdUI->Enable( m_nCurPlayTrack < m_nNumPlayTracks );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
}

void CCDexView::OnUpdatePrevtrack( CCmdUI* pCmdUI ) 
{
	pCmdUI->Enable( m_lStatus == PLAYING );
}


void CCDexView::OnRclickTracklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CMenu menu;
	menu.LoadMenu( IDR_RTRACK );

	// translate the menu items
	g_language.TranslateMenu( &menu, IDR_RTRACK, FALSE );

	POINT point;
	GetCursorPos( &point );

	// Get the item that's focused
	int nFocused=m_TrackList.GetNextItem( -1, LVNI_FOCUSED );

	if ( nFocused >= 0 )
	{
		menu.GetSubMenu(0)->TrackPopupMenu(	TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
											point.x,
											point.y,
											AfxGetMainWnd() );
	}

	*pResult = 0;
}

void CCDexView::OnEndlabeleditTracklist(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	m_TrackList.SetItem( &pDispInfo->item  );

	GetDocument()->SaveToIni( TRUE );

	*pResult = 0;

	// Edit next track?
	if ( m_TrackList.GetEditNext() )
	{
		PostMessage( WM_EDITNEXTTRACK, 0, 0 );
	}

	m_TrackList.SetEditNext( FALSE );
}


DWORD CCDexView::SetRibDBInfo( CTaskInfo& newTask, WORD i ) 
{
	DWORD nReturn = 0;

	CUString strName;
	CUString strDir;

    CTagData& tagData( newTask.GetTagData() );

	// get a pointer to the document
	CCDexDoc* pDoc = GetDocument();
	ASSERT_VALID( pDoc );

	// helper, just to make life a little easier
	CDInfo& cdInfo = pDoc->GetCDInfo();

	// Get current track number
	WORD wTrack = cdInfo.GetTrack( i );

	int nNumTracks = GetDocument()->GetCDInfo().GetNumTracks();

	// set path name to track name for now
	newTask.SetFullFileName( cdInfo.GetTrackName( i )+ _W( ".ext" ) );

	// set the output directory
	//newTask.SetOutDir(n,g_config.GetMP3OutputDir());

	// set start sector
	newTask.SetStartSector( cdInfo.GetStartSector( wTrack ) );

	// set end sector
	newTask.SetEndSector( cdInfo.GetSaveEndSector( wTrack ) );


	CUString strTrackName( cdInfo.GetTrackName( i ) );

	// Do we want to split the trackname
	if ( g_config.GetSplitTrackName() )
	{
		CUString strArtist( m_strArtist );
		int nPos=0;

		// set Non splityted artist name
		newTask.SetNonSplitArtist( strArtist );

		// Find the separator character
		if ( ( nPos = strTrackName.Find( g_config.GetSplitTrackChar() ) ) >0 )
		{
			// Split it up
			strArtist = strTrackName.Left( nPos );
			strTrackName = strTrackName.Right( strTrackName.GetLength() - nPos - 1 );

			strArtist.TrimLeft();
			strArtist.TrimRight();

			strTrackName.TrimLeft();
			strTrackName.TrimRight();
		}

		// set artist
		tagData.SetArtist( strArtist );
	}
	else
	{
		// set artist
		tagData.SetArtist( CUString( m_strArtist ) );

		// and also set Non splitted artist name
		newTask.SetNonSplitArtist( CUString( m_strArtist ) );
	}

	// set track name
	tagData.SetTitle( strTrackName );


	// set album
	tagData.SetAlbum( CUString( m_strTitle ) );

	// set year
	tagData.SetYear( CUString( m_strYear ) );

	// set genre string
	CString strGenre;
    if ( m_Genre.GetCurSel() >= 0 )
    {
	    m_Genre.GetLBText( m_Genre.GetCurSel(), strGenre );
    }
	tagData.SetGenre( CUString( strGenre ) );

	// set track number
	tagData.SetTrackNumber( wTrack );

	// set the track offset
	tagData.SetTrackOffset( m_nTrackOffset );

	// set total number of tracks
	tagData.SetTotalTracks( nNumTracks  );

	// set CDDB ID
	tagData.SetCDBID( cdInfo.GetDiscID() );

	// set Volume ID
	tagData.SetVOLID( cdInfo.GetVolID() );

	// set MCDI string
	CUString strTmp = cdInfo.GetMCDI();
	tagData.SetMCDI( cdInfo.GetMCDI() );

	// set RAW toc byte array
	tagData.SetRawToc( cdInfo.GetRawToc() );

    tagData.SetComment( g_config.GetID3Comment() );
    tagData.SetEncodedBy( g_config.GetID3EncodedBy() );

	// Build the file name string
	newTask.BuildFileName( strName, strDir );

	// Set the output file name
	newTask.SetFullFileName( g_config.GetMP3OutputDir() + strDir + strName + _W(".ext") );

	// Set the output directory
	newTask.SetOutDir(  g_config.GetMP3OutputDir() + strDir );

	newTask.SetValidTagData( true );

	return nReturn;
}

void CCDexView::OnGetTrack2XXX(int nWavOrMp3) 
{
	int			nNumTracks = GetDocument()->GetCDInfo().GetNumTracks();
	BOOL		bHaveAudioTracks = FALSE;
	CUString		strLang;

	// Get latest values
	SaveCDInfo();

	// get a pointer to the document
	CCDexDoc* pDoc = GetDocument();
	ASSERT_VALID( pDoc );

	// helper, just to make life a little easier
	CDInfo& cdInfo=pDoc->GetCDInfo();

	// Create new modeless dialog
	m_pCopyDlg = new CCopyDialog( this );

	// Fill the track list for the selection dialog box
	for (int i=0; i < nNumTracks; i++ )
	{
		CTaskInfo newTask;

		// Get current track number
		WORD wTrack=cdInfo.GetTrack( i );

		// Get the selected audio tracks
		if (	cdInfo.IsAudioTrack( wTrack ) && 
				m_TrackList.GetItemState( i, LVIF_STATE | LVIS_SELECTED ) == LVIS_SELECTED )
		{
			// indicate that we have at least one track
			bHaveAudioTracks = TRUE;

			// Set track info
			int n = SetRibDBInfo( newTask, i );

			// set proper file name extention
			if ( WAVETYPE == nWavOrMp3 )
			{
				newTask.SetEncoderType( ENCODER_FIXED_WAV );
				newTask.SetFileExt( _T( "wav" ) );
			}
			else
			{
				ENCODER_TYPES nEncoderType = ENCODER_TYPES( g_config.GetEncoderType() );

				auto_ptr<CEncoder> pEncoder( EncoderObjectFactory( nEncoderType ) );

				newTask.SetEncoderType( ENCODER_TYPES( nEncoderType ) );

				// set file extention
				if ( NULL != pEncoder.get() )
				{
					newTask.SetFileExt( pEncoder->GetExtension() );
				}
				newTask.SetEncoderType( ENCODER_TYPES( g_config.GetEncoderType() ) );
			}
			// Yes we want to rip the files
			newTask.SetRip( TRUE );

			newTask.SetNormalize( g_config.GetNormTrack() );

			// Convert to a Riff Wav file
			newTask.SetRiffWav( g_config.GetRiffWav() );

			// Convert to MPEG?
			if ( WAVETYPE == nWavOrMp3 )
			{
				// No conversion required
				newTask.SetConvert( FALSE );

				// Do NOT Delete the WAV file
				newTask.SetDeleteWav( FALSE );
			}
			else
			{
				// Conversion required
				newTask.SetConvert( TRUE );

				// Delete the WAV file
				newTask.SetDeleteWav( TRUE );
			}

			m_pCopyDlg->GetTasks().AddTaskInfo( newTask );
		}
	}

	// Check if there are any selected audio tracks
	if ( !bHaveAudioTracks )
	{
		// if not, delete the m_pCopyDlg object
		delete m_pCopyDlg;
		m_pCopyDlg=NULL;

		//.show a user guidence message
		strLang = g_language.GetString(IDS_NO_TRACKS_SELECTED);
		CDexMessageBox( strLang );

		// and bail out
		return;
	}

	// retain old status
	m_lOldStatus = m_lStatus;

	// Indicate start of recording
	m_lStatus = RECORDING;

	// Disable the Tracklist
	m_TrackList.EnableWindow( FALSE );



	// Create and show the dialog
	m_pCopyDlg->Create( IDD_COPYDIALOG, this );
	m_pCopyDlg->ShowWindow( SW_SHOW );
	m_pCopyDlg->CenterWindow();
	m_pCopyDlg->UpdateWindow();

}



void CCDexView::SaveCDInfo() 
{
	int nTrackIdx;

	ENTRY_TRACE( _T( "CCDexView::SaveCDInfo" ) );

	UpdateData(TRUE);

	GetDocument()->GetCDInfo().SetArtist( CUString( m_strArtist ) );
	GetDocument()->GetCDInfo().SetTitle( CUString( m_strTitle ) );

	GetDocument()->GetCDInfo().SetYear( CUString( m_strYear ) );

	// set genre string
	CString strGenre;

    if ( m_Genre.GetCurSel()>= 0 ) 
    {
	    m_Genre.GetLBText( m_Genre.GetCurSel(), strGenre );
    }

	GetDocument()->GetCDInfo().SetGenre( CUString( strGenre ) );

	int nNumTracks=GetDocument()->GetCDInfo().GetNumTracks();

	for ( nTrackIdx = 0; nTrackIdx < nNumTracks; nTrackIdx++ )
	{
		GetDocument()->GetCDInfo().SetTrackName( CUString( m_TrackList.GetItemText(nTrackIdx,0) ), nTrackIdx );
	}

	if ( GetDocument()->SaveToIni() )
	{
		// Save CD info out of one of the data bases
		GetDocument()->GetCDInfo().SaveCDInfo();

		// Once is enough
		GetDocument()->SaveToIni( FALSE );
	}
	EXIT_TRACE( _T( "CCDexView::SaveCDInfo" ) );
}

void CCDexView::OnUpdateConfigure( CCmdUI* pCmdUI ) 
{
	pCmdUI->Enable( RECORDING != m_lStatus );
}


void CCDexView::OnDropFiles( HDROP hDropInfo ) 
{

	// Check if output directory does exist
	if ( CDEX_OK != DoesDirExist(g_config.GetCnvOutputDir(),TRUE) )
		return;

	// Get the number of dropped files
	int nTotalFiles=::DragQueryFile( hDropInfo, (UINT)-1,NULL, 0 );
	
	// Ask user what to do with the dropped files
	CDropDlg dlg;

	if ( IDCANCEL != dlg.DoModal() )
	{
		// Create new modeless dialog
		m_pCopyDlg = new CCopyDialog(this);

		for (int i=0;i<nTotalFiles;i++)
		{
			CTaskInfo newTask;

			TCHAR lpszFileName[1000];

			// Get drop file name
			::DragQueryFile( hDropInfo, i, lpszFileName, _countof( lpszFileName ) );

			// set full file name
			newTask.SetFullFileName( lpszFileName );

			// set output dir
			newTask.SetOutDir( g_config.GetCnvOutputDir() );

			// setup encoder type
			newTask.SetEncoderType(	 ENCODER_TYPES( g_config.GetEncoderType() ) );

			newTask.SetNormalize( dlg.m_bNormalize );

			newTask.SetConvert( dlg.m_bEncode );

			newTask.SetDeleteWav( g_config.GetDeleteWAV() );

			newTask.SetRiffWav( g_config.GetRiffWav() );
		
			m_pCopyDlg->GetTasks().AddTaskInfo( newTask );

		}

		if ( dlg.m_bEncode || dlg.m_bNormalize )
		{
			// Get the current status
			m_lOldStatus=m_lStatus;

			// Indicate start of recording
			m_lStatus=RECORDING;

			// do the converion
			ConvertWav2MP3( );
		}
	}

	// Finish up the drag and drop session
	::DragFinish( hDropInfo );
}



void CCDexView::ConvertWav2MP3() 
{
	// Create the modeless dialog box
	m_pCopyDlg->Create( IDD_COPYDIALOG, this );

	// Show the dialog
	m_pCopyDlg->ShowWindow( SW_SHOW );

	// Center the window
	m_pCopyDlg->CenterWindow();

	// Update the window
	m_pCopyDlg->UpdateWindow();
}


void CCDexView::OnReadcdplayerini() 
{
	CUString		strLang;
	CCDexDoc*	pDoc = GetDocument();
	CWaitCursor cWait;

	// Get Local CDDB Info
	if ( FALSE == pDoc->GetCDInfo().ReadCDPlayerIni() )
	{
		strLang = g_language.GetString( IDS_ENTRYNOTINCDPLAYERINI );
		CDexMessageBox(strLang);
	}

	UpdateCDInfo();
}

void CCDexView::OnReadlocalcddb() 
{
	CUString		strLang;
	CCDexDoc*	pDoc = GetDocument();

	int			nNumTracks = pDoc->GetCDInfo().GetNumTracks();

	if ( nNumTracks > 0 )
	{
		// Get Local CDDB Info
		if ( FALSE == pDoc->GetCDInfo().ReadLocalCDDB() )
		{
			strLang = g_language.GetString( IDS_ENTRYNOTINLOCALCDDB );
			CDexMessageBox(strLang);
		}
	
		UpdateCDInfo();
	}
}

void CCDexView::OnDestroy() 
{
	// Just in case
	if ( PLAYING  == m_lStatus )
	{
		m_cCDPlay.Stop();
	}

	// And get rid of the TIMER
	KillTimer( TIMERID );

	// Save the stuff
	SaveCDInfo();

	// Destroy window
	CFormView::OnDestroy();
}


void CCDexView::OnMp3toriffwav() 
{
	CUString		strLang;

	// Create new modeless dialog
	CMp3toRiffWavDlg dlg;

	// create custom open file dialog
	dlg.m_pFileDlg=new COpenFileDlg(	g_config.GetMP3InputDir(), 
										_T( "MP3;MP2" ),
										IDI_FILE_ICON );

	dlg.m_pFileDlg->m_bDeleteOriginal = FALSE;

	dlg.m_pFileDlg->ShowDelOrigCtrl( TRUE );
	dlg.m_pFileDlg->ShowNormalizeCtrl( FALSE );
	dlg.m_pFileDlg->ShowRiffWavCtrl( FALSE );
	dlg.m_pFileDlg->ShowKeepDirLayoutCtrl( FALSE );

	strLang = g_language.GetString( IDS_CONVERT_BUTTON_TEXT );
	dlg.m_pFileDlg->SetOpenButtonText( strLang );

	// check if OK button has been pressed
    if ( IDOK == dlg.m_pFileDlg->DoModal() ) 
	{
		m_lOldStatus = m_lStatus;
		m_lStatus = RECORDING;

		g_config.SetMP3InputDir( dlg.m_pFileDlg->GetDir() );

		// Modeless dialog box
		dlg.DoModal();

		// switch back to old status
		m_lStatus = m_lOldStatus;
	}

}

void CCDexView::OnUpdateMp3toriffwav(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_lStatus!=RECORDING && g_bEncoderPresent);
}


void CCDexView::SetStatusBar( LPCTSTR lpszText ) 
{
	if (m_pStatus)
		m_pStatus->SetPaneText(1, lpszText);
}


void CCDexView::OnEditRenametrack() 
{
	ENTRY_TRACE( _T( "CCDexView::OnEditRenametrack" ) );

	// Get the item that's focused
	int nFocused = m_TrackList.GetNextItem( -1, LVNI_FOCUSED );

	// Set Focus
	m_TrackList.SetFocus();

	if ( nFocused >= 0 )
	{
		m_TrackList.EditLabel( nFocused );
	}

	EXIT_TRACE( _T( "CCDexView::OnEditRenametrack" ) );
}

void CCDexView::OnEditSelectall() 
{
	m_TrackList.SelectAll();
}

void CCDexView::OnEditRefresh() 
{
	OnNewCD();
}

void CCDexView::OnEditCopytrackstoclipboard() 
{
	CCDexDoc*	pDoc = GetDocument();

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

	// Create string with all the data we want to put in the clip board

	// How many tracks do we have?
	int nNumTracks=pDoc->GetCDInfo().GetNumTracks();

	CUString strClip;

	UpdateData(TRUE);

	strClip += CUString( m_strArtist ) + _W( "\r\n" );
	strClip += CUString( m_strTitle ) + _W( "\r\n" );
	strClip += _W( "\r\n" );

	// Loop trough the tracks and fill in the data
	for (int i=0;i<nNumTracks;i++)
	{

		CUString strEntry;

		strEntry += CUString( m_TrackList.GetItemText( i, TL_TRACKNUMBER ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_TRACKNAME ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_STARTTIME ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_PLAYTIME ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_TRACKSIZE ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_TRACKSTATUS ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_CHECKSUM  ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_GAP ) ) + _W( "\t" );
		strEntry += CUString( m_TrackList.GetItemText( i, TL_ISRC ) );

		strClip += strEntry + _W( "\r\n" );

	}

	HANDLE hData=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, ( strClip.GetLength()+1 ) * sizeof( TCHAR ) );

	if ( hData )
	{
        CUStringConvert strCnv;

		LPSTR lpszClip= (LPSTR)GlobalLock( hData );

        // copy the data
        memcpy( lpszClip, strCnv.ToT( strClip ), ( strClip.GetLength()+1 ) * sizeof( TCHAR ) );  

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

void CCDexView::OnCddbSavetocdplayerini() 
{
	CWaitCursor cWait;

	SaveCDInfo ();
	CCDexDoc* pDoc = GetDocument();
	pDoc->GetCDInfo().SaveCDPlayerIni();
}

void CCDexView::OnCddbSavetolocalcddb() 
{
	SaveCDInfo ();
	CCDexDoc* pDoc = GetDocument();
	CDdb localCDDB( &pDoc->GetCDInfo(), NULL );
	// Save it to local CDDB
	localCDDB.WriteCurrentEntry();
}

LRESULT CCDexView::OnChangeCDSelection(WPARAM nSelection,LPARAM lParam)
{
	ENTRY_TRACE( _T( "CCDexView::OnChangeCDSelection" ) );

	if ( -1 != nSelection )
	{
		LTRACE( _T( "nSelection=%d" ), nSelection );

		// Save the CD Info
		SaveCDInfo();

		// Set active CD-ROM
		CR_SetActiveCDROM( nSelection );

		// Save it to the ini file
		g_config.Save();
		g_config.Load();
	}

	// Get new TOC 
	OnNewCD();

	// Update the CD Selection in the toolbar
	CPlayToolBar* pCDInfoBar = ((CMainFrame*)AfxGetMainWnd())->GetPlayToolBar();
	if (pCDInfoBar!=NULL)
	{
		LTRACE( _T( "Call UpdateCDSelection" ) );
		pCDInfoBar->UpdateCDSelection();
		LTRACE( _T( "Call UpdateCDSelection done" ) );
	}
	EXIT_TRACE( _T( "CCDexView::OnChangeCDSelection" ) );
	return 0;
}

void CCDexView::OnConfigcd() 
{
	OnSelectConfigure( 2 );
}


void CCDexView::OnClose() 
{
	CFormView::OnClose();
}

void CCDexView::OnUpdateViewerrorlog(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(AfxGetApp()->GetProfileInt( _T( "Version 1.0" ), _T( "DisableExceptionHandling" ), 0 ) != 1 );
}


static int nSeekPos = - 1;

LRESULT CCDexView::OnSeekButton( WPARAM wParam,LPARAM lParam )
{
	CUString strLang;
	CUString strInfo;

	static int nAbsPos,nRelPos;
	
	lParam *= 25;

	// First time, get current playing position
	if ( 1 == wParam )
	{
		// start with current time (convert to sectors)
		nSeekPos = m_cCDPlay.GetCurrentTimeInSecs() * TRACKSPERSEC;

		// pause further playing
		m_cCDPlay.Pause( TRUE );
	}

	if ( lParam > 0 )
	{
		// skip forward, limited to number of play sectors
		nSeekPos = min( nSeekPos + (int)lParam,  (int)m_cCDPlay.GetPlaySectors() );
	}
	else
	{
		// skip backward, limited to zero
		nSeekPos = max( nSeekPos + lParam,  0 );
	}

	// Convert current seek position to MSF format
	DWORD dwSec = ( ( nSeekPos ) / TRACKSPERSEC ) % 60;
	DWORD dwMin = ( ( nSeekPos ) / TRACKSPERSEC / 60 ) % 60;
	DWORD dwHour = ( ( nSeekPos ) / TRACKSPERSEC / 3600 );

	strLang = g_language.GetString( IDS_SEEKING_TRACK );

	strInfo.Format( strLang, m_PlayTracksArray[ max( m_nCurPlayTrack - 1, 0 ) ], dwHour, dwMin, dwSec );

	// display seek position in status bar
    CUStringConvert strCnv;
	SetStatusBar( strCnv.ToT( strInfo ) );

	return 0;
}


    
LRESULT CCDexView::OnChildDialogClosed( WPARAM wParam,LPARAM lParam )
{
	switch ( wParam )
	{
		case 0:
			delete m_pCopyDlg;
			m_pCopyDlg = NULL;
			if ( ( 0 != lParam ) && (  TRUE == g_config.GetAutoShutDown() ) )
			{
				ShutDown();
			}
		break;
		case 1:
			delete m_pConvertDlg;
			m_pConvertDlg = NULL;
		break;
		default:
			ASSERT( FALSE );
			break;
	}

	// Restore status
	m_lStatus=m_lOldStatus;

	// Enable the Tracklist again
	m_TrackList.EnableWindow( TRUE );

	// Update the track status only;
	UpdateTrackStatus();



	return 0;
}

LRESULT CCDexView::OnUpdateTrackStatus(WPARAM wParam,LPARAM lParam)
{
	UpdateTrackStatus();
	return 0;
}

LRESULT CCDexView::OnSeekButtonUp( WPARAM wParam, LPARAM lParam )
{
	m_cCDPlay.Pause( FALSE );

	// check if were at the end of a track, if so, stop playing track
	if ( nSeekPos >= (int)m_cCDPlay.GetPlaySectors() )
	{
		m_cCDPlay.Stop();
	}
	else
	{
		// seek to new position
		m_cCDPlay.SeekToSector( nSeekPos );
	}
	return 0;
}



void CCDexView::OnMp3towav() 
{
	CUString	strLang;

	// create custom open file dialog
    m_pConvertDlg=new CMP3ToWavDlg( this );
	m_pConvertDlg->m_pFileDlg =new CMpegToWavFileOpen(g_config.GetMP3InputDir(), GetInFileNameExt() , IDI_FILE_ICON);

    // Create the modeless dialog box
    m_pConvertDlg->m_pFileDlg->m_bNormalize = g_config.GetNormTrack();
    m_pConvertDlg->m_pFileDlg->m_bDeleteOriginal=g_config.GetDeleteWAV();
    m_pConvertDlg->m_pFileDlg->m_bRiffWav=g_config.GetRiffWav();
    m_pConvertDlg->m_pFileDlg->EnableSettingsDlg( TRUE );
    m_pConvertDlg->m_pFileDlg->EnableSettingsDlg( FALSE );
    
    strLang = g_language.GetString( IDS_CONVERT_BUTTON_TEXT );
	m_pConvertDlg->m_pFileDlg->SetOpenButtonText( strLang );

	// check if OK button has been pressed
    if ( IDOK == m_pConvertDlg->m_pFileDlg->DoModal() ) 
	{
		m_lOldStatus=m_lStatus;
		m_lStatus=RECORDING;

		g_config.SetNormTrack( m_pConvertDlg->m_pFileDlg->m_bNormalize );
		g_config.SetMP3InputDir(m_pConvertDlg->m_pFileDlg->GetDir());
		g_config.SetDeleteWAV( m_pConvertDlg->m_pFileDlg->m_bDeleteOriginal );
		g_config.SetRiffWav( m_pConvertDlg->m_pFileDlg->m_bRiffWav );

		// Create the modeless dialog box
		m_pConvertDlg->Create(IDD_MP3TOWAV,NULL);

		// Show the dialog
		m_pConvertDlg->ShowWindow(SW_SHOW);

		// Center the window
		m_pConvertDlg->CenterWindow();

		// Update the window
		m_pConvertDlg->UpdateWindow();
    }
	else
	{
		// delete the cobversion dialog box
		delete m_pConvertDlg;
		m_pConvertDlg = NULL;
	}
}

void CCDexView::OnUpdateMp3towav(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_lStatus==IDLE || m_lStatus==NONE);
}

void CCDexView::OnViewripstatus() 
{
	CRipStatusDlg	dlg;
	CUString			strRipInfo;
	int				nSelected=-1;
	
	// No status information yet
	dlg.m_strRipStatus= _W("");

	// Check if track list is valid
	if ( NULL != m_TrackList.m_hWnd )
	{
		// Get the selected items
		while( ( nSelected = m_TrackList.GetNextItem( nSelected, LVNI_SELECTED ) ) != -1 )
		{
			// Get status info, add to dialog string
			dlg.m_strRipStatus += m_RipInfoDB.GetRipInfo( nSelected + 1 ) + _W( "\r\n" );
		}
	}

	// No selected Tracks?? If so, add all track status info
	if ( dlg.m_strRipStatus.IsEmpty() )
	{
		// How many tracks do we have?
		int nNumTracks = GetDocument()->GetCDInfo().GetNumTracks();

		// Add status info of all tracks
		for (nSelected = 0 ; nSelected < nNumTracks; nSelected++ )
		{
			// Get status info of the track, and add it to the dialog string
			dlg.m_strRipStatus += m_RipInfoDB.GetRipInfo( nSelected + 1 );
		}
	}

	dlg.DoModal();
}

void CCDexView::OnComparefiles() 
{
	FileCompareDlg dlg;
	dlg.DoModal();
}

void CCDexView::OnPlayfiles() 
{
	BOOL rc = FALSE ;

	TCHAR szAppName[ MAX_PATH + 1 ] = { _T( '\0' ), };
	CUString strParam;
	

	STARTUPINFO			sInfo;
	PROCESS_INFORMATION	pi ;

	memset( &sInfo, 0, sizeof ( sInfo ) ) ;
	sInfo.cb = sizeof( STARTUPINFO );


	GetModuleFileName( NULL, szAppName, _countof( szAppName ) );

	strParam.Format( _W( "%s  /PLAY" ), (LPCWSTR)CUString( szAppName ) );

    CUStringConvert strCnv;
    CUStringConvert strCnv1;

	rc = ::CreateProcess (
							NULL,
							strCnv.ToT( strParam ),
							NULL,
							NULL,
							FALSE,
							CREATE_DEFAULT_ERROR_MODE,
							NULL,
							strCnv1.ToT( g_config.GetAppPath() ),
							&sInfo,
							&pi );
}

void CCDexView::OnUpdatePlayfiles(CCmdUI* pCmdUI) 
{
}

void CCDexView::OnOptionClearstatus() 
{
	// Remove the status
	m_RipInfoDB.RemoveAll();
	// Reflect chanes to user
	UpdateTrackStatus();
}

void CCDexView::OnRiffwavtomp3() 
{
	CUString	strLang;

	// Create new modeless dialog
	CRiffWavToMP3 dlg;

	// create custom open file dialog
	dlg.m_pFileDlg=new COpenFileDlg(g_config.GetWAVInputDir(), _T( "WAV" ), IDI_FILE_ICON);

	dlg.m_pFileDlg->m_bDeleteOriginal = FALSE;
	dlg.m_pFileDlg->ShowDelOrigCtrl( TRUE );
	dlg.m_pFileDlg->ShowNormalizeCtrl( FALSE );
	dlg.m_pFileDlg->ShowRiffWavCtrl( FALSE );
	dlg.m_pFileDlg->ShowKeepDirLayoutCtrl( FALSE );

	strLang = g_language.GetString( IDS_CONVERT_BUTTON_TEXT );
	dlg.m_pFileDlg->SetOpenButtonText( strLang );

	// check if OK button has been pressed
    if (dlg.m_pFileDlg->DoModal() == IDOK) 
	{
		m_lOldStatus=m_lStatus;
		m_lStatus=RECORDING;

		// Modelss dialog box
		dlg.DoModal();

		// switch back to old status
		m_lStatus=m_lOldStatus;
    }
}

void CCDexView::OnToolsRecordfiles() 
{
	CRecordFiles dlg;
	dlg.DoModal();
}

BOOL CCDexView::PreTranslateMessage(MSG* pMsg) 
{
	// If edit control is visible in tree view control, sending a
	// WM_KEYDOWN message to the edit control will dismiss the edit
	// control.  When ENTER key was sent to the edit control, the parent
	// window of the tree view control is responsible for updating the
	// item's label in TVN_ENDLABELEDIT notification code.
	if ( pMsg->message == WM_KEYDOWN )
	{
 		CEdit* edit = NULL;
 		TCHAR ckey = toupper( pMsg->wParam &0xFF );

 		if (GetDlgItem(IDC_TITLE)->m_hWnd==pMsg->hwnd)
 			edit=(CEdit*)GetDlgItem(IDC_TITLE);
 		if (GetDlgItem(IDC_YEAR)->m_hWnd==pMsg->hwnd)
 			edit=(CEdit*)GetDlgItem(IDC_YEAR);
 		if (GetDlgItem(IDC_ARTIST)->m_hWnd==pMsg->hwnd)
 			edit=(CEdit*)GetDlgItem(IDC_ARTIST);
 		if (GetDlgItem(IDC_TRACKOFFSET)->m_hWnd==pMsg->hwnd)
 			edit=(CEdit*)GetDlgItem(IDC_TRACKOFFSET);
 
 		if (edit)
 		{
//			LTRACE("Control key status = %d %d\n",LOBYTE(GetKeyState( VK_CONTROL )),HIWORD(GetKeyState( VK_CONTROL )));
 
 			if( ( GetKeyState( VK_CONTROL )<-1 ) && ( ckey == _T( 'C' ) ) )
 			{
 				edit->Copy();
 				return TRUE;
 			}
 			if( ( GetKeyState( VK_CONTROL )<-1 ) && ( ckey == _T( 'V' ) ) )
 			{
 				edit->Paste();
 				return TRUE;
 			}
 			if( ( GetKeyState( VK_CONTROL )<-1 ) && ( ckey == _T( 'X' ) ) )
 			{
 				edit->Cut();
 				return TRUE;
 			}
 			if( ( GetKeyState( VK_CONTROL )<-1 ) && ( ckey == _T( 'Z' ) ) )
 			{
 				edit->Undo();
 				return TRUE;
 			}
 			if( ( GetKeyState( VK_CONTROL )<-1 ) && ( ckey == _T( 'A' ) ) )
 			{
 				edit->SetSel(0,-1);
 				return TRUE;
 			}
 			if( pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_CONTROL || pMsg->wParam == VK_INSERT || pMsg->wParam == VK_SHIFT )
 			{
 				edit->SendMessage(WM_KEYDOWN, pMsg->wParam, pMsg->lParam);
 				return TRUE;
 			}
 		}
	}
	return CFormView::PreTranslateMessage(pMsg);
}

void CCDexView::OnCddbSubmittoremotecddb() 
{
	CUString	strLang;

	UpdateData(TRUE);

	// Get the latest values out of the controls
	SaveCDInfo();

	// Can take a while
	CWaitCursor Wait;

	int i;
	int nNumTracks=GetDocument()->GetCDInfo().GetNumTracks();

	for ( i=0;i<nNumTracks;i++)
	{
		CUString strNoArtist( g_language.GetString( IDS_NOARTIST ) );
		CUString strNoTitle( g_language.GetString( IDS_NOTITLE )  );
		CUString strAudioTrack( g_language.GetString( IDS_AUDIOTRACK )  );
		CUString strDataTrack( g_language.GetString( IDS_DATATRACK )  );
		CUString strUnknown( g_language.GetString( IDS_UNKNOWN )  );


		CString strGenre;
		m_Genre.GetLBText( m_Genre.GetCurSel(), strGenre );

		CUString strTrack = GetDocument()->GetCDInfo().GetTrackName(i);

        CUStringConvert strCnv;
        CUStringConvert strCnv1;
        CUStringConvert strCnv2;

		if (	strTrack.Find( strAudioTrack )	>=0 || 
				strTrack.Find( strDataTrack )	>=0 ||
				m_strTitle.Find( strCnv.ToT( strNoTitle ) )	>=0 ||
				m_strArtist.Find( strCnv1.ToT( strNoArtist ) )	>=0 ||
				strTrack.IsEmpty()					||
				m_strTitle.IsEmpty()				||
				m_strArtist.IsEmpty()				||
				strGenre.CompareNoCase( strCnv2.ToT(strUnknown ) ) == 0
				)
		{
			strLang = g_language.GetString( IDS_CDDBINFOINCOMPLETE );
			CDexMessageBox( strLang );
			return;
		}
	}

	// Create Remote CDDB dialog box
	CCDdbDlg dlg(	SubmitRemoteCDDBThreadFunc,
					&( GetDocument()->GetCDInfo() ) );

	dlg.DoModal();

	// Update Data
	//UpdateCDInfo();
}


void CCDexView::OnCddbReadcdtext() 
{
	CCDexDoc* pDoc=GetDocument();

#ifdef _DEBUG
//	CCdexFileOpen dlg(FALSE);
//	dlg.DoModal();
#endif

	// Get Local CDDB Info
	if ( pDoc->GetCDInfo().ReadCDText() )
	{
		UpdateCDInfo();
	}

}

void CCDexView::OnCddbBatchQuery() 
{

    CDInfo* pCDInfo = new CDInfo();
    CGetRemoteCDInfoDlg dlg( pCDInfo, true );

	//// Create Remote CDDB dialog box
	//CCDdbDlg dlg(	BatchRemoteCDDBThreadFunc,
	//				&( GetDocument()->GetCDInfo() ) );

	dlg.DoModal();

    delete pCDInfo;

	// refresh track list
	OnReadlocalcddb();
}

void CCDexView::OnToolsBurn() 
{
	// Save status
	m_lOldStatus = m_lStatus;

	// Indicate start of recording
	m_lStatus = RECORDING;

	CCdrDao dlg;
	dlg.DoModal();

	// Restore status
	m_lStatus = m_lOldStatus;

}


void CCDexView::OnMpegtompeg() 
{
	CUString	strLang;

	// create custom open file dialog
	COpenFileDlg fileDlg(g_config.GetWAVInputDir(), GetInFileNameExt(), IDI_FILE_ICON);

	fileDlg.m_bNormalize=g_config.GetNormTrack();
	fileDlg.m_bDeleteOriginal=g_config.GetDeleteWAV();
	fileDlg.m_bRiffWav=g_config.GetRiffWav();

	fileDlg.ShowDelOrigCtrl( TRUE );
	fileDlg.ShowNormalizeCtrl( FALSE );
	fileDlg.ShowRiffWavCtrl( TRUE );
	fileDlg.ShowKeepDirLayoutCtrl( TRUE );

	strLang = g_language.GetString( IDS_CONVERT_BUTTON_TEXT );
	fileDlg.SetOpenButtonText( strLang );

	// check if OK button has been pressed
    if (fileDlg.DoModal() == IDOK) 
    {
		g_config.SetNormTrack(fileDlg.m_bNormalize);
		g_config.SetDeleteWAV(fileDlg.m_bDeleteOriginal);
		g_config.SetRiffWav(fileDlg.m_bRiffWav);

		// Get values out of controls
		UpdateData( TRUE );

		// Create new modeless dialog
		m_pCopyDlg=new CCopyDialog(this);

		// Save to the config file
		g_config.SetWAVInputDir( fileDlg.GetDir() );

		// Save status
		m_lOldStatus=m_lStatus;

		// Indicate start of recording
		m_lStatus=RECORDING;

		// Get input directory
		POSITION pos=fileDlg.GetStartPosition();

		DWORD dwTrack = 1;

		while (pos!=NULL)
		{
			CTaskInfo newTask;

            CTagData& tagData( newTask.GetTagData() );

			POSITION nFilePos = pos;

			CUString strSubPath( fileDlg.GetSubPath( nFilePos ) );

            CUString fileName = fileDlg.GetNextPathName( pos );
			// set full file name
			newTask.SetFullFileName( fileName );

			// set output dir
			newTask.SetOutDir( g_config.GetCnvOutputDir() + strSubPath );

			// setup encoder type
			newTask.SetEncoderType( ENCODER_TYPES( g_config.GetEncoderType() ) );

			newTask.SetValidTagData( false );

			// try to read the TAG
			CID3Tag id3Tag;
            CUString strExt = newTask.GetFileExt();

            if (    0 == strExt.CompareNoCase( _W( "MP3" ) ) ||
                    0 == strExt.CompareNoCase( _W( "MP2" ) )
                    )
            {
                    
			if ( id3Tag.OpenFile( newTask.GetFullFileName() ) )
			{
				id3Tag.LoadTagProperties();

				if ( id3Tag.IsV1Tag() || id3Tag.IsV1Tag() )
				{
					// AF tagData.SetArtist( id3Tag.GetGetArtist() );
					// AF tagData.SetAlbum( id3Tag.GetAlbum() );
					// AF TODO tagData.SetValidTagData( true );

				}
			}
			//
            }
			// newTask.SetTrack( dwTrack );
			// newTask.SetTrackOffset( 0 );
			newTask.SetNormalize( FALSE );
			newTask.SetConvert( TRUE );
			newTask.SetDeleteWav( fileDlg.m_bDeleteOriginal );

			newTask.SetRiffWav( g_config.GetRiffWav() );

			m_pCopyDlg->GetTasks().AddTaskInfo( newTask );

			dwTrack++;
		}

		// Start the Wave File conversion dialog
		ConvertWav2MP3( );
    }
}

void CCDexView::OnUpdateMpegtompeg(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_lStatus!=RECORDING && g_bEncoderPresent);
}

void CCDexView::OnToolsSearchtrackindex() 
{
	// do a binary search over the tracks
	int nTrack = 0;

	CCDexDoc*	pDoc = GetDocument();
	CDInfo&		cdInfo = pDoc->GetCDInfo();

	int nNumTracks= cdInfo.GetNumTracks();

	for ( nTrack = 0; nTrack < nNumTracks; nTrack++ )
	{
		WORD wTrack = cdInfo.GetTrack( nTrack );
		DWORD dwStart = cdInfo.GetStartSector(wTrack);
		DWORD dwEnd = cdInfo.GetSaveEndSector(wTrack);


		DWORD dwTargetPos = ( dwStart + dwEnd ) /2;

		int nReadIndex = 0;
		int nReadTrack = 0;
		DWORD dwReadPos = 0;

		CR_SetPlayPosition( dwTargetPos );

		for ( int q=0; q< 10; q++ )
		{
			CUString strTmp;
			CR_GetSubChannelTrackInfo(	nReadIndex, nReadTrack, dwReadPos );
			strTmp.Format( _W( "SubChannel info %d %d %d (target pos is %d)\n" ), nReadIndex, nReadTrack, dwReadPos, dwTargetPos );
			// OutputDebugString( strTmp );
		}
	}
}

BOOL CCDexView::SetCurrentPrivilege (LPCTSTR Privilege, BOOL bEnablePrivilege)
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp, tpPrevious;
	DWORD cbPrevious = sizeof( TOKEN_PRIVILEGES );
	BOOL bSuccess = FALSE;

	if ( ! LookupPrivilegeValue( NULL, Privilege, &luid ) )
		return FALSE;

	if( ! OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken ) )
		return FALSE;

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = 0;

	AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof( TOKEN_PRIVILEGES ), &tpPrevious, &cbPrevious );

	if ( ERROR_SUCCESS == GetLastError() )
	{
		tpPrevious.PrivilegeCount = 1;
		tpPrevious.Privileges[0].Luid = luid;

		if ( bEnablePrivilege )
			tpPrevious.Privileges[0].Attributes |= ( SE_PRIVILEGE_ENABLED );
		else
			tpPrevious.Privileges[0].Attributes &= ~( SE_PRIVILEGE_ENABLED );

		AdjustTokenPrivileges( hToken, FALSE, &tpPrevious, cbPrevious, NULL, NULL );

		if ( GetLastError() == ERROR_SUCCESS )
			bSuccess=TRUE;
	}

	CloseHandle( hToken );

	return bSuccess;
}

void CCDexView::ShutDown( ) 
{
	CUString	strLang;

	if ( IsWindowsNTOS() )
	{
		// Prepare ShutDown (need to set the correct privilage to do so
		if ( ! SetCurrentPrivilege( SE_SHUTDOWN_NAME, TRUE ) )
		{
			strLang = g_language.GetString( IDS_FAILED_TO_SET_PRIVILEGE );
			CDexMessageBox( strLang );
		}
	}

	// And initiate power-off
	ExitWindowsEx( EWX_POWEROFF, NULL);
}


void CCDexView::FormatTrackName( ENUM_FILENAME_CONVERTS nType ) 
{

	CUString		strTmp;
	CCDexDoc*	pDoc = GetDocument();


	// Set the latest values
	SaveCDInfo();

	// How many tracks do we have?
	int nNumTracks	= pDoc->GetCDInfo().GetNumTracks();

	strTmp = pDoc->GetCDInfo().GetArtist();
	pDoc->GetCDInfo().SetArtist( FormatTrackNameString( strTmp, nType )  );

	strTmp = pDoc->GetCDInfo().GetTitle();
	pDoc->GetCDInfo().SetTitle( FormatTrackNameString( strTmp, nType )  );

	for (int i=1;i<=nNumTracks;i++)
	{
		// Always handy to have a temp string
		strTmp = pDoc->GetCDInfo().GetTrackName( i-1 );
		pDoc->GetCDInfo().SetTrackName( FormatTrackNameString( strTmp, nType ), i-1 );
	}

	UpdateCDInfo();

	GetDocument()->SetModifiedFlag(TRUE);

}

void CCDexView::OnTracknameModAl() 
{
	FormatTrackName( FILENAME_CONVERT_LOWER );
}

void CCDexView::OnTracknameModAu() 
{
	FormatTrackName( FILENAME_CONVERT_UPPER );
	
}

void CCDexView::OnTracknameModFcc() 
{
	FormatTrackName( FILENAME_CONVERT_FC_UPPER );
}

void CCDexView::OnTracknameModFcew() 
{
	FormatTrackName( FILENAME_CONVERT_FCEW_UPPER );
}

void CCDexView::OnUpdateTracknameModAl(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_lStatus==IDLE );
}

void CCDexView::OnUpdateTracknameModAu(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_lStatus==IDLE );
}

void CCDexView::OnUpdateTracknameModFcc(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_lStatus==IDLE );
}

void CCDexView::OnUpdateTracknameModFcew(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_lStatus==IDLE );
}

LRESULT CCDexView::OnEditNextTrack(WPARAM wParam,LPARAM lParam)
{
	int nFocused=m_TrackList.GetNextItem( -1, LVNI_FOCUSED );

	nFocused++;
	m_TrackList.SetFocus();

	if ( nFocused < m_TrackList.GetItemCount() )
	{
		m_TrackList.EditLabel( nFocused );
	}
	return 0;
}

LRESULT CCDexView::OnWinAmpPlugInFinished(WPARAM wParam,LPARAM lParam)
{
	WinampPlugInFinished();
	return 0;
}

void CCDexView::OnConfigureWinPlugins() 
{
	CWinampInConfig	dlg;
	dlg.DoModal();
}

void CCDexView::OnUpdateConfigureWinPlugins(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( GetNumWinampPlugins() > 0 );
}

BOOL CCDexView::DestroyWindow() 
{
	// free all Winamp DLLs
	DeInitWinAmpPlugins();
	return CFormView::DestroyWindow();
}



void CCDexView::OnDblclkGenre() 
{
	CDexMessageBox( _T( "Enter New Genre Entry" ) );
}


void CCDexView::OnEditAddeditgenres() 
{
	CGenreEditDlg dlg;

	if ( IDOK == dlg.DoModal() )
	{
		UpdateCDInfo();
	}
}

void CCDexView::OnEditModifytracknameExchangeartisttrack() 
{
	FormatTrackName( FILENAME_CONVERT_SWAP_ARTIST_TRACK );
}


void CCDexView::OnProfileEditChange()
{
}


void CCDexView::OnUpdateCDSelBar(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_lStatus != RECORDING );
}

void CCDexView::OnUpdateConfigCD(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( m_lStatus != RECORDING );
}

void CCDexView::UpdateGenreSelection( const CString& strSelection )
{

	int nSelection = 0;
	
	nSelection = g_GenreTable.SearchGenre( CUString( strSelection ) );

	// Check if the selected genre is present in the drop down list
	if ( -1 == nSelection ) 
	{
		// ADD selections
		g_GenreTable.AddEntry( 255, CUString( strSelection ), _T( "misc" ), true );

		// Get new inserted selection item
		nSelection = g_GenreTable.SearchGenre( CUString( strSelection ) );
	}

	// Delete all old strings
	m_Genre.ResetContent();

	// Add all the genre strings
	for (unsigned int i=0; i < g_GenreTable.GetNumGenres(); i++)
	{
        CUStringConvert strCnv;
		m_Genre.AddString( strCnv.ToT( g_GenreTable.GetGenre( i ) ) );
	}

	// Set current selection
	m_Genre.SelectString( -1, strSelection );

}

void CCDexView::OnDeleteprofile() 
{
	CUString strLang;

	strLang = g_language.GetString( IDS_DEFAULT );

	// get name of current profile to delete
	CUString strDelete( m_pUtilToolBar->GetProfileSelectionString() );

	// delete the profile file
	CDexDeleteFile( g_config.GetAppPath() + _W("\\") + strDelete + PROFILE_EXTENTION );

	// select default profile
	g_config.SelectProfile( strLang );

	// save settings
	g_config.Save();

	// update util toolbar to reflect changes
	AddProfileStrings();
	
}

void CCDexView::OnUpdateDeleteprofile(CCmdUI* pCmdUI) 
{
	CUString strLang;

	strLang = g_language.GetString( IDS_DEFAULT );

	if ( m_pUtilToolBar->GetProfileSelectionString() == strLang )
	{
		pCmdUI->Enable( FALSE );
	}
	else
	{
		pCmdUI->Enable( TRUE );
	}
}


void CCDexView::AddProfileStrings() 
{
	bool	bHasProfiles = false;
	CUString strLang;
	CUString strProfile = g_config.GetProfileName();

	strLang = g_language.GetString( IDS_DEFAULT );

	CFileFind ff;

    CUStringConvert strCnv;

	BOOL bFound = ff.FindFile( strCnv.ToT( g_config.GetAppPath() + _W( "\\*" ) + PROFILE_EXTENTION ) );

	m_pUtilToolBar->DeleteAllProfileSelections();

	m_pUtilToolBar->AddProfileSelection( strLang );

	while ( bFound )
	{
		bFound = ff.FindNextFile();
		bHasProfiles = true;
		CUString strFile = ff.GetFileName();
		strFile = strFile.Left( strFile.GetLength() - CUString( PROFILE_EXTENTION ).GetLength() );

		if ( strLang != strFile )
		{
			m_pUtilToolBar->AddProfileSelection( strFile );
		}
	}

	if ( strProfile.IsEmpty() )
	{
		strProfile = strLang;
	}

	m_pUtilToolBar->SetProfileSelection( strProfile );
	g_config.SetProfileName( strProfile );
}

void CCDexView::OnSaveprofile() 
{
	CEnterProfileName dlg;

	if ( IDOK == dlg.DoModal() )
	{
		if ( !dlg.m_strProfileName.IsEmpty() )
		{
			// set new profile name
			g_config.SetProfileName( CUString( dlg.m_strProfileName ) );

			// save settings
			g_config.Save();

			// update util toolbar to reflect changes
			AddProfileStrings();
		}
	}
}

void CCDexView::OnUpdateSaveprofile(CCmdUI* pCmdUI) 
{
		pCmdUI->Enable( RECORDING != m_lStatus && CONFIGURE != m_lStatus );
}

LRESULT CCDexView::OnChangeProfileSelection(WPARAM nSelection,LPARAM lParam)
{
	// get name of current profile to delete
	CUString strSelected( m_pUtilToolBar->GetProfileSelectionString() );

	// select default profile
	g_config.SelectProfile( strSelected );

	// save settings
	g_config.Save();
	return 0;
}


void CCDexView::UpdateListControls()
{
	CUString strLang;

	strLang = g_language.GetString( IDS_NAME );
	m_TrackList.SetHeaderControlText( TL_TRACKNAME, strLang );
	
	strLang = g_language.GetString( IDS_TRACK );
	m_TrackList.SetHeaderControlText( TL_TRACKNUMBER, strLang );

	strLang = g_language.GetString( IDS_START_TIME );
	m_TrackList.SetHeaderControlText( TL_STARTTIME, strLang );

	strLang = g_language.GetString( IDS_PLAY_TIME );
	m_TrackList.SetHeaderControlText( TL_PLAYTIME, strLang );

	strLang = g_language.GetString( IDS_SIZE );
	m_TrackList.SetHeaderControlText( TL_TRACKSIZE, strLang );

	strLang = g_language.GetString( IDS_STATUS );
	m_TrackList.SetHeaderControlText( TL_TRACKSTATUS, strLang );

	if ( ( NULL != m_pStatus ) && ( NULL != m_pStatus->m_hWnd ) ) 
	{
		CUString helpString = g_language.GetString( 0xE001 );
        CUStringConvert strCnv;
		m_pStatus->SetPaneText(0, strCnv.ToT( helpString ) );
	}
	
	// Align the stuff
	PlaceControls();
}

void CCDexView::OnCddbWinampdb() 
{
	CUString		strLang;
	CCDexDoc*	pDoc = GetDocument();

	// Get Local CDDB Info
	if ( FALSE == pDoc->GetCDInfo().ReadFromWinampDB() )
	{
		strLang = g_language.GetString( IDS_ENTRYNOTINLOCALCDDB );
		CDexMessageBox(strLang);
	}

	UpdateCDInfo();
}


void CCDexView::OnMbrainz() 
{
	LTRACE( _T( "Entering CCDexView::OnMbrainz" ) );

  CWaitCursor Wait;

  CMusicBrainz brainz(&(GetDocument()->GetCDInfo()));

//  if(brainz.ReadCDTOC())
    if(!brainz.GetInfoFromServer())
      GetDocument()->GetCDInfo().SetTitle( _T("Unknown CD for MusicBrainz and FreeDB") );

  UpdateCDInfo();

	LTRACE( _T( "Leaving CCDexView::OnMbrainz" ) );
}

void CCDexView::OnUpdateCddbWinampdb(CCmdUI* pCmdUI) 
{
	if (	( m_lStatus == IDLE ) && 
			( !g_config.GetWinampDBPath().IsEmpty() ) )
	{
		pCmdUI->Enable( TRUE );
	}
	else
	{
		pCmdUI->Enable( FALSE );
	}
	
}

void CCDexView::OnUpdateEject(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( ( IDLE == m_lStatus ) || ( NONE == m_lStatus ) );	
}

void CCDexView::OnKillfocusTrackoffset() 
{
	int nTrackIdx = 0;
	CCDexDoc* pDoc = GetDocument();

	UpdateData( TRUE );

	// How many tracks do we have?
	int nNumTracks = pDoc->GetCDInfo().GetNumTracks();

	// Loop trough the tracks and fill in the data
	for ( nTrackIdx = 1; nTrackIdx <= nNumTracks; nTrackIdx++ )
	{
		CUString strTmp;
        CUStringConvert strCnv;
		strTmp.Format( _W( "%02d" ), pDoc->GetCDInfo().GetTrack( nTrackIdx-1 ) + m_nTrackOffset );
		m_TrackList.SetItemText( nTrackIdx - 1, TL_TRACKNUMBER, strCnv.ToT( strTmp ) );
	}
}

void CCDexView::OnChangeTrackoffset() 
{
}

#include "CueSheet.h"

void CCDexView::OnToolsGeneratecuesheet()
{
	CCueSheet cueSheetDlg = new CCueSheet();
	cueSheetDlg.m_pDoc = GetDocument();

	if ( m_noPreGaps )
	{
		OnToolsDetectpre();
	}
	if ( m_noIsrc )
    {
        OnToolsReadupc();
    }


	
	cueSheetDlg.DoModal();

}


void CCDexView::OnToolsDetectpre()
{
    CCDexMsgBox* pMsgBox = NULL;
    
    CWaitCursor Wait;
    pMsgBox = new CCDexMsgBox( this );

	pMsgBox->Create( IDD_CDEXMSGBOX, this );

    CUStringConvert strCnv;
    pMsgBox->SetMessageBoxCaption( strCnv.ToT( g_language.GetString( IDS_DETECT_GAPS ) ) );
	
    // Show the dialog
	pMsgBox->ShowWindow( SW_SHOW );

	// Center the window
	pMsgBox->CenterWindow();

	// Update the window
	pMsgBox->UpdateWindow();

    // detect pre-gaps
	CCDexDoc* pDoc=GetDocument();
	
    CDInfo& cdInfo = pDoc->GetCDInfo();

    int prevTrackStart = 0;

    // set gap of track 0 to 2 seconds
    cdInfo.SetGap( 0, 150 );

	// Construct CDROMPARAMS
	CDROMPARAMS cdParams;

	// Get cdParames of current selected CD-ROM
	CR_GetCDROMParameters(&cdParams);

    int method = 1;

    READMETHOD readMethod = cdParams.DriveTable.ReadMethod;

    // select detection method
    if ( (readMethod == READMMC  ) || (readMethod == READMMC2 ) || (readMethod == READMMC3 ) || (readMethod == READMMC4 ) )
    {
        method = 0;
    }

    // loop trough the tracks
    for (int i = 2; i <= cdInfo.GetNumTracks(); ++i )
	{
        CUString strMsg;
        strMsg.Format( g_language.GetString( IDS_DETECTGAPFORTRACK ), i );
        pMsgBox->SetMessageBoxText( strMsg );
        DWORD highestPrevTrackSector = 0;

        int gapSize =  (int)cdInfo.GetStartSector( i );

        int left = cdInfo.GetStartSector( i - 1 );
        int right = cdInfo.GetStartSector( i );

        int lowestSector = right;

        // do binary search
        while ( ( left < right ) )
        {
            if ( pMsgBox && pMsgBox->AbortPressed() == true )
            {        
                break;
            }

            int mid = (left + right) /2;

            BOOL hasPrevTrack = FALSE;
            DWORD sectorIndex = 0;


            BYTE buffer[ 4096 ];
            memset( buffer, 0, sizeof( buffer ) );

            BYTE addressQ = 0;

            if ( method == 0 )
            {
                CR_ReadSubchannelData( (PBYTE)buffer, sizeof( buffer ), (int)mid, 1 );
                
                // get lower 4 bits
                addressQ = buffer[ 0 ] & 0x0F;
            }
            else
            {
                CR_ReadBlockDirect( (PBYTE)buffer, sizeof( buffer ), (DWORD)mid, 1 );
                CR_ReadSubChannel( (PBYTE)buffer, sizeof( buffer ), (BYTE)0x01, 0x00 );
                memmove( buffer, &buffer[ 5 ], sizeof( buffer ) - 5 );
                addressQ = buffer[ 0 ] >> 4;
            }

            for ( sectorIndex = 0; sectorIndex < 1 ; sectorIndex++ )
            {
                DWORD dwOffset = 16 * sectorIndex;

                switch ( addressQ )
                {
                    // 00h Q sub-channel mode information not supplied
                    case 0x00:
                        break;
                    // 01h Q sub-channel encodes current position data
                    case 0x01:
                        {
                            // BCD encoded!
                            int trackNum = 10 * ( buffer[ dwOffset + 1 ] >> 4 ) + ( buffer[ dwOffset + 1 ] & 0x0F );
                            int index    = 10 * ( buffer[ dwOffset + 2 ] >> 4 ) + ( buffer[ dwOffset + 2 ] & 0x0F );

                            if ( trackNum == ( i - 1 ) )
                            {
                                hasPrevTrack = TRUE;
                                highestPrevTrackSector = max( highestPrevTrackSector, (DWORD)mid );
                            }
                            if ( trackNum == i )
                            {
                                lowestSector = min( lowestSector, mid );
                            }
                        }
                        break;
                    // 02h Q sub-channel encodes media catalogue number
                    case 0x02:
                        break;
                    // 03h Q sub-channel encodes ISRC
                    case 0x03:
                        break;
                    // 04h - 0Fh Reserved
                    default:
                        break;
                }
            }

            if ( hasPrevTrack )
            {
                left = mid + 1;
            }
            else
            {
                right = mid - 1;
            }
        }
        gapSize =  (int)cdInfo.GetStartSector( i ) - lowestSector;
        cdInfo.SetGap( i - 1, gapSize );
    }

    m_TrackList.UnHideColumn( TL_GAP );

    pMsgBox->DestroyWindow();

    UpdateCDInfo();
    PlaceControls();

	m_noPreGaps = false;
}

void CCDexView::OnToolsReadupc()
{
    CWaitCursor Wait;

    CCDexMsgBox* pMsgBox = NULL;
    
    pMsgBox = new CCDexMsgBox( this );

	pMsgBox->Create( IDD_CDEXMSGBOX, this );

    CUStringConvert strCnv;
    pMsgBox->SetMessageBoxCaption( strCnv.ToT( g_language.GetString( IDS_READ_ISRC_CAPTION ) ) );
	
    // Show the dialog
	pMsgBox->ShowWindow( SW_SHOW );

	// Center the window
	pMsgBox->CenterWindow();

	// Update the window
	pMsgBox->UpdateWindow();

    CCDexDoc* pDoc=GetDocument();

    LTRACE2( _T( "CCDexView::OnToolsReadupc " ) );

    CDInfo& cdInfo = pDoc->GetCDInfo();

    CUString strMsg = g_language.GetString( IDS_READISRC_PLEASEWAIT );
    pMsgBox->SetMessageBoxText( strMsg );
    cdInfo.ReadUpcAndISRC();

    m_TrackList.UnHideColumn( TL_ISRC );

    pMsgBox->DestroyWindow();

    UpdateCDInfo();
    PlaceControls();
    m_noIsrc = false;
}
