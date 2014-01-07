/*
** Copyright (C) 1999-2002 Erik de Castro Lopo <erikd@zip.com.au>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published by
** the Free Software Foundation; either version 2.1 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
** 
** You should have received a copy of the GNU Lesser General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* This file contains definitions commong to WAV and W64 files. */


#ifndef WAV_W64_H_INCLUDED
#define WAV_W64_H_INCLUDED

/*------------------------------------------------------------------------------
** List of known WAV format tags
*/

enum
{	
	WAVE_FORMAT_UNKNOWN				= 0x0000,		/* Microsoft Corporation */
	WAVE_FORMAT_PCM     		    = 0x0001, 		/* Microsoft PCM format */

	WAVE_FORMAT_MS_ADPCM			= 0x0002,		/* Microsoft ADPCM */
	WAVE_FORMAT_IEEE_FLOAT			= 0x0003,		/* Micrososft 32 bit float format */
	
	WAVE_FORMAT_IBM_CVSD			= 0x0005,		/* IBM Corporation */
	WAVE_FORMAT_ALAW				= 0x0006,		/* Microsoft Corporation */
	WAVE_FORMAT_MULAW				= 0x0007,		/* Microsoft Corporation */
	WAVE_FORMAT_OKI_ADPCM			= 0x0010,		/* OKI */
	WAVE_FORMAT_IMA_ADPCM			= 0x0011,		/* Intel Corporation */
	WAVE_FORMAT_MEDIASPACE_ADPCM	= 0x0012,		/* Videologic */
	WAVE_FORMAT_SIERRA_ADPCM		= 0x0013,		/* Sierra Semiconductor Corp */
	WAVE_FORMAT_G723_ADPCM			= 0x0014,		/* Antex Electronics Corporation */
	WAVE_FORMAT_DIGISTD				= 0x0015,		/* DSP Solutions, Inc. */
	WAVE_FORMAT_DIGIFIX				= 0x0016,		/* DSP Solutions, Inc. */
	WAVE_FORMAT_DIALOGIC_OKI_ADPCM	= 0x0017,		/*  Dialogic Corporation  */
	WAVE_FORMAT_MEDIAVISION_ADPCM	= 0x0018,		/*  Media Vision, Inc. */

	WAVE_FORMAT_YAMAHA_ADPCM		= 0x0020,		/* Yamaha Corporation of America */
	WAVE_FORMAT_SONARC				= 0x0021,		/* Speech Compression */
	WAVE_FORMAT_DSPGROUP_TRUESPEECH = 0x0022,		/* DSP Group, Inc */
	WAVE_FORMAT_ECHOSC1				= 0x0023,		/* Echo Speech Corporation */
	WAVE_FORMAT_AUDIOFILE_AF18  	= 0x0024,		/* Audiofile, Inc. */
	WAVE_FORMAT_APTX				= 0x0025,		/* Audio Processing Technology */
	WAVE_FORMAT_AUDIOFILE_AF10  	= 0x0026,		/* Audiofile, Inc. */

	WAVE_FORMAT_DOLBY_AC2			= 0x0030,		/* Dolby Laboratories */
	WAVE_FORMAT_GSM610				= 0x0031,		/* Microsoft Corporation */
	WAVE_FORMAT_MSNAUDIO			= 0x0032,		/* Microsoft Corporation */
	WAVE_FORMAT_ANTEX_ADPCME		= 0x0033, 		/* Antex Electronics Corporation */
	WAVE_FORMAT_CONTROL_RES_VQLPC	= 0x0034,		/* Control Resources Limited */
	WAVE_FORMAT_DIGIREAL			= 0x0035,		/* DSP Solutions, Inc. */
	WAVE_FORMAT_DIGIADPCM			= 0x0036,		/* DSP Solutions, Inc. */
	WAVE_FORMAT_CONTROL_RES_CR10	= 0x0037,		/* Control Resources Limited */
	WAVE_FORMAT_NMS_VBXADPCM		= 0x0038,		/* Natural MicroSystems */
	WAVE_FORMAT_ROCKWELL_ADPCM		= 0x003B,		/* Rockwell International */
	WAVE_FORMAT_ROCKWELL_DIGITALK	= 0x003C, 		/* Rockwell International */

	WAVE_FORMAT_G721_ADPCM			= 0x0040,		/* Antex Electronics Corporation */
	WAVE_FORMAT_MPEG				= 0x0050,		/* Microsoft Corporation */

	WAVE_FORMAT_MPEGLAYER3			= 0x0055,		/* MPEG 3 Layer 1 */

	IBM_FORMAT_MULAW				= 0x0101,		/* IBM mu-law format */
	IBM_FORMAT_ALAW					= 0x0102,		/* IBM a-law format */
	IBM_FORMAT_ADPCM				= 0x0103,		/* IBM AVC Adaptive Differential PCM format */

	WAVE_FORMAT_CREATIVE_ADPCM		= 0x0200,		/* Creative Labs, Inc */

	WAVE_FORMAT_FM_TOWNS_SND		= 0x0300,		/* Fujitsu Corp. */
	WAVE_FORMAT_OLIGSM				= 0x1000,		/* Ing C. Olivetti & C., S.p.A. */
	WAVE_FORMAT_OLIADPCM			= 0x1001,		/* Ing C. Olivetti & C., S.p.A. */
	WAVE_FORMAT_OLICELP				= 0x1002,		/* Ing C. Olivetti & C., S.p.A. */
	WAVE_FORMAT_OLISBC				= 0x1003,		/* Ing C. Olivetti & C., S.p.A. */
	WAVE_FORMAT_OLIOPR				= 0x1004,		/* Ing C. Olivetti & C., S.p.A. */

	WAVE_FORMAT_VOXWARE				= 0x181C,		/* ????? */
	WAVE_FORMAT_INTERWAV_VSC112		= 0x7150,		/* ????? */

	WAVE_FORMAT_EXTENSIBLE			= 0xFFFE

	/*	
	** Following definitions found in Bill Schottstaedt's sndlib library.
	
	0x0004: VSELP

	0x0019: HP cu codec

	0x0027: prosody 1612
	0x0028: lrc,

	0x0039: Roland rdac
	0x003a: echo sc3

	0x003d: Xebec,

	0x0041: G728 CELP
	0x0042: MS G723

	0x0052: RT24
	0x0053: PAC

	0x0059: Lucent G723
	0x0060: Cirrus,
	0x0061: ESS Tech pcm
	0x0062: voxware (obsolete)
	0x0063: canopus atrac,
	0x0064: G726
	0x0065: G722
	0x0066: DSAT
	0x0067: DSAT display,
	0x0069: voxware (obsolete)
	0x0070: voxware ac8 (obsolete)
	0x0071: voxware ac10 (obsolete)
	0x0072: voxware ac16 (obsolete)
	0x0073: voxware ac20 (obsolete)
	0x0074: voxware rt24
	0x0075: voxware rt29
	0x0076: voxware rt29hw (obsolete)
	0x0077: voxware vr12 (obsolete)
	0x0078: voxware vr18 (obsolete)
	0x0079: voxware tq40 (obsolete)
	0x0080: softsound
	0x0081: voxware tq60 (obsolete)
	0x0082: MS RT24
	0x0083: G729A,
	0x0084: MVI_MVI2
	0x0085: DF G726
	0x0086: DF GSM610
	0x0088: isaudio
	0x0089: onlive,
	0x0091: sbc24
	0x0092: dolby ac3 spdif
	0x0097: zyxel adpcm
	0x0098: philips lpcbb,
	0x0099: packed

	0x0100: rhetorex adpcm

	0x0111: vivo G723
	0x0112: vivo siren
	0x0123: digital g273

	0x0202: Creative fastspeech 8
	0x0203: Creative fastspeech 10
	0x0220: quarterdeck

	0x0400: BTV digital
	0x0680: VME vmpcm

	0x1100: LH codec
	0x1400: Norris
	0x1401: isaudio
	0x1500: Soundspace musicompression
	0x2000: DVM
	*/
} ;

typedef	struct
{	unsigned short	format ;
	unsigned short	channels ;
	unsigned int	samplerate ;
	unsigned int	bytespersec ;
	unsigned short	blockalign ;
	unsigned short	bitwidth ;
} MIN_WAV_FMT ;

typedef	struct 
{	unsigned short	format ;
	unsigned short	channels ;
	unsigned int	samplerate ;
	unsigned int	bytespersec ;
	unsigned short	blockalign ;
	unsigned short	bitwidth ;
	unsigned short	extrabytes ;
	unsigned short	dummy ;
} WAV_FMT_SIZE20 ;

typedef	struct
{	unsigned short	format ;
	unsigned short	channels ;
	unsigned int	samplerate ;
	unsigned int	bytespersec ;
	unsigned short	blockalign ;
	unsigned short	bitwidth ;
	unsigned short	extrabytes ;
	unsigned short	samplesperblock ;
	unsigned short	numcoeffs ;
	struct
	{	short	coeff1 ;
		short	coeff2 ;
	}	coeffs [7] ;
} MS_ADPCM_WAV_FMT ;

typedef	struct
{	unsigned short	format ;
	unsigned short	channels ;
	unsigned int	samplerate ;
	unsigned int	bytespersec ;
	unsigned short	blockalign ;
	unsigned short	bitwidth ;
	unsigned short	extrabytes ;
	unsigned short	samplesperblock ;
} IMA_ADPCM_WAV_FMT ;

typedef	struct
{	unsigned short	format ;
	unsigned short	channels ;
	unsigned int	samplerate ;
	unsigned int	bytespersec ;
	unsigned short	blockalign ;
	unsigned short	bitwidth ;
	unsigned short	extrabytes ;
	unsigned short	samplesperblock ;
} GSM610_WAV_FMT ;

typedef struct
{	unsigned int	esf_field1 ;
	unsigned short	esf_field2 ;
	unsigned short	esf_field3 ;
	char	esf_field4 [8] ;
} EXT_SUBFORMAT ;

typedef	struct
{	unsigned short	format ;
	unsigned short	channels ;
	unsigned int	samplerate ;
	unsigned int	bytespersec ;
	unsigned short	blockalign ;
	unsigned short	bitwidth ;
	unsigned short	extrabytes ;
	unsigned short	validbits ;
	unsigned int	channelmask ;
	EXT_SUBFORMAT	esf ;
} EXTENSIBLE_WAV_FMT ;

typedef union
{	unsigned short		format ;
	MIN_WAV_FMT			min ;
	IMA_ADPCM_WAV_FMT	ima ;
	MS_ADPCM_WAV_FMT	msadpcm ;
	EXTENSIBLE_WAV_FMT	ext ;
	GSM610_WAV_FMT		gsm610 ;
	WAV_FMT_SIZE20		size20 ;
	char				padding [512] ;
} WAV_FMT ;

typedef struct
{	int frames ;
} FACT_CHUNK ;

#define		WAV_W64_GSM610_BLOCKSIZE	65
#define		WAV_W64_GSM610_SAMPLES		320

/*------------------------------------------------------------------------------------ 
**	Functions defined in wav_ms_adpcm.c
*/

#define	MSADPCM_ADAPT_COEFF_COUNT	7

void	msadpcm_write_adapt_coeffs (SF_PRIVATE *psf) ;

/*------------------------------------------------------------------------------------ 
**	Functions defined in wav_gsm610.c
*/

int 	wav_w64_srate2blocksize (int srate_chan_product) ;
char const* wav_w64_format_str (int k) ;
int		wav_w64_read_fmt_chunk (SF_PRIVATE *psf, WAV_FMT *wav_fmt, int structsize) ;

#endif
