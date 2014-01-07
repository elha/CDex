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
 
#include <float.h>

#include "Common.h"
#include "Layer2.h"
#include "AvailableBits.h"
#include "Subband.h"

#define VBRDEBUG


enum byte_order NativeByteOrder = order_littleEndian;


// Local Function proto types
static void IDCT32(DOUBLE* xin,DOUBLE* xout );


static void VBR_maxmnr(		DOUBLE		mnr[2][SBLIMIT],
							CHAR		used[2][SBLIMIT],
							INT			sblimit,
							INT			stereo,
							INT*		min_sb,
							INT*		min_ch,
							PSETTINGS	pSettings );

static int II_bits_for_nonoise (	DOUBLE			perm_smr[2][SBLIMIT],
									UINT			scfsi[2][SBLIMIT],
									PFRAME			pFrame,
									INT				vbrlevel );



/************************************************************************
* encode_info()
*
* PURPOSE:  Puts the syncword and header pHeaderrmation on the output
* bitstream.
*
************************************************************************/
 
void encode_info(FRAME *pFrame,BITSTREAM *pBitstream)
{
	PHEADER pHeader = pFrame->pHeader;

	/* Total heade is 32 bits in big endian representation */
	/* ID                            POSITION         SIZE */
    /* frame sync                       31-21           12 */
    /* MPEG_ID                          20-19            2 */
    /* LAYER                            18-17            2 */
    /* CRC disabled                     16-16            1 */
    /* Bitrate index                    15-12            4 */
    /* Sampling rate index              11-10            2 */
    /* Padding bit                       9- 9            1 */
    /* Padding bit                       9- 9            1 */
    /* Private bit                       8- 8            1 */
    /* Channel mode                      7- 6            2 */
    /* Join stereo extention             5- 4            2 */
    /* Copyright bit                     3- 3            1 */
    /* Original bit                      2- 2            1 */
    /* Emphasis                          1- 0            2 */

	putbits(pBitstream,0xfff,12);					// syncword 12 bits
	put1bit(pBitstream,pHeader->version);			// ID        1 bit
	putbits(pBitstream,4-pHeader->lay,2);			// HEADER     2 bits
	put1bit(pBitstream,!pHeader->bCRC);				// bit set => no err prot
	putbits(pBitstream,pHeader->bitrate_index,4);
	putbits(pBitstream,pHeader->sampling_frequency,2);
	put1bit(pBitstream,pHeader->padding);
	put1bit(pBitstream,pHeader->bPrivate);             // private_bit
	putbits(pBitstream,pHeader->mode,2);
	putbits(pBitstream,pHeader->mode_ext,2);
	put1bit(pBitstream,pHeader->copyright);
	put1bit(pBitstream,pHeader->original);
	putbits(pBitstream,pHeader->emphasis,2);
}

 
/************************************************************************
*
* mod()
*
* PURPOSE:  Returns the absolute value of its argument
*
************************************************************************/
 
__inline double mod(double a)
{
    return (a > 0) ? a : -a;
}
 

 
/************************************************************************
*
* encode_CRC
*
************************************************************************/
 
void encode_CRC(UINT crc,BITSTREAM* pBitstream)
{
   putbits(pBitstream, crc, 16);
}



/******************************** Layer II ******************************/
 
void II_scale_factor_calc(	double	sb_sample[][3][SCALE_BLOCK][SBLIMIT],
							UINT	scalar[][3][SBLIMIT],
							int		stereo,
							int		sblimit)
{
	int i,j, k,t;
//	double s[SBLIMIT];
 
	for (k=0;k<stereo;k++) 
	{
		for (t=0;t<3;t++)
		{
			// LOOP THROUGH THE AVAILABLE SIDE BANDS
			for (i=0;i<sblimit;i++)
			{
				double dAbs=0.0f;
				double dMax=0.0f;

				int nIdx=0;
				int	nFirst=0;
				int	nLast=(SCALE_RANGE-1);
				int	q=0;

				// DETERMINE MAX ABS VALUE FOR EACH SCALE_BLOCK (12 SAMPLES)
				for (j=0;j<SCALE_BLOCK;j++)
				{
					dAbs= fabs(sb_sample[k][t][j][i]);

					// KEEP TRACK OF MAX VALUE
					if (dAbs > dMax)
					{
						dMax = dAbs;
					}
				}

				// FIND NEAREST SCALE FACTOR
				for (q=0;q<6;q++)
				{
					nIdx=(nLast+nFirst)/2;
					if (dMax<multiple[nIdx])	nFirst=nIdx;
					else						nLast=nIdx;
				}
				if (dMax>multiple[nIdx])	
					nIdx--;

				// ASSIGN INDEX
				scalar[k][t][i] = nIdx;

			}
/* 
			for (i=0;i<sblimit;i++)
			{
				scalar[k][t][i]=0;


				for (j=SCALE_RANGE-2;j>=0;j--)    // $A 6/16/92
				{
					if (s[i] <= multiple[j])
					{
						scalar[k][t][i] = j;
						break;
					}
				}

			}
*/
			for (i=sblimit;i<SBLIMIT;i++)
			{
				scalar[k][t][i] = SCALE_RANGE-1;
			}
		}
	}
}

/************************************************************************
*
* pick_scale  (Layer II)
*
* PURPOSE:For each subband, puts the smallest scalefactor of the 3
* associated with a frame into #max_sc#.  This is used
* used by Psychoacoustic Model I.
* (I would recommend changin max_sc to min_sc)
*
************************************************************************/
 
void pick_scale(	UINT scalar[2][3][SBLIMIT],
					FRAME *pFrame,
					double max_sc[2][SBLIMIT]
				)
{
  int i,j,k;
  UINT	max;
  int stereo  = pFrame->nChannels;
  int sblimit = pFrame->sblimit;
 
  for (k=0;k<stereo;k++)
    for (i=0;i<sblimit;max_sc[k][i] = multiple[max],i++)
      for (j=1, max = scalar[k][0][i];j<3;j++)
         if (max > scalar[k][j][i]) max = scalar[k][j][i];
  for (i=sblimit;i<SBLIMIT;i++) max_sc[0][i] = max_sc[1][i] = 1E-20;
}


/************************************************************************
*
* II_transmission_pattern (Layer II only)
*
* PURPOSE:For a given subband, determines whether to send 1, 2, or
* all 3 of the scalefactors, and fills in the scalefactor
* select pHeaderrmation accordingly
*
* SEMANTICS:  The subbands and channels are classified based on how much
* the scalefactors changes over its three values (corresponding
* to the 3 sets of 12 samples per subband).  The classification
* will send 1 or 2 scalefactors instead of three if the scalefactors
* do not change much.  The scalefactor select pHeaderrmation,
* #scfsi#, is filled in accordingly.
*
************************************************************************/
 
void II_transmission_pattern(
								UINT	scalar[2][3][SBLIMIT],
								UINT	scfsi[2][SBLIMIT],
								FRAME *pFrame,
								int		sblimit
							)
{
   int stereo  = pFrame->nChannels;
//   int sblimit = pFrame->sblimit;
   int dscf[2];
   int nClass[2],i,j,k;
static int pattern[5][5] = {0x123, 0x122, 0x122, 0x133, 0x123,
                            0x113, 0x111, 0x111, 0x444, 0x113,
                            0x111, 0x111, 0x111, 0x333, 0x113,
                            0x222, 0x222, 0x222, 0x333, 0x123,
                            0x123, 0x122, 0x122, 0x133, 0x123};
 
	for (k=0;k<stereo;k++)
	{
		for (i=0;i<sblimit;i++) 
		{
			// calculate difference between scale factors
			dscf[0] =  (scalar[k][0][i]-scalar[k][1][i]);
			dscf[1] =  (scalar[k][1][i]-scalar[k][2][i]);

			// determine class
			for (j=0;j<2;j++)
			{
				if (dscf[j]<=-3) nClass[j] = 0;
				else if (dscf[j] > -3 && dscf[j] <0) nClass[j] = 1;
				else if (dscf[j] == 0) nClass[j] = 2;
				else if (dscf[j] > 0 && dscf[j] < 3) nClass[j] = 3;
				else nClass[j] = 4;
			}

			switch (pattern[nClass[0]][nClass[1]])
			{
				case 0x123 :    scfsi[k][i] = 0;
				break;
				case 0x122 :    scfsi[k][i] = 3; 
								scalar[k][2][i] = scalar[k][1][i];
				break;
				case 0x133 :    scfsi[k][i] = 3;
								scalar[k][1][i] = scalar[k][2][i];
				break;
				case 0x113 :    scfsi[k][i] = 1;
								scalar[k][1][i] = scalar[k][0][i];
				break;
				case 0x111 :    scfsi[k][i] = 2;
								scalar[k][1][i] = scalar[k][2][i] = scalar[k][0][i];
				break;
				case 0x222 :    scfsi[k][i] = 2;
								scalar[k][0][i] = scalar[k][2][i] = scalar[k][1][i];
				break;
				case 0x333 :    scfsi[k][i] = 2;
								scalar[k][0][i] = scalar[k][1][i] = scalar[k][2][i];
				break;
				case 0x444 :    scfsi[k][i] = 2;
								if (scalar[k][0][i] > scalar[k][2][i])
									scalar[k][0][i] = scalar[k][2][i];
								scalar[k][1][i] = scalar[k][2][i] = scalar[k][0][i];
			}
		}
	}
}


/************************************************************************
*
* II_encode_scale (Layer II)
*
* PURPOSE:The encoded scalar factor pHeaderrmation is arranged and
* queued into the output fifo to be transmitted.
*
* For Layer II, the three scale factors associated with
* a given subband and channel are transmitted in accordance
* with the scfsi, which is transmitted first.
*
************************************************************************/

void II_encode_scale(
						UINT				bit_alloc[2][SBLIMIT],
						UINT				scfsi[2][SBLIMIT],
						UINT				scalar[2][3][SBLIMIT],
						FRAME*		pFrame,
						BITSTREAM*	pBitstream )
{
   int stereo  = pFrame->nChannels;
   int sblimit = pFrame->sblimit;
   int jsbound = pFrame->jsbound;
   int i,j,k;
 
   for (i=0;i<sblimit;i++) for (k=0;k<stereo;k++)
     if (bit_alloc[k][i])  putbits(pBitstream,scfsi[k][i],2);
 
   for (i=0;i<sblimit;i++) for (k=0;k<stereo;k++)
     if (bit_alloc[k][i])  /* above jsbound, bit_alloc[0][i] == ba[1][i] */
        switch (scfsi[k][i]) {
           case 0: for (j=0;j<3;j++)
                     putbits(pBitstream,scalar[k][j][i],6);
                   break;
           case 1:
           case 3: putbits(pBitstream,scalar[k][0][i],6);
                   putbits(pBitstream,scalar[k][2][i],6);
                   break;
           case 2: putbits(pBitstream,scalar[k][0][i],6);
        }
}
 
/************************************************************************
*
* II_bits_for_nonoise (Layer II)
*
* PURPOSE:Returns the number of bits required to produce a
* mask-to-noise ratio better or equal to the noise/no_noise threshold.
*
* SEMANTICS:
* bbal = # bits needed for encoding bit allocation
* bsel = # bits needed for encoding scalefactor select pHeaderrmation
* banc = # bits needed for ancillary data (header pHeader included)
*
* For each subband and channel, will add bits until one of the
* following occurs:
* - Hit maximum number of bits we can allocate for that subband
* - MNR is better than or equal to the minimum masking level
*   (NOISY_MIN_MNR)
* Then the bits required for scalefactors, scfsi, bit allocation,
* and the subband samples are tallied (#req_bits#) and returned.
*
* (NOISY_MIN_MNR) is the smallest MNR a subband can have before it is
* counted as 'noisy' by the logic which chooses the number of JS
* subbands.
*
* Joint stereo is supported.
*
************************************************************************/


// SNR TABLE, as defined in ISO spec
// TABLE 3-C.5: LAYER II SIGNAL-TO-NOISE RATIOS
static double snr[18] = 
{
	 0.00,		//  0
	 7.00,		//  1
	11.00,		//  2
	16.00,		//  3
	20.84,		//  4
	25.28,		//  5
	31.59,		//  6
	37.75,		//  7
	43.84,		//  8
    49.89,		//  9
	55.93,		// 10
	61.96,		// 11
	67.98,		// 12
	74.01,		// 13
    80.03,		// 14
	86.05,		// 15
	92.01,		// 16
	98.01		// 17
};

static int II_bits_for_nonoise
//static int VBR_bits_for_nonoise
	(
		DOUBLE	perm_smr[2][SBLIMIT],
		UINT	scfsi[2][SBLIMIT],
		FRAME*	pFrame,
		INT		vbrlevel
	)
{
	int			sb,ch,ba;
	int			stereo  = pFrame->nChannels;
	int			sblimit = pFrame->sblimit;
	int			jsbound = pFrame->jsbound;
	ALLOC_16*	pAlloc	= pFrame->alloc[0];
	int			req_bits = 32;					// 32 bits for ancillary data (MPEG Header)
	int			maxAlloc;
	int			sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */

	// Reserve 16 bits for CRC, if required
	if (pFrame->pHeader->bCRC) 
	{
		req_bits+=16;
	}

	// Count number of bits needed for bit allocation, upto jsbound
	for (sb=0; sb<jsbound; ++sb)
	{
		req_bits += stereo * pAlloc[sb][0].bits;
	}

	// Count number of bits needed for bit allocation, from jsbound to SBLIMIT
	// Above jsbound, one single channel
	for (sb=jsbound; sb<sblimit; ++sb)
	{
		req_bits += pAlloc[sb][0].bits;
	}


	for(sb=0; sb<sblimit; ++sb)
	{
		for(ch=0; ch<((sb<jsbound)?stereo:1); ++ch)
		{
			maxAlloc = (1<<pAlloc[sb][0].bits)-1;

			// Check how many bits we need to encode this side band
			// so that the Mask to Noise Ratio (MNR) is above > vbrlevel
			for(ba=0; ba<maxAlloc-1; ba++)
			{
				// Check if we have found enough bits already
				if( (-perm_smr[ch][sb] + snr[pAlloc[sb][ba].quant+((ba>0)?1:0)])
					>= NOISY_MIN_MNR+vbrlevel)
						break;
			}


			// check other JS channel
			if(stereo == 2 && sb >= jsbound)
			{
				for(;ba<maxAlloc-1; ++ba)
				{
					if( (-perm_smr[1-ch][sb]+ snr[pAlloc[sb][ba].quant+((ba>0)?1:0)])
						>= NOISY_MIN_MNR+vbrlevel)
						break;
				}
			}

			// If this sideband has bits allocated to it, add the required bits to allocate this sideband
			if(ba>0)
			{
				// add the number of allocation bits
				req_bits+= SCALE_BLOCK * (pAlloc[sb][ba].group * pAlloc[sb][ba].bits);

				// scale factor bits required for subband
				req_bits+= 2 + 6 * sfsPerScfsi[scfsi[ch][sb]];

				// for joint stereo, each new js sb has L+R scfsis if sb>=jsbound
				if(stereo == 2 && sb >= jsbound)
				{
					req_bits+= 2 + 6 * sfsPerScfsi[scfsi[1-ch][sb]];
				}
			}
		}
	}
  return req_bits;
}



static void VBR_maxmnr
	(
		DOUBLE		mnr[2][SBLIMIT],
		CHAR		used[2][SBLIMIT],
		INT			sblimit,
		INT			stereo,
		INT*		min_sb,
		INT*		min_ch,
		PSETTINGS	pSettings
	)
{
#ifdef HACKIT
    /* short circuit the min_value check and just keep assigning bits */
    int i,k;
    double small;

    small = 999999.0;
    *min_sb = -1;
    *min_ch = -1;
    for(k=0;k<stereo;k++)
      for (i=0;i<sblimit;i++)
        if (used[k][i]!=2 && mnr[k][i]<glopts->vbrlevel)
          {
            //small = mnr[k][i];
            *min_sb = i;
            *min_ch = k;
	    break;
          }

#else
    int i,k;
    double dSmall;

    dSmall = 999999.0;
    *min_sb = -1;
    *min_ch = -1;
    for(k=0;k<stereo;++k)
      for (i=0;i<sblimit;i++)
        if (used[k][i]  != 2 && dSmall > mnr[k][i])
          {
            dSmall = mnr[k][i];
            *min_sb = i;
            *min_ch = k;
          }
#endif
}


int VBR_bit_allocation
	(
		DOUBLE			perm_smr[2][SBLIMIT],
		UINT			scfsi[2][SBLIMIT],
		UINT			bit_alloc[2][SBLIMIT],
		INT*			adb,
		FRAME*	pFrame,
		PSETTINGS		pSettings
	)
{
  int i, min_ch, min_sb, oth_ch, k, increment, scale, seli, ba;
  int bspl, bscf, bsel, ad, bbal=0;
  double mnr[2][SBLIMIT];
  char used[2][SBLIMIT];
  int stereo  = pFrame->nChannels;
  int sblimit = pFrame->sblimit;
  int jsbound = pFrame->jsbound;
  al_table *alloc = pFrame->alloc;
  static char init= 0;
  static int banc=32, berr=0;
  static int sfsPerScfsi[] = { 3,2,1,2 };    /* lookup # sfs per scfsi */

  if (!init)
    {
      init = 1;
      if (pFrame->pHeader->bCRC) berr=16; /* added 92-08-11 shn */
    }

  for (i=0; i<jsbound; ++i)
    bbal += stereo * (*alloc)[i][0].bits;
  for (i=jsbound; i<sblimit; ++i)
    bbal += (*alloc)[i][0].bits;
  *adb -= bbal + berr + banc;
  ad = *adb;

  for (i=0;i<sblimit;i++) for (k=0;k<stereo;k++)
      {
        mnr[k][i]=snr[0]-perm_smr[k][i];
        bit_alloc[k][i] = 0;
        used[k][i] = 0;
      }
  bspl = bscf = bsel = 0;

  do
    {
      /* locate the subband with minimum SMR */
      VBR_maxmnr(mnr, used, sblimit, stereo, &min_sb, &min_ch, pSettings);

      if(min_sb > -1)
        {   /* there was something to find */
          /* find increase in bit allocation in subband [min] */
          increment = SCALE_BLOCK * ((*alloc)[min_sb][bit_alloc[min_ch][min_sb]+1].group *
                                     (*alloc)[min_sb][bit_alloc[min_ch][min_sb]+1].bits);
          if (used[min_ch][min_sb])
            increment -= SCALE_BLOCK * ((*alloc)[min_sb][bit_alloc[min_ch][min_sb]].group*
                                        (*alloc)[min_sb][bit_alloc[min_ch][min_sb]].bits);

          /* scale factor bits required for subband [min] */
          oth_ch = 1 - min_ch;    /* above js bound, need both chans */
          if (used[min_ch][min_sb])
            scale = seli = 0;
          else
            {          /* this channel had no bits or scfs before */
              seli = 2;
              scale = 6 * sfsPerScfsi[scfsi[min_ch][min_sb]];
              if(stereo == 2 && min_sb >= jsbound)
                {
                  /* each new js sb has L+R scfsis */
                  seli += 2;
                  scale += 6 * sfsPerScfsi[scfsi[oth_ch][min_sb]];
                }
            }

          /* check to see enough bits were available for */
          /* increasing resolution in the minimum band */
          if (ad >= bspl + bscf + bsel + seli + scale + increment)
            {
              ba = ++bit_alloc[min_ch][min_sb]; /* next up alloc */
              bspl += increment;                /* bits for subband sample */
              bscf += scale;                    /* bits for scale factor */
              bsel += seli;                     /* bits for scfsi code */
              used[min_ch][min_sb] = 1;         /* subband has bits */
              mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
                                    snr[(*alloc)[min_sb][ba].quant+1];
              /* Check if subband has been fully allocated max bits */
              if (ba >= (1<<(*alloc)[min_sb][0].bits)-1)
                used[min_ch][min_sb] = 2;       /* don't let this sb get any more bits */
            }
          else
            used[min_ch][min_sb] = 2; /* can't increase this alloc */

          if(min_sb >= jsbound && stereo == 2)
            {
              /* above jsbound, alloc applies L+R */
              ba = bit_alloc[oth_ch][min_sb] = bit_alloc[min_ch][min_sb];
              used[oth_ch][min_sb] = used[min_ch][min_sb];
              mnr[oth_ch][min_sb] = -perm_smr[oth_ch][min_sb] +
                                    snr[(*alloc)[min_sb][ba].quant+1];
            }

        }
    }
  while(min_sb > -1);   /* until could find no channel */

  /* Calculate the number of bits left */
  ad -= bspl+bscf+bsel;
  *adb = ad;
  for (k=0;k<stereo;k++)
    for (i=sblimit;i<SBLIMIT;i++)
      bit_alloc[k][i]=0;

  return 0;
}


/************************************************************************
*
* II_main_bit_allocation  (Layer II)
*
* PURPOSE:For joint stereo mode, determines which of the 4 joint
* stereo modes is needed.  Then calls *_a_bit_allocation(), which
* allocates bits for each of the subbands until there are no more bits
* left, or the MNR is at the noise/no_noise threshold.
*
* SEMANTICS:
*
* For joint stereo mode, joint stereo is changed to stereo if
* there are enough bits to encode stereo at or better than the
* no-noise threshold (NOISY_MIN_MNR).  Otherwise, the system
* iteratively allocates less bits by using joint stereo until one
* of the following occurs:
* - there are no more noisy subbands (MNR >= NOISY_MIN_MNR)
* - mode_ext has been reduced to 0, which means that all but the
*   lowest 4 subbands have been converted from stereo to joint
*   stereo, and no more subbands may be converted
*
*     This function calls *_bits_for_nonoise() and *_a_bit_allocation().
*
************************************************************************/

// Not all bitrates are allowed per mode (see Alloc tables 3-B.2_
// In addition, removed 48 kbits per channel since it was often selected
// due to the imperfect Psychoacoustic model and due to the low number of subbands
int AllowedBrIdx[2][10]=
{
//	 0  1  2  3  4  5  6   7   8   9  10  11  12  13  14
//	{0,32,48,56,64,80,96,112,128,160,192,224,256,320,384},		// Layer 2

// 	  0  1  2  3  4  5  6  7  8  9 
	{ 1, 3, 4, 5, 6, 7, 8, 9,10,-1 },	// mono
	{ 4, 7, 8, 9,10,11,12,13,14,-1 }	// stereo
};

void II_main_bit_allocation
	(
		DOUBLE			perm_smr[2][SBLIMIT],
		UINT			scfsi[2][SBLIMIT],
		UINT			bit_alloc[2][SBLIMIT],
		INT*			adb,
		FRAME*			pFrame,
		PSETTINGS		pSettings
	)
{
	int  noisy_sbs;
	int  rq_db, av_db = *adb;

	// If mode equals to joint stereo, find the optimum jsbound setting
	if( pFrame->actual_mode == MPG_MD_JOINT_STEREO)
	{
		// Start with standard stereo setup
		pFrame->pHeader->mode = MPG_MD_STEREO;
		pFrame->pHeader->mode_ext = 0;
		pFrame->jsbound = pFrame->sblimit;

		// Calculate the required bits
		if((rq_db=II_bits_for_nonoise(perm_smr, scfsi, pFrame,0)) > *adb)
		{
			int  mode_ext=4;

			// Were a few bits short, switch to joint stereo
			pFrame->pHeader->mode = MPG_MD_JOINT_STEREO;

			// Start with mode extention 3 (is decremented first in while loop)
			do
			{
				// Decrease mode extention
				mode_ext--;

				// Get new joint stereo subband boundary
				pFrame->jsbound = js_bound(pFrame->pHeader->lay, mode_ext);

				// Calculate the number of required bits with the new settings
				rq_db = II_bits_for_nonoise(perm_smr, scfsi, pFrame,0);

				// Check if we have enough bits, and if we still can lower the
				// joint stereo subband boundary
			} while( (rq_db > *adb) && (mode_ext > 0));

			pFrame->pHeader->mode_ext = mode_ext;

		}    /* well we either eliminated noisy sbs or mode_ext == 0 */
	}


	if (pSettings->bUseVbr == FALSE)
	{
	   noisy_sbs = II_a_bit_allocation(perm_smr, scfsi, bit_alloc, adb, pFrame);
	}
	else
	{
		int nCurrentAllocTab=0;
		int nReqBits=0;
		int	nChanIdx = pSettings->pFrame->nChannels-1;

		int i;


		// Start with minimum bitrate
		for (i=0;i<15;i++)
		{
			if ( pSettings->nMinBitRateIndex<=AllowedBrIdx[nChanIdx][i])
				break;
		}

		pFrame->pHeader->bitrate_index=AllowedBrIdx[nChanIdx][i++];

		// Recalculate the sbLimit and select proper ALLOC table
		hdr_to_frps(pFrame);

		// Determine the available bits for this frame, with current bitrate settings
		*adb = available_bits(pSettings);

		// Work out how many bits are needed for there to be no noise (ie all MNR > 0.0 + VBRLEVEL)
		// Keep bumping up the bitrate until we have more than this number of bits

		nCurrentAllocTab=pFrame->tab_num;

		if (pSettings->nFrame==192)
		{
			printf("break\n");
		}

		nReqBits=II_bits_for_nonoise(perm_smr, scfsi, pFrame,pSettings->nVbrQuality+(nCurrentAllocTab==2)?20:0);
//		nReqBits=II_bits_for_nonoise(perm_smr, scfsi, pFrame,pSettings->nVbrQuality);

		if ( pSettings->nDebugLevel > 3 )
		{
				DebugPrintf("VBR: req_bits=%i for bitrate setting %d adb=%d table=%d\n",
					nReqBits,
					bitrate[pFrame->pHeader->version][pFrame->pHeader->lay-1][pFrame->pHeader->bitrate_index],
					*adb,
					nCurrentAllocTab
				);
		}

		while ( (nReqBits > *adb) && (AllowedBrIdx[nChanIdx][i]!=-1) )
		{
			// We are not there yet, increase bit rate if possible
			pFrame->pHeader->bitrate_index=AllowedBrIdx[nChanIdx][i++];

			// Limit to maximum bitrate index
			//if (pFrame->pHeader->bitrate_index > 14) 
			//	pFrame->pHeader->bitrate_index = 14;

			// Get new frame header parameters for this bitrate setting
			hdr_to_frps(pFrame);

			// Did we switch from alloc table? If so, calculate new required bits
			if (nCurrentAllocTab!=pFrame->tab_num)
			{
				nReqBits=II_bits_for_nonoise(perm_smr, scfsi, pFrame,pSettings->nVbrQuality);			
				nCurrentAllocTab=pFrame->tab_num;
			}

			// Get the number of available bits for the new frame settings
			*adb = available_bits(pSettings);

			if ( pSettings->nDebugLevel > 2 )
			{
				DebugPrintf("VBR: req_bits=%i for bitrate setting %d adb=%d table=%d\n",
						nReqBits,
						bitrate[pFrame->pHeader->version][pFrame->pHeader->lay-1][pFrame->pHeader->bitrate_index],
						*adb,
						nCurrentAllocTab
				);
			}

		}

	   noisy_sbs = VBR_bit_allocation(perm_smr, scfsi, bit_alloc, adb, pFrame,pSettings);

	}

	if ( pSettings->nDebugLevel > 2 )
	{
		DebugPrintf("VBR	frame=%d bitrate=%4d, noisy_sbs=%4d , adb is %3d sblimit=%2d sbound=%2d\n", 
			pSettings->nFrame,
			bitrate[pFrame->pHeader->version][pFrame->pHeader->lay-1][pFrame->pHeader->bitrate_index],
			noisy_sbs,
			*adb,
			pFrame->sblimit,
			pFrame->jsbound
		);
	}

}
 
/************************************************************************
*
* II_a_bit_allocation (Layer II)
*
* PURPOSE:Adds bits to the subbands with the lowest mask-to-noise
* ratios, until the maximum number of bits for the subband has
* been allocated.
*
* SEMANTICS:
* 1. Find the subband and channel with the smallest MNR (#min_sb#,
*    and #min_ch#)
* 2. Calculate the increase in bits needed if we increase the bit
*    allocation to the next higher level
* 3. If there are enough bits available for increasing the resolution
*    in #min_sb#, #min_ch#, and the subband has not yet reached its
*    maximum allocation, update the bit allocation, MNR, and bits
    available accordingly
* 4. Repeat until there are no more bits left, or no more available
*    subbands. (A subband is still available until the maximum
*    number of bits for the subband has been allocated, or there
*    aren't enough bits to go to the next higher resolution in the
    subband.)
*
************************************************************************/

int II_a_bit_allocation_old
	(
		double	perm_smr[2][SBLIMIT],
		UINT	scfsi[2][SBLIMIT],
		UINT	bit_alloc[2][SBLIMIT],
		int*	adb,
		FRAME*	pFrame
	)
{
   int i, min_ch, min_sb, oth_ch, k, increment, scale, seli, ba;
   int bspl, bscf, bsel, ad, noisy_sbs, bbal=0;

	double		mnr[2][SBLIMIT];
	double		dSmall;
	char		used[2][SBLIMIT];

	int			stereo  = pFrame->nChannels;
	int			sblimit = pFrame->sblimit;
	int			jsbound = pFrame->jsbound;
	ALLOC_16*	pAlloc=	pFrame->alloc[0];
	int			sfsPerScfsi[] = { 3,2,1,2 };    // lookup # sfs per scfsI
	INT			berr=0;							// Number of CRC bits
 
	if (pFrame->pHeader->bCRC)
		berr=16;

	for (i=0; i<jsbound; ++i)
	{
		bbal += stereo * pAlloc[i][0].bits;
	}

	for (i=jsbound; i<sblimit; ++i)
	{
		bbal += pAlloc[i][0].bits;
	}

	*adb -= bbal + berr + 32;
	ad = *adb;

	
	for (k=0;k<stereo;k++)
	{
		for (i=0;i<sblimit;i++)
		{
			mnr[k][i]=snr[0]-perm_smr[k][i];
			bit_alloc[k][i] = 0;
			used[k][i] = 0;
		}
	}

	bspl = bscf = bsel = 0;
 
	do
	{
		dSmall = 999999.0;
		min_sb = -1;
		min_ch = -1;


		// locate the subband with minimum SMR (AF, swapped for loops)
		for(k=0;k<stereo;++k)
		{
			for (i=0;i<sblimit;i++)
			{
				if (used[k][i] != 2 && dSmall > mnr[k][i])
				{
					dSmall = mnr[k][i];
					min_sb = i;
					min_ch = k;
				}
			}
		}

		// there was something to find
		if(min_sb > -1)
		{   
			// find increase in bit allocation in subband [min]
			increment = SCALE_BLOCK * 
						pAlloc[min_sb][bit_alloc[min_ch][min_sb]+1].group *
                        pAlloc[min_sb][bit_alloc[min_ch][min_sb]+1].bits;
			
			if (used[min_ch][min_sb])
			{
				increment -= SCALE_BLOCK *
						pAlloc[min_sb][bit_alloc[min_ch][min_sb]].group*
						pAlloc[min_sb][bit_alloc[min_ch][min_sb]].bits;
			}

			// scale factor bits required for subband [min]
			// above js bound, need both chans
			oth_ch = 1 - min_ch;    

			if (used[min_ch][min_sb])
			{
				scale = seli = 0;
			}
			else 
			{
				// this channel had no bits or scfs before
				seli = 2;
				scale = 6 * sfsPerScfsi[scfsi[min_ch][min_sb]];
				if(stereo == 2 && min_sb >= jsbound)
				{
					// each new js sb has L+R scfsis
					seli += 2;
					scale += 6 * sfsPerScfsi[scfsi[oth_ch][min_sb]];
				}
			}

			// check to see enough bits were available for
			// increasing resolution in the minimum band
			if (ad >= bspl + bscf + bsel + seli + scale + increment)
			{
				ba = ++bit_alloc[min_ch][min_sb]; /* next up alloc */
				bspl += increment;  /* bits for subband sample */
				bscf += scale;      /* bits for scale factor */
				bsel += seli;       /* bits for scfsi code */
				used[min_ch][min_sb] = 1; /* subband has bits */
				mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
                               snr[pAlloc[min_sb][ba].quant+1];

				/* Check if subband has been fully allocated max bits */
				if (ba >= (1<<pAlloc[min_sb][0].bits)-1)
					used[min_ch][min_sb] = 2;
			}
			else
			{
				used[min_ch][min_sb] = 2; /* can't increase this alloc */
			}

			if(min_sb >= jsbound && stereo == 2)
			{
				/* above jsbound, alloc applies L+R */
				ba = bit_alloc[oth_ch][min_sb] = bit_alloc[min_ch][min_sb];
				used[oth_ch][min_sb] = used[min_ch][min_sb];
				mnr[oth_ch][min_sb] = -perm_smr[oth_ch][min_sb] + snr[pAlloc[min_sb][ba].quant+1];
			}
		}
	} while(min_sb > -1);   /* until could find no channel */

	/* Calculate the number of bits left */
	ad -= bspl+bscf+bsel;
	*adb = ad;

	for (k=0;k<stereo;k++)
	{
		for (i=sblimit;i<SBLIMIT;i++)
		{
			bit_alloc[k][i]=0;
		}
	}

	noisy_sbs = 0;
	dSmall = mnr[0][0];

	// calc worst noise in case
	for(k=0;k<stereo;++k)
	{
		for (i=0;i<sblimit;i++)
		{
//			if (dSmall > mnr[k][i])
//				dSmall = mnr[k][i];

			if(mnr[k][i] < NOISY_MIN_MNR)
			{
				// noise is not masked
				noisy_sbs++;
			}

		}
	}

   return noisy_sbs;
}


int II_a_bit_allocation
	(
		double	perm_smr[2][SBLIMIT],
		UINT	scfsi[2][SBLIMIT],
		UINT	bit_alloc[2][SBLIMIT],
		int*	adb,
		FRAME*	pFrame
	)
{
	int i, min_ch, min_sb, k;
//, increment,ba;
//	int ad,
	int noisy_sbs;

	double		mnr[2][SBLIMIT];
//	char		used[2][SBLIMIT];

	int			nChannels  = pFrame->nChannels;
	int			sblimit = pFrame->sblimit;
	int			jsbound = pFrame->jsbound;
	ALLOC_16*	pAlloc=	pFrame->alloc[0];
	int			sfsPerScfsi[] = { 3,2,1,2 };    // lookup # sfs per scfsI
	INT			berr=0;							// Number of CRC bits
 
	// subtract 32 ancillary bits (the MPEG header)
	*adb-=32;

	// subatract CRC 16 bits ?
	if (pFrame->pHeader->bCRC)
		*adb-=16;

	// subtract allocation bits till jsbound
	for (i=0; i<jsbound; ++i)
		*adb -= nChannels * pAlloc[i][0].bits;

	// subtract allocation from jsbound and above
	for (i=jsbound; i<sblimit; ++i)
		*adb -= pAlloc[i][0].bits;


	// loop through all channels
	for (k=0;k<nChannels;k++)
	{
		// loop through all subbands
		for (i=0;i<sblimit;i++)
		{
			// calculate the mask to noise ratio
			mnr[k][i]=snr[0]-perm_smr[k][i];

			// for joint stereo only look for the minimum mask to noise ratio
			if ( (i>=jsbound) && (k==1) )
			{
				// pick the minimum Mask to Noise Ratio of the two channels
				if (mnr[0][i]>mnr[1][i])
				{
					// channel 1 has the minimum MNR
					mnr[0][i]=DBL_MAX;
				}
				else
				{
					// channel 0 has the minimum MNR
					mnr[1][i]=DBL_MAX;
				}
			}

			// clear bit_alloc for all subbands and all channels
			bit_alloc[k][i] = 0;
		}
	}


	do
	{
	   int nScaleInfoBits=0;

		DOUBLE	dSmall = DBL_MAX;
		min_sb = -1;
		min_ch = -1;


		// locate the subband with minimum Mask to Noise Ratio
		// in all channels
		for(k=0;k<nChannels;++k)
		{
			// in all subbands
			for (i=0;i<sblimit;i++)
			{
				// check if this is the minimum
				if (dSmall > mnr[k][i])
				{
					// yes it is, retain the value, sidband
					// and channel where the minimum MNR is located
					dSmall = mnr[k][i];
					min_sb = i;
					min_ch = k;
				}
			}
		}

		// did we find a proper MNR value
		if(min_sb > -1)
		{   
			INT ba=bit_alloc[min_ch][min_sb];
			INT nNewBits=0;

			// determine the amount of bits for this allocation resolution setting
			nNewBits = SCALE_BLOCK * pAlloc[min_sb][ba+1].group * pAlloc[min_sb][ba+1].bits;
			
			if (ba)
			{
				// calculate the additional bits needed for the new allocation resolution
				nNewBits-= SCALE_BLOCK * pAlloc[min_sb][ba].group * pAlloc[min_sb][ba].bits;
			}
			else
			{
				// this channel had no allocation bits before
				// therefore subtract the bits required to encode the scsi bands
				nNewBits+=2+ 6 * sfsPerScfsi[scfsi[min_ch][min_sb]];

				// Joint stereo, and we're reached over the js boundary
				if(nChannels == 2 && min_sb >= jsbound)
				{
					// also reserve the scsi bands bits to encode the 
					// other joint stereo side bands
					nNewBits+= 2+ 6 * sfsPerScfsi[scfsi[1-min_ch][min_sb]];
				}
			}

			// check to see enough bits were available for
			// increasing resolution in the minimum band
			if (*adb >= nNewBits)
			{
				ba = ++bit_alloc[min_ch][min_sb]; /* next up alloc */

				if(min_sb >= jsbound && nChannels == 2)
					bit_alloc[1-min_ch][min_sb] = bit_alloc[min_ch][min_sb];

				*adb -= nNewBits;			/* subtract new bits  */

				// Check if subband has been fully allocated to max bits
				if (ba >= (1<<pAlloc[min_sb][0].bits)-1)
				{
					// if so, don't assign more bits to this sideband
					mnr[min_ch][min_sb]=DBL_MAX;
				}
				else
				{
					// calculate new Mask to Noise Ratio
					mnr[min_ch][min_sb] = -perm_smr[min_ch][min_sb] +
                               snr[pAlloc[min_sb][ba].quant+1];
				}
			}
			else
			{
				// There are not enough bits to encode this sideband, thus
				// remove from bit allocation pool
				mnr[min_ch][min_sb]=DBL_MAX;
			}
		}
	} while(min_sb > -1);   /* until could find no channel */


	// set all bit_alloc values from sblimit and above to zero
	for (k=0;k<nChannels;k++)
	{
		for (i=sblimit;i<SBLIMIT;i++)
		{
			bit_alloc[k][i]=0;
		}
	}

	noisy_sbs = 0;

	// calculate the number of noisy side bands
	for(k=0;k<nChannels;++k)
	{
		for (i=0;i<sblimit;i++)
		{
			if(mnr[k][i] < NOISY_MIN_MNR)
			{
				// noise is not fully masked
				noisy_sbs++;
			}

		}
	}

   return noisy_sbs;
}

 
/************************************************************************
*
* II_subband_quantization (Layer II)
*
* PURPOSE:Quantizes subband samples to appropriate number of bits
*
* SEMANTICS:  Subband samples are divided by their scalefactors, which
 makes the quantization more efficient. The scaled samples are
* quantized by the function a*x+b, where a and b are functions of
* the number of quantization levels. The result is then truncated
* to the appropriate number of bits and the MSB is inverted.
*
* Note that for fractional 2's complement, inverting the MSB for a
 negative number x is equivalent to adding 1 to it.
*
************************************************************************/
 
static double a[17] = {
  0.750000000, 0.625000000, 0.875000000, 0.562500000, 0.937500000,
  0.968750000, 0.984375000, 0.992187500, 0.996093750, 0.998046875,
  0.999023438, 0.999511719, 0.999755859, 0.999877930, 0.999938965,
  0.999969482, 0.999984741 };
 
static double b[17] = {
  -0.250000000, -0.375000000, -0.125000000, -0.437500000, -0.062500000,
  -0.031250000, -0.015625000, -0.007812500, -0.003906250, -0.001953125,
  -0.000976563, -0.000488281, -0.000244141, -0.000122070, -0.000061035,
  -0.000030518, -0.000015259 };
 
void II_subband_quantization(	UINT scalar[2][3][SBLIMIT],
								double sb_samples[2][3][SCALE_BLOCK][SBLIMIT],
								UINT j_scale[3][SBLIMIT],
								double j_samps[3][SCALE_BLOCK][SBLIMIT],
								UINT bit_alloc[2][SBLIMIT],
								UINT sbband[2][3][SCALE_BLOCK][SBLIMIT],
								FRAME *pFrame)
{
	int		i, j, k, s, n, qnt, sig;
	int		stereo  = pFrame->nChannels;
	int		sblimit = pFrame->sblimit;
	int		jsbound = pFrame->jsbound;
	UINT		stps;
	double	d;
	int		q;

	al_table *alloc = pFrame->alloc;
#if 0
	for (s=0;s<3;s++)
     for (j=0;j<SCALE_BLOCK;j++)
       for (i=0;i<sblimit;i++)
         for (k=0;k<((i<jsbound)?stereo:1);k++)
           if (bit_alloc[k][i])
		   {
             /* scale and quantize floating point sample */
             if(stereo == 2 && i>=jsbound)       /* use j-stereo samples */
               d = j_samps[s][j][i] / multiple[j_scale[s][i]];
             else
               d = sb_samples[k][s][j][i] / multiple[scalar[k][s][i]];

             if (fabs(d) > 1.0)
			 {
               printf("Not scaled properly %d %d %d %d\n",k,s,j,i);
			 }

             qnt = (*alloc)[i][bit_alloc[k][i]].quant;

             d = d * a[qnt] + b[qnt];
             
				// extract MSB N-1 bits from the floating point sample
				if (d >= 0)
					sig = 1;
				else { sig = 0; d += 1.0; }
					n = 0;

             stps = (*alloc)[i][bit_alloc[k][i]].steps;
             while ((1<<n) < (int)stps) n++;
             n--;

             sbband[k][s][j][i] = (UINT) (d * (double) (1L<<n));
             /* tag the inverted sign bit to sbband at position N */
             /* The bit inversion is a must for grouping with 3,5,9 steps
                so it is done for all subbands */
             if (sig) sbband[k][s][j][i] |= 1<<n;
           }
#else


	for (q=0;q< ((sblimit>jsbound)?2:1);q++)
	{
		int nChannels=stereo;
		int nSbLimit=jsbound;
		
		// For Joint stereo, q=0..1 otherwhise q=0
		if (q==1)
		{
			// For joint stereo, above jsbound, thus one side channel 
			nSbLimit=sblimit;
			nChannels=1;
		}

		for (k=0;k<nChannels;k++)
		{
			for (s=0;s<3;s++)
			{
				for (j=0;j<SCALE_BLOCK;j++)
				{
					for (i=0;i<nSbLimit;i++)
					{	
						if (bit_alloc[k][i])
						{
							// scale and quantize floating point sample
							// use j-stereo samples
							if(q)
								d = j_samps[s][j][i] / multiple[j_scale[s][i]];
							else
								d = sb_samples[k][s][j][i] / multiple[scalar[k][s][i]];

							if (fabs(d) > 1.0)
							{
								printf("Not scaled properly %d %d %d %d\n",k,s,j,i);
							}


							qnt = (*alloc)[i][bit_alloc[k][i]].quant;

							d = d * a[qnt] + b[qnt];
             
							// extract MSB N-1 bits from the floating point sample
							if (d >= 0)
								sig = 1;
							else
							{
								sig = 0;
								d += 1.0;
							}

							n = 0;

							stps = (*alloc)[i][bit_alloc[k][i]].steps;

							while ((1<<n) < (int)stps)
								n++;
							n--;

							sbband[k][s][j][i] = (UINT) (d * (double) (1L<<n));

							// tag the inverted sign bit to sbband at position N 
							// The bit inversion is a must for grouping with 3,5,9 steps
							// so it is done for all subbands
							if (sig)
								sbband[k][s][j][i] |= 1<<n;
						}
					}
				}
			}
		}
	}

#endif

	for (k=0;k<stereo;k++)
	{
		for (s=0;s<3;s++)
		{
			for (i=0;i<SCALE_BLOCK;i++)
			{
				for (j=sblimit;j<SBLIMIT;j++)
				{
					sbband[k][s][i][j] = 0;
				}
			}
		}
	}
}
 
/*************************************************************************
* II_encode_bit_alloc (Layer II)
*
* PURPOSE:Writes bit allocation pHeaderrmation onto bitstream
*
* Layer I uses 4 bits/subband for bit allocation pHeaderrmation,
* and Layer II uses 4,3,2, or 0 bits depending on the
* quantization table used.
*
************************************************************************/
 
void II_encode_bit_alloc(
							UINT bit_alloc[2][SBLIMIT],
							FRAME *pFrame,
							BITSTREAM *pBitstream
						)
{
	int i,k;
	int nChannels		= pFrame->nChannels;
	int sblimit			= pFrame->sblimit;
	int jsbound			= pFrame->jsbound;
	ALLOC_16*	pAlloc	= pFrame->alloc[0];
 
	for (i=0;i<sblimit;i++)
	{
		for (k=0;k<((i<jsbound)?nChannels:1);k++)
		{
			putbits(pBitstream,bit_alloc[k][i],pAlloc[i][0].bits);
		}
	}
}
 
/************************************************************************
*
* II_sample_encoding  (Layer II)
*
* PURPOSE:Put one frame of subband samples on to the bitstream
*
* SEMANTICS:  The number of bits allocated per sample is read from
* the bit allocation pHeaderrmation #bit_alloc#.  Layer 2
* supports writing grouped samples for quantization steps
* that are not a power of 2.
*
************************************************************************/

void II_sample_encoding(	UINT		sbband[2][3][SCALE_BLOCK][SBLIMIT],
							UINT		bit_alloc[2][SBLIMIT],
							PFRAME		pFrame,
							PBITSTREAM	pBitstream
						)
{
	UINT j,s,x,y;
	int					i,k;
	int nChannels		= pFrame->nChannels;
	int sblimit			= pFrame->sblimit;
	int jsbound			= pFrame->jsbound;
	ALLOC_16*	pAlloc	= pFrame->alloc[0];
	 
	for (s=0;s<3;s++)
	{
		for (j=0;j<SCALE_BLOCK;j+=3)
		{
			for (i=0;i<sblimit;i++)
			{
				for (k=0;k<((i<jsbound)?nChannels:1);k++)
				{
					if (bit_alloc[k][i])
					{
						if (pAlloc[i][bit_alloc[k][i]].group == 3)
						{
							for (x=0;x<3;x++) 
								putbits(pBitstream,sbband[k][s][j+x][i],pAlloc[i][bit_alloc[k][i]].bits);
						}
						else
						{
							UINT temp;
							y =pAlloc[i][bit_alloc[k][i]].steps;
							temp =	sbband[k][s][j][i] +
									sbband[k][s][j+1][i] * y +
									sbband[k][s][j+2][i] * y * y;
							putbits(pBitstream,temp,pAlloc[i][bit_alloc[k][i]].bits);
						}
					}
				}
			}
		}
	}
}



void II_combine_LR(double	sb_sample[2][3][SCALE_BLOCK][SBLIMIT],
				   double	joint_sample[3][SCALE_BLOCK][SBLIMIT],
				   int		sblimit)
{  
	// make a filtered mono for joint stereo
	int sb, smp, sufr;

	for(sb = 0; sb<sblimit; ++sb)
		for(smp = 0; smp<SCALE_BLOCK; ++smp)
			for(sufr = 0; sufr<3; ++sufr)
				joint_sample[sufr][smp][sb] = .5 * (sb_sample[0][sufr][smp][sb]
										   + sb_sample[1][sufr][smp][sb]);
}


