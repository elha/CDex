/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 - 2007 Albert L. Faber
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
#include "ISndStreamWinAmp.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "ISndStreamWinAmp" ) );


static int 	g_nOutSampleRate = 0;
static int g_nNumChannels = 0;
static int g_nBitsPerSample = 0;
static int g_nBufferLenMs = 0;
static int g_nPreBufferMs = 0;
static int g_nBitRate = 128000;

volatile static int g_bOutOpened = FALSE;

static Out_Module g_OutModule;
static char g_lpszOutModuleName[] = { "CDex WAV Output Emulator" };
static In_Module myInModule;

volatile static int g_bFinished = FALSE;
volatile static int g_bInitialized = FALSE;

CEvent	g_eFinished( TRUE, TRUE );

#define WAPI_FIFO_NUM_BUFFERS 32
#define WAPI_FIFO_BUFFER_SIZE 8200

typedef struct WAPI_FIFO_STRUCT_TAG
{
	INT		nBytesWritten;
	BYTE	pbtBuffer[ WAPI_FIFO_BUFFER_SIZE ];
} WAPI_FIFO;


WAPI_FIFO	g_pFifo[WAPI_FIFO_NUM_BUFFERS];
CMutex		g_mFifoAcces;
CEvent		g_eFifoNotFull( TRUE, TRUE );
CEvent		g_eFifoNotEmpty( TRUE, TRUE );
int			g_nFifoFreeBuffers = WAPI_FIFO_NUM_BUFFERS;
int			g_nFifoReadIdx = 0;
int			g_nFifoWriteIdx = 0;
int			g_nFifoTotalBytes = 0;

vector <WINAMPPLUGINPROP> gs_vWinAmpProps;

CUString	g_strWinampExt( _T("") );

typedef In_Module*	(*WINAMPGETINMODULE2)( void );
typedef Out_Module*	(*WINAMPGETOUTMODULE)( void );

CUString GetWinampFileNameExt()
{
	return g_strWinampExt;
}

void WinampPlugInFinished()
{
	g_bFinished = TRUE;

	g_eFinished.SetEvent();
}

int GetNumWinampPlugins()
{
	return gs_vWinAmpProps.size();
}

CUString GetWinampPluginInfo( int i )
{
	CUString strRet( _W("") );

	if ( i< (int)gs_vWinAmpProps.size() )
	{
		strRet = CUString( gs_vWinAmpProps[i].pModule->description, CP_UTF8);
	}
	return strRet;
}

CUString GetWinampPluginFileName( int i )
{
	CUString strRet( _T("") );

	if ( i< (int)gs_vWinAmpProps.size() )
	{
		strRet = gs_vWinAmpProps[i].strFileName;
	}
	return strRet;
}

void WinampPluginConfigure( int i, HWND hWnd )
{
	if ( i< (int)gs_vWinAmpProps.size() )
	{
		gs_vWinAmpProps[i].pModule->Config( hWnd );
	}
}

void WinampPluginAbout( int i, HWND hWnd )
{
	if ( i< (int)gs_vWinAmpProps.size() )
	{
		gs_vWinAmpProps[i].pModule->About( hWnd );
	}
}


void TestOutputPlug( HWND hWnd )
{
	WINAMPPLUGINPROP	tmpProp;
    WIN32_FIND_DATA		sFF;
    HANDLE				hFind = NULL;
	CUString			strPluginDir( g_config.GetAppPath() + _W( "\\Plugins\\out_*.dll" ) );

	LTRACE2( _T( "Scanning for plugins using mask %s" ) , strPluginDir );

    CUStringConvert strCnv;

	hFind = FindFirstFile( strCnv.ToT( strPluginDir ), &sFF);

	g_strWinampExt = _T("");

	// setup the Output module Window Handle
	g_OutModule.hMainWindow = hWnd;

	// setup the Output Module Instance Handle
	g_OutModule.hDllInstance = NULL;

	while ( NULL != hFind &&
			hFind != INVALID_HANDLE_VALUE )
	{

		if ( ! ( sFF.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) 
		{
            CUStringConvert strCnv;
            CUString strFullName( g_config.GetAppPath() + _W( "\\Plugins\\" ) + CUString( sFF.cFileName ) );


			LTRACE2( _T( "Getting plugin %s" ), strCnv.ToT( strFullName ));

			tmpProp.hDll = NULL;
		
			tmpProp.hDll = CDexLoadLibrary( strFullName );
			if ( tmpProp.hDll )
			{


				WINAMPGETOUTMODULE pGetMod = (WINAMPGETOUTMODULE)
					GetProcAddress(	tmpProp.hDll,
									"winampGetOutModule" );

				// call "winampGetInModule" DLL function
				Out_Module* pOutModule = pGetMod();
			}

			// scan for next file
            if ( NULL != hFind && !FindNextFile( hFind, &sFF) ) 
			{
                FindClose( hFind );
                hFind = NULL;
            }

		}
	}
}

void InitWinAmpPlugins( HWND hWnd )
{
	WINAMPPLUGINPROP	tmpProp;
    WIN32_FIND_DATA		sFF;
    HANDLE				hFind = NULL;
	CUString			strPluginDir( g_config.GetAppPath() + _W( "\\Plugins\\in_*.dll" ) );

	LTRACE2( _T( "Scanning for plugins using mask %s"), strPluginDir );

    CUStringConvert strCnv;

	hFind = FindFirstFile( strCnv.ToT( strPluginDir ), &sFF);

	g_strWinampExt = _W("");

	// setup the Output module Window Handle
	g_OutModule.hMainWindow = hWnd;

	// setup the Output Module Instance Handle
	g_OutModule.hDllInstance = NULL;

	while ( NULL != hFind &&
			hFind != INVALID_HANDLE_VALUE )
	{

		if ( ! ( sFF.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) 
		{

            CUStringConvert strCnv;
            CUString strFullName( g_config.GetAppPath() + _W( "\\Plugins\\" ) + CUString( sFF.cFileName ) );

            LTRACE2( _T( "Getting plugin %s" ), strFullName );

			tmpProp.hDll = NULL;
		
			tmpProp.hDll = CDexLoadLibrary( strFullName );

			if ( tmpProp.hDll )
			{


				WINAMPGETINMODULE2 pGetMod = (WINAMPGETINMODULE2)
					GetProcAddress(	tmpProp.hDll,
									"winampGetInModule2" );

				// call "winampGetInModule2" DLL function
				tmpProp.pModule = pGetMod();

				// set the file name
				tmpProp.strFileName = sFF.cFileName;

				// setup window handler
				tmpProp.pModule->hMainWindow = hWnd;
				tmpProp.pModule->hDllInstance = tmpProp.hDll;

				tmpProp.pModule->Init(  );
				tmpProp.strExt = CUString( tmpProp.pModule->FileExtensions, CP_UTF8 );
				
				
				if ( 0 != tmpProp.pModule->UsesOutputPlug )
				{
					if ( !g_strWinampExt.IsEmpty() )
						g_strWinampExt += _W( ";" );

					g_strWinampExt+= tmpProp.strExt;

					LTRACE2( _T( "Adding plugin %s to list" ), sFF.cFileName );

					gs_vWinAmpProps.push_back( tmpProp );
				}
				else
				{
					tmpProp.pModule->Quit(  );
					LTRACE2( _T( "Plugin (%s) NOT added, since it does not use the output plugin" ), sFF.cFileName );
				}
			}

			// scan for next file
            if ( NULL != hFind && !FindNextFile( hFind, &sFF) ) 
			{
                FindClose( hFind );
                hFind = NULL;
            }

		}
	}
}

void DeInitWinAmpPlugins(  )
{
	int i = 0;

	// free all the input DLLs
	for ( i=0; i < (int)gs_vWinAmpProps.size(); i++ )
	{
		WINAMPPLUGINPROP	tmpProp = gs_vWinAmpProps[i];
		if ( NULL != tmpProp.hDll )
		{
			tmpProp.pModule->Quit( );
			FreeLibrary( tmpProp.hDll );
		}
	}

	// remove all elements
	gs_vWinAmpProps.clear();
}

// CONSTRUCTOR
ISndStreamWinAmp::ISndStreamWinAmp()
	:ISndStream()
{
	SetFileExtention( _T( "wav" ) );
	m_dwSamples=0;
	m_dwTotalFileSize = 0;
	m_dwCurrentFilePos = 0;

	memset( &g_OutModule, 0 , sizeof( g_OutModule ) );
}

// DESRUCTOR
ISndStreamWinAmp::~ISndStreamWinAmp()
{
	CloseStream();
}

void SetInfo( int bitrate, int srate, int stereo, int synched )
{
	LTRACE2( _T( "SetInfo %d %d %d %d" ), bitrate, srate, stereo, synched );

	if ( bitrate > 0 )
	{
		if ( bitrate < 1000 )
		{	
			g_nBitRate = bitrate * 1000;
		}
		else
		{
			g_nBitRate = bitrate ;
		}

	}
}


void WMIN_SAVSAInit( int maxlatency_in_ms, int srate )
{
	LTRACE2( _T( "SAVSAInit %d %d" ), maxlatency_in_ms, srate );
	// call once in Play(). maxlatency_in_ms should be the value returned from outMod->Open()
}

// call after opening audio device with max latency in ms and samplerate
void WMIN_SAVSADeInit()
{
	LTRACE2( _T( "SAVSADeInit" ) );
	// call in Stop()
}


// simple vis supplying mode
void SAAddPCMData( void *PCMData, int nch, int bps, int timestamp )
{
	LTRACE2( _T( "SAAddPCMData %08x %d %d %d" ), PCMData, nch, bps, timestamp );
}

// advanced vis supplying mode, only use if you're cool. Use SAAddPCMData for most stuff.
int SAGetMode()
{
	LTRACE3( _T( "SAGetMode" ) );
	// gets csa (the current type (4=ws,2=osc,1=spec))
	// use when calling SAAdd()
	return 1;
}

void SAAdd( void *data, int timestamp, int csa)
{
	LTRACE3( _T( "SAAdd %08x %d %d" ), data, timestamp, csa );
}


// vis stuff (plug-in)
// simple vis supplying mode
void VSAAddPCMData( void *PCMData, int nch, int bps, int timestamp )
{
	LTRACE3( _T( "VSAAddPCMData %08x %d %d %d" ), PCMData, nch, bps, timestamp );
	// sets the vis data directly from PCM data
	// quick and easy way to get vis working :)
	// needs at least 576 samples :)
}

// advanced vis supplying mode, only use if you're cool. Use VSAAddPCMData for most stuff.
int VSAGetMode( int *specNch, int *waveNch )
{
	LTRACE3( _T( "VSAAddPCMData %08x %08x " ), specNch, waveNch );
	// use to figure out what to give to VSAAdd
	return 0;
}

void VSAAdd( void *data, int timestamp )
{
	// filled in by winamp, called by plug-in
	LTRACE3( _T( "VSAAdd %08x %d" ), data, timestamp );
}


// call this in Play() to tell the vis plug-ins the current output params. 
void VSASetInfo(int nch, int srate)
{
	LTRACE3( _T( "VSASetInfo %d %d"), nch, srate );
}

// dsp plug-in processing: 
// (filled in by winamp, called by input plug)
// returns 1 if active (which means that the number of samples returned by dsp_dosamples
// could be greater than went in.. Use it to estimate if you'll have enough room in the
// output buffer
int dsp_isactive() 
{
	LTRACE3( _T( "dsp_isactive" ) );
	return 0;
}

// returns number of samples to output. This can be as much as twice numsamples. 
// be sure to allocate enough buffer for samples, then.
int dsp_dosamples( short int *samples, int numsamples, int bps, int nch, int srate )
{
	LTRACE3( _T( "dsp_dosamples %08x %d %d %d %d"), samples, numsamples, bps, nch, srate );
	return 0;
}

// eq stuff
// 0-64 each, 31 is +0, 0 is +12, 63 is -12. Do nothing to ignore.
void EQSet( int on, char data[10], int preamp)
{
	LTRACE2( _T( "EQSet %d %d %d %d" ), on, data[0], data[1], data[2] );
}

void OutConfig( HWND hWnd )
{
	LTRACE2( _T( "OutConfig %d" ), hWnd );
}

void OutAbout( HWND hWnd )
{
	LTRACE2( _T( "OutAbout %d" ), hWnd );
}

void OutInit()
{
	LTRACE2( _T( "OutInit" ) );
}

void OutQuit()
{
	LTRACE2( _T( "OutQuit" ) );
}


int OutOpen( int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms )
{
	LTRACE2(	_T( "OutOpen %d %d %d %d %d" ),
				samplerate, 
				numchannels, 
				bitspersamp, 
				bufferlenms, 
				prebufferms );

	g_nOutSampleRate = samplerate;
	g_nNumChannels = numchannels;
	g_nBitsPerSample = bitspersamp;
	g_nBufferLenMs = bufferlenms;
	g_nPreBufferMs = prebufferms;

	g_bOutOpened = TRUE;

	return 500;
}

void OutClose()
{
	LTRACE2( _T( "OutClose" ) );
}

int OutWrite(char *buf, int len)
{
	int nReturn = 1;
	// 0 on success. Len == bytes to write (<= 8192 always). buf is straight audio data. 
	// 1 returns not able to write (yet). Non-blocking, always.

	LTRACE2( _T( "OutWrite %08x %d" ), buf, len );

	// block for the MOD pluging ?
	// wait till buffer is available, or check if were finished yet
	HANDLE pWaitEvents[2] = { (HANDLE)g_eFifoNotFull, (HANDLE)g_eFinished }; 

	int nEventIdx = WaitForMultipleObjects( 2, pWaitEvents, FALSE, INFINITE ) - WAIT_OBJECT_0 ;

	// check if write buffer has triggered this event
	if ( 0 == nEventIdx )
	{
		// were sure that the write buffer is available !
		ASSERT( len <= WAPI_FIFO_BUFFER_SIZE );

		if ( 0 == len || NULL == buf )
		{
		}
		else
		{
			memcpy( g_pFifo[g_nFifoWriteIdx].pbtBuffer, buf, len );
			g_pFifo[g_nFifoWriteIdx].nBytesWritten = len;
		}


		// increase FIFO write index ( no FIFO lock need, since were
		// the only ones that are increasing this parameter
		g_nFifoWriteIdx = ( ( g_nFifoWriteIdx + 1 ) % WAPI_FIFO_NUM_BUFFERS );

		nReturn = 0;
	}
	else
	{
		g_pFifo[g_nFifoWriteIdx].nBytesWritten = 0;
		g_nFifoWriteIdx = ( ( g_nFifoWriteIdx + 1 ) % WAPI_FIFO_NUM_BUFFERS );

		nReturn = 1;
	}

	// lock access to FIFO parameters
	g_mFifoAcces.Lock();

	g_nFifoFreeBuffers--;


	ASSERT( g_nFifoFreeBuffers >= 0 );

	if ( 0 == nReturn )
	{
		g_nFifoTotalBytes+= len;
	}

	LTRACE(		_T( "WriteBuffer %d FreeBuffers on Write :%d bytes in fifo %d" ),
				g_nFifoWriteIdx, 
				g_nFifoFreeBuffers,
				g_nFifoTotalBytes );

	if ( 1 == g_nFifoFreeBuffers )
	{
		LTRACE( _T( "Reset g_eFifoNotFull event" ) );
		g_eFifoNotFull.ResetEvent();
	}

	if ( (WAPI_FIFO_NUM_BUFFERS -1 ) == g_nFifoFreeBuffers  )
	{
		LTRACE( _T( "Set g_eFifoNotEmpty event" ) );
		g_eFifoNotEmpty.SetEvent();
	}

	// unlock access to FIFO parameters
	g_mFifoAcces.Unlock();

	LTRACE2( _T( "OutWrite Writing %d bytes to buffer %d freebuffers %d" ), 
		len,
		g_nFifoWriteIdx,
		g_nFifoFreeBuffers );

	::Sleep(0);

	return nReturn;
}

int OutCanWrite()
{
	int nReturn = 0;

	if ( g_nFifoFreeBuffers > 1 )
		nReturn = WAPI_FIFO_BUFFER_SIZE;


	LTRACE2( _T( "CanWrite returns %d " ), nReturn );
	return nReturn;
}

void OutSetVolume( int volume )
{
	LTRACE2( _T( "OutSetVolume set to %d " ), volume );
	// volume is 0-255
}

void OutSetPan( int pan )
{
	LTRACE2( _T( "OutSetPan set to %d" ), pan );
	// pan is -128 to 128
}

void OutFlush( int t )
{
	LTRACE2( _T( "OutFlush" ) );

	// flushes buffers and restarts output at time t (in ms) 
	// (used for seeking)
}

int OutGetOutputTime()
{
	LTRACE3( _T( "OutGetOutputTime" ) );
	return 0;
}

int OutGetWrittenTime()
{
	LTRACE3( _T( "OutGetWrittenTime" ) );
	return 0;

	// returns time written in MS (used for synching up vis stuff)
}


int OutIsPlaying()
{
	int nReturn  = 0;

	if ( g_nFifoFreeBuffers != WAPI_FIFO_NUM_BUFFERS )
	{
		nReturn = 1;
	}

	LTRACE2( _T( "OutIsPlaying %d" ), nReturn );

	// non0 if output is still going or if data in buffers waiting to be
	// written (i.e. closing while IsPlaying() returns 1 would truncate the song
	return nReturn;

}


int OutPause(int pause) // returns previous pause state
{
	return 1-pause;
}


void ISndStreamWinAmp::FlushFIFO( )
{
	// lock access to FIFO parameters
	g_mFifoAcces.Lock();

	g_nFifoFreeBuffers = WAPI_FIFO_NUM_BUFFERS;
	g_nFifoReadIdx = 0;
	g_nFifoWriteIdx = 0;

	g_eFifoNotFull.SetEvent( );
	g_eFifoNotEmpty.ResetEvent( );

	g_nFifoTotalBytes = 0;

	// unlock access to FIFO parameters
	g_mFifoAcces.Unlock();
}


BOOL ISndStreamWinAmp::OpenStream( const CUString& strFileName )
{

	DWORD		i = 0;
	BOOL	bFound = FALSE;

	LTRACE2( _T( "ISndStreamWinAmp::OpenStream( %s )" ), strFileName );

	int nPos = 0;

	CUString strExt( _T( "wav" ) );

	g_bFinished = FALSE;
	g_bInitialized = FALSE;
	g_bOutOpened = FALSE;

	g_eFinished.ResetEvent();


	FlushFIFO( );


	nPos = strFileName.ReverseFind('.');

	if ( nPos >= 0  )
	{
		strExt = strFileName.Right( strFileName.GetLength() - nPos - 1  );
		strExt.MakeLower();
	}

	// Set the buffer size
	SetBufferSize( 9000 );

	SetFileName( strFileName );

	int nRet = 0;
	// loop through the available plugings 

	for ( i=0; i < gs_vWinAmpProps.size(); i++ )
	{
		m_WinAmpProp = gs_vWinAmpProps[i];


//		m_WinAmpProp.pModule->Init( );

		// check if the module can handle the file format
		if ( ( nRet = m_WinAmpProp.pModule->IsOurFile( (LPSTR)GetDosFileName( strFileName ) ) ) )
		{
			bFound = TRUE;
			break;
		}
/*
		if ( ( 0 == strExt.CompareNoCase( _T( "flac" ) ) ) && ( 0 == strExt.CompareNoCase(  m_WinAmpProp.strExt ) ) )
		{
			bFound = TRUE;
			break;
		}
*/
		CUString strPluginExt( m_WinAmpProp.pModule->FileExtensions, CP_UTF8 );
		strPluginExt.MakeLower();

		// check first on extention
		if ( strPluginExt.Find( strExt ) >=0 )
		{
			bFound = TRUE;
			break;
		}

//		m_WinAmpProp.pModule->Quit( );
	}

	if ( !bFound ) 
	{
		LTRACE2( _T( "ISndStreamWinAmp::OpenStream Failed, no proper plugin found!" ) );
		return FALSE;
	}

	if (   0 == m_WinAmpProp.pModule->UsesOutputPlug ) 
	{
		LTRACE2( _T( "ISndStreamWinAmp::OpenStream; Failed, does not use Output Plugin " ) );
		return FALSE;
	}

	LTRACE2( _T( "ISndStreamWinAmp::OpenStream found proper plugin ( %s )" ) , m_WinAmpProp.pModule->description );

	// setup proper function calls
	m_WinAmpProp.pModule->SetInfo = SetInfo;
	m_WinAmpProp.pModule->SAAddPCMData = SAAddPCMData;
	m_WinAmpProp.pModule->SAGetMode = SAGetMode;
	m_WinAmpProp.pModule->SAAdd = SAAdd;
	m_WinAmpProp.pModule->VSAAddPCMData = VSAAddPCMData;
	m_WinAmpProp.pModule->VSAGetMode = VSAGetMode;
	m_WinAmpProp.pModule->VSAAdd = VSAAdd;
	m_WinAmpProp.pModule->VSASetInfo = VSASetInfo;
	m_WinAmpProp.pModule->dsp_isactive = dsp_isactive;
	m_WinAmpProp.pModule->dsp_dosamples = dsp_dosamples;
	m_WinAmpProp.pModule->EQSet = EQSet;
	m_WinAmpProp.pModule->SAVSADeInit = WMIN_SAVSADeInit;
	m_WinAmpProp.pModule->SAVSAInit = WMIN_SAVSAInit;

	m_WinAmpProp.pModule->outMod = &g_OutModule;

	g_OutModule.version = OUT_VER;
	g_OutModule.description = g_lpszOutModuleName;
	g_OutModule.id = 33;

	g_OutModule.Config = OutConfig;
	g_OutModule.About = OutAbout;

	g_OutModule.Init = OutInit;
	g_OutModule.Quit = OutQuit;
	g_OutModule.Open = OutOpen;
	g_OutModule.Close = OutClose;
	g_OutModule.Write = OutWrite;
	g_OutModule.CanWrite = OutCanWrite;
	g_OutModule.IsPlaying = OutIsPlaying;
	g_OutModule.Pause = OutPause;
	g_OutModule.SetVolume = OutSetVolume;
	g_OutModule.SetPan = OutSetPan;
	g_OutModule.Flush = OutFlush;
	g_OutModule.GetOutputTime = OutGetOutputTime;
	g_OutModule.GetWrittenTime = OutGetWrittenTime;

	SetChannels( 2 );
	SetSampleRate( 44100 );
	SetBitRate( 176000 * 8 );
	
//	m_dwSamples=wfInfo.samples;

	m_dwTotalFileSize = 1000;
	m_dwCurrentFilePos = 0;


	// start the decoding thread
	if ( 0 != m_WinAmpProp.pModule->Play( (LPSTR)GetDosFileName( strFileName ) ) )
	{
		return FALSE;
	}

	// Wait till output device has been opened by the
	// input plugin
	while ( FALSE == g_bOutOpened )
	{
		::Sleep( 5 );
	}

	SetChannels( g_nNumChannels );
	SetSampleRate( g_nOutSampleRate );
	SetBitRate( g_nBitRate );

	g_bInitialized = TRUE;

	// return Success
	return TRUE;
}


DWORD ISndStreamWinAmp::GetTotalTime()
{
	// return total time in ms
	return 	m_WinAmpProp.pModule->GetLength();
}

DWORD ISndStreamWinAmp::GetCurrentTime()
{
	// return total time in ms
	return m_WinAmpProp.pModule->GetOutputTime();
}


DWORD ISndStreamWinAmp::Read(PBYTE pbData,DWORD dwNumBytes)
{
	int		nReturn = 0;
	LONG	lastcount = 0;

	LTRACE3(	_T( "ISndStreamWinAmp::Read dwNumBytes %d  CurrentFilePos %d TotalFileSize %d " ),
				dwNumBytes,
				m_dwCurrentFilePos,
				m_dwTotalFileSize );

	m_dwCurrentFilePos+= dwNumBytes;

	int nCurrentTime = m_WinAmpProp.pModule->GetOutputTime();
	int nTotalTime = m_WinAmpProp.pModule->GetLength();

	if ( nTotalTime )
	{
		int t = g_nOutSampleRate*g_nNumChannels;
		int	l = 0;
		int ms = g_nFifoTotalBytes;

		if ( t )
		{
			l=ms%t;
			ms /= t;
			ms *= 1000;
			ms += (l*1000)/t;

			if (g_nBitsPerSample == 16)
				ms/=2;
		}
		else
		{
			ms = 0;
		}

		SetPercent( (INT)( (double) (nCurrentTime - ms) * 100.0 / nTotalTime ) );
	}
	else
	{
		SetPercent( 0 );
	}



	// wait for incoming data
	HANDLE pWaitEvents[2] = { (HANDLE)g_eFifoNotEmpty, (HANDLE)g_eFinished }; 

	int nEventIdx = WaitForMultipleObjects( 2, pWaitEvents, FALSE, INFINITE ) - WAIT_OBJECT_0 ;

	if ( 1 == nEventIdx )
	{
		nReturn = 0;
	}
	else
	{

		nReturn = g_pFifo[g_nFifoReadIdx].nBytesWritten;

		memcpy( pbData, g_pFifo[g_nFifoReadIdx].pbtBuffer, nReturn );


		LTRACE2( _T( "ISndStreamWinAmp::Read getting %d bytes from buffer %d (freebuffers = %d)" ), 
			g_pFifo[g_nFifoReadIdx].nBytesWritten,
			g_nFifoReadIdx,
			g_nFifoFreeBuffers
			);

		// lock access to FIFO parameters
		g_mFifoAcces.Lock();

		if ( g_nFifoFreeBuffers < WAPI_FIFO_NUM_BUFFERS )
		{
			g_nFifoFreeBuffers++;
		}

		if ( WAPI_FIFO_NUM_BUFFERS == g_nFifoFreeBuffers )
		{
			LTRACE( _T( "Reset g_eFifoNotEmpty Event" ) );
			g_eFifoNotEmpty.ResetEvent();
		}

		if ( 2 == g_nFifoFreeBuffers )
		{
			LTRACE( _T( "Set g_eFifoNotFull Event" ) );
			g_eFifoNotFull.SetEvent();
		}

		g_nFifoTotalBytes-= nReturn;

		// unlock access to FIFO parameters
		g_mFifoAcces.Unlock();

		// increase FIFO read index pointer
		g_nFifoReadIdx = ( ( g_nFifoReadIdx + 1 ) % WAPI_FIFO_NUM_BUFFERS );

		LTRACE( _T( "FreeBuffers on Read :%d, bytes in fifo %d" ),
				g_nFifoFreeBuffers,
				g_nFifoTotalBytes );

	}

	// update the bitrate info
	SetBitRate( g_nBitRate );

	LTRACE2( _T( "ISndStreamWinAmp::Read returns %d" ), nReturn );

	return nReturn;
}


BOOL ISndStreamWinAmp::CloseStream()
{
	if ( FALSE == g_bFinished && TRUE == g_bInitialized )
	{
		g_bFinished = TRUE;
		m_WinAmpProp.pModule->Stop( );
	}

	SetPercent( 0 );
	g_bInitialized = FALSE;


	return TRUE;
}

__int64	ISndStreamWinAmp::Seek( __int64 ddwOffset, UINT nFrom )
{
	LONG lSeekPosition = 0;
	int nPercent=(int)ddwOffset;


	int nCurrentTime = m_WinAmpProp.pModule->GetOutputTime();
	int nTotalTime = m_WinAmpProp.pModule->GetLength();

	switch( nFrom )
	{
		case SEEK_PERCENT:
			if (nPercent<0 ) nPercent= 0;
			if (nPercent>99) nPercent= 99;

			lSeekPosition = (LONG) ( (int)ddwOffset / 100.0 * nTotalTime );

		break;
		case SEEK_TIME:
			lSeekPosition = (int)ddwOffset;
			break;
		default:
			ASSERT( FALSE );
		break;
	}

	if ( FALSE == g_bFinished && TRUE == g_bInitialized )
	{
		FlushFIFO();
		m_WinAmpProp.pModule->SetOutputTime( lSeekPosition );
		m_WinAmpProp.pModule->UnPause(  );
	}

	return lSeekPosition;
}


void ISndStreamWinAmp::Flush()
{
	if ( FALSE == g_bFinished && TRUE == g_bInitialized )
	{
		m_WinAmpProp.pModule->Pause();
		FlushFIFO();
		m_WinAmpProp.pModule->UnPause();
	}
}

void ISndStreamWinAmp::InfoBox( HWND hWnd )
{
	m_WinAmpProp.pModule->InfoBox( (LPSTR)GetDosFileName( GetFileName() ), hWnd );
}

void ISndStreamWinAmp::Pause()
{
	m_WinAmpProp.pModule->Pause();
}

