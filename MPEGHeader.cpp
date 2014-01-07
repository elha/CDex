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
#include <math.h>
#include "MPEGHeader.h"
#include "config.h"

INITTRACE( _T( "MPEGHeader" ) );

#ifdef DEBUG
	#define DEBUG_SHOW_TOC
#endif



DWORD MPEGSamplerates[2][3] = { {44100, 48000, 32000},{22050, 24000, 16000}};

DWORD MPEGBitrates[3][3][15] = 
{
	{
		{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
		{0,32,48,56,64 ,80 ,96 ,112,128,160,192,224,256,320,384},
		{0,32,40,48,56 ,64 ,80 ,96 ,112,128,160,192,224,256,320}
	},
	{
		{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256},
		{0, 8,16,24,32,40,48,56 , 64, 80, 96,112,128,144,160},
		{0, 8,16,24,32,40,48,56 , 64, 80, 96,112,128,144,160}
	},
	{
		{0, 8,16,24,32,40,48,56 , 64, 80, 96,112,128,144,160},
		{0, 8,16,24,32,40,48,56 , 64, 80, 96,112,128,144,160},
		{0, 8,16,24,32,40,48,56 , 64, 80, 96,112,128,144,160},
	}
};

// 4   Xing
// 4   flags
// 4   frames
// 4   bytes
// 100 toc

/*-------------------------------------------------------------*/
static int ExtractI4(unsigned char *pData)
{
	int x;
	// big endian extract

	x = pData[0];
	x <<= 8;
	x |= pData[1];
	x <<= 8;
	x |= pData[2];
	x <<= 8;
	x |= pData[3];

	return x;
}

// CONSTRUCTOR
MPEGHeader::MPEGHeader()
{
	// Clear header
	Init();
}

// DESTRUCTOR
MPEGHeader::~MPEGHeader()
{
}

VOID MPEGHeader::Init()
{
	m_nFlags=0;
	m_nFrames=0;
    m_nBytes=0;
    m_nVbrScale=0;
	m_bIsXingHeader=FALSE;
	m_nSampleRate=0;
	m_nBitRate=0;
	m_nFrameSize=0;
	memset(m_btToc,0x00,sizeof(m_btToc));
	memset(&m_RawMPEGHeader,0x00,sizeof(m_RawMPEGHeader));
}


INT MPEGHeader::CalcFrameSize()
{
	m_nFrameSize=0;

	if (GetSampleRate()==0)
	{
		ASSERT(FALSE);
		return 0;
	}

	switch (GetLayer())
	{
		case 1:
			m_nFrameSize = (12 * GetBitRate()*1000)/GetSampleRate();
			if (m_RawMPEGHeader.Padding)
				m_nFrameSize++;
			m_nFrameSize  <<= 2;		// one slot is 4 bytes long
		break;
		case 2:
		case 3:
			m_nFrameSize = (144 * GetBitRate()*1000) / GetSampleRate();

			if (m_RawMPEGHeader.Version == MPEG_II)
				m_nFrameSize/=2;

			if (m_RawMPEGHeader.Padding)
				m_nFrameSize++;
		break;
		default:
			ASSERT(FALSE);
	}

	return m_nFrameSize;
}


/*-------------------------------------------------------------*/
BOOL MPEGHeader::ReadHeader(BYTE *pData, int nSize, int* pnOffset)
{
	unsigned int	i=0;
	BOOL bValidHeader=FALSE;

	while (i<(nSize-sizeof(m_RawMPEGHeader)))
	{
		// Copy header bytes
		memcpy(&m_RawMPEGHeader,&pData[i],sizeof(m_RawMPEGHeader));

		// Check Header
		if (m_RawMPEGHeader.FrameSyncH==0xFF && m_RawMPEGHeader.FrameSyncL==0x0F)
		{
			// we found it
			bValidHeader=TRUE;
			break;
		}

		i++;
	}

	// check if we have found a valid header
	if (bValidHeader==FALSE)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	pData+=i;

	// set the offset if possible
	if (pnOffset)
	{
		*pnOffset=i;
	}

	// get Xing header data
	m_nFlags = 0;     // clear to null incase fail

	// Set SampleRate
	m_nSampleRate= MPEGSamplerates[1-m_RawMPEGHeader.Version][m_RawMPEGHeader.SampleRateIndex];

	// Set BitRate
	m_nBitRate= MPEGBitrates[1-m_RawMPEGHeader.Version][3-m_RawMPEGHeader.Layer][m_RawMPEGHeader.BitRateIndex];

	// Set Frame Size
	CalcFrameSize();


	// Determine offset of header
	if( m_RawMPEGHeader.Version==MPEG_I ) 
	{
		// MPEG-I
		if( m_RawMPEGHeader.ChannelMode != 3 )
		{
			// MONO
			pData+=(32+4);
		}
		else
		{
			// STEREO, DUAL-CHANNEL or JOINT-STEREO
			pData+=(17+4);
		}
	}
	else
	{
		// MPEG-II
		if( m_RawMPEGHeader.ChannelMode != 3 )
		{
			// MONO
			pData+=(17+4);
		}
		else
		{
			// STEREO, DUAL-CHANNEL or JOINT-STEREO
			pData+=(9+4);
		}
	}

	// Check for Xing Tag 
	if( pData[0] != 'X' ) return TRUE;
	if( pData[1] != 'i' ) return TRUE;
	if( pData[2] != 'n' ) return TRUE;
	if( pData[3] != 'g' ) return TRUE;
	pData+=4;

	m_nFlags= ExtractI4(pData);
	pData+=4;

	if( m_nFlags & FRAMES_FLAG )
	{
		m_nFrames= ExtractI4(pData); pData+=4;
	}

	if( m_nFlags & BYTES_FLAG )
	{
		m_nBytes= ExtractI4(pData); pData+=4;
	}

	if( m_nFlags & TOC_FLAG )
	{
		// Get the first 100 bytes
		for(i=0;i<100;i++)
			m_btToc[i] = pData[i];

		// Increase buffer
		pData+=100;
	}

	if( m_nFlags & VBR_SCALE_FLAG )
	{
		m_nVbrScale = ExtractI4(pData); pData+=4;
	}

#ifdef DEBUG_SHOW_TOC
	for(i=0;i<100;i++)
	{
		if( (i%10) == 0 ) printf("\n");
		{
			LTRACE( _T( " %3d" ), m_btToc[ i ] );
		}
	}
#endif

	// This is a valid Xing Header
	m_bIsXingHeader=TRUE;

	return TRUE;
}



/*-------------------------------------------------------------*/
int seek_vbr(unsigned char TOC[100], int file_bytes, float percent)
{
	// interpolate in TOC to get file seek point in bytes
	int a, seekpoint;
	float fa, fb, fx;


	if( percent < 0.0f )   percent = 0.0f;
	if( percent > 100.0f ) percent = 100.0f;

	a = (int)percent;
	if( a > 99 ) a = 99;
	fa = TOC[a];
	if( a < 99 )
	{
		fb = TOC[a+1];
	}
	else 
	{
		fb = 256.0f;
	}


	fx = fa + (fb-fa)*(percent-a);

	seekpoint = (int)((1.0f/256.0f)*fx*file_bytes); 


	return seekpoint;
}



