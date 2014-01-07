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

#ifndef MPEG_ERROR_INCLUDED
#define MPEG_ERROR_INCLUDED

#include <stdio.h>

//typedef void (*ERRORFUNC)(char* lpszFile,int nLine,char* lpszError);
//extern ERRORFUNC pErrorFunc;

//#define MPegError(c) {pErrorFunc(__FILE__,__LINE__,c); }

#if defined(_WIN32) && !defined(__CYGWIN__) 
	void MP2LibError(const char* pzFormat, ...);
	void DebugPrintf(const char* pzFormat, ...);
#else
	#define DebugPrintf printf
	#define MP2LibError printf
#endif

#endif
