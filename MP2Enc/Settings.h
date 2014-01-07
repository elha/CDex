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

#ifndef SETTINGS_H_INCLUDED
#define SETTINGS_H_INCLUDED

#include "Machine.h"
#include "Types.h"

// LOCAL TYPE DEFINITIONS
typedef double	SBS[2][3][SCALE_BLOCK][SBLIMIT];
typedef double	JSBS[3][SCALE_BLOCK][SBLIMIT];
typedef UINT	SUB[2][3][SCALE_BLOCK][SBLIMIT];
typedef double	MYIN[2][HAN_SIZE];

typedef struct PSY_II_COEFF_SAVE_TAG
{
	FLOAT	real;
	FLOAT	imag;
	FLOAT	power;
} PSY_II_COEFF_SAVE, *PPSY_II_COEFF_SAVE;

typedef struct PSY_II_SPRD_MATRIX_TAG
{
	unsigned int	num_coeffs;
	unsigned int	position[ CBANDS ];
	FLOAT			coeffs[ CBANDS ];

} PSY_II_SPRD_MATRIX, *PPSY_II_SPRD_MATRIX;


typedef struct PSY_II_PARAMS_TAG
{
	PSY_II_COEFF_SAVE	coeff_save[ 2 ][ 2 ][ HBLKSIZE ];
	FLOAT				window[ BLKSIZE ];
	FLOAT*				pfAbsThreshold;
	FLOAT				cbval[ CBANDS ];
	INT					partition[ HBLKSIZE ];
	FLOAT				rnorm[ CBANDS ];
	DOUBLE				tmn[ CBANDS ];
	PSY_II_SPRD_MATRIX	spreadfunc_matrix[ CBANDS ];
} PSY_II_PARAMS,*PPSY_II_PARAMS;


typedef struct SETTINGS_TAG
{

	BOOL		bUseVbr;
	LONG		nVbrQuality;
	BOOL		bUsePadBit;
	INT			nAncilBits;
	DOUBLE		dAvgSlotsPerFrame;
	INT			nFrame;
	INT			nMinBitRateIndex;
	ULONG		nBitsPerSlot;
	ULONG		nSamplesPerFrame;
	DWORD		dwPsyModel;
	DWORD		dwQuickMode;
	INT			nDebugLevel;

	PBITSTREAM	pBitstream;
	PFRAME		pFrame;

	INT			nMaxLeftEnergy;
	INT			nMaxRightEnergy;

	SBS*			sb_sample;
	JSBS*			j_sample;
	SUB*			subband;
	UINT			bit_alloc[2][SBLIMIT];
	UINT			scfsi[2][SBLIMIT];
	UINT			scalar[2][3][SBLIMIT];
	UINT			j_scale[3][SBLIMIT];
	double			smr[2][SBLIMIT];				// signal to mask ratio
	double			lgmin[2][SBLIMIT];
	FLOAT			sam[2][1344]; /* was [1056]; */
	int				model;
	UINT			crc;
	ULONG			frameBits;
	ULONG			num_samples;

	INT				adb;
    PFLOAT			buffer[2];//[MPEGFRAMESIZE];

	PSY_II_PARAMS	Psy_II_params;

} SETTINGS, *PSETTINGS;

#endif
