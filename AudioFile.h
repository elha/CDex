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


#ifndef AUDIOFILE_INCLUDED
#define AUDIOFILE_INCLUDED

#include <mmsystem.h>
#include <mmreg.h>
#include "Config.h"


extern "C"
{
	#include "sndfile.h"	
}


// Single byte alignment
#pragma pack( push, 1 )

// AF:TODO EXCLUDE FOR MSVC7 and up
//#define WAVE_FORMAT_SONY_SCX   ( 0x0270 )

typedef struct sonyscxwaveformat_tag {
  WAVEFORMATEX  wfx;
  BYTE			btExtraDontKnow[ 14 ];
} SONYSCXWAVEFORMAT;



// not needed for MSVC 6.0 and higher
#if ( _MSC_VER <= 1100 )
	#define WAVE_FORMAT_MPEGLAYER3 ( 0x0055 )

	#define MPEGLAYER3_WFX_EXTRA_BYTES   12

	// WAVE_FORMAT_MPEGLAYER3 format sructure
	//
	typedef struct mpeglayer3waveformat_tag {
	  WAVEFORMATEX  wfx;
	  WORD          wID;
	  DWORD         fdwFlags;
	  WORD          nBlockSize;
	  WORD          nFramesPerBlock;
	  WORD          nCodecDelay;
	} MPEGLAYER3WAVEFORMAT;

	typedef MPEGLAYER3WAVEFORMAT          *PMPEGLAYER3WAVEFORMAT;

	//==========================================================================;

	#define MPEGLAYER3_ID_UNKNOWN            0
	#define MPEGLAYER3_ID_MPEG               1
	#define MPEGLAYER3_ID_CONSTANTFRAMESIZE  2

	#define MPEGLAYER3_FLAG_PADDING_ISO      0x00000000
	#define MPEGLAYER3_FLAG_PADDING_ON       0x00000001
	#define MPEGLAYER3_FLAG_PADDING_OFF      0x00000002

#endif // _MSC_VER <= 1100


// Define some FOURCC tags
#define FOURCC_WAVE	mmioFOURCC ('W', 'A', 'V', 'E')
#define FOURCC_FMT	mmioFOURCC ('f', 'm', 't', ' ')
#define FOURCC_DATA	mmioFOURCC ('d', 'a', 't', 'a')

// Define CHUNKHDR
typedef struct CHUNKHDR 
{
  FOURCC ckid;		// chunk ID
  DWORD dwSize; 	// chunk size
} CHUNKHDR;


// Simplified header for standard WAV files
typedef struct PCMWAVEHDR_TAG 
{
  CHUNKHDR		chkRiff;
  FOURCC		fccWave;
  CHUNKHDR		chkFmt;
  PCMWAVEFORMAT pwf;
  CHUNKHDR		chkData;
} PCMWAVEHDR;


class CWAV : public CObject
{
private:
	DWORD	m_dwDataSize;				// Size of data section
	WORD	m_wSampleRate;				// Samples per sec per channel
	WORD	m_wChannels;				// Number of channels (1=MONO, 2=STEREO)
	WORD	m_wBitsPerSample;			// Should be 8 or 16 bits per sample
	CUString	m_strFileName;				// Just for debugging purposes
	DWORD	m_dwSamplesToConvert;
	CWAV*	m_pTmpWav;


private:
	SNDFILE*	m_pSndFile;
	SF_INFO		m_wfInfo;

public:

	// CONSTRUCTORS
	CWAV();

	// DESTRUCTOR
	~CWAV();

	// METHODS
	CDEX_ERR	OpenForWrite(const CUString& strFileName,WORD wSampleRate,WORD wBitsPerSample,WORD wNumChannels);
	CDEX_ERR	OpenForRead(CUString& strFileName);
	CDEX_ERR	CloseStream();

	int			SaveDataChunk(BYTE* pbtDataPtr,DWORD dwNumBytes);
	int			ReadDataChunk(BYTE* pbtDataPtr,DWORD dwNumBytes);

	BOOL		GetMaxWaveValue(INT& nPercent,INT& nPeakValue);

	DWORD		StartNormalizeAudioFile(CUString strFileName);
	BOOL		NormalizeAudioFileChunk(DOUBLE dNormFactor,int& nPerCentComplete);
	void		CloseNormalizeAudioFile(CUString strFileName,BOOL bAbort=FALSE, BOOL bReplace=TRUE);

	CDEX_ERR	ConvertToRiffWav(CUString strFileName,int nBitRate,int nSampleRate, WORD wChannels,int nLayer);
	CDEX_ERR	StripRiffWavHeader(CUString strFileName);


	// ACCESSORS
	WORD	GetBitsPerSample() const	{return m_wBitsPerSample;}
	WORD	GetNumChannels() const		{return m_wChannels;}
	WORD	GetSampleRate() const		{return m_wSampleRate;}
	DWORD	GetDataSize() const			{return m_dwDataSize;}
private:
	CDEX_ERR SaveAtracTag( FILE* pFile, int nBitRate, int nSampleRate, WORD wChannels );
	CDEX_ERR SaveMP3Tag( FILE* pFile, int nBitRate, int nSampleRate, WORD wChannels );
	CDEX_ERR SaveMP2Tag( FILE* pFile, int nBitRate, int nSampleRate, WORD wChannels );

};

#pragma pack(pop)



#endif