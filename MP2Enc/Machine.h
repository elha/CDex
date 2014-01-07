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


#ifndef MACHINE_H_INCLUDED
#define MACHINE_H_INCLUDED

#include        <stdio.h>
#include        <string.h>
#include        <math.h>

#define HAS_WIN_TYPES 1


#if defined(_WIN32) && !defined(__CYGWIN__) 
	#include <windows.h>
#else
	#define MAX_PATH		255
	typedef void			VOID;
	typedef int*			PVOID;

	typedef int				BOOL;
	typedef int*			PBOOL;

	typedef char			CHAR;
	typedef char*			PCHAR;
	typedef char*			LPSTR;

	typedef unsigned char	BYTE;
	typedef unsigned char*	PBYTE;

	typedef short			SHORT;
	typedef short*			PSHORT;

	typedef unsigned short	WORD;
	typedef unsigned short*	PWORD;

	typedef int				INT;
	typedef int*			PINT;
	
	typedef unsigned int	UINT;
	typedef unsigned int*	PUINT;

	typedef long			LONG;
	typedef long*			PLONG;

	typedef unsigned long	ULONG;
	typedef unsigned long*	PULONG;

	typedef unsigned long	DWORD;
	typedef unsigned long*	PDWORD;

	typedef float			FLOAT;
	typedef float*			PFLOAT;

	typedef double			DOUBLE;
	typedef double*			PDOUBLE;


#endif

#endif