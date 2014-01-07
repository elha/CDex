/*
** Copyright (C) 2000 Albert L. Faber
** 
** Floating-Point processeing was added by DSPguru on March 2002.
** Modified Source-code can be found at http://DSPguru.doom9.net
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Leeser General Public License as published by
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

#ifndef MP2ENC_H_INCLUDED
#define MP2ENC_H_INCLUDED

#include "Settings.h"

#define		MP2ENC_MD_STEREO		0
#define		MP2ENC_MD_JSTEREO		1
#define		MP2ENC_MD_DUALCHANNEL	2
#define		MP2ENC_MD_MONO			3

#define		MP2ERR_OK							0x00000000
#define		MP2ERR_INVALID_FORMAT				0x00000001
#define		MP2ERR_INVALID_FORMAT_PARAMETERS	0x00000002
#define		MP2ERR_NO_MORE_HANDLES				0x00000003
#define		MP2ERR_INVALID_HANDLE				0x00000004

typedef		unsigned long			HMP2_STREAM;
typedef		HMP2_STREAM*			PHMP2_STREAM;
typedef		unsigned long			MP2ERR;

#ifndef HAS_WIN_TYPES
	typedef char			CHAR,	*PCHAR;
	typedef unsigned char	BYTE,	*PBYTE;
	typedef short			SHORT,	*PSHORT;
	typedef unsigned short	WORD,	*PWORD;
	typedef long			INT,	*PINT;
	typedef long			LONG,	*PLONG;
	typedef unsigned long	DWORD,	*PDWORD;
	typedef float			FLOAT,	*PFLOAT;
	typedef double			DOUBLE,	*PDOUBLE;
#endif

#define MP2ENC_CUR_IF_VERSION 1

typedef struct
{
	DWORD	dwVersion;
	DWORD	dwSize;

	DWORD	dwSampleRate;	// 48000,44100,32000 for MPEG1, 24000,22050,16000 for MPEG2
	DWORD	dwMode;			// MP2ENC_MODE_STEREO, MP2ENC_MODE_DUALCHANNEL, MP2ENC_MODE_MONO
	DWORD	dwBitrate;		// 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256 and 320
							
	DWORD	bPrivate;		
	DWORD	bCRC;
	DWORD	bCopyright;
	DWORD	bOriginal;

	DWORD	bUseVbr;
	DWORD	dwVbrQuality;	// 0 = best quality, 9= worsed quality

	DWORD	dwPsyModel;		// 0,1,2
	DWORD	dwQuickMode;	// Skip every Nth frame

	BOOL	bWriteAncil;	// write ancillary bits into frame ?
	BOOL	bNoPadding;		// disable padding bits ?

	INT		nDebugLevel;	// debug level, zero is no debug info

	BYTE	reserved[128-sizeof(DWORD)*16];
		
} MP2ENC_CONFIG, *PMP2ENC_CONFIG;


MP2ERR	MP2EncOpen			(PMP2ENC_CONFIG pMp2Config, PDWORD dwSamples, PDWORD dwBufferSize, PHMP2_STREAM phStream);
MP2ERR	MP2EncEncodeFrame	(HMP2_STREAM hStream, DWORD nSamples, PFLOAT buffer_l,PFLOAT buffer_r, PBYTE pOutput, PDWORD pdwOutput);
MP2ERR	MP2EncClose			(HMP2_STREAM hStream);

PSETTINGS GetStreamSettings( DWORD hStream );


#endif