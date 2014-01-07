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

#ifndef PSYCHO_0_H_INCLUDED
#define PSYCHO_0_H_INCLUDED

#include "common.h"

void Psycho_0
	(
		PSETTINGS	pSettings,
		FLOAT*		buffer,
		FLOAT		savebuf[1056],
		INT			chn,
		INT			lay,
		DOUBLE		snr32[SBLIMIT],
		DOUBLE		sfreq
	);

#endif

