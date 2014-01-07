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
#include "Config.h"
#include "Ini.h"
#include "FileVersion.h"
#include "EncoderObjectFactory.h"
#include <mmsystem.h>
#include <direct.h>
#include <math.h>
#include <limits.h>
#include "DriveOffset.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define NUMDEFAULTSITES 4

// Define a global instance of CConfig
CConfig			g_config;
CRemoteSites	g_RemoteSites;
CGenreTable		g_GenreTable;

INITTRACE( _T( "Config" ) );

struct PasswordStruct
{
	int		nUserName;
	char    lpszUserName[80];
	int		nPassword;
	char    lpszPassword[80];
	int		nUserKey;
	int		nPasswordKey;
};


// Good old Create dir with recursion
int MyCreateDir(CUString strDir)
{
	CUString strDirToCreate;

	if (strDir[strDir.GetLength()-1] == _T( '\\' ) )
		strDir=strDir.Left(strDir.GetLength()-1);

	if ( _wchdir( strDir )!=0)
	{
		int nPos=strDir.ReverseFind( _T( '\\' ) );
		if (nPos>=0)
		{
			// Are we at the root level yet
			if (strDir.ReverseFind( _W( ':' ) ) != nPos - 1 )
			{
				strDirToCreate = strDir.Mid( nPos + 1, strDir.GetLength() );

				// Recursion
				MyCreateDir( strDir.Left( nPos ) );
			}
			else
			{
				strDirToCreate=strDir;
			}
		}
	}

	strDirToCreate.TrimLeft();
	strDirToCreate.TrimRight();

	// Create recursivly the directories

    CUStringConvert strCnv; 

	if ( 0 == _tmkdir( strCnv.ToT( strDirToCreate ) ) )
	{
		if ( 0 == _tchdir( strCnv.ToT( strDirToCreate ) ) )
		{
			return 0;
		}
	}
	return -1;
}

CADebug::CADebug()
{
}

CADebug::CADebug(CUString& strFileName,BOOL bDelete)
{
	Open( strFileName, bDelete );
}

CADebug::~CADebug()
{
	Close();
}


void CADebug::Open( CUString& strFileName, BOOL bDelete )
{
	// Open output file
	m_strFileName = strFileName;

	// Have to delete the OldFile first?
	if ( bDelete )
	{
        CUStringConvert strCnv;
		DeleteFile( strCnv.ToT( m_strFileName ) );
	}
}

void CADebug::Close()
{
	if ( m_pFile )
	{
		fclose( m_pFile );
	}
	m_pFile = NULL;
}

void CADebug::DumpBinaray(LPBYTE pbtBuffer,int nSize)
{
	int nIdx=0;
	int i;

    m_pFile = CDexOpenFile( m_strFileName, _W( "a+" ) );

	if (m_pFile==NULL)
	{
		CDexMessageBox( _W( "Can't open output file" ) );
		return;
	}

	while (nIdx<nSize)
	{
		int nToDo=min(nSize-nIdx,16);

		for (i=0;i<nToDo;i++,nIdx++)
		{
			fprintf(m_pFile,"%02x ",pbtBuffer[nIdx]);
		}

		fprintf(m_pFile,"\n");
	}

	if (m_pFile)
		fclose(m_pFile);
}


void CADebug::printf(const char* pzFormat, ...)
{
    CUStringConvert strCnv;
    m_pFile = CDexOpenFile( m_strFileName, _W( "a+" ) );

	if (m_pFile==NULL)
	{
		CDexMessageBox( _W( "Can't open output file" ) );
		return;
	}

	BOOL bLogFile=TRUE;
    char szBuffer[1024];
    
	va_list ap;
	va_start(ap, pzFormat);
	
	_vsnprintf(szBuffer, 1000, pzFormat, ap);
    
	// Dump it to the file
	fprintf(m_pFile,"%s",szBuffer);

	va_end(ap);

	if (m_pFile)
		fclose(m_pFile);

}




void FAR PASCAL EXPORT ErrorCallBackFunc(char* lpszFile,int nLine,char* lpszError)
{
	CTime myTime=CTime::GetCurrentTime();

	FILE* fpError=NULL;


	// Open error log file
    CUStringConvert strCnv;
	
    fpError = CDexOpenFile( g_config.GetAppPath() + _W( "\\CDexErrorLog.txt" ), _W( "a" ) );

	if (fpError==NULL)
	{
		CDexMessageBox( _W( "Can't open CDexErrorLog.txt file" ) );
		return;
	}
	
    CUString strFileName( lpszFile, CP_UTF8 );
	int nPos=strFileName.ReverseFind( _W( '\\' ) );
	if (nPos)
		strFileName=strFileName.Right(strFileName.GetLength()-nPos-1);

	CUString strLog,strTmp;
	strTmp.Format( _W( "Error occured in CDex version %s" ), (LPCWSTR)g_config.GetVersion());
    strLog=strTmp;
	strTmp.Format( _W( "date %04d:%02d:%02d " ),myTime.GetYear(),myTime.GetMonth(),myTime.GetDay());
    strLog+=strTmp;
	strTmp.Format( _W( "time %02d:%02d:%02d " ),myTime.GetHour(),myTime.GetMinute(),myTime.GetSecond());
    strLog+=strTmp;
	strTmp.Format( _W( "Error:%s (file:%s line %05d)" ),lpszError,strFileName,nLine);
    strLog+=strTmp;


    fprintf( fpError, "%s\n" , strCnv.ToUTF8( strLog ) );
	fclose( fpError );
}

CConfig::CConfig()
{
	ENTRY_TRACE( _T( "CConfig::CConfig()" ) );

	// Get CDex version
	CFileVersion myVersion;
	TCHAR lpszModuleFileName[MAX_PATH];

	GetModuleFileName(	NULL, 
						lpszModuleFileName,
						sizeof( lpszModuleFileName ) );

	myVersion.Open( lpszModuleFileName );
	m_strVersion=myVersion.GetProductVersion();

	m_strMP3OutputDir	= _T( "" );
	m_strCnvOutputDir	= _T( "" );
	m_strRecOutputDir	= _T( "" );
	m_strWAVInputDir	= _T( "" );
	m_strMP3InputDir	= _T( "" );
	m_strPlayDir		= _T( "" );
	m_bNormTrack		= FALSE;
	m_bRiffWav			= FALSE;
	m_bRetainWavFile	= FALSE;
	m_bWinEncoder		= TRUE;
	m_bCDDBLongDirNames	= TRUE;

	m_bPLSPlayList		= FALSE;
	m_bM3UPlayList		= FALSE;

	m_bSaveToLocCDDB	= TRUE;
	m_bSaveToCDPlayer	= FALSE;

	m_nThreadPriority	= THREAD_PRIORITY_NORMAL;

	m_strCDDBPath		= _T( "" );
	m_strWinampDBPath	= _T( "" );

	m_nCDDBProxyPort	= 80;
	m_nCDDBTimeOut		= 20;
	m_nCDDBUseProxy		= FALSE;
	m_nCDDBUseAuthentication= FALSE;
	m_bCDDBAutoConnect		= FALSE;
	
	m_nFNBufferSize		= 64000;

	m_nPCopyEncType		= 0;
	m_bFileOpenRecursive= FALSE;

#ifdef _DEBUG
	m_strEmailAddress	= _T( "afaber@hccnet.nl" );
#else
	m_strEmailAddress	= _T( "" );
#endif
	m_strProxyAddress	= _T( "" );

	m_strFileFormat=CUString( _T( "%1\\%2\\%7-%4" ) );

	m_strPlsFileFormat=CUString( _T( "%1\\%2\\playlist" ) );

	m_nLocalCDDBType	= WINDOWSTYPECDDB;

	m_strAppPath		= GetCDexAppPath();

	m_bDumpToc = FALSE;
	m_bDelWavAfterConv = FALSE;


	m_nLowNormLevel		= 90;
	m_nHighNormLevel	= 98;
	m_nLNormFactor		= 90;
	m_nHNormFactor		= 98;

	m_nEncoderType		= 0;


	m_strID3Comment		= _W( "" );
	m_strID3EncodedBy	= _W( "" );
	m_strCDDBHSubmitAddr = _W( "freedb.freedb.org" );
	m_strCDDBESubmitAddr = _W( "freedb-submit@freedb.org" );
	m_strCDDBESubmitServer= _W( "" );

	m_nCDDBSubmitVia = 1;
	m_nID3Version = ID3_VERSION_2;
	m_strRILC = _T( "5F 5F 5F 5F 5F 5F 5F 5F 5F 20" );

	m_cSplitTrackChar = _T( '/' );
	m_bSplitTrackName = FALSE;
	m_bAutoRip = FALSE;
	m_bEjectWhenFinished = FALSE;
	m_bAutoShutDown = FALSE;
	m_bKeepDirLayout = TRUE;

	m_bSelectAllCDTracks = TRUE;
	m_nOverwriteExisting = 0;
	m_nRecordingDevice = 0;
	m_nID3V2TrackNumber = ID3TRKNRTTYPE_T;

	m_bCDDBWriteAsDosFile = FALSE;

	m_bCDPlayDigital = FALSE;

	m_bRecordAddSeqNr = TRUE;
	m_bRecordEncoderType = ENCODER_FIXED_WAV;

	TCHAR lpszTemp[ MAX_PATH + 1] = { _T( '\0' ),};

	GetTempPath( sizeof( lpszTemp ), lpszTemp );

	m_strTempDir = CUString( lpszTemp );

	m_strIniFileName = _W( "CDex.ini" );

	m_strProfileName = _W( "" );

	m_strLanguage = _W( "english" );

	m_bUseStatusServer = FALSE;
	m_strStatusServer = _T("localhost" );
	m_nStatusServerPort = 4242;


	// setup the ini filename
	g_config.GetIni().SetIniFileName( g_config.GetIniFileName() );


	EXIT_TRACE( _T( "CConfig::CConfig()" ) );

}

void CConfig::Save()
{
	ENTRY_TRACE( _T( "CConfig::Save()" ) );

#ifdef UNICODE
    g_config.GetIni().SetBom();
#endif
	// Save MP3 Parameters
	g_config.GetIni().SetValue( _T( "General" ), _T( "MP3OutputDir" ), m_strMP3OutputDir );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CnvOutputDir" ), m_strCnvOutputDir );
	g_config.GetIni().SetValue( _T( "General" ), _T( "RecOutputDir" ), m_strRecOutputDir );
	g_config.GetIni().SetValue( _T( "General" ), _T( "bNormTrack" ), m_bNormTrack );
	g_config.GetIni().SetValue( _T( "General" ), _T( "OutputRiffWavaFile" ), m_bRiffWav );
	g_config.GetIni().SetValue( _T( "General" ), _T( "RetainWavFile" ), m_bRetainWavFile );
	g_config.GetIni().SetValue( _T( "General" ), _T( "WinEncoder" ), m_bWinEncoder );
	g_config.GetIni().SetValue( _T( "General" ), _T( "FNBufferSize" ), m_nFNBufferSize );
	g_config.GetIni().SetValue( _T( "General" ), _T( "MP3InputDir" ), m_strMP3InputDir );
	g_config.GetIni().SetValue( _T( "General" ), _T( "PlayDir" ), m_strPlayDir );

	// WAV settings
	g_config.GetIni().SetValue( _T( "General" ), _T( "WAVInputDir" ), m_strWAVInputDir );
	g_config.GetIni().SetValue( _T( "General" ), _T( "DeleteWAVAfterConversion" ), m_bDelWavAfterConv );

	// Save CD Device parameters
	g_config.GetIni().SetValue( _T( "General" ), _T( "M3UPlayList" ), m_bM3UPlayList );
	g_config.GetIni().SetValue( _T( "General" ), _T( "PLSPlayList" ), m_bPLSPlayList );


	// CDDB stuff
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDDBPath" ), m_strCDDBPath );
	g_config.GetIni().SetValue( _T( "General" ), _T( "WinampDBPath" ), m_strWinampDBPath );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDDDEmailAddress" ), m_strEmailAddress );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDDDProxyAddress" ), m_strProxyAddress );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDBBUseProxy" ), m_nCDDBUseProxy );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDBBUseAuthentication" ), m_nCDDBUseAuthentication );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDBBProxyPort" ), m_nCDDBProxyPort );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDBBTimeOut" ), m_nCDDBTimeOut );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDBBLongDirNames" ), m_bCDDBLongDirNames );
	g_config.GetIni().SetValue( _T( "General" ), _T( "CDBBAutoConnect" ), m_bCDDBAutoConnect );

	g_config.GetIni().SetValue( _T( "General" ), _T( "AutoRip" ), m_bAutoRip );

	g_config.GetIni().SetValue( _T( "General" ), _T( "SplitTrackName" ), m_bSplitTrackName );
	g_config.GetIni().SetValue( _T( "General" ), _T( "SplitTrackChar" ), (int)m_cSplitTrackChar );

	g_config.GetIni().SetValue( _T( "General" ), _T( "EjectWhenFinished" ), m_bEjectWhenFinished );
	g_config.GetIni().SetValue( _T( "General" ), _T( "SelectAllCDTracks" ), m_bSelectAllCDTracks );

	g_config.GetIni().SetValue( _T( "General" ), _T( "AutoShutDown" ), m_bAutoShutDown );
	g_config.GetIni().SetValue( _T( "General" ), _T( "KeepDirLayout" ), m_bKeepDirLayout );

	g_config.GetIni().SetValue( _T( "General" ), _T( "CDDBWriteAsDosFile" ), m_bCDDBWriteAsDosFile );

	g_config.GetIni().SetValue( _T( "General" ), _T( "CDPlayDigital" ), m_bCDPlayDigital );

	g_config.GetIni().SetValue( _T( "General" ), _T( "RecordAddSeqNr" ), m_bRecordAddSeqNr );

	g_config.GetIni().SetValue( _T( "General" ), _T( "RecordEncoderType" ), m_bRecordEncoderType );

	// temp directory
	g_config.GetIni().SetValue( _T( "General" ), _T( "TempDir" ), m_strTempDir );


	// Write Proxy stuff if authentication is enabled
	if ( m_nCDDBUseAuthentication )
	{
		int i;
		int nVersion=001;
		int nKey;
		int nLength;
		int nSkip;
		int nIndex=0;

		// Write version number
		g_config.GetIni().SetValue( _T( "ProxyAuthentication" ), _T( "Version" ), nVersion );

		// Write User Name
		g_config.GetIni().SetValue( _T( "ProxyAuthentication" ), _T( "UserName" ), m_strCDDBProxyUser );

		// Create buffer string
		INT pBuffer[ 255 ];

		// Initialize pseudo random generator
		srand( GetTickCount() );

		// Generate encryption key
//#define DEBUG_PWD
#ifdef DEBUG_PWD
		nKey = 5367;
		m_strCDDBProxyPassword = _T( "testpwd" );
#else
		nKey=rand();
#endif

		// Store key in buffer zero
		pBuffer[ nIndex++ ] =nKey;

		// Get the length of Proxy User name
		nLength=m_strCDDBProxyPassword.GetLength();

		// Store length in buffer one
		pBuffer[nIndex++]=nLength ^ (int)sqrt((double)abs(nKey)+100);

		int nSum=0;

		// determine a skip value between 3..10 bo
		nSkip=(nLength + nKey)&7 + 3;

		// Generate nSkip keys and keep track of sum
		for (i=0;i<nSkip;i++,nIndex++)
		{
			// Re-initialize pseudo random generator
			srand(GetTickCount()+pBuffer[i]);

			// Do a rand in order to generate a better key
			rand();

#ifdef DEBUG_PWD
			pBuffer[nIndex]=500;
#else
			// Generate random number, store in buffer
			pBuffer[nIndex]=rand();
#endif
			// Add generated number to sum
			nSum+=pBuffer[nIndex];
		}

		// Write encrypted password into buffer
		for (i=0;i<nLength;i++,nIndex++)
		{
			// Store nLength in Byte one
			pBuffer[nIndex]= (int)(m_strCDDBProxyPassword.GetAt(i)) ^ (nSum^ (int)sqrt((double)abs(pBuffer[nIndex-1]+pBuffer[nIndex-2])+3454));
		}

		// Write length of buffer
		g_config.GetIni().SetValue( _T( "ProxyAuthentication" ),
									_T( "PasswordLength" ),
									nIndex );

		// Write buffer
		for (i=0;i<nIndex;i++)
		{
			CUString strKey;
			strKey.Format( _W( "Password%03d" ), i );
			g_config.GetIni().SetValue( _T( "ProxyAuthentication" ),
										strKey,
										pBuffer[i] );
		}
	}


	g_config.GetIni().SetValue( _W("General" ), _W( "FileFormatString" ),m_strFileFormat);
	g_config.GetIni().SetValue( _W("General" ), _W( "PlsFileFormatString" ),m_strPlsFileFormat);

	g_config.GetIni().SetValue( _W("General" ), _W( "LocalCDDBType" ),m_nLocalCDDBType);
	g_config.GetIni().SetValue( _W("General" ), _W( "SaveToCDPlayer" ),m_bSaveToCDPlayer);
	g_config.GetIni().SetValue( _W("General" ), _W( "SaveToLocalCDDB" ),m_bSaveToLocCDDB);
	g_config.GetIni().SetValue( _W("General" ), _W( "PCopyEncType" ),m_nPCopyEncType);

	g_config.GetIni().SetValue( _W("General" ), _W( "NormLowLevel" ),m_nLowNormLevel);
	g_config.GetIni().SetValue( _W("General" ), _W( "NormHighLevel" ),m_nHighNormLevel);
	g_config.GetIni().SetValue( _W("General" ), _W( "LNormFactor" ),m_nLNormFactor);
	g_config.GetIni().SetValue( _W("General" ), _W( "HNormFactor" ),m_nHNormFactor);

	g_config.GetIni().SetValue( _W("General" ), _W( "EncoderType" ),m_nEncoderType);
	g_config.GetIni().SetValue( _W("General" ), _W( "ExtEncPath" ),m_strExtEncPath);
	g_config.GetIni().SetValue( _W("General" ), _W( "ExtEncOpts" ),m_strExtEncOpts);

	g_config.GetIni().SetValue( _W("General" ), _W( "TagPictureComment" ),m_strTagPictureComment);
	g_config.GetIni().SetValue( _W("General" ), _W( "TagPictureFile" ),m_strTagPictureFile);
	g_config.GetIni().SetValue( _W("General" ), _W( "AddTagPicture" ),m_bAddTagPicture );

	g_config.GetIni().SetValue( _W("General" ), _W( "ThreadPriority" ),m_nThreadPriority);

	g_config.GetIni().SetValue( _W("General" ), _W( "CDDBSubmitVia" ),m_nCDDBSubmitVia);
	g_config.GetIni().SetValue( _W("General" ), _W( "ID3Version" ),m_nID3Version);
	g_config.GetIni().SetValue( _W("General" ), _W( "ID3EncodedBy" ),m_strID3EncodedBy); // Hydra
	g_config.GetIni().SetValue( _W("General" ), _W( "ID3Comment" ),m_strID3Comment);
	g_config.GetIni().SetValue( _W("CDDB" ), _W( "CDDBESubmitAddr" ),m_strCDDBESubmitAddr);
	g_config.GetIni().SetValue( _W("CDDB" ), _W( "CDDBHSubmitAddr" ),m_strCDDBHSubmitAddr);
	g_config.GetIni().SetValue( _W("CDDB" ), _W( "CDDBESubmitServer" ),m_strCDDBESubmitServer);
	
	g_config.GetIni().SetValue( _W("General" ), _W( "RILC" ),m_strRILC);

	g_config.GetIni().SetValue( _W("General" ), _W( "OverwriteExisting" ),m_nOverwriteExisting);
	g_config.GetIni().SetValue( _W("General" ), _W( "FileOpenRecursive" ),m_bFileOpenRecursive);
	g_config.GetIni().SetValue( _W("General" ), _W( "RecordingDevice" ),m_nRecordingDevice);
	g_config.GetIni().SetValue( _W("General" ), _W( "ID3V2TrackNumber" ),m_nID3V2TrackNumber);

	g_config.GetIni().SetValue( _W("General" ), _W( "ProfileName" ),m_strProfileName);

	g_config.GetIni().SetValue( _W("General" ), _W( "Language" ),m_strLanguage);

	// Status notification settings
	g_config.GetIni().SetValue( _W("StatusNotification" ), _W( "UseStatusServer" ), m_bUseStatusServer );
	g_config.GetIni().SetValue( _W("StatusNotification" ), _W( "StatusServer" ), m_strStatusServer );
	g_config.GetIni().SetValue( _W("StatusNotification" ), _W( "StatusServerPort" ), m_nStatusServerPort );

	// Save the remote site info
	g_RemoteSites.Save();

	// Save CD-ROM settings
    SaveCDRomSettings();

	// Flush ini file
    CUStringConvert strCnv;
    WritePrivateProfileString( 	NULL, NULL, NULL, strCnv.ToT( g_config.GetIniFileName() ) );

	// copy profile
	if ( !m_strProfileName.IsEmpty() )
	{
		// backup current settings to active profile name
		CDexCopyFile(   GetIniFileName(),
					    GetAppPath() + _W("\\") + m_strProfileName + PROFILE_EXTENTION,
					    FALSE );
	}
	EXIT_TRACE( _T( "CConfig::Save()" ) );
}

void CConfig::Load()
{
	ENTRY_TRACE( _T( "CConfig::Load()" ) );

	// Flush ini file
    CUStringConvert strCnv;
    WritePrivateProfileString( 	NULL, NULL, NULL, strCnv.ToT( g_config.GetIniFileName() ) );

	// Load MP3 Parameters
	m_bNormTrack		=g_config.GetIni().GetValue( _T("General" ), _T( "bNormTrack" ),m_bNormTrack);
	m_bRiffWav			=g_config.GetIni().GetValue( _T("General" ), _T( "OutputRiffWavaFile" ),m_bRiffWav);
	m_bRetainWavFile	=g_config.GetIni().GetValue( _T("General" ), _T( "RetainWavFile" ),m_bRetainWavFile);
	m_bWinEncoder		=g_config.GetIni().GetValue( _T("General" ), _T( "WinEncoder" ),m_bWinEncoder);
	m_nFNBufferSize		=g_config.GetIni().GetValue( _T("General" ), _T( "FNBufferSize" ),m_nFNBufferSize);

	m_strMP3OutputDir=	g_config.GetIni().GetValue( _T("General" ), _T( "MP3OutputDir" ),m_strMP3OutputDir);
	m_strCnvOutputDir=	g_config.GetIni().GetValue( _T("General" ), _T( "CnvOutputDir" ),m_strCnvOutputDir);
	m_strRecOutputDir=	g_config.GetIni().GetValue( _T("General" ), _T( "RecOutputDir" ),m_strRecOutputDir);
	m_strMP3InputDir=	g_config.GetIni().GetValue( _T("General" ), _T( "MP3InputDir" ),m_strMP3InputDir);
	m_strPlayDir=		g_config.GetIni().GetValue( _T("General" ), _T( "PlayDir" ),m_strPlayDir);

	// WAV settings
	m_strWAVInputDir=	g_config.GetIni().GetValue( _T("General" ), _T( "WAVInputDir" ),m_strWAVInputDir);
	m_bDelWavAfterConv=	g_config.GetIni().GetValue( _T("General" ), _T( "DeleteWAVAfterConversion" ),m_bDelWavAfterConv);

	m_bM3UPlayList=		g_config.GetIni().GetValue( _T("General" ), _T( "M3UPlayList" ),m_bM3UPlayList);
	m_bPLSPlayList=		g_config.GetIni().GetValue( _T("General" ), _T( "PLSPlayList" ),m_bPLSPlayList);

	m_bAutoRip= g_config.GetIni().GetValue( _T("General" ), _T( "AutoRip" ),m_bAutoRip);

	m_bSplitTrackName=	g_config.GetIni().GetValue( _T("General" ), _T( "SplitTrackName" ),m_bSplitTrackName);
	m_cSplitTrackChar=	(CHAR)g_config.GetIni().GetValue( _T("General" ), _T( "SplitTrackChar" ),m_cSplitTrackChar);
	
	m_bEjectWhenFinished=	g_config.GetIni().GetValue( _T("General" ), _T( "EjectWhenFinished" ),m_bEjectWhenFinished);
	m_bSelectAllCDTracks=	g_config.GetIni().GetValue( _T("General" ), _T( "SelectAllCDTracks" ),m_bSelectAllCDTracks);

	m_bAutoShutDown=	g_config.GetIni().GetValue( _T("General" ), _T( "AutoShutDown" ),m_bAutoShutDown);
	m_bKeepDirLayout=	g_config.GetIni().GetValue( _T("General" ), _T( "KeepDirLayout" ),m_bKeepDirLayout);

	m_nThreadPriority=		g_config.GetIni().GetValue( _T("General" ), _T( "ThreadPriority" ),m_nThreadPriority);

	m_strTempDir=	g_config.GetIni().GetValue( _T("General" ), _T( "TempDir" ),m_strTempDir);

	m_bCDDBWriteAsDosFile=	g_config.GetIni().GetValue( _T("General" ), _T( "CDDBWriteAsDosFile" ),m_bCDDBWriteAsDosFile);

	m_bCDPlayDigital=	g_config.GetIni().GetValue( _T("General" ), _T( "CDPlayDigital" ),m_bCDPlayDigital);

	m_bRecordAddSeqNr=	g_config.GetIni().GetValue( _T("General" ), _T( "RecordAddSeqNr" ),m_bRecordAddSeqNr);
	m_bRecordEncoderType=	g_config.GetIni().GetValue( _T("General" ), _T( "RecordEncoderType" ),m_bRecordEncoderType);

	if ( m_strCnvOutputDir.IsEmpty() )
        m_strCnvOutputDir=GetAppPath()+ _W( "\\my music\\");

	if (m_strRecOutputDir.IsEmpty())
        m_strRecOutputDir=GetAppPath()+ _W( "\\my music\\" );

	if (m_strMP3OutputDir.IsEmpty())
        m_strMP3OutputDir=GetAppPath()+ _W( "\\my music\\" );

	if (m_strMP3InputDir.IsEmpty())
        m_strMP3InputDir=GetAppPath()+ _W( "\\my music\\" );

	if (m_strWAVInputDir.IsEmpty())
        m_strWAVInputDir=GetAppPath()+ _W( "\\my music\\" );

	if (m_strPlayDir.IsEmpty())
        m_strPlayDir=GetAppPath() + _W( "\\my music\\" );

	// CDDB stuff
	m_strCDDBPath = g_config.GetIni().GetValue( _T("General" ), _T( "CDDBPath" ),m_strCDDBPath);

	if (m_strCDDBPath.IsEmpty())
	{
		m_strCDDBPath = GetAppPath() + _W( "\\LocalCDDB\\");
	}

	m_strWinampDBPath = FormatDirName( g_config.GetIni().GetValue( _T("General" ), _T( "WinampDBPath" ), m_strWinampDBPath) );

	// Add trailing back space
	m_strCDDBPath = FormatDirName( m_strCDDBPath );

	m_nCDDBUseProxy=	g_config.GetIni().GetValue( _T("General" ), _T( "CDBBUseProxy" ),m_nCDDBUseProxy);
	m_nCDDBUseAuthentication=g_config.GetIni().GetValue( _T("General" ), _T( "CDBBUseAuthentication" ),m_nCDDBUseAuthentication);
	m_nCDDBProxyPort=	g_config.GetIni().GetValue( _T("General" ), _T( "CDBBProxyPort" ),m_nCDDBProxyPort);
	m_nCDDBTimeOut=	g_config.GetIni().GetValue( _T("General" ), _T( "CDBBTimeOut" ),m_nCDDBTimeOut);
	m_bCDDBAutoConnect=	g_config.GetIni().GetValue( _T("General" ), _T( "CDBBAutoConnect" ),m_bCDDBAutoConnect);

	// Status notification settings
	m_bUseStatusServer = g_config.GetIni().GetValue( _T("StatusNotification" ), _T( "UseStatusServer" ), m_bUseStatusServer );
	m_strStatusServer = g_config.GetIni().GetValue( _T("StatusNotification" ), _T( "StatusServer" ), m_strStatusServer );
	m_nStatusServerPort = g_config.GetIni().GetValue( _T("StatusNotification" ), _T( "StatusServerPort" ), m_nStatusServerPort );

	// Read Proxy stuff if authentication is enabled
	if (m_nCDDBUseAuthentication)
	{
		int i;
		int nVersion=001;
		int nKey;
		int nLength;
		int nSkip;
		int nIndex=0;
		int nBufferLength=0;
		int	nSum=0;


		// Create buffer string
		INT pBuffer[255];

		// Clear memory of buffer
		memset(pBuffer,0x00,sizeof(pBuffer));

		// Read version number
		//nVersion=g_config.GetIni().GetValue( _T("ProxyAuthentication" ), _T( "Version" ),nVersion);

		// Read User Name
		m_strCDDBProxyUser=g_config.GetIni().GetValue( _T("ProxyAuthentication" ), _T( "UserName" ),m_strCDDBProxyUser);

		// Read buffer length
		nBufferLength=g_config.GetIni().GetValue( _T("ProxyAuthentication" ), _T( "PasswordLength" ),nBufferLength);

		// Read buffer
		for (i=0;i<nBufferLength;i++)
		{
			CUString strKey;
			strKey.Format( _W( "Password%03d" ), i );

			pBuffer[ i ] = g_config.GetIni().GetValue(	_T( "ProxyAuthentication" ),
														strKey,
														pBuffer[ i ] );
		}

		// Get main encryption key
		nKey=pBuffer[nIndex++];

		// Get length of string back
		nLength=pBuffer[nIndex++] ^ (int)sqrt((double)abs(nKey)+100);

		// determine a skip value between 3..10 
		nSkip=(nLength + nKey)&7 + 3;

		// Generate nSkip keys and keep track of sum
		for (i=0;i<nSkip;i++,nIndex++)
		{
			// Add generated number to sum
			nSum+=pBuffer[nIndex];
		}

		char	lpszTemp[80];
		int		iTemp;

		// Clear result string
		memset(lpszTemp,0x00,sizeof(lpszTemp));

		// Decypher encrypted password
		for (i=0;i<nLength;i++,nIndex++)
		{
			// Decypher character
			iTemp=pBuffer[nIndex] ^ (nSum^(int)sqrt((double)abs(pBuffer[nIndex-1]+pBuffer[nIndex-2])+3454));
			lpszTemp[i]= (char)iTemp;
		}

		// 
        m_strCDDBProxyPassword = CUString( lpszTemp, CP_UTF8);
	}


	m_bCDDBLongDirNames=g_config.GetIni().GetValue( _W("General" ), _W( "CDBBLongDirNames" ),m_bCDDBLongDirNames);
	m_nLocalCDDBType=	g_config.GetIni().GetValue( _W("General" ), _W( "LocalCDDBType" ),m_nLocalCDDBType);
	m_bSaveToCDPlayer=	g_config.GetIni().GetValue( _W("General" ), _W( "SaveToCDPlayer" ),m_bSaveToCDPlayer);
	m_bSaveToLocCDDB=	g_config.GetIni().GetValue( _W("General" ), _W( "SaveToLocalCDDB" ),m_bSaveToLocCDDB);
	m_strEmailAddress=	g_config.GetIni().GetValue( _W("General" ), _W( "CDDDEmailAddress" ),m_strEmailAddress);
	m_strProxyAddress=	g_config.GetIni().GetValue( _W("General" ), _W( "CDDDProxyAddress" ),m_strProxyAddress);

	m_nPCopyEncType=	g_config.GetIni().GetValue( _W("General" ), _W( "PCopyEncType" ),m_nPCopyEncType);

	m_nLowNormLevel=	g_config.GetIni().GetValue( _W("General" ), _W( "NormLowLevel" ),m_nLowNormLevel);
	m_nHighNormLevel=	g_config.GetIni().GetValue( _W("General" ), _W( "NormHighLevel" ),m_nHighNormLevel);
	m_nLNormFactor=		g_config.GetIni().GetValue( _W("General" ), _W( "LNormFactor" ),m_nLNormFactor);
	m_nHNormFactor=		g_config.GetIni().GetValue( _W("General" ), _W( "HNormFactor" ),m_nHNormFactor);

	m_nEncoderType=		g_config.GetIni().GetValue( _W("General" ), _W( "EncoderType" ),m_nEncoderType);
	m_strExtEncPath=	g_config.GetIni().GetValue( _W("General" ), _W( "ExtEncPath" ),m_strExtEncPath);
	m_strExtEncOpts=	g_config.GetIni().GetValue( _W("General" ), _W( "ExtEncOpts" ),m_strExtEncOpts);

	m_strTagPictureComment = g_config.GetIni().GetValue( _W("General" ), _W( "TagPictureComment" ),m_strTagPictureComment);
	m_strTagPictureFile    = g_config.GetIni().GetValue( _W("General" ), _W( "TagPictureFile" ),m_strTagPictureFile);
	m_bAddTagPicture       = g_config.GetIni().GetValue( _W("General" ), _W( "AddTagPicture" ),m_bAddTagPicture );

	m_nCDDBSubmitVia=	g_config.GetIni().GetValue( _W("General" ), _W( "CDDBSubmitVia" ),m_nCDDBSubmitVia);
	m_nID3Version=		g_config.GetIni().GetValue( _W("General" ), _W( "ID3Version" ),m_nID3Version);

	m_strID3EncodedBy=		g_config.GetIni().GetValue( _W("General" ), _W( "ID3EncodedBy" ),m_strID3EncodedBy);

	m_strID3Comment=	g_config.GetIni().GetValue( _W("General" ), _W( "ID3Comment" ),m_strID3Comment);
	m_strCDDBESubmitAddr=g_config.GetIni().GetValue( _W("CDDB" ), _W( "CDDBESubmitAddr" ),m_strCDDBESubmitAddr);
	m_strCDDBHSubmitAddr=g_config.GetIni().GetValue( _W("CDDB" ), _W( "CDDBHSubmitAddr" ),m_strCDDBHSubmitAddr);
	m_strCDDBESubmitServer=g_config.GetIni().GetValue( _W("CDDB" ), _W( "CDDBESubmitServer" ),m_strCDDBESubmitServer);
	
	m_strRILC			=g_config.GetIni().GetValue( _W("General" ), _W( "RILC" ),m_strRILC);

	m_nOverwriteExisting=	g_config.GetIni().GetValue( _W("General" ), _W( "OverwriteExisting" ),m_nOverwriteExisting);
	m_bFileOpenRecursive=	g_config.GetIni().GetValue( _W("General" ), _W( "FileOpenRecursive" ),m_bFileOpenRecursive);

	m_nRecordingDevice=	g_config.GetIni().GetValue( _W("General" ), _W( "RecordingDevice" ),m_nRecordingDevice);
	m_nID3V2TrackNumber=	ID3TRKNRTTYPE( g_config.GetIni().GetValue( _W("General" ), _W( "ID3V2TrackNumber" ), m_nID3V2TrackNumber ) );

	m_strProfileName=	g_config.GetIni().GetValue( _W("General" ), _W( "ProfileName" ),m_strProfileName);

	m_strLanguage=	g_config.GetIni().GetValue( _W("General" ), _W( "Language" ),m_strLanguage);

	// DEBUG STUFF
	m_bDumpToc=		g_config.GetIni().GetValue( _W("General" ), _W( "DumpToc" ),m_bDumpToc);

//	m_nSepChar=g_config.GetIni().GetValue( _W("General" ), _W( "SepCharacter" ),m_nSepChar);

	CUString strTmp;

	// Get the string
	m_strFileFormat=g_config.GetIni().GetValue( _W("General" ), _W( "FileFormatString" ),m_strFileFormat);
	m_strPlsFileFormat=g_config.GetIni().GetValue( _W("General" ), _W( "PlsFileFormatString" ),m_strPlsFileFormat);

	// Load the remote site info
	g_RemoteSites.Load();

	// Create the local CDDB path
	DoesDirExist( GetCDDBPath(), FALSE );

	EXIT_TRACE( _T( "CConfig::Load()" ) );
}


CDEX_ERR DoesDirExist(CUString& strDirTest,BOOL bVerbose) 
{
	if (strDirTest.IsEmpty())
		return TRUE;

	// current dir
	TCHAR lpszCurDir[ MAX_PATH + 1 ] = { _T( '\0' ),};
	
	// Get current directory
	_tgetcwd( lpszCurDir, sizeof( lpszCurDir ) );

	// Does the dir exist
    CUStringConvert strCnv;

    if ( _tchdir( strCnv.ToT( strDirTest ) ) != 0 )
	{
		if (!bVerbose)
		{
			if (MyCreateDir(strDirTest)!=0)
			{
				// return to current directory
				_tchdir( lpszCurDir );
				return CDEX_OK;
			}
		}
		else
		{
			CUString strTmp;
			strTmp.Format(	_W( "The output directory %s does not exist\n\rWoul you like to create it" ), (LPCWSTR)strDirTest );
			
            if (CDexMessageBox( strTmp, MB_YESNO ) == IDYES )
			{
				if (MyCreateDir(strDirTest)!=0)
				{
					// return to current directory
					_tchdir( lpszCurDir );
					return CDEX_OK;
				}
			}
			else
			{
				_tchdir( lpszCurDir );
				return CDEX_ERROR;
			}
		}
	}

	// return to current directory
	_tchdir( lpszCurDir );

	// Return that directory does exist
	return CDEX_OK;
}





// CONSTRUCTOR
CRemoteSites::CRemoteSites()
{
	// Start Fresh
	ClearAll();

	// Always add the default servers
	AddDefaultSites();
}

// DESTRUCTOR
CRemoteSites::~CRemoteSites()
{
}

// ASSIGNMENT OPERATOR
CRemoteSites& CRemoteSites::operator=(const CRemoteSites& rhs)
{
	// Avoid self assignment
	if (this!= &rhs)
	{
		m_vRemoteSites = rhs.m_vRemoteSites;
		m_nActiveSite = rhs.m_nActiveSite;	
	}
	return *this;
}

// COPY CONSTRUCTOR
CRemoteSites::CRemoteSites(const CRemoteSites& rhs)
{
	m_vRemoteSites = rhs.m_vRemoteSites;
	m_nActiveSite = rhs.m_nActiveSite;	
}


// AddSite
void CRemoteSites::AddSite(	CUString strAddress,
							CUString strLocation,
							CUString strPath,
							int		nPort,
							int		nProtocol,
							int		nEditable)
{
	// Create a new CRemoteSiteInfo class on the stack
	CRemoteSiteInfo newSite;

	// Fill in the information
	newSite.m_strAddress=strAddress;
	newSite.m_strLocation=strLocation;
	newSite.m_strPath=strPath;

	newSite.m_nPort=nPort;
	newSite.m_nProtocol=nProtocol;
	newSite.m_nEditable=nEditable;


	// Add it to the array of RemoteSites
	m_vRemoteSites.push_back(newSite);
}

// Edit Site information
void CRemoteSites::SetSite(	int iIndex,
						    CUString strAddress,
							CUString strLocation,
							CUString strPath,
							int		nPort,
							int		nProtocol)
{
	// Edit the information
	m_vRemoteSites[iIndex].m_strAddress=strAddress;
	m_vRemoteSites[iIndex].m_strLocation=strLocation;
	m_vRemoteSites[iIndex].m_strPath=strPath;

	m_vRemoteSites[iIndex].m_nPort=nPort;
	m_vRemoteSites[iIndex].m_nProtocol=nProtocol;
}

// Clear All Sites
void CRemoteSites::ClearAll()
{
	m_vRemoteSites.clear();
	m_nActiveSite=0;
}


void CRemoteSites::AddDefaultSites()
{
	// This site (both CDDB protocl as well as HTTP protocol are ALWAYS available)

	// The http protocol site
	AddSite( _T("freedb.freedb.org" ), _T( "freedb, Random freedb server" ), _T( "/~cddb/cddb.cgi" ),80,OPTIONS_CDDB_USEHTTP );

	// The cddp protocol site
	AddSite( _T("freedb.freedb.org" ), _T( "freedb, freedb, Random freedb server" ), _T( "-" ),888,OPTIONS_CDDB_USETCPIP );

	// The http protocol site
	// AddSite( _T("cddb.cddb.com" ), _T( "Carmel, IN USA" ), _T( "/~cddb/cddb.cgi" ),80,OPTIONS_CDDB_USEHTTP );

	// The cddp protocol site
	// AddSite( _T("cddb.cddb.com" ), _T( "Carmel, IN USA" ), _T( "-" ),888,OPTIONS_CDDB_USETCPIP );
}


// Save Site to INI file
void CRemoteSites::Save()
{
	CUString strCDDBServer( _W( "CDDB Servers" ) );

	// Get the number of remote servers
	int nNumSites=m_vRemoteSites.size();

	// Save number of entries
	g_config.GetIni().SetValue( strCDDBServer, _T( "NumSites" ),nNumSites);
	g_config.GetIni().SetValue( strCDDBServer, _T( "ActiveSite" ),m_nActiveSite);

	// Save all entries to INI file
	for (int i=0; i<nNumSites; i++ )
	{
		CUString strIndex;
		strIndex.Format( _W( "%d" ), i );

		g_config.GetIni().SetValue( strCDDBServer, _W( "Address" ) + strIndex, m_vRemoteSites[i].m_strAddress );
		g_config.GetIni().SetValue( strCDDBServer, _W( "Location" ) + strIndex, m_vRemoteSites[i].m_strLocation );
		g_config.GetIni().SetValue( strCDDBServer, _W( "Path" ) + strIndex, m_vRemoteSites[i].m_strPath );
		g_config.GetIni().SetValue( strCDDBServer, _W( "Port" ) + strIndex,m_vRemoteSites[i].m_nPort );
		g_config.GetIni().SetValue( strCDDBServer, _W( "Protocol" ) + strIndex,m_vRemoteSites[i].m_nProtocol );
		g_config.GetIni().SetValue( strCDDBServer,  _W( "Editable" ) + strIndex,m_vRemoteSites[i].m_nEditable );
	}
}


// Load sites from INI file
void CRemoteSites::Load()
{

	CUString strCDDBServer = _T( "CDDB Servers" );

	// Remove all junk
	ClearAll();

	// Load number of entries
	int nNumSites= g_config.GetIni().GetValue( strCDDBServer, _T( "NumSites" ), 0 );
	m_nActiveSite= g_config.GetIni().GetValue( strCDDBServer, _T( "ActiveSite" ), 0 );

	if ( nNumSites > 0)
	{
		// Load all entries to INI file
		for (int i=0;i<nNumSites;i++)
		{
			CUString strIndex;
			strIndex.Format( _W( "%d" ), i );

			AddSite(	
					g_config.GetIni().GetValue( strCDDBServer, _W( "Address" ) + strIndex, _T( "" ) ),
					g_config.GetIni().GetValue( strCDDBServer, _W( "Location" ) + strIndex,_T( "" ) ),
					g_config.GetIni().GetValue( strCDDBServer, _W( "Path" ) + strIndex, _T( "" ) ),
					g_config.GetIni().GetValue( strCDDBServer, _W( "Port" ) + strIndex, 0 ),
					g_config.GetIni().GetValue( strCDDBServer, _W( "Protocol" ) + strIndex, 0 ),
					g_config.GetIni().GetValue( strCDDBServer, _W( "Editable" ) + strIndex, FALSE )
					);
		}
	}
	else
	{
		// Add the default sites
		AddDefaultSites();
	}
}


// CONSTRUCTOR
CRemoteSiteInfo::CRemoteSiteInfo()
{
	m_strAddress = _W( "" );
	m_strLocation=_W( "" );
	m_strPath=_W( "" );
	m_nPort=0;
	m_nProtocol=0;
	m_nEditable=FALSE;
}


// DESTRUCTOR
CRemoteSiteInfo::~CRemoteSiteInfo()
{
}

// ASSIGNMENT OPERATOR
CRemoteSiteInfo& CRemoteSiteInfo::operator=(const CRemoteSiteInfo& rhs)
{
	// Avoid self assignment
	if (this!= &rhs)
	{
		m_strAddress=rhs.m_strAddress;
		m_strLocation=rhs.m_strLocation;
		m_strPath=rhs.m_strPath;
		m_nPort=rhs.m_nPort;
		m_nProtocol=rhs.m_nProtocol;
		m_nEditable=rhs.m_nEditable;
	}
	return *this;
}

// COPY CONSTRUCTOR
CRemoteSiteInfo::CRemoteSiteInfo(const CRemoteSiteInfo& rhs)
{
	m_strAddress=rhs.m_strAddress;
	m_strLocation=rhs.m_strLocation;
	m_strPath=rhs.m_strPath;
	m_nPort=rhs.m_nPort;
	m_nProtocol=rhs.m_nProtocol;
	m_nEditable=rhs.m_nEditable;
}

CUString CConfig::FormatDirName( const CUString& strDir ) const
{
    CUString strReturn =  strDir;

	if ( strDir.GetLength() > 0 )
	{
		if ( strDir[ strDir.GetLength() - 1 ] != _W( '\\' ) )
        {
		    strReturn += _W( "\\" );
        }
	}
	
	return strReturn;
}

CUString CConfig::GetMP3OutputDir() const
{
	return FormatDirName( m_strMP3OutputDir );
}


CUString CConfig::GetMP3InputDir() const
{
	return FormatDirName( m_strMP3InputDir );
}


CUString CConfig::GetPlayDir() const
{
	return FormatDirName( m_strPlayDir );
}

CUString CConfig::GetCnvOutputDir() const
{
	return FormatDirName( m_strCnvOutputDir );
}

CUString CConfig::GetRecOutputDir() const
{
	return FormatDirName( m_strRecOutputDir );
}

CUString CConfig::GetWAVInputDir() const
{
	return FormatDirName( m_strWAVInputDir );
}

CUString CConfig::GetTempDir() const
{
	return FormatDirName( m_strTempDir );
}

CUString CConfig::GetCDDBPath() const
{
	return FormatDirName( m_strCDDBPath );
}

CUString CConfig::GetWinampDBPath() const
{
	return FormatDirName( m_strWinampDBPath );
}

void CConfig::SelectProfile( const CUString& strProfileName )
{
	// copy profile
    CUStringConvert strCnv;

	CopyFile(	strCnv.ToT( GetAppPath() + _W("\\") + strProfileName + PROFILE_EXTENTION),
				strCnv.ToT( GetIniFileName() ), 
				FALSE );

	// Load settings
	Load();

	// set new profile name
	SetProfileName( strProfileName );
}
void CConfig::LoadCDRomSettings()
{
    CIni& myIni = GetIni();

    for ( int cdRomIdx = 0; cdRomIdx < CR_GetNumCDROM(); cdRomIdx++ ) 
    {
        CDROMPARAMS cdSettings;

        CR_SetActiveCDROM( cdRomIdx );
        CR_GetCDROMParameters( &cdSettings );

        TCHAR lpszKey[255] = { _T( '\0') ,} ;

        // assign data, replace spaces with underscores
	    for (unsigned int j = 0; j < strlen( cdSettings.lpszCDROMID ); j++ )
        {
		    if ( cdSettings.lpszCDROMID[ j ] == _T( ' ' ) )
            {
			    lpszKey[j] = _T( '_' );
            }
            else
            {
			    lpszKey[j] = cdSettings.lpszCDROMID[j];
            }
        }
	    cdSettings.nOffsetStart			= myIni.GetValue(lpszKey, _T("nOffsetStart"			),cdSettings.nOffsetStart );
	    cdSettings.nOffsetEnd			= myIni.GetValue(lpszKey, _T("nOffsetEnd"			),cdSettings.nOffsetEnd );
	    cdSettings.nSpeed				= myIni.GetValue(lpszKey, _T("nSpeed"				),cdSettings.nSpeed );
	    cdSettings.nSpinUpTime			= myIni.GetValue(lpszKey, _T("nSpinUpTime"			),cdSettings.nSpinUpTime );
	    cdSettings.bJitterCorrection	= myIni.GetValue(lpszKey, _T("bJitterCorrection"	),cdSettings.bJitterCorrection );
	    cdSettings.bSwapLefRightChannel	= myIni.GetValue(lpszKey, _T("bSwapLefRightChannel" ),cdSettings.bSwapLefRightChannel );
	    cdSettings.nNumOverlapSectors	= myIni.GetValue(lpszKey, _T("nNumOverlapSectors"	),cdSettings.nNumOverlapSectors );
	    cdSettings.nNumReadSectors		= myIni.GetValue(lpszKey, _T("nNumReadSectors"		),cdSettings.nNumReadSectors );
	    cdSettings.nNumCompareSectors	= myIni.GetValue(lpszKey, _T("nNumCompareSectors"	),cdSettings.nNumCompareSectors );
	    cdSettings.nAspiTimeOut			= myIni.GetValue(lpszKey, _T("nAspiTimeOut"			),cdSettings.nAspiTimeOut );
	    cdSettings.nAspiRetries			= myIni.GetValue(lpszKey, _T("nAspiRetries"			),cdSettings.nAspiRetries );
	    cdSettings.bEnableMultiRead		= myIni.GetValue(lpszKey, _T("bEnableMultiRead"		),cdSettings.bEnableMultiRead );
	    cdSettings.bMultiReadFirstOnly	= myIni.GetValue(lpszKey, _T("bMultiReadFirstOnly"	),cdSettings.bMultiReadFirstOnly );
	    cdSettings.bFUA					= myIni.GetValue(lpszKey, _T("bFUA"					),cdSettings.bFUA);
	    cdSettings.nMultiReadCount		= myIni.GetValue(lpszKey, _T("nMultiReadCount"		),cdSettings.nMultiReadCount );
	    cdSettings.bLockDuringRead		= myIni.GetValue(lpszKey, _T("bLockDuringRead"		),cdSettings.bLockDuringRead );
	    cdSettings.bUseCDText			= myIni.GetValue(lpszKey, _T("bUseCDText"			),cdSettings.bUseCDText );
	    cdSettings.bReadC2Errors        = myIni.GetValue(lpszKey, _T("bReadC2Errors"		),cdSettings.bReadC2Errors );

	    // Custom Drive Settings
	    cdSettings.DriveTable.DriveType		= DRIVETYPE(myIni.GetValue(lpszKey, _T("DriveType"),cdSettings.DriveTable.DriveType ));
	    cdSettings.DriveTable.ReadMethod	= READMETHOD(myIni.GetValue(lpszKey, _T("ReadMethod"),cdSettings.DriveTable.ReadMethod ));
	    cdSettings.DriveTable.SetSpeed		= SETSPEED(myIni.GetValue(lpszKey, _T("SetSpeed"),cdSettings.DriveTable.SetSpeed ));
	    cdSettings.DriveTable.EnableMode	= ENABLEMODE(myIni.GetValue(lpszKey, _T("EnableMode"),cdSettings.DriveTable.EnableMode ));
	    cdSettings.DriveTable.nDensity		= myIni.GetValue(lpszKey, _T("nDensity"),cdSettings.DriveTable.nDensity );
	    cdSettings.DriveTable.Endian		= ENDIAN(myIni.GetValue(lpszKey, _T("Endian"),cdSettings.DriveTable.Endian ));
	    cdSettings.DriveTable.bAtapi		= myIni.GetValue(lpszKey, _T("bAtapi"),cdSettings.DriveTable.bAtapi );
	    cdSettings.bAspiPosting				= myIni.GetValue(lpszKey, _T("bAspiPosting"		),cdSettings.bAspiPosting );
	    cdSettings.nRippingMode				= myIni.GetValue(lpszKey, _T("nRippingMode"		),cdSettings.nRippingMode );
	    cdSettings.nParanoiaMode			= myIni.GetValue(lpszKey, _T("nParanoiaMode"	),cdSettings.nParanoiaMode );
	    cdSettings.nSampleOffset			= myIni.GetValue(lpszKey, _T("nSampleOffset"	),cdSettings.nSampleOffset );


        if ( cdSettings.nSampleOffset == LONG_MIN )
        {
            CUString strSearch;

            strSearch.Format( _W( "%s - %s" ), 
                (LPCWSTR)CUString( cdSettings.lpszVendorID, CP_UTF8 ), 
                (LPCWSTR)CUString( cdSettings.lpszProductID, CP_UTF8 ) ); 
            DriveOffset driveOffset;
        
            LONG offset = 0;

            if ( driveOffset.GetDriveOffset( strSearch, offset ) )
            {
                    cdSettings.nSampleOffset = offset;
            }
        }

        CR_SetCDROMParameters( &cdSettings );
    }

    LONG nActive = 0;

	// Load active CD-ROM setting
	nActive = myIni.GetValue( _T( "CD-ROM" ), _T( "nActive" ), nActive );

	// Make sure selection if valid
	nActive= min( max( 0, nActive ), (int)CR_GetNumCDROM() - 1 );

    CR_SetActiveCDROM( nActive );
}

void CConfig::SaveCDRomSettings()
{
    LONG currentSelection = CR_GetActiveCDROM();


    CIni& myIni = GetIni();
    
    for ( int cdRomIdx = 0; cdRomIdx < CR_GetNumCDROM(); cdRomIdx++ ) 
    {
        CDROMPARAMS cdromParams;

        CR_SetActiveCDROM( cdRomIdx );
        CR_GetCDROMParameters( &cdromParams );


        TCHAR lpszKey[255] = { _T( '\0') ,} ;

        // assign data, replace spaces with underscores
	    for (unsigned int j = 0; j < strlen( cdromParams.lpszCDROMID ); j++ )
        {
		    if ( cdromParams.lpszCDROMID[ j ] == _T( ' ' ) )
            {
			    lpszKey[j] = _T( '_' );
            }
            else
            {
			    lpszKey[j] = cdromParams.lpszCDROMID[j];
            }
        }

        myIni.SetValue(lpszKey, _T("nOffsetStart" ),cdromParams.nOffsetStart);
	    myIni.SetValue(lpszKey, _T("nOffsetEnd" ),cdromParams.nOffsetEnd);
	    myIni.SetValue(lpszKey, _T("nSpeed"	),cdromParams.nSpeed );
	    myIni.SetValue(lpszKey, _T("nSpinUpTime" ),cdromParams.nSpinUpTime );
	    myIni.SetValue(lpszKey, _T("bJitterCorrection" ),cdromParams.bJitterCorrection );
	    myIni.SetValue(lpszKey, _T("bSwapLefRightChannel" ),cdromParams.bSwapLefRightChannel );
	    myIni.SetValue(lpszKey, _T("nNumOverlapSectors"	),cdromParams.nNumOverlapSectors );
	    myIni.SetValue(lpszKey, _T("nNumReadSectors" ),cdromParams.nNumReadSectors );
	    myIni.SetValue(lpszKey, _T("nNumCompareSectors"	),cdromParams.nNumCompareSectors );
	    myIni.SetValue(lpszKey, _T("nAspiTimeOut" ),cdromParams.nAspiTimeOut );
	    myIni.SetValue(lpszKey, _T("nAspiRetries" ),cdromParams.nAspiRetries );
	    myIni.SetValue(lpszKey, _T("bEnableMultiRead" ),cdromParams.bEnableMultiRead );
	    myIni.SetValue(lpszKey, _T("bMultiReadFirstOnly" ),cdromParams.bMultiReadFirstOnly );
	    myIni.SetValue(lpszKey, _T("bFUA" ),cdromParams.bFUA );
	    myIni.SetValue(lpszKey, _T("nSampleOffset" ),cdromParams.nSampleOffset);
	    myIni.SetValue(lpszKey, _T("nMultiReadCount" ),cdromParams.nMultiReadCount );
	    myIni.SetValue(lpszKey, _T("bLockDuringRead" ),cdromParams.bLockDuringRead );
	    myIni.SetValue(lpszKey, _T("bUseCDText" ),cdromParams.bUseCDText );
	    myIni.SetValue(lpszKey, _T("bReadC2Errors" ),cdromParams.bReadC2Errors );

	    // Write Drive Settings
	    myIni.SetValue(lpszKey, _T("DriveType" ),cdromParams.DriveTable.DriveType );
	    myIni.SetValue(lpszKey, _T("ReadMethod" ),cdromParams.DriveTable.ReadMethod );
	    myIni.SetValue(lpszKey, _T("SetSpeed" ),cdromParams.DriveTable.SetSpeed );
	    myIni.SetValue(lpszKey, _T("EnableMode" ),cdromParams.DriveTable.EnableMode );
	    myIni.SetValue(lpszKey, _T("nDensity" ),cdromParams.DriveTable.nDensity );
	    myIni.SetValue(lpszKey, _T("Endian" ),cdromParams.DriveTable.Endian );
	    myIni.SetValue(lpszKey, _T("bAtapi" ),cdromParams.DriveTable.bAtapi );
	    myIni.SetValue(lpszKey, _T("bAspiPosting" ),cdromParams.bAspiPosting );
	    myIni.SetValue(lpszKey, _T("nParanoiaMode" ),cdromParams.nParanoiaMode );
	    myIni.SetValue(lpszKey, _T("nRippingMode" ),cdromParams.nRippingMode );
    }

    // restore old setting
    CR_SetActiveCDROM( currentSelection );

	// Save active CD-ROM setting
    myIni.SetValue( _T( "CD-ROM" ), _T( "nActive" ), currentSelection );
}


int CConfig::GetTransportLayer()
{
    int transportLayer = 0;

    CIni& myIni = GetIni();
    transportLayer = myIni.GetValue( _T( "CD-ROM" ), _T( "nTransportLayer" ), transportLayer );
    return transportLayer;
}

void CConfig::SetTransportLayer( int transportLayer )
{
    CIni& myIni = GetIni();
    myIni.SetValue( _T( "CD-ROM" ), _T( "nTransportLayer" ), transportLayer );
}
