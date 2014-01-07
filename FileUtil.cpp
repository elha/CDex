/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
**
** http://cdexos.sourceforge.net/
** http://sourceforge.net/projects/cdexos 
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


/*******************************************************************************

 FILENAME:		FileUtil.cpp
 PROJECT:		none
 PURPOSE:		Provide CFile IO Stream operators for Core Exception objects

 REMARKS:		none

 *******************************************************************************/

// INCLUDES

#include "stdafx.h"
#include "FileUtil.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  operator>> - open an CArchive to get overload >> as file read.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//inline CFile& operator>>(CFile &f, CUString &str)
//{
//	CArchive ar(&f,CArchive::load);
//	ar >> str;
//	ar.Close();
//	return f;
//}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  operator<< - open an CArchive to get overload >> as file read.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//inline CFile& operator<<(CFile &f, CUString &str)
//{
//	CArchive ar(&f,CArchive::store);
//	ar << str;
//	ar.Close();
//	return f;
//}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  operator>> - open an CArchive to get overload >> as file read.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//inline CFile& operator>>(CFile &f, CUString &str)
//{
//	return operator>>(f,(CUString&)str);
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  operator<< - open an CArchive to get overload >> as file read.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//inline CFile& operator<<(CFile &f, CUString &str)
//{
//	return operator<<(f,(CUString&)str);
//}

