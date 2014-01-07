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


#ifndef IN_VORBIS_UTIL_H_INCLUDED
#define IN_VORBIS_UTIL_H_INCLUDED

int UTF8ToWideChar( LPWSTR target, LPCSTR utf8, WCHAR unknown );
int _utf8_to_ucs4( unsigned int *target, const char *utf8, int n );
PCHAR UTT8_2_ASCII( CHAR* pszUTF, CHAR* pOutput, DWORD dwMaxSize );
void clean_vpi_info();
void get_vpi_config( );
void store_vpi_config( );

#define UTF8_ILSEQ	-1
#define VPI_NO_SEEK -1

#define IN_VORBIS_MAX_TAGS (16)

typedef enum REPLAY_MODES_TAG 
{
	REPLAYGAIN_MODE_RADIO,
	REPLAYGAIN_MODE_AUDIOPHILE,
	REPLAYGAIN_MODE_LAST
} REPLAY_MODES;

typedef struct VPI_INFO_TAG
{
	CHAR	pszFileName[ MAX_PATH + 1 ];
	PCHAR	pszVendor;
	PCHAR	pszTitle;

	PCHAR	pszTags[ IN_VORBIS_MAX_TAGS ];

	DWORD	dwBitrate;
	DWORD	dwBitrateNominal;
	DWORD	dwChannels;
	DWORD	dwSamplerate;

	DWORD	dwFileSize;
	DWORD	dwFileLengthInMs;
	DWORD	dwSerialNumber;
	DWORD	dwBitrateAverage;

	INT		iCurrentSection;

	FLOAT	fReplayGainScale;
	FLOAT	fReplayGainPeak;

	// status
	BOOL	bPaused;
	DOUBLE	dDecodePosInMs;
	BOOL	bKillDecodeThread;
	LONG	lSeekToPos;
	
} VPI_INFO;

typedef struct VPI_CONFIG_TAG
{
	BOOL			bUseReplayGain;
	BOOL			bUseReplayGainBooster;
	REPLAY_MODES	nReplayGainMode;
	INT				nReplayGainPreAmp;
	BOOL			bUseHardLimitter;
	DWORD			dwProxyPort;
	CHAR			szProxyAddress[ MAX_PATH ];
} VPI_CONFIG;


extern VPI_INFO vpi_info;
extern VPI_CONFIG vpi_config;
extern HANDLE gs_hModule;

PCHAR get_tag_value_by_name( const CHAR* pszTag );
PCHAR get_tag_value_by_number( int nComment );
DWORD get_number_of_tags( );
PCHAR CreateTitleString( PCHAR pszFileName );

#endif