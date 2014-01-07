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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>


#include "machine.h"
#include "mp2enc.h"
#include "common.h"
#include "layer2.h"
#include "AvailableBits.h"
#include "Subband.h"
#include "Psycho_0.h"
#include "Psycho_I.h"
#include "Psycho_II.h"
#include "fft.h"



#define MAX_STREAMS 10

char strTmp[255];


// Local function prototypes
static MP2ERR InitEncoder(PHMP2_STREAM	phStream);
static MP2ERR DeInitEncoder(HMP2_STREAM hStream);

static MP2ERR GlobalInit(PHMP2_STREAM	phStream);
static void GlobalDeInit();

static SETTINGS g_Settings[ MAX_STREAMS ];


// Define global variables
int					qsfactor = 0;		// quality speed factor used in loop.c
int					swapbytes = FALSE;	// force byte swapping
FILE*				musicin;
//BITSTREAM   bs;
char*				programName;
int					iswav=0;
int					autoconvert=FALSE;
int					resample=FALSE;

static int			g_nActiveStreams=0;

static MP2ERR GlobalInit(PHMP2_STREAM	phStream)
{

	if ( g_nActiveStreams>=MAX_STREAMS )
		return MP2ERR_NO_MORE_HANDLES;

	if (g_nActiveStreams==0)
	{
		// Initialize the allocation tables
		InitBitAllocTables();
	}

	// Assign stream number
	*phStream=g_nActiveStreams;

	// clear settings structure of the active stream
	memset(&g_Settings[*phStream],0,sizeof(SETTINGS));

	// Increase the number of active streams
	g_nActiveStreams++;

	return MP2ERR_OK;
}

static void GlobalDeInit()
{
	g_nActiveStreams--;
}

static MP2ERR InitEncoder(PHMP2_STREAM	phStream)
{
	MP2ERR		err;
	PSETTINGS	pSettings=NULL;

	fft_init();

	// Init stream independent vars, and get stream number
	if ( (err= GlobalInit(phStream) ) != MP2ERR_OK )
		return err;

	// Get a pointer to the settings of this stream number
	pSettings = &g_Settings[ *phStream ];

	// Allocate frame strucutre
	pSettings->pFrame=(PFRAME)calloc(1,sizeof(FRAME));

	// Allocate header strucutre
	pSettings->pFrame->pHeader=(PHEADER)calloc(1,sizeof(HEADER));

	// Allocate Bitstream strucutre
	pSettings->pBitstream=(PBITSTREAM)calloc(1,sizeof(BITSTREAM));

    // Allocate dynamic memory buffers
	pSettings->sb_sample	=(SBS* )calloc(1,sizeof(SBS) );
    pSettings->j_sample		=(JSBS*)calloc(1,sizeof(JSBS));
    pSettings->subband		=(SUB* )calloc(1,sizeof(SUB) );

    // no table loaded
	pSettings->pFrame->tab_num = -1;

    pSettings->pFrame->alloc = NULL;

	// Default: MPEG-1
    pSettings->pFrame->pHeader->version = MPEG_AUDIO_ID;

	pSettings->nBitsPerSlot = 8;
	pSettings->nSamplesPerFrame = MPEGFRAMESIZE; 

	return MP2ERR_OK;
}


static MP2ERR DeInitEncoder(HMP2_STREAM hStream)
{
	PSETTINGS pSettings = &g_Settings[ hStream ];

	if (pSettings)
	{
		if (pSettings->pBitstream)
		{
			free(pSettings->pBitstream);
			pSettings->pBitstream=NULL;
		}

		if (pSettings->pFrame)
		{
			if (pSettings->pFrame->pHeader)
			{
				free(pSettings->pFrame->pHeader);
				pSettings->pFrame->pHeader=NULL;
			}

			free(pSettings->pFrame);
			pSettings->pFrame=NULL;
		}

		if (pSettings->sb_sample)
		{
			free(pSettings->sb_sample);
			pSettings->sb_sample=NULL;
		}

		if (pSettings->j_sample)
		{
			free(pSettings->j_sample);
			pSettings->j_sample=NULL;
		}

		if (pSettings->subband)
		{
			free(pSettings->subband);
			pSettings->subband=NULL;
		}


		//free(pSettings);
		//pSettings=NULL;
	}

	// Free global resources
	GlobalDeInit();

	return MP2ERR_OK;
}


MP2ERR	MP2EncOpen
	(
		PMP2ENC_CONFIG	pConfig,
		PDWORD			dwSamples, 
		PDWORD			dwBufferSize,
		PHMP2_STREAM	phStream)
{
	// Initialize the encoder
	MP2ERR		err;
	int			nCRC=pConfig->bCRC;
	int			nVBR=(nCRC>>12)&0x0F;
	PHEADER		pHeader=NULL;
	PSETTINGS	pSettings=NULL;

	// Init global and stream variables
	if ( ( err=InitEncoder(phStream) ) != MP2ERR_OK )
		return err;

	// Get a pointer to the settings of this stream number
	pSettings=&g_Settings[*phStream];

	// Get a pointer to the header, easier to work with
	pHeader = pSettings->pFrame->pHeader;

	// Copy configuration paramters
	//StreamConfig[*phStream]=pConfig,sizeof(BE_CONFIG));

	// Set MP3 buffer size
	*dwBufferSize=BUFFER_SIZE;

	// Setup MP3 paramters
    pHeader->emphasis=0;
    pHeader->mode_ext=0;

	// pSettings->pFrame->pHeader.extension;
    pHeader->mode		=pConfig->dwMode;
    pHeader->copyright	=pConfig->bCopyright;
    pHeader->original	=pConfig->bOriginal;
    pHeader->bPrivate	=pConfig->bPrivate;
	pHeader->bCRC		=pConfig->bCRC;

	// Insert Ancillary data into the data stream ?
	if ( pConfig->bWriteAncil )
	{
		if (pConfig->dwMode== MP2ENC_MD_MONO)
			pSettings->nAncilBits=3*8;
		else
			pSettings->nAncilBits=5*8;
	}

	pSettings->bUsePadBit=(pConfig->bNoPadding)?0:1;

	// Use VBR?
	pSettings->bUseVbr=pConfig->bUseVbr;

	if ( pSettings->bUseVbr )
	{
		// nVbr is between 1..10, where 1 is the highest quality and 10 the worst
		// nVbrQuality on the other hand, has a different scaling,
		// where -5 is the lowest, and +15 is about the highest
		pSettings->nVbrQuality= -5+(10-pConfig->dwVbrQuality)*2;

		// No padding allowed in VBR mode
		pSettings->bUsePadBit=0;

		// force stereo mode
		pHeader->mode= MPG_MD_STEREO;
	}


	// Set layer depending on encoder type
	pHeader->lay = 2;

	// Set MPEG version to detault
	pHeader->version=MPEG_AUDIO_ID;

	// Set sample frequency index, and MPEG version
	if((pHeader->sampling_frequency = SmpFrqIndex(pConfig->dwSampleRate,&pHeader->version)) < 0) 
	{
		return MP2ERR_INVALID_FORMAT_PARAMETERS;
	}
	
	// Set bitrate index depending on the layer
    pSettings->pFrame->pHeader->bitrate_index=BitrateIndex(pHeader->lay,pConfig->dwBitrate,pHeader->version);

	// Set min bitrate index, used for VBR encoding
	pSettings->nMinBitRateIndex= pHeader->bitrate_index;

	pSettings->dwPsyModel	= pConfig->dwPsyModel;
	pSettings->dwQuickMode	= pConfig->dwQuickMode;
	pSettings->nDebugLevel	= pConfig->nDebugLevel;

	// Set header parameters    
    hdr_to_frps( pSettings->pFrame );

	// Clear min/max energies
	pSettings->nMaxLeftEnergy=0;
	pSettings->nMaxRightEnergy=0;


	// Set number of input samples depending on the number of samples
	if (pHeader->mode== MP2ENC_MD_MONO)
	{
		// Mono channel, thus MPEGFRAMSIZE samples needed
		*dwSamples=MPEGFRAMESIZE;
	}
	else
	{
		// For Stereo.Joint stereo and dual channel, need 2*MPEGFRAMESIZE samples
		*dwSamples=MPEGFRAMESIZE*2;
	}


	// Init appropriate psycho acoutic model
	switch (pSettings->dwPsyModel)
	{
		case 0:
		case 1:
			break;
		case 2: 
			Init_Psycho_II(pSettings);
		default:
			MP2LibError("Invalid Psycho Acoustic Model setting (%d)\n",pSettings->dwPsyModel);
	}

	if ( pSettings->nDebugLevel > 0 )
	{
		DebugPrintf("************************* MP2 Encoder Settings ***********************\n");
		DebugPrintf("Samplerate        %d\n",(INT)(s_freq[pHeader->version][pHeader->sampling_frequency]*1000));
		DebugPrintf("Bitrate           %d\n",bitrate[pHeader->version][pHeader->lay-1][pHeader->bitrate_index]);
		DebugPrintf("Layer             %d\n",pSettings->pFrame->pHeader->lay);
		DebugPrintf("MPEG              %s\n",(pSettings->pFrame->pHeader->version==0)?"II":"I");
		DebugPrintf("PsyModel          %d\n",pSettings->dwPsyModel);
		DebugPrintf("dwQuickMode       %d\n",pSettings->dwQuickMode);
		DebugPrintf("nAncilBits        %d\n",pSettings->nAncilBits);
		DebugPrintf("bUsePadBit        %d\n",pSettings->bUsePadBit);
		DebugPrintf("CRC               %s\n",(pHeader->bCRC)?"yes":"no");
		DebugPrintf("Original          %s\n",(pHeader->original)?"yes":"no");
		DebugPrintf("Copyright         %s\n",(pHeader->copyright)?"yes":"no");
		DebugPrintf("Private           %s\n",(pHeader->bPrivate)?"yes":"no");
		DebugPrintf("Use VBR %s, Quality is %d\n",pSettings->bUseVbr?"TRUE":"FALSE",pSettings->nVbrQuality);
		DebugPrintf("**********************************************************************\n");
	}


	// Everything went OK, thus return SUCCESSFUL
	return MP2ERR_OK;
}



MP2ERR	MP2EncClose
	(
		HMP2_STREAM hStream
	)
{

	// Deinit the encoder
	return DeInitEncoder(hStream);
}




MP2ERR	MP2EncEncodeFrame (
		HMP2_STREAM	hStream,
		DWORD		nSamples,
		PFLOAT		buffer_l,
		PFLOAT		buffer_r,
		PBYTE		pOutput,
		PDWORD		pdwOutput	
		)
{
	int				i,k=0;
	int				nIndex=0;
	PSETTINGS		pSettings=&g_Settings[hStream];


	// Initialize bitstream
	pSettings->pBitstream->buf_byte_idx = 0;
	pSettings->pBitstream->buf_bit_idx	= 8;
	pSettings->pBitstream->totbit		= 0;
	pSettings->pBitstream->buf			= pOutput;

	// clear output buffer
	//memset(pOutput,0,BUFFER_SIZE);

	// clean min/max energy value
	if ( pSettings->nAncilBits )
	{
		pSettings->nMaxLeftEnergy=0;
		pSettings->nMaxRightEnergy=0;
	}

	pSettings->buffer[0] = buffer_l;
	pSettings->buffer[1] = buffer_r;


	// Get Min/Max settings for ancillary data
	if ( pSettings->nAncilBits )
	{
		if ( 1 == pSettings->pFrame->nChannels )
		{
			for ( i=0; i< (int)nSamples; i++ )
			{
				pSettings->nMaxLeftEnergy = max( pSettings->nMaxLeftEnergy,
												 (int)buffer_l[i] );
			}
		}
		else
		{
			for ( i = 0 ; i < (int)nSamples / 2; i++ )
			{
				pSettings->nMaxLeftEnergy = max(	pSettings->nMaxLeftEnergy, 
													(int)buffer_l[ i ] );
				pSettings->nMaxRightEnergy = max(	pSettings->nMaxRightEnergy,
													(int)buffer_r[ i ] );
			}
		}
	}

	pSettings->adb = available_bits( pSettings );

	if ( pSettings->nDebugLevel > 1 )
	{
		DebugPrintf("Encoding Frame=%d adb=%d padding=%d\n",
			pSettings->nFrame,
			pSettings->adb,
			pSettings->pFrame->pHeader->padding
		);
	}

	// NUMBLOCKS*SCALE_BLOCK*SBLIMIT= 3*12*32 = 1152 PCM samples per channel
	for ( i = 0; i < 3; i++ )
	{
		waf_subband( &pSettings->buffer[0][SCALE_BLOCK*SBLIMIT*i],0,(*pSettings->sb_sample)[0][i] );

		if ( pSettings->pFrame->nChannels > 1 )
		{
			waf_subband(	&pSettings->buffer[ 1 ][ SCALE_BLOCK * SBLIMIT * i ],
							1,
							(*pSettings->sb_sample)[ 1 ][ i ] );
		}	
	}

	// CALCULATE ALL SCALE FACTORS FOR VBR
	if (pSettings->bUseVbr)
		II_scale_factor_calc(*pSettings->sb_sample, pSettings->scalar, pSettings->pFrame->nChannels, SBLIMIT);
	else
		II_scale_factor_calc(*pSettings->sb_sample, pSettings->scalar, pSettings->pFrame->nChannels, pSettings->pFrame->sblimit);


	if(pSettings->pFrame->actual_mode == MPG_MD_JOINT_STEREO)
	{
		II_combine_LR(*pSettings->sb_sample, *pSettings->j_sample, pSettings->pFrame->sblimit);
		II_scale_factor_calc(pSettings->j_sample, &pSettings->j_scale, 1, pSettings->pFrame->sblimit);
	}       // this way we calculate more mono than we need but it is cheap
	
	if ( (pSettings->nFrame % pSettings->dwQuickMode)==0)
	{
		if ( pSettings->dwPsyModel == 1 )
		{
			double	max_sc[2][SBLIMIT];
			pick_scale(pSettings->scalar, pSettings->pFrame, max_sc);
			Psycho_I(pSettings->buffer, max_sc, pSettings->smr, pSettings->pFrame);
		}
		else
		{
			for (k=0;k<pSettings->pFrame->nChannels;k++)
			{
				if ( pSettings->dwPsyModel == 0 )
				{
					Psycho_0(	pSettings,
								&pSettings->buffer[k][0],
								&pSettings->sam[k][0],
								k, 
								pSettings->pFrame->pHeader->lay,
								pSettings->smr[k],
								(FLOAT)s_freq[pSettings->pFrame->pHeader->version][pSettings->pFrame->pHeader->sampling_frequency]*1000
								);
				}
				else
				{
					Psycho_II(	pSettings,
								&pSettings->buffer[k][0],
								&pSettings->sam[k][0],
								k, 
								pSettings->pFrame->pHeader->lay,
								pSettings->smr[k],
								(FLOAT)s_freq[pSettings->pFrame->pHeader->version][pSettings->pFrame->pHeader->sampling_frequency]*1000
								);
				}
			}
		}
	}

	// Calculate the transmission patterns for all subbands if VBR is enabled
	if (pSettings->bUseVbr)
		II_transmission_pattern(pSettings->scalar, pSettings->scfsi, pSettings->pFrame,SBLIMIT);
	else
		II_transmission_pattern(pSettings->scalar, pSettings->scfsi, pSettings->pFrame,pSettings->pFrame->sblimit);


	II_main_bit_allocation(pSettings->smr, pSettings->scfsi, pSettings->bit_alloc, &pSettings->adb, pSettings->pFrame, pSettings);

	
	if (pSettings->pFrame->pHeader->bCRC)
		II_CRC_calc(pSettings->pFrame, pSettings->bit_alloc, pSettings->scfsi, &pSettings->crc);
	
	// Encode header
	encode_info(pSettings->pFrame, pSettings->pBitstream);
	
	// Encode CRC, if required
	if (pSettings->pFrame->pHeader->bCRC)
		encode_CRC(pSettings->crc, pSettings->pBitstream);
	
	II_encode_bit_alloc(pSettings->bit_alloc, pSettings->pFrame, pSettings->pBitstream);
	II_encode_scale(pSettings->bit_alloc, pSettings->scfsi, pSettings->scalar, pSettings->pFrame, pSettings->pBitstream);
	II_subband_quantization(pSettings->scalar, *pSettings->sb_sample, pSettings->j_scale, *pSettings->j_sample, pSettings->bit_alloc, *pSettings->subband, pSettings->pFrame);
	II_sample_encoding(*pSettings->subband, pSettings->bit_alloc, pSettings->pFrame, pSettings->pBitstream);

	// fill unsued bits with zeros
	for (i=0;i<pSettings->adb;i++)
		put1bit(pSettings->pBitstream, 0);

	// write ancillary data, if required
	if (pSettings->nAncilBits)
	{
		UINT temp;

		pSettings->nMaxRightEnergy=min(pSettings->nMaxRightEnergy,0x7FF0);
		pSettings->nMaxLeftEnergy=min(pSettings->nMaxLeftEnergy,0x7FF0);

		if (pSettings->pFrame->nChannels==2)
		{
			temp=(pSettings->nMaxRightEnergy>>8);
			putbits(pSettings->pBitstream,temp,8);

			temp=(pSettings->nMaxRightEnergy&0xFF);
			putbits(pSettings->pBitstream,temp,8);
		}

		temp=0x00;
		putbits(pSettings->pBitstream,temp,8);

		temp=(pSettings->nMaxLeftEnergy>>8);
		putbits(pSettings->pBitstream,temp,8);
		temp=(pSettings->nMaxLeftEnergy&0xFF);
		putbits(pSettings->pBitstream,temp,8);
	}

	
	// Bump up frame number
	pSettings->nFrame++;

	// Set number of output bytes
	*pdwOutput=pSettings->pBitstream->totbit/8;

	// Return success
	return MP2ERR_OK;
}


PSETTINGS GetStreamSettings( DWORD hStream )
{
	PSETTINGS pReturn = NULL;

	if ( hStream < MAX_STREAMS )
	{
		pReturn = &g_Settings[ hStream ];
	}

	return pReturn;
}


