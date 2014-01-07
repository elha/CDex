/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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
#include "Codec.h"
#include <math.h>
#include <msacm.h>
 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static SHORT			pMyBuffer[2][1152];
static int gs_nDebug=0;


// CONSTRUCTOR
CAcmFormat::CAcmFormat()
{
	ENTRY_TRACE( _T( "CAcmFormat::CAcmFormat()" ) );
	cbStruct=0;
	dwFormatIndex=0; 
	dwFormatTag=0;
	fdwSupport=0; 
	pwfx=NULL;
	cbwfx=0; 
	memset(szFormat,0x00,sizeof(szFormat));
	EXIT_TRACE( _T( "CAcmFormat::CAcmFormat()" ) );
}

// DESTRUCTOR
CAcmFormat::~CAcmFormat()
{
	ENTRY_TRACE( _T( "CAcmFormat::~CAcmFormat()" ) );
	delete [] pwfx;
	EXIT_TRACE( _T( "CAcmFormat::~CAcmFormat()" ) );
}

// ASSIGNMENT OPERATOR
CAcmFormat& CAcmFormat::operator=(const CAcmFormat& rhs)
{
	ENTRY_TRACE( _T( "CAcmFormat::operator=" ) );

	// Avoid self assignment
	if (this!= &rhs)
	{
		// Deltete old pwfx
		delete [] pwfx;pwfx=NULL;

		cbStruct=		rhs.cbStruct;
		dwFormatIndex=	rhs.dwFormatIndex; 
		dwFormatTag=	rhs.dwFormatTag;
		fdwSupport=		rhs.fdwSupport; 
		cbwfx=			rhs.cbwfx;

		if (rhs.pwfx!=NULL)
		{
			// Allocate space for pwfx
			pwfx=(LPWAVEFORMATEX) new BYTE[cbwfx];

			// Assign it
			memcpy(pwfx,rhs.pwfx,cbwfx);
		}

		_tcscpy( szFormat, rhs.szFormat );
	}
	EXIT_TRACE( _T( "CAcmFormat::operator=" ) );
	return *this;
}

// COPY CONSTRUCTOR
CAcmFormat::CAcmFormat(const CAcmFormat& rhs)
{
	ENTRY_TRACE( _T( "CAcmFormat::CAcmFormat(const CAcmFormat& rhs)" ) );

	cbStruct=		rhs.cbStruct;
	dwFormatIndex=	rhs.dwFormatIndex; 
	dwFormatTag=	rhs.dwFormatTag;
	fdwSupport=		rhs.fdwSupport; 
	cbwfx=			rhs.cbwfx;

	if (rhs.pwfx!=NULL)
	{
		// Allocate space for pwfx
		pwfx=(LPWAVEFORMATEX) new BYTE[cbwfx];

		// Assign it
		memcpy(pwfx,rhs.pwfx,cbwfx);
	}

	_tcscpy( szFormat, rhs.szFormat );
	EXIT_TRACE( _T( "CAcmFormat::CAcmFormat(const CAcmFormat& rhs)" ) );
}


CAcmFormat::CAcmFormat(const ACMFORMATDETAILS& rhs)
{
	ENTRY_TRACE( _T( "CAcmFormat::CAcmFormat(const ACMFORMATDETAILS& rhs)" ) );
	cbStruct=		rhs.cbStruct;
	dwFormatIndex=	rhs.dwFormatIndex; 
	dwFormatTag=	rhs.dwFormatTag;
	fdwSupport=		rhs.fdwSupport; 
	cbwfx=			rhs.cbwfx; 

	if (rhs.pwfx!=NULL)
	{
		// Allocate space for pwfx
		pwfx=(LPWAVEFORMATEX) new BYTE[cbwfx];

		// Assign it
		memcpy(pwfx,rhs.pwfx,cbwfx);
	}

	_tcscpy( szFormat, rhs.szFormat );
	EXIT_TRACE( _T( "CAcmFormat::CAcmFormat(const ACMFORMATDETAILS& rhs)" ) );
}

// CONSTRUCTOR
CAcmDriverInfo::CAcmDriverInfo()
{
	ENTRY_TRACE( _T( "CAcmDriverInfo::CAcmDriverInfo()" ) );
	hadid=NULL;
	memset(&dd,0x00,sizeof(ACMDRIVERDETAILS));
	EXIT_TRACE( _T( "CAcmDriverInfo::CAcmDriverInfo()" ) );
}

// DESTRUCTOR
CAcmDriverInfo::~CAcmDriverInfo()
{
	LTRACE2( _T( "CAcmDriverInfo::~CAcmDriverInfo() %s" ), dd.szLongName );
}

// ASSIGNMENT OPERATOR
CAcmDriverInfo& CAcmDriverInfo::operator=(const CAcmDriverInfo& rhs)
{
	ENTRY_TRACE( _T( "CAcmDriverInfo::operator=" ) );
	// Avoid self assignment
	if (this!= &rhs)
	{
		hadid=rhs.hadid;
		dd=rhs.dd;
		m_vFormatDetails=rhs.m_vFormatDetails;
	}
	EXIT_TRACE( _T( "CAcmDriverInfo::operator=" ) );
	return *this;
}

// COPY CONSTRUCTOR
CAcmDriverInfo::CAcmDriverInfo(const 
                               CAcmDriverInfo& rhs)
{
	ENTRY_TRACE( _T( "CAcmDriverInfo::CAcmDriverInfo(const CAcmDriverInfo& rhs)" ) );
	hadid=rhs.hadid;
	dd=rhs.dd;
	m_vFormatDetails=rhs.m_vFormatDetails;
	EXIT_TRACE( _T( "CAcmDriverInfo::CAcmDriverInfo(const CAcmDriverInfo& rhs)" ) );
}


// CONSTRUCTOR
CAcmCodec::CAcmCodec()
{
	ENTRY_TRACE( _T( "CAcmCodec::CAcmCodec()" ) );

	m_bIntermPCM=FALSE;
	m_pbtSrcBuffer = NULL;
	m_pbtPcmBuffer = NULL;
	m_pbtDstBuffer = NULL;

	m_pPcmAcmDriver = NULL;
	m_pDstAcmDriver = NULL;

	m_pPcmAcmFormat = NULL;
	m_pDstAcmFormat = NULL;

	m_bMP3CodecInstalled = FALSE;

	m_nOffset = 0;
	m_nBufferSize = 0;

	EXIT_TRACE( _T( "CAcmCodec::CAcmCodec()" ) );
}


void CAcmCodec::ReInit()
{
	ENTRY_TRACE( _T( "CAcmCodec::ReInit()" ) );
	delete [] m_pbtSrcBuffer;
	delete [] m_pbtPcmBuffer;
	delete [] m_pbtDstBuffer;

	m_pbtSrcBuffer=NULL;
	m_pbtPcmBuffer=NULL;

	m_pbtDstBuffer=NULL;
	m_pPcmAcmDriver=NULL;
	m_pDstAcmDriver=NULL;

	m_pPcmAcmFormat=NULL;
	m_pDstAcmFormat=NULL;

	m_hPcmStream=NULL;
	m_hDstStream=NULL;

	m_hPcmDriver=NULL;
	m_hDstDriver=NULL;

	memset(&m_DstStreamHdr, 0, sizeof(m_DstStreamHdr));
	memset(&m_PcmStreamHdr, 0, sizeof(m_PcmStreamHdr));
	EXIT_TRACE( _T( "CAcmCodec::ReInit()" ) );
}


// DESRUCTOR
CAcmCodec::~CAcmCodec()
{
	ENTRY_TRACE( _T( "CAcmCodec::~CAcmCodec()" ) );
	delete [] m_pbtSrcBuffer;
	delete [] m_pbtPcmBuffer;
	delete [] m_pbtDstBuffer;
	EXIT_TRACE( _T( "CAcmCodec::~CAcmCodec()" ) );
}

int CAcmCodec::GetNumDetails(int nCodecIndex) const
{
	return m_vAcmDriverInfo[nCodecIndex].m_vFormatDetails.size();
}

CUString CAcmCodec::GetFormatString(int nCodecIndex,int nFormatIndex) const
{
	return m_vAcmDriverInfo[ nCodecIndex ].m_vFormatDetails[ nFormatIndex ].szFormat;
}


CUString CAcmCodec::GetFormatString(int nCodecIndex,int nFormatIndex,DWORD dwTag) const
{
	if (m_vAcmDriverInfo[nCodecIndex].m_vFormatDetails[nFormatIndex].dwFormatTag==dwTag)
	{
		return m_vAcmDriverInfo[nCodecIndex].m_vFormatDetails[nFormatIndex].szFormat;
	}
	else
		return _W( "" );
}


// Only get the Codec that support type dwTag
// Return empty string when TAG ID is not supported
CUString	CAcmCodec::GetACMCodecName(int nCodecIndex,DWORD dwTag)
{
	ENTRY_TRACE( _T( "CAcmCodec::GetACMCodecName()" ) );

	// Sanity check
	ASSERT((DWORD)nCodecIndex<m_vAcmDriverInfo.size());

	BOOL bTagSupported=FALSE;

	// Loop through formats
	for (DWORD i=0;i<m_vAcmDriverInfo[nCodecIndex].m_vFormatDetails.size();i++)
	{
		// Check format tag out of ACMFORMATDETAILS
		if (m_vAcmDriverInfo[nCodecIndex].m_vFormatDetails[i].dwFormatTag==dwTag)
			bTagSupported=TRUE;
	}

	if (bTagSupported)
	{
		EXIT_TRACE( _T( "CAcmCodec::GetACMCodecName() OK" ) );
		// Return long name of the nCodecIndex
		return CUString(m_vAcmDriverInfo[nCodecIndex].dd.szLongName);
	}
	else
	{
		EXIT_TRACE( _T( "CAcmCodec::GetACMCodecName() EMPTY") );
		return _W( "" );
	}
}

void CAcmCodec::ShowError(MMRESULT& mmr)
{
	ASSERT(FALSE);
	LTRACE( _T( "++++++++++++++++++++++ Codec Error +++++++++++++" ) );
}



void CAcmCodec::DisplayCodecInfo(int nCodecIndex) const
{
	ENTRY_TRACE( _T( "CAcmCodec::DisplayCodecInfo()" ) );

	ACMDRIVERDETAILS dd=m_vAcmDriverInfo[nCodecIndex].dd;

	CUString strTmp;
	CUString strOut;

	strOut.Format( _W( "Codec details for driver index %d" ), nCodecIndex );
	strOut+= _W( "\r\n-------------------------------------------------\r\n" );	
	strOut+= _W( "\r\nDriver Details information\r\n" );	
	strTmp.Format( _W( "   Short name: %s\r\n" ), (LPCWSTR)CUString( dd.szShortName )); 
    strOut+=strTmp;
	strTmp.Format( _W( "   Long name:  %s\r\n" ), (LPCWSTR)CUString( dd.szLongName ));
    strOut+=strTmp;
	strTmp.Format( _W( "   Copyright:  %s\r\n" ), (LPCWSTR)CUString( dd.szCopyright ));
    strOut+=strTmp;
	strTmp.Format( _W( "   Licensing:  %s\r\n" ), (LPCWSTR)CUString( dd.szLicensing ));
    strOut+=strTmp;
	strTmp.Format( _W( "   Features:   %s\r\n" ), (LPCWSTR)CUString( dd.szFeatures));
    strOut+=strTmp;
	strTmp.Format( _W( "   Supports %u formats\r\n" ), dd.cFormatTags );
    strOut+=strTmp;
	strTmp.Format( _W( "   Supports %u filter formats\r\n" ), dd.cFilterTags );
    strOut+=strTmp;

	CDexMessageBox( strOut );
	EXIT_TRACE( _T( "CAcmCodec::DisplayCodecInfo()" ) );
}

// Get the ACM version.
void CAcmCodec::GetVersion()
{
	ENTRY_TRACE( _T( "CAcmCodec::GetVersion()" ) );
	CUString strDebug;

    DWORD dwACMVer = acmGetVersion();

	LTRACE( _T( "ACM version %u.%.02u build %u" ), HIWORD( dwACMVer ) >> 8, HIWORD( dwACMVer ) & 0x00FF, LOWORD( dwACMVer ) );
    if (LOWORD(dwACMVer) == 0) 
		LTRACE( _T( "Retail Version" ) );

	EXIT_TRACE( _T( "CAcmCodec::GetVersion()" ));
}


BOOL CALLBACK CAcmCodec::FormatEnumProc(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport)
{
	ENTRY_TRACE( _T( "CAcmCodec::FormatEnumProc()" ) );

	// Get a pointer to the CAcmCodec caller object
	CAcmCodec* pAcmCodec=(CAcmCodec*)dwInstance;

	if (pafd)
	{
		CAcmFormat newFormat(*pafd);

		LTRACE( _T( "New codec enumeration:    %4.4lXH, %s" ), pafd->dwFormatTag, pafd->szFormat);

		if (pAcmCodec->m_vAcmDriverInfo.size() >0 )
		{
//			if (pafd->pwfx->wFormatTag==0x55)
			if (pafd->pwfx->wFormatTag== 0x270  || pafd->pwfx->wFormatTag== 0x55)
			{
				pAcmCodec->m_bMP3CodecInstalled=TRUE;
				// Push pafd to format array
				pAcmCodec->m_vAcmDriverInfo.back().m_vFormatDetails.push_back(newFormat);
				LTRACE( _T( "Add encoder format to list" ) );
			}
		}
	}
	EXIT_TRACE( _T( "CAcmCodec::FormatEnumProc()") );

	return TRUE; // Continue enumerating.
}
/*
void EnumerateFormats(void) {
	int i;
	MMRESULT retval;

	for (i = 0;i < NumModes;i++) if (ModeList[i].wfx) free(ModeList[i].wfx);
	if(hidMP3) {
		//Open Driver
		retval = acmDriverOpen( &hMP3,hidMP3,0);
    	if (retval) BadExit(retval);
		memset(&fd, 0, sizeof(fd));
		memset(&wf,0,sizeof(wf));
    	fd.cbStruct = sizeof(fd);
    	//fd.dwFormatTag = WAVE_FORMAT_UNKNOWN;
		fd.pwfx = &wf;
    	retval = acmMetrics((HACMOBJ)hMP3, ACM_METRIC_MAX_SIZE_FORMAT, &fd.cbwfx);
		fd.pwfx->nChannels=2;
		fd.pwfx->nSamplesPerSec = 44100;
		fd.pwfx->wBitsPerSample = 16;
		fd.pwfx->wFormatTag = WAVE_FORMAT_PCM;
		fd.pwfx->nBlockAlign = 2*16/8;
		fd.pwfx->nAvgBytesPerSec =  44100 * (2*16/8);
    	NumModes = 0;
		retval = acmFormatEnum(hMP3,&fd,EnumFormats,0L,ACM_FORMATENUMF_CONVERT);
		if(hMP3) acmDriverClose(hMP3,0);
        if (retval) {
			printf("No information available\n");
    		BadExit(retval);
    	}
	}
}

*/

// acmDriverEnumCallback function
BOOL CALLBACK CAcmCodec::DriverEnumProc(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport)
{   
	MMRESULT		mmr;
	CAcmDriverInfo	newDriverInfo;
	HACMDRIVER		had = NULL;
	CUString			strDebug;

	ENTRY_TRACE( _T( "CAcmCodec::DriverEnumProc()") );

	// Get a pointer to the CAcmCodec caller object
	CAcmCodec* pAcmCodec=(CAcmCodec*)dwInstance;

	// Set hadid of current Codec enumeration
	newDriverInfo.hadid=hadid;

	// Clear detail structure.    
	memset(&newDriverInfo.dd,0x00,sizeof(ACMDRIVERDETAILS));

	// Set structure size of ACMDRIVERDETAILS structure
	newDriverInfo.dd.cbStruct = sizeof(ACMDRIVERDETAILS);

	// Get the details of this driver
	mmr = acmDriverDetails(hadid, &newDriverInfo.dd, 0);    
	if (mmr) 
	{
		pAcmCodec->ShowError(mmr);    
		return TRUE;
	} 

	// Skip buggy codecs
	if ( _tcsstr(newDriverInfo.dd.szLongName, _T( "Delrina" ) ) !=NULL 
		|| _tcsstr(newDriverInfo.dd.szLongName, _T( "VoxWare" ) ) !=NULL 
		|| _tcsstr(newDriverInfo.dd.szLongName, _T( "Ulead MPEG1 Layer2" ) ) !=NULL 
		)
	{
		return TRUE;
	}

	// Open the driver.    
	mmr = acmDriverOpen(&had,hadid, 0);    
	if (mmr) 
	{
		pAcmCodec->ShowError(mmr);    
		return TRUE;
	} 
	
	strDebug.Format( _W( "%s id: %8.8lxH supports: "), (LPCWSTR)CUString( newDriverInfo.dd.szLongName ), hadid );
    if (fdwSupport & ACMDRIVERDETAILS_SUPPORTF_ASYNC) 
    {
        strDebug+= _W( " async conversions ");
    }
    if (fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CODEC)
    {
        strDebug+= _W( " different format conversions" );
    }
    if (fdwSupport & ACMDRIVERDETAILS_SUPPORTF_CONVERTER)
    {
        strDebug += _W( " same format conversions" );
    }
    if (fdwSupport & ACMDRIVERDETAILS_SUPPORTF_FILTER) 
    {
        strDebug+= _W( " filtering" );
    }
    CUStringConvert strCnv;  

	LTRACE( strCnv.ToT( strDebug ) );

	DWORD dwSize = 0;

	// Add newDriverInfo to array
	pAcmCodec->m_vAcmDriverInfo.push_back(newDriverInfo);

	// Get metrics of the maximum WAVEFORMATEX structure size 
	mmr = acmMetrics((HACMOBJ)had, ACM_METRIC_MAX_SIZE_FORMAT, &dwSize);
	
	// Correct size in case sizeof(WAVEFORMAT) is returned instead of 
	// sizeof(WAVEFORMATEX), can happen when only MS-PCM codecs are installed
	if (dwSize < sizeof(WAVEFORMATEX)) 
			dwSize = sizeof(WAVEFORMATEX);

	// Allocate memory for temp wave format struct
	WAVEFORMATEX* pwf = (WAVEFORMATEX*) new BYTE[dwSize];
	
	// clear allocated memory of pwf structure
	memset(pwf,0x00,dwSize);

	// Set WAVEFORMATEX fields
	pwf->wFormatTag			= WAVE_FORMAT_PCM;	// Search for PCM input format
	pwf->nChannels			= 2;				// Stereo
	pwf->nSamplesPerSec		= 44100;			// 44100 Hz
	pwf->wBitsPerSample		= 16;				// 16 bits/sample
	pwf->nBlockAlign		= pwf->nChannels * pwf->wBitsPerSample / 8; // set alignment
	pwf->nAvgBytesPerSec	= pwf->nSamplesPerSec * pwf->nBlockAlign;
	pwf->cbSize				= (WORD)( dwSize-sizeof(WAVEFORMATEX) );
	
	// Create fd struct
	ACMFORMATDETAILS fd;

	// clear contents of fd struct
	memset(&fd, 0, sizeof(fd));        

	// Set structure size
	fd.cbStruct = sizeof(fd);

	// Set WAVEFORMAT related parameters
	fd.pwfx = pwf;
	fd.cbwfx = dwSize;

	// Add some debug info
	LTRACE2( _T( "Start FormatEnumProc" ) );

	// Enumerate the available formats for this particular driver
	mmr = acmFormatEnum(had, &fd, FormatEnumProc,dwInstance,ACM_FORMATENUMF_CONVERT);

	if (mmr) 
	{
		pAcmCodec->ShowError(mmr);        
		return TRUE;
	}

	// And close driver
	mmr=acmDriverClose(had, 0);    
	if (mmr) 
	{
		pAcmCodec->ShowError(mmr);    
		return TRUE;
	} 

	// Delete pwf struct
	delete [] pwf;  

	EXIT_TRACE( _T( "CAcmCodec::DriverEnumProc()" ) );

	// Continue enumeration
	return TRUE;
}


int CAcmCodec::GetNumEncoders(DWORD dwTag)
{
	return 0;
}


void CAcmCodec::EnumDrivers()
{
    CUStringConvert strCnv; 

	// Do we have to debug this stuff?
	gs_nDebug= GetPrivateProfileInt( _T( "Debug" ), _T( "DebugCodec" ) , gs_nDebug, strCnv.ToT( g_config.GetIniFileName() ) );

	ENTRY_TRACE( _T( "CAcmCodec::EnumDrivers()" ) );

	// Clear all AcmDriverInfo entries
	m_vAcmDriverInfo.clear();

	// Get the ACM driver version
	GetVersion();

#ifdef VERBOSE
    // Enumerate the set of enabled drivers.
    LATRACE2("Enabled drivers:\n");
#endif

	MMRESULT mmr;
	// Enumerate all Codecs
    mmr = acmDriverEnum(DriverEnumProc,(DWORD)this,ACM_DRIVERENUMF_NOLOCAL); 

	// Was there an error
    if (mmr) 
		ShowError(mmr);
//	GetNumEncoders(0x55);
	EXIT_TRACE( _T( "CAcmCodec::EnumDrivers()" ) );
}



// 1152 frames, two channels, and two bytes per sample
const int nMaxChunkSize = 1152 * 2 * 2 * 2;

CDEX_ERR CAcmCodec::InitStream( CUString&	strCodec,
								CUString&	strFormat,
								INT			nCodecID,
								int			nSampleFreq,
								int			nChannels,
								BOOL		bHighQuality )
{
	BOOL bFound;
	CUString strTmp;

	ENTRY_TRACE( _T( "CAcmCodec::InitStream()" ) );

	m_bIntermPCM=FALSE;

	WAVEFORMATEX wfPCM;
	memset(&wfPCM, 0, sizeof(wfPCM));

	bFound=FALSE;

	// For safety reasons, when DeInitStream has not been called
	ReInit();

	int nChunkSize = nMaxChunkSize;

	if ( nCodecID == 0x270 )
	{
//		nChunkSize = 3072;
	}

	// Find proper codec

	int nCodecIndex=0;

	for (nCodecIndex=0;nCodecIndex<GetNumCodecs();nCodecIndex++)
	{
		CUString strName(GetACMCodecName(nCodecIndex,nCodecID));

		if (strName.Find(strCodec)>=0)
		{
			bFound=TRUE;
			break;
		}
	}

	if (bFound==FALSE)
		return CDEX_ERROR;


	bFound=FALSE;

	int nFormat=0;

	for (nFormat=0;nFormat<GetNumDetails(nCodecIndex);nFormat++)
	{
		CUString strFmt=GetFormatString(nCodecIndex,nFormat,nCodecID);
//		CUString strFmt=GetFormatString( nCodecIndex, nFormat );
		if (strFmt.Find(strFormat)>=0)
		{
			bFound=TRUE;
			break;
		}
	}

	if (bFound==FALSE)
	{
		LTRACE( _T( "CAcmCodec::InitStream() Coul not find target format") );
		return CDEX_ERROR;
	}

	// Set pointer to MP3 driver
	m_pDstAcmDriver=&m_vAcmDriverInfo[nCodecIndex];
	// Set pointer to MP3 format
	m_pDstAcmFormat=&m_pDstAcmDriver->m_vFormatDetails[nFormat];

	// Setup WAVEFORMATEX source structure
	WAVEFORMATEX wfSrc;
	memset(&wfSrc, 0, sizeof(wfSrc));
	wfSrc.cbSize = 0;
	wfSrc.wFormatTag = WAVE_FORMAT_PCM;		// PCM
	wfSrc.nChannels = nChannels;			// Stereo or mono
	wfSrc.nSamplesPerSec = nSampleFreq;		// Set Sample frequency
	wfSrc.wBitsPerSample = 16;				// 16 bits/sample
	wfSrc.nBlockAlign = wfSrc.nChannels * wfSrc.wBitsPerSample / 8;
	wfSrc.nAvgBytesPerSec = wfSrc.nSamplesPerSec * wfSrc.nBlockAlign;

//	m_dwSrcSamples = nMaxChunkSize / sizeof( SHORT );
//	m_dwDstSamples = nMaxChunkSize / sizeof( SHORT );

	m_pbtSrcBuffer= new BYTE[ 2 * nMaxChunkSize ];
	m_pbtDstBuffer= new BYTE[ nMaxChunkSize ];

	m_nOffset = 0;
	m_nBufferSize = 0;

	// Open the driver.    
	HACMDRIVER had = NULL;
	MMRESULT mmr = acmDriverOpen(&m_hDstDriver, m_pDstAcmDriver->hadid, 0);    
	
	// Get a pointer to the destination WAVEFORMATEX structure
	LPWAVEFORMATEX pwfDst=m_pDstAcmFormat->pwfx;

	// Try to open the stream
	mmr = acmStreamOpen(&m_hDstStream,
						m_hDstDriver,		// The conversion ACM driver
						&wfSrc,		// Source format
						pwfDst,		// Destination format
						NULL,		// No filter
						NULL,		// No callback
						NULL,		// Instance data (not used)
						(bHighQuality == TRUE) ? ACM_STREAMOPENF_NONREALTIME : 0 );
	if (mmr)
	{
		LTRACE( _T( "CAcmCodec::InitStream() Failed to open conversion stream") );
		CDexMessageBox( _W( "Failed to open a stream to do PCM to MP3 conversion" ) );
		return CDEX_ERROR;
	}


	// Set struct size
	m_DstStreamHdr.cbStruct = sizeof( m_DstStreamHdr );

	//Set source pointer
	m_DstStreamHdr.pbSrc = m_pbtSrcBuffer;
	m_DstStreamHdr.cbSrcLength = nChunkSize;
	m_DstStreamHdr.dwSrcUser = nChunkSize;

	// Set destination pointer
	m_DstStreamHdr.pbDst = m_pbtDstBuffer;
	m_DstStreamHdr.cbDstLength = nChunkSize;
	m_DstStreamHdr.dwDstUser = nChunkSize;

	// Prepare the stream header the header.
	mmr = acmStreamPrepareHeader( m_hDstStream, &m_DstStreamHdr, 0 ); 

//	if (mmr)
//	{
//		LTRACE("CAcmCodec::InitStream() Failed to open conversion stream");
//		CDexMessageBox( _W( "Failed to create WinCodec stream" ));
//		return CDEX_ERROR;
//	}


	EXIT_TRACE( _T( "CAcmCodec::InitStream() OK" ) );

	return CDEX_OK;
}


//////////////////// DEINIT /////////////////////////////
CDEX_ERR CAcmCodec::DeInitStream( PBYTE pOutput, PDWORD pdwOutput )
{
	CDEX_ERR err;

	ENTRY_TRACE( _T( "CAcmCodec::DeInitStream()" ) );

	// Get last bytes out of the encoder
	err = EncodeChunk(	0,
						NULL,
						pOutput,
						pdwOutput,
						ACM_STREAMCONVERTF_BLOCKALIGN | ACM_STREAMCONVERTF_END );

	// Uprepare the header
	if (m_hDstStream)
		acmStreamUnprepareHeader(m_hDstStream, &m_DstStreamHdr, 0); 

	// Close the stream.
	if (m_hPcmStream)
		acmStreamClose(m_hPcmStream, 0);

	// Close the driver
	if (m_hPcmDriver)
		acmDriverClose(m_hPcmDriver, 0);

	// Close the stream.
	if (m_hDstStream)
		acmStreamClose(m_hDstStream, 0);

	// Close the driver
	if (m_hDstDriver)
		acmDriverClose(m_hDstDriver, 0);

	
	// Delete buffer memory
	delete [] m_pbtSrcBuffer;m_pbtSrcBuffer=NULL;
	delete [] m_pbtDstBuffer;m_pbtDstBuffer=NULL;
///////////////////////////////////////////////////////////////////

	EXIT_TRACE( _T( "CAcmCodec::DeInitStream()" ) );
	return err;
}


/*
CDEX_ERR CAcmCodec::WinEncoderFunc(void* pMP3Params)
{
	CUString strTmp;

	// Initialize encoder
	if (InitStream(g_config.GetWinCodec(),g_config.GetWinCodecFmt())==CDEX_ERROR)
		return CDEX_ERROR;
	
	// Get the MP3 parameters
	MP3PARAMS* pParams=(MP3PARAMS*)pMP3Params;
		

	MMRESULT mmr;

	// Open MP3 file
	FILE* fp = fopen(pParams->lpszFileName,"wb+");

	if (fp==NULL)
		return CDEX_ERROR;

	while(pParams->pBufferFunc(pMyBuffer)>0 )
	{
		SHORT*	pSrcBuffer=(SHORT*)m_pbtSrcBuffer;
		int j=0;

		// Descramble bytes in correct order
		for (int i=0;i<1152;i++)
		{
			pSrcBuffer[j++]=pMyBuffer[0][i];
			pSrcBuffer[j++]=pMyBuffer[1][i];
		}

		m_DstStreamHdr.cbSrcLength=4*1152;

		mmr = acmStreamConvert(m_hDstStream, &m_DstStreamHdr,ACM_STREAMCONVERTF_BLOCKALIGN);

		if (mmr)
		{
			printf("Failed to do PCM to PCM conversion\n");
			return CDEX_ERROR;
		}

#ifdef VERBOSE
		LATRACE("Converted OK\n");
#endif

		if (m_DstStreamHdr.cbDstLengthUsed>0)
			fwrite(m_DstStreamHdr.pbDst,1,m_DstStreamHdr.cbDstLengthUsed,fp);
	}


	mmr = acmStreamConvert(m_hDstStream,
						   &m_DstStreamHdr,
						   ACM_STREAMCONVERTF_BLOCKALIGN |
						   ACM_STREAMCONVERTF_END);

	if (m_DstStreamHdr.cbDstLengthUsed>0)
		fwrite(m_DstStreamHdr.pbDst,1,m_DstStreamHdr.cbDstLengthUsed,fp);


	DeInitStream();

	////////////////////////// END OF MOVE TO ENCODE CHUNK //////////////////////////

	// CLose the output file
	fclose(fp);

	return CDEX_OK;
}
*/


CDEX_ERR CAcmCodec::EncodeChunk( DWORD nSamples, PSHORT pSamples, PBYTE pOutput, PDWORD pdwOutput,int nFlags )
{
	MMRESULT mmr = 0;

	// Copy the number of input samples
	memcpy( &m_pbtSrcBuffer[ m_nOffset ], pSamples, nSamples * sizeof( SHORT ) );

	m_nOffset += nSamples * sizeof( SHORT );

	*pdwOutput = 0;

	while ( m_nOffset >= m_nBufferSize && m_nOffset > 0 )
	{
		m_DstStreamHdr.cbSrcLength = m_nOffset;

		mmr = acmStreamConvert( m_hDstStream, &m_DstStreamHdr, nFlags );
		

		if ( m_DstStreamHdr.cbSrcLengthUsed > 0 )
		{
			memmove(	m_pbtSrcBuffer, 
						&m_pbtSrcBuffer[ m_nOffset ],
						m_nOffset - m_DstStreamHdr.cbSrcLengthUsed );

			m_nOffset -= m_DstStreamHdr.cbSrcLengthUsed;
		}

		// Copy the output bytes
		if ( m_DstStreamHdr.cbDstLengthUsed > 0 )
		{
			memcpy( &pOutput[ *pdwOutput ], m_DstStreamHdr.pbDst, m_DstStreamHdr.cbDstLengthUsed );
			*pdwOutput += m_DstStreamHdr.cbDstLengthUsed;
		}

		if ( 0 == m_nBufferSize )
		{
			m_nBufferSize = m_DstStreamHdr.cbSrcLengthUsed;
		}
	}

	if ( ( nFlags & ACM_STREAMCONVERTF_END ) && m_nOffset > 0 )
	{
		// get the last part out of the encoder
		m_DstStreamHdr.cbSrcLength = m_nOffset;

		mmr = acmStreamConvert( m_hDstStream, &m_DstStreamHdr, nFlags );

		// Copy the output bytes
		if ( m_DstStreamHdr.cbDstLengthUsed > 0 )
		{
			memcpy( &pOutput[ *pdwOutput ], m_DstStreamHdr.pbDst, m_DstStreamHdr.cbDstLengthUsed );
			*pdwOutput += m_DstStreamHdr.cbDstLengthUsed;
		}
	}


	if ( mmr )
	{
		ASSERT( FALSE );
		return CDEX_ERROR;
	}

#ifdef VERBOSE
	LTRACE("Converted OK\n");
#endif

	// Set the number of output bytes


	return CDEX_OK;
}




