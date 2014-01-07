/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
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


#ifndef ENCODEROBJECTFACTORY_H_INCLUDED
#define ENCODEROBJECTFACTORY_H_INCLUDED

// forward class declarations
class CEncoder;

#ifdef _DEBUG
//#define ENCODER_TYPES_EXPERIMENTAL
#endif


enum ENCODER_TYPES {	
		ENCODER_FIXED_WAV = -1,
		ENCODER_INT,				// 0
		ENCODER_EXT,				// 1
		ENCODER_WIN,				// 2
		ENCODER_BLD,				// 3
		ENCODER_MP2,				// 4
		ENCODER_VQF,				// 5
		ENCODER_AAC,				// 6
		ENCODER_WMA,				// 7
		ENCODER_AACPSY,				// 8
		ENCODER_EXTXING,			// 9
		ENCODER_WAV,				// 10
		ENCODER_FAAC,				// 11
		ENCODER_OGG,				// 12
		ENCODER_GGO,				// 13
		ENCODER_MPP,				// 14
		ENCODER_APE,				// 15
		ENCODER_NTTVQF,				// 16
		ENCODER_FLAC,				// 17

#ifdef ENCODER_TYPES_EXPERIMENTAL
		ENCODER_SCX,
//		ENCODER_NTTVQF,
#endif
		NUMENCODERS
};


CEncoder* EncoderObjectFactory( ENCODER_TYPES nEncoderID );



#endif
