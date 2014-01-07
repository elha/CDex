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


#ifndef MPEGHEADER_H_INCLUDED
#define MPEGHEADER_H_INCLUDED




/*

	FILE STRUCTURE AUDIO MPEG FILE
--------------------------------------------------------------------------
file

	a file has one or more 'frames'.

	a frame has a 'frame header' and 'frame data'
--------------------------------------------------------------------------
frame length

	the frame length depends on the format and can vary per frame.
	the frame ends before the next sync header (see frame header).
--------------------------------------------------------------------------
frame header

	the frame header of an audio mpeg file consist of 4 bytes, which
	are at the beginning of a frame. They don't have to be the same
	for every frame. (p.e. the bit 'padding' can change per frame)

	byte       0        1        2        3
	binary 11111111 1111abbc ddddeefg hhjjkmpp		BIG ENDIAN    (Motorola)
           11111111 cbba1111 gfeedddd ppmkjjhh		LITTLE ENDIAN (Intel)

		1 - sync header
		a - version
		b - layer
		c - error protection
		d - bit rate index
		e - sample rate index
		f - padding
		g - extension
		h - channel mode
		j - mode extension
		k - copyright 
		m - original
		p - emphasis
--------------------------------------------------------------------------
1 - sync header

	must be 11111111 1111xxxx ($fffx)
--------------------------------------------------------------------------
a - version

	0 = mpg-ii
	1 = mpg-i
--------------------------------------------------------------------------
b - layer

	0 = layer-1
	3 = layer-2
	2 = layer-3
--------------------------------------------------------------------------
c - error protection

	0 = on
	1 = off
--------------------------------------------------------------------------
d - bit rate

	                     mpg-i
	(hex)   bps@38kHz  bps@44.1kHz  bps@48kHz
	$0          0            0           0
	$1         32           32          32
	$2         64           48          40
	$3         96           56          48
	$4        128           64          56
	$5        160           80          64
	$6        192           96          80
	$7        224          112          96
	$8        256          128         112
	$9        288          160         128
	$a        320          192         160
	$b        352          224         192
	$c        384          256         224
	$d        416          320         256
	$e        448          384         320
	
	                    mpg-ii
	(hex)   bps@16kHz  bps@22kHz  bps@24kHz
	$0          0            0           0
	$1         32            8           8
	$2         48           16          16
	$3         56           24          24
	$4         64           32          32
	$5         80           40          40
	$6         96           48          48
	$7        112           56          56
	$8        128           64          64
	$9        144           80          80
	$a        160           96          96
	$b        176          112         112
	$c        192          128         128
	$d        224          144         144
	$e        256          160         160
--------------------------------------------------------------------------
e - sample rate

	      mpg-i     mpg-ii  (version)
	0 = 44.1 kHz    22 kHz
	1 =   48 kHz    24 kHz
	2 =   38 kHz    16 kHz
--------------------------------------------------------------------------
f - padding

	1 = unused bits are filled (padding required)
	0 = all bits in frame are used
--------------------------------------------------------------------------
g - extension 

	0 = none
	1 = private
--------------------------------------------------------------------------
h - channel mode

	0 = stereo
	1 = joint stereo
	2 = dual channel
	3 = mono
--------------------------------------------------------------------------
j = mode extension

	when encoding with joint stereo, this specifies to which freq.band
	it is bound.

            layer-1&2  layer-3
	0 =     4         0
	1 =     8         4
        2 =    12         8
        3 =    16        16
--------------------------------------------------------------------------
k - copyright

	0 = no
	1 = yes
--------------------------------------------------------------------------
m - original

	0 = no
	1 = yes
--------------------------------------------------------------------------
p - emphasis

	0 = none
	1 = 50/15 microseconds
	3 = CITT j.17
--------------------------------------------------------------------------
*/

#define MPEG_II	0
#define MPEG_I	1

#pragma pack(push,1)

typedef struct MPEGRAMEHEADER_TAG
{
	// BYTE 0
	BYTE	FrameSyncH;						// should be all "1"

	// BYTE 1
	BYTE	ErrorProtection	: 1;
	BYTE	Layer			: 2;
	BYTE	Version			: 1;
	BYTE	FrameSyncL		: 4;

	// BYTE 2
	BYTE	Extention		: 1;
	BYTE	Padding			: 1;
	BYTE	SampleRateIndex	: 2;
	BYTE	BitRateIndex	: 4;

	// BYTE 3
	BYTE	emphasis		: 2;
	BYTE	Original		: 1;
	BYTE	Copyright		: 1;
	BYTE	ModeExtention	: 2;
	BYTE	ChannelMode		: 2;
} MPEGFRAMEHEADER;



typedef struct ID3V2_TAG
{
	CHAR	pzID[3];		// byte 0..2
	BYTE	btMajorRev;		// byte 3
	BYTE	btMinorRev;		// byte 4
	BYTE	btFlags;		// byte 5
	BYTE	btSize[4];		// byte 6..9
}ID3V2;

#pragma pack(pop)





/***********************************************************************
 * A Xing header may be present in the ancillary
 * data field of the first frame of an mp3 bitstream
 * The Xing header (optionally) contains
 *      frames      total number of audio frames in the bitstream
 *      bytes       total number of bytes in the bitstream
 *      toc         table of contents
 *
 * toc (table of contents) gives seek points
 * for random access 
 * the ith entry determines the seek point for
 * i-percent duration
 * seek point in bytes = (toc[i]/256.0) * total_bitstream_bytes
 * e.g. half duration 
 * seek point = (toc[50]/256.0) * total_bitstream_bytes
 ******************************************************************** 
 */



#define FRAMES_FLAG     0x0001
#define BYTES_FLAG      0x0002
#define TOC_FLAG        0x0004
#define VBR_SCALE_FLAG  0x0008

#define FRAMES_AND_BYTES (FRAMES_FLAG | BYTES_FLAG)

// structure to receive extracted header
// toc may be NULL
class MPEGHeader
{
public:
	// CONSTRUCTOR
	MPEGHeader();

	// DESTRUCTOR
	~MPEGHeader();

	// ACCESSORS
	INT	GetLayer() const {return 4-m_RawMPEGHeader.Layer;}
	INT	GetVBRScale() const {return m_nVbrScale;}
	INT	GetNumFrames() const {return m_nFrames;}
	INT	GetToc(int nIndex) const {return m_btToc[nIndex];}
	INT	IsXingHeader() const {return m_bIsXingHeader;}
	INT	GetBitRate() const {return m_nBitRate;}
	INT	GetSampleRate() const {return m_nSampleRate;}
	INT	GetChannels() const {return (m_RawMPEGHeader.ChannelMode==3)?1:2;}
	INT	GetFrameSize() const {return m_nFrameSize;}
	BOOL IsMPEG_I() const {return (m_RawMPEGHeader.Version==MPEG_I);}
	BOOL IsMPEG_II() const {return (m_RawMPEGHeader.Version==MPEG_II);}


	// MUTATORS
	VOID	Init();
	BOOL	ReadHeader(BYTE* pData,int nSize,int* pnOffset=NULL);
	INT		CalcFrameSize();

private:
    int		m_nFlags;			// from Xing header data
    int		m_nFrames;			// total bit stream frames from Xing header data
    int		m_nBytes;			// total bit stream bytes from Xing header data
    int		m_nVbrScale;		// encoded vbr scale from Xing header data
    BYTE	m_btToc[100];		// may be NULL if toc not desired
	BOOL	m_bIsXingHeader;
	int		m_nSampleRate;
	int		m_nBitRate;
	int		m_nFrameSize;
	MPEGFRAMEHEADER m_RawMPEGHeader;
};


// return 0=fail, 1=success
// X   structure to receive header data (output)
// buf bitstream input 

int seek_vbr(unsigned char TOC[100], int file_bytes, float percent);
// return seekpoint in bytes (may be at eof if percent=100.0)
// TOC = table of contents from Xing header
// file_bytes = number of bytes in mp3 file
// percent = play time percentage of total playtime. May be
//           fractional (e.g. 87.245)




extern DWORD MPEGSamplerates[2][3];
extern DWORD MPEGBitrates[3][3][15]; 


#endif