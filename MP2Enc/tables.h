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

#ifndef TABLES_H_INCLUDED
#define TABLES_H_INCLUDED

#include "Types.h"

extern float table_AbsThr0[];
extern float table_AbsThr1[];
extern float table_AbsThr2[];

extern double enwindow[];

extern ALLOCTABLE table_ALLOC0[];
extern ALLOCTABLE table_ALLOC1[];
extern ALLOCTABLE table_ALLOC2[];
extern ALLOCTABLE table_ALLOC3[];
extern ALLOCTABLE table_ALLOC4[];


extern int table_2CB0[27];
extern int table_2CB1[27];
extern int table_2CB2[25];
extern int table_2CB4[24];
extern int table_2CB5[24];
extern int table_2CB6[22];


extern g_thres table_2TH1[127];
extern g_thres table_2TH2[133];
extern g_thres table_2TH0[131];
extern g_thres table_2TH4[133];
extern g_thres table_2TH5[133];
extern g_thres table_2TH6[133];

#endif