/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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


#ifndef FILEUTIL_H
#define FILEUTIL_H
/*******************************************************************************

 FILENAME:		FileUtil.h
 PROJECT:		Core
 PURPOSE:		Provide CFile IO Stream operators for Core Exception objects

 REMARKS:		none

 *******************************************************************************/

// INCLUDES


// REFERENCES


// Import/export macro


// Overload operators for CUString
//CFile& operator>>(CFile &f, CUString &str);
// CFile& operator<<(CFile &f, CUString &str);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  operator<< - overload << as file write.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//template <class T> CFile& operator<<(CFile &f, T& a)
//{
//	f.Write(&a,sizeof(a));
//	return f;
//}
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////  operator>> - overload >> as file read.
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//template <class T> CFile& operator>>(CFile &f, T& a)
//{
//	f.Read(&a,sizeof(a));
//	return f;
//}

#endif	// FILEUTIL_H