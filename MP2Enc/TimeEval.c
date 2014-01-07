/*
** Copyright (C) 2000 Albert L. Faber
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
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

#include "TimeEval.h"
#include "machine.h"


int TimeEval(int nFlag)
{
#if defined(_WIN32) && !defined(__CYGWIN__) 
	static LARGE_INTEGER	temps1,temps2,nlFreq;
	static BOOL				bFirst=TRUE;
	static double			dMyFreq;
	double					dCurrent;
	int						time_ms;

	if (bFirst)
	{
		QueryPerformanceFrequency(&nlFreq);

		dMyFreq=(double)nlFreq.QuadPart/(double)1000000;
		bFirst=FALSE;
	}

	if (nFlag==0)
	{
		QueryPerformanceCounter(&temps1);
	}
	else
	{
		QueryPerformanceCounter(&temps2);
		dCurrent=(double)(temps2.QuadPart-temps1.QuadPart)/dMyFreq;
		time_ms=(int)(dCurrent/1000+0.5);
		return time_ms;
	}
#endif
	return 0;
}