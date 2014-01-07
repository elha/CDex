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

#include "Common.h"
#include "Psycho_II.h"
#include "FFT.h"
#include "Tables.h"


//#define DYNALLOC

#include "common.h"
#include "Psycho_II.h"

//#define USE_FCB_TABLE 1

#ifdef USE_FCB_TABLE
	#define FCB_ENTRIES 1000
	FLOAT	FCB_TABLE[FCB_ENTRIES+1];
#endif



static const double  nmt = 5.5;

static const FLOAT crit_band[27] = 
{
	0,     100,    200, 300, 400, 510, 630,  770,
    920,   1080,  1270,1480,1720,2000,2320, 2700,
    3150,  3700,  4400,5300,6400,7700,9500,12000,
    15500,25000, 30000
};

static const FLOAT   bmax[27] = 
{
	20.0F, 20.0F, 20.0F, 20.0F, 20.0F, 17.0F, 15.0F,
    10.0F,  7.0F,  4.4F,  4.5F,  4.5F,  4.5F,  4.5F,
    4.5F,   4.5F,  4.5F,  4.5F,  4.5F,  4.5F,  4.5F,
    4.5F,   4.5F,  4.5F,  3.5F,  3.5F,  3.5F
};

static FLOAT   bmax_pow10[ 27 ] = { 0,}; 


static const int flush=(int)(384*3.0/2.0);
static const int syncsize = 1056;
static const int sync_flush = 1056- (int)(384*3.0/2.0);



void Init_Psycho_II
	(
		PSETTINGS	pSettings
	)
{
	unsigned int	i;
	int				j=0;
	FLOAT			freq_mult, bval_lo;
	double			temp1, temp2, temp3;

	INT				pLinesInPartition[CBANDS];
	float			fTmpArray[HBLKSIZE];
	PHEADER			pHeader=pSettings->pFrame->pHeader;
	PPSY_II_PARAMS	pParams=&(pSettings->Psy_II_params);
	FLOAT			sfreq=0.0f;
	INT				nSpreadNotZero=0;		   
	FLOAT			fSpreadFunc[ CBANDS ][ CBANDS ];


	for ( i=0; i< sizeof( bmax ) / sizeof( bmax[ 0 ] ); i++ )
	{
		bmax_pow10[ i ] = (FLOAT)( exp( (DOUBLE) -bmax[ i ] * LN_TO_LOG10 ) );
	}

	// get the sampling frequency in Hz
	sfreq=(FLOAT)(s_freq[pHeader->version][pHeader->sampling_frequency]*1000);

	#ifdef USE_FCB_TABLE
		for ( i=0; i<FCB_ENTRIES+1; i++ )
		{
			FCB_TABLE[i]= (float)(-0.434294482*log(i / FCB_ENTRIES * 0.5f)-0.301029996);
		}
	#endif

	// Pick the absolute threshold table
	switch( pHeader->sampling_frequency )
	{
		case 0: pParams->pfAbsThreshold = table_AbsThr1; break;	// 44100, 22050
		case 1: pParams->pfAbsThreshold = table_AbsThr2; break;	// 48000, 24000
		case 2: pParams->pfAbsThreshold = table_AbsThr0; break;	// 32000, 16000
		default:    
			MP2LibError("error, invalid sampling frequency index (%d)\n");
	}


	// calculate Hanning window coefficients
	for(i=0;i<BLKSIZE;i++)
	{
		pParams->window[i]=(float)(0.5*(1-cos(2.0*PI*(i-0.5)/BLKSIZE)));
	}



/*****************************************************************************
 * Initialization: Compute the following constants for use later             *
 *    partition[HBLKSIZE] = the partition number associated with each        *
 *                          frequency line                                   *
 *    cbval[CBANDS]       = the center (average) bark value of each          *
 *                          partition                                        *
 *    pLinesInPartition[CBANDS]    = the number of frequency lines in each partition  *
 *    tmn[CBANDS]         = tone masking noise                               *
 *****************************************************************************/
	/* compute fft frequency multiplicand */
	freq_mult = (float)sfreq/BLKSIZE;
 
	/* calculate fft frequency, then bval of each line (use fTmpArray[] as tmp storage)*/
	for(i=0;i<HBLKSIZE;i++)
	{
		temp1 = i*freq_mult;
		j = 1;

		while(temp1>crit_band[j])
			j++;

		fTmpArray[i]=(float)(j-1+(temp1-crit_band[j-1])/(crit_band[j]-crit_band[j-1]));
	}

	pParams->partition[0] = 0;

		/* temp2 is the counter of the number of frequency lines in each partition */
		temp2 = 1;
		pParams->cbval[0]=fTmpArray[0];
		bval_lo=fTmpArray[0];

		for(i=1;i<HBLKSIZE;i++)
		{
			if((fTmpArray[i]-bval_lo)>0.33)
			{
				pParams->partition[i]=pParams->partition[i-1]+1;
				pParams->cbval[pParams->partition[i-1]] = (float)(pParams->cbval[pParams->partition[i-1]]/temp2);
				pParams->cbval[pParams->partition[i]] = fTmpArray[i];
				bval_lo = fTmpArray[i];
				pLinesInPartition[pParams->partition[i-1]] = (int)temp2;
				temp2 = 1;
			}
			else
			{
				pParams->partition[i]=pParams->partition[i-1];
				pParams->cbval[pParams->partition[i]] += fTmpArray[i];
				temp2++;
			}
			//printf ("Partition[%3d]=%d\n",i,partition[i]);
		}

		pLinesInPartition[pParams->partition[i-1]] = (int)temp2;
		pParams->cbval[pParams->partition[i-1]] = (float)(pParams->cbval[pParams->partition[i-1]]/temp2);
 
		/************************************************************************
		* Now compute the spreading function, SpreadFunc[j][i], the value of the spread-*
		* ing function, centered at band j, for band i, store for later use    *
		************************************************************************/

//		memset(SpreadNotZero,sizeof(SpreadNotZero)*sizeof(int),0x00);
		memset( pParams->spreadfunc_matrix, 0, sizeof( pParams->spreadfunc_matrix ) );

		for(j=0;j<CBANDS;j++)
		{
			for(i=0;i<CBANDS;i++)
			{
				temp1 = (pParams->cbval[i] - pParams->cbval[j])*1.05;
				if(temp1>=0.5 && temp1<=2.5)
				{
					temp2 = temp1 - 0.5;
					temp2 = 8.0 * (temp2*temp2 - 2.0 * temp2);
				}
				else
					temp2 = 0;

				temp1 += 0.474;

				temp3 = 15.811389+7.5*temp1-17.5*sqrt((double) (1.0+temp1*temp1));

				if(temp3 <= -100) 
				{
					fSpreadFunc[i][j] = 0;
				}
				else 
				{
					nSpreadNotZero++;
					temp3 = (temp2 + temp3)*LN_TO_LOG10;
					fSpreadFunc[ i ][ j ] = (float)exp( temp3 );

					pParams->spreadfunc_matrix[ i ].position[ pParams->spreadfunc_matrix[ i ].num_coeffs   ] = j;
					pParams->spreadfunc_matrix[ i ].coeffs[ pParams->spreadfunc_matrix[ i ].num_coeffs++ ] = (float)exp( temp3 );

				}
			}
		}

		DebugPrintf("SpreadNotZero is %d \n",nSpreadNotZero);

		/* Calculate Tone Masking Noise values */
		for(j=0;j<CBANDS;j++)
		{
			FLOAT* pfSpread = fSpreadFunc[j];
			temp1 = 15.5 + pParams->cbval[j];
			pParams->tmn[j] = (temp1>24.5) ? temp1 : 24.5;
			/* Calculate normalization factors for the net spreading functions */
			temp1= 0;

			for(i=0;i<CBANDS;i++)
			{
				temp1 += *pfSpread++;
			}
			pParams->rnorm[j]=(float)temp1*pLinesInPartition[j];
		}
}


void DeInit_Psycho_II(void)
{

}


void Psycho_II
	(
		PSETTINGS	pSettings,
		FLOAT*		buffer,
		FLOAT		savebuf[1056],
		INT			chn,
		INT			lay,
		DOUBLE		snr32[SBLIMIT],
		DOUBLE		sfreq
	)
{
	unsigned int	i,k;
	int				j=0;
	double			tb, temp1, temp2, temp3;
	int				nIndex = 0;

	/* The static variables "r", "nNew", "old" and "oldest" have    */
	/* to be remembered for the unpredictability measure.  For "r" and        */
	/* "phi_sav", the first index from the left is the channel select and     */
	/* the second index is the "age" of the data.                             */

	static int     nNew = 0, old = 1, oldest = 0;

	float			grouped_c[ CBANDS ];
	float			grouped_e[ CBANDS ];
	float			snrtmp[ 2 ][ 32 ];
	float			nb[ CBANDS ];

	static float	wsamp_r[ BLKSIZE ];
	static float	energy[ BLKSIZE ];

	// Get pointer to the psycho acoustic model parameters
	PPSY_II_PARAMS pParams=&(pSettings->Psy_II_params);


	for( i=0; i<2; i++)
	{
		// clear grouped_c array
		memset( grouped_c, 0, sizeof( grouped_c ) );

		// clear grouped_e array
		memset( grouped_e, 0, sizeof( grouped_e ) );

		/*****************************************************************************
		 * Net offset is 480 samples (1056-576) for layer 2; this is because one must*
		 * stagger input data by 256 samples to synchronize psychoacoustic model with*
		 * filter bank outputs, then stagger so that center of 1024 FFT window lines *
		 * up with center of 576 "new" audio samples.                                *
		 *                                                                           *
		 * For layer 1, the input data still needs to be staggered by 256 samples,   *
		 * then it must be staggered again so that the 384 "new" samples are centered*
		 * in the 1024 FFT window.  The net offset is then 576 and you need 448 "new"*
		 * samples for each iteration to keep the 384 samples of interest centered   *
		 *****************************************************************************/

		for (j = 0; j < syncsize; j++)
		{
			if ( j < sync_flush )
			{
				// shift old data
				savebuf[j] = savebuf[ j + flush ];
			}
			else
			{
				// shift in new data to end of buffer
				savebuf[j] = *buffer++;
			}
			if (j < BLKSIZE)
			{
				// calculate FFT input data (apply Hanning window to sample data)
				wsamp_r[ j ] = pParams->window[ j ] * savebuf[j];
			}
		}

		/**********************************************************************
		 * Compute FFT                                                        *
		 *********************************************************************/
		MP2RealFour1024( wsamp_r, energy );

		// fft(wsamp_r,energy,phi,1024);

		// only update data "age" pointers after you are done with both channels 
		// for the layer 2 double computations, the pointers  are reset automatically 
		// on the second pass

		if( nNew == 0 )
		{
			nNew = 1;
			oldest = 1;
		}
		else
		{
			nNew = 0;
			oldest = 0;
		}

		/* toggle old */
		old = 1 -old;

		/**********************************************************************
		 * calculate the unpredictability measure, given energy[f] and phi[f] *
		 *********************************************************************/
		for( j = 0; j < HBLKSIZE; j++ )
		{
			float	fR1 = pParams->coeff_save[ chn ][ old    ][ j ].real;
			float	fR2 = pParams->coeff_save[ chn ][ oldest ][ j ].real;
			float	fI1 = pParams->coeff_save[ chn ][ old    ][ j ].imag;
			float	fI2 = pParams->coeff_save[ chn ][ oldest ][ j ].imag;
			float	fP1 = pParams->coeff_save[ chn ][ old    ][ j ].power;
			float	fP2 = pParams->coeff_save[ chn ][ oldest ][ j ].power;

			double	dPrime = ( 2.0 * fP1 - fP2 );

			double dM3 = dPrime / (fP1 * fP1 * fP2 );
			double dM1 = ( fR1 * fR1 - fI1 * fI1 );
			double dM2 = 2.0 * fR1 * fI1;
			
			pParams->coeff_save[ chn ][ nNew ][ j ].real  = wsamp_r[ 2 * j  ];
			pParams->coeff_save[ chn ][ nNew ][ j ].imag  = -wsamp_r[ 2 * j + 1 ];
			pParams->coeff_save[ chn ][ nNew ][ j ].power = (float)sqrt((double) energy[j]);

			temp1 = pParams->coeff_save[ chn ][ nNew ][ j ].real - 
				    ( dM1 * fR2 + dM2 * fI2 ) * dM3;

			temp2 = pParams->coeff_save[ chn ][ nNew ][ j ].imag - 
				    ( -dM1 * fI2 + dM2 * fR2 ) * dM3;

			temp3 = pParams->coeff_save[ chn ][ nNew ][ j ].power + fabs( dPrime );

			if(temp3 != 0.0 )
			{
				temp3 = sqrt( temp1 * temp1 + temp2 * temp2 ) / temp3;
			}
			else
			{
				temp3 = 0.0;
			}

			/*****************************************************************************
			 * Calculate the grouped, energy-weighted, unpredictability measure,         *
			 * grouped_c[], and the grouped energy. grouped_e[]                          *
			 *****************************************************************************/

			grouped_e[ pParams->partition[ j ] ] += energy[ j ];
			grouped_c[ pParams->partition[ j ] ] += (float)( energy[ j ] * temp3 );
		}


		/*****************************************************************************
		 * convolve the grouped energy-weighted unpredictability measure             *
		 * and the grouped energy with the spreading function, SpreadFunc[j][k]      *
		 *****************************************************************************/
        for( j = 0; j < CBANDS; j++ )
		{
			FLOAT				fSumEcb	= 0.0f;
			FLOAT				fSumCb	= 0.0f;
			FLOAT				fCB		= 0.0f;
			FLOAT				fECB	= 0.0f;
			PPSY_II_SPRD_MATRIX pMatrix = &pParams->spreadfunc_matrix[ j ];

			for ( k = 0; k < pMatrix->num_coeffs; k++ )
			{
				int pos     = pMatrix->position[ k ];
				FLOAT coeff = pMatrix->coeffs[ k ];

				fSumEcb += coeff * grouped_e[ pos ];
				fSumCb += coeff * grouped_c[ pos ];

			}

			if( 0 != fSumEcb )
			{
			   fECB = fSumEcb;
			   fCB  = fSumCb / fSumEcb;
			}


			/*****************************************************************************
			* Calculate the required SNR for each of the frequency partitions           *
			*         this whole section can be accomplished by a table lookup          *
			*****************************************************************************/

			// Clip fCB value between 0.05 and 0.5
			if( fCB < .05f )
			{
				fCB = 0.05f;
			}
			else if(fCB>.5)
			{
				fCB = 0.5f;
			}

			#ifdef USE_FCB_TABLE
				tb=FCB_TABLE[(int)(fCB*FCB_ENTRIES+0.5)];
			#else
				tb = -0.434294482*log(fCB)-0.301029996;
			#endif


			/* linear interpolation */
			fCB = (float)( pParams->tmn[ j ] * tb + nmt * ( 1.0 - tb ) );

			k = (WORD)( pParams->cbval[ j ] + 0.5 );


			if ( fCB <= bmax[ k ] )
			{
				fCB = bmax_pow10[ k ];
			}
			else
			{
				fCB = (float)(exp((double) -fCB*LN_TO_LOG10));
			}

			/*****************************************************************************
			* Calculate the permissible noise energy level in each of the frequency     *
			* partitions. Include absolute threshold and pre-echo controls              *
			*         this whole section can be accomplished by a table lookup          *
			*****************************************************************************/
			if( pParams->rnorm[ j ] )
			{
				nb[j] = fECB * fCB / pParams->rnorm[j];
			}
			else
			{
				nb[j] = 0;
			}
		}


		 /****************************************************************************
		 * Translate the 512 threshold values to the 32 filter bands of the coder    *
		 *****************************************************************************/

		// For the lower frequency bands, determine the sum of all energy within a 
		// subbands, and also the lowest threshold value. The sum of the energies 
		// within a band are divided by the lowest threshold value (times the number
		// of energy values within that subband
		nIndex =0;

		for( j = 0; j < 208/16; j++ )
		{
			FLOAT fMinThreshold = 60802371420160.0;
			FLOAT fSumEnergy = 0.0;

			for( k = 0; k < 17; k++ )
			{
				FLOAT fThreshold = nb[ pParams->partition[ nIndex ] ];

				if ( fThreshold < pParams->pfAbsThreshold[ nIndex ] )
				{
					fThreshold = pParams->pfAbsThreshold[ nIndex ];
				}

				// Keep track of minimum threshold
				if( fMinThreshold > fThreshold )
				{
					fMinThreshold = fThreshold;
				}

				// Add current energy to fSumEnergy
				fSumEnergy += energy[ nIndex ];

				nIndex++;
			}

			nIndex--;

			// Calculate average energy value
			snrtmp[ i ][ j ] = fSumEnergy / ( fMinThreshold * 17.0f );
		}

		// Calculate the average energy in the higher energy bands
		// They are normalized to the average thresholds
		nIndex = 208;

		// Calculate the average energy in the higher energy bands
		// They are normalized to the average thresholds
		for( j = 208/16; j< (HBLKSIZE-1)/16; j++ )
		{
			FLOAT fSumThreshold = 0.0;
			FLOAT fSumEnergy = 0.0;

			for( k = 0; k < 17; k++ )
			{
				FLOAT fThreshold = nb[ pParams->partition[ nIndex ] ];

				// Add (minimum) threshold value to threshold sum
				if ( fThreshold < pParams->pfAbsThreshold[ nIndex ] )
				{
					fSumThreshold += pParams->pfAbsThreshold[ nIndex ];
				}
				else
				{
					fSumThreshold+= fThreshold;
				}

				// Add current energy to fSumEnergy
				fSumEnergy+= energy[ nIndex ];

				nIndex++;
			}

			nIndex--;

			// Calculate average energy value
			snrtmp[ i ][ j ] = fSumEnergy / fSumThreshold;
		}
	}



	/************************************************
	 * Get the max SNR of the two iteration loops   *
	 ***********************************************/
	 
	for(i=0; i<32; i++)
	{
		if ( snrtmp[ 0 ][ i ] > snrtmp[ 1 ][ i ] )
		{
			// Normalize SNR to dB?
			snr32[ i ]= (4.342944819 * log( snrtmp[ 0 ][ i ] ) );
		}
		else
		{
			// Normalize SNR to dB?
			snr32[ i ]= (4.342944819 * log( snrtmp[ 1 ][ i ] ) );
		}
	}

	/********************************************
	 * End of Psychoacuostic calculation loop   *
	 *******************************************/
}


