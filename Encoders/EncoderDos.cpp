/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
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
#include "Encode.h"
#include "AudioFile.h"
#include "TaskInfo.h"
#include "EncoderDos.h"
#include "EncoderDosDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderDos" ) );


// +++++++++++++++++++++++++++++ CEncoderDos +++++++++++++++++++++++++++++

CEncoderDos::CEncoderDos():CEncoder()
{
	ENTRY_TRACE( _T( "CEncoderDos::CEncoderDos()" ) );

	m_bRiffWavSupport = TRUE;
	m_strEncoderPath = _T( "" );
	m_strExtension = _T( "mp3" );
	m_strEncoderID = _T( "External Encoder" );
	m_nEncoderID = ENCODER_EXT;
	m_strUser2 = _T( "%1 %2 -br %3" );
	m_bSendStdinWavHeader = FALSE;

	LoadSettings();
	SetChunkSupport ( GetOnTheFlyEncoding () ) ;

	SetSampleBufferSize ( 0x1000 ) ;
	m_hReadPipe = NULL;
	m_hWritePipe = NULL ;
	m_pChildProcess = NULL ;

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = TRUE;


	EXIT_TRACE( _T( "CEncoderDos::CEncoderDos()" ) );

}

CEncoderDos::~CEncoderDos()
{
	if ( m_pChildProcess ) 
	{
		delete m_pChildProcess ;
		m_pChildProcess = NULL ;
	}

	ForceCloseHandle ( m_hReadPipe ) ;
	ForceCloseHandle ( m_hWritePipe ) ;

}

CEncoderDlg* CEncoderDos::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg=new CEncoderDosDlg;
	}
	return m_pSettingsDlg;
}


CDEX_ERR CEncoderDos::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR bReturn = CDEX_OK;

	CEncoder::InitEncoder( pTask );

	if ( !GetChunkSupport () )
	{
		bReturn = CDEX_ERROR;
	}

	return bReturn;
}

CDEX_ERR CEncoderDos::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	BOOL	bReturn = CDEX_OK;
	BOOL	rc = 0;
	SECURITY_ATTRIBUTES sa;
	HANDLE	hToEncPipeRead = NULL ;
	HANDLE	hToEncPipeWrite = NULL ;
	HANDLE	hChildReadHandle  = NULL ;	
	CUString	strOutDir=strOutFileName;
	LPCSTR	lpszShortOutPath = NULL;
	BOOL	bHideDosBox = GetUserN1();
	CUString strTrackNumber;
	CUString strTotTracks;
	CUString	strLang;

	ENTRY_TRACE( _T( "CEncoderDos::OpenStream( \"%s\", %d,, %d" ),
			strOutFileName,
			dwSampleRate,
			nChannels );

	// strip of file name
	int nPos=strOutDir.ReverseFind('\\');

	if (nPos)
		strOutDir=strOutDir.Left(nPos+1);


	// Get short directory name for output path
    lpszShortOutPath = GetDosFileName(	strOutDir ); 

	// Create temp output file name
	m_strTempFileName=	CUString( lpszShortOutPath, CP_ACP) + 
						CUString( _W( "temp" ) ) +
						_W( "." ) + 
						GetExtension();

	// Save real output name
	m_strOutFileName=strOutFileName + _W( "." ) + GetExtension();


//	WORD wBitsPerSample = 16;
	CUString strTmp;
	CUString strExec;

	strExec = PrepareCommandLine();

	sa.nLength = sizeof ( sa ) ;
	sa.lpSecurityDescriptor = NULL ;
	sa.bInheritHandle = FALSE ;

	rc = CreatePipe ( &hToEncPipeRead, &hToEncPipeWrite, &sa, 0 ) ;

	if ( !rc )
	{
		strLang = g_language.GetString( IDS_ENCODER_ERROR_CANTCREATEPIPE );

		CDexMessageBox( strLang );

		bReturn = CDEX_ERROR;
	}

	if ( CDEX_OK == bReturn )
	{
		if ( DuplicateHandle ( GetCurrentProcess () , hToEncPipeRead , 
							   GetCurrentProcess () , &hChildReadHandle , 
							   0 , TRUE ,
							   DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) )
		{
			hToEncPipeRead = NULL ;
		}
		else
		{
			strLang = g_language.GetString( IDS_ENCODER_ERROR_CANTCREATEPIPE );

			CDexMessageBox( strLang );

			bReturn = CDEX_ERROR;
		}
	}

	if ( CDEX_OK == bReturn )
	{
		m_pChildProcess = new CChildProcess( TRUE, strExec, CalculateChildPriorityClass(), hChildReadHandle, NULL, NULL, bHideDosBox ) ;

		if ( NULL == m_pChildProcess )
		{
			bReturn = CDEX_ERROR;
		}
	}

	if ( CDEX_OK == bReturn )
	{

        CTagData& tagData( m_pTask->GetTagData() );

        strTrackNumber.Format( _W( "%02d" ), tagData.GetTrackNumber() + tagData.GetTrackOffset() );

		strTotTracks.Format( _W( "%02d" ), tagData.GetTotalTracks() );

		m_pChildProcess->SubstituteParameter( _T( "%1" ), CUString( "-", CP_UTF8 ) ) ;
		m_pChildProcess->SubstituteParameter( _T( "%2" ), m_strTempFileName);
		strTmp.Format( _W( "%d" ), m_nBitrate * 1000 );
		m_pChildProcess->SubstituteParameter( _T( "%3" ), strTmp ) ;
		strTmp.Format( _W( "%d" ),m_nBitrate);

		m_pChildProcess->SubstituteParameter( _T( "%4" ), strTmp ) ;
		m_pChildProcess->SubstituteParameter( _T( "%tn" ), strTrackNumber ) ;
		m_pChildProcess->SubstituteParameter( _T( "%tt" ), strTotTracks ) ;
		m_pChildProcess->SubstituteParameter( _T( "%a" ), tagData.GetArtist() ) ;
		m_pChildProcess->SubstituteParameter( _T( "%b" ), tagData.GetAlbum() ) ;
        m_pChildProcess->SubstituteParameter( _T( "%t" ), tagData.GetTitle() ) ;
		m_pChildProcess->SubstituteParameter( _T( "%g" ), tagData.GetGenre() ) ;
		m_pChildProcess->SubstituteParameter( _T( "%y" ), tagData.GetYear() ) ;

		if ( !m_pChildProcess->StartProcess() )
		{
			strLang = g_language.GetString( IDS_ENCODER_ERROR_CANT_LAUNCH_EXENCODER );
			CDexMessageBox( strLang );

			bReturn = CDEX_ERROR;
		}
		else
		{

			ForceCloseHandle ( hChildReadHandle ) ;

			m_hWritePipe = hToEncPipeWrite ;

			// Initialize the input stream
			bReturn = InitInputStream();
		}
	}



	if ( CDEX_OK == bReturn )
	{
		if ( m_bSendStdinWavHeader )
		{
			const int RIFFWAVHEADERSIZE = 44;

			DWORD* pdwValue = NULL;
			BYTE pbtRiffWavHeader[ RIFFWAVHEADERSIZE ] = { 0, };

			DWORD nBytesWritten = 0;

			// Write RIFF tag
			memcpy( &pbtRiffWavHeader[  0 ], "RIFF", 4 );

			pdwValue = (DWORD*)&pbtRiffWavHeader[  4 ];
			*pdwValue = 0x7FFFFFFF;

			// Write WAVE tag
			memcpy( &pbtRiffWavHeader[  8 ], "WAVE", 4 );

			memcpy( &pbtRiffWavHeader[ 12 ], "fmt ", 4 );
			pdwValue = (DWORD*)&pbtRiffWavHeader[ 16 ];
			*pdwValue = sizeof( WAVEFORMATEX ) - sizeof( WORD ); // don't write cbSize field

			WAVEFORMATEX*	pFmt = (WAVEFORMATEX*)&pbtRiffWavHeader[ 20 ];
			pFmt->cbSize = 0;
			pFmt->nSamplesPerSec = dwSampleRate;
			pFmt->wBitsPerSample = 16;
			pFmt->nChannels = nChannels; // input channels = output channels
			pFmt->wFormatTag = 1;
			pFmt->nBlockAlign = (pFmt->wBitsPerSample / 8) * pFmt->nChannels;
			pFmt->nAvgBytesPerSec = pFmt->nBlockAlign * pFmt->nSamplesPerSec;

			memcpy( &pbtRiffWavHeader[ RIFFWAVHEADERSIZE - 8 ], "data", 4 );

			pdwValue = (DWORD*)&pbtRiffWavHeader[  RIFFWAVHEADERSIZE - 4 ];
			*pdwValue = 0x7FFFFFFF - RIFFWAVHEADERSIZE + 8;

			if ( ! WriteFile (	m_hWritePipe,
								pbtRiffWavHeader,
								RIFFWAVHEADERSIZE,
								&nBytesWritten, NULL ) )
			{
				CUString strMsg = GetLastErrorString();

				strLang = g_language.GetString( IDS_ENCODER_ERROR_CANT_SENDDATATO_EXENCODER );
				CDexMessageBox( strLang );

				bReturn = CDEX_ERROR;
			}
		}
	}


	// Error cleanup
	if ( CDEX_OK != bReturn )
	{
		if ( m_pChildProcess ) 
		{
			delete m_pChildProcess ;
			m_pChildProcess = NULL ;
		}

		ForceCloseHandle ( hToEncPipeRead ) ;
		ForceCloseHandle ( hToEncPipeWrite ) ;
		ForceCloseHandle ( hChildReadHandle ) ;

		m_hReadPipe  = NULL;
		m_hWritePipe = NULL ;
	}


	EXIT_TRACE( _T( "CEncoderDos::OpenStream( ), return value %d" ), bReturn );

	return bReturn;

}

CDEX_ERR CEncoderDos::EncodeChunk( PSHORT pbsInSamples,DWORD dwNumSamples )
{
	CDEX_ERR	bReturn = CDEX_OK;
	CUString		strLang;
	DWORD		nBytesWritten = 0;


	LTRACE2( _T( "CEncoderDos::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	// Sanity check
	ASSERT( pbsInSamples );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

	if ( m_hWritePipe )
	{
		if ( ! WriteFile (	m_hWritePipe,
							m_psInputStream,
							dwNumSamples * sizeof( SHORT ),
							&nBytesWritten, NULL ) )
		{
			CUString strMsg = GetLastErrorString();

			strLang = g_language.GetString( IDS_ENCODER_ERROR_CANT_SENDDATATO_EXENCODER );
			CDexMessageBox( strLang );

			bReturn = CDEX_ERROR;
		}
		LTRACE2( _T( "CEncoderDos::EncodeChunk() Feed Encoder %d samples, returned %d bytes" ),
				dwNumSamples,
				nBytesWritten );

		FlushFileBuffers( m_hWritePipe );
	}

	LTRACE2( _T( "Leaving CEncoderDos::EncodeChunk(), return status %d" ),
					bReturn );

	return bReturn;
}

CDEX_ERR CEncoderDos::CloseStream()
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoderDos::CloseStream" ) );

	FlushFileBuffers( m_hWritePipe );
	ForceCloseHandle ( m_hReadPipe );
	ForceCloseHandle ( m_hWritePipe );

	if ( m_pChildProcess ) 
	{
		while ( ! m_pChildProcess->WaitForFinish ( 100L ) ) 
		{
		} ;
		delete m_pChildProcess;
		m_pChildProcess = NULL;
	}

	// Delete strDst
	if ( 0 == CDexDeleteFile(m_strOutFileName) )
	{
	}

	// Rename output file
	if ( CDexMoveFile(m_strTempFileName,m_strOutFileName)==0 )
	{
		CUString strMsg = GetLastErrorString();

		CDexMessageBox(	strMsg, MB_OK | MB_ICONINFORMATION );
	}

	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CEncoderDos::CloseStream, return value %d" ), bReturn );

	return bReturn;
}

CDEX_ERR CEncoderDos::DeInitEncoder()
{
	(void)CEncoder::DeInitEncoder();

	if ( GetChunkSupport() )
		return CDEX_OK;
	else
		return CDEX_ERROR;

}

CDEX_ERR CEncoderDos::DosEncode(	const CUString& strInFileName,
									const CUString& strInFileExt, 
									const CUString& strOutFileName,
									const CUString& strOutDir,
									INT&  nPercent,
									BOOL& bAbort)
{

	CUString	strLang;
	CUString strTmp;
	CUString    strExec;
	TCHAR	lpszShortPath[ MAX_PATH + 1 ] = {'\0'};
	TCHAR	lpszShortOutPath[ MAX_PATH + 1 ] = {'\0'};
	BOOL	bHideDosWindow = GetUserN1();
	WORD	wCurrentTrack = 0;
	CUString strTrackNumber;
	CUString strTotTracks;

	ENTRY_TRACE( _T( "CEncoderDos::DosEncode" ) );
	LTRACE( _T( "Input File Name is \"%s.%s\"" ), strInFileName, strInFileExt );
	LTRACE( _T( "Output File Name is \"%s.%s\"" ), strOutFileName, m_strExtension );

	strExec = PrepareCommandLine();

    CUStringConvert strCnv;

    LTRACE( _T( "Executing: %s" ), strCnv.ToT( strExec ) );

	m_pChildProcess = new CChildProcess( TRUE, strExec, CalculateChildPriorityClass(), NULL, NULL, NULL, bHideDosWindow ) ;

	if ( !m_pChildProcess )
	{
		return FALSE ;
	}

	CUString strInputFile( strInFileName + CUString( _W( "." ) ) + strInFileExt );

    // Obtain short file name
    GetShortPathName(	strCnv.ToT( strInputFile ),
						lpszShortPath,
						MAX_PATH ); 

	// Obtain short file name for output path
	GetShortPathName(	strCnv.ToT( strOutDir ),
						lpszShortOutPath,
						MAX_PATH ); 

	// delete temp file
	CUString strTmpOutFile( CUString( lpszShortOutPath )  + _W( "temp." ) + m_strExtension );

	if ( 0 == CDexDeleteFile( strTmpOutFile ) )
	{
	}


    CTagData& tagData( m_pTask->GetTagData() );

    strTrackNumber.Format( _W( "%02d" ), tagData.GetTrackNumber() + tagData.GetTrackOffset() );

	strTotTracks.Format( _W( "%02d" ), tagData.GetTotalTracks() );

	m_pChildProcess->SubstituteParameter( _T( "%1" ), CUString( lpszShortPath ) ) ;
	m_pChildProcess->SubstituteParameter( _T( "%2" ), strTmpOutFile ) ;
	strTmp.Format( _W( "%d" ), m_nBitrate * 1000 );
	m_pChildProcess->SubstituteParameter( _T( "%3" ), strTmp ) ;
	strTmp.Format( _W( "%d" ), m_nBitrate );

	m_pChildProcess->SubstituteParameter( _T( "%4" ), strTmp ) ;
	m_pChildProcess->SubstituteParameter( _T( "%tn" ), strTrackNumber ) ;
	m_pChildProcess->SubstituteParameter( _T( "%tt" ), strTotTracks ) ;
	m_pChildProcess->SubstituteParameter( _T( "%a" ), tagData.GetArtist() ) ;
	m_pChildProcess->SubstituteParameter( _T( "%b" ), tagData.GetAlbum() ) ;
    m_pChildProcess->SubstituteParameter( _T( "%t" ), tagData.GetTitle() ) ;
	m_pChildProcess->SubstituteParameter( _T( "%g" ), tagData.GetGenre() ) ;
	m_pChildProcess->SubstituteParameter( _T( "%y" ), tagData.GetYear() ) ;

	if ( !m_pChildProcess->StartProcess() )
	{
		strLang = g_language.GetString( IDS_ENCODER_ERROR_CANT_LAUNCH_EXENCODER );

		CDexMessageBox( strLang );

		return FALSE;
	}
	while ( !m_pChildProcess->WaitForFinish(1000L) ) ;
	{
	}

	delete m_pChildProcess ;
	m_pChildProcess = NULL;

	CUString strDst( strOutFileName + CUString( _W( "." ) ) + m_strExtension );

	// Delete strDst
	if ( 0 == CDexDeleteFile( strDst ) )
	{
	}

	LTRACE( _T( "Renaming file from \"%s\" to \"%s\"" ), strTmpOutFile, strDst );

	// Rename output file
	if ( 0 == CDexMoveFile( strTmpOutFile, strDst ) )
	{
		CUString strMsg = GetLastErrorString();

//		CDexMessageBox(	strMsg, MB_OK | MB_ICONINFORMATION );
	}

	EXIT_TRACE( _T( "CEncoderDos::DosEncode" ) );

	return CDEX_OK;
}



// +++++++++++++++++++++++++++++ CAacEncoder +++++++++++++++++++++++++++++


CAacEncoder::CAacEncoder():CEncoderDos()
{
	m_strEncoderPath = _T( "" );
	m_strEncoderID = _T( "Astrid/Quartex AAC encoder" );
	m_nEncoderID = ENCODER_AAC;
	m_strExtension = _T( "aac" );
	m_nBitrate = 128;
	m_strUser2 = _T( "%1 %2 %4" );

	LoadSettings();
	m_bRiffWavSupport = FALSE;
}

CAacEncoder::~CAacEncoder()
{
}


CEncoderDlg* CAacEncoder::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg=new CEncoderDosDlg;
	}
	return m_pSettingsDlg;
}




CDEX_ERR CAacEncoder::DosEncode(	const CUString& strInFileName,
									const CUString& strInFileExt, 
									const CUString& strOutFileName,
									const CUString& strOutDir,
									INT&  nPercent,
									BOOL& bAbort)
{
	// Call the encoder
	return CEncoderDos::DosEncode( strInFileName, strInFileExt, strOutFileName, strOutDir, nPercent, bAbort );
}

CAacPsyEncoder::CAacPsyEncoder():CAacEncoder()
{
	m_strEncoderPath = _T( "" );
	m_strEncoderID = _T( "Psytel AAC encoder" );
	m_nEncoderID = ENCODER_AACPSY;
	m_strExtension = _T( "aac" );
	m_nBitrate = 128;
	m_strUser2 = _T( "-br %4 -if %1 -of %2" );

	LoadSettings();
	m_bRiffWavSupport = FALSE;
}

CAacPsyEncoder::~CAacPsyEncoder()
{
}



CUString CEncoderDos::PrepareCommandLine()
{

	CUString retValue ;

	// Interpred the input command option string
	if (m_strEncoderPath.IsEmpty())
	{
		CUString strParam( GetUser2() );
		m_strEncoderPath.Format( _W( "%s %s" ), (LPCWSTR)GetUser1(), (LPCWSTR)strParam );
	}

	retValue = m_strEncoderPath ;

	return ( retValue ) ;
}

DWORD CEncoderDos::CalculateChildPriorityClass()
{
	switch ( g_config.GetThreadPriority() )
	{
	case THREAD_PRIORITY_IDLE:
	case THREAD_PRIORITY_LOWEST:
	case THREAD_PRIORITY_BELOW_NORMAL:
		return IDLE_PRIORITY_CLASS;
	case THREAD_PRIORITY_NORMAL:
		return NORMAL_PRIORITY_CLASS;
	case THREAD_PRIORITY_ABOVE_NORMAL:
	case THREAD_PRIORITY_HIGHEST:
		return HIGH_PRIORITY_CLASS;
	case THREAD_PRIORITY_TIME_CRITICAL:
		return REALTIME_PRIORITY_CLASS;
	default:
		return NORMAL_PRIORITY_CLASS;
	}
}



void CEncoderDos::SaveSettings()
{
	CUString strKey = GetIniKey();

	ENTRY_TRACE( _T( "CEncoderDos::SaveSettings()" ) );

	// call parent method
	CEncoder::SaveSettings();

	g_config.GetIni().SetValue( strKey, _T( "bSendStdinWavHeader" ), m_bSendStdinWavHeader );

	EXIT_TRACE( _T( "CEncoderDos::SaveSettings()" ) );
}


void CEncoderDos::LoadSettings()
{
	CUString strKey = GetIniKey();

	ENTRY_TRACE( _T( "CEncoderDos::LoadSettings()" ) );

	// call parent method
	CEncoder::LoadSettings();

	m_bSendStdinWavHeader = g_config.GetIni().GetValue( strKey, _T( "bSendStdinWavHeader" ), m_bSendStdinWavHeader );

	EXIT_TRACE( _T( "CEncoderDos::LoadSettings()" ) );
}

