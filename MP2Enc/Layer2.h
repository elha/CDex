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

#ifndef ENCODER_H_INCLUDED
#define ENCODER_H_INCLUDED

#include "common.h"
#include "tables.h"


/***********************************************************************
*
*  Encoder Include Files
*
***********************************************************************/

/***********************************************************************
*
*  Encoder Definitions
*
***********************************************************************/

/* General Definitions */

/* Default Input Arguments (for command line control) */

#define DFLT_LAY        2      /* default encoding layer is II */
#define DFLT_MOD        's'    /* default mode is stereo */
#define DFLT_PSY        2      /* default psych model is 1 */
#define DFLT_SFQ        44.1   /* default input sampling rate is 44.1 kHz */
#define DFLT_EMP        'n'    /* default de-emphasis is none */
#define DFLT_EXT        ".mp2" /* default output file extension */
#define FILETYPE_ENCODE 'TEXT'
#define CREATOR_ENCODE  'MpgD'

// This is the smallest MNR a subband can have before it is counted
// as 'noisy' by the logic which chooses the number of JS subbands

#define NOISY_MIN_MNR   0.0

/***********************************************************************
*
*  Encoder Type Definitions
*
***********************************************************************/


extern int iswav;


/***********************************************************************
*
*  Encoder Function Prototype Declarations
*
***********************************************************************/


//void filter_subband(double z[HAN_SIZE],double s[SBLIMIT]);


void   encode_info(FRAME*, BITSTREAM*);
#endif

// The following functions are in the file "encode.c"

double mod(double);
void   pick_scale(UINT[2][3][SBLIMIT], FRAME*,
                           double[2][SBLIMIT]);
void   put_scale(UINT[2][3][SBLIMIT], FRAME*,
                           double[2][SBLIMIT]);
void II_main_bit_allocation
	(
		double			perm_smr[2][SBLIMIT],
		UINT			scfsi[2][SBLIMIT],
		UINT			bit_alloc[2][SBLIMIT],
		INT*			adb,
		FRAME*	fr_ps,
		PSETTINGS		pSettings
	);

int    II_a_bit_allocation(double[2][SBLIMIT], UINT[2][SBLIMIT],
                           UINT[2][SBLIMIT], int*, FRAME*);

void II_subband_quantization(	UINT scalar[2][3][SBLIMIT],
								double sb_samples[2][3][SCALE_BLOCK][SBLIMIT],
								UINT j_scale[3][SBLIMIT],
								double j_samps[3][SCALE_BLOCK][SBLIMIT],
								UINT bit_alloc[2][SBLIMIT],
								UINT sbband[2][3][SCALE_BLOCK][SBLIMIT],
								FRAME *fr_ps);

void   II_encode_bit_alloc(UINT[2][SBLIMIT], FRAME*,BITSTREAM*);
void II_sample_encoding(	UINT sbband[2][3][SCALE_BLOCK][SBLIMIT],
							UINT bit_alloc[2][SBLIMIT],
							FRAME *fr_ps,
							BITSTREAM *bs
						);

void   encode_CRC(UINT, BITSTREAM*);


void II_combine_LR(double	sb_sample[2][3][SCALE_BLOCK][SBLIMIT],
				   double	joint_sample[3][SCALE_BLOCK][SBLIMIT],
				   int		sblimit);

void II_scale_factor_calc(	double	sb_sample[][3][SCALE_BLOCK][SBLIMIT],
							UINT	scalar[][3][SBLIMIT],
							int		nChannels,
							int		sblimit);

void II_transmission_pattern(
								UINT			scalar[2][3][SBLIMIT],
								UINT			scfsi[2][SBLIMIT],
								FRAME*	fr_ps,
								int				sblimit

							);

void II_encode_scale(
						UINT				bit_alloc[2][SBLIMIT],
						UINT				scfsi[2][SBLIMIT],
						UINT				scalar[2][3][SBLIMIT],
						FRAME*		fr_ps,
						BITSTREAM*	bs
					);



