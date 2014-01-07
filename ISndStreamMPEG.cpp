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
#include "Config.h"
#include "ISndStreamMPEG.h"
#include "ID3Tag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ENCDELAY 576
#define DECDELAY 528

#define MAXSIZE_MP3FRAME 12000

// CONSTRUCTOR
ISndStreamMPEG::ISndStreamMPEG()
	:ISndStream()
{
	SetFileExtention("mp3");
	SetStreamType( SNDSTREAM_MPEG );

	m_pFile=NULL;
	m_hDLL=NULL;
	m_pOpenStream=NULL;
	m_pDecode=NULL;
	m_pCloseStream=NULL;
	m_pVersion=NULL;
	m_hStream=NULL;
	m_pGetBitRate=NULL;
	m_dwTotalFileSize=0;
	m_dwCurrentFilePos=0;
	m_dwBytesToDo=0;
	m_dwSkipBytes=0;
}

// DESRUCTOR
ISndStreamMPEG::~ISndStreamMPEG()
{
	CloseStream();
}


BOOL ISndStreamMPEG::OpenStream(CString strFileName)
{
	DWORD	dwID3TagOffset = 0;
	INT		nXingTagOffset = 0;

	// Set buffer to hold maximum MP3 frame
	SetBufferSize( MAXSIZE_MP3FRAME );

	// Get the DLL functions
	m_hDLL=LoadLibrary("MpgLib.dll");

	// just for development purposes, try to load the DLL from the devo path
#ifdef _DEBUG
	if (m_hDLL==NULL)
		m_hDLL=LoadLibrary(g_config.GetAppPath()+"\\MpgLibDLL\\Debug\\MpgLib.dll");
#else
	if (m_hDLL==NULL)
		m_hDLL=LoadLibrary(g_config.GetAppPath()+"\\MpgLibDLL\\Release\\MpgLib.dll");
#endif
	if (m_hDLL==NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	//Get pointer to functions 
	m_pOpenStream	=(MPGLIB_OPENSTREAM)GetProcAddress(m_hDLL,TEXT_MPGLIB_OPENSTREAM);
	m_pDecode		=(MPGLIB_DECODECHUNK)GetProcAddress(m_hDLL,TEXT_MPGLIB_DECODECHUNK);
	m_pCloseStream	=(MPGLIB_CLOSESTREAM)GetProcAddress(m_hDLL,TEXT_MPGLIB_CLOSESTREAM);
	m_pVersion		=(MPGLIB_VERSION)GetProcAddress(m_hDLL,TEXT_MPGLIB_VERSION);
	m_pGetBitRate	=(MPGLIB_GETBITRATE)GetProcAddress(m_hDLL,TEXT_MPGLIB_GETBITRATE);
	m_pFlush		=(MPGLIB_GETBITRATE)GetProcAddress(m_hDLL,TEXT_MPGLIB_FLUSH);

	// Check if all functions are available
	if ( (m_pOpenStream==NULL) || (m_pDecode==NULL) || (m_pCloseStream==NULL)|| (m_pVersion==NULL))
	{
		ASSERT(FALSE);
		return FALSE;
	}

	// Register the file name
	SetFileName(strFileName);



	// Get MP3 ID3V2 Tag offset
	CID3Tag	ID3Tag;

	ID3Tag.Init();
	ID3Tag.OpenFile( GetFileName() );
	ID3Tag.LoadTag();


	if ( ID3Tag.IsV2Tag() )  
	{
		dwID3TagOffset = ID3Tag.GetTagSize();
	}


	// Open MPEG stream
	if (! (m_pFile = fopen( GetFileName(), "rb" ) ) )
	{
		ASSERT(FALSE);
		return FALSE;
	}

	m_dwCurrentFilePos = 0;


	if ( dwID3TagOffset )
	{
		fseek(m_pFile, dwID3TagOffset, SEEK_SET );
	}		

	fread( m_pbDataInBuffer, 1024, 1, m_pFile );

	// Clear header information
	m_Header.Init();

	// get Xing VBR header if available
	if ( m_Header.ReadHeader( m_pbDataInBuffer, 1024, &nXingTagOffset ) == FALSE )
	{
		return FALSE;
	}

	// Extract properties from header information

	SetSampleRate( m_Header.GetSampleRate() );
	SetBitRate( m_Header.GetSampleRate() );

	// Get file size
	fseek( m_pFile, 0, SEEK_END );
	m_dwTotalFileSize= ftell( m_pFile );

	// seek back to beginning of stream
	fseek( m_pFile, dwID3TagOffset + nXingTagOffset, SEEK_SET );

	m_dwBytesToDo=m_dwTotalFileSize-nXingTagOffset-dwID3TagOffset;

	// Initialize MPEG structure
	m_pOpenStream(&m_hStream);

	SetChannels(m_Header.GetChannels());

	m_dwSkipBytes=1105*sizeof(SHORT)*m_Header.GetChannels();

	// Return Success
	return TRUE;
}



DWORD ISndStreamMPEG::Read(PBYTE pbData,DWORD dwNumBytes)
{
	BOOL bEos=FALSE;

	static int nFrame=0;

	// check if file pointer is valid
	ASSERT(m_pFile);

	// check size of output buffer, must be larger than largest frame
	ASSERT(dwNumBytes>=4096);

	int nErrorCode=!MPGLIB_OK;
	int	nDataOutBuffer=0;



//	while ( (nErrorCode!=MPGLIB_OK) && (m_nDataInBuffer>0))
//	while ( (m_nDataInBuffer>0) && (nDataOutBuffer==0))
	// Try to decode a block, until were not at the end of the input stream
	// end till we have a decoded frame
	while ( (bEos==FALSE) && (nDataOutBuffer==0) )
	{

		// Try to decode the data
		nErrorCode= m_pDecode(	m_hStream,
								NULL,
								0,
								pbData,
								dwNumBytes,
								&nDataOutBuffer);

		if ( nErrorCode==MPGLIB_NEED_MORE )
		{
			DWORD dwRead= (m_dwBytesToDo>MPEG_WAV_BUFFERSIZE)?MPEG_WAV_BUFFERSIZE:m_dwBytesToDo;

			// Read MPEG data from disk
			dwRead=fread(m_pbDataInBuffer,1,dwRead,m_pFile);

			m_dwCurrentFilePos+= dwRead;

			if (dwRead!=0)
			{
				m_dwBytesToDo-=dwRead;

//				CString strTmp;
//				strTmp.Format("******* Read %d bytes at frame %d\n",dwRead,nFrame);
//				OutputDebugString(strTmp);

				// Fed new data to Decode library, and decode a frame
				nErrorCode = m_pDecode(m_hStream,
										m_pbDataInBuffer,
										dwRead,
										pbData,
										dwNumBytes,
										&nDataOutBuffer);
			}
			else
			{
				// Were stuck, no more data, and the decoder needs more
				bEos=TRUE;
			}

			// Get bitrate of this frame 
			SetBitRate(m_pGetBitRate());

		}

		if ( m_dwTotalFileSize )
		{
			SetPercent( (INT)( (double) m_dwCurrentFilePos * 100.0 / m_dwTotalFileSize ) );
		}
		else
		{
			SetPercent( 0 );
		}

	}

	nFrame++;
	return nDataOutBuffer;
}


BOOL ISndStreamMPEG::CloseStream()
{
	if (m_pFile)
	{
		m_pCloseStream(m_hStream);
		fclose(m_pFile);
	}
	if (m_hDLL)
	{
		FreeLibrary(m_hDLL);
	}
	m_hDLL=NULL;
	m_pFile=0;
	return TRUE;
}


// FIXME
DWORD ISndStreamMPEG::GetTotalTime()
{
	DOUBLE	dFilePos=0.0;
	int		nSamplesPerFrame;
	DWORD	dwTotalTime;

	switch(m_Header.GetLayer())
	{
		// FIXME
		case 1:nSamplesPerFrame=1152;break;
		case 2:nSamplesPerFrame=1152;break;
		case 3:nSamplesPerFrame=1152;break;
		default: 
			ASSERT(FALSE);
			return 0;
	}

	if ( m_Header.IsMPEG_II() )
		nSamplesPerFrame/=2;


	if ( m_dwTotalFileSize )
		dFilePos=(DOUBLE)ftell(m_pFile)/(DOUBLE)m_dwTotalFileSize;

	// total file length in ms
	if (m_Header.IsXingHeader())
	{
		
		dwTotalTime=m_Header.GetNumFrames()*nSamplesPerFrame/GetSampleRate()*1000;
	}
	else
	{
		if (m_Header.GetFrameSize())
		{
			int nFrames=m_dwTotalFileSize/m_Header.GetFrameSize();

			dwTotalTime=nFrames*nSamplesPerFrame/GetSampleRate()*1000;
		}
	}

	return dwTotalTime;
}

DWORD ISndStreamMPEG::GetCurrentTime()
{
	DOUBLE	dPercent=0.0;

	DOUBLE	dRelFilePos=0.0;
	int		nSamplesPerFrame;
	DWORD	dwTime;


	if (m_dwTotalFileSize)
		dRelFilePos=(DOUBLE)ftell(m_pFile)/(DOUBLE)m_dwTotalFileSize;

	switch(m_Header.GetLayer())
	{
		// FIXME
		case 1:nSamplesPerFrame=1152;break;
		case 2:nSamplesPerFrame=1152;break;
		case 3:nSamplesPerFrame=1152;break;
		default: 
			ASSERT(FALSE);
			return 0;
	}

	if ( m_Header.IsMPEG_II() )
		nSamplesPerFrame/=2;
	
	// total file length in ms
	if (m_Header.IsXingHeader())
	{
		// loop through TOC
		for (int i=0;i<100;i++)
		{
			double dPosition=dRelFilePos*255.0;

			if ((DOUBLE)m_Header.GetToc(i) > dPosition)
			{
				double dA;
				double dB;
				double dPercent;
				double dDelta=(DOUBLE)m_Header.GetToc(i)-dPosition;
				
				if (i>0)
				{
					dB=m_Header.GetToc(i);
					dA=m_Header.GetToc(i-1);
				}
				else
				{
					dB=m_Header.GetToc(i);
					dA=0.0;
				}
				if ( (dB-dA)>0)
				{
					dDelta/=(dB-dA);
					dPercent=i+(1.0-dDelta);
				}
				else
				{
					dPercent=i;
				}

				dwTime=	(DWORD)(
						(double) m_Header.GetNumFrames()*
						(double) dPercent/100*
						(double) nSamplesPerFrame /
						(double) GetSampleRate()*1000.0);
				return dwTime;
			}
		}
	}
	else
	{
		if (m_Header.GetFrameSize())
		{
			int nFrames=nFrames=ftell(m_pFile)/m_Header.GetFrameSize();
			dwTime=(DWORD)( (double)nFrames * (double)nSamplesPerFrame / (double)GetSampleRate() * 1000.0);
		}

	}

	return dwTime;
}


LONG ISndStreamMPEG::Seek( LONG lOff, UINT nFrom )
{
	double dFilePosition=0.0;
	int nPercent=lOff;

	if (nPercent<0 ) nPercent= 0;
	if (nPercent>99) nPercent=99;

	// Flush the stream
	Flush();

	if (nFrom==SEEK_PERCENT)
	{
		if (m_Header.IsXingHeader())
		{

			dFilePosition=(double)m_Header.GetToc(nPercent)*(double)m_dwTotalFileSize/255.0;
		}
		else
		{
			dFilePosition=(double)nPercent*(double)m_dwTotalFileSize/100.0;
		}
	}

	// seek file pointer
	fseek(m_pFile,(LONG)dFilePosition,SEEK_SET);

	return (LONG)dFilePosition;
}

void ISndStreamMPEG::Flush()
{
	m_pFlush();
}

void ISndStreamMPEG::InfoBox( HWND hWnd )
{
}