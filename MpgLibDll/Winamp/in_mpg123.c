/*
** Copyright (C) 2000 Albert L. Faber
**  
** partly copyrighted by by Naoki Shibata
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* system includes */
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <assert.h>
#include <math.h>
#include <string.h>

/* project includes */
#include "in_mpg123.h"
#include "httpstream.h"
#include "VbrTag.h"
#include "ConfigDlg.h"
#include "InfoDlg.h"

/* mpglib includes */
#include "config.h"
#include "mpg123.h"
#include "mpglib.h"
#include "in2.h"
#include "VbrTag.h"
#include "common.h"
#include "interface.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VERSIONSTRINGBASE "0.01"

#ifdef __ICL
#define VERSIONSTRING VERSIONSTRINGBASE"+"
#else
#define VERSIONSTRING VERSIONSTRINGBASE
#endif


#define WM_WA_MPEG_EOF WM_USER+2

/* local function prototypes */
void config( HWND );
void about( HWND );
void init();
void quit();
void getfileinfo( char *, char *, int * );
int infoDlg( char *, HWND );
int isourfile( char * );
int play( char * );
void pause();
void unpause();
int ispaused();
void stop();
int getlength();
int getoutputtime();
void setoutputtime( int );
void setvolume( int );
void setpan( int );
void eq_set( int, char data[10], int ) ;



In_Module mod = 
{
	IN_VER,
	"CDex mpg123 plugin ver " VERSIONSTRING,
	0,	// hMainWindow
	0,  // hDllInstance
	"MP3\0MP3 Audio File (*.MP3)\0RMP\0RIFF MP3 Audio File (*.RMP)\0",
	1,	// is_seekable
	1,  // uses output
	config,
	about,
	init,
	quit,
	getfileinfo,
	infoDlg,
	isourfile,
	play,
	pause,
	unpause,
	ispaused,
	stop,
	
	getlength,
	getoutputtime,
	setoutputtime,

	setvolume,
	setpan,

	0,0,0,0,0,0,0,0,0,
	0,0,

	eq_set,

	NULL,

	0
};


static char lastfn[MAX_PATH]; // currently playing file (used for getting info on the current file)
static int file_length;       // file length, in bytes
static double decode_pos_ms;     // current decoding position, in milliseconds
static int paused;            // are we paused?
static int seek_needed;       // if != -1, it is the point that the decode thread should seek to, in ms.
static char sample_buffer[576*4*2]; // sample buffer
static volatile int decoder_in_use = 0;
static int xvolume,xpan;

char *stream_buffer=NULL;
volatile int strbufp_s=0,strbufp_e=0,strbuf_eof=0;
int strbuflen;

plotting_data*	mpg123_pinfo		= NULL;


static void*	pBufferThread		= NULL;
static HANDLE	hInputFile			= INVALID_HANDLE_VALUE;
static BOOL		bKillDecodeThread	= FALSE;
static HANDLE	hDecodeThread		= INVALID_HANDLE_VALUE;

DWORD WINAPI __stdcall DecodeThread(void *b);  // the decode thread procedure
DWORD WINAPI __stdcall ReceiveThread(void *b); // the receive thread procedure

// Variable to estimate bitrate

int totalFramesize;
int nframe;

// Variables which keep state of config panel

id3tag current_id3;


VBRTAGDATA current_vbrtag;

typedef struct mp3info
{
	int nbytes;
	int	freq;
	int nch;
	int hpos;
	int hasVbrtag;
	int lsf;
	int bitrate;
	int isStream;
	double length;
} mp3info;

mp3info cur_mp3info;

static int head_check2(unsigned long head,mp3info *info)
{
	int lsf,srate,nch,lay,mpeg25,freq,mode;

    if ((head & 0xffe00000) != 0xffe00000) return FALSE;
    if (!((head>>17)&3)) return FALSE;
    if (((head>>12)&0xf) == 0xf) return FALSE;
    if (((head>>10)&0x3) == 0x3 ) return FALSE;
    //if ((head & 0xffff0000) == 0xfffe0000) return FALSE;

    if( head & (1<<20) ) {
      lsf = (head & (1<<19)) ? 0x0 : 0x1;
      mpeg25 = 0;
    } else {
      lsf = 1;
      mpeg25 = 1;
    }

	if(mpeg25) srate = 6 + ((head>>10)&0x3);
    else srate = ((head>>10)&0x3) + (lsf*3);

    mode  = ((head>>6)&0x3);
	freq  = freqs[srate];
    nch   = (mode == MPG_MD_MONO) ? 1 : 2;
	lay   = 4-((head>>17)&3);

	if (lay != 3) return FALSE;
	if (nch != info->nch) return FALSE;
	if (freq != info->freq) return FALSE;

    return TRUE;
}

// for streaming
int getmp3info2( mp3info *info )
{
	int i;
	int head;
	int lsf,srate;
	int bitrate_index,mode,nch,lay,extension,mpeg25,padding;
	int		nStart = 0;
	BOOL	bFound = FALSE;

	while ( FALSE == bFound )
	{
		// wait for ( the next) 4 bytes in the stream
		stream_buffer = httpStartOfStream( pBufferThread, nStart + 4 );

		// be sure the thread is not aborted
		if ( NULL == stream_buffer )
		{
			return 0;
		}

		// offset in stream
		stream_buffer += nStart;

		// copy header bytes
		*( 3 + (char *)&head ) = stream_buffer[ i     ];
		*( 2 + (char *)&head ) = stream_buffer[ i + 1 ];
		*( 1 + (char *)&head ) = stream_buffer[ i + 2 ];
		*( 0 + (char *)&head ) = stream_buffer[ i + 3 ];

		// check header
		lay = 4-( ( head>>17 ) & 3 );

		if (	strbuf_eof ||
				head_check( head, lay ) )
		{
			bFound = TRUE;
		}
		else
		{
			nStart+= 4;
		}

		// check if we reached the end of the stream buffer
		if ( nStart >= CIRCULAR_BUFFER_SIZE )
		{
			return 0;
		}
	}


	strbufp_s = i;

    if( head & (1<<20) ) {
      lsf = (head & (1<<19)) ? 0x0 : 0x1;
      mpeg25 = 0;
    } else {
      lsf = 1;
      mpeg25 = 1;
    }

    bitrate_index = ((head>>12)&0xf);
	padding       = ((head>>9)&0x1);
    mode          = ((head>>6)&0x3);
    nch           = (mode == MPG_MD_MONO) ? 1 : 2;
	lay           = 4-((head>>17)&3);
	extension     = ((head>>8)&0x1);

	if(mpeg25) srate = 6 + ((head>>10)&0x3);
    else srate = ((head>>10)&0x3) + (lsf*3);

	if (lay != 3) return 0;

	info->isStream = 1;
	info->nbytes = 0;
	info->hpos = 0;
	info->length = 0;
	info->lsf  = lsf;
	info->freq = freqs[srate];
	info->nch  = nch;
	info->hasVbrtag = 0;
	info->bitrate = tabsel_123[lsf][lay-1][bitrate_index]*1000;
	
	return 1;
}

int getmp3info(	char*		fn,
				mp3info*	info,
				id3tag*		id3,
				VBRTAGDATA*	vbr )
{
	unsigned long	head;
	unsigned long	len;
	int				lsf;
	int				srate;
	int				bitrate_index;
	int				mode;
	int				nch;
	int				lay;
	int				extension;
	int				mpeg25;
	int				padding;
	int				hpos;
	int				i;
	HANDLE			hInputFile = INVALID_HANDLE_VALUE;

	hInputFile = CreateFile(	fn,
								GENERIC_READ,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL );

	// check result operning the file
	if ( hInputFile == INVALID_HANDLE_VALUE ) 
	{
		return 0;
	}

	info->isStream = 0;
	info->nbytes = GetFileSize( hInputFile, NULL );

	hpos = 0;

	// skip ID3v2 tag
	{
		unsigned char c1,c2,c3,c4;

		ReadFile( hInputFile, &c1, 1, &len, NULL );
		ReadFile( hInputFile, &c2, 1, &len, NULL );
		ReadFile( hInputFile, &c3, 1, &len, NULL );
		ReadFile( hInputFile, &c4, 1, &len, NULL );

		if (c1 == 'I' && c2 == 'D' && c3 == '3' && c4 == 2)
		{
			SetFilePointer( hInputFile,6,NULL,FILE_BEGIN );
			ReadFile( hInputFile, &c1, 1, &len, NULL );
			ReadFile( hInputFile, &c2, 1, &len, NULL );
			ReadFile( hInputFile, &c3, 1, &len, NULL );
			ReadFile( hInputFile, &c4, 1, &len, NULL );

			// calculate tag length
			hpos = c1 * 2097152 + c2 * 16384 + c3 * 128 + c4;
		}
	}

	for( i = 0; i < 65536; i++ )
	{
		SetFilePointer( hInputFile, hpos+i, NULL, FILE_BEGIN );

		ReadFile(hInputFile, 3 + (char *)&head, 1, &len, NULL );
		ReadFile(hInputFile, 2 + (char *)&head, 1, &len, NULL );
		ReadFile(hInputFile, 1 + (char *)&head, 1, &len, NULL );
		ReadFile(hInputFile, 0 + (char *)&head, 1, &len, NULL );

		lay = 4-( ( head>>17 ) & 3 );

		if (	len == 0 || 
				head_check( head, lay ) )
		{
			break;
		}
	}

	if ( len == 0 || i == 65536 )
	{
		CloseHandle( hInputFile );
		hInputFile=INVALID_HANDLE_VALUE;
		return 0;
	}

	hpos += i;

	info->hpos = hpos;

	{
		info->nbytes -= hpos;

		// read ID3 tag

		SetFilePointer(hInputFile,-128,NULL,FILE_END);
		ReadFile(hInputFile,id3,128,&len,NULL);
		if (strncmp(id3->tag,"TAG",3) == 0) info->nbytes -= 128;
	}
	
	SetFilePointer(hInputFile,hpos,NULL,FILE_BEGIN);

    if( head & (1<<20) ) {
      lsf = (head & (1<<19)) ? 0x0 : 0x1;
      mpeg25 = 0;
    } else {
      lsf = 1;
      mpeg25 = 1;
    }

    bitrate_index = ((head>>12)&0xf);
	padding       = ((head>>9)&0x1);
    mode          = ((head>>6)&0x3);
    nch           = (mode == MPG_MD_MONO) ? 1 : 2;
	lay           = 4-((head>>17)&3);
	extension     = ((head>>8)&0x1);

	if( mpeg25 )
	{
		srate = 6 + ((head>>10)&0x3);
	}
    else
	{
		srate = ((head>>10)&0x3) + (lsf*3);
	}

	// read VBR tag
	SetFilePointer(hInputFile,hpos,NULL,FILE_BEGIN);

	info->lsf  = lsf;
	info->freq = freqs[srate];
	info->nch  = nch;
	
	{
		unsigned char buf[ sizeof( VBRTAGDATA ) + 36 ];

		ReadFile( hInputFile, &buf, sizeof( buf ), &len, NULL );

		if ( GetVbrTag( &current_vbrtag,buf ) )
		{
			int cur_bitrate = (int)(current_vbrtag.bytes*8/(current_vbrtag.frames*576.0*(lsf?1:2)/freqs[srate]));
			info->length = current_vbrtag.frames*576.0*(lsf?1:2)/freqs[srate];
			info->nbytes = current_vbrtag.bytes;
			info->hasVbrtag = 1;
			info->bitrate = cur_bitrate;
		} else {
			int framesize = tabsel_123[lsf][lay-1][bitrate_index]*144000/(freqs[srate]<<lsf)+padding;
			info->length = info->nbytes/framesize*576.0*(lsf?1:2)/freqs[srate];
			info->hasVbrtag = 0;
			info->bitrate = tabsel_123[lsf][lay-1][bitrate_index]*1000;
			//info->length = info->nbytes * 8 / info->bitrate;
		}
	}

	CloseHandle( hInputFile );

	return 1; 
}

int SeekPoint(unsigned char TOC[NUMTOCENTRIES], int file_bytes, float percent)
{
	/* interpolate in TOC to get file seek point in bytes */
	int a, seekpoint;
	float fa, fb, fx;

	if( percent < (float)0.0 )   percent = (float)0.0;
	if( percent > (float)100.0 ) percent = (float)100.0;

	a = (int)percent;
	if( a > 99 ) a = 99;
	fa = TOC[a];
	if( a < 99 ) {
		fb = TOC[a+1];
	} else {
		fb = (float)256.0;
	}

	fx = fa + (fb-fa)*(percent-a);

	seekpoint = (int)(((float)(1.0/256.0))*fx*file_bytes); 

	return seekpoint;
}

void reflect_setting(void)
{
	if (xch_enable) {
		mod.FileExtensions = "MP3\0MP3 Audio File (*.MP3)\0RMP\0RIFF MP3 Audio File (*.RMP)\0";
	} else {
		mod.FileExtensions = "\0\0";
	}

	if ( xch_dispavg && decoder_in_use )
		mod.SetInfo(cur_mp3info.bitrate/1000,-1,-1,-1);

	// FIXME
	//forceDecoding = xch_force;
}

void config_getinifn(In_Module *this_mod, char *ini_file)
{	// makes a .ini file in the winamp directory named "plugin.ini"
	char *p;
	GetModuleFileName(this_mod->hDllInstance,ini_file,MAX_PATH);
	p=ini_file+strlen(ini_file);
	while (p >= ini_file && *p != '\\') p--;
	if (++p >= ini_file) *p = 0;
	strcat(ini_file,"plugin.ini");
}


void init() {
	char ini_file[MAX_PATH];
	config_getinifn(&mod,ini_file);
	xch_enable  = GetPrivateProfileInt("CDex mpg123 plugin","Enable" ,  1,ini_file);
	xch_supzero = GetPrivateProfileInt("CDex mpg123 plugin","SupZero",  0,ini_file);
	xch_force   = GetPrivateProfileInt("CDex mpg123 plugin","Force"  ,  0,ini_file);
	xch_dispavg = GetPrivateProfileInt("CDex mpg123 plugin","DispAvg",  1,ini_file);
	xch_24bit   = GetPrivateProfileInt("CDex mpg123 plugin","Hibit"  ,  0,ini_file);
	xch_volume  = GetPrivateProfileInt("CDex mpg123 plugin","Volume" ,  0,ini_file);
	xch_reverse = GetPrivateProfileInt("CDex mpg123 plugin","Reverse",  0,ini_file);
	xch_invert  = GetPrivateProfileInt("CDex mpg123 plugin","Invert" ,  0,ini_file);
	xvolume     = GetPrivateProfileInt("CDex mpg123 plugin","XVolume",255,ini_file);
	xpan        = GetPrivateProfileInt("CDex mpg123 plugin","XPan"   ,  0,ini_file);
	xstrbuflen  = GetPrivateProfileInt("CDex mpg123 plugin","StrBuf" ,256,ini_file);
	GetPrivateProfileString("CDex mpg123 plugin","Format","%1 - %2",xed_title,255,ini_file);
	GetPrivateProfileString("CDex mpg123 plugin","Proxy" ,"",xed_proxy,255,ini_file);
	if (xstrbuflen <= 0) xstrbuflen = 256;

	reflect_setting();

	httpInit();

	// FIXME?
//	In_Module* (*_winampGetInModule2)(void);
}

void quit() {

	char string[ 32 ] = {'\0',};
	char ini_file[ MAX_PATH ] = {'\0',};

	config_getinifn( &mod, ini_file );

	wsprintf(string,"%d",xch_enable);
	WritePrivateProfileString("CDex mpg123 plugin","Enable",string,ini_file);

	wsprintf(string,"%d",xch_supzero);
	WritePrivateProfileString("CDex mpg123 plugin","SupZero",string,ini_file);

	wsprintf(string,"%d",xch_force);
	WritePrivateProfileString("CDex mpg123 plugin","Force",string,ini_file);

	wsprintf(string,"%d",xch_dispavg);
	WritePrivateProfileString("CDex mpg123 plugin","DispAvg",string,ini_file);

	wsprintf(string,"%d",xch_24bit);
	WritePrivateProfileString("CDex mpg123 plugin","Hibit",string,ini_file);

	wsprintf(string,"%d",xch_volume);
	WritePrivateProfileString("CDex mpg123 plugin","Volume",string,ini_file);

	wsprintf(string,"%d",xch_reverse);
	WritePrivateProfileString("CDex mpg123 plugin","Reverse",string,ini_file);

	wsprintf(string,"%d",xch_invert);
	WritePrivateProfileString("CDex mpg123 plugin","Invert",string,ini_file);

	wsprintf(string,"%d",xvolume);
	WritePrivateProfileString("CDex mpg123 plugin","XVolume",string,ini_file);

	wsprintf(string,"%d",xpan);
	WritePrivateProfileString("CDex mpg123 plugin","XPan",string,ini_file);

	wsprintf(string,"%d",xstrbuflen);
	WritePrivateProfileString("CDex mpg123 plugin","StrBuf",string,ini_file);

	WritePrivateProfileString("CDex mpg123 plugin","Format",xed_title,ini_file);
	WritePrivateProfileString("CDex mpg123 plugin","Proxy",xed_proxy,ini_file);

	httpShutdown();

}

void config( HWND hWndParent )
{
	INT nRet = IDCANCEL;
	
	nRet = DialogBox(	mod.hDllInstance, 
						MAKEINTRESOURCE( IDD_CONFIGDLG ), 
						hWndParent, 
						ConfigDlgProc ); 

	reflect_setting();
}

void about( HWND hwndParent )
{
	MessageBox(hwndParent,"CDex mpg123 plugin " VERSIONSTRING " for CDex\n"
				"http://cdexos.sourceforge.net\n","About mpg123 plugin",MB_OK);
}

int isourfile( char *fn )
{
	// set window handle
	httpSetHwnd( mod.hMainWindow );

	// initialize stream variables
    setHttpVars();

    return IsUrl( fn );
} 

void pause()
{
	paused = 1;
	mod.outMod->Pause( 1 );
}

void unpause()
{
	paused = 0;
	mod.outMod->Pause( 0 );
}

int ispaused()
{
	return paused;
}

int getoutputtime()
{ 
	return decode_pos_ms+(mod.outMod->GetOutputTime()-mod.outMod->GetWrittenTime()); 
}

void setoutputtime( int time_in_ms )
{ 
	seek_needed=time_in_ms;
}

void setvolume( int volume )
{
	xvolume = volume;
	mod.outMod->SetVolume( volume );
}

void setpan(int pan)
{
	xpan = pan;
	mod.outMod->SetPan( pan );
}

int infoDlg( char *fn, HWND hWndParent )
{
	mp3info		info;
	VBRTAGDATA	vbr;
	int			id = IDCANCEL;

	if (!getmp3info(fn,&info,&xediting_id3,&vbr)) return 0;
	
	xediting_id3fn = fn;


	
	id = DialogBox(	mod.hDllInstance, 
					MAKEINTRESOURCE( IDD_INFODLG ), 
					hWndParent, 
					InfoDlgProc ); 

	if ( id == IDOK && strcmp( lastfn, fn ) == 0 )
		current_id3 = xediting_id3;

	return 0;
}

int getlength()
{ 
	return cur_mp3info.length * 1000;
}

void eq_set(int on, char data[10], int preamp) 
{ 
}

int play( char* fn ) 
{ 
	int maxlatency;
	int thread_id;
	int bps = 16;
	static LPCTSTR proxy_bypass = "";
	static LPCTSTR Lversion = "CDex in_mpg123 " VERSIONSTRING;

	while( decoder_in_use )
	{
		Sleep(30);
	}

	// set window handle
	httpSetHwnd( mod.hMainWindow );

	// initialize the http stream vars
	setHttpVars();

	// no buffer created yet
	pBufferThread = NULL;

	// no file opened yet
	hInputFile = INVALID_HANDLE_VALUE;

	if ( IsUrl( fn ) )
	{
		// not seekable when getting the data from an http stream
        mod.is_seekable = FALSE;

		// check if we have to go through a proxy
		if ( '\0' != xed_proxy[0] )
		{
			// if so, set proxy address
			httpSetProxy( xed_proxy );
		}

		current_id3.tag[0] = '\0';
		
		// start the buffering
		if ( NULL == ( pBufferThread = httpStartBuffering( fn, TRUE ) ) )
		{
			MessageBox( mod.hMainWindow, "Connection failed" , "streaming error" , 0 );
            return -1;
		}

		if ( !getmp3info2( &cur_mp3info ) ||
			( maxlatency = mod.outMod->Open( cur_mp3info.freq,cur_mp3info.nch,bps, -1,-1)) < 0)
		{
			MessageBox(mod.hMainWindow,"Valid MP3 frame not found","streaming error",0);
		}

	}
	else
	{
		mod.is_seekable = 1;

		if ( ! getmp3info( fn, &cur_mp3info, &current_id3, &current_vbrtag ) )
		{
			return 1;
		}

		// create the input file handle
		hInputFile = CreateFile(	fn,
									GENERIC_READ,
									FILE_SHARE_READ | FILE_SHARE_WRITE,
									NULL,
									OPEN_EXISTING,
									FILE_ATTRIBUTE_NORMAL,
									NULL );

		// check file open result
		if ( INVALID_HANDLE_VALUE == hInputFile )
		{
			return 1;
		}

		file_length = cur_mp3info.nbytes;

		maxlatency = mod.outMod->Open(cur_mp3info.freq,cur_mp3info.nch,bps, -1,-1);
	
		if( maxlatency < 0 ) // error opening device
		{
			CloseHandle( hInputFile );
			hInputFile = INVALID_HANDLE_VALUE;
			return 1;
		}

		SetFilePointer( hInputFile, cur_mp3info.hpos, NULL, FILE_BEGIN );
	}

	strcpy( lastfn, fn );
	paused = 0;
	decode_pos_ms = 0;
	seek_needed = -1;

	mod.SetInfo(	cur_mp3info.bitrate / 1000,
					cur_mp3info.freq / 1000,
					cur_mp3info.nch,
					1 );

	// initialize vis stuff
	mod.SAVSAInit( maxlatency, cur_mp3info.freq );
	mod.VSASetInfo( cur_mp3info.freq, cur_mp3info.nch );

	// set the output plug-ins default volume
	mod.outMod->SetVolume( -666 );
	
	bKillDecodeThread = FALSE;

	hDecodeThread = (HANDLE) CreateThread(	NULL,
											0,
											(LPTHREAD_START_ROUTINE) DecodeThread,
											(void *) &bKillDecodeThread,
											0,
											(unsigned long *)&thread_id );

	SetThreadPriority( hDecodeThread, THREAD_PRIORITY_HIGHEST );

	return 0; 
}


void stop()
{ 
	// close the http stream when applicable
	if ( pBufferThread )
	{
		httpStopBuffering( pBufferThread );
		pBufferThread = NULL;
	}

	// stop the decoding thread
	if ( hDecodeThread != INVALID_HANDLE_VALUE )
	{
		bKillDecodeThread = TRUE;

		if ( WAIT_TIMEOUT == WaitForSingleObject( hDecodeThread, INFINITE ) )
		{
			MessageBox( mod.hMainWindow, "error asking decode thread to die!\n", "error killing decode thread", 0 );
			TerminateThread( hDecodeThread, 0 );
		}

		CloseHandle( hDecodeThread );
		hDecodeThread = INVALID_HANDLE_VALUE;
	}

	// close the input file ( when opened )
	if ( hInputFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hInputFile );
		hInputFile = INVALID_HANDLE_VALUE;
	}

	// close the output module
	mod.outMod->Close();

	// deinit VIS stuff
	mod.SAVSADeInit();
}


int genre_last=147;
char *genre_list[]={
	"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
	"Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
	"Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
	"Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
	"Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
	"Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
	"Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
	"Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
	"Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
	"Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
	"Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
	"New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
	"Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
	"Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
	"Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
	"Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
	"Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
	"Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
	"Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
	"Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
	"Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
	"Goa", "Drum & Bass", "Club House", "Hardcore", "Terror",
	"Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat",
	"Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary C",
	"Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
	"SynthPop",
};

#define TITLEBUFLEN (MAX_PATH-10)


int gettitlefromid3(id3tag *tag,char *filename,char *title)
{
	char *p,*q,*r,*s,buf[31];
	int remain = TITLEBUFLEN;

//	if((sif.m_file != "") && (sif.m_title != "")) {
//		return gettitlefromsif(filename, title);
//	}
	
	if (strncmp(tag->tag,"TAG",3) != 0) return 0;

	q = title;

	for(p=xed_title;*p != '\0' && remain > 0;)
	{
		if (*p != '%') {
			*q++ = *p++;
			remain--;
			continue;
		}

		switch(*++p)
		{
		case '%':
			*q++ = '%';
			remain--;
			break;
		case '1':
			strncpy(buf,tag->artist,30);
			buf[30] = '\0';
			for(r = buf;*r != '\0';r++) ;
			for(r--;r >= buf && *r == ' ';r--) ;
			*(r+1) = '\0';
			strncpy(q,buf,remain);
			remain -= strlen(buf);
			q += strlen(buf);
			break;
		case '2':
			strncpy(buf,tag->title,30);
			buf[30] = '\0';
			for(r = buf;*r != '\0';r++) ;
			for(r--;r >= buf && *r == ' ';r--) ;
			*(r+1) = '\0';
			strncpy(q,buf,remain);
			remain -= strlen(buf);
			q += strlen(buf);
			break;
		case '3':
			strncpy(buf,tag->album,30);
			buf[30] = '\0';
			for(r = buf;*r != '\0';r++) ;
			for(r--;r >= buf && *r == ' ';r--) ;
			*(r+1) = '\0';
			strncpy(q,buf,remain);
			remain -= strlen(buf);
			q += strlen(buf);
			break;
		case '4':
			strncpy(buf,tag->year,4);
			buf[4] = '\0';
			for(r = buf;*r != '\0';r++) ;
			for(r--;r >= buf && *r == ' ';r--) ;
			*(r+1) = '\0';
			strncpy(q,buf,remain);
			remain -= strlen(buf);
			q += strlen(buf);
			break;
		case '5':
			strncpy(buf,tag->comment,30);
			buf[30] = '\0';
			for(r = buf;*r != '\0';r++) ;
			for(r--;r >= buf && *r == ' ';r--) ;
			*(r+1) = '\0';
			strncpy(q,buf,remain);
			remain -= strlen(buf);
			q += strlen(buf);
			break;
		case '6':
			if (tag->genre > genre_last) {
				strncpy(q,"unknown",remain);
				remain -= 7;
				q += 7;
				break;
			}
			strncpy(q,genre_list[tag->genre],remain);
			remain -= strlen(genre_list[tag->genre]);
			q += strlen(genre_list[tag->genre]);
			break;
		case '7':
			r=s=filename+strlen(filename);
			while (*r != '\\' && r >= filename) r--;
			r++;
			while (*s != '.'  && s >= filename) s--;
			if (s < r) s = filename+strlen(filename);
			while(r < s && remain > 0) {
				*q++ = *r++;
				remain--;
			}
			break;
		case '8':
			s=filename+strlen(filename);
			while (*s != '\\' && s >= filename) s--;
			if (*s == '\\') s++;
			r = filename;
			while(r < s && remain > 0) {
				*q++ = *r++;
				remain--;
			}
			break;
		case '9':
			r=s=filename+strlen(filename);
			while (*r != '\\' && r >= filename) r--;
			while (*s != '.'  && s >= filename) s--;
			if (s < r) s = filename+strlen(filename);
			while(*s != '\0' && remain > 0) {
				*q++ = *s++;
				remain--;
			}
			break;
		default:
			if (remain >= 1) { *q++ = '%'; remain--; }
			if (remain >= 1) { *q++ = *p;  remain--; }
			break;
		}

		p++;
	}

	*q = '\0';
	
	return 1;
}

void getfileinfo(char *filename, char *title, int *length_in_ms)
{
	if (!filename || !*filename)
	{
		// currently playing file
		if (length_in_ms) *length_in_ms=cur_mp3info.length*1000;
		if (title) 
		{
			if (!gettitlefromid3(&current_id3,lastfn,title)) {
				char *p=lastfn+strlen(lastfn);
				while (*p != '\\' && p >= lastfn) p--;
				strncpy(title,++p,TITLEBUFLEN);
			}
		}
	}
	else
	{
		// some other file
		mp3info info;
		id3tag id3;
		VBRTAGDATA vbr;
		
		if (getmp3info(filename,&info,&id3,&vbr)) {
			if (length_in_ms) 
			{
				*length_in_ms = info.length*1000;
			}
			if (title) 
			{
				if (!gettitlefromid3(&id3,filename,title)) {
					char *p=filename+strlen(filename);
					while (*p != '\\' && p >= filename) p--;
					strncpy(title,++p,TITLEBUFLEN);
				}
			}
		} else {
			// not MP3 file
			if (length_in_ms) 
			{
				*length_in_ms = 0;
			}
			if (title) 
			{
				char *p=filename+strlen(filename);
				while (*p != '\\' && p >= filename) p--;
				strncpy(title,++p,TITLEBUFLEN);
			}
		}
	}
}

// Decoding thread

#define OBSIZE 16384
#define FBSIZE 512

static char obuf[OBSIZE];
static int bstart,bend;
static MPSTR mp;
static int decoder_stat;

void postproc(char *buf,int size)
{
	int Bps = 2;

	/* check if any post processings is required */
	if (!xch_reverse && !xch_invert && !xch_volume)
	{
		return;
	}
	
	if (cur_mp3info.nch == 2)
	{
		int i;
		int pl=127,pr=127;
		if (xpan > 0) pl = 127-xpan;
		if (xpan < 0) pr = 127+xpan;

		for(i=0;i<size/2/Bps;i++)
		{
			short *l = (short *)&buf[i*4],*r = (short *)&buf[i*4+2];

			if (xch_reverse) {
				short s;
				s = *l; *l = *r; *r = s;
			}
		
			if (xch_invert) {
				*l = *l == -32768 ? 32767 : -*l; *r = *r == -32768 ? 32767 : -*r;
			}

			if (xch_volume) {
				*l = xvolume*pl*(int)*l/(255*127);
				*r = xvolume*pr*(int)*r/(255*127);
			}
		}
	} else if (cur_mp3info.nch == 1) {
		int i;
		for(i=0;i<size/Bps;i++)
		{
			short *p = (short *)&buf[i*2];

			if (xch_invert) {
				*p = *p == -32768 ? 32767 : -*p;
			}

			if (xch_volume) {
				*p = xvolume*(int)*p/255;
			}
		}
	}
}

/*
void preload_stream(void)
{
	for(;;)
	{
		Sleep(200);
		int l = strbufp_s > strbufp_e ? strbuflen+strbufp_e-strbufp_s : strbufp_e-strbufp_s;
		if (l > strbuflen*7/8 || strbuf_eof) break;
	}
}
*/

int get_576_samples( char *out, int supZero, int flush )
{
	int at_eof = 0;
	int nch = cur_mp3info.nch;
	int Bps = 2;
	int ssize = 576*nch*Bps;

	if ( flush )
	{
		ExitMP3( &mp );

		/* clear mp structure */
		memset( &mp, 0x00, sizeof( mp ) );

		InitMP3( &mp );
		bstart = bend = 0;
		decoder_stat = MP3_ERR;
	}
	
	while (!at_eof && ssize > bend - bstart)
	{
		if (bstart != 0) {
			int i;
			for(i=0;i<bend-bstart;i++) obuf[i] = obuf[i+bstart];
			bend -= bstart;
			bstart = 0;
		}

		if (decoder_stat != MP3_OK)
		{


			char	buf[ FBSIZE ];
			size_t	len = 0;
			int		size = 0;



			if (cur_mp3info.isStream)
			{
				assert( pBufferThread );

				// get the data from the http stream
				if ( NULL != pBufferThread )
				{
					len = httpRead(	buf, 1, FBSIZE, pBufferThread );
				}

			}
			else 
			{
				// get data from the input file
				ReadFile( hInputFile, buf, FBSIZE,&len, NULL );
			}

			at_eof = ( len == 0 );
			size = 0;

			// decode chunk
			decoder_stat = decodeMP3( &mp,buf, len, obuf+bend, OBSIZE-bend, &size );

			// do post processing
			postproc( obuf, size );

			bend += size;
		}
		else
		{
			int size=0;
			decoder_stat = decodeMP3( &mp, NULL, 0, obuf+bend, OBSIZE-bend, &size );
			postproc(obuf,size);
			bend += size;
		}

		if (decoder_stat == MP3_ERR)
		{
			//MessageBox(NULL,errorstring,"Decoder error",MB_OK);
			return -1;
		}

		if ( supZero )
		{
			int p,q,c;
			for( p = bstart; p < bend; p += Bps*nch )
			{
				for(c=0;c<nch;c++)
				{
					for(q=0;q<Bps;q++)
					{
						if ( 0 != obuf[ p + c * Bps + q ] )
						{
							goto BreakSupLoop;
						}
					}
				}
			}
			BreakSupLoop:

			if ( p < bend )
			{
				bstart = p;
				supZero = 0;
			}
			else
			{
				bend = bstart;
			}
		}
	}

	{
		int l,i;
		if (ssize < bend - bstart) l = ssize; else l = bend - bstart;
		for(i=0;i<l;i++) out[i] = obuf[bstart+i];

		bstart += l;
		return l;
	}
}

DWORD WINAPI __stdcall DecodeThread( void *b )
{
	int done=0;
	int supZero = xch_supzero;
	int isStream = cur_mp3info.isStream;
	int Bps = 2;
	int flush = 0;

	decoder_in_use = 1;

	totalFramesize=0;
	nframe=0;
	
	/* clear mp structure */
	memset( &mp, 0x00, sizeof( mp ) );

	InitMP3( &mp );
	decoder_stat = MP3_ERR;
	bstart = bend = 0;

	while (! *((int *)b) ) 
	{
		if ( -1 != seek_needed )
		{
			int offs;
			unsigned long head,len,hpos;

			if (cur_mp3info.hasVbrtag)
			{
				offs = SeekPoint(current_vbrtag.toc,cur_mp3info.nbytes,seek_needed/(cur_mp3info.length*10));
			}
			else
			{
				int framesize = totalFramesize/nframe;
				float frameDuration = 576.0*(cur_mp3info.lsf?1:2)/cur_mp3info.freq;
				//float length = cur_mp3info.nbytes * 8 / cur_mp3info.bitrate;
				float length = cur_mp3info.nbytes*frameDuration/framesize;

				offs = (float)cur_mp3info.nbytes * seek_needed / (length * 1000);
			}

			offs += cur_mp3info.hpos;
			decode_pos_ms = seek_needed;
			seek_needed=-1;
			done=0;
			mod.outMod->Flush( decode_pos_ms );
			flush = 1;


			for(hpos=0;hpos<65536;hpos++)
			{
				SetFilePointer(hInputFile,offs+hpos,NULL,FILE_BEGIN);
				ReadFile( hInputFile, 3 + (char *)&head, 1, &len, NULL );
				ReadFile( hInputFile, 2 + (char *)&head, 1, &len, NULL );
				ReadFile( hInputFile, 1 + (char *)&head, 1, &len, NULL );
				ReadFile( hInputFile, 0 + (char *)&head, 1, &len, NULL );

				if (len == 0 || head_check2(head,&cur_mp3info))
				{
					break;
				}
			}

			if (hpos == 65536 || len == 0) {
				done = 1;
			//break; // syncword not found
			}

			SetFilePointer(hInputFile,offs+hpos,NULL,FILE_BEGIN);
		}
		if (done)
		{
			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying())
			{
				ExitMP3(&mp);
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				decoder_in_use = 0;
				return 0;
			}
			Sleep(10);
		}
		else if (mod.outMod->CanWrite() >= ((576*cur_mp3info.nch*Bps)<<(mod.dsp_isactive()?1:0)))
		{	
			int l;
			l=get_576_samples(sample_buffer,supZero,flush);
			supZero = 0;
			flush = 0;
			if (l == -1) {
				if (!isStream) {
					// errorneous frame found. try to resync.
					unsigned long head,len,hpos;
					unsigned long offs = SetFilePointer(hInputFile,0,NULL,FILE_CURRENT);

					for(hpos=0;hpos<65536;hpos++)
					{
						SetFilePointer(hInputFile,offs+hpos,NULL,FILE_BEGIN);
						ReadFile(hInputFile,3+(char *)&head,1,&len,NULL);
						ReadFile(hInputFile,2+(char *)&head,1,&len,NULL);
						ReadFile(hInputFile,1+(char *)&head,1,&len,NULL);
						ReadFile(hInputFile,0+(char *)&head,1,&len,NULL);
						if (len == 0 || head_check2(head,&cur_mp3info)) break;
					}

					if (hpos == 65536 || len == 0) {
						done = 1;
						//break; // syncword not found
					} else {
						SetFilePointer(hInputFile,offs+hpos,NULL,FILE_BEGIN);
						flush = 1;
					}
				} else {
					unsigned long head;
					int i;

					for(i=0;i<65536;i++)
					{
						while(!strbuf_eof && (strbufp_e+strbuflen-strbufp_s)%strbuflen < 4) Sleep(100);

						*(3+(char *)&head) = stream_buffer[(strbufp_s  )%strbuflen];
						*(2+(char *)&head) = stream_buffer[(strbufp_s+1)%strbuflen];
						*(1+(char *)&head) = stream_buffer[(strbufp_s+2)%strbuflen];
						*(0+(char *)&head) = stream_buffer[(strbufp_s+3)%strbuflen];

						if (strbuf_eof || head_check2(head,&cur_mp3info)) break;
						strbufp_s = (strbufp_s+1) % strbuflen;
					}

					if (i == 65536 || strbuf_eof) {
						done = 1;
					} else {
						flush = 1;
					}
				}
			} else if (!l)
			{
				done=1;
			}
			else
			{
				mod.SAAddPCMData((char *)sample_buffer,cur_mp3info.nch,Bps*8,decode_pos_ms);
				mod.VSAAddPCMData((char *)sample_buffer,cur_mp3info.nch,Bps*8,decode_pos_ms);
				decode_pos_ms+=(576*1000.0)/cur_mp3info.freq;
				if (mod.dsp_isactive()) l=mod.dsp_dosamples((short *)sample_buffer,l/cur_mp3info.nch/Bps,Bps*8,cur_mp3info.nch,cur_mp3info.freq)*(cur_mp3info.nch*Bps);
				mod.outMod->Write(sample_buffer,l);
			}
		}
		else
		{
			if (!cur_mp3info.hasVbrtag) {

				int framesize = 0;
				float frameDuration = 0.0f;

				if (nframe)
				{
					framesize = totalFramesize/nframe;
				}

				frameDuration = 576.0*(cur_mp3info.lsf?1:2)/cur_mp3info.freq;
				//cur_mp3info.length = cur_mp3info.nbytes * 8 / cur_mp3info.bitrate;
				cur_mp3info.length = cur_mp3info.nbytes*frameDuration/framesize;
			}
			if (!xch_dispavg) {
				static int lastBitrate;

				int currentBitrate = tabsel_123[mp.fr.lsf][2][mp.fr.bitrate_index];

				if (lastBitrate != currentBitrate) {
					mod.SetInfo(currentBitrate,-1,-1,-1);
					lastBitrate = currentBitrate;
				}
			}
			Sleep(20);
		}
	}
	ExitMP3(&mp);
	decoder_in_use = 0;
	return 0;
}

/*
// avoid CRT. Evil. Big. Bloated.
BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}
*/
__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &mod;
}

