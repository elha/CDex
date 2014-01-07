/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006, 2007, 2008 Georgy Berdyshev
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


#include "StdAfx.h"
#include <stdio.h>
#include "AudioFile.h"
#include "Config.h"
#include "ID3Tag.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BLOCKSIZE 16000
static short gpsSamples[BLOCKSIZE];

INITTRACE( _T( "AudioFile" ) );


// CWAV CONSTRUCTOR
CWAV::CWAV()
{
	m_pTmpWav = NULL;
	
	m_dwDataSize = 0;
	m_wChannels = 2;
	m_wBitsPerSample = 16;
	m_wSampleRate = 44100;
	m_dwSamplesToConvert = 0;
	
	m_pSndFile = NULL;

	(void)memset( &m_wfInfo, 0x00, sizeof(SF_INFO) );
}


// CWAV DESTRUCTOR
CWAV::~CWAV()
{
	delete m_pTmpWav;
	m_pTmpWav = NULL;
	CloseStream();
}


CDEX_ERR CWAV::CloseStream()
{
	if (m_pSndFile)
	{
		if (sf_close(m_pSndFile) !=0)
		{
			ASSERT(FALSE);
			return CDEX_ERROR;
		}
		m_pSndFile = NULL;
	}
	// No Errors
	return CDEX_OK;       
}



// CWAV::OpenForRead Method
CDEX_ERR CWAV::OpenForWrite(const CUString& strFileName, WORD wSampleRate, WORD wBitsPerSample, WORD wChannels)
{
	// Clear m_wfInfo struct
	(void)memset( &m_wfInfo, 0, sizeof( SF_INFO ) );
	
	m_wChannels = m_wfInfo.channels = wChannels;
	m_wBitsPerSample = wBitsPerSample;
	m_wfInfo.frames = -1;
	m_wfInfo.sections = 1;
	m_wSampleRate = m_wfInfo.samplerate = wSampleRate;
	
	int nCompression = SF_FORMAT_PCM_16;
	int	nFormat = SF_FORMAT_WAV;
	
	// Set format options
	m_wfInfo.format =(nFormat | nCompression);

    CUStringConvert strCnv;

	// Open stream
    #ifdef _UNICODE
	if (!(m_pSndFile = sf_open(	(const tchar*)strCnv.ToT( strFileName ),
								SFM_WRITE,
								&m_wfInfo ) ) )
    #else
	if (!(m_pSndFile = sf_open(	strCnv.ToT( strFileName ),
								SFM_WRITE,
								&m_wfInfo ) ) )
    #endif
	{
		ASSERT(FALSE);
		return CDEX_ERROR;
	}
	
	// return success
	return CDEX_OK;
}


int CWAV::SaveDataChunk(BYTE*	pbtDataPtr, DWORD dwNumBytes)
{
	// Write chunk to disk
	return (int)sf_write_short(	m_pSndFile, 
							(SHORT*)pbtDataPtr,
							(sf_count_t)( dwNumBytes / sizeof( SHORT ) ) * sizeof( SHORT ) );
}	


int CWAV::ReadDataChunk(BYTE* pBuf, DWORD dwNumBytes)
{
	ASSERT(m_pSndFile);
	return (int)sf_read_short(	m_pSndFile,
							(PSHORT)pBuf,
							dwNumBytes / sizeof( SHORT ) ) * sizeof( SHORT );
}


CDEX_ERR CWAV::OpenForRead(CUString& strFileName)
{
	// Clear m_wfInfo struct
	memset(&m_wfInfo, 0, sizeof(SF_INFO));

    CUStringConvert strCnv;

	// Open stream
    #ifdef _UNICODE
    if ( ! ( m_pSndFile = sf_open((const tchar*)strCnv.ToT( strFileName ),
									SFM_READ,
									&m_wfInfo ) ) )
    #else
    if ( ! ( m_pSndFile = sf_open(	strCnv.ToT( strFileName ),
									SFM_READ,
									&m_wfInfo ) ) )
    #endif
	{
		ASSERT( FALSE );
		return CDEX_ERROR;
	}
	
	m_dwSamplesToConvert = (DWORD)( m_wfInfo.frames * m_wfInfo.channels );
	m_dwDataSize = m_dwSamplesToConvert*sizeof(SHORT);
	m_wChannels = m_wfInfo.channels;

	switch ( m_wfInfo.format & SF_FORMAT_SUBMASK )
	{
		case SF_FORMAT_PCM_S8:
			m_wBitsPerSample = 8;
		break;
		case SF_FORMAT_PCM_16:
			m_wBitsPerSample = 16;
		break;
		case SF_FORMAT_PCM_24:
			m_wBitsPerSample = 24;
		break;
		case SF_FORMAT_PCM_32:
			m_wBitsPerSample = 32;
		break;
		default:
			m_wBitsPerSample = 16;
			ASSERT( FALSE );
	}

	m_wSampleRate = m_wfInfo.samplerate;
	
	// Return success
	return CDEX_OK;
}

#define AUDIO_FILE_NRMFILEEXT _W( ".nrm.wav" )

DWORD CWAV::StartNormalizeAudioFile(CUString strFileName)
{
	m_pTmpWav = new CWAV();
	
	if (CDEX_OK != OpenForRead( strFileName + _W( ".wav" ) ) )
		return 0;
	

	if (CDEX_OK != m_pTmpWav->OpenForWrite(strFileName + AUDIO_FILE_NRMFILEEXT,
		m_wfInfo.samplerate,
		m_wBitsPerSample,
		m_wfInfo.channels))
		return 0;
	
	// everything ok, return number of bytes to convert
	return m_dwDataSize;
}



BOOL CWAV::NormalizeAudioFileChunk(DOUBLE dNormFactor, int& nPerCentComplete)
{
	if (m_dwSamplesToConvert>0)
	{
		// Determine how many byte to convert this time
		DWORD dwBlockSize = min(m_dwSamplesToConvert, BLOCKSIZE);
		
		// Read Chunk
		dwBlockSize = ReadDataChunk((BYTE*)gpsSamples, dwBlockSize*sizeof(SHORT))/sizeof(SHORT);
		
		// Normalize chunk
		for (DWORD i = 0; i < dwBlockSize; i++)
		{
			if ((double)gpsSamples[i]*dNormFactor>32767)
			{
				LTRACE( _T( "Value out of range :%f\n" ), (double)gpsSamples[i]*dNormFactor);
				gpsSamples[i] = 32767;
			}
			else if ((double)gpsSamples[i]*dNormFactor < -32768)
			{
				LTRACE( _T( "Value out of range :%f\n" ), (double)gpsSamples[i]*dNormFactor);
				gpsSamples[i]=-32768;
			} else 
			{
				gpsSamples[i] =(short)((double)gpsSamples[i]*dNormFactor);
			}
		}	
		
		// And write the byte back to the file
		m_pTmpWav->SaveDataChunk((BYTE*)gpsSamples, dwBlockSize*sizeof(SHORT));
		
		// Decrement the number of bytes to convert
		m_dwSamplesToConvert -= dwBlockSize;
	}
	
	if (m_dwDataSize)
		nPerCentComplete =(int)(100 - 100.0*(DOUBLE)m_dwSamplesToConvert/((DOUBLE)m_dwDataSize/2));
	else
		nPerCentComplete = 0;
	
	if (m_dwSamplesToConvert>0)
		return FALSE;
	return TRUE;
}

void CWAV::CloseNormalizeAudioFile(CUString strFileName, BOOL bAbort, BOOL bReplace)
{
	// Close input stream
	CloseStream();
	
	// Close temp output stream
	m_pTmpWav->CloseStream();
	
	delete m_pTmpWav;
	m_pTmpWav = NULL;
	
	
	if ( bAbort )
	{
		// Delete temp file 
		if ( CDexDeleteFile(strFileName + AUDIO_FILE_NRMFILEEXT ) == 0 )
		{
		}
	}
	else
	{
		if ( bReplace )
		{
			// Delete un-normalized file name
			if ( CDexDeleteFile( strFileName + _W( ".wav" ) ) == 0 )
			{
			}
			
			// Rename output file
			if ( CDexMoveFile(	strFileName + AUDIO_FILE_NRMFILEEXT, 
							strFileName + _W( ".wav" ) ) ==0 )
			{
				CUString strMsg = GetLastErrorString();
				
				// Display the string.
				CDexMessageBox( strMsg, MB_OK | MB_ICONINFORMATION);
			}
		}
	}
}




BOOL CWAV::GetMaxWaveValue(INT& nPercent, INT& nPeakValue)
{
	if (m_dwSamplesToConvert>0)
	{
		// Determine how many byte to convert this time
		DWORD dwBlockSize = min(m_dwSamplesToConvert, BLOCKSIZE);
		
		// Read Chunk
		dwBlockSize = ReadDataChunk((BYTE*)gpsSamples, dwBlockSize*sizeof(SHORT))/sizeof(SHORT);
		
		// get file pionter position
		for (DWORD i = 0; i < dwBlockSize; i++)
		{
			nPeakValue= max(nPeakValue, abs(gpsSamples[i]));
		}	
		
		// Decrement the number of bytes to convert
		m_dwSamplesToConvert -= dwBlockSize;
	}
	
	if (m_dwDataSize)
		nPercent =(int)(100 - 100.0*(DOUBLE)m_dwSamplesToConvert/((DOUBLE)m_dwDataSize/2));
	else
		nPercent = 0;
	
	if (m_dwSamplesToConvert>0)
		return FALSE;
	return TRUE;
}


CDEX_ERR CWAV::SaveAtracTag( FILE* pFile, int nBitRate, int nSampleRate, WORD wChannels )
{
	SONYSCXWAVEFORMAT scxHeader;
	// Clear structure
	memset( &scxHeader, 0x00, sizeof( scxHeader ) );
	
	nBitRate = 132;

	scxHeader.wfx.wFormatTag = WAVE_FORMAT_SONY_SCX;		// Set format tag
	scxHeader.wfx.nChannels = wChannels;					// How many channels
	scxHeader.wfx.nSamplesPerSec = nSampleRate;				// The sample rate
	scxHeader.wfx.nAvgBytesPerSec = nBitRate * 1000 / 8;	// nBitrate in bytes per sec
//	scxHeader.wfx.nBlockAlign = 16 * wChannels / 8;
	scxHeader.wfx.nBlockAlign = 384;

	scxHeader.wfx.wBitsPerSample = 0;						// not used
	scxHeader.wfx.cbSize = sizeof(SONYSCXWAVEFORMAT) - sizeof(WAVEFORMATEX);	// Set number of extra bytes

	scxHeader.btExtraDontKnow[  0 ] = 0x01;
	scxHeader.btExtraDontKnow[  1 ] = 0x00;
	scxHeader.btExtraDontKnow[  2 ] = 0x00;
	scxHeader.btExtraDontKnow[  3 ] = 0x10;
	scxHeader.btExtraDontKnow[  4 ] = 0x00;
	scxHeader.btExtraDontKnow[  5 ] = 0x00;
	scxHeader.btExtraDontKnow[  6 ] = 0x00;
	scxHeader.btExtraDontKnow[  7 ] = 0x00;
	scxHeader.btExtraDontKnow[  8 ] = 0x00;
	scxHeader.btExtraDontKnow[  9 ] = 0x00;
	scxHeader.btExtraDontKnow[ 10 ] = 0x01;
	scxHeader.btExtraDontKnow[ 11 ] = 0x00;
	scxHeader.btExtraDontKnow[ 12 ] = 0x00;
	scxHeader.btExtraDontKnow[ 13 ] = 0x00;

	fwrite( &scxHeader, sizeof( scxHeader ), 1, pFile );

	return CDEX_OK;

}


CDEX_ERR CWAV::SaveMP2Tag( FILE* pFile, int nBitRate, int nSampleRate, WORD wChannels )
{
	MPEG1WAVEFORMAT mp2Header;
	
	// Clear structure
	memset( &mp2Header, 0x00, sizeof( mp2Header ) );
	
	mp2Header.wfx.wFormatTag = WAVE_FORMAT_MPEG;			// Set format tag
	mp2Header.wfx.nChannels = wChannels;					// How many channels
	mp2Header.wfx.nSamplesPerSec = nSampleRate;			// The sample rate
	mp2Header.wfx.nAvgBytesPerSec = nBitRate*1000/8;		// nBitrate in bytes per sec
	mp2Header.wfx.nBlockAlign = 1;							// assume varible bit rate
	mp2Header.wfx.wBitsPerSample = 0;						// not used
	mp2Header.wfx.cbSize = sizeof(MPEG1WAVEFORMAT) - sizeof(WAVEFORMATEX);	// Set number of extra bytes
	
	mp2Header.fwHeadLayer = ACM_MPEG_LAYER2;
	
	mp2Header.dwHeadBitrate = nBitRate*1000;				// Bits per second
	mp2Header.fwHeadMode = (wChannels == 2) ? 
					ACM_MPEG_STEREO:ACM_MPEG_SINGLECHANNEL;		// Set mode (MONO/JOINT/STEREO/DUAL)
	mp2Header.fwHeadModeExt = 0;							// Extra mode information for joint stereo
	mp2Header.wHeadEmphasis = 0;							// No emphasis
	mp2Header.fwHeadFlags = ACM_MPEG_ID_MPEG1;				// MPEG flags (optionally can set more information like private etc
	mp2Header.fwHeadFlags = 0x14;							// MPEG flags (optionally can set more information like private etc
	mp2Header.dwPTSLow = 0;								// Not assiciated with video stream
	mp2Header.dwPTSHigh = 0;								// Not assiciated with video stream

	fwrite( &mp2Header, sizeof( mp2Header ), 1, pFile );

	return CDEX_OK;
}


CDEX_ERR CWAV::SaveMP3Tag( FILE* pFile, int nBitRate, int nSampleRate, WORD wChannels )
{
	MPEGLAYER3WAVEFORMAT MP3Header;

	memset( &MP3Header, 0x00, sizeof( MP3Header ) );

	MP3Header.wfx.wFormatTag		= WAVE_FORMAT_MPEGLAYER3;		// Set format tag      
	MP3Header.wfx.nChannels			= wChannels;					// How many channels       
	MP3Header.wfx.nSamplesPerSec	= nSampleRate;					// The sample rate  
	MP3Header.wfx.nAvgBytesPerSec	= nBitRate * 1000 / 8;			// nBitrate in bytes per sec 
	MP3Header.wfx.nBlockAlign		= 1;							// assume varible bit rate     
	MP3Header.wfx.wBitsPerSample	= 0;							// not used
	MP3Header.wfx.cbSize			= MPEGLAYER3_WFX_EXTRA_BYTES;	// default 

	MP3Header.wID					= MPEGLAYER3_ID_MPEG;
	MP3Header.fdwFlags				= MPEGLAYER3_FLAG_PADDING_OFF;
	MP3Header.nBlockSize			= 0x01A1;
	MP3Header.nFramesPerBlock		= 0x0001;
	MP3Header.nCodecDelay			= 0x0571;

	switch (nBitRate)
	{
		case 18: 
			MP3Header.nFramesPerBlock = 0x0004;
			break;
		case 20: 
			MP3Header.nFramesPerBlock = 0x0002;
			break;
	}

	/* determined by experiment, no documentation found yet */
	double dProduct;
	double dCompression =(double)nSampleRate*16/((double)nBitRate*1000);

	switch (nSampleRate)
	{
		case 8000:	dProduct = 1152.00;
			break;
		case 11025:	dProduct = 1146.60;
			break;
		case 16000:	dProduct = 1152.00;
			break;
		case 22050:	dProduct = 1146.60;
			break;
		case 32000:	dProduct = 2304.00;
			break;
		case 44100:	dProduct = 2293.20;
			break;
		case 48000:	dProduct = 2293.20;
			break;
	}
	
	
	MP3Header.nBlockSize = (WORD)(dProduct/dCompression);

	fwrite( &MP3Header, sizeof( MP3Header ), 1, pFile );

	return CDEX_OK;
}


CDEX_ERR CWAV::ConvertToRiffWav( CUString strFileName, int nBitRate, int nSampleRate, WORD wChannels, int nLayer )
{
	
	
	
	DWORD	dwID3TagOffset = 0;
	CUString	strInFileName;
	
	if (0 == nLayer)
	{
		strInFileName = strFileName + _W( ".scx" );
	}
	else if ( 3 == nLayer )
	{
		strInFileName = strFileName + _W( ".mp3" );
	}
	else
	{
		strInFileName = strFileName + _W( ".mp2" );
	}
	
	// Check presense of ID3V2 tag
	// Get MP3 ID3V2 Tag offset
	CID3Tag	ID3Tag;
	
	ID3Tag.Init();
	ID3Tag.OpenFile(strInFileName);
	ID3Tag.LoadTagProperties();
	
	if (ID3Tag.IsV2Tag())  
	{
		dwID3TagOffset = ID3Tag.GetTagSize();
	}
	
	
	FILE* fpIn = NULL;
    
	fpIn = CDexOpenFile(  strInFileName, _W( "rb" ) );
	
	CUString strRiffWav( strFileName + _W( ".wav" ) );
	
	// And delete the WAV file first
	CDexDeleteFile( strFileName + _W( ".wav" ) );
	
	FILE* fpOut	= CDexOpenFile( strRiffWav, _W( "wb" ) );
	
	if ( NULL == fpIn || NULL == fpOut )
	{
		ASSERT(FALSE);
		return CDEX_ERROR;
	}
	
	// Get number of samples in file
	fseek( fpIn, 0L, SEEK_END );
	
	// Get current file position
	DWORD dwSamplesToConvert= ftell( fpIn );
	
	// Go back to the start of the file, but skip ID3V2 tag
	fseek( fpIn, 0, SEEK_SET );
	
	
	// Set this field to resulting files size - sizeof(CHUNKHEADER)
	// Thus total file size = 3*sizeof(CHUNCHEADER)+sizeof(MP3WAVEFORMAT)+ dwSamplesToConvert
	DWORD dwRiffSize;
	DWORD dwFmtSize;
	
	if ( 0 == nLayer )
	{
		dwFmtSize= sizeof( SONYSCXWAVEFORMAT );
	}
	else if ( 2 == nLayer )
	{
		dwFmtSize= sizeof( MPEG1WAVEFORMAT );
	}
	else
	{
		dwFmtSize= sizeof( MPEGLAYER3WAVEFORMAT );
	}
	
	dwRiffSize = dwSamplesToConvert + dwFmtSize + 32;	// 32 bytes for TAGS ( excluding RIFF WAV )
	
	// Write header to output file UNICODE
	fprintf( fpOut, "RIFF" );							// 4 bytes
	fwrite( &dwRiffSize, sizeof( DWORD ), 1, fpOut );	// 4 bytes
	fprintf( fpOut, "WAVE" );							// 4 bytes
	fprintf( fpOut, "fmt " );							// 4 bytes
	fwrite( &dwFmtSize, sizeof( DWORD ), 1, fpOut );	// 4 bytes
	
	
	if ( 0 == nLayer )
	{
		SaveAtracTag( fpOut, nBitRate, nSampleRate, wChannels );
	}
	else if (nLayer == 2)
	{
		SaveMP2Tag( fpOut, nBitRate, nSampleRate, wChannels );
	}
	else
	{
		SaveMP3Tag( fpOut, nBitRate, nSampleRate, wChannels );
	}
	
	DWORD dwTmp = 4;	/* total chunk size -4 bytes for fact, -4 bytes for length */
	fprintf(fpOut, "fact" );							// 4 bytes
	fwrite( &dwTmp, sizeof( DWORD ), 1, fpOut );		// 4 bytes
	
	if ( 3 == nLayer )
	{
		// Round to the number of frames
		dwTmp = dwSamplesToConvert/1152/2/wChannels;
		dwTmp *= 1152 * 2 * wChannels;
		
		
		// Estimate the amount of samples
		dwTmp *= ( (DWORD)nSampleRate * 2 * (DWORD)wChannels );
		dwTmp /= ( nBitRate * 1000 / 8 );
	}
	else
	{
		DWORD dwSampleBitRate = (DWORD)nSampleRate * (DWORD)wChannels * 16;
		double dCompressionFactor = (double)dwSampleBitRate / ((double)nBitRate * 1000.0 );
		
		// Round to the number of frames
		dwTmp = dwSamplesToConvert / 384;
		dwTmp *= 384;
		
		
		// Estimate the amount of samples
		// dwTmp*=((DWORD)nSampleRate*2*(DWORD)wChannels);
		// dwTmp/=(nBitRate*1000/8);
		dwTmp =(DWORD)( (double)dwTmp * dCompressionFactor );
		dwTmp /= 2;					/* from bytes to samples */
		dwTmp /=(DWORD)wChannels;	/* in samples per channel */
	}
	
	fwrite( &dwTmp, sizeof( DWORD ), 1, fpOut );
	
	// write data tag
	fprintf( fpOut, "data" );									// 4 bytes
	fwrite( &dwSamplesToConvert, sizeof( DWORD ), 1, fpOut );	// 4 bytes
	

	while ( dwSamplesToConvert > 0 )
	{
		// Determine how many byte to convert this time
		DWORD dwBlockSize = min( dwSamplesToConvert, BLOCKSIZE );
		
		// read chunck
		fread( gpsSamples, dwBlockSize, sizeof( BYTE ), fpIn );
		
		// And write the byte back to the file
		fwrite( gpsSamples, dwBlockSize, sizeof( BYTE ), fpOut );
		
		// Decrement the number of bytes to convert
		dwSamplesToConvert -= dwBlockSize;
	}
	
	// flush the files
	fflush( fpIn );
	fflush( fpOut );
	
	// And close the wave file
	fclose( fpIn );
	fclose( fpOut );
	
	return CDEX_OK;
}




CDEX_ERR CWAV::StripRiffWavHeader(CUString strFileName)
{
	FILE* fpIn	= CDexOpenFile( strFileName + _W( ".wav" ), _W( "rb" ) );

    if (fpIn == NULL)
	{
		CUString strLang;
		CUString strMsg;

		strLang = g_language.GetString( IDS_ERROR_COULDNOTOPENFILE );
		strMsg.Format( strLang, (LPCWSTR)( strFileName + _W( ".wav" ) ) );

        CDexMessageBox( strMsg );

		return CDEX_ERROR;
	}
	
	// Get file size;
	fseek(fpIn, 0, SEEK_END);
	int nFileSize = ftell( fpIn );
	
	
	// seek back to beginning of the file
	fseek(fpIn, 0, SEEK_SET);
	
	// read first 8 bytes
	fread( gpsSamples, 1, 8, fpIn );
	
	// check if this is a RIFF-WAVE file
	if (strstr((LPSTR)gpsSamples, "RIFF") == NULL)
	{
		CUString strLang;
		CUString strMsg;

		strLang = g_language.GetString( IDS_ERROR_NOTARIFFWAVFILE );

		strMsg.Format( strLang, (LPCWSTR)( strFileName + _W( ".wav" ) ) ); 

        CDexMessageBox( strMsg );

		fclose( fpIn );

		return CDEX_ERROR;
	}
	// read following 4 bytes
	fread(gpsSamples, 1, 4, fpIn);

	if ( NULL == strstr( (LPSTR)gpsSamples, "WAVE" ) )
	{
		CUString strLang;
		CUString strMsg;

		strLang = g_language.GetString( IDS_ERROR_NOTARIFFWAVFILE );

		strMsg.Format( strLang, (LPCWSTR)( strFileName + _W( ".wav" ) ) ); 

        CDexMessageBox( strMsg );

		fclose( fpIn );

		return CDEX_ERROR;
	}
	
	fread(gpsSamples, 1, 4 + 4, fpIn);
	
	if (strstr((LPSTR)gpsSamples, "fmt") == NULL)
	{
		CUString strLang;
		CUString strMsg;

		strLang = g_language.GetString( IDS_ERROR_COULDNOTFINDFORMATTAG );

		strMsg.Format( strLang, (LPCWSTR)( strFileName + _W( ".wav " ) ) ); 
        
        CDexMessageBox( strMsg );

		fclose( fpIn );

		return CDEX_ERROR;
	}
	
	WAVEFORMATEX wfx;
	
	// read WAVE Header
	fread(&wfx, sizeof(wfx), 1, fpIn);
	
	if (wfx.wFormatTag != WAVE_FORMAT_MPEGLAYER3 && wfx.wFormatTag != WAVE_FORMAT_MPEG)
	{
		CUString strLang;
		CUString strMsg;

		strLang = g_language.GetString( IDS_ERROR_NOTARIFFWAVFILE );

		strMsg.Format( strLang, (LPCWSTR)( strFileName + _W( ".wav" ) ) ); 

        CDexMessageBox( strMsg );

		fclose( fpIn );

		return CDEX_ERROR;
	}
	
	
	// seek data tag
	int i = 8;
	BOOL bFound = FALSE;
	
	while ((i < nFileSize) && (bFound == FALSE))
	{
		fseek(fpIn, i, SEEK_SET);
		fread(gpsSamples, 1, 4, fpIn);
		if ( NULL != strstr( (LPSTR)gpsSamples, "data" ) )
		{
			// Skip another 4 bytes for data value
			bFound = TRUE;
			fseek(fpIn, i + 8, SEEK_SET);
		}
		i++;
	}
	
	if (i>nFileSize - 4)
	{
		fclose(fpIn);
		CUString strLang;
		strLang = g_language.GetString( IDS_ERROR_DATATAGNOTFOUND );

        CDexMessageBox( strLang );

		return CDEX_ERROR;
	}
	
	// And delete the WAV file first
	CDexDeleteFile( strFileName + _W( ".mp3" ) );
	
	// Create output file
    FILE* fpOut	= CDexOpenFile( strFileName + _W( ".mp3" ), _W( "wb" ) );
	
	
	// How many samples to convert
	DWORD dwSamplesToConvert =(nFileSize - ftell(fpIn));
	
	while (dwSamplesToConvert>0)
	{
		// Determine how many byte to convert this time
		DWORD dwBlockSize = min(dwSamplesToConvert, BLOCKSIZE);
		
		// read chunck
		fread(gpsSamples, dwBlockSize, sizeof(BYTE), fpIn);
		
		// And write the byte back to the file
		fwrite(gpsSamples, dwBlockSize, sizeof(BYTE), fpOut);
		
		// Decrement the number of bytes to convert
		dwSamplesToConvert -= dwBlockSize;
	}
	
	// And close the wave file
	fclose(fpIn);
	fclose(fpOut);
	
	return CDEX_OK;
}
