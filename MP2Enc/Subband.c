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

#include "Subband.h"
#include "Tables.h"

static void IDCT32(DOUBLE* xin,DOUBLE* xout );
static void create_dct_matrix(double filter[16][32]);


/************************************************************************
*
* window_subband()
*
* PURPOSE:  Overlapping window on PCM samples
*
* SEMANTICS:
* 32 16-bit pcm samples are scaled to fractional 2's complement and
* concatenated to the end of the window buffer #x#. The updated window
* buffer #x# is then windowed by the analysis window #c# to produce the
* windowed sample #z#
*
************************************************************************/
/************************************************************************
*
* filter_subband()
*
* PURPOSE:  Calculates the analysis filter bank coefficients
*
* SEMANTICS:
*      The windowed samples #z# is filtered by the digital filter matrix #m#
* to produce the subband samples #s#. This done by first selectively
* picking out values from the windowed samples, and then multiplying
* them by the filter matrix, producing 32 subband samples.
*
************************************************************************/

static void create_dct_matrix(double filter[16][32])
{
  register int i,k;
 
  for (i=0; i<16; i++)
    for (k=0; k<32; k++) {
      if ((filter[i][k] = 1e9*cos((double)((2*i+1)*k*PI64))) >= 0)
	modf(filter[i][k]+0.5, &filter[i][k]);
      else
	modf(filter[i][k]-0.5, &filter[i][k]);
      filter[i][k] *= 1e-9;
    }
}

static void IDCT32(DOUBLE* xin,DOUBLE* xout )
{
	register int	i;
	static int		bInit=0;
	DOUBLE			s0;
	DOUBLE			s1;
	static DOUBLE	m[16][32];

	if( !bInit )
	{
		create_dct_matrix( m );
		bInit++;
	}

	for( i=0; i<16; i++ )
	{
		s0 = s1 = 0.0;

//#define DONOTUNROLLIDCT
#ifdef DONOTUNROLLIDCT
		for( j=0; j<32; j+=2 )
		{
			s0 += m[i][j]*xin[j+0];
			s1 += m[i][j+1]*xin[j+1];
		}
#else
		s0 += m[i][ 0]*xin[ 0];
		s1 += m[i][ 1]*xin[ 1];
		s0 += m[i][ 2]*xin[ 2];
		s1 += m[i][ 3]*xin[ 3];
		s0 += m[i][ 4]*xin[ 4];
		s1 += m[i][ 5]*xin[ 5];
		s0 += m[i][ 6]*xin[ 6];
		s1 += m[i][ 7]*xin[ 7];
		s0 += m[i][ 8]*xin[ 8];
		s1 += m[i][ 9]*xin[ 9];
		s0 += m[i][10]*xin[10];
		s1 += m[i][11]*xin[11];
		s0 += m[i][12]*xin[12];
		s1 += m[i][13]*xin[13];
		s0 += m[i][14]*xin[14];
		s1 += m[i][15]*xin[15];
		s0 += m[i][16]*xin[16];
		s1 += m[i][17]*xin[17];
		s0 += m[i][18]*xin[18];
		s1 += m[i][19]*xin[19];
		s0 += m[i][20]*xin[20];
		s1 += m[i][21]*xin[21];
		s0 += m[i][22]*xin[22];
		s1 += m[i][23]*xin[23];
		s0 += m[i][24]*xin[24];
		s1 += m[i][25]*xin[25];
		s0 += m[i][26]*xin[26];
		s1 += m[i][27]*xin[27];
		s0 += m[i][28]*xin[28];
		s1 += m[i][29]*xin[29];
		s0 += m[i][30]*xin[30];
		s1 += m[i][31]*xin[31];
#endif
		xout[i] = s0+s1;
		xout[31-i] = s0-s1;
	}
}

// Window and filter subband
//
// Step 1:
//		Put new samples into input buffer (32 samples)
//		thus:
//			// Shift samples down 32 samples
//		    for (i=511;i>=32;i--) 
//			{
//				pdInputFifo[nChannel][i]=pdInputFifo[nChannel][i-32];
//			}
//		    for (;i>=0;i--) 
//			{
//				// Put in new data samples
//				pdInputFifo[nChannel][i  ]=(double)*(*pBuffer)++ *dOneOverScale;
//			}
// Step 2: 
//		Calculate z[i] and store directly in y[i%64]
//		thus:
//		
//			//clear y array
//			memset(y,0,64*sizeof(double));
//
//			// multiplt buffered input with window, add directly to y[i]
//		    for (i=0;i<HAN_SIZE;i++) 
//			{
//				// Apply hanning window
//				y[i%64]+=pdInputFifo[nChannel][i] * enwindow[i];
//			}
// Step 3:
//		Multiply with DFT matrix 
//
//			for (i=0;i<SBLIMIT;i++)
//			{
//				for (j=0, s[i]= 0;j<64;j++)
//				{
//					s[i] += (*m)[i][j] * y[j];
//				}
//		}
//		m[i][j]=cos((2*i+1)*(j-16)*PI/64)
// That is all
//

// WINDOW AND FILTER COMBINED
void waf_subband( FLOAT z[ 384 ], INT ch, DOUBLE s[ SCALE_BLOCK ][ SBLIMIT ] )
{
	INT		i,j,m,k;

	DOUBLE t[12];
	DOUBLE y[12][64];
	DOUBLE c0;
	DOUBLE yprime[32];
	static double	dOneOverScale=1.0/SCALE;


	static DOUBLE xk2[2][864]={0,};

	DOUBLE*	xk=&xk2[ch][0];

	// shift buffer, shift 384 samples
	for (i=863;i>=384;i--)
		xk[i] = xk[i-384];

	// shift in new samples
	for (i=383;i>=0;i--)
		xk[i] = z[383-i]*dOneOverScale;

	for (j=0;j<64;j++)
	{
		memset(t,0,sizeof(t));

		for (i=0;i<8;i++)
		{
			m = i*64 + j;

			c0 = enwindow[m];

			t[0]  += c0 * xk[m+352];
			t[1]  += c0 * xk[m+320];
			t[2]  += c0 * xk[m+288];
			t[3]  += c0 * xk[m+256];
			t[4]  += c0 * xk[m+224];
			t[5]  += c0 * xk[m+192];
			t[6]  += c0 * xk[m+160];
			t[7]  += c0 * xk[m+128];
			t[8]  += c0 * xk[m+ 96];
			t[9]  += c0 * xk[m+ 64];
			t[10] += c0 * xk[m+ 32];
			t[11] += c0 * xk[m    ];
        }

		for (i=0;i<12;i++)
		{
			y[i][j] = t[i];
		}
	}

	for (k=0;k<12;k++)
	{
		DOUBLE* ya=y[k];

		yprime[0] = ya[16];

		for( i=1; i<=16; i++ )
			yprime[i] = ya[i+16]+ya[16-i];
		for( i=17; i<=31; i++ ) 
			yprime[i] = ya[i+16]-ya[80-i];
		
		IDCT32(yprime,s[k]);
	}
}
