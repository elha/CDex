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
#include "RecordFiles.h"
#include "config.h"
#include "RecordWavStream.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "RecordFiles" ) );

#define TIMERID 3
#define TIMERSLOT 200

static const int IDLE = 0;
static const int PAUSED = 1;
static const int RECORDING = 2;
static const int ABORT = 3;

static WORD wCurrentFile = 0;
static BOOL bThreadActive = FALSE;
static WORD g_nRecordedSecs = 0;
static int SampleRates[] ={ 8000, 11025, 16000, 22050, 24000, 32000, 44100, 48000};
static const int nInBufSize = RECSTREAMBUFFERSIZE;

static BYTE pInBuf[ 2 * nInBufSize ];

CTasks CRecordFiles::m_Tasks;

UINT CRecordFiles::RecordThreadFunc( PVOID pParams )
{
	
	CDEX_ERR		bReturn = CDEX_OK;
	int				nSampleRate;
	int				nChannels;
	int				nOffset = 0;
	int				nBytesInBuffer = 0;
	RecordWavStream sndIStream;
	CUString			strFileName;

	bThreadActive = TRUE;

	ENTRY_TRACE( _T( "CRecordFiles::RecordThreadFunc" ) );

	wCurrentFile = 0;

	ASSERT( pParams );

	CRecordFiles* pDlg = (CRecordFiles*)pParams;
	
	pDlg->m_nTotalTime = 0;
	pDlg->m_nCurrentTime = 0;
	
	// Get real sample rate in samples per second
	nSampleRate = SampleRates[ pDlg->m_nSampleRate ];
	nChannels = pDlg->m_nChannels + 1;


	CString strDir;

	pDlg->m_RecOutputDir.GetWindowText( strDir );

	// create output directory does exist, no warning
	DoesDirExist( CUString( strDir ), FALSE );


	if ( pDlg->m_bAddSequenceNumber )
	{
		strFileName.Format( _W( "%s%s-%d" ), (LPCWSTR)CUString( strDir ), (LPCWSTR)CUString( pDlg->m_strRecFileName ), pDlg->m_nSequenceStart );
	}
	else
	{
		strFileName.Format( _W( "%s%s" ), (LPCWSTR)CUString( strDir ), (LPCWSTR)CUString( pDlg->m_strRecFileName ) );
	}

	CTaskInfo newTask;
	newTask.SetFullFileName( strFileName );
	newTask.SetOutDir( g_config.GetMP3OutputDir() );

	pDlg->GetTasks().AddTaskInfo( newTask );

	
	ENCODER_TYPES nEncoderType = ENCODER_FIXED_WAV;

	if ( pDlg->m_nEncoderType > 0 )
	{
		nEncoderType = ENCODER_TYPES( pDlg->m_pEncoder[ pDlg->m_nEncoderType ]->GetID() );
	}

	LTRACE( _T( "Recording to encoder ID %d" ),  nEncoderType );

	// Instantiate new encoder
	auto_ptr<CEncoder> pEncoder( EncoderObjectFactory( nEncoderType ) );

	// Initialize the encoder
	bReturn = pEncoder->InitEncoder( &pDlg->GetTasks().GetTaskInfo( 0 ) );

	if ( CDEX_OK == bReturn )
	{
		// open encoder stream
		bReturn = pEncoder->OpenStream(	strFileName,
										nSampleRate,
										nChannels );
		if ( CDEX_OK == bReturn )
		{
			
			sndIStream.SetSampleRate( nSampleRate );
			sndIStream.SetChannels( nChannels );
			sndIStream.SetRecordingDevice( pDlg->m_RecordingDevice.GetCurSel() );

			if ( FALSE == sndIStream.OpenStream() )
			{
				pDlg->m_nStatus = IDLE;
				pDlg->m_pThread = NULL;
				bThreadActive = FALSE;
				bReturn = CDEX_ERROR;
			}

			if ( CDEX_OK == bReturn )
			{
			
				sndIStream.Start();
							
				DWORD nTotalRecordedBytes = 0;

				DWORD dwSampleBufferSize = pEncoder->GetSampleBufferSize();

				pDlg->m_bThreadError = CDEX_OK;

				while ( FALSE == *(volatile BOOL*)&pDlg->m_bAbort )
				{
					int nRead = 0;

					// if paused, flush the incoming packets
					while ( *(volatile int*)&pDlg->m_nStatus == PAUSED  &&
							FALSE == *(volatile BOOL*)&pDlg->m_bAbort )
					{
						sndIStream.Flush();
						::Sleep( 10 );
					}

					// get one of the recorded memory blocks
					nRead = sndIStream.Read( &pInBuf[ nBytesInBuffer ], RECSTREAMBUFFERSIZE );

					
					nBytesInBuffer += nRead;
					nTotalRecordedBytes += nRead;


					// check if buffer contains enough data to be send to the encoder
					while ( nBytesInBuffer >= (int)(dwSampleBufferSize * sizeof( SHORT ) ) )
					{
						// feed samples to encoder
						bReturn = pEncoder->EncodeChunk( (PSHORT)pInBuf, dwSampleBufferSize );

						if (( CDEX_OK != bReturn ) ||  
							( TRUE == pDlg->m_bAbort ) )
						{
							break;
						}

						// decrement the number of avaialable bytes in the buffer
						nBytesInBuffer -= dwSampleBufferSize * sizeof( SHORT );

						// move samples upfront
						memmove( pInBuf, &pInBuf[ dwSampleBufferSize * sizeof( SHORT ) ], nBytesInBuffer );

					}

					if (( CDEX_OK != bReturn ) ||  
						( TRUE == pDlg->m_bAbort ) )
					{
						LTRACE( _T( "Abort CRecordFiles::RecordThreadFunc main loop" ) );
						break;
					}

					// 16 Bits per channel ! Thus two bytes per sample
					g_nRecordedSecs = (int)nTotalRecordedBytes / nSampleRate / nChannels / sizeof( SHORT );

					if ( 1 == pDlg->m_nTimeLimited )
					{
						if ( (int)(nTotalRecordedBytes / nSampleRate / nChannels / sizeof( SHORT ) ) > pDlg->m_nRecordTime )
						{
							pDlg->m_bAbort = TRUE;
						}
					}
				}
			}
			// close the encoder stream
			pEncoder->CloseStream();
		}
		// deinit the encoder
		pEncoder->DeInitEncoder();
	}

	// close the input stream
	sndIStream.CloseStream();
	
	// update status
	pDlg->m_nStatus = IDLE;
	pDlg->m_pThread = NULL;
	
	bThreadActive = FALSE;

	pDlg->m_bThreadError = bReturn;
	pDlg->m_bIncrementSeqNr = TRUE;

	EXIT_TRACE( _T( "CRecordFiles::RecordThreadFunc" ) );


	return bReturn;
}


/////////////////////////////////////////////////////////////////////////////
// CRecordFiles dialog


CRecordFiles::CRecordFiles(CWnd* pParent /*=NULL*/)
	: CDialog(CRecordFiles::IDD, pParent)
{
	//	m_hRecordBMP=NULL;
	//	m_hStopBMP=NULL;
	//	m_hPauseBMP=NULL;
	ENTRY_TRACE( _T( "CRecordFiles::CRecordFiles()" ) );
	
	m_dwBitRate = 0;
	m_nPercentCompleted = 0;
	
	for ( int i = 0; i < m_nEncoders; i++ )
	{
		m_pEncoder[ i ] = NULL;
	}
	m_nEncoders = 0;

	m_bIncrementSeqNr = FALSE;
	
	m_pThread = NULL;
	m_bAbort = FALSE;
	m_bThreadError = CDEX_OK;

	//{{AFX_DATA_INIT(CRecordFiles)
	m_nSampleRate = sizeof( SampleRates ) / sizeof( SampleRates[ 0 ] -1 );
	m_nChannels = 1;
	m_nTimeLimited = 0;
	m_strRecFileName = _T("Record");
	m_nRecordTime = 60;
	m_nSequenceStart = 0;
	//}}AFX_DATA_INIT

	m_nEncoderType = 0;
	m_bAddSequenceNumber = g_config.GetRecordAddSeqNr();

	m_Tasks.ClearAll();

	EXIT_TRACE( _T( "CRecordFiles::CRecordFiles()" ) );
}

CRecordFiles::~CRecordFiles()
{
	for (int i=0;i<m_nEncoders;i++)
	{
		delete m_pEncoder[i];
	}
}


void CRecordFiles::DoDataExchange(CDataExchange* pDX)
{
	ENTRY_TRACE( _T( "CRecordFiles::DoDataExchange()" ) );

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRecordFiles)
	DDX_Control(pDX, IDC_ENCODERTYPE, m_EncoderType);
	DDX_Control(pDX, IDC_RECORDINGTIME, m_RecordTime);
	DDX_Control(pDX, IDC_FREELENGTH, m_FreeLength);
	DDX_Control(pDX, IDC_RECORDINGDEVICE, m_RecordingDevice);
	DDX_Control(pDX, IDC_RECORD_CHANNELS, m_Channels);
	DDX_Control(pDX, IDC_SAMPLERATE, m_SampleRate);
	DDX_Control(pDX, IDC_PAUSE, m_Pause);
	DDX_Control(pDX, IDC_STOP, m_Stop);
	DDX_Control(pDX, IDC_RECORD, m_Record);
	DDX_CBIndex(pDX, IDC_SAMPLERATE, m_nSampleRate);
	DDX_Radio(pDX, IDC_RECORD_CHANNELS, m_nChannels);
	DDX_Radio(pDX, IDC_FREELENGTH, m_nTimeLimited);
	DDX_Text(pDX, IDC_RECFILENAME, m_strRecFileName );
	DDX_Text(pDX, IDC_RECORDINGTIME, m_nRecordTime);
	DDV_MinMaxInt(pDX, m_nRecordTime, 0, 99999999);
	DDX_Text(pDX, IDC_RECORDSTATUS, m_strRecordStatus);
	DDX_CBIndex(pDX, IDC_ENCODERTYPE, m_nEncoderType);
	DDX_Check(pDX, IDC_ADDSEQUENCENUMBER, m_bAddSequenceNumber);
	DDX_Text(pDX, IDC_SEQUENCESTART, m_nSequenceStart);
	DDV_MinMaxUInt(pDX, m_nSequenceStart, 0, 99999);
	//}}AFX_DATA_MAP

	LTRACE( _T( "CRecordFiles::DoDataExchange() before DDX_Control" ) );

	CUString strLang( g_language.GetString( IDS_PLEASE_SELECTFOLDER ) );
	CUStringConvert strCnv;
    DDX_GetFolderControl(pDX, IDC_RECORDINGOUTPUTDIR, m_RecOutputDir, 0, strCnv.ToT( strLang ) );

	EXIT_TRACE( _T( "CRecordFiles::DoDataExchange()" ) );
}


BEGIN_MESSAGE_MAP(CRecordFiles, CDialog)
	//{{AFX_MSG_MAP(CRecordFiles)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RECORD, OnRecord)
	ON_BN_CLICKED(IDC_STOP, OnStop)
	ON_BN_CLICKED(IDC_PAUSE, OnPause)
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_FREELENGTH, OnFreelength)
	ON_BN_CLICKED(IDC_FREELENGTH1, OnFreelength)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRecordFiles message handlers

BOOL CRecordFiles::OnInitDialog() 
{
	int i;

	ENTRY_TRACE( _T( "CRecordFiles::OnInitDialog()" ) );

	// Init parent
	CDialog::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	LTRACE( _T( "CRecordFiles::OnInitDialog() Init m_Volume" ) );

	LTRACE( _T( "CRecordFiles::OnInitDialog() setup status"  ) );

	// Thread has not been aborted
	m_bAbort = FALSE;

	// No thread yet
	m_pThread = NULL;

	m_nTotalTime = 0;
	m_nCurrentTime = 0;

	m_nStatus = IDLE;

/*
	m_Record.LoadBitmaps(	_T( "IDB_RECORDU" ),
							_T( "IDB_RECORDD" ),
							_T( "IDB_RECORDF" ),
							_T( "IDB_RECORDX" ) );

	m_Pause.LoadBitmaps(	_T( "IDB_RECORDPAUSEU" ),
							_T( "IDB_RECORDPAUSED" ),
							_T( "IDB_RECORDPAUSEF" ),
							_T( "IDB_RECORDPAUSEX" ) );

	m_Stop.LoadBitmaps(		_T( "IDB_RECORDSTOPU" ),
							_T( "IDB_RECORDSTOPD" ),
							_T( "IDB_RECORDSTOPF" ),
							_T( "IDB_RECORDSTOPX" ) );
*/

	LTRACE( _T( "CRecordFiles::OnInitDialog() setup sample rate control"  ) );

	for ( i = 0; i < sizeof( SampleRates ) / sizeof( SampleRates[ 0 ] ); i++ )
	{
		CUString strTmp;
		strTmp.Format( _W( "%d" ), SampleRates[ i ] );

        CUStringConvert strCnv; 

		m_SampleRate.AddString( strCnv.ToT( strTmp ) );
	}


	LTRACE( _T( "CRecordFiles::OnInitDialog() select sample rate"  ) );

	m_SampleRate.SetCurSel( sizeof( SampleRates ) / sizeof( SampleRates[ 0 ] ) - 2 );


	int nRecordingDevices = waveInGetNumDevs();

	LTRACE( _T( "CRecordFiles::OnInitDialog() got %d devices"  ), nRecordingDevices );

	for (i = 0; i < nRecordingDevices; i++)
	{
		WAVEINCAPS waveInCaps;

		memset( &waveInCaps, 0 , sizeof( WAVEINCAPS ) );

		LTRACE( _T( "CRecordFiles::OnInitDialog() quering device %d"  ), i );

		if ( waveInGetDevCaps( i, &waveInCaps, sizeof( WAVEINCAPS ) ) == MMSYSERR_NOERROR )
		{
			LTRACE( _T( "CRecordFiles::OnInitDialog() add device %s"  ), waveInCaps.szPname );

			m_RecordingDevice.AddString( waveInCaps.szPname );
		}

		LTRACE( _T( "CRecordFiles::OnInitDialog() select default recording device"  ) );
		m_RecordingDevice.SetCurSel(0);
	}

	// Get default setting
	if ( nRecordingDevices < g_config.GetRecordingDevice() )
	{
		LTRACE( _T( "CRecordFiles::OnInitDialog() select default recording device %d"  ), g_config.GetRecordingDevice() );
		m_RecordingDevice.SetCurSel( g_config.GetRecordingDevice() );
	}

	LTRACE( _T( "CRecordFiles::OnInitDialog() enable window"  ));
	m_RecordTime.EnableWindow( m_nTimeLimited == 1 );

	LTRACE( _T( "CRecordFiles::OnInitDialog() set controls"  ));
	SetControls();

	LTRACE( _T( "CRecordFiles::OnInitDialog() set encoder"  ));
	SetEncoderTypes();

    CUStringConvert strCnv;
     
	// Set Recording output directory
	m_RecOutputDir.SetWindowText( strCnv.ToT( g_config.GetRecOutputDir() ) );

	LTRACE( _T( "CRecordFiles::OnInitDialog() Update Data"  ));
	UpdateData( FALSE );

	// Start the timer
	LTRACE( _T( "CRecordFiles::OnInitDialog() start the timer"  ));
	SetTimer( TIMERID, TIMERSLOT, NULL );

	EXIT_TRACE( _T( "CRecordFiles::OnInitDialog()" ));

	return TRUE;
}


void CRecordFiles::OnCancel() 
{
	ENTRY_TRACE( _T( "CRecordFiles::OnCancel()" ));

	UpdateData( TRUE );

	// Kill thread if necessary
	m_bAbort = TRUE;
	
	// Make sure the thread has not been paused
	if ( m_pThread )
	{
		m_pThread->ResumeThread();
	}

	// Stop the timer
	KillTimer( TIMERID );
	
	
	// Wait for thread to finish
	if ( m_pThread )
	{
		WaitForSingleObject( m_pThread->m_hThread, INFINITE );
	}
	
	// Save settings
	g_config.SetRecordingDevice( m_RecordingDevice.GetCurSel() );

	g_config.SetRecordAddSeqNr( m_bAddSequenceNumber );

	g_config.SetRecordEncoderType( m_pEncoder[ m_nEncoderType ]->GetID() );

	CString strDir;

	// Set Recording output directory to config file
	m_RecOutputDir.GetWindowText( strDir );

	g_config.SetRecOutputDir( CUString( strDir ) );

	CDialog::OnCancel();

	EXIT_TRACE( _T( "CRecordFiles::OnCancel()" ));
}


void CRecordFiles::SetControls() 
{
}


void CRecordFiles::OnTimer(UINT nIDEvent) 
{
	ENTRY_TRACE( _T( "CRecordFiles::OnTimer()" ) );
    CUStringConvert strCnv;

	GetDlgItem( IDC_RECORD )->EnableWindow( m_nStatus == IDLE );
	GetDlgItem( IDC_PAUSE )->EnableWindow( m_nStatus != IDLE );
	GetDlgItem( IDC_STOP )->EnableWindow( m_nStatus == RECORDING );

	if ( FALSE == bThreadActive )
	{
		// No thread yet
		m_pThread = NULL;

		if ( GetDlgItem( IDC_RECORDSTATUS ) )
		{
			GetDlgItem( IDC_RECORDSTATUS )->SetWindowText( _T( "Idle" ) );
		}
	}
	else
	{
		CUString strTmp;

		strTmp.Format(	_W( "Recorded %d seconds" ), g_nRecordedSecs );


		if ( GetDlgItem( IDC_RECORDSTATUS ) )
		{
			GetDlgItem( IDC_RECORDSTATUS )->SetWindowText( strCnv.ToT( strTmp ) );
		}
	}

	if ( m_bIncrementSeqNr )
	{
		m_bIncrementSeqNr = FALSE;

		m_nSequenceStart++;

		UpdateData( FALSE );
	}

	if ( GetDlgItem( IDC_PAUSE ) )
	{
		((CButton*)GetDlgItem( IDC_PAUSE ) )->SetCheck( m_nStatus == PAUSED );
	}


	CDialog::OnTimer( nIDEvent );

	EXIT_TRACE( _T( "CRecordFiles::OnTimer()" ) );
}

void CRecordFiles::OnRecord() 
{
	ENTRY_TRACE( _T( "CRecordFiles::OnRecord()" ) );

	// Get latest data out of controls
	UpdateData( TRUE );
	
	if ( NULL == m_pThread )
	{
		g_nRecordedSecs = 0;
		m_bAbort = FALSE;
		
		// Kick off thread
		m_pThread = AfxBeginThread( RecordThreadFunc, this, g_config.GetThreadPriority());

		// Set status
		m_nStatus = RECORDING;
	}
    GetDlgItem( IDC_STOP )->SetFocus();

    EXIT_TRACE( _T( "CRecordFiles::OnRecord()" ) );
}



void CRecordFiles::OnStop() 
{
    // set focus to record buton
    m_Record.SetFocus();
    this->SetFocus();

	ENTRY_TRACE( _T( "CRecordFiles::OnStop() " ) );

	m_bAbort = TRUE;
	
	if ( m_pThread )
	{
		m_pThread->ResumeThread();
	}

	EXIT_TRACE( _T( "CRecordFiles::OnStop() " ) );
}



void CRecordFiles::OnPause() 
{
	ENTRY_TRACE( _T( "CRecordFiles::OnPause() " ) );

	if ( m_nStatus == RECORDING )
	{
		m_nStatus = PAUSED;
	}
	else
	{
		m_nStatus = RECORDING;
	}
	EXIT_TRACE( _T( "CRecordFiles::OnPause(), status is %d" ), m_nStatus );

}


void CRecordFiles::SetEncoderTypes() 
{
	ENTRY_TRACE( _T( "CRecordFiles::SetEncoderTypes()" ) );

	int nEncoder;

	m_nEncoders = 0;

	for ( nEncoder = ENCODER_FIXED_WAV; nEncoder < NUMENCODERS; nEncoder++ )
	{
		m_pEncoder[ m_nEncoders ] = EncoderObjectFactory( ENCODER_TYPES( nEncoder ) );

		if ( m_pEncoder[ m_nEncoders ] && m_pEncoder[ m_nEncoders ]->GetAvailable() )
		{
			LTRACE( _T( "CRecordFiles::SetEncoderTypes() adding encoder %d type :%s:" ), nEncoder, m_pEncoder[ m_nEncoders ]->GetEncoderID() );
            CUStringConvert strCnv;

			// Add encoder string to encoder selection combo
			m_EncoderType.AddString( strCnv.ToT( m_pEncoder[ m_nEncoders ]->GetEncoderID() ) );

			// Is this the selected encoder ?
			if ( g_config.GetRecordEncoderType() == m_pEncoder[ m_nEncoders ]->GetID() )
			{
				m_nEncoderType = m_nEncoders;
			}

			m_nEncoders++;
		}
	}


	LTRACE( _T( "CRecordFiles::SetEncoderTypes() m_nEncoderType is set to %d" ), m_nEncoderType );

	EXIT_TRACE( _T( "CRecordFiles::SetEncoderTypes()" ) );
}

void CRecordFiles::OnFreelength() 
{
	ENTRY_TRACE( _T( "CRecordFiles::OnFreelength()" ));

	UpdateData( TRUE );

	m_RecordTime.EnableWindow( m_nTimeLimited == 1 );

	UpdateData( FALSE );

	EXIT_TRACE( _T( "CRecordFiles::OnFreelength()" ));
}
