/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
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


#ifndef CONFIG_INCLUDED
#define CONFIG_INCLUDED

#include "Util.h"
#include "Ini.h"
#include "GenreTable.h"
#include ".\CDRip\CDRip.h"

#define PROFILE_EXTENTION _W( ".prf.ini" )


#define MAX_TOC_SIZE 804

typedef BYTE RAW_TOC_TYPE[ MAX_TOC_SIZE ];

typedef struct tagMP3PARAMS
{
	char			lpszFileName[255];
	VOID*			pErrorFunc;
	int				nBitsPerSample;
	int				nChannels;
	DWORD			dwSampleFreq;

	int				nMP3Mode;
	int				nMP3BitRate;
	int				nMP3AcousticModel;
	int				nMP3Emphasis;
	int				nLayer;
	char			lpszWavFileName[255];

} MP3PARAMS;


#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// Config.h : header file
//

#include "cdrip\cdrip.h"


#define WAVETYPE			(0)
#define MP3TYPE				(1)

#define ID3_VERSION_NONE	(0)
#define ID3_VERSION_1		(1)
#define	ID3_VERSION_2		(2)
#define	ID3_VERSION_BOTH	(3)


// Error func

void FAR PASCAL EXPORT ErrorCallBackFunc(char* lpszFile,int nLine,char* lpszError);

#define WINDOWSTYPECDDB 0
#define UNIXTYPECDDB 1

#define WAVE_RIFF_PCM 0
#define WAVE_PCM_LOHI 1
#define WAVE_PCM_HILO 2
#define WAVE_PCM_AIFF 3

#define TYPE_MPEG_I   1
#define TYPE_MPEG_II  0

#define LAYR_I        0
#define LAYR_II       1
#define LAYR_III      2

#define MODE_STEREO   0
#define MODE_JSTEREO  1
#define MODE_DUAL     2
#define MODE_MONO     3

#define PSY_NONE      0
#define PSYC_MUSICAM  1
#define PSYC_ATT      2

#define EMPH_NONE     0
#define EMPH_5015     1
#define EMPH_CITT     2


#define MAXTRK		100


#define FNMASK_ARTIST		0x01
#define FNMASK_TITLE		0x02
#define FNMASK_TRACKNAME	0x04
#define FNMASK_TRACKNUMBER  0x08


#define IDX_ARTIST		0x00
#define IDX_TITLE		0x01
#define IDX_TRACKNAME	0x02
#define IDX_TRACKNUMBER 0x03



#define OPTIONS_CDDB_USEHTTP  0x01
#define OPTIONS_CDDB_USETCPIP 0x00


//#define OPTIONS_CDDB_USEFTP  0x04
//#define OPTIONS_CDDB_USEAUTHENTICATION 0x08

//#define OPTIONS_CDDB_USEPROXY 0x10
//#define OPTIONS_CDDB_USEPROXY 0x20
//#define OPTIONS_CDDB_USEPROXY 0x40

class CRemoteSiteInfo: public CObject
{
public:
	// CONSTRUCTOR
	CRemoteSiteInfo();

	// DESTRUCTOR
	~CRemoteSiteInfo();

	// COPY CONSTRUCTOR
	CRemoteSiteInfo(const CRemoteSiteInfo& rhs);

	// ASSIGNMENT OPERATOR
	CRemoteSiteInfo& operator=(const CRemoteSiteInfo& rhs);

	CUString	m_strAddress;
	CUString m_strLocation;
	CUString m_strPath;
	int		m_nPort;
	int		m_nProtocol;
	int		m_nEditable;
};

class CRemoteSites:public CObject
{
private:
	vector<CRemoteSiteInfo>	m_vRemoteSites;
	WORD					m_nActiveSite;	
public:
	// CONSTRUCTOR
	CRemoteSites();

	// DESTRUCTOR
	~CRemoteSites();

	// COPY CONSTRUCTOR
	CRemoteSites(const CRemoteSites& rhs);

	// ASSIGNMENT OPERATOR
	CRemoteSites& operator=(const CRemoteSites& rhs);

	// ACCESSORS
	CUString GetAddress(int iIndex)	const {return m_vRemoteSites[iIndex].m_strAddress;}
	CUString GetAddress()	const {return m_vRemoteSites[m_nActiveSite].m_strAddress;}
	void	SetAddress(int iIndex, const CUString& strValue) {m_vRemoteSites[iIndex].m_strAddress= strValue;}

	CUString GetLocation(int iIndex)	const {return m_vRemoteSites[iIndex].m_strLocation;}
	CUString GetLocation()	const {return m_vRemoteSites[m_nActiveSite].m_strLocation;}

	CUString GetPath(int iIndex)	const {return m_vRemoteSites[iIndex].m_strPath;}
	CUString GetPath()	const {return m_vRemoteSites[m_nActiveSite].m_strPath;}
	void	SetPath(int iIndex, const CUString& strValue) {m_vRemoteSites[iIndex].m_strPath = strValue;}

	int		GetPort(int iIndex)	const {return m_vRemoteSites[iIndex].m_nPort;}
	int		GetPort()	const {return m_vRemoteSites[m_nActiveSite].m_nPort;}

	int		IsEditable(int iIndex)	const {return m_vRemoteSites[iIndex].m_nEditable;}
	int		IsEditable()	const {return m_vRemoteSites[m_nActiveSite].m_nEditable;}

	int		GetProtocol(int iIndex)	const {return m_vRemoteSites[iIndex].m_nProtocol;}
	int		GetProtocol()	const {return m_vRemoteSites[m_nActiveSite].m_nProtocol;}
	
	WORD	GetNumSites() const {return m_vRemoteSites.size();}
	int		GetActiveSite() const {return m_nActiveSite;}
	void 	SetActiveSite(int nActiveSite) {m_nActiveSite=nActiveSite;}

	// MUTATORS
	void AddSite(	CUString strAddress,
					CUString strLocation,
					CUString strPath,
					int		nPort,
					int		nProtocol,
					int		nEditable=FALSE);
	void SetSite(	int		iIndex,
				    CUString strAddress,
					CUString strLocation,
					CUString strPath,
					int		nPort,
					int		nProtocol);
	void ClearAll();
	void Save();
	void Load();

	void AddDefaultSites();
private:
};


class CADebug:public CObject
{
private:
	FILE*	m_pFile;
	CUString	m_strFileName;
public:
	// CONSTRUCTORS
	CADebug();
	CADebug( CUString& strFileName, BOOL bDelete = FALSE );

	// DESTRUCTOR
	~CADebug();

	void Close();
	void Open( CUString& strFileName, BOOL bDelete );

	// Binary Dumping of a buffer
	void DumpBinaray(LPBYTE pbtBuffer,int nSize);

	void printf(const char* pzFormat, ...);
};


extern CRemoteSites	g_RemoteSites;
int			MyCreateDir(CUString strDir);
CDEX_ERR	DoesDirExist(CUString& strDirTest,BOOL bVerbose);

/////////////////////////////////////////////////////////////////////////////
// CConfigDlg dialog

class CConfig
{
public:
	enum ID3TRKNRTTYPE { ID3TRKNRTTYPE_T, ID3TRKNRTTYPE_T_TT, ID3TRKNRTTYPE_0T_TT, ID3TRKNRTTYPE_0T_0TT };
private:
	CUString		m_strMP3OutputDir;
	CUString		m_strPlayDir;
	CUString		m_strCnvOutputDir;
	CUString		m_strWAVInputDir;
	CUString		m_strRecOutputDir;
	CUString		m_strTempDir;
	CUString		m_strMP3InputDir;
	BOOL		m_bNormTrack;
	CUString 	m_strAppPath;
	CUString		m_strVersion;
	CUString		m_strCDDBPath;
	CUString		m_strWinampDBPath;
	BOOL		m_bCDDBLongDirNames;
	int			m_nCDDBProxyPort;
	int			m_nCDDBTimeOut;
	int			m_nCDDBUseProxy;
	BOOL		m_bCDDBAutoConnect;
	CUString		m_strEmailAddress;
	CUString		m_strProxyAddress;
	BOOL		m_bPLSPlayList;
	BOOL		m_bM3UPlayList;
	BOOL		m_bDumpToc;
	CUString		m_strFileFormat;
	CUString		m_strPlsFileFormat;
	int			m_nLocalCDDBType;
	int			m_nPCopyEncType;
	BOOL		m_bDelWavAfterConv;
	BOOL		m_bRiffWav;
	BOOL		m_bRetainWavFile;
	BOOL		m_bWinEncoder;
	BOOL		m_bSaveToLocCDDB;
	BOOL		m_bSaveToCDPlayer;

	int			m_nLowNormLevel;
	int			m_nHighNormLevel;
	int			m_nLNormFactor;
	int			m_nHNormFactor;

	int			m_nEncoderType;
	CUString		m_strExtEncPath;
	CUString		m_strExtEncOpts;

	int			m_nThreadPriority;
	CUString		m_strCDDBProxyUser;
	CUString		m_strCDDBProxyPassword;
	int			m_nCDDBUseAuthentication;

	int			m_nFNBufferSize;

	CUString		m_strID3Comment;
	CUString		m_strID3EncodedBy; // Hydra
	CUString		m_strCDDBESubmitAddr;
	CUString		m_strCDDBHSubmitAddr;
	CUString		m_strCDDBESubmitServer;
	int			m_nCDDBSubmitVia;
	int			m_nID3Version;

	CUString		m_strRILC;
    CUString		m_strTagPictureComment;
    CUString		m_strTagPictureFile;
    BOOL            m_bAddTagPicture;

	BOOL        m_bAutoRip;

	TCHAR		m_cSplitTrackChar;
	BOOL		m_bSplitTrackName;

	BOOL		m_bEjectWhenFinished;
	BOOL		m_bAutoShutDown;
	BOOL		m_bKeepDirLayout;

	BOOL		m_bSelectAllCDTracks;

	int			m_nOverwriteExisting;
	int			m_nRecordingDevice;

	ID3TRKNRTTYPE	m_nID3V2TrackNumber;

	BOOL		m_bFileOpenRecursive;
	CIni		m_Ini;

	CUString		m_strIniFileName;
	CUString		m_strProfileName;

	BOOL		m_bCDDBWriteAsDosFile;

	BOOL		m_bCDPlayDigital;
	BOOL		m_bRecordAddSeqNr;
	INT			m_bRecordEncoderType;

	CUString		m_strLanguage;

	//Status notification settings
	BOOL		m_bUseStatusServer;
	CUString		m_strStatusServer;
	int			m_nStatusServerPort;
	
public:
	CConfig();								// standard constructor
//	CConfig(const CConfig& newConfig);		// copy constructor

//	CConfig& operator=(const CConfig& newConfig);   // copy constructor


	CUString GetPlayDir() const;
	void	SetPlayDir(CUString& strValue)	{m_strPlayDir=strValue;}

	CUString GetMP3OutputDir() const;
	void	SetMP3OutputDir(CUString& strValue)	{ m_strMP3OutputDir=strValue; }

	CUString GetCnvOutputDir() const;
	void	SetCnvOutputDir(CUString& strValue)	{ m_strCnvOutputDir=strValue; }

	CUString GetRecOutputDir() const;
	void	SetRecOutputDir(CUString& strValue)	{ m_strRecOutputDir=strValue; }

	CUString GetWAVInputDir() const;
	void	SetWAVInputDir(CUString& strValue)	{ m_strWAVInputDir=strValue; }

	CUString GetMP3InputDir() const;
	void	SetMP3InputDir(CUString& strValue)	{ m_strMP3InputDir=strValue; }

	CUString GetTempDir() const;
	void	SetTempDir(CUString& strValue)		{ m_strTempDir=strValue; }

	BOOL	GetNormTrack()	const			{return m_bNormTrack;}
	void	SetNormTrack(BOOL nValue)		{m_bNormTrack=nValue;}

	BOOL	GetRiffWav()	const			{return m_bRiffWav;}
	void	SetRiffWav(BOOL nValue)			{m_bRiffWav=nValue;}

	BOOL	GetRetainWavFile()	const		{return m_bRetainWavFile;}
	void	SetRetainWavFile(BOOL nValue)	{m_bRetainWavFile=nValue;}

	BOOL	GetWinEncoder()	const			{return m_bWinEncoder;}
	void	SetWinEncoder(BOOL nValue)			{m_bWinEncoder=nValue;}

	CUString GetAppPath() const				{return m_strAppPath;}
	CUString GetVersion() const				{return m_strVersion;}

	CUString GetCDDBPath() const;
	void	SetCDDBPath(CUString& strValue)	{m_strCDDBPath=strValue;}
	
	CUString GetWinampDBPath() const;
	void	SetWinampDBPath(CUString& strValue)	{m_strWinampDBPath=strValue;}

	BOOL	GetCDDBLongDirNames() const		{return m_bCDDBLongDirNames;}
	void	SetCDDBLongDirNames(BOOL nValue){m_bCDDBLongDirNames=nValue;}

	int		GetCDDBProxyPort() const		{return m_nCDDBProxyPort;}
	void	SetCDDBProxyPort(int nValue)	{m_nCDDBProxyPort=nValue;}

	int		GetCDDBTimeOut() const			{return m_nCDDBTimeOut;}
	void	SetCDDBTimeOut(int nValue)		{m_nCDDBTimeOut=nValue;}
	
	int		GetCDDBUseProxy() const			{return m_nCDDBUseProxy;}
	void	SetCDDBUseProxy(int nValue)		{m_nCDDBUseProxy=nValue;}

	int		GetCDDBAutoConnect() const			{return m_bCDDBAutoConnect;}
	void	SetCDDBAutoConnect(BOOL nValue)		{m_bCDDBAutoConnect=nValue;}

	CUString GetEmailAddress() const				{return m_strEmailAddress;}
	void	SetEmailAddress(CUString& strValue)	{m_strEmailAddress=strValue;}

	CUString GetProxyAddress() const				{return m_strProxyAddress;}
	void	SetProxyAddress(CUString& strValue)	{m_strProxyAddress=strValue;}

	BOOL	GetM3UPlayList()	const			{return m_bM3UPlayList;}
	void	SetM3UPlayList(BOOL nValue)		{m_bM3UPlayList=nValue;}

	BOOL	GetPLSPlayList()	const			{return m_bPLSPlayList;}
	void	SetPLSPlayList(BOOL nValue)		{m_bPLSPlayList=nValue;}

	BOOL	GetDumpToc()	const			{return m_bDumpToc;}

	void	Load();
	void	Save();

	CUString	GetFileFormat() const			{return m_strFileFormat;}
	void	SetFileFormat(CUString strValue)	{m_strFileFormat=strValue;}

	CUString	GetPlsFileFormat() const			{return m_strPlsFileFormat;}
	void	SetPlsFileFormat(CUString strValue)	{m_strPlsFileFormat=strValue;}

	int		GetLocalCDDBType() const			{return m_nLocalCDDBType;}
	void	SetLocalCDDBType(int nValue)		{m_nLocalCDDBType=nValue;}

	BOOL	GetDeleteWAV()	const				{return m_bDelWavAfterConv;}
	void	SetDeleteWAV(BOOL nValue)			{m_bDelWavAfterConv=nValue;}

	BOOL	GetSaveToCDPlayer()	const			{return m_bSaveToCDPlayer;}
	void	SetSaveToCDPlayer(BOOL nValue)		{m_bSaveToCDPlayer=nValue;}

	BOOL	GetSaveToLocCDDB()	const			{return m_bSaveToLocCDDB;}
	void	SetSaveToLocCDDB(BOOL nValue)		{m_bSaveToLocCDDB=nValue;}


	int		GetLowNormLevel() const				{return m_nLowNormLevel;}
	int		GetHighNormLevel() const			{return m_nHighNormLevel;}
	int		GetLNormFactor() const				{return m_nLNormFactor;}
	int		GetHNormFactor() const				{return m_nHNormFactor;}

	void	SetLowNormLevel(int nValue)			{m_nLowNormLevel=nValue;}
	void	SetHighNormLevel(int nValue)		{m_nHighNormLevel=nValue;}
	void	SetLNormFactor(int nValue)			{m_nLNormFactor=nValue;}
	void	SetHNormFactor(int nValue)			{m_nHNormFactor=nValue;}

	void	SetEncoderType(int nValue)			{m_nEncoderType=nValue;}
	int		GetEncoderType() const				{return m_nEncoderType;}

	CUString GetExtEncPath() const				{return m_strExtEncPath;}
	void	SetExtEncPath(CUString& strValue)	{m_strExtEncPath=strValue;}

	CUString GetExtEncOpts() const				{return m_strExtEncOpts;}
	void	SetExtEncOpts(CUString& strValue)	{m_strExtEncOpts=strValue;}

	CUString GetTagPictureComment() const				{return m_strTagPictureComment;}
	void	SetTagPictureComment(CUString& strValue)	{m_strTagPictureComment=strValue;}

	CUString GetTagPictureFile() const				{return m_strTagPictureFile;}
	void	SetTagPictureFile(CUString& strValue)	{m_strTagPictureFile=strValue;}

	BOOL    GetAddTagPicture()    const {return m_bAddTagPicture;}
	void    SetAddTagPicture(BOOL nValue)   {m_bAddTagPicture=nValue;}

	CUString GetCDDBProxyUser() const				{return m_strCDDBProxyUser;}
	void	SetCDDBProxyUser(CUString& strValue)		{m_strCDDBProxyUser=strValue;}

	CUString GetCDDBProxyPassword() const			{return m_strCDDBProxyPassword;}
	void	SetCDDBProxyPassword(CUString& strValue)	{m_strCDDBProxyPassword=strValue;}

	int		GetCDDBUseAuthentication() const	{return m_nCDDBUseAuthentication;}
	void	SetCDDBUseAuthentication(int nValue){m_nCDDBUseAuthentication=nValue;}

	void	SetThreadPriority(int nValue)			{m_nThreadPriority=nValue;}
	int		GetThreadPriority() const				{return m_nThreadPriority;}

	int		GetFNBufferSize() const				{return m_nFNBufferSize;}
	void	SetFNBufferSize(BOOL nValue)		{m_nFNBufferSize=nValue;}

	int		GetCDDBSubmitVia() const			{return m_nCDDBSubmitVia;}
	void	SetCDDBSubmitVia(BOOL nValue)		{m_nCDDBSubmitVia=nValue;}

	int		GetID3Version() const				{return m_nID3Version;}
	void	SetID3Version(int nValue)			{m_nID3Version=nValue;}

	CUString GetID3Comment() const				{return m_strID3Comment;}

	CUString GetID3EncodedBy() const				{return m_strID3EncodedBy;}
	void	SetID3EncodedBy(CUString& strValue)	{m_strID3EncodedBy=strValue;}

	void	SetID3Comment(CUString& strValue)	{m_strID3Comment=strValue;}

	CUString GetCDDBESubmitAddr() const				{return m_strCDDBESubmitAddr;}
	void	SetCDDBESubmitAddr(CUString& strValue)	{m_strCDDBESubmitAddr=strValue;}

	CUString GetCDDBHSubmitAddr() const				{return m_strCDDBHSubmitAddr;}
	void	SetCDDBHSubmitAddr(CUString& strValue)	{m_strCDDBHSubmitAddr=strValue;}

	CUString GetCDDBESubmitServer() const			{return m_strCDDBESubmitServer;}
	void	SetCDDBESubmitServer(CUString& strValue)	{m_strCDDBESubmitServer=strValue;}

	CUString GetRILC() const							{return m_strRILC;}
	void	SetRILC(CUString& strValue)				{m_strRILC=strValue;}

	BOOL    GetAutoRip()    const					{return m_bAutoRip;}
	void    SetAutoRip(BOOL nValue)					{m_bAutoRip=nValue;}

	CHAR	GetSplitTrackChar() const				{return (CHAR)m_cSplitTrackChar;}
	void	SetSplitTrackChar(CHAR cValue)			{m_cSplitTrackChar=cValue;}

	BOOL	GetSplitTrackName()	const				{return m_bSplitTrackName;}
	void	SetSplitTrackName(BOOL nValue)			{m_bSplitTrackName=nValue;}

	BOOL	GetEjectWhenFinished()	const			{return m_bEjectWhenFinished;}
	void	SetEjectWhenFinished(BOOL nValue)		{m_bEjectWhenFinished=nValue;}

	BOOL	GetSelectAllCDTracks()	const			{return m_bSelectAllCDTracks;}
	void	SetSelectAllCDTracks(BOOL nValue)		{m_bSelectAllCDTracks=nValue;}

	int		GetOverwriteExisting() const			{return m_nOverwriteExisting;}
	void	SetOverwriteExisting(int nValue)		{m_nOverwriteExisting=nValue;}

	int		GetPCopyEncType() const			{return m_nPCopyEncType;}
	void	SetPCopyEncType(int nValue)		{m_nPCopyEncType=nValue;}

	BOOL	GetFileOpenRecursive()	const			{return m_bFileOpenRecursive;}
	void	SetFileOpenRecursive(BOOL nValue)		{m_bFileOpenRecursive=nValue;}

	BOOL	GetAutoShutDown()	const					{ return m_bAutoShutDown; }
	void	SetAutoShutDown(BOOL nValue)				{ m_bAutoShutDown=nValue; }

	BOOL	GetKeepDirLayout()	const					{ return m_bKeepDirLayout; }
	void	SetKeepDirLayout(BOOL nValue)				{ m_bKeepDirLayout=nValue; }

	int		GetRecordingDevice() const					{ return m_nRecordingDevice; }
	void	SetRecordingDevice(int nValue)				{ m_nRecordingDevice=nValue; }

	ID3TRKNRTTYPE GetID3V2TrackNumber() const			{ return m_nID3V2TrackNumber; }
	void	SetID3V2TrackNumber(ID3TRKNRTTYPE nValue)	{ m_nID3V2TrackNumber=nValue; }

	CUString	GetIniFileName() const { 
        CUString returnString ( GetAppPath() );
        returnString += _W( "\\" );
        returnString +=  m_strIniFileName; 
        return returnString;
    }
	CUString	SetIniFileName( const CUString strIniName ) { m_strIniFileName = strIniName; }

	CUString	GetProfileName() const { return m_strProfileName; }
	void	SetProfileName( const CUString strProfileName ) { m_strProfileName = strProfileName; }

	BOOL	GetCDDBWriteAsDosFile()	const			{ return m_bCDDBWriteAsDosFile; }
	void	SetCDDBWriteAsDosFile(BOOL nValue)		{ m_bCDDBWriteAsDosFile = nValue; }

	BOOL	GetCDPlayDigital()	const				{ return m_bCDPlayDigital; }
	void	SetCDPlayDigital(BOOL nValue)			{ m_bCDPlayDigital = nValue; }

	BOOL	GetRecordAddSeqNr()	const				{ return m_bRecordAddSeqNr; }
	void	SetRecordAddSeqNr(BOOL nValue)			{ m_bRecordAddSeqNr = nValue; }

	INT		GetRecordEncoderType()	const			{ return m_bRecordEncoderType; }
	void	SetRecordEncoderType(INT nValue)		{ m_bRecordEncoderType = nValue; }


	CIni&	GetIni() { return m_Ini; }

	void	SelectProfile( const CUString& strProfileName );

	CUString	GetLanguage() const { return m_strLanguage; }
	void	SetLanguage( const CUString strLanguage ) { m_strLanguage = strLanguage; }

	// Status notification settings
	BOOL	GetUseStatusServer()	const					{ return m_bUseStatusServer; }
	void	SetUseStatusServer(BOOL nValue)				{ m_bUseStatusServer=nValue; }

	CUString	GetStatusServer() const { return m_strStatusServer; }
	void	SetStatusServer( const CUString strStatusServer ) { m_strStatusServer = strStatusServer; }

	int 	GetStatusServerPort() const					{ return m_nStatusServerPort; }
	void	SetStatusServerPort(int nValue)				{ m_nStatusServerPort=nValue; }
	
    void    SaveCDRomSettings();
    void    LoadCDRomSettings();

    int GetTransportLayer();
    void SetTransportLayer( int transportLayer );

    int GetCDRipDebugLevel() const { 
            int debugLevel = 0;
            CUStringConvert strCnv;
            ::GetPrivateProfileInt( _T( "Debug" ), _T( "CDRip" ), 0, strCnv.ToT( m_strIniFileName ) );
            return debugLevel;}

private:
	CUString CConfig::FormatDirName( const CUString& strDir ) const;
};


extern CConfig g_config;
extern CGenreTable g_GenreTable;

#define GENRE_TABLE_FILE_NAME ( g_config.GetCDDBPath() + _W( "CDexGenres.txt" ) )

#endif
