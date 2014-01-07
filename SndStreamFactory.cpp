/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 Albert L. Faber
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

#include "SndStreamFactory.h"

#include "ISndStreamWAV.h"
//#include "ISndStreamMPEG.h"
//#include "ISndStreamOGG.h"
#include "ISndStreamWMA.h"
#include "ISndStreamWinAmp.h"


ISndStream* ICreateStream(CUString& strFileName)
{
	ISndStream* sndStream=NULL;

		// Get extention of filename
	CUString strExt=strFileName.Right(3);

	// Make it lower case for comparison
	strExt.MakeLower();

	// Is this an MP2 or MP3 file
	if ( strExt == _T( "mp3" ) || strExt == _T( "mp2" ) ) 
	{
//		sndStream=new ISndStreamMPEG;
		sndStream=new ISndStreamWinAmp;
	} else if ( strExt == _W( "wav" ) ) 
	{
		sndStream=new ISndStreamWAV;

	} else if (strExt == _W( "wma" ) ) 
	{
		sndStream=new ISndStreamWMA;
	} else
	{
		sndStream=new ISndStreamWinAmp;
	}

	return sndStream;
}



CUString GetInFileNameExt()
{
	CUString strExt = GetWinampFileNameExt();
	if ( !strExt.IsEmpty() )
		strExt+= _W( ";" );

	strExt+= _W( "OGG;WAV;MP3;MP2;MP1;WMA" );

	return strExt;
}
