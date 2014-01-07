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

/***********************************************************************
*
*  Global Include Files
*
***********************************************************************/


#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include "Machine.h"
#include "Settings.h"
#include "types.h"
#include "MPegError.h"

/***********************************************************************
*
*  Global Variable External Declarations
*
***********************************************************************/

extern char*	mode_names[4];
extern char*	layer_names[3];
extern char*	version_names[2];
extern double   s_freq[2][4];
extern int      bitrate[2][3][15];
extern double	multiple[64];


/***********************************************************************
*
*  Global Function Prototype Declarations
*
***********************************************************************/


void InitBitAllocTables();


int		js_bound(int lay,int m_ext);

void	hdr_to_frps(FRAME* fr_ps);

void	WriteHdr(FRAME *fr_ps,FILE *s);

void	WriteBitAlloc(UINT bit_alloc[2][SBLIMIT],FRAME *f_p,FILE *s);

void	WriteScale(	UINT bit_alloc[2][SBLIMIT],
					UINT scfsi[2][SBLIMIT],
					UINT scalar[2][3][SBLIMIT],
					FRAME* fr_ps,
					FILE*	s);

void	WriteSamples(	int ch,
						UINT sample[SBLIMIT],
						UINT bit_alloc[SBLIMIT],
						FRAME* fr_ps,
						FILE *s	);	


int		SmpFrqIndex(LONG sRate,INT* version);

void*	mem_alloc(ULONG block,char* item);
void	mem_free(void** ptr_addr);
int		memcheck(char *array,int test,int num);

int		pick_table(FRAME*);
void	II_CRC_calc(FRAME*, UINT[2][SBLIMIT],UINT[2][SBLIMIT], UINT*);
void	putbits(BITSTREAM *bs,UINT val,INT N);
void	desalloc_buffer(BITSTREAM*);
void	put1bit(BITSTREAM*, int);
void	update_CRC(UINT, UINT, UINT*);

extern int  BitrateIndex(int, int, int);
extern int  SmpFrqIndex(long, int*);


#endif
