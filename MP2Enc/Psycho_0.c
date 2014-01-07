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

#include "machine.h"
#include "types.h"
#include "Psycho_0.h"

/* a bunch of SNR values I sort of made up  MFC 1 oct 99 */
static FLOAT snrdef[2][32] = 
{
	{ 30,  17,  16,  10,  3,   12,   8, 2.5,
       5,   5,   6,   6,  5,    6,  10,   6,
      -4, -10, -21, -30, -42, -55, -68, -75,
     -75, -75, -75, -75, -91,-107,-110,-108},

	{ 30,  17,  16,  10,  3,   12,   8, 2.5,
       5,   5,   6,   6,  5,    6,  10,   6,
      -4, -10, -21, -30, -42, -55, -68, -75,
     -75, -75, -75, -75, -91,-107,-110,-108}
};


void Psycho_0
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
	int i;

	for (i=0;i<SBLIMIT;i++)
	{
		snr32[i] = snrdef[chn][i];
	}
}

