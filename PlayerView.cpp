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
#include "config.h"
#include "PlayerView.h"
#include "Ini.h"
#include "CDInfo.h"
#include "SndStreamFactory.h"
#include "OpenFileDlg.h"
#include "ISndStreamWinAmp.h"
#include "WinampInConfig.h"
#include "direct.h"


#define TIMERID 4
#define TIMERSLOT 700


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const int IDLE = 0;
static const int PAUSING = 1;
static const int PLAYING = 2;
static const int SEEKING = 3;
static const int ABORT = 4;


INITTRACE( _T( "PlayerView" ) );

/////////////////////////////////////////////////////////////////////////////
// CPlayerView

IMPLEMENT_DYNCREATE(CPlayerView, CFormView)

CPlayerView::CPlayerView()
	: CFormView(CPlayerView::IDD),
	m_ePaused( TRUE, TRUE )
{
	m_dwBitRate=0;

	m_pInStream = NULL;
	m_pPlayStream = NULL;
	m_bPlayRandom = FALSE;
	m_ThreadCommand = THREAD_PLAY_FINISH;

	m_eThreadFinished.SetEvent();

	//{{AFX_DATA_INIT(CPlayerView)
	m_strFileName = _T("");
	//}}AFX_DATA_INIT
}

CPlayerView::~CPlayerView()
{
	delete m_pInStream;
	delete m_pPlayStream;
}




UINT CPlayerView::PlayThreadFunc(PVOID pParams)
{
	BYTE*			pStream		 = NULL;
	CDEX_ERR		bErr		 = CDEX_OK;
	CPlayerView*	pDlg		 = NULL;
	int				nCurrentFile = -1;
	CString			strFileName;

	ENTRY_TRACE( _T( "CPlayerView::PlayFunc" ) );

	nCurrentFile	= 0;

	pDlg=(CPlayerView*)pParams;

	pDlg->m_bAbortThread	= FALSE;
	pDlg->m_nTotalTime = 0;
	pDlg->m_nCurrentTime = 0;

	nCurrentFile = 	pDlg->m_PlayList.GetCurSel( );

	if ( pDlg->GetPlayRandom() )
	{
		srand( (unsigned)time( NULL ) );
		nCurrentFile = rand() % pDlg->m_vFileNames.size();
	}

	if ( nCurrentFile < 0 )
	{
		nCurrentFile = 0;
	}

	while(	( nCurrentFile >= 0 ) &&
			( nCurrentFile < (int)pDlg->m_vFileNames.size() ) && 
			( FALSE == pDlg->m_bAbortThread ) )
	{
		DWORD	dwStreamIndex=0;

		strFileName = pDlg->m_vFileNames[ nCurrentFile ];

#ifdef _DEBUG
//		strFileName = "http://urn.nottingham.ac.uk/urnlive.pls";
//		strFileName = "http://reclib.su.nottingham.ac.uk:8080";
#endif

		pDlg->m_PlayList.SetCurSel( nCurrentFile );

		LTRACE( _T( "Playing track %s (file %d)" ), strFileName, nCurrentFile );

		pDlg->SetThreadCommand( THREAD_PLAY_NEXT_TRACK );

		// Delete the old stream if it does exist
		if ( pDlg->m_pInStream )
		{
			delete pDlg->m_pInStream; pDlg->m_pInStream = NULL;
		}


		// create proper input stream
		pDlg->m_pInStream = ICreateStream( CUString( strFileName ) ); 

		pDlg->m_dwSeekOffset = 0;

		// Open the MPEG stream
		if (pDlg->m_pInStream->OpenStream( CUString( strFileName ) ) )
		{
			DWORD	dwInBufferSize = pDlg->m_pInStream->GetBufferSize();
			int		nMPEGBytes=0;

			// Create a play stream object
			pDlg->m_pPlayStream=new PlayWavStream;

			pDlg->m_pPlayStream->CopyStreamInfo( *pDlg->m_pInStream );

			// Initialize play stream object
			if ( FALSE == pDlg->m_pPlayStream->OpenStream( _W( "" ) ) )
			{
				pDlg->m_bAbortThread = TRUE;
			}

			// Allocate stream input buffer
			auto_ptr<BYTE> pStream( new BYTE[ dwInBufferSize  + STREAMBUFFERSIZE ] );
	
			pDlg->m_bAbortCurrent = FALSE;

			// Set status
			pDlg->m_nStatus = PLAYING;

			while ( ( !pDlg->m_bAbortThread ) && 
					( FALSE == pDlg->m_bAbortCurrent ) )
			{

				if ( PLAYING != pDlg->m_nStatus )
				{
					WaitForSingleObject( pDlg->m_ePaused, INFINITE );
					dwStreamIndex = 0;
					pDlg->m_pInStream->Seek( pDlg->m_dwSeekOffset, SEEK_TIME );

				}

				if ( pDlg->m_bAbortThread )
				{
					break;
				}

				if ( (nMPEGBytes = pDlg->m_pInStream->Read( (BYTE*)( pStream.get() ) + dwStreamIndex,dwInBufferSize ) ) >0 )
				{
					// increase current stream index position
					dwStreamIndex += nMPEGBytes ;

					// play the stuff when there is STREAMBUFFERSIZE samples are present
					while ( ( dwStreamIndex >= STREAMBUFFERSIZE ) && 
							( FALSE == pDlg->m_bAbortThread )  &&
							( FALSE == pDlg->m_bAbortCurrent ) )
					{
						if ( PLAYING != pDlg->m_nStatus )
						{
							break;
						}

						pDlg->m_pPlayStream->Write( pStream.get(), STREAMBUFFERSIZE );
						dwStreamIndex-= STREAMBUFFERSIZE;
						memmove( pStream.get() , (BYTE*)pStream.get() + STREAMBUFFERSIZE, dwStreamIndex );
					}

					pDlg->m_dwBitRate=pDlg->m_pInStream->GetBitRate();
					pDlg->m_nTotalTime=pDlg->m_pInStream->GetTotalTime();
				}
				else
				{
					while ( dwStreamIndex )
					{
						DWORD dwBytes = min( STREAMBUFFERSIZE, dwStreamIndex );

						if ( pDlg->m_bAbortThread )
						{
							break;
						}

						if ( dwBytes < STREAMBUFFERSIZE )
						{
							memset( (BYTE*)pStream.get() + dwBytes, 0, STREAMBUFFERSIZE - dwBytes );
						}

						pDlg->m_pPlayStream->Write( pStream.get(), STREAMBUFFERSIZE );

						dwStreamIndex-= dwBytes;

						if ( dwStreamIndex )
						{
							memmove( pStream.get() , (BYTE*)pStream.get() + dwBytes, dwStreamIndex );
						}
					}

					pDlg->m_bAbortCurrent = TRUE;
				}
			}
		}
		else
		{
			pDlg->MessageBox( _T( "Problem opening the file" ) );
		}

		// Close the mpeg stream
		if (pDlg->m_pInStream)
		{
			pDlg->m_pInStream->CloseStream();

			// Don't delete it, since we want to able to get 
			// the file details
		}

		// Close the play stream
		if (pDlg->m_pPlayStream)
		{
			LTRACE( _T( "Closing the output stream !!!!" ) );
			pDlg->m_pPlayStream->CloseStream();
		}


		delete pDlg->m_pPlayStream; pDlg->m_pPlayStream = NULL;

		switch ( pDlg->GetThreadCommand() )
		{
			case THREAD_PLAY_NEXT_TRACK:
				if ( pDlg->GetPlayRandom() )
				{
					int nNewFile = nCurrentFile;

					// randomize until we got a different track
					while ( pDlg->m_vFileNames.size() > 1 &&
							nCurrentFile == nNewFile )
					{
						srand( (unsigned)time( NULL ) );
						nNewFile = rand() % pDlg->m_vFileNames.size();
					}
					nCurrentFile = nNewFile;
				}
				else
				{
					nCurrentFile++;
				}
			break;
			case THREAD_PLAY_PREV_TRACK:
				if ( nCurrentFile > 0 )
				{
					nCurrentFile--;
				}
			break;
			case THREAD_PLAY_FINISH:
				pDlg->m_bAbortThread = TRUE;
			break;
			case THREAD_PLAY_TRACK:
				nCurrentFile = 	pDlg->m_PlayList.GetCurSel( );
			break;
		}
	}

	pDlg->m_nStatus = IDLE;

	pDlg->m_pThread = NULL;

	pDlg->m_eThreadFinished.SetEvent();

	EXIT_TRACE( _T( "CPlayerView::PlayThreadFunc" ) );

	return 0;
}


void CPlayerView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPlayerView)
	DDX_Control(pDX, IDC_PLAYLIST, m_PlayList);
	DDX_Control(pDX, IDC_TIMEINFO, m_TimeCtrl);
	DDX_Control(pDX, IDC_POSITION, m_Position);
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_VOLUME, m_Volume);
}


BEGIN_MESSAGE_MAP(CPlayerView, CFormView)
	//{{AFX_MSG_MAP(CPlayerView)
	ON_BN_CLICKED(IDC_DETAILS, OnDetails)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_COMMAND(ID_FILEOPEN, OnFileOpen)
	ON_COMMAND(ID_PREV    , OnPrev)
	ON_COMMAND(ID_NEXT    , OnNext)
	ON_COMMAND(ID_PLAY    , OnPlay)
	ON_COMMAND(ID_PAUSE   , OnPause)
	ON_COMMAND(ID_STOP    , OnStop)
	ON_UPDATE_COMMAND_UI(ID_NEXT, OnUpdateNext)
	ON_UPDATE_COMMAND_UI(ID_PAUSE, OnUpdatePause)
	ON_UPDATE_COMMAND_UI(ID_PLAY, OnUpdatePlay)
	ON_UPDATE_COMMAND_UI(ID_PREV, OnUpdatePrev)
	ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
	ON_UPDATE_COMMAND_UI(ID_FILEOPEN, OnUpdateFileopen)
	ON_WM_DESTROY()
	ON_COMMAND(ID_CONFIGURE_WIN_PLUGINS, OnConfigureWinPlugins)
	ON_LBN_SELCHANGE(IDC_PLAYLIST, OnSelchangePlaylist)
	ON_LBN_DBLCLK(IDC_PLAYLIST, OnDblclkPlaylist)
	ON_COMMAND(ID_PLAYLIST_CLEAR, OnPlaylistClear)
	ON_COMMAND(ID_PLAYLIST_SAVE, OnPlaylistSave)
	ON_COMMAND(ID_OPTIONS_PLAYRANDOM, OnOptionsPlayrandom)
	ON_UPDATE_COMMAND_UI(ID_OPTIONS_PLAYRANDOM, OnUpdateOptionsPlayrandom)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DROPFILES()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_MESSAGE( WM_SEEKSLIDERDOWN, OnStartSeek )
	ON_MESSAGE( WM_SEEKSLIDERUP, OnStopSeek )
	ON_MESSAGE( WM_USER+2, OnWinAmpPlugInFinished )
	ON_MESSAGE( WM_USER+0, OnWinAmpPlugWmUser0 )
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPlayerView diagnostics

#ifdef _DEBUG
void CPlayerView::AssertValid() const
{
	CFormView::AssertValid();
}

void CPlayerView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPlayerView message handlers

void CPlayerView::OnInitialUpdate() 
{
	CFormView::OnInitialUpdate();
    CUStringConvert strCnv;
	
	OnUpdate( NULL, WM_CDEX_INITIAL_UPDATE, NULL );

	CPaintDC dc(this);

	m_cVolumeBitmap.Create(&dc,IDB_VOLUME );


	// Allow Drag and dropping of files
	DragAcceptFiles();

	AddMultiPlayerFiles( strCnv.ToT( ((CCDexApp*)AfxGetApp())->GetCommandLineParams() ) );

	// Do we have to debug this stuff?
	m_Volume.Init(	MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,
					NO_SOURCE,
					MIXERCONTROL_CONTROLTYPE_VOLUME,
					CMixerFader::MAIN);

	m_pThread = NULL;
	m_nTotalTime=0;
	m_nCurrentTime=0;

	BOOL	m_bAbortThread = FALSE;
	BOOL	m_bAbortCurrent = FALSE;

	m_nStatus = IDLE;

	SetControls();

	InitWinAmpPlugins( GetSafeHwnd() );

	// Set timer
	SetTimer(TIMERID,TIMERSLOT,NULL);

	CString strValue( g_language.GetString( IDS_CDEX_PLAYER_VERSION ) );
	AfxGetApp()->m_pMainWnd->SetWindowText( strValue + strCnv.ToT(  g_config.GetVersion() ) );


	PlaceControls();

	UpdateData( FALSE );
	Invalidate( FALSE );

	((CCDexApp*)AfxGetApp())->SetInitialized();

}

void CPlayerView::AddMultiPlayerFiles( const CString& strAdd ) 
{
	if ( strAdd.GetLength() > 0 )
	{
		CString strAddFileNames( strAdd );
		int nPos = 0;
		nPos = 0;
		do 
		{
			nPos = strAddFileNames.Find( _T('?' ) );
			if ( nPos <= 0 )
			{
				AddToPlayList( strAddFileNames );
			}
			else
			{
				AddToPlayList( strAddFileNames.Left( nPos ) );
				strAddFileNames = strAddFileNames.Mid( nPos + 1 );
			}		
		} while ( nPos > 0 );
	}
}


void CPlayerView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CString strAddFileNames;

	switch ( lHint)
	{
		case 0:
		break;
		case WM_CDEX_INITIAL_UPDATE:
			// translate the menu items
			g_language.InitDialogStrings( (CDialog*)this, IDR_PLAYERFRAME );
		break;
		case WM_PLAYER_ADD_FILE:
			strAddFileNames =  (LPSTR)pHint;
			AddMultiPlayerFiles( strAddFileNames );
		break;
		case WM_CDEX_SAVE_CD_INFO:
		break;
		default:
			ASSERT( FALSE );
		break;

	}
}

void CPlayerView::OnDetails() 
{
	ENTRY_TRACE( _T( "CPlayerView::OnDetails()" ) );

	if ( m_pInStream )
	{
		m_pInStream->InfoBox( GetSafeHwnd() );
	}

	EXIT_TRACE( _T( "CPlayerView::OnDetails()" ) );
}

void CPlayerView::OnFileOpen() 
{
	POSITION	pos=NULL;

	CString strExt( _W( "M3U;PLS;" ) + GetInFileNameExt() );

	// create custom open file dialog
	COpenFileDlg fileDlg( g_config.GetPlayDir(), CUString( strExt ), IDI_FILE_ICON);

	fileDlg.ShowDelOrigCtrl( FALSE );
	fileDlg.ShowNormalizeCtrl( FALSE );
	fileDlg.ShowRiffWavCtrl( FALSE );
	fileDlg.ShowKeepDirLayoutCtrl( FALSE );

	// check if OK button has been pressed
    if ( IDOK == fileDlg.DoModal() ) 
    {
		// Save to the config file
		g_config.SetPlayDir(fileDlg.GetDir());
		g_config.Save();

		pos = fileDlg.GetStartPosition();


		// Obtain the number of files
		while ( NULL != pos )
		{
			AddToPlayList( CString( fileDlg.GetNextPathName(pos) ) );
		}	
    }
}



void CPlayerView::OnClose() 
{
	CFormView::OnClose();
}


void CPlayerView::SetControls() 
{
	CString strTime;

	int nCurrentFile = m_PlayList.GetCurSel();

	if ( ( nCurrentFile >= 0 ) && ( nCurrentFile < (int)m_vFileNames.size() ) )
	{
		m_strFileName = m_vFileNames[ nCurrentFile ];

		if ( m_strFileName.GetLength() > 70 )
		{
			m_strFileName= _T( "..." ) + m_strFileName.Right( 70 );
		}
	}
	else
	{
		m_strFileName = g_language.GetString( IDS_NOFILES_SELECTED );
	}

	if (m_nStatus==PLAYING || m_nStatus==PAUSING || m_nStatus==SEEKING)
	{
		strTime.Format( _T( "%d:%02d / %d:%02d Bitrate %d kbps" ), 
							( m_nCurrentTime / 60000 ),
							( m_nCurrentTime / 1000 )% 60,
							( m_nTotalTime / 60000 ),
							( m_nTotalTime / 1000 ) % 60,
							m_dwBitRate / 1000 );
	}
	else
	{
		strTime.Format( _T( "%d:%02d / %d:%02d Bitrate %d kbps" ), 
							0, 0, 0, 0, 0 );
	}

	m_TimeCtrl.SetWindowText( strTime );
	UpdateData(FALSE);
}


void CPlayerView::OnTimer(UINT nIDEvent) 
{
	OnUpdate( this,0 , NULL );

	if ( IDLE == m_nStatus )
	{
		m_nCurrentTime = 0;
		m_Position.SetPos( m_nCurrentTime );
	}

	if ( PLAYING == m_nStatus )
	{
		if ( m_pPlayStream )
		{
			m_nCurrentTime = (	m_pPlayStream->GetCurrentTime() + 
								m_dwSeekOffset );
		}

		m_Position.SetRange( 0, m_nTotalTime );
		m_Position.SetPos( m_nCurrentTime );
	}

	SetControls();
}

void CPlayerView::OnPlay() 
{
	if ( m_vFileNames.size() > 0 )
	{
		m_nStatus = PLAYING;
		m_eThreadFinished.ResetEvent();

		// Kick off thread
		m_pThread = AfxBeginThread(	PlayThreadFunc,
									(void*)this,
									g_config.GetThreadPriority() );
	}

	Invalidate( TRUE );
}



void CPlayerView::OnStop() 
{
	ENTRY_TRACE( _T( "CPlayerView::OnStop()" ) );

	// stop immediately the output WAV playing
	if ( m_pPlayStream )
	{
		m_pPlayStream->Pause();
		m_pPlayStream->Stop();
	}

	// Now make sure that the thread gets aborted properly
	m_bAbortThread = TRUE;

	// Set status to abort
	m_nStatus = ABORT;

	// Make sure that pause event has been set
	m_ePaused.SetEvent();

	// Wait for thread to finish
	if ( WAIT_OBJECT_0 != WaitForSingleObject( m_eThreadFinished, 13000 ) )
	{
		if ( m_pThread )
		{
			m_pThread->ResumeThread();
			TerminateThread( m_pThread->m_hThread, -2 );
		}
	}

	m_pThread = NULL;

	m_nStatus = IDLE;

	SetControls();

	m_Position.SetPos( 0 );

	EXIT_TRACE( _T( "CPlayerView::OnStop()" ) );

}

void CPlayerView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( pScrollBar && pScrollBar->m_hWnd == m_Volume.m_hWnd )
	{
		ReflectLastMsg( pScrollBar->GetSafeHwnd() );
	}
	else
	{
//		CPlayerView::OnVScroll( nSBCode, nPos, pScrollBar );
	}
}

void CPlayerView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if ( pScrollBar && pScrollBar->m_hWnd == m_Position.m_hWnd )
	{
		if ( SEEKING == m_nStatus )
		{
			m_nCurrentTime = m_Position.GetPos();
		}

		SetControls();
	}
	else
	{
//		CPlayerView::OnHScroll( nSBCode, nPos, pScrollBar );
	}
}

void CPlayerView::OnNext() 
{
	ENTRY_TRACE( _T( "CPlayerView::OnNext()" ) );

	m_ThreadCommand = THREAD_PLAY_NEXT_TRACK;
	m_bAbortCurrent = TRUE;

	EXIT_TRACE( _T( "CPlayerView::OnNext()" ) );
}

void CPlayerView::OnPause() 
{
	ENTRY_TRACE( _T( "CPlayerView::OnPause()" ) );

	if ( PLAYING == m_nStatus )
	{
		m_nStatus = PAUSING;

		if ( m_pPlayStream )
		{
			m_dwSeekOffset+= m_pPlayStream->GetCurrentTime();

			m_pPlayStream->Pause();
		}

		m_ePaused.ResetEvent();
	}
	else
	{

		if ( m_pPlayStream )
		{
			m_pPlayStream->Resume();
		}

		m_nStatus = PLAYING;

		m_ePaused.SetEvent();

	}
	UpdateData( FALSE );

	EXIT_TRACE( _T( "CPlayerView::OnPause()" ) );

}

void CPlayerView::OnPrev() 
{
	m_ThreadCommand = THREAD_PLAY_PREV_TRACK;
	m_bAbortCurrent = TRUE;
}

LONG CPlayerView::OnWinAmpPlugWmUserMessage( WPARAM wParam, LPARAM lParam )
{
	int nReturn = 0;

	ENTRY_TRACE( _T( "CPlayerView::OnWinAmpPlugWmUserMessage( %d, %d)" ), wParam, lParam );

	switch ( lParam )
	{
		case 104:
			// Return Playing Status 
			if ( PAUSING == m_nStatus  )
			{
				nReturn = 3;
			} else
			if ( PLAYING == m_nStatus  )
			{
				nReturn = 1;
			}
		break;

		case 105:
			// return playing position in ms
			return m_nCurrentTime;
		break;

		case 106:
			// resume playing at pos wParam
			nReturn = 1;
		break;
	}

	EXIT_TRACE( _T( "CPlayerView::OnWinAmpPlugWmUserMessage, return %d" ), nReturn );

	return nReturn;
}

void CPlayerView::OnUpdateNext(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IDLE != m_nStatus  );
}

void CPlayerView::OnUpdatePause(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( PAUSING == m_nStatus );
	pCmdUI->Enable( IDLE != m_nStatus  );
}

void CPlayerView::OnUpdatePlay(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IDLE == m_nStatus  );
}

void CPlayerView::OnUpdatePrev(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IDLE != m_nStatus  );
}

void CPlayerView::OnUpdateStop(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IDLE != m_nStatus  );
}

void CPlayerView::OnUpdateFileopen(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable( IDLE == m_nStatus  );
}

LRESULT CPlayerView::OnStartSeek(WPARAM wParam,LPARAM lParam) 
{
	if (	PLAYING == m_nStatus ||
			PAUSING == m_nStatus )
	{

		if ( m_pPlayStream )
		{
			m_dwSeekOffset+= m_pPlayStream->GetCurrentTime();

			m_pPlayStream->Pause();
		}

		m_ePaused.ResetEvent();
		m_nStatus = SEEKING;
	}
	return 0;
}

LRESULT CPlayerView::OnStopSeek(WPARAM wParam,LPARAM lParam)
{
	if ( SEEKING == m_nStatus )
	{
		if ( m_pPlayStream )
		{
			m_pPlayStream->Resume();
		}

		m_dwSeekOffset = lParam;

		m_nStatus = PLAYING;

		m_ePaused.SetEvent();
	}
	return 0;
}

void CPlayerView::OnDestroy() 
{
	// Abort playing thread, if necessary
	OnStop();

	// Stop the timer
	KillTimer( TIMERID );

	DeInitWinAmpPlugins();
	CFormView::OnDestroy();
	
	
}

void CPlayerView::OnConfigureWinPlugins() 
{
	CWinampInConfig	dlg;
	dlg.DoModal();
}

/*
HBRUSH CPlayerView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// Select transparent drawing mode
	pDC->SetBkMode(TRANSPARENT);

	// Set transparent drawing mode
//	return (HBRUSH)GetStockObject( BLACK_BRUSH );
	return (HBRUSH)GetStockObject( LTGRAY_BRUSH );
	
}
*/

void CPlayerView::OnSelchangePlaylist() 
{
	m_ThreadCommand = THREAD_PLAY_TRACK;
	m_bAbortCurrent = TRUE;
}

void CPlayerView::OnDblclkPlaylist() 
{
	m_bAbortCurrent = TRUE;
	
}

void CPlayerView::OnPlaylistClear() 
{
	// Delete all files
	m_vFileNames.clear();
	m_PlayList.ResetContent();
	
}

void CPlayerView::OnPlaylistSave() 
{
	static TCHAR BASED_CODE szFilter[] = _T( "M3U PlayList (*.m3u)|*.m3u|PLS Playlist (*.pls)|*.pls||" );

	CFileDialog dlg(	FALSE, _T( "m3u" ), _T( "playlist"),
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
						szFilter );


	if ( IDOK == dlg.DoModal() )
	{
		unsigned int i = 0;
		CString strFileName = dlg.GetPathName();

		// delete old file
		FILE* pFile = CDexOpenFile( CUString( strFileName ), _W( "w" ) );
		fclose( pFile );

		// strip exentention
		strFileName = strFileName.Left( strFileName.GetLength() - 4 );

        CUString cuFileName( strFileName ); 
        PlayList playList( cuFileName );

        CString strExt( dlg.GetFileExt() );

		strExt.MakeLower();

		BOOL bIsPls =  strExt == _T( "pls" );


		for ( i = 0; i< m_vFileNames.size() ; i++ )
		{
			CString strEntry = m_vFileNames[ i ];
			if ( bIsPls )
			{
                playList.AddPLSEntry( CUString( strEntry ) );
			}
			else
			{
                playList.AddM3UEntry( CUString( strEntry ) );
			}
		}



	}
}

void CPlayerView::OnDropFiles(HDROP hDropInfo) 
{
	// Get the number of dropped files
	int nTotalFiles=::DragQueryFile(hDropInfo,(UINT)-1,NULL,0);
	
	for (int i=0;i<nTotalFiles;i++)
	{
		TCHAR lpszFileName[ MAX_PATH + 1 ];

		// Get drop file name
		::DragQueryFile( hDropInfo, i, lpszFileName, MAX_PATH );

		AddToPlayList( CString( lpszFileName ) );
	}

	// Finish up the drag and drop session
	::DragFinish(hDropInfo);
}


LRESULT CPlayerView::OnWinAmpPlugInFinished(WPARAM wParam,LPARAM lParam)
{
	WinampPlugInFinished();
	return 0;
}

LONG CPlayerView::OnWinAmpPlugWmUser0(WPARAM wParam,LPARAM lParam)
{
	switch ( lParam )
	{
		case 104:
			// Return Playing Status 
			if ( PAUSING == m_nStatus  )
				return 3;
			if ( PLAYING == m_nStatus  )
				return 1;
			return 0;
		break;
		case 105:
			// return playing position in ms
			return m_nCurrentTime;
		break;
		case 106:
			// resume playing at pos wParam
			return 1;
		break;
	}
	return 0;
}

void CPlayerView::OnOptionsPlayrandom() 
{
	SetPlayRandom( ! GetPlayRandom() );
}

void CPlayerView::OnUpdateOptionsPlayrandom(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck( GetPlayRandom() );
}

void CPlayerView::AddStringToPlayList( const CString& strAdd ) 
{
	m_PlayList.AddString( strAdd );
	CDC* dc = GetDlgItem(IDC_PLAYLIST)->GetDC();
	CSize sz = dc->GetTextExtent( strAdd );
	m_PlayList.SetHorizontalExtent( sz.cx + 3 );
	m_vFileNames.push_back( strAdd );

}

void CPlayerView::AddToPlayList( const CString& strFileName ) 
{
	CString strExt( strFileName );
	CString strDir( strFileName );
	CString strCurrentDir;

	int nPos;

	nPos = strExt.ReverseFind( '.' );

	if ( nPos > 0 ) 
	{
		strExt = strExt.Mid( nPos + 1 );
	}


	nPos = strDir.ReverseFind( _T( '\\' ) );

	if ( nPos > 0 ) 
	{
		strDir = strDir.Left( nPos );
	}
	

	char lpszCurDir[ MAX_PATH + 1 ] = {'\0',};
	
	// Get current directory
	_getcwd( lpszCurDir, sizeof( lpszCurDir ) );

	if ( 0 == strExt.CompareNoCase( _T( "M3U" ) ) )
	{
		// go to the directory
		if ( 0 == _tchdir( strDir ) )
		{
			// open the playlist
			FILE* pFile = CDexOpenFile( CUString( strFileName ), _W( "rt" ) );

			if ( NULL != pFile )
			{
				char lpszLine[ 8192 ] = {'\0',};

				// read the lines in the playlist file
				while ( NULL != fgets( lpszLine, sizeof( lpszLine ), pFile ) )
				{
					if ( '\n' == lpszLine[ strlen( lpszLine ) -1 ] )
					{
						lpszLine[ strlen( lpszLine ) -1 ] = '\0';
					}

					// skip extended info
					if ( '#' != lpszLine[0] )
					{
						CString strEntry( lpszLine );

						int nPos = 0;

						if ( strDir.Find( _T( ":\\" ) ) < 0 ) 
						{
							if ( 0 == ( nPos = strEntry.Find( _T( ".\\" ) ) ) )
							{
								strEntry = strDir + strEntry.Mid( 1 );
							}
							else
							{
								strEntry = strDir + _T( "\\" ) + strEntry;
							}
						}

						AddStringToPlayList( strEntry );
					}
				}

				// close the playlist file
				fclose( pFile );
			}
		}

	}
	else if ( 0 == strExt.CompareNoCase( _T( "PLS" ) ) )
	{
		CIni	plsIni;
		int		nNumEntries = 0;
		CString	strEntry;
		CString	strNumber;
		int		i = 0;
		

		// go to the directory
		if ( 0 == _tchdir( strDir ) )
		{

			plsIni.SetIniFileName( CUString( strFileName ) );

			nNumEntries = plsIni.GetValue(	_T( "playlist" ),
											_T( "NumberOfEntries" ), 
											nNumEntries );

			for ( i = 1; i <= nNumEntries; i++ )
			{
				strNumber.Format( _T( "File%d"), i );

				strEntry = plsIni.GetValue( CUString( _W( "playlist" ) ),
											CUString( strNumber ),
											_W( "" ) );


				if ( !strEntry.IsEmpty() )
				{

					int nPos = 0;

					if ( strDir.Find( _T( ":\\" ) ) < 0 ) 
					{
						if ( 0 == ( nPos = strEntry.Find( _T( ".\\" ) ) ) )
						{
							strEntry = strDir + strEntry.Mid( 1 );
						}
						else
						{
							strEntry = strDir + _T( "\\" ) + strEntry;
						}
					}
					AddStringToPlayList( strEntry );
				}
			}
		}
	}
	else
	{
		AddStringToPlayList( strFileName );
	}

	// switch back to the current directory
	chdir( lpszCurDir );

}


void CPlayerView::PlaceControls()
{
	if ( m_PlayList.m_hWnd )
	{

		CRect	rcList;
		CRect	rcParent;

		// Get size of parent window
		GetClientRect( rcParent );

		// Get size position of Track List
		m_PlayList.GetWindowRect( rcList );

		// Convert from screen to client (this parent window of course)
		ScreenToClient( rcList );

		rcList.right = rcParent.right - 10;
		rcList.bottom = rcParent.bottom;

		// And adjust the size of the track list
		m_PlayList.MoveWindow( rcList );
	}	
}


void CPlayerView::OnSize(UINT nType, int cx, int cy) 
{
	CFormView::OnSize(nType, cx, cy);
	PlaceControls();
}



void CPlayerView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	
	// Get the location of the dialog place holder
	CRect rcSheet;
	GetDlgItem( IDC_PLAYER_VOLUME_ICON )->GetWindowRect( &rcSheet );
	ScreenToClient( &rcSheet );

	CTransBmp::DrawTransparent( &m_cVolumeBitmap,
								&dc,
								rcSheet.left,
								rcSheet.top );
}


