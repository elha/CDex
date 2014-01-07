/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
** Copyright (C) Jack Moffitt (jack@icecast.org)
** Copyright (C) Michael Smith (msmith@labyrinth.net.au)
** Copyright (C) Aaron Porter (aaron@javasource.org)
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

#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <commctrl.h>
#include <mmreg.h>
#include <msacm.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <limits.h>
#include <assert.h>

#include <vorbis/vorbisfile.h>

#include "httpstream.h"

#include "in2.h"
#include "in_vorbis_config.h"
#include "in_vorbis_info.h"
#include "in_vorbis_util.h"
#include "in_vorbis_tags.h"
#include "resource.h"
#include "in_version.h"

#define SAMPLE_BUFFER_SIZE ( 576 )
#define WM_WA_MPEG_EOF WM_USER + 2

In_Module mod;

PSHORT pSampleBufferShort = NULL;
PFLOAT pSampleBufferFloat = NULL;

OggVorbis_File input_file; // input file handle

HANDLE thread_handle=INVALID_HANDLE_VALUE;	// the handle to the decode thread


void * btdvp=0;

static void show_config( HWND hwndParent );
static void	show_about( HWND hwndParent );
static int	show_info( PCHAR pszFileName, HWND hwnd );

static void quit();
static void init();
static int	isourfile( char *pszFileName );

static void	getfileinfo( char *filename, char *title, int *length_in_ms );
static int	get_576_samples( PSHORT pShortBuf );
static void	DecodeThread( void *pKillDecodeThread );
static void generate_title( vorbis_comment *comment, PCHAR pszFileName );



static void show_config( HWND hwndParent )
{
	DoConfig( mod.hDllInstance, hwndParent );
}


static void show_about( HWND hWndParent )
{
	char const about[] =
    "OggVorbis Winamp Player Plugin, " 	" Version " VPI_VERSION "\n" \
    "Copyright \xa9 " VPI_PUBLISHYEAR " " VPI_AUTHOR "\n\n"
	"based on the original work of Jack Moffitt <jack@icecast.org> \nand Michael Smith <msmith@labyrinth.net.au>\n\n"
    "This program is free software; you can redistribute it and/or modify it\n"
    "under the terms of the GNU General Public License as published by\n"
    "the Free Software Foundation; either version 2 of the License, or (at\n"
    "your option) any later version.\n\n"

    "This program is distributed in the hope that it will be useful, but\n"
    "WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
    "See the GNU General Public License for more details.";

	MessageBox( hWndParent, about, "About Ogg Winamp Plug-in", MB_ICONINFORMATION | MB_OK);
}

static int show_info( PCHAR pszFileName, HWND hwnd )
{
    strncpy( vpi_info.pszFileName, pszFileName, MAX_PATH );
	vpi_info.pszFileName[ MAX_PATH ] = '\0'; 

    DialogBox(	mod.hDllInstance, 
				MAKEINTRESOURCE( IDD_INFO_BASIC ),
				hwnd,
				info_dialog_proc );

    return 0;
}

static void init() 
{
	// init HTTP stuff
	httpInit();

	// get config parameters
	get_vpi_config();
}


static void quit()
{ 
	httpShutdown();
	clean_vpi_info();
}

static int isourfile( char *pszFileName )
{
    setHttpVars();

    return isOggUrl( pszFileName );
} 


size_t read_func( void *ptr, size_t size, size_t nmemb, void *datasource )
{
	size_t			nReturn = 0;
	HANDLE			file = (HANDLE)datasource;
	unsigned long	bytesread = 0;

	if( ReadFile( file, ptr, (unsigned long)(size*nmemb), &bytesread, NULL ) )
	{
		nReturn = bytesread / size;
	}

	return nReturn;
}

static int seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	HANDLE			file = (HANDLE)datasource;
	int				seek_type;
	unsigned long	retval;
	int				seek_highword = (int)(offset>>32);

	switch( whence )
	{
		case SEEK_SET:
			seek_type = FILE_BEGIN;
		break;

		case SEEK_CUR:
			seek_type = FILE_CURRENT;
		break;

		case SEEK_END:
			seek_type = FILE_END;
		break;
	}

	// On failure, SetFilePointer returns 0xFFFFFFFF, which is (int)-1
	retval = SetFilePointer(file, (int)(offset&0xffffffff), &seek_highword, seek_type);

	// Exactly mimic stdio return values
	if( retval == 0xFFFFFFFF )
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

int close_func( void *datasource )
{
	HANDLE file = (HANDLE)datasource;

	// Return value meaning is inverted from fclose()
	return (CloseHandle(file)?0:EOF);
}

long tell_func(void *datasource)
{
	HANDLE file = (HANDLE)datasource;

	return (long)SetFilePointer(file, 0, NULL, FILE_CURRENT); /* This returns the right number */
}


int play( char *fn )
{ 
	int maxlatency;
	HANDLE stream;
	vorbis_info *vi = NULL;
	ov_callbacks callbacks = {read_func, seek_func, close_func, tell_func};

	setHttpVars();

	if ( isOggUrl( fn ) )
	{
        mod.is_seekable = FALSE;

		if ( (btdvp = httpStartBuffering(fn, &input_file, TRUE)) == 0 )
		{
            return -1;
		}
	}
    else
    {
		char title[ 2048 ] ;
		int length_in_ms;

		getfileinfo( fn, title, &length_in_ms );

        mod.is_seekable = TRUE;

	    stream = CreateFile(	fn,
								GENERIC_READ,
								FILE_SHARE_READ | FILE_SHARE_WRITE,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL );

	    if ( stream == INVALID_HANDLE_VALUE )
	    {	
		    return -1;
	    }

	    if (ov_open_callbacks(stream, &input_file, NULL, 0, callbacks) < 0) {
		    CloseHandle(stream);
		    return 1;
	    }
    }

	vpi_info.dwFileLengthInMs = (int)ov_time_total(&input_file, -1) * 1000;


	strncpy( vpi_info.pszFileName, fn, MAX_PATH );
	vpi_info.pszFileName[ MAX_PATH ] ='\0';

	vpi_info.bPaused = FALSE;
	vpi_info.dDecodePosInMs = 0.0;
	vpi_info.lSeekToPos = VPI_NO_SEEK;

	vi = ov_info( &input_file, -1 );

	vpi_info.dwSamplerate = vi->rate;
	vpi_info.dwChannels = vi->channels;

	if ( vpi_info.dwChannels == 0 )
	{
		vpi_info.dwChannels = 1;
	}

	vpi_info.dwBitrateNominal = vi->bitrate_nominal;
	vpi_info.dwFileSize = (DWORD)ov_raw_total( &input_file, -1 );
	vpi_info.dwBitrate = ov_bitrate( &input_file, -1 );

	if ( vpi_info.dwFileLengthInMs > 0 )
	{
		vpi_info.dwBitrateAverage = (DWORD)( vpi_info.dwFileSize * 8.0 / ( vpi_info.dwFileLengthInMs / 1000.0 ) );
	}


//    ogg_uint32_t serial = ogg_page_serialno(page);

	if( vpi_info.dwChannels > 2) /* We can't handle this */
	{
		ov_clear(&input_file);
		return 1;
	}

	// allocate the sample buffer - it's twice as big as we apparently need,
	// because mod.dsp_dosamples() may use up to twice as much space. 
	pSampleBufferShort = malloc( SAMPLE_BUFFER_SIZE * vpi_info.dwChannels * 2 * 2 ); 
	
	if (pSampleBufferShort == NULL)
	{
		ov_clear(&input_file);
		return 1;
	}

	maxlatency = mod.outMod->Open( vpi_info.dwSamplerate, vpi_info.dwChannels, 16, -1, -1);
	if (maxlatency < 0) {
		// error opening device
		ov_clear(&input_file);
		return 1;
	}
	
	// dividing by 1000 for the first parameter of setinfo makes it
	// display 'H'... for hundred.. i.e. 14H Kbps.
	mod.SetInfo( vpi_info.dwBitrate / 1000, vpi_info.dwSamplerate / 1000, vpi_info.dwChannels, 1);

	// initialize vis stuff
	mod.SAVSAInit( maxlatency, vpi_info.dwSamplerate );
	mod.VSASetInfo( vpi_info.dwSamplerate, vpi_info.dwChannels );

	// set the output plug-ins default volume
	mod.outMod->SetVolume( -666 );

	// initialize kill decode thread variable
	vpi_info.bKillDecodeThread = FALSE;

	// start decodign thread
	thread_handle = (HANDLE)_beginthread( DecodeThread,0, (void *)( &vpi_info.bKillDecodeThread ) );
	
	return 0; 
}

void pause() 
{
	vpi_info.bPaused = TRUE; 
	mod.outMod->Pause( 1 ); 
}

void unpause() 
{ 
	vpi_info.bPaused = FALSE;
	mod.outMod->Pause( 0 );
}

int ispaused() 
{ 
	return vpi_info.bPaused; 
}

void stop() 
{ 
	if ( btdvp )
    {
		httpStopBuffering( btdvp );
        btdvp = 0;
    }

	if (thread_handle != INVALID_HANDLE_VALUE)
	{
		vpi_info.bKillDecodeThread = TRUE;
		
		if ( WAIT_TIMEOUT == WaitForSingleObject(thread_handle, INFINITE) )
		{
			MessageBox( mod.hMainWindow,
						"error asking thread to die!\n",
						"error killing decode thread",
						MB_OK );
			TerminateThread( thread_handle, 0 );
		}
		ov_clear( &input_file );
		thread_handle = INVALID_HANDLE_VALUE;

	}

	// deallocate sample buffer
	if ( pSampleBufferShort )
	{
		free( pSampleBufferShort );
	}

	mod.outMod->Close();

	mod.SAVSADeInit();
}

static int getlength()
{ 
	return vpi_info.dwFileLengthInMs; 
}

static int getoutputtime()
{ 
	return ( (long)vpi_info.dDecodePosInMs ) + (mod.outMod->GetOutputTime() - mod.outMod->GetWrittenTime()); 
}

static void setoutputtime( int time_in_ms )
{ 
	vpi_info.lSeekToPos = time_in_ms; 
}

static void setvolume( int volume ) 
{ 
	mod.outMod->SetVolume( volume );
}

static void setpan( int pan ) 
{ 
	mod.outMod->SetPan( pan );
}


double grabbag__replaygain_compute_scale_factor(double peak, double gain, double preamp, BOOL prevent_clipping )
{
	double scale;
	assert( peak >= 0.0 );
 	gain += preamp;
	scale = (float) pow(10.0, gain * 0.05);
	if(prevent_clipping && peak > 0.0) {
		const double max_scale = (float)(1.0 / peak);
		if(scale > max_scale)
			scale = max_scale;
	}
	return scale;
}

void ReadVorbisTags( vorbis_comment* pComment )
{
	int	nComment = 0;

	for ( nComment = 0; nComment< pComment->comments; nComment++ )
	{
		int nCount = pComment->comment_lengths[ nComment ];

		vpi_info.pszTags[ nComment ] = calloc( nCount + 1 , sizeof( CHAR ) );
		memcpy( vpi_info.pszTags[ nComment ], pComment->user_comments[ nComment ], nCount );
	}


}


static void generate_title( vorbis_comment *comment, PCHAR pszFileName )
{
	PCHAR	pEntry		= NULL;
	PCHAR	pszHTTPTitle = NULL;

	ReadVorbisTags( comment );

	pszHTTPTitle = httpGetTitle( pszFileName );

	if ( pszHTTPTitle )
	{
		CreateTitleString( pszHTTPTitle );
	}
	else
	{
		CreateTitleString( pszFileName );
	}

	if ( vpi_config.bUseReplayGain )
	{
		if ( REPLAYGAIN_MODE_AUDIOPHILE == vpi_config.nReplayGainMode )
        {
            pEntry = vorbis_comment_query(comment, VORBIS_TAG_RPGAIN_AG, 0);

            if ( !pEntry )
			{
				pEntry = vorbis_comment_query( comment, VORBIS_TAG_RPGAIN_AG_ALT, 0 );
			}
    	}

		if (!pEntry)
        {
            pEntry = vorbis_comment_query( comment, VORBIS_TAG_RPGAIN_TG, 0 );

            if (!pEntry)
			{
				pEntry = vorbis_comment_query( comment, VORBIS_TAG_RPGAIN_AG_ALT, 0 );
			}
    	}

		if ( pEntry )
		{
			vpi_info.fReplayGainScale = (FLOAT) pow( 10., atof( pEntry ) / 20 );
		}


		if ( vpi_config.bUseHardLimitter )
		{
			if ( REPLAYGAIN_MODE_AUDIOPHILE == vpi_config.nReplayGainMode )
			{
				pEntry = vorbis_comment_query(comment, "replaygain_album_peak", 0);
    		}

			if ( !pEntry )
			{
				pEntry = vorbis_comment_query( comment, "replaygain_track_peak", 0 );

				if (!pEntry)
				{
					pEntry = vorbis_comment_query( comment, "rg_peak", 0 );
				}
    		}

			if ( pEntry )
			{
				vpi_info.fReplayGainPeak = (FLOAT)atof( pEntry );

				if ( vpi_info.fReplayGainScale * vpi_info.fReplayGainPeak > 1.0f )
					vpi_info.fReplayGainScale = 1.0f / vpi_info.fReplayGainPeak;

			}

			/* security */
			if (vpi_info.fReplayGainScale > 15.0f )
			{
				vpi_info.fReplayGainScale = 15.0f;
			}
		}
	}

}

static void getfileinfo( PCHAR pszFileName, char *title, int *length_in_ms)
{
	HANDLE stream;
	OggVorbis_File vf;
	vorbis_comment*	comment = NULL;

	ov_callbacks callbacks = {read_func, seek_func, close_func, tell_func};

	clean_vpi_info();

	vpi_info.iCurrentSection = -1;


	if ( pszFileName != NULL && pszFileName[0] != 0 )
    {
        if ( isOggUrl( pszFileName ) )
        {
            if (!httpStartBuffering( pszFileName, &vf, FALSE ) )
			{
                return;
			}
        }
        else
        {
		    stream = CreateFile( pszFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
			    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		    if(stream == INVALID_HANDLE_VALUE)
			{
			    return;
			}

		    // The ov_open() function performs full stream detection and machine
		    // initialization.  If it returns 0, the stream *is* Vorbis and we're
		    // fully ready to decode.
		    

		    if (ov_open_callbacks(stream, &vf, NULL, 0, callbacks) < 0)
			{
			    CloseHandle( stream );
			    return;
		    }
        }

		vpi_info.dwFileLengthInMs = (int)ov_time_total(&vf, -1) * 1000;
		*length_in_ms = vpi_info.dwFileLengthInMs;

		comment = ov_comment(&vf, -1);

		if( comment )
		{
			generate_title(comment, pszFileName);
			if( vpi_info.pszTitle )
			{
				strcpy( title, vpi_info.pszTitle );
			}
		}
		else
		{
			strcpy( title, pszFileName );
		}

		// once the ov_open() succeeds, the file belongs to vorbisfile.
		// ov_clear() will close it.
	
		ov_clear( &vf );
		
	}
	else 
	{
		/* This is the only section of code which uses vorbisfile calls 
		   in one thread whilst the main playback thread is running. 
		   Technically, we should protect it with critical sections, but
		   these two calls appear to be safe on win32/x86 */

		comment = ov_comment( &input_file, -1 );

		if( comment )
		{
			generate_title( comment, vpi_info.pszFileName );

			if ( vpi_info.pszTitle )
			{
				strcpy( title, vpi_info.pszTitle );
			}
		}
   		else
		{
			strcpy(title, pszFileName);
		}

		*length_in_ms = (int)ov_time_total(&input_file, -1) * 1000;

	}
}

void eq_set(int on, char data[10], int preamp) 
{ 
	/* Waiting on appropriate libvorbis API additions. */
}


// render 576 samples into buf. 
// note that if you adjust the size of pSampleBufferShort, for say, 1024
// sample blocks, it will still work, but some of the visualization 
// might not look as good as it could. Stick with 576 sample blocks
// if you can, and have an additional auxiliary (overflow) buffer if 
// necessary.

static int get_576_samples( PSHORT pShortBuf)
{
	int ret = 0;
	PFLOAT* ppFloatBuf = NULL;

    if ( vpi_config.bUseReplayGain )
	{
		int		nIndex = 0;
		int		nOutIdx = 0;
		DWORD	dwChannel = 0;

		ret = ov_read_float(	&input_file,
								&ppFloatBuf,
								SAMPLE_BUFFER_SIZE * vpi_info.dwChannels,		// max # samples
								&vpi_info.iCurrentSection );

		/* ReplayGain processing */
		for ( nIndex = 0; nIndex < ret ; nIndex++ )
		{
			for ( dwChannel = 0; dwChannel < vpi_info.dwChannels; dwChannel++)
			{
				FLOAT fSampleValue = ppFloatBuf[ dwChannel ][ nIndex ];

				fSampleValue *= vpi_info.fReplayGainScale;

				if ( vpi_config.bUseReplayGainBooster )
				{
					fSampleValue *= 2.;

					/* hard 6dB limiting */
					if ( fSampleValue < -0.5 )
					{
						fSampleValue = (FLOAT)( tanh((fSampleValue + 0.5) / (1-0.5)) * (1-0.5) - 0.5 );
					}
					else if ( fSampleValue > 0.5 )
					{
						fSampleValue = (FLOAT)( tanh((fSampleValue - 0.5) / (1-0.5)) * (1-0.5) + 0.5 );
					}
				}

				// normalize value to short range
				fSampleValue *= SHRT_MAX;

				// limit output value
				if (fSampleValue > SHRT_MAX )
				{
					fSampleValue = SHRT_MAX;
				}
				else 
				{
					if (fSampleValue < -SHRT_MAX)
					{
						fSampleValue = -SHRT_MAX;
					}
				}

				pShortBuf[ nOutIdx++ ] = (SHORT)fSampleValue;
			}
		}
			ret = nOutIdx * sizeof( SHORT );
	}
	else
	{
		ret = ov_read(	&input_file,
						(PCHAR)pShortBuf, 
						SAMPLE_BUFFER_SIZE * vpi_info.dwChannels * sizeof( SHORT ), // size in bytes
						0,		// big or little endian
						2,		// bytes per sample
						1,		// 0 = unsigned 1 = signed
						&vpi_info.iCurrentSection );
	}

	return ret;
}

static void DecodeThread( void *pKillDecodeThread )
{
	int eos = 0;
	int lostsync = 0;
	double lastupdate = 0;

	while ( FALSE == *( (volatile BOOL *)pKillDecodeThread ) ) 
	{
		if ( vpi_info.lSeekToPos != VPI_NO_SEEK )
		{
			// - (seek_needed % 1000)
			vpi_info.dDecodePosInMs = (double)vpi_info.lSeekToPos;

			lastupdate = vpi_info.dDecodePosInMs;

			vpi_info.lSeekToPos = VPI_NO_SEEK;

			eos = 0;

			mod.outMod->Flush( (long)vpi_info.dDecodePosInMs );

			ov_time_seek( &input_file, vpi_info.dDecodePosInMs / 1000 );
		}

		if ( eos )
		{
			mod.outMod->CanWrite();
			
			if (!mod.outMod->IsPlaying())
			{
				PostMessage(mod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				return;
			}
			
			Sleep(10);

		}
		else if (mod.outMod->CanWrite() >= ((SAMPLE_BUFFER_SIZE * (int)vpi_info.dwChannels * 2) << (mod.dsp_isactive() ? 1 : 0)))
		{
			int ret;
			ret = get_576_samples( pSampleBufferShort );
			
			if ( 0 == ret )
			{
				// eof
				eos = 1;
			}
			else 
			{
				if(ret < 0)
				{
					/* Hole in data, lost sync, or something like that */
					/* Inform winamp that we lost sync                 */
					mod.SetInfo( vpi_info.dwBitrate / 1000, vpi_info.dwSamplerate / 1000, vpi_info.dwChannels, 0);
					lostsync = 1;
				}
				else
				{
					/* Update current bitrate (not currently implemented), and set sync (if lost) */
					if(lostsync || (vpi_info.dDecodePosInMs - lastupdate > 500))
					{
						vpi_info.dwBitrate = ov_bitrate_instant(&input_file);
						mod.SetInfo( vpi_info.dwBitrate / 1000, vpi_info.dwSamplerate / 1000, vpi_info.dwChannels, 1);
						lostsync = 0;
						lastupdate = vpi_info.dDecodePosInMs;
					}
					mod.SAAddPCMData((char *)pSampleBufferShort, vpi_info.dwChannels, 16, (long)vpi_info.dDecodePosInMs);
					mod.VSAAddPCMData((char *)pSampleBufferShort, vpi_info.dwChannels, 16, (long)vpi_info.dDecodePosInMs);
					vpi_info.dDecodePosInMs += (ret/(2*vpi_info.dwChannels) * 1000) / (float)vpi_info.dwSamplerate;
									
					if (mod.dsp_isactive()) 
					{
						ret = mod.dsp_dosamples((short *)pSampleBufferShort, ret / vpi_info.dwChannels / (2), 16, vpi_info.dwChannels, vpi_info.dwSamplerate) * (vpi_info.dwChannels * (2));
					}
					
					mod.outMod->Write( (PCHAR)pSampleBufferShort, ret );
				}
			}
		}
		else
		{
			Sleep( 20 );
		}
	}
	
	_endthread();
}

In_Module mod = 
{
	IN_VER,
	"OggVorbis Input Plugin " VPI_VERSION,
	0,	// hMainWindow
	0,  // hDllInstance
	"OGG\0OggVorbis File (*.OGG)\0",
	1,	// is_seekable
	1, // uses output
	show_config,
	show_about,
	init,
	quit,
	getfileinfo,
	show_info,
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

	0,0,0,0,0,0,0,0,0, // vis stuff


	0,0, // dsp
 
	eq_set,

	NULL,		// setinfo

	0 // out_mod

};

__declspec( dllexport ) In_Module * winampGetInModule2()
{
	return &mod;
}


