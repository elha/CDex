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

#ifndef PSYCHO_I_H_INCLUDED
#define PSYCHO_I_H_INCLUDED

#include "common.h"

typedef struct {
        double     x;
        int        type, next, map;
} mask, *mask_ptr;

/* Psychoacoustic Model 1 Type Definitions */

typedef int        IFFT2[FFT_SIZE/2];
typedef int        IFFT[FFT_SIZE];
typedef double     D9[9];
typedef double     D10[10];
typedef double     D640[640];
typedef double     D1408[1408];
typedef double     DFFT2[FFT_SIZE/2];
typedef double     DSBL[SBLIMIT];
typedef double     D2SBL[2][SBLIMIT];
typedef double     DFFT[FFT_SIZE];


// Psychacoustic Model 1 Definitions

#define CB_FRACTION     0.33
#define MAX_SNR         1000
#define NOISE           10
#define TONE            20
#define DBMIN           -200.0
#define LAST            -1
#define STOP            -100
#define POWERNORM       90.3090 /* = 20 * log10(32768) to normalize */
                                /* max output power to 96 dB per spec */



void Psycho_I
	(
		PFLOAT	buffer[2],
		DOUBLE	scale[2][SBLIMIT], 
		DOUBLE	ltmin[2][SBLIMIT],
		PFRAME	pFrame
	);


void read_cbound(int lay,int freq);

void read_freq_band(g_ptr* ltg,int lay,int freq);

DOUBLE add_db(DOUBLE a,DOUBLE b);

void make_map(mask power[HAN_SIZE],g_thres* ltg);

void II_f_f_t(DOUBLE sample[FFT_SIZE],mask power[HAN_SIZE]);

void II_hann_win(DOUBLE sample[FFT_SIZE]);

void II_pick_max(mask power[SBLIMIT],DOUBLE spike[HAN_SIZE]);

void II_tonal_label(mask power[HAN_SIZE],int* tone);

void noise_label(mask* power,int* noise,g_thres* ltg);

void subsampling(mask power[HAN_SIZE],g_thres* ltg,int* tone,int* noise);

void threshold(mask power[HAN_SIZE],g_thres* ltg,int* tone,int* noise,int bit_rate);

void II_smr(	DOUBLE	ltmin[SBLIMIT],
				DOUBLE	spike[SBLIMIT],
				DOUBLE	scale[SBLIMIT],
				int		sblimit
			);

void II_minimum_mask(g_thres* ltg,DOUBLE ltmin[SBLIMIT],int sblimit);

#endif