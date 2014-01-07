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
#ifndef CODEC_INCLUDED
#define CODEC_INCLUDED

#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>			// Multimedia registration
#include <msacm.h>			// Audio Compression Manager
#include <vector>
#include "config.h"

//CAcmCodec g_AcmCodec;

// 1 byte alignment or SCSI structures
#pragma pack(push,1)

class CAcmFormat : public ACMFORMATDETAILS
{
public:
	// CONSTRUCTOR
	CAcmFormat();

	// DESTRUCTOR
	~CAcmFormat();

	// COPY CONSTRUCTOR
	CAcmFormat(const CAcmFormat& rhs);

	// COPY CONSTRUCTOR
	CAcmFormat(const ACMFORMATDETAILS& rhs);

	
	// ASSIGNMENT OPERATOR
	CAcmFormat& operator=(const CAcmFormat& rhs);
};


class CAcmDriverInfo : public CObject
{
public:
	// CONSTRUCTOR
	CAcmDriverInfo();

	// DESTRUCTOR
	~CAcmDriverInfo();

	// COPY CONSTRUCTOR
	CAcmDriverInfo(const CAcmDriverInfo& rhs);

	// ASSIGNMENT OPERATOR
	CAcmDriverInfo& operator=(const CAcmDriverInfo& rhs);

	HACMDRIVERID				hadid;
	ACMDRIVERDETAILS			dd;
	vector<CAcmFormat>			m_vFormatDetails;
};


class CCodecDetails:public CObject
{
	CCodecDetails();
	~CCodecDetails();

};

class CAcmCodec:public CObject
{
public:
	vector<CAcmDriverInfo>		m_vAcmDriverInfo;
private:
	BOOL						m_bIntermPCM;
	int							m_nOffset;
	PBYTE						m_pbtSrcBuffer;
	PBYTE						m_pbtPcmBuffer;
	PBYTE						m_pbtDstBuffer;

	CAcmDriverInfo*				m_pPcmAcmDriver;
	CAcmDriverInfo*				m_pDstAcmDriver;

	CAcmFormat*					m_pPcmAcmFormat;
	CAcmFormat*					m_pDstAcmFormat;

	HACMSTREAM					m_hPcmStream;
	HACMSTREAM					m_hDstStream;

	HACMDRIVER					m_hPcmDriver;
	HACMDRIVER					m_hDstDriver;

	// Create ACM Stream Header
	ACMSTREAMHEADER				m_PcmStreamHdr;
	ACMSTREAMHEADER				m_DstStreamHdr;

	DWORD						m_dwSrcSamples;
	DWORD						m_dwDstSamples;

	BOOL						m_bMP3CodecInstalled;
	int							m_nBufferSize;
public:
	CAcmCodec();
	~CAcmCodec();

	void	ShowError(MMRESULT& mmr);

	// Callback functions have to be static
	static	BOOL CALLBACK DriverEnumProc(HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport);
	static	BOOL CALLBACK FormatEnumProc(HACMDRIVERID hadid, LPACMFORMATDETAILS pafd, DWORD dwInstance, DWORD fdwSupport);

	void	GetVersion();
	void	EnumDrivers();
	int		GetNumEncoders(DWORD dwTag);
	void	ReInit();
	CDEX_ERR DeInitStream(PBYTE pOutput, PDWORD pdwOutput);
	BOOL	IsMP3CodecInstalled() const {return m_bMP3CodecInstalled;}
//	CDEX_ERR	WinEncoderFunc(void* pMP3Params);

	CUString	GetACMCodecName(int CodecIndex) const;
	void	DisplayCodecInfo(int nCodecIndex) const;
	int		GetNumCodecs() const {return m_vAcmDriverInfo.size();}
	int		GetNumDetails(int nCodecIndex)const;
	CUString	GetACMCodecName(int nCodecIndex,DWORD dwTag);
	CUString GetFormatString(int nCodecIndex,int nFormatIndex) const;
	CUString GetFormatString(int nCodecIndex,int nFormatIndex,DWORD dwTag) const;
	CDEX_ERR InitStream(CUString& strCodec,CUString& strFormat,INT nCodecID,int nSampleFreq,int nChannels,BOOL bHighQuality);
	CDEX_ERR EncodeChunk(DWORD nSamples, PSHORT pSamples, PBYTE pOutput, PDWORD pdwOutput,int nFlags=ACM_STREAMCONVERTF_BLOCKALIGN);
};


void TestEnumDrivers();

#pragma pack(pop)

#endif