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
#include "MP3ToWavDlg.h"
#include "OSndStreamWAV.h"
#include "AudioFile.h"
#include "MpegToWavSettings.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TIMERID 3
#define TIMERSLOT 800


INITTRACE( _T( "MP3ToWavDlg" ) );


BEGIN_MESSAGE_MAP(CMpegToWavFileOpen, COpenFileDlg )
	//{{AFX_MSG_MAP(CMpegToWavFileOpen)
	ON_BN_CLICKED(IDC_SETTINGS, OnSettings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CMpegToWavFileOpen::CMpegToWavFileOpen(CUString directory, CUString fileType, int iconId, BOOL bOpenDlg, CWnd* pParent )
	: COpenFileDlg(directory, fileType, iconId, bOpenDlg, pParent )
{
	//{{AFX_DATA_INIT(CMpegToWavFileOpen)
	//}}AFX_DATA_INIT
}

CMpegToWavFileOpen::~CMpegToWavFileOpen() 
{
}

void CMpegToWavFileOpen::OnSettings() 
{
	CMpegToWavSettings dlg;
	dlg.DoModal();
}


UINT CMP3ToWavDlg::ConvertFunc(PVOID pParams)
{
	CUString		strLang;
	int			nPeakValue	= 0;
	DWORD		dwIndex		= 0;
	CDEX_ERR	bErr		= CDEX_OK;
	PBYTE		pStream		= NULL;
 	DWORD       dwTotalTime = 0;

	ENTRY_TRACE( _T( "ConvertFunc" ) );

	CMP3ToWavDlg* pDlg = (CMP3ToWavDlg*)pParams;

	pDlg->m_wCurrentTrack = 0;
	pDlg->m_bAbortThread = FALSE;


	CUString strOutDir(g_config.GetCnvOutputDir());

	while(	pDlg->m_wCurrentTrack < pDlg->m_nTotalFiles && 
			!pDlg->m_bAbortThread )
	{
		BOOL bNoToAll = FALSE;

		nPeakValue	= 0;

		POSITION nFileListPos = pDlg->m_Pos;

		pDlg->m_strMP3FileName = pDlg->m_pFileDlg->GetNextPathName( pDlg->m_Pos );
		pDlg->m_strWavFileName = pDlg->m_pFileDlg->GetFileName( nFileListPos );

		CUString strSubPath = pDlg->m_pFileDlg->GetSubPath( nFileListPos );

		int nPos = pDlg->m_strWavFileName.ReverseFind('.');

		if (nPos > 0 )
		{
			pDlg->m_strWavFileName= pDlg->m_strWavFileName.Left( nPos );
		}

		strOutDir = g_config.GetCnvOutputDir() + strSubPath;

		// Prepend output directory
		pDlg->m_strWavFileName= strOutDir + CUString( pDlg->m_strWavFileName );

		// Create Ouput directory
		if ( CDEX_OK != DoesDirExist( strOutDir, FALSE )  )
		{
			LTRACE( _T( "ConvertFunc::Error creating output dir %s" ), strOutDir );

			strLang = g_language.GetString( IDS_CANNOT_CREATE_OUTDIR );
			CDexMessageBox( strLang + strOutDir );
			bErr = CDEX_ERROR;
		}

		// Clear error flag
		bErr = CDEX_OK;

		LTRACE( _T( "Converting track %d" ), pDlg->m_wCurrentTrack );

		// Reset estimate timer
		//pDlg->m_TimeTrack.ReInit();

		pDlg->m_nPercentCompleted=0;

		// create proper input stream
		pDlg->m_pInStream = ICreateStream( CUString( pDlg->m_strMP3FileName ) );

		if ( FALSE == CheckNoFileOverwrite(	pDlg,
										CUString( pDlg->m_strWavFileName ) + _W( ".wav" ), 
										TRUE,
										pDlg->m_bYesToAll,
										bNoToAll ) )
		{
			// Open the input stream if available
			if (	pDlg->m_pInStream && 
					pDlg->m_pInStream->OpenStream( CUString( pDlg->m_strMP3FileName ) ) )
			{
				DWORD	dwBufferSize = pDlg->m_pInStream->GetBufferSize();

				// Reset estimate timer
 				dwTotalTime = pDlg->m_pInStream->GetTotalTime();
 				pDlg->m_TimeTrack.ReInit( dwTotalTime );

				// Create a play stream object
				OSndStreamWAV wavStream;

				wavStream.CopyStreamInfo( *pDlg->m_pInStream );

				// Initialize output WAV stream object
				wavStream.OpenStream( CUString( pDlg->m_strWavFileName ) );

				// Allocate stream input buffer
				pStream = new BYTE[ dwBufferSize ];

				int nMPEGBytes = 0;

				double dSampleRatio = 1.0;

				BOOL bFinished = FALSE;

				while( ( FALSE == bFinished ) && !pDlg->m_bAbortThread )
				{
					nMPEGBytes = pDlg->m_pInStream->Read( pStream, dwBufferSize );
	
					if ( nMPEGBytes > 0 )
					{

						for ( dwIndex=0; dwIndex< (DWORD)nMPEGBytes / 2; dwIndex++ )
						{
							if ( abs( ((SHORT*)pStream)[dwIndex]) > nPeakValue )
								nPeakValue=abs( ((SHORT*)pStream )[ dwIndex ] );
						}

						if ( nMPEGBytes > 0 )
						{
							wavStream.Write( pStream, nMPEGBytes );
						}

						int nTotalTime = pDlg->m_pInStream->GetTotalTime();
						int nCurrentTime = pDlg->m_pInStream->GetCurrentTime();

						if ( nTotalTime )
						{
							pDlg->m_nPercentCompleted = nCurrentTime * 100 / nTotalTime;
						}
						else
						{
							pDlg->m_nPercentCompleted = 0;
						}
					}
					else
					{
						bFinished = TRUE;
					}

				}

				delete [] pStream; pStream= NULL;

				// Close input stream
				pDlg->m_pInStream->CloseStream();

				// Close the WAV stream
				wavStream.CloseStream();

				// close the Input stream

				// Check if we have to normalize the file
				if (	pDlg->m_pFileDlg->m_bNormalize && 
						FALSE == pDlg->m_bAbortThread )
				{
					double dNormFactor=1.0;

					// Reset estimate timer
					pDlg->m_TimeTrack.ReInit(dwTotalTime);

					// Set percentage completed to zero
					pDlg->m_nPercentCompleted=0;

					// Determine normalization factor
					dNormFactor=((nPeakValue-1.0)/327.68);

					// DO we have to normalize this file
					if (dNormFactor<(double)g_config.GetLowNormLevel() || dNormFactor>(double)g_config.GetHighNormLevel())
					{
						if (dNormFactor<(double)g_config.GetLowNormLevel())
						{
							// Normalize to for desired level
							dNormFactor=(double)g_config.GetLNormFactor()/dNormFactor;
						}
						else
						{
							// Normalize to for desired level
							dNormFactor=(double)g_config.GetHNormFactor()/dNormFactor;
						}

						// Little fine tuning to avoid overflows due to round off errors
						dNormFactor*=0.9999;

						LTRACE( _T( "CMP3ToWavDlg::Normalizing Track/File %s with factor %f" ), pDlg->m_strWavFileName, dNormFactor );

						CWAV myWav;

						// Step 1: Open the input WAV file
						if ( myWav.StartNormalizeAudioFile( CUString( pDlg->m_strWavFileName ) )!=0)
						{
							// Step 2: Loop through data and normalize chunk
							while ( myWav.NormalizeAudioFileChunk( dNormFactor, pDlg->m_nPercentCompleted ) == FALSE && 
									!pDlg->m_bAbortThread )
							{
								::Sleep(0);
							}
					
							// Step 3: Close the input WAV file, and replace original
							myWav.CloseNormalizeAudioFile( CUString( pDlg->m_strWavFileName ) , pDlg->m_bAbortThread, TRUE );
						}
					}
				}

				// Check if thread has been aborted, if so, remove original file
				if ( TRUE == pDlg->m_bAbortThread )
				{
					LTRACE( _T( "Deleting file ( due to abort ) %s.wav" ), pDlg->m_strWavFileName );
					DeleteFile( pDlg->m_strWavFileName + _T( ".wav" ) );
				}

				// Check if we have to delete the file
				if ( pDlg->m_pFileDlg->m_bDeleteOriginal && ( FALSE == pDlg->m_bAbortThread )  )
				{
					LTRACE( _T( "Deleting file ( user request ) %s" ), pDlg->m_strMP3FileName );
					DeleteFile( pDlg->m_strMP3FileName );
				}
			}
		}
		else
		{
			if ( TRUE == bNoToAll )
			{
				pDlg->m_bAbortThread = TRUE;
			}
		}

		// delete input stream object
		delete pDlg->m_pInStream;pDlg->m_pInStream=NULL;

		// Jump to the next track
		pDlg->m_wCurrentTrack++;
	}

	pDlg->m_eThreadFinished.SetEvent();

	// OK close the stuff
	pDlg->PostMessage( WM_COMMAND, IDCANCEL,0);

	EXIT_TRACE( _T( "RipFunc" ) );
	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CMP3ToWavDlg dialog


CMP3ToWavDlg::CMP3ToWavDlg( CWnd* pParent /*=NULL*/ )
	: CDialog( CMP3ToWavDlg::IDD, pParent )
{
	m_pThread=NULL;
	m_nPercentCompleted=0;
	m_pFileDlg=NULL;
	m_pInStream=NULL;
	m_pParent = pParent;

	//{{AFX_DATA_INIT(CMP3ToWavDlg)
	m_strFileName = _T("");
	m_strTimeInfo = _T("");
	//}}AFX_DATA_INIT
}


CMP3ToWavDlg::~CMP3ToWavDlg()
{
	delete m_pFileDlg;
	delete m_pInStream;
};

void CMP3ToWavDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMP3ToWavDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Text(pDX, IDC_PROCESSFILENAME, m_strFileName);
	DDX_Text(pDX, IDC_TIMEINFO, m_strTimeInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMP3ToWavDlg, CDialog)
	//{{AFX_MSG_MAP(CMP3ToWavDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMP3ToWavDlg message handlers

BOOL CMP3ToWavDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// Get start position
	m_Pos=m_pFileDlg->GetStartPosition();

	m_strMP3FileName = _T( "" );
	m_strWavFileName = _T( "" );

	m_bYesToAll = FALSE;

	m_eThreadFinished.ResetEvent();

	// Clear to total number of files
	m_nTotalFiles=0;

	// Obtain the number of files
	while ( NULL != m_Pos )
	{
		m_nTotalFiles++;
		m_pFileDlg->GetNextPathName( m_Pos );
	}	
			
	// Get start position again
	m_Pos=m_pFileDlg->GetStartPosition();
	
	// Set range of progress bar
	m_Progress.SetRange( 0, 100 );

	// Fire of thread
	m_pThread = AfxBeginThread(	ConvertFunc,
								(void*)this,
								g_config.GetThreadPriority() );

	// Setup timer, so the UI is update regulary
	SetTimer( TIMERID, TIMERSLOT, NULL );

	return TRUE;
}

void CMP3ToWavDlg::OnTimer(UINT nIDEvent) 
{
	CUString strLang;
	CString strTmp;
	CUString strTime;

	strTmp.Format( _T( " (%d/%d)" ), m_wCurrentTrack+1, m_nTotalFiles );

	// Update the time statistics based on the percentage completed
	m_TimeTrack.Calculate( m_nPercentCompleted );

	// Call the default dialog timer event
	CDialog::OnTimer( nIDEvent );

	// Update progress bar position
	m_Progress.SetPos( m_nPercentCompleted );

	m_strFileName = m_strWavFileName + strTmp;

	if ( m_strFileName.GetLength() > 65 )
	{
		int nPos = m_strFileName.Find( _T( ":" ) );

		if ( nPos < 0 )
			nPos = 0;

		m_strFileName = m_strFileName.Left( nPos + 2 ) +
						_T( " ... " ) +
						m_strFileName.Right( 60 ) ;

	}


	// Get time estimate
	strLang = g_language.GetString( IDS_ELAPSED_TIME );

	strTime.Format( strLang, (LPCWSTR)m_TimeTrack.GetElapsedString(), (LPCWSTR)m_TimeTrack.GetRemainingString(), (LPCWSTR)m_TimeTrack.GetEstimateString() );

	m_strTimeInfo = strTime;

	// Update the controls
	UpdateData(FALSE);
}

void CMP3ToWavDlg::OnCancel() 
{
	ENTRY_TRACE( _T( "CMP3ToWavDlg::OnCancel" ) );

	// Beep if we have finished
	MessageBeep( MB_OK );
	MessageBeep( 0xFFFFFFFF );

	// Set abort flag, so don't process any more files
	m_bAbortThread = TRUE;

	// Stop the timer
	KillTimer( TIMERID );

	// Wait for thread to finish
	WaitForSingleObject( m_eThreadFinished, INFINITE );


	// Destroy myself
	DestroyWindow();

	EXIT_TRACE( _T( "CMP3ToWavDlg::OnCancel" ) );
}

void CMP3ToWavDlg::PostNcDestroy() 
{
	// Notify parent that were closing
	if ( m_pParent )
	{
		m_pParent->SendMessage( WM_DIALOG_CLOSED, 1, 0 );
	}
	LTRACE( _T( "CMP3ToWavDlg::PostNCDestroy()" ) );
}

