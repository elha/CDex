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
//#include <stdio.h>
//#include <direct.h>
//#include <math.h>
//#include "CDexEngine.h"
//
//
//typedef struct DETECT_GAPS_PARAMS_TAG
//{
//    <vector WORD>& gaps;
//    bool bAbortThread;
//    SETGUIMSG* SetOutputMessage;
//
//} DETECT_GAPS_PARAMS;
//
//
//CCDexEngine::CCDexEngine()
//{
//    m_pMessageCallback = NULL;
//}
//
//
//class CCDexEngine::DetectGaps( <vector WORD>& detectedGaps )
//{
//    CCDexEngine() {};
//    virtual ~CCDexEngine() {};
//    void ;
//};

//#include "cdex.h"
//
//#include "cdexDoc.h"
//#include "cdexView.h"
//#include "CopyDlg.h"
//#include "config.h"
//#include "Audiofile.h"
//#include "ID3Tag.h"
//#include "Filename.h"
//#include "Encode.h"
//#include "AsyncEncoder.h"
//#include "ISndStream.h"
//#include "SndStreamFactory.h"
//#include "StatusReporter.h"
//
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif
//
//INITTRACE( _T( "CopyDlg" ) );
//
//
//
//typedef BOOL (PASCAL *GFDPEX )(LPCSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);
//
//
//
//static void GetCDRipInfo(int& nSampleFreq,int& nChannels,int& nBitsPerChannel)
//{
//	// Initialize to default values
//	nSampleFreq = 44100;
//	nChannels = 2;
//	nBitsPerChannel = 16;
//}
//
//
//#define TIMERID 2
//#define TIMERSLOT 800
//
//
//#define IS_IDLE					0
//#define IS_READING				1
//#define IS_CONVERTING			2
//#define IS_READYTOCONVERT		3
//#define IS_NORMALIZING			4
//#define IS_READYTONORMALIZE		5
//#define IS_WRITETAG				6
//#define IS_NONE					7
//#define IS_DETPEAKVALUE			8
//
//CTasks CCopyDialog::m_Tasks;
//CTaskInfo CCopyDialog::m_CurrentTask;
//
//
//CCopyDialog::CCopyDialog(CCDexView* pView):
//			CDialog(CCopyDialog::IDD, pView)
//{
//	// Set view pointer
//	m_pView = pView;
//
//	// initialize variables
//	m_wCurrentTrack			= 0;
//	m_iStatus				= IS_IDLE;
//
//	m_pRipInfoDB			= NULL;
//
//	m_bResetTimeTrack		= FALSE;
//	m_bYesToAll				= TRUE;
//	m_bRetainWavFile		= FALSE;
//
//	m_nPercent				= 0;
//
//	m_nJitterErrors			= 0;
//	m_nJitterPos			= 50;
//
//	m_bCancelled			= FALSE;
//
//	m_Tasks.ClearAll();
//
//	EXIT_TRACE( _T( "CCopyDialog::CCopyDialog" ) );
//
//}
//
//CCopyDialog::~CCopyDialog()
//{
//	ENTRY_TRACE( _T( "CCopyDialog::~CCopyDialog" ) );
//	delete m_pRipInfoDB;
//	m_pRipInfoDB = NULL;
//	EXIT_TRACE( _T( "CCopyDialog::~CCopyDialog" ) );
//}
//
//void CCopyDialog::DoDataExchange(CDataExchange* pDX)
//{
//    CDialog::DoDataExchange(pDX);
//    //{{AFX_DATA_MAP(CCopyDialog)
//    DDX_Control(pDX, IDC_RECORDCTRL, m_TrackCtrl);
//    DDX_Control(pDX, IDC_JITTERCTRL, m_JitterCtrl);
//    DDX_Text(pDX, IDC_INFO1, m_strInfo1);
//    DDX_Text(pDX, IDC_INFO2, m_strInfo2);
//    DDX_Text(pDX, IDC_PEAKVALUE, m_strPeakValue);
//    DDX_Text(pDX, IDC_INFO3, m_strInfo3);
//    //}}AFX_DATA_MAP
//    DDX_Control(pDX, IDC_INFO4, m_strInfo4);
//    DDX_Control(pDX, IDC_INFO5, m_strInfo5);
//}
//
//
//BEGIN_MESSAGE_MAP(CCopyDialog, CDialog)
//	//{{AFX_MSG_MAP(CCopyDialog)
//	ON_WM_TIMER()
//	//}}AFX_MSG_MAP
//END_MESSAGE_MAP()
//
//
//
//
//CDEX_ERR CCopyDialog:: CalculateNormalizationFactor( CCopyDialog*	pDlg )
//{
//	CDEX_ERR bReturn = CDEX_OK;
//
//	double dNormPercentage = CTaskInfo::NORM_DEFAULT_VALUE * 100.0;
//
//	ASSERT( pDlg );
//
//	ENTRY_TRACE( _T( "CCopyDialog::CalculateNormalizationFactor( %p )" ), pDlg );
//
//	// Determine normalization factor
//	dNormPercentage = ( pDlg->GetCurrentTask().GetPeakValue() - CTaskInfo::NORM_TUNE_FACTOR ) / 32768.0 * 100;
//
//	// Is normaliztion required ?
//	if ( dNormPercentage < (double)g_config.GetLowNormLevel() )
//	{
//		// Normalize to the desired level
//
//		pDlg->GetCurrentTask().SetNormalizationFactor( 
//												(double) g_config.GetLNormFactor() / 
//												dNormPercentage * 
//												CTaskInfo::NORM_TUNE_FACTOR );
//
//
//	}
//	else if ( dNormPercentage > (double)g_config.GetHNormFactor() )
//	{
//		// Normalize to the desired level
//		pDlg->GetCurrentTask().SetNormalizationFactor( 
//												(double) g_config.GetHNormFactor() / 
//												dNormPercentage * 
//												CTaskInfo::NORM_TUNE_FACTOR );
//	}
//	else
//	{
//		pDlg->GetCurrentTask().SetNormalizationFactor( CTaskInfo::NORM_DEFAULT_VALUE );
//
//	}
//
//	LTRACE( _T( "CCopyDialog::CalculateNormalizationFactor(), peak value = %d, norm levels(%d,%d), norm factors(%d,%d), dNormPercentage = %7.4f => normalization factor set to %7.4f" ),
//		pDlg->GetCurrentTask().GetPeakValue( ),
//		g_config.GetLowNormLevel(),
//		g_config.GetHighNormLevel(),
//		g_config.GetLNormFactor(),
//		g_config.GetHNormFactor(),
//		dNormPercentage,
//		pDlg->GetCurrentTask().GetNormalizationFactor() );
//
//	EXIT_TRACE( _T( "CCopyDialog::CalculateNormalizationFactor( %p ), return value: %d" ), pDlg, bReturn );
//
//	return bReturn;
//}
//
//
//CDEX_ERR CCopyDialog::RipToEncoder(	CCopyDialog*	pDlg,
//									ENCODER_TYPES	nEncoderType,
//									BOOL			bIsTempFile,
//									BOOL&			bNoToAll )
//{		
//	CDEX_ERR		bReturn			= CDEX_OK;
//	CUString			strLang;
//	int				nSampleFreq		= 44100;
//	int				nChannels		= 2;
//	int				nBitsPerChannel = 16;
////	LARGE_INTEGER	liRipStart;
////	LARGE_INTEGER	liRipStop;
//	LARGE_INTEGER	liTicksPerSecond;
//	DWORD			dwNumberOfSamples = 0;
//
//	ENTRY_TRACE( _T( "CCopyDialog::RipToEncoder, nEncoderType = %d"), nEncoderType );
//
//	// Initialize paramters
//	pDlg->m_nPercent		= 0;
//	pDlg->m_nJitterErrors	= 0;
//
//	pDlg->GetCurrentTask().SetPeakValue( 0 );
//
//
//	// get high performance counter frequency
//	QueryPerformanceFrequency( &liTicksPerSecond );
//	DOUBLE dTicksPerSecond = (DOUBLE)liTicksPerSecond.QuadPart;
//
//	// initialize encoder object we have to rip to
//	auto_ptr<CEncoder> pEncoder( EncoderObjectFactory( nEncoderType ) );
//
//	CUString strRipFileNoExt( GetCurrentTask().GetOutFileNameNoExt() );
//	CUString strFullRipFile( GetCurrentTask().GetOutFullFileName() );
//	CUString strFullRipFileNoExt( GetCurrentTask().GetOutFullFileNameNoExt() );
//
//	// Initialize the encoder
//	if ( CDEX_OK != pEncoder->InitEncoder( &GetCurrentTask() ) )
//	{
//		return CDEX_FILEOPEN_ERROR;
//	}
//
//	// special case, we first have to rip to a WAV file (temporarily)
//	if ( ENCODER_FIXED_WAV == nEncoderType )
//	{
//		// Get the information regarding the CD-Ripping settings
//		GetCDRipInfo( nSampleFreq, nChannels, nBitsPerChannel );
//	}
//
//	if ( bIsTempFile )
//	{
//		// rip to temp dir
//		pDlg->GetCurrentTask().SetInDir( g_config.GetTempDir() );
//
//		// set input file name
//		GetCurrentTask().SetFullFileName( g_config.GetTempDir() + strRipFileNoExt + _W( ".wav" ) );
//
//		strFullRipFile = GetCurrentTask().GetFullFileName( );
//		strFullRipFileNoExt = GetCurrentTask().GetFullFileNameNoExt();
//	}
//
//	// Check if file name does already exist, but exclude temp files
//	if ( !bIsTempFile &&
//		 CheckNoFileOverwrite( pDlg, strFullRipFile, TRUE, pDlg->m_bYesToAll, bNoToAll ) )
//	{
//		return CDEX_FILEOPEN_ERROR;
//	}
//
//	// Reset estimate timer
//	pDlg->m_bResetTimeTrack = TRUE;
//
//	// Reserve space for ID3 V2 tag
//	if ( ( g_config.GetID3Version() >= ID3_VERSION_2 ) && pEncoder->GetCanWriteTagV2() )
//	{
//		// Reserve space for ID3V2 tag
//		pEncoder->SetId3V2PadSize( 4096 );
//	}
//
//	// Open conversion stream of encoder
//	if ( CDEX_OK != pEncoder->OpenStream(	strFullRipFileNoExt,
//											nSampleFreq,
//											nChannels ) )
//	{
//		return CDEX_FILEOPEN_ERROR;
//	}
//
//	// Step 2: Get the requested buffer size of the output stream
//	DWORD dwSampleBufferSize = pEncoder->GetSampleBufferSize();
//
//	// Step 3: Setup the ripper
//	LONG nBufferSize;
//
//	CUString strRipInfoDB;
//
//	// Create RipInfoDB file names based on CDDB ID
//    CTagData& tagData( GetCurrentTask().GetTagData() );
//
//	strRipInfoDB.Format( _W( "%08X"), tagData.GetCDBID() );
//
//	// Create a RipInfoDB object
//	pDlg->m_pRipInfoDB = new CRipInfoDB;
//
//	// Set Rip information file name
//	pDlg->m_pRipInfoDB->SetFileName( strRipInfoDB );
//
//	// Delete old stuff
//    pDlg->m_pRipInfoDB->DeleteTrack( tagData.GetTrackNumber() );
//
//	// Set current track number
//	pDlg->m_pRipInfoDB->SetCurrentTrack( tagData.GetTrackNumber() );
//
//	// Create start info
//	CTime myTime( CTime::GetCurrentTime() );
//	strLang = g_language.GetString( IDS_RIP_TRACK_TO_MPEG );
//	strRipInfoDB.Format( strLang, myTime.Format("%A, %B %d, %Y %H:%M:%S"), (LPCWSTR)GetCurrentTask().GetFullFileName() );
//
//	// Add start info to RipFileInfo
//	pDlg->m_pRipInfoDB->SetRipInfo( strRipInfoDB );
//
//
//	LTRACE( _T( "CCopyDialog::RipToEncoder, ripping from sector :%d up an till sector %d = %d sectors" ), 
//			GetCurrentTask().GetStartSector(),
//			GetCurrentTask().GetEndSector(),
//			GetCurrentTask().GetEndSector() - GetCurrentTask().GetStartSector() + 1 );
//
//	if (CR_OpenRipper(	&nBufferSize,
//						GetCurrentTask().GetStartSector(),
//						GetCurrentTask().GetEndSector(),
//                        TRUE
//						)==CDEX_OK)
//	{
//		CAsyncEncoder	feeder( pEncoder.get(), (BOOL&)pDlg->m_bAbortThread, dwSampleBufferSize, 256 );
//		LONG			nNumBytesRead = 0;
//		LONG			nOffset = 0;
//	
//		// create the stream buffer, allocate on enocder frame additional memory
//        // allocate extra memory for offset correction
//		auto_ptr<BYTE> pbtBufferStream( new BYTE[ nBufferSize + dwSampleBufferSize * sizeof( SHORT ) + 16383 ] );
//
//		// Get a pointer to the buffer
//		BYTE* pbtStream = pbtBufferStream.get();
//
//		CDEX_ERR ripErr;
//
////		QueryPerformanceCounter( &liRipStart );
//
//		// Read all chunks
//		while (	( CDEX_RIPPING_DONE  != ( ripErr = CR_RipChunk( pbtStream + nOffset, &nNumBytesRead, (BOOL&)pDlg->m_bAbortThread ) ) ) 
//				&& !pDlg->m_bAbortThread )
//		{
//			SHORT*	psEncodeStream=(SHORT*)pbtStream;
//			DWORD	dwSamplesToConvert= ( nNumBytesRead + nOffset ) / sizeof( SHORT );
///*
//			QueryPerformanceCounter( &liRipStop );
//
//			DOUBLE dRipTicks = ( (DOUBLE)liRipStop.QuadPart - (DOUBLE)liRipStart.QuadPart );
//
//			if ( dTicksPerSecond )
//			{
//				DOUBLE dRipTimeInSecs = dRipTicks / ( dTicksPerSecond );
//				CUString strOut;
//					strOut.Format( "Rip time %f\n", dRipTimeInSecs );
//				OutputDebugString( strOut );
//			}
//*/
//			// Check for jitter errors
//			if ( CDEX_JITTER_ERROR == ripErr )
//			{
//				DWORD dwStartSector,dwEndSector;
//
//				// Get info where jitter error did occur
//				CR_GetLastJitterErrorPosition( dwStartSector, dwEndSector );
//
//				// Add the jitter error to the logging file
//				pDlg->m_pRipInfoDB->SetJitterError( dwStartSector, dwEndSector, GetCurrentTask().GetStartSector() );
//			}
//
//			// Check if an error did occur
//			if ( CDEX_ERROR == ripErr )
//			{
//				LTRACE( _T( "RipToEncoder::CDEX_ERROR" ) );
//				break;
//			}
//
//			// Get progress indication
//			pDlg->m_nPercent = CR_GetPercentCompleted();
//
//			// Get relative jitter position
//			pDlg->m_nJitterPos = CR_GetJitterPosition();
//
//			// Get the number of jitter errors
//			pDlg->m_nJitterErrors = CR_GetNumberOfJitterErrors();
//
//			// Get the Peak Value
//			pDlg->GetCurrentTask().SetPeakValue( CR_GetPeakValue() );
//
//			// Convert the samples with the encoder
//			while ( dwSamplesToConvert >= dwSampleBufferSize )
//			{
//				if ( (BOOL&)pDlg->m_bAbortThread )
//				{
//					return CDEX_ERROR;
//				}
//
//				dwNumberOfSamples += dwSampleBufferSize;
//
//				// add samples to feeder
//				if( CDEX_OK != feeder.Add( psEncodeStream, dwSampleBufferSize ) )
//				{
//					return CDEX_ERROR;
//				}
//
//				// Decrease the number of samples to convert
//				dwSamplesToConvert -= dwSampleBufferSize;
//
//				// Increase the sample buffer pointer
//				psEncodeStream += dwSampleBufferSize;
//			}
//
//			// Copy the remaing bytes up front, if necessary
//			if ( dwSamplesToConvert > 0 )
//			{
//				// Calculate the offset in bytes
//				nOffset = dwSamplesToConvert * sizeof( SHORT );
//
//				// Copy up front
//				memcpy( pbtStream, psEncodeStream, nOffset );
//			}
//			else
//			{
//				nOffset = 0;
//			}
//
////			QueryPerformanceCounter( &liRipStart );
//		}
//		
//		if ( nOffset && !pDlg->m_bAbortThread )
//		{
//			dwNumberOfSamples += nOffset / sizeof( SHORT );
//
//			if(feeder.Add((SHORT*)pbtStream, nOffset / sizeof( SHORT ) )!= CDEX_OK )
//			{
//				return CDEX_ERROR;
//			}
//		}
//		LTRACE( _T( "CCopyDialog::RipToEncoder, Wait for encoder to Finish" ) );
//
//		feeder.WaitForFinished();
//
//		LTRACE( _T( "CCopyDialog::RipToEncoder, Encoder finished" ) );
//
//		// Close the Ripper session
//        CR_CloseRipper( &pDlg->m_lCRC );
//	}
//
//	// set length in msec
//	GetCurrentTask().SetLengthInMs( (DWORD)( (double)dwNumberOfSamples / (double)nSampleFreq / (double)nChannels * 1000.0 ) );
//
//	LTRACE( _T( "CCopyDialog::RipToEncoder, LengthInMs = %d" ), GetCurrentTask().GetLengthInMs() );
//
//	// Create prolog info
//	CTimeSpan myEndTime = CTime::GetCurrentTime()-myTime;
//
//	LTRACE( _T( "CCopyDialog::RipToEncoder, bAbort = %d" ), pDlg->m_bAbortThread );
//
//	if ( TRUE == pDlg->m_bAbortThread )
//	{
//		myTime = CTime::GetCurrentTime();
//
//		strLang = g_language.GetString( IDS_RIP_ENCODE_ABORT );
//
//		strRipInfoDB.Format( CUString( myTime.Format( _T( "%A, %B %d, %Y %H:%M:%S" ) ) ) );
//
//		// Add prolog info to RipFileInfo
//		pDlg->m_pRipInfoDB->SetAbortError( strRipInfoDB );
//	}
//	else
//	{
//		strLang = g_language.GetString( IDS_RIP_FINISHED_OK );
//
//		strRipInfoDB.Format( strLang, myEndTime.Format( _T( "%H:%M:%S" ) ) );
//
//		// Add prolog info to RipFileInfo
//		pDlg->m_pRipInfoDB->SetRipInfoFinshedOK( strRipInfoDB,  pDlg->m_lCRC );
//	}
//
//	// Close the output stream
//	pEncoder->CloseStream();
//
//	// De-initialize the encoder
//	pEncoder->DeInitEncoder();
//
//	// When aborted, return an error
//	if ( pDlg->m_bAbortThread )
//	{
//		LTRACE( _T( "RipToEncoder:: bAbort = TRUE" ) );
//		bReturn = CDEX_ERROR;
//	}
//
//	EXIT_TRACE( _T( "CCopyDialog::RipToEncoder, return value %d" ), bReturn );
//
//	// Everything went well, indicate so
//	return bReturn;
//}
//
//
//CDEX_ERR CCopyDialog::WavToMpeg(	CCopyDialog*	pDlg,
//									INT&			nSampleRate,
//									INT&			nChannels,
//									BOOL			bWriteId3V2Tag,
//									BOOL&			bNoToAll )
//{		
//	CUString		strLang;
//	CDEX_ERR	bReturn		= CDEX_OK;
//	BOOL		bEncoderStreamOpen = FALSE;
//	BOOL		bInStreamOpen = FALSE;
//
//			
//	ENTRY_TRACE( _T( "CCopyDialog::WavToMpeg, sample rate %d channels %d bWriteId3V2Tag %d" ),
//		nSampleRate,
//		nChannels,
//		bWriteId3V2Tag );
//
//	// Step 0: Open the encoder
//	auto_ptr<CEncoder> pEncoder( EncoderObjectFactory( GetCurrentTask().GetEncoderType() ) );
//
//	CTime myTime( CTime::GetCurrentTime() );
//
//	// Set the normalization value 
//	pEncoder->SetNormalizationFactor( pDlg ->GetCurrentTask().GetNormalizationFactor() );
//
//	// Determine if encoder supports Chunk encoding
//	if ( pEncoder->GetChunkSupport() )
//	{
//		DWORD dwOutBufferSize	= 0;
//		DWORD dwInBufferSize	= 0;
//		DWORD dwStreamIndex		= 0;
//
//		// create the input stream
//		auto_ptr<ISndStream> pInStream( ICreateStream( GetCurrentTask().GetFullFileName() ) );
//
//		// Initialize the encoder
//		bReturn = pEncoder->InitEncoder( &GetCurrentTask() );
//
//		// Step 1: Check that input name not equal to output name
//		if ( CDEX_OK == bReturn  )
//		{
//			if ( 0 == GetCurrentTask().GetFullFileName().CompareNoCase( GetCurrentTask().GetOutFullFileName() ) )
//			{
//				CUString strLang;
//				CUString strMsg;
//				strLang = g_language.GetString( IDS_INPUTISOUTPUTFILENAME );
//				strMsg.Format( strLang, (LPCWSTR)GetCurrentTask().GetFullFileName(), (LPCWSTR)GetCurrentTask().GetOutFullFileName() );
//                CDexMessageBox( strMsg );
//				bReturn = CDEX_ERROR;
//			}
//		}
//
//		// Step 2: Setup and open the input stream
//		if ( CDEX_OK == bReturn  )
//		{
//
//			// Make exception for MPEG to MPEG transcoding
//			if (	SndStream::SNDSTREAM_MPEG == pInStream->GetStreamType() &&
//					pEncoder->GetCanWriteTagV2() )
//			{
//				// bWriteId3V2Tag = TRUE;
//			}
//
//			// Open input stream
//			if ( ( NULL != pInStream.get() ) && ( pInStream->OpenStream( GetCurrentTask().GetFullFileName() ) ) )
//			{
//				bInStreamOpen = TRUE;
//
//				nSampleRate= pInStream->GetSampleRate();
//
//				// Check if file name does already exist
//				if ( TRUE == CheckNoFileOverwrite( pDlg, GetCurrentTask().GetOutFullFileName(), TRUE, pDlg->m_bYesToAll, bNoToAll  ) )
//				{
//					bReturn = CDEX_FILEOPEN_ERROR;
//				}
//			}
//			else
//			{
//				strLang = g_language.GetString( IDS_ERROR_OPENING_INPUT_FILE );
//				CUString strTmp;
//				strTmp.Format( _W( "%s\r\n%s" ), (LPCWSTR)strLang, (LPCWSTR)GetCurrentTask().GetFullFileName() );
//                CDexMessageBox( strTmp );
//				bReturn = CDEX_ERROR;
//			}
//
//			pDlg->m_bResetTimeTrack = TRUE;
//		}
//
//		// Step 3: Setup and open the output stream
//		if ( CDEX_OK == bReturn  )
//		{
//			dwInBufferSize = pInStream->GetBufferSize();
//
//			// Reserve space for ID3 V2 tag
//			if ( ( g_config.GetID3Version() >= ID3_VERSION_2 ) && bWriteId3V2Tag )
//			{
//				// Reserve space for ID3V2 tag
//				pEncoder->SetId3V2PadSize( 4096 );
//			}
//
//			// Open encoding stream
//			bReturn = pEncoder->OpenStream(	GetCurrentTask().GetOutFullFileNameNoExt(),
//											pInStream->GetSampleRate(),
//											pInStream->GetChannels() );
//
//			bEncoderStreamOpen = (CDEX_OK == bReturn);
//
//			dwOutBufferSize = pEncoder->GetSampleBufferSize() * 2;
//
//		}
//
//		// Step 4: Create input stream buffer
//		auto_ptr<BYTE> pStream( new BYTE[ dwInBufferSize + dwOutBufferSize ] );
//
//		if ( NULL == pStream.get() )
//		{
//			bReturn = CDEX_ERROR;
//		}
//
//		// Step 5: Convert the input stream
//		if ( CDEX_OK == bReturn  )
//		{
//			BOOL bFinished = FALSE;
//
//			while ( ( FALSE == pDlg->m_bAbortThread ) && ( CDEX_OK == bReturn ) && !bFinished )
//			{
//				int nInputBytes = pInStream->Read( (BYTE*)( pStream.get() ) + dwStreamIndex, dwInBufferSize );
//
//				if ( nInputBytes > 0 )
//				{
//					// increase current stream index position
//					dwStreamIndex+= nInputBytes;
//
//					// play the stuff when there is STREAMBUFFERSIZE samples are present
//					while ( ( dwStreamIndex >= dwOutBufferSize ) && 
//							( FALSE == pDlg->m_bAbortThread ) )
//					{
//						// Encode this chunk
//						bReturn = pEncoder->EncodeChunk( (SHORT*)pStream.get() , dwOutBufferSize / 2 );
//
//						if (  bReturn == CDEX_OK ) 
//						{
//							dwStreamIndex-= dwOutBufferSize;
//
//							if ( dwStreamIndex )
//							{
//								memmove( pStream.get(), (BYTE*)( pStream.get() ) + dwOutBufferSize, dwStreamIndex );
//							}
//
//							// Update percentage
//							pDlg->m_nPercent = pInStream->GetPercent();
//						}
//					}
//				}
//				else
//				{
//					// play the stuff when there is STREAMBUFFERSIZE samples are present
//					while ( dwStreamIndex && ( CDEX_OK == bReturn ) ) 
//					{
//						dwOutBufferSize = min( dwOutBufferSize, dwStreamIndex );
//
//						// Encode this chunk
//						bReturn = pEncoder->EncodeChunk( (SHORT*)pStream.get() , dwOutBufferSize / 2 );
//
//						if ( bReturn == CDEX_OK ) 
//						{
//
//							dwStreamIndex-= dwOutBufferSize;
//
//							if ( dwStreamIndex )
//							{
//								memmove( pStream.get(), (BYTE*)( pStream.get() ) + dwOutBufferSize, dwStreamIndex );
//							}
//
//							// Update percentage
//							pDlg->m_nPercent = pInStream->GetPercent();
//						}
//					}
//					bFinished = TRUE;
//				}
//			}
//		}
//
//		// set length in msec
//		if( bInStreamOpen )
//		{
//			GetCurrentTask().SetLengthInMs( pInStream->GetTotalTime() );
//
//			LTRACE( _T( "CCopyDialog::WavToMpeg, LengthInMs = %d" ), GetCurrentTask().GetLengthInMs() );
//
//			// Close the input stream
//			pInStream->CloseStream();
//		}
//
//		if( bEncoderStreamOpen )
//		{
//			// Close the output stream
//			pEncoder->CloseStream();
//		}
//
//		if ( CDEX_OK == bReturn )
//		{
//			if ( /*	SndStream::SNDSTREAM_MPEG == pInStream->GetStreamType() && */
//					( pEncoder->GetCanWriteTagV1() || pEncoder->GetCanWriteTagV2() ) )
//			{
//				// copy ID3 Tag
//				CID3Tag::CopyTags( GetCurrentTask().GetFullFileName(),
//								   GetCurrentTask().GetOutFullFileName() ) ;
//			}
//		}
//
//		// De-initialize the encoder
//		pEncoder->DeInitEncoder();
//
//
//		if ( pDlg->m_bAbortThread )
//		{
//			CUString strFileToDelete = GetCurrentTask().GetOutFullFileName();
//            CUStringConvert strCnv; 
//            DeleteFile( strCnv.ToT( strFileToDelete ) );
//            LTRACE( _T( "Delete file \"%s\" due to abort" ), strCnv.ToT( strFileToDelete ) );
//		}
//
//		if ( NULL != pDlg->m_pRipInfoDB )
//		{
//			CUString strRipInfoDB;
//			CTimeSpan myEndTime = CTime::GetCurrentTime()-myTime;
//
//			if ( TRUE == pDlg->m_bAbortThread )
//			{
//				strLang = g_language.GetString( IDS_ENCODE_ABORT );
//				strRipInfoDB.Format( strLang, myTime.Format( _T( "%A, %B %d, %Y %H:%M:%S" ) ) );
//
//				// Add prolog info to RipFileInfo
//				pDlg->m_pRipInfoDB->SetAbortError( strRipInfoDB );
//			}
//			else
//			{
//				strLang = g_language.GetString( IDS_ENCODE_FINISHED_OK );
//
//				strRipInfoDB.Format( strLang, myEndTime.Format( _T( "%H:%M:%S" ) ) );
//
//				// Add prolog info to RipFileInfo
//                pDlg->m_pRipInfoDB->SetRipInfoFinshedOK( strRipInfoDB, pDlg->m_lCRC );
//			}
//		}
//	}
//	else
//	{
//		LTRACE( _T( "CCopyDialog::WavToMpeg, Calling DOS Encoder" ) );
//
//		pEncoder->InitEncoder( &GetCurrentTask() );
//
//		// Do the encoding
//		CUString strInDir( GetCurrentTask().GetInDir() );
//		CUString strOutDir( GetCurrentTask().GetOutDir() );
//		CUString strInFileName( GetCurrentTask().GetFullFileNameNoExt() );
//		CUString strOutFileName( GetCurrentTask().GetOutFullFileNameNoExt() );
//		CUString strNrmInFileName( strInFileName );
//
//
//		// Does this file needs normalization ?
//		if ( CTaskInfo::NORM_DEFAULT_VALUE != pDlg->GetCurrentTask().GetNormalizationFactor() )
//		{
//			LTRACE( _T( "Normalizing for external codec" ) );
//
//			pDlg->SetupControls( IS_NORMALIZING );
//
//			// Normalize the file, keep the nrm file 
//			NormWav( pDlg, strNrmInFileName , FALSE );
//			strNrmInFileName += _W( ".nrm" );
//
//			pDlg->SetupControls( IS_CONVERTING );
//		}
//
//		bReturn = pEncoder->DosEncode(	strNrmInFileName, 
//										GetCurrentTask().GetFileExt(), 
//										strOutFileName, 
//										strOutDir,
//										(int&)pDlg->m_nPercent,
//										(BOOL&)pDlg->m_bAbortThread );
//
//
//		// check if we have normalized the file, if so, delete the intermediate file
//		if ( CTaskInfo::NORM_DEFAULT_VALUE != pDlg->GetCurrentTask().GetNormalizationFactor() )
//		{
//			CUString strFileToDelete;
//
//			// delete the normalization file
//			strFileToDelete = strNrmInFileName + _W( "." ) + GetCurrentTask().GetFileExt();
//
//			LTRACE( _T( "Deleting normalized file \"%s\"" ), strFileToDelete );
//
//            CUStringConvert strCnv;
//			DeleteFile( strCnv.ToT( strFileToDelete ) ) ;
//		}
//
//		if ( pDlg->m_bAbortThread )
//		{
//			CUString strFileToDelete = GetCurrentTask().GetOutFullFileName();
//            CUStringConvert strCnv;
//			CDexDeleteFile( strFileToDelete ) ;
//			LTRACE( _T( "Delete file \"%s\" due to abort" ), strCnv.ToT( strFileToDelete ) );
//		}
//
//	}
//
//	EXIT_TRACE( _T( "CCopyDialog::WavToMpeg, return value: %d" ), bReturn );
//
//	return bReturn;
//}
//
//
//CDEX_ERR CCopyDialog::GetMaxWaveValue(	CCopyDialog*	pDlg )
//						 
//						 
//{
//	CDEX_ERR bReturn = CDEX_OK;
//
//	ENTRY_TRACE( _T( "CCopyDialog::GetMaxWaveValue" ) );
//
//	// init incoming parameters
//	pDlg->GetCurrentTask().SetPeakValue( 0 );
//	pDlg->m_nPercent = 0;
//
//	// create WAV object
//	CWAV inWav;
//
//	// Step 1: Open the input WAV file
//	if ( inWav.OpenForRead( GetCurrentTask().GetFullFileName() ) != CDEX_OK )
//	{
//		ASSERT(FALSE);
//		bReturn = CDEX_ERROR;
//	}
//	else
//	{
//		int nPeak = pDlg->GetCurrentTask().GetPeakValue();
//		while( FALSE == inWav.GetMaxWaveValue( 
//			(int&)pDlg->m_nPercent,
//			nPeak  )
//			)
//		{
//			::Sleep( 0 );
//		}
//
//		pDlg->GetCurrentTask().SetPeakValue( nPeak );
//
//		// Close audio file
//		inWav.CloseStream();
//	}
//
//
//	EXIT_TRACE( _T( "GetMaxWaveValue, peak value is %d, return value d" ), pDlg->GetCurrentTask().GetPeakValue(), bReturn );
//
//	return bReturn;
//}
//
//
//
//CDEX_ERR CCopyDialog::NormWav(	CCopyDialog*	pDlg,
//								CUString			strWavFileName,
//								BOOL			bReplace )
//{		
//	CDEX_ERR bReturn = CDEX_OK;
//
//	ENTRY_TRACE( _T( "CCopyDialog::NormWav( %s, %f" ), strWavFileName );
//
//	// Local parameters
//	CWAV		inWav;
//
//	pDlg->m_nPercent = 0;
//
//	// Step 1: Open the input WAV file
//	if ( 0 == inWav.StartNormalizeAudioFile( strWavFileName ) )
//	{
//		bReturn = CDEX_ERROR;
//	}
//	else
//	{
//		// Step 2: Loop through data and normalize chunk
//		while (	( inWav.NormalizeAudioFileChunk( pDlg->GetCurrentTask().GetNormalizationFactor(), (int&)pDlg->m_nPercent ) == FALSE ) && 
//				!pDlg->m_bAbortThread )
//		{
//			::Sleep(0);
//		}
//
//		// Step 3: Close the WAV file 
//		inWav.CloseNormalizeAudioFile(  strWavFileName, 
//										pDlg->m_bAbortThread,
//										bReplace );
//	}
//
//	EXIT_TRACE( _T( "CCopyDialog::NormWav, return value: %d" ), bReturn );
//
//	return bReturn;
//}
//
//
//
//CDEX_ERR CCopyDialog::AddToPlayList( CCopyDialog*	pDlg ) 
//{
//	CUString strLang;
//
//	ENTRY_TRACE( _T( "CCopyDialog::AddToPlayList" ) );
//
//	CUString strName( _W( "" ) );
//	CUString strDir( _W( "" ) );
//
//	CUString strBuild[NUMFILENAME_IDS];
//
//    CTagData& tagData( GetCurrentTask().GetTagData() );
//
//	// Build playlist filename
//	strBuild[0] = tagData.GetArtist();
//	strBuild[1] = tagData.GetAlbum();
//    strBuild[2].Format( _W( "%d" ), (LONG)tagData.GetTrackNumber() + tagData.GetTrackOffset() );
//    strBuild[3] = tagData.GetTitle();
//	strBuild[4].Format( _W( "%08x" ), tagData.GetCDBID() );
//	strBuild[5].Format( _W( "%08x" ), tagData.GetVOLID() );
//	strBuild[6].Format( _W( "%02d" ), (LONG)tagData.GetTrackNumber() + tagData.GetTrackOffset() );
//    strBuild[7].Format( _W( "%02d" ), tagData.GetTotalTracks() );
//	strBuild[8] = tagData.GetYear();
//	strBuild[9] = tagData.GetGenre();
//	strBuild[10] = GetCurrentTask().GetNonSplitArtist();
//
//	// Build the new file name
//	::BuildFileName(g_config.GetPlsFileFormat(),strBuild,strName,strDir);
//
//	CUString strPlsDir;
//	if ( strDir.Find( _W( ":\\" ) ) > 0 ) 
//	{
//		strPlsDir = strDir;
//	}
//	else
//	{
//		strPlsDir=g_config.GetMP3OutputDir()+strDir;
//	}
//
//	if ( g_config.GetM3UPlayList() || g_config.GetPLSPlayList() )
//	{
//
//		// Create playlist output directory if necessary
//		if ( CDEX_OK != DoesDirExist( strPlsDir, FALSE ) )
//		{
//			LTRACE( _T( "Write Id3 Tag And Play List::Error creating output dir %s" ), strPlsDir );
//			strLang = g_language.GetString( IDS_CANNOT_CREATE_OUTDIR );
//			CDexMessageBox( strLang + strPlsDir );
//		}
//		else
//		{
//
//			// Create Playlist object with proper filename
//			PlayList myList( strPlsDir+strName );
//
//			// Add this entry to the playlist
//			myList.AddEntry(GetCurrentTask().GetOutFullFileName() );
//		}
//	}
//
//	// some debugging information 
//	EXIT_TRACE( _T( "CCopyDialog::AddToPlayList" ) );
//	return CDEX_OK;
//}
//
//
//CDEX_ERR CCopyDialog::WriteId3Tag(	CCopyDialog*	pDlg,
//									BOOL			bCanWriteTagV1,
//									BOOL			bCanWriteTagV2 ) 
//{
//	ENTRY_TRACE( _T( "CCopyDialog::WriteId3Tag file to tag \"%s\"" ), GetCurrentTask().GetOutFullFileName() );
//
//
//	// Add ID3 Tag if necessary
//	CID3Tag myTag( GetCurrentTask().GetOutFullFileName() );
//
//    myTag.SetTagData( GetCurrentTask().GetTagData() );
//
//    // AF CHECK, can be removed, have to check all conditions
//	if ( GetCurrentTask().GetLengthInMs() )
//	{
//		CUString strTmp;
//		strTmp.Format( _W( "%d" ), GetCurrentTask().GetLengthInMs() );
//        myTag.GetTagDataReference().SetLengthInMs( strTmp );
//	}
//    // AF END
//
//	int nVersion = g_config.GetID3Version();
//
//	if ( ( nVersion > ID3_VERSION_NONE ) && (bCanWriteTagV1 || bCanWriteTagV2) )
//	{
//		// Get the desired ID3 tag version
//
//		// Fall back to ID3V1 if the file format can't handle ID3V2 tags
//		if ( ( bCanWriteTagV2 == FALSE )  && 
//			 ( nVersion >= ID3_VERSION_2 ) )
//		{
//			nVersion = ID3_VERSION_1;
//		}
//	
//		// Set the ID3 tag version
//		myTag.IsV1Tag( nVersion & 0x01 );
//		myTag.IsV2Tag( (nVersion & 0x02)?true:false );
//
//		// And save the tag
//		myTag.SaveTag();
//	}
//
//
//	EXIT_TRACE( _T( "CCopyDialog::WriteId3Tag" ) );
//	return CDEX_OK;
//}
//
//
//
//CDEX_ERR CCopyDialog::MP3ToRiffWav(	CCopyDialog*	pDlg,
//									DWORD			dwBitRate,
//									DWORD			dwSampleRate,
//									INT				nNumChannels	) 
//{
//	CDEX_ERR bReturn = CDEX_OK;
//
//	ENTRY_TRACE( _T( "MP3ToRiffWav" ) );
//
//	CWAV myWav;
//
//	int nLayer = 0;
//
//	if ( _T( '3' ) == ( GetCurrentTask().GetOutFileExt() ).GetAt(2)  )
//	{
//		nLayer = 3;
//	}
//	else if ( _T( '2' ) == ( GetCurrentTask().GetOutFileExt() ).GetAt(2)  )
//	{
//		nLayer=2;
//	}
//
//	// Convert MP3 file to RIFF WAV file format
//	if (g_config.GetRiffWav())
//	{
//		CUString strOrig = GetCurrentTask().GetOutFullFileName();
//
//		if ( CDEX_OK == myWav.ConvertToRiffWav( GetCurrentTask().GetOutFullFileNameNoExt(),
//												dwBitRate,
//												dwSampleRate,
//												nNumChannels,
//												nLayer ) )
//		{
//			//. change the file extention of the output file
//			GetCurrentTask().SetOutFileExt(  _T( "wav" ) );
//
//			CDexDeleteFile( strOrig );
//		}
//		else
//		{
//			bReturn = CDEX_ERROR;
//		}
//	}
//
//	EXIT_TRACE( _T( "MP3ToRiffWav, return value %d" ), bReturn );
//	return bReturn;
//}
//
//
//UINT CCopyDialog::RipFunc( PVOID pParams )
//{
//	CUString		strLang;
//	CDEX_ERR	bErr		= CDEX_OK;
//	BOOL		bNoToAll	= FALSE;
//
//	// tracing info
//	ENTRY_TRACE( _T( "CCopyDialog::RipFunc" ) );
//	
//	// get pointer to active CCopyDialog object
//	CCopyDialog* pDlg= (CCopyDialog*)pParams ;
//
//	// start with track zero
//	pDlg->m_wCurrentTrack = 0;
//
//	// thread has not been aborted yet
//	pDlg->m_bAbortThread = FALSE;
//
//	if ( g_config.GetUseStatusServer() )
//	{
//		g_statusReporter.AlbumStart( GetCurrentTask() );
//	}
//
//	// loop through all the tracks, as long as the thread has not been aborted
//	while( ( pDlg->m_wCurrentTrack < pDlg->GetTasks().GetNumTasks() ) && 
//			!pDlg->m_bAbortThread )
//	{
//		BOOL bOnTheFly		= FALSE;
//		BOOL bRipToTempFile = FALSE;
//
//		bErr = CDEX_OK;
//
//		pDlg->SetCurrentTask( GetTasks().GetTaskInfo( pDlg->m_wCurrentTrack ) );
//
//		// Set nominal normalization factor
//		pDlg->GetCurrentTask().SetNormalizationFactor( CTaskInfo::NORM_DEFAULT_VALUE );
//		pDlg->GetCurrentTask().SetPeakValue( 0 );
//
//
//		g_statusReporter.TrackStart( GetCurrentTask() );
//
//		LTRACE( _T( "CCopyDialog::RipFunc, Operating on track %d" ), pDlg->m_wCurrentTrack );
//
//		// Instantiate temporary encoder object to get its properties
//		auto_ptr<CEncoder> pEncoder( EncoderObjectFactory( pDlg->GetCurrentTask().GetEncoderType() ) );
//
//		pDlg->GetCurrentTask().SetOutFileExt( pEncoder->GetExtension() );
//
//
//		if (	pEncoder->GetChunkSupport() && 
//				pEncoder->GetOnTheFlyEncoding() && 
//				pDlg->GetCurrentTask().GetRip() &&
//				pDlg->GetCurrentTask().GetConvert() && 
//				!pDlg->GetCurrentTask().GetNormalize() )
//		{
//			LTRACE( _T( "On the fly is TRUE" ) );
//			bOnTheFly = TRUE;
//		}
//
//
//		if ( pDlg->GetCurrentTask().GetConvert() && !bOnTheFly && pDlg->GetCurrentTask().GetRip() )
//		{
//			bRipToTempFile = TRUE;
//		}
//
//		INT		nBitRate = pEncoder->GetBitrate();
//		INT		nChannels = pEncoder->GetNumChannels();
//		INT		nSampleRate = pEncoder->GetSampleRate();
//		INT		nBitsPerChannel = 16;
//
//		// retain the original file name, so we can delete it if required
//		CUString strInFileName    = pDlg->GetCurrentTask().GetFullFileNameNoExt();
//		CUString strInFileNameExt = pDlg->GetCurrentTask().GetFileExt();
//		CUString strOutDir		 = pDlg->GetCurrentTask().GetOutDir();
//
//
//		// Create Ouput directory
//		if ( CDEX_OK != DoesDirExist( strOutDir, FALSE ) )
//		{
//			LTRACE( _T( "RipFunc::Error creating output dir \"%s\"" ), strOutDir );
//			strLang = g_language.GetString( IDS_CANNOT_CREATE_OUTDIR );
//// BradeyH: Stall here			
//			CDexMessageBox( strLang + strOutDir );
//			bErr = CDEX_FILEOPEN_ERROR;
//		}
//
//		if ( ( CDEX_OK == bErr ) && bOnTheFly )
//		{
//			LTRACE( _T( "RipFunc::Ripping on the fly" ) );
//
//			pDlg->SetupControls( IS_READING );
//
//			bErr = RipToEncoder(	pDlg,
//									pDlg->GetCurrentTask().GetEncoderType(),
//									bRipToTempFile,
//									bNoToAll );
//
//			LTRACE( _T( "RipFunc::End of Ripping on the fly error is %d" ), bErr );
//		}
//		else
//		{
//			// Rip Track to WAV
//			if ( ( CDEX_OK == bErr ) && pDlg->GetCurrentTask().GetRip() && !pDlg->m_bAbortThread )
//			{
//				LTRACE( _T( "CCopyDialog::RipFunc, Ripping to WAV " ) );
//
//				// Set proper status
//				pDlg->SetupControls( IS_READING );
//
//				// Rip to WAV
//				if ( pDlg->GetCurrentTask().GetRip() && pDlg->GetCurrentTask().GetConvert() )
//				{
//					// Only a temp file, so setup input directory to temp path
//					pDlg->GetCurrentTask().SetInDir( g_config.GetTempDir() );
//				}
///*
//				else
//				{
//					// Check if file name does already exist
//					if ( TRUE == CheckNoFileOverwrite( pDlg, pDlg->GetCurrentTask().GetFullFileName(), TRUE, m_bYesToAll, bNoToAll  ) )
//						{
//							bErr = CDEX_FILEOPEN_ERROR;
//						}
//				}
//*/
//				if ( CDEX_OK == bErr )
//				{
//					bErr = RipToEncoder(	pDlg, 
//											ENCODER_FIXED_WAV,
//											bRipToTempFile,
//											bNoToAll );
//				}
//			}
//
//			if ( bErr != CDEX_OK  ) {
//				CUString strError( GetLastCDexErrorString() );
//				if ( !strError.IsEmpty() ) {
//					CDexMessageBox( strError );
//				}
//			}
//
//			// Check if WAV needs normalization
//			if ( bErr==CDEX_OK && pDlg->GetCurrentTask().GetNormalize() && !pDlg->m_bAbortThread )
//			{
//				if ( TRUE == pDlg->GetCurrentTask().GetConvert() )
//				{
//					// when the file is ripped, the peak value is alredy known!
//					if ( FALSE == pDlg->GetCurrentTask().GetRip() )
//					{
//						// No ripping, thus have to determine the peak value
//						pDlg->SetupControls( IS_DETPEAKVALUE );
//
//						LTRACE( _T( "RipFunc::Get Max Value" ) );
//
//						bErr = GetMaxWaveValue( pDlg );
//
//					}
//
//					CalculateNormalizationFactor( pDlg );
//				}
//				else
//				{
//					if ( 0 == pDlg->GetCurrentTask().GetPeakValue() )
//					{
//						bErr = GetMaxWaveValue( pDlg );
//					}
//
//					// Normalize the ripped file
//					CalculateNormalizationFactor( pDlg );
//
//					if ( CTaskInfo::NORM_DEFAULT_VALUE != pDlg->GetCurrentTask().GetNormalizationFactor()  && !pDlg->m_bAbortThread )
//					{
//
//						LTRACE( _T( "RipFunc::Normalizing Track %s" ), pDlg->GetCurrentTask().GetFullFileName() );
//
//						pDlg->SetupControls( IS_NORMALIZING );
//
//						bErr = NormWav( pDlg,
//										pDlg->GetCurrentTask().GetFullFileNameNoExt(),
//										TRUE );
//					}
//
//				}
//
//				// Do we need to add RIP Status information  ?
//				if ( pDlg->GetCurrentTask().GetRip() )
//				{
//					CUString strRipInfoDB;
//
//					// Create string with proper iformation
//					strRipInfoDB.Format( _W( "Normalizing Track with Factor %10.3f (peak value=%d)"), pDlg->GetCurrentTask().GetNormalizationFactor(), pDlg->GetCurrentTask().GetPeakValue() );
//
//					// Add start info to RipFileInfo
//					pDlg->m_pRipInfoDB->SetRipInfo( strRipInfoDB );
//				}
//
//			}
//
//			if ( ( bErr==CDEX_OK ) && pDlg->GetCurrentTask().GetRip() )
//			{
//				// Get the information regarding the CD-Ripping settings
//				GetCDRipInfo( nSampleRate, nChannels, nBitsPerChannel );
//			}
//
//			// Convert WAV to MP3
//			if ( ( bErr==CDEX_OK ) && pDlg->GetCurrentTask().GetConvert() && !pDlg->m_bAbortThread )
//			{
//				BOOL bWriteID3V2Tag = ( ( TRUE == pDlg->GetCurrentTask().GetRip() ) && ( g_config.GetID3Version() >= ID3_VERSION_2 ) ); 
//				LTRACE( _T( "CCopyDialog::RipFunc, Convert Track/file" ) );
//
//				pDlg->SetupControls( IS_CONVERTING );
//
//				bErr = WavToMpeg(	pDlg,
//									nSampleRate,
//									nChannels,
//									bWriteID3V2Tag,
//									bNoToAll );
//			}
//
//			// Delete the converted WAV file ?
//			if ( ( bErr == CDEX_OK ) && 
//				 pDlg->GetCurrentTask().GetDeleteWav() && 
//				 !pDlg->m_bAbortThread && 
//				 !bRipToTempFile )
//			{
//				LTRACE( _T( "CCopyDialog::RipFunc Deleting file \"%s\"" ), pDlg->GetCurrentTask().GetFullFileName() );
//				CDexDeleteFile( pDlg->GetCurrentTask().GetFullFileName() );
//			}
//		}
//
//		// Set state to IDLE
//		pDlg->SetupControls(IS_IDLE);
//
//
//		// Write ID3 Tag
//		if ( ( CDEX_OK == bErr ) && !pDlg->m_bAbortThread )
//		{
//			LTRACE( _T( "CCopyDialog::RipFunc, Write ID3 Tag" ) );
//
//			if ( bOnTheFly || ( pDlg->GetCurrentTask().GetConvert() && pDlg->GetCurrentTask().GetRip() ) )
//			{
//				bErr = WriteId3Tag(	pDlg,
//									pEncoder->GetCanWriteTagV1(),
//									pEncoder->GetCanWriteTagV2() );
//			}
//		}
//
//		// Convert to RiffWav?
//		if (	( CDEX_OK == bErr ) && 
//				pDlg->GetCurrentTask().GetRiffWav() && 
//				pDlg->GetCurrentTask().GetConvert() && 
//				!pDlg->m_bAbortThread && 
//				pEncoder->GetRiffWavSupport() )
//		{
//			LTRACE( _T( "RipFunc::Converting to a RIFF-WAV file" ) );
//
//			// Convert to a RIFF-WAV file
//			CUString strOutFileName = strOutDir + pDlg->GetCurrentTask().GetFileNameNoExt() ;
//
//			bErr = MP3ToRiffWav( pDlg, nBitRate, nSampleRate, nChannels );
//		}
//
//		// Add to play list 
//		if ( ( CDEX_OK == bErr ) && !pDlg->m_bAbortThread)
//		{
//			LTRACE( _T( "CCopyDialog::RipFunc::Write to play list" ) );
//
//			bErr= AddToPlayList( pDlg );
//
//		}
//
//		if ( bErr )
//		{
//			LTRACE( _T( "RipFunc:: Error occured, set bAbortThread" ) );
//
//			if ( CDEX_FILEOPEN_ERROR != bErr )
//			{
//				pDlg->m_bAbortThread = TRUE;
//			}
//		}
//
//		if ( pDlg->m_bAbortThread && ( CDEX_FILEOPEN_ERROR != bErr ) && ( TRUE == pDlg->GetCurrentTask().GetRip() ) )
//		{
//			// CDEX_FILEOPEN_ERROR indicates the output file already exists - so don't delete it! 
//			LTRACE( _T( "RipFunc::Due to abort, delete generated output file %s" ), pDlg->GetCurrentTask().GetFullFileName() );
//			CDexDeleteFile( pDlg->GetCurrentTask().GetFullFileName() );
//		}
//		else
//		{
//			// Delete or keep the intermediate WAV file
//			if ( bRipToTempFile )
//			{
//				if ( pDlg->m_bRetainWavFile )
//				{
//					CUString strSrcFile;
//					CUString strDstFile;
//
//					strDstFile = strOutDir + pDlg->GetCurrentTask().GetFileNameNoExt() + _W( ".wav" ) ;
//
//					strSrcFile = pDlg->GetCurrentTask().GetFullFileName();
//
//					FILE* fp= NULL;
//					
//					int nKeep = 0;
//
//					// check for existing filename, pick new filename if it does exist
//					while ( NULL != ( fp = CDexOpenFile( strDstFile, _W( "r" ) ) ) )
//					{
//						CUString strNewExt;
//						strNewExt.Format( _W( "_%d.wav" ), nKeep );
//						strDstFile = strDstFile.Left( strDstFile.GetLength() -4 - (nKeep > 0?2:0) ) + strNewExt;
//						nKeep++;
//						fclose( fp );
//
//					}
//
//					
//					LTRACE( _T( "CCopyDialog::RipFunc, Keep WAV file, rename \"%s\"->\"%s\"" ), strSrcFile, strDstFile );
//
//					CDexMoveFile( strSrcFile, strDstFile );
//				}
//				else
//				{
//					LTRACE( _T( "RipFunc Deleting file %s" ), pDlg->GetCurrentTask().GetFullFileName() );
//					CDexDeleteFile( pDlg->GetCurrentTask().GetFullFileName() );
//				}
//			}
//		}
//
//		// Update the track status of the main window
//		pDlg->m_pView->PostMessage( WM_UPDATETRACKSTATUS );
//
//		g_statusReporter.TrackFinish( bErr, pDlg->m_bCancelled, pDlg->m_nJitterErrors );
//
//		// Do the next track
//		pDlg->m_wCurrentTrack++;
//
//		delete pDlg->m_pRipInfoDB;
//		pDlg->m_pRipInfoDB = NULL;
//
//		LTRACE( _T( "CCopyDialog::RipFunc, Increasedto %d" ), pDlg->m_wCurrentTrack );
//
//	}
//
//	LTRACE( _T( "CCopyDialog::RipFunc, Send IDCANCEL message" ) );
//
//	if ( g_config.GetUseStatusServer() )
//	{
//		g_statusReporter.AlbumFinish( bErr, pDlg->m_bCancelled );
//	}
//
//	// Indicate thread status
//	pDlg->m_eThreadFinished.SetEvent();
//	pDlg->m_pThread = NULL;
//
//	// OK close the stuff
//	pDlg->PostMessage( WM_COMMAND, IDCANCEL, 0 );
//
//	EXIT_TRACE( _T( "CCopyDialog::RipFunc, IDCANCEL message has been send" ) );
//
//	return 0;
//}
//
//
//void CCopyDialog::OnTimer(UINT nIDEvent) 
//{
//	UpdateControls();
//}
//
//void CCopyDialog::UpdateControls() 
//{
//	CUString strLang;
//	CUString strJitterErrors;
//    CUStringConvert strCnv;
//
//	static int iCount=0;
//
//	m_mLockControls.Lock();
//
//	if ( m_bResetTimeTrack )
//	{
//		// Reset estimate timer
//		DWORD dwTrackLength = (GetCurrentTask().GetEndSector() - GetCurrentTask().GetStartSector()) * 1000 / 75;
//		m_TimeTrack.ReInit(dwTrackLength);
//		m_bResetTimeTrack = FALSE;
//	}
//
//	// Calculate new time estimates
//	m_TimeTrack.Calculate( m_nPercent );
//
//    int nTotalPercentage = 0;
//
//    nTotalPercentage = (int)( ( (double)( m_wCurrentTrack + 1 ) / (double)m_Tasks.GetNumTasks() ) * 100.0 );
//
//    nTotalPercentage = (int)( ( (double)m_wCurrentTrack  / (double)m_Tasks.GetNumTasks() ) * 100  + m_nPercent / m_Tasks.GetNumTasks() );
//
//    if ( ( m_nTotalTimeOnStatus ) == IS_READING && ( m_iStatus != IS_READING ) )
//    {
//            nTotalPercentage = (int)( ( (double)( m_wCurrentTrack + 1 ) / (double)m_Tasks.GetNumTasks() ) * 100.0 );
//    }
//
//	m_totalTimeTrack.Calculate( nTotalPercentage );
//
//	CUString strTime;
//	CUString strTotalTime;
//
//	// Get time estimate
//	strLang = g_language.GetString( IDS_ELAPSED_TIME );
//    strTime.Format( strLang, (LPCWSTR)m_TimeTrack.GetElapsedString(), (LPCWSTR)m_TimeTrack.GetRemainingString(), (LPCWSTR)m_TimeTrack.GetEstimateString());
//    strTotalTime.Format( strLang, (LPCWSTR)m_totalTimeTrack.GetElapsedString(), (LPCWSTR)m_totalTimeTrack.GetRemainingString(), (LPCWSTR)m_totalTimeTrack.GetEstimateString());
//
//    m_strInfo4.SetWindowText( strCnv.ToT( strTotalTime ) );
//	CUString strTitle;
//
//	// Default to CDex version
//	strTitle = _W("CDex Version ") + g_config.GetVersion();
//
//    m_JitterCtrl.SetPos( nTotalPercentage );
//
//    switch(m_iStatus)
//	{
//		case IS_READING:
//			strLang = g_language.GetString( IDS_EXTRACT_TRACK_TITLE );
//			strTitle.Format( strLang, GetCurrentTask().GetTagData().GetTrackNumber(), m_nPercent, m_wCurrentTrack + 1, GetTasks().GetNumTasks() );
//
//            m_TrackCtrl.SetPos( m_nPercent );
//
//			m_strInfo3=strTime;
//			{
//				double dDB=-96;
//				if ( GetCurrentTask().GetPeakValue() > 0 )
//				{
//					dDB=20*log10((double)GetCurrentTask().GetPeakValue()/32766.0);
//				}
//
//				int nPeakPercent=(int)((double)GetCurrentTask().GetPeakValue()/327.66);
//
//				strLang = g_language.GetString(IDS_PEAK_VALUE);
//                CUStringConvert strCnv;
//				m_strPeakValue.Format( strCnv.ToT( strLang ), dDB, nPeakPercent );
//			}
//
//			strLang = g_language.GetString( IDS_NUM_JITTER_ERRORS );
//			strJitterErrors.Format( strLang, m_nJitterErrors );
//
//		break;
//		case IS_CONVERTING:
//			strLang = g_language.GetString( IDS_CONVERTING_TRACK_TITLE );
//
//			strTitle.Format( strLang, (LPCWSTR)GetCurrentTask().GetTagData().GetTrackNumber(), m_nPercent, m_wCurrentTrack + 1, GetTasks().GetNumTasks() );
//
//			m_TrackCtrl.SetPos( m_nPercent );
//
//			m_strInfo3 = strTime;
//			m_strPeakValue = _T( "" );
//
//			strLang = g_language.GetString( IDS_NORMALIZATION_FACTOR );
//			m_strPeakValue.Format( strCnv.ToT( strLang ), GetCurrentTask().GetNormalizationFactor() );
//
//		break;
//		case IS_NORMALIZING:
//			strLang = g_language.GetString(IDS_NORMALIZING_TRACK_TITLE );
//			strTitle.Format( strLang, GetCurrentTask().GetTagData().GetTrackNumber(),m_nPercent, m_wCurrentTrack + 1, GetTasks().GetNumTasks() );
//			m_TrackCtrl.SetPos( m_nPercent );
//			m_strInfo3=strTime;
//			m_strPeakValue = _W( "" );
//		break;
//		case IS_DETPEAKVALUE:
//			strLang = g_language.GetString( IDS_DETERMINING_PEAK_VALUE_TITLE );
//			strTitle.Format( strLang,GetCurrentTask().GetTagData().GetTrackNumber(), m_nPercent, m_wCurrentTrack + 1,GetTasks().GetNumTasks() );
//			m_TrackCtrl.SetPos( m_nPercent );
//			m_strInfo3 = strTime;
//			{
//				double dDB=-96;
//				if ( GetCurrentTask().GetPeakValue() > 0 )
//				{
//					dDB=20*log10((double)GetCurrentTask().GetPeakValue()/32766.0);
//				}
//
//				int nPeakPercent=(int)((double)GetCurrentTask().GetPeakValue()/327.66);
//				strLang = g_language.GetString( IDS_REPORT_PEAK_VALUE );
//                
//                CUStringConvert strCnv;
//                m_strPeakValue.Format( strCnv.ToT( strLang ), dDB,nPeakPercent );
//			}
//			break;
//		case IS_IDLE:
//			m_TrackCtrl.SetPos( 0 );
//            // m_JitterCtrl.SetPos( 0 );
//
//			m_strInfo3 = _T( "" );
//			m_strPeakValue = _T( "" );
//		break;
//	}
//
//	// m_JitterCtrl.SetWindowText( strCnv.ToT( strJitterErrors ) );
//    m_strInfo5.SetWindowText( strCnv.ToT( strJitterErrors ) );
//
//	// Set title
//	AfxGetApp()->m_pMainWnd->SetWindowText( strCnv.ToT( strTitle ) );
//
//	UpdateData(FALSE);
//
//	m_mLockControls.Unlock();
//
//}
//
//void CCopyDialog::SetupControls(int nStatus) 
//{
//	CUString strLang;
//    CUStringConvert strCnv;
//
//	DWORD dwTrackLength = (GetCurrentTask().GetEndSector() - GetCurrentTask().GetStartSector()) * 1000 / 75;
//
//	m_mLockControls.Lock();
//	m_iStatus=nStatus;
//
//	m_nPercent = 0;
//
//	// Reset estimate timer
//	m_TimeTrack.ReInit(dwTrackLength);
//
//	strLang = g_language.GetString(IDS_CONVERTING_TRACK_NAME);
//    
//    m_strInfo2.Format(	strCnv.ToT( strLang ), (LPCWSTR)GetCurrentTask().GetFileNameNoExt() );
//
//	if ( m_strInfo2.GetLength() > 65 )
//	{
//		int nPos = m_strInfo2.Find( _T( ":" ) );
//
//		if ( nPos < 0 )
//			nPos = 0;
//
//		m_strInfo2 = m_strInfo2.Left( nPos + 2 ) +
//					_T( " ... " ) +
//					m_strInfo2.Right( 60 ) ;
//	}
//
//	switch (nStatus)
//	{
//		case IS_READING:
//			// Update progress bar text information
//			strLang = g_language.GetString( IDS_EXTRACTING_TRACK_NUMBER );
//            m_strInfo1.Format( strCnv.ToT( strLang ), GetCurrentTask().GetTagData().GetTrackNumber(), m_wCurrentTrack + 1, GetTasks().GetNumTasks() );
//		break;
//		case IS_CONVERTING:
//			// Update progress bar text information
//			strLang = g_language.GetString(IDS_CONVERTING_TRACK_NUMBER);
//			m_strInfo1.Format( strCnv.ToT( strLang ), m_wCurrentTrack+1, GetTasks().GetNumTasks() );
//		break;
//		case IS_NORMALIZING:
//			strLang = g_language.GetString( IDS_NORMALIZING_TRACK_NUMBER );
//			m_strInfo1.Format( strCnv.ToT( strLang ), GetCurrentTask().GetNormalizationFactor(), m_wCurrentTrack + 1, GetTasks().GetNumTasks() );
//		break;
//		case IS_DETPEAKVALUE:
//			strLang = g_language.GetString(IDS_GETPEAKVALUE_TRACK_NUMBER);
//			m_strInfo1.Format( strCnv.ToT( strLang ), m_wCurrentTrack+1,GetTasks().GetNumTasks() );
//		break;
//		case IS_IDLE:
//			strLang = g_language.GetString(IDS_PROCESSING_TRACK_NUMBER);
//			m_strInfo1.Format( strCnv.ToT( strLang ),m_wCurrentTrack+1, GetTasks().GetNumTasks() );
//		break;
//		default:
//			ASSERT(FALSE);
//	}
//
//	// Force update the controls
//	PostMessage(WM_TIMER,0,0);
//
//	m_mLockControls.Unlock();
//}
//
//BOOL CCopyDialog::OnInitDialog() 
//{
//	CUString strLang;
//
//
//	CDialog::OnInitDialog();
//	
//
//	SetCurrentTask( GetTasks().GetTaskInfo( 0 ) );
//
//	// translate dialog resources
//	g_language.InitDialogStrings( this, IDD );
//
//
//	m_pThread = NULL;
//
//	m_bYesToAll = FALSE;
//
//	m_bRetainWavFile = g_config.GetRetainWavFile();
//
//	// Start in IDLE mode
//	m_iStatus=IS_IDLE;
//
//	// Indicate thread status
//	m_eThreadFinished.ResetEvent();
//
//	// Fire of thread
//	m_pThread = AfxBeginThread( RipFunc, (void*)this, g_config.GetThreadPriority() );
//
//	// Set timer to upgate controls frequently
//	SetTimer( TIMERID, TIMERSLOT, NULL );
//
//    CUStringConvert strCnv;
//
//    m_nTotalTimeOnStatus = 0;
//	if ( GetCurrentTask().GetRip() )
//	{
//		strLang = g_language.GetString( IDS_EXTRACT_DIALOG );
//		SetWindowText( strCnv.ToT( strLang ) );
//        m_nTotalTimeOnStatus  = IS_READING;
//	}
//	else
//	{
//		strLang = g_language.GetString( IDS_FILE_CONVERSION );
//        SetWindowText( strCnv.ToT( strLang ) );
//        // AF TODO REMOVE
//		// GetDlgItem( IDC_JITTERCTRL )->ShowWindow( SW_HIDE );
//		// GetDlgItem( IDC_JITTERIND )->ShowWindow( SW_HIDE );
//		// GetDlgItem( IDC_JITTERIND1 )->ShowWindow( SW_HIDE );
//		// GetDlgItem( IDC_JITTERIND2 )->ShowWindow( SW_HIDE );
//	}
//
//	m_TrackCtrl.SetShowText( TRUE );
//	m_JitterCtrl.SetShowText( TRUE );
//
//    m_totalTimeTrack.ReInit( 0 );
//
//	return TRUE;
//}
//
//
//void CCopyDialog::OnCancel() 
//{
//	// If the thread's still around, someone hit the Cancel button
//	m_bCancelled = ( NULL != m_pThread );
//
//	ENTRY_TRACE( _T( "CCopyDialog::OnCancel, m_bCancelled = %d" ), m_bCancelled );
//
//	m_bAbortThread = TRUE;
//
//	// wait for thread to finish
//	WaitForSingleObject( m_eThreadFinished, INFINITE );
//
//	// just to be sure
//	CR_LockCD( FALSE );
//
//	// Eject when finished
//	if (	( g_config.GetEjectWhenFinished()  ) && 
//			( TRUE == GetCurrentTask().GetRip() ) &&
//			( FALSE == m_bCancelled ) )
//
//	{
//		LTRACE( _T( "CCopyDialog::OnCancel Eject the CD" ) );
//		m_pView->OnEject();
//	}
//
//	if ( FALSE == m_bCancelled )
//	{
//		MessageBeep( MB_OK );
//	}
//	else
//	{
//		MessageBeep( MB_ICONEXCLAMATION );
//	}
//
//	MessageBeep( 0xFFFFFFFF );
//
//	// Stop the timer
//	KillTimer( TIMERID );
//
//	// Set status to IDLE
//	m_iStatus = IS_IDLE;
//
//	// Update the controls, so title is set properly
//	UpdateControls();
//
//	// Make sure to update the latest track status
//	if ( m_pView )
//	{
//		m_pView->PostMessage( WM_UPDATETRACKSTATUS );
//	}
//
//	// Destroy myself
//	DestroyWindow();
//
//	EXIT_TRACE( _T( "CCopyDialog::OnCancel" ) );
//}
//
//
//void CCopyDialog::PostNcDestroy() 
//{
//	LTRACE( _T( "CCopyDialog::PostNcDestroy" ) );
//	if ( m_pView )
//	{
//		m_pView->PostMessage( WM_DIALOG_CLOSED, 0, (LPARAM)!m_bCancelled );
//	}
//
//	// delete this;	
//}
//
//
