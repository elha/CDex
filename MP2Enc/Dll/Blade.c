/*
** Copyright (C) 2000 Albert L. Faber
** 
** Floating-Point processeing was added by DSPguru on March 2002.
** Modified Source-code can be found at http://DSPguru.doom9.net
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU Leeser General Public License as published by
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

#include "../machine.h"
#include "../MP2Enc.h"
#include "Blade.h"


// Define global variables

MP2ERR	MP2EncEncodeFrame	(HMP2_STREAM hStream, DWORD nSamples, PFLOAT buffer_l,PFLOAT buffer_r, PBYTE pOutput, PDWORD pdwOutput);
MP2ERR	MP2EncClose			(HMP2_STREAM hStream);

DLLEXPORT BE_ERR	beInitStream(PBE_CONFIG pbeConfig, PDWORD dwSamples, PDWORD dwBufferSize, PHBE_STREAM phbeStream)
{
	// Initialize the encoder
	int				nCRC=pbeConfig->format.mp3.bCRC;
	int				nVBR=(nCRC>>12)&0x0F;
	MP2ENC_CONFIG	config;

	// clear everything
	memset(&config,0,sizeof(config));

	// set defaults
	config.dwVersion	= MP2ENC_CUR_IF_VERSION;
	config.dwSize		= sizeof(MP2ENC_CONFIG);

	config.dwSampleRate	= pbeConfig->format.mp3.dwSampleRate;
	config.dwMode		= pbeConfig->format.mp3.byMode;
	config.dwBitrate	= pbeConfig->format.mp3.wBitrate;
							
	config.bPrivate		= pbeConfig->format.mp3.bPrivate;		
	config.bCRC			= (pbeConfig->format.mp3.bCRC&0x01)?1:0;	
	config.bCopyright	= (pbeConfig->format.mp3.bCopyright&0x01)?1:0;
	config.bOriginal	= pbeConfig->format.mp3.bOriginal;

	config.bUseVbr		= (nVBR>0)?1:0;
	config.dwVbrQuality	= nVBR-1;

	config.dwPsyModel	= 2;
	config.dwQuickMode	= 1;

	config.bWriteAncil	= pbeConfig->format.mp3.bCRC&0x02;
	config.bNoPadding	= (pbeConfig->format.mp3.bCRC&0x04)?1:0;

	config.nDebugLevel	= 1;

#ifdef DSPguru
	config.max_bitrate=(pbeConfig->format.mp3.bCopyright>>2);
	if (!config.max_bitrate) config.max_bitrate=14;
#endif

	return MP2EncOpen(&config,dwSamples,dwBufferSize,phbeStream);
}



DLLEXPORT BE_ERR	beDeinitStream(HBE_STREAM hbeStream, PBYTE pOutput, PDWORD pdwOutput)
{
	// no output bits
	*pdwOutput=0;

	// Everything went OK, thus return SUCCESSFUL
	return BE_ERR_SUCCESSFUL;
}


DLLEXPORT BE_ERR	beCloseStream(HBE_STREAM hbeStream)
{
	// DeInit encoder
	return MP2EncClose(hbeStream);
}



DLLEXPORT VOID		beVersion(PBE_VERSION pbeVersion)
{
	// DLL Release date
	char lpszDate[20]={'\0',};
	char lpszTemp[5]={'\0',};

	// Set Major version number
	pbeVersion->byMajorVersion=pbeVersion->byDLLMajorVersion=1;

	// Set minor version number
	pbeVersion->byMinorVersion=pbeVersion->byDLLMinorVersion=13;

	// Get compilation date
	strcpy(lpszDate,__DATE__);

	// Get the first three character, which is the month
	strncpy(lpszTemp,lpszDate,3);

	// Set month
	if (strcmp(lpszTemp,"Jan")==0)	pbeVersion->byMonth=1;
	if (strcmp(lpszTemp,"Feb")==0)	pbeVersion->byMonth=2;
	if (strcmp(lpszTemp,"Mar")==0)	pbeVersion->byMonth=3;
	if (strcmp(lpszTemp,"Apr")==0)	pbeVersion->byMonth=4;
	if (strcmp(lpszTemp,"May")==0)	pbeVersion->byMonth=5;
	if (strcmp(lpszTemp,"Jun")==0)	pbeVersion->byMonth=6;
	if (strcmp(lpszTemp,"Jul")==0)	pbeVersion->byMonth=7;
	if (strcmp(lpszTemp,"Aug")==0)	pbeVersion->byMonth=8;
	if (strcmp(lpszTemp,"Sep")==0)	pbeVersion->byMonth=9;
	if (strcmp(lpszTemp,"Oct")==0)	pbeVersion->byMonth=10;
	if (strcmp(lpszTemp,"Nov")==0)	pbeVersion->byMonth=11;
	if (strcmp(lpszTemp,"Dec")==0)	pbeVersion->byMonth=12;

	// Get day of month string (char [4..5])
	pbeVersion->byDay=atoi( lpszDate + 4 );

	// Get year of compilation date (char [7..10])
	pbeVersion->wYear=atoi( lpszDate + 7 );

	memset(pbeVersion->zHomepage,0x00,BE_MAX_HOMEPAGE);

	strcpy(pbeVersion->zHomepage,"http://cdexos.sourceforge.net");

#ifdef HAVE_NASM
	pbeVersion->byMMXEnabled=1;
#else
	pbeVersion->byMMXEnabled=0;
#endif

	pbeVersion->byAlphaLevel = 0;
	pbeVersion->byBetaLevel = 0;

	memset( pbeVersion->btReserved, 0x00, sizeof( pbeVersion->btReserved ) );

}



DLLEXPORT BE_ERR beEncodeChunk(HBE_STREAM hbeStream, DWORD nSamples, 
			PSHORT pSamples, PBYTE pOutput, PDWORD pdwOutput)
{
	DWORD nSample;

	static FLOAT fBuffer_l[ MPEGFRAMESIZE ];
	static FLOAT fBuffer_r[ MPEGFRAMESIZE ];

	PSETTINGS pSettings = GetStreamSettings( hbeStream );



	if ( 1 == pSettings->pFrame->nChannels )
	{
		for ( nSample = 0; nSample < nSamples; nSample++ )
		{
			fBuffer_l[ nSample ] = (FLOAT)pSamples[ nSample ];
		}
	}
	else
	{
		int nSampleIndex = 0;

		for ( nSample = 0; nSample < nSamples /2 ; nSample++ )
		{
			fBuffer_l[ nSample ] = (FLOAT)pSamples[ nSampleIndex++ ];
			fBuffer_r[ nSample ] = (FLOAT)pSamples[ nSampleIndex++ ];
		}
	}

	return MP2EncEncodeFrame(	hbeStream,
								nSamples,
								fBuffer_l,
								fBuffer_r,
								pOutput,
								pdwOutput );
}

// accept floating point audio samples, scaled to the range of a signed 16-bit
//  integer (within +/- 32768), in non-interleaved channels  -- DSPguru.

DLLEXPORT BE_ERR beEncodeChunkFloatS16NI(HBE_STREAM hbeStream, DWORD nSamples, 
			PFLOAT buffer_l, PFLOAT buffer_r, PBYTE pOutput, PDWORD pdwOutput)
{
	return MP2EncEncodeFrame(	hbeStream,
								nSamples,
								buffer_l,
								buffer_r,
								pOutput,
								pdwOutput );
}


BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  ul_reason_for_call, 
                      LPVOID lpReserved)
{
    switch( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
		break;
		case DLL_THREAD_ATTACH:
		break;
	    case DLL_THREAD_DETACH:
		break;
	    case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}



