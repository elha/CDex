/*
** Copyright (C) 2000 Albert L. Faber
** 
** Integration of Lame/GoGo's FHT assembly routines by DSPguru :
** http://DSPguru.doom9.org.
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

#include <math.h>
#include "types.h"

static void fht(float *fz, int n);
static int  has_i387 ( void );

static void TestFour1024();
static void Oldfour512(float* pfData, int nDummy);


static int  has_i387 ( void )
{
#ifdef HAVE_NASM 
    return 1;
#else
    return 0;   /* don't know, assume not */
#endif
}    


void		(*four512)		(float*,int );
#ifdef HAVE_NASM
extern void fht_SSE			(float *fz, int n);
extern void fht_FPU			(float *fs, int n);
extern void fht_3DN			(float *fs, int n);
extern int	has_3DNow_nasm	(void);
extern int	has_SIMD_nasm	(void);
extern int	has_i387		(void);
#endif


void fft_init()
{
#ifdef HAVE_NASM_NO
	if (has_3DNow_nasm())
	{
		four512 = fht_3DN;
	}
	else if (has_SIMD_nasm())
	{
		four512 = fht_SSE;
	}
	else if ( has_i387() ) 
	{
		four512 = fht_FPU;
	}
	else
#endif
	{
		four512 = Oldfour512;
//		four512 = fht;
	}
//TestFour1024();
//  mov   dword ptr [four512],offset fht_SSE 

}

#define TRI_SIZE (5-1) /* BLKSIZE =  4**5 */

static const float costab[TRI_SIZE*2] = {
  9.238795325112867e-01f, 3.826834323650898e-01f,
  9.951847266721969e-01f, 9.801714032956060e-02f,
  9.996988186962042e-01f, 2.454122852291229e-02f,
  9.999811752826011e-01f, 6.135884649154475e-03f
};

#ifdef M_SQRT2
# define        SQRT2                   M_SQRT2
#else
# define        SQRT2                   1.41421356237309504880f
#endif

static void fht(float *fz, int n)
{
    const float *tri = costab;
    int           k4;
    float *fi, *fn, *gi;

    n <<= 1;        /* to get BLKSIZE, because of 3DNow! ASM routine */
    fn = fz + n;
    k4 = 4;
    do {
	float s1, c1;
	int   i, k1, k2, k3, kx;
	kx  = k4 >> 1;
	k1  = k4;
	k2  = k4 << 1;
	k3  = k2 + k1;
	k4  = k2 << 1;
	fi  = fz;
	gi  = fi + kx;
	do {
	    float f0,f1,f2,f3;
	    f1      = fi[0]  - fi[k1];
	    f0      = fi[0]  + fi[k1];
	    f3      = fi[k2] - fi[k3];
	    f2      = fi[k2] + fi[k3];
	    fi[k2]  = f0     - f2;
	    fi[0 ]  = f0     + f2;
	    fi[k3]  = f1     - f3;
	    fi[k1]  = f1     + f3;
	    f1      = gi[0]  - gi[k1];
	    f0      = gi[0]  + gi[k1];
	    f3      = SQRT2  * gi[k3];
	    f2      = SQRT2  * gi[k2];
	    gi[k2]  = f0     - f2;
	    gi[0 ]  = f0     + f2;
	    gi[k3]  = f1     - f3;
	    gi[k1]  = f1     + f3;
	    gi     += k4;
	    fi     += k4;
	} while (fi<fn);
	c1 = tri[0];
	s1 = tri[1];
	for (i = 1; i < kx; i++) {
	    float c2,s2;
	    c2 = 1 - (2*s1)*s1;
	    s2 = (2*s1)*c1;
	    fi = fz + i;
	    gi = fz + k1 - i;
	    do {
		float a,b,g0,f0,f1,g1,f2,g2,f3,g3;
		b       = s2*fi[k1] - c2*gi[k1];
		a       = c2*fi[k1] + s2*gi[k1];
		f1      = fi[0 ]    - a;
		f0      = fi[0 ]    + a;
		g1      = gi[0 ]    - b;
		g0      = gi[0 ]    + b;
		b       = s2*fi[k3] - c2*gi[k3];
		a       = c2*fi[k3] + s2*gi[k3];
		f3      = fi[k2]    - a;
		f2      = fi[k2]    + a;
		g3      = gi[k2]    - b;
		g2      = gi[k2]    + b;
		b       = s1*f2     - c1*g3;
		a       = c1*f2     + s1*g3;
		fi[k2]  = f0        - a;
		fi[0 ]  = f0        + a;
		gi[k3]  = g1        - b;
		gi[k1]  = g1        + b;
		b       = c1*g2     - s1*f3;
		a       = s1*g2     + c1*f3;
		gi[k2]  = g0        - a;
		gi[0 ]  = g0        + a;
		fi[k3]  = f1        - b;
		fi[k1]  = f1        + b;
		gi     += k4;
		fi     += k4;
	    } while (fi<fn);
	    c2 = c1;
	    c1 = c2 * tri[0] - s1 * tri[1];
	    s1 = c2 * tri[1] + s1 * tri[0];
        }
	tri += 2;
    } while (k4<n);
}


float TWIDDLE512R[256];
float TWIDDLE512I[256];


#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr


void CreateTwiddle512()
{
	int i;

	for (i=0;i<sizeof(TWIDDLE512R)/sizeof(TWIDDLE512R[0]);i++)
	{
		// TWIDDLE = exp(-2*i*pi/N)
		// Thus TWIDDLE^n=TWIDDLE[n]=exp(-2*pi*n/N)
		TWIDDLE512R[i]=(float)cos(2*3.14159265355129793/512*(double)i);
		TWIDDLE512I[i]=(float)sin(2*3.14159265355129793/512*(double)i);
	}
}

static void Oldfour512(float* pfData, int nDummy)
{
	const int nL=9;
	int		n,n1,i,j,l,k,nTwoI;
	int		nLE,nLE2,nID;
		float	fIr,fIi;
	float	fBr,fBi;
	float	tempr;			// Temp for swap

	static int init=0;

	if (init==0)
	{
		CreateTwiddle512();
		init++;
	}

	// n= 2^nL
	n=1<<nL;
	n1=n-1;

	// Set j to 1
	j = n >> 1;

	// No need to touch first & last
	for(i=1;i<n1;i++)
	{	
		// Don't swap twice!
		if(i<j)
		{	
			SWAP(pfData[2*i  ],pfData[2*j  ]);
			SWAP(pfData[2*i+1],pfData[2*j+1]);
		}

		// Scan counter from left looking for set bits. Clear
		// each with a subtract and propagate a carry
		// to the right. Fall out when we find a 0 bit.

		for(k=n>>1;k <= j;k >>= 1)
			j -= k;

		// Add in the propagated carry
		j += k;
	}
#define UNROLLFIRST
#ifdef UNROLLFIRST
	// Do first stage separately
	for (i=0;i<n;i+=2)
	{
		nTwoI=i*2;
		nID=nTwoI+2;

		fIr=pfData[nID  ];
		fIi=pfData[nID+1];

		// fB=fI thus to not assign, but add/subtract value directly

		// Complex multiplication and subtract 
		// of pfData[nID]=pfData[i]-pfData[nID]*fA
		pfData[nID  ]=pfData[nTwoI  ]-fIr;
		pfData[nID+1]=pfData[nTwoI+1]-fIi;

		// Complex multiplication and subtract 
		// of pfData[i]=pfData[i]+pfData[nID]*fA
		pfData[nTwoI  ]+=fIr;
		pfData[nTwoI+1]+=fIi;

	}


	for (l=1;l<nL;l++)
#else
	for (l=0;l<nL;l++)
#endif
	{
		int nTwiddleIndex=0;

		nLE=1<<(l+1);			// nLE=2^l

		nLE2=nLE/2;

#ifdef TESTTWIDDLE
		float	fAngle;
		float	fAr,fAi;
		float	fWr,fWi;

		fAngle=-2*3.1415926/(float)nLE;
		fAr=1.0;
		fAi=0.0;

		fWr=cos(fAngle);
		fWi=sin(fAngle);
#endif


#define UNROLLSECOND
#ifdef UNROLLSECOND
		j=0;
		for (i=0;i<n;i+=nLE)
		{
			// Do first stage separately
			nTwoI=2*i;
			nID=nTwoI+nLE2*2;

			fIr=pfData[nID  ];
			fIi=pfData[nID+1];

			// fB=fI thus to not assign, but add/subtract value directly

			// Complex multiplication and subtract 
			// of pfData[nID]=pfData[i]-pfData[nID]*fA
			pfData[nID  ]=pfData[nTwoI  ]-fIr;
			pfData[nID+1]=pfData[nTwoI+1]-fIi;

			// Complex multiplication and subtract 
			// of pfData[i]=pfData[i]+pfData[nID]*fA
			pfData[nTwoI  ]+=fIr;
			pfData[nTwoI+1]+=fIi;
		}

		// Adjust Twiddle Factor Index when unrolled
		nTwiddleIndex+=n/nLE;

		for (j=1;j<nLE2;j++)
#else
		for (j=0;j<nLE2;j++)
#endif
		{

			for (i=j;i<n;i+=nLE)
			{
				nTwoI=i*2;

				nID=nTwoI+nLE2*2;

				fIr=pfData[nID  ];
				fIi=pfData[nID+1];

				// Complex multiplication
				// fB=fI*fA
				fBr=fIr*TWIDDLE512R[nTwiddleIndex]-fIi*TWIDDLE512I[nTwiddleIndex];
				fBi=fIr*TWIDDLE512I[nTwiddleIndex]+fIi*TWIDDLE512R[nTwiddleIndex];

				// Complex multiplication and subtract 
				// of pfData[nID]=pfData[i]-pfData[nID]*fA
				pfData[nID  ]=pfData[nTwoI  ]-fBr;
				pfData[nID+1]=pfData[nTwoI+1]-fBi;

				// Complex multiplication and subtract 
				// of pfData[i]=pfData[i]+pfData[nID]*fA
				pfData[nTwoI  ]+=fBr;
				pfData[nTwoI+1]+=fBi;

			}
			nTwiddleIndex+=n/nLE;
		}
	}
}


void MP2RealFour1024(float* pfData,float* pfEnergy)
{
	const int n=BLKSIZE;
	const int n3=n+1;

	unsigned long i1,i2,i3,i4;
	int	  i;
	float h1r,h1i,h2r,h2i;
	float wr,wi,wpr,wpi,wtemp,theta;

	theta=3.141592653589793F/(float) (n>>1);

	four512(pfData,512);

	wtemp=(float)sin(0.5*theta);
	wpr = -2.0F*wtemp*wtemp;
	wpi=(float)sin(theta);
	wr=1.0F+wpr;
	wi=wpi;

	for (i=1;i<(n>>2);i++)
	{
		// Get H[n] => RE index=2*i IM index=2*i+1
		i1=i+i;
		i2=i1+1;

		// Get H[N/2-n] => RE N-2*i IM index=N-2*i+1
		i3=n3-i2;
		i4=i3+1;

		h1r= 0.5f*(pfData[i1]+pfData[i3]);
		h1i= 0.5f*(pfData[i2]-pfData[i4]);

		h2r= 0.5f*(pfData[i2]+pfData[i4]);
		h2i=-0.5f*(pfData[i1]-pfData[i3]);

		pfData[i1]=h1r+wr*h2r-wi*h2i;
		pfData[i2]=h1i+wr*h2i+wi*h2r;

		pfData[i3]=  h1r-wr*h2r+wi*h2i;
		pfData[i4]= -h1i+wr*h2i+wi*h2r;

		wr=(wtemp=wr)*wpr-wi*wpi+wr;
		wi=wi*wpr+wtemp*wpi+wi;
	}
	pfData[0] = (h1r=pfData[0])+pfData[1];
	pfData[1] = h1r-pfData[1];

	i=0;

	{
		float* pfE=pfEnergy;
		float* pfReal=&pfData[0];
		float* pfImag=&pfData[1];

		while (i<n)
		{
			*pfE=*pfReal * *pfReal+*pfImag * *pfImag;

			if (*pfE<= 0.0005F)
			{
				*pfE=0.0005F;
			}

			pfE++;
			pfReal+=2;
			pfImag+=2;
			i+=2;
		}
	}
}



static void TestFour1024()
{
	int i = 0;
	FILE* fp = NULL;
	float pfInput[ BLKSIZE ];
	float pfOutput[ BLKSIZE ];
	
	for ( i=0; i< BLKSIZE; i++ )
	{
		pfInput[i] = (float)( 5 * sin( 30 * 3.14159 * i / BLKSIZE ) + 2 * cos( 12 * 3.14159 * i / BLKSIZE ) );
//		pfInput[i] = (float)( sin( 30 * 3.14159 * i / BLKSIZE ));
	}

	MP2RealFour1024( pfInput, pfOutput );

	fp = fopen( "c:\\temp\\four.txt", "w" );

	for ( i=0; i< BLKSIZE/2; i++ )
	{
		fprintf( fp, "%d\t%f\t%f\t%f\t%f\n", i,pfInput[2*i],pfInput[2*i+1],sqrt(pfOutput[i]), atan2(pfInput[2*i],pfInput[2*i+1]) );
	}
	fclose( fp );
}
