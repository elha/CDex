/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
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


#include "StdAfx.h"
#include "EncoderObjectFactory.h"
#include "config.h"

#include "Encode.h"
#include "EncoderFaacDll.h"
#include "EncoderVorbisDll.h"
#include "EncoderGogoDll.h"
#include "EncoderApeDll.h"
#include "EncoderNttVqfDll.h"
#include "EncoderWMA8Dll.h"
#include "EncoderLameDll.h"
#include "EncoderWinCodecMP3.h"
#include "EncoderDos.h"
#include "EncoderDosMpp.h"
#include "EncoderDosVqf.h"
#include "EncoderDosXing.h"
#include "EncoderWAV.h"
#include "EncoderWAVFixed.h"
#include "EncoderMP2Dll.h"
#include "EncoderBladeDll.h"
#include "EncoderFlacDll.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderObjectFactory" ) );

CEncoder* EncoderObjectFactory( ENCODER_TYPES nEncoderID )
{
	// instantiate requested encoder object
	switch ( nEncoderID )
	{
		case ENCODER_FIXED_WAV:	return new CEncoderWAVFixed;
		case ENCODER_INT:		return new CEncoderLameDll;
		case ENCODER_EXT:		return new CEncoderDos;
		case ENCODER_WIN:
			if ( g_AcmCodec.IsMP3CodecInstalled() )
			{
				return new CEncoderWinCodecMP3;
			}
			else
			{
				return NULL;
			}
		case ENCODER_BLD:		return new CEncoderBladeDll;
		case ENCODER_MP2:		return new CEncoderMP2Dll;
		case ENCODER_VQF:		return new CEncoderDosVqf;
		case ENCODER_AAC:		return new CAacEncoder;			
		case ENCODER_AACPSY:	return new CAacPsyEncoder;			
		case ENCODER_WMA:		return new CEncoderWMA8Dll;
		case ENCODER_EXTXING:	return new CEncoderDosXing;
		case ENCODER_WAV:		return new CEncoderWAV;
		case ENCODER_FAAC:		return new CEncoderFaacDll;
		case ENCODER_OGG:		return new CEncoderVorbisDll;
		case ENCODER_GGO:		return new CEncoderGogoDll;
		case ENCODER_MPP:		return new CEncoderDosMpp;			
		case ENCODER_APE:		return new CEncoderApeDll;
		case ENCODER_NTTVQF:	return new CEncoderNttVqfDll;
		case ENCODER_FLAC:		return new CEncoderFlacDll;

#ifdef ENCODER_TYPES_EXPERIMENTAL

		case ENCODER_SCX:
			if ( g_AcmCodec.IsMP3CodecInstalled() )
				return new CWinSCXEncoder;
			else
				return NULL;
#endif
		default:
			ASSERT( FALSE );
	}
	return NULL;
}


