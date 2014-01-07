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

#ifdef _WIN32
	#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpglib_error.h"


static char szBuffer[255];



char* get_last_error()
{
//	g_bErrorSet=FALSE;
	return szBuffer;
}


void log_error(const char* pzFormat, ...)
{
    va_list ap;

	// Clear buffer
	memset(szBuffer,0x00,sizeof(szBuffer));

	va_start(ap, pzFormat);
	_vsnprintf(szBuffer, sizeof(szBuffer), pzFormat, ap);

	#ifdef _DEBUG
	#ifdef WIN32
		strcat(szBuffer, "\n\r");
		OutputDebugString(szBuffer);
	#else
		fprintf(stderr,"%s\n",szBuffer);
	#endif
	#endif
    va_end(ap);
}		


