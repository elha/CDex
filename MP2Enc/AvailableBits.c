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

#include "AvailableBits.h"

/* function returns the number of available bits */
int available_bits(PSETTINGS pSettings)
{
	int		adb=0;  
	DOUBLE	dAverage=0.0;
	DOUBLE	dFrac=0.0;
	int		nWhole=0;
	PHEADER pHeader=pSettings->pFrame->pHeader;

    // Figure average number of 'slots' per frame.
    // Bitrate means TOTAL for both channels, not per side !
	dAverage = ( 
		(double)pSettings->nSamplesPerFrame / 
		(double)s_freq[pHeader->version][pHeader->sampling_frequency] *
		(double)bitrate[pHeader->version][pHeader->lay-1][pHeader->bitrate_index] / (double)pSettings->nBitsPerSlot
	);

	nWhole = (int)dAverage;
	dFrac = dAverage - (double)nWhole;

	// Default, no padding
	adb=nWhole*pSettings->nBitsPerSlot;
	pHeader->padding= 0;


	if (dFrac != 0 && pSettings->bUsePadBit)
	{
		int nBits1=(int)(dAverage*(pSettings->nFrame+1));
		int nBits2=(int)(dAverage*(pSettings->nFrame+2));

		// Do we need to pad this frame ?
		if ( (nBits2-nBits1) != nWhole )
		{
			adb+=pSettings->nBitsPerSlot;
			pHeader->padding= 1;
		}
	}

	// subtract the ancillary bits
	return adb-pSettings->nAncilBits;
}
