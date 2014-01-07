/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
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


#ifndef ENCODER_H_INCLUDED
#define ENCODER_H_INCLUDED

#include "BladeMP3EncDLL.h"
#include "EncoderObjectFactory.h"

// forward class declaration
class CEncoderDlg;

extern "C"
{
	#include "sndfile.h"	
	#include "aacenc.h"
}


#include "config.h"
#include "Codec.h"
#include "ChildProcess.h"

// forward class declarations
class CTaskInfo;



class CEncoder : public CObject
{
protected:
	BOOL			m_bChunkSupport;
	BOOL			m_bOnTheFlyEncoding;
	BYTE*			m_pbOutputStream;				// OUTPUT BYTE BUFFER
	FILE*			m_fpOut;
	DWORD			m_dwOutBufferSize;				// IN BYTES !

	SHORT*			m_psInputStream;
	DWORD			m_dwInBufferSize;				// IN SHORTS !

	CUString			m_strEncoderID;
	CUString			m_strEncoderPath;
	CUString			m_strUser1;
	CUString			m_strUser2;
	INT				m_nBitrate;
	INT				m_nMaxBitrate;
	INT				m_nMinBitrate;
	INT				m_nSampleRate;
	INT				m_nOutSampleRate;
	INT				m_nInputChannels;
	INT				m_nMode;
	INT				m_nUser1;
	INT				m_nUser2;
	
	BOOL			m_bPrivate;
	BOOL			m_bCopyRight;
	BOOL			m_bOriginal;
	BOOL			m_bCRC;
	CEncoderDlg*	m_pSettingsDlg;	
	INT				m_nEncoderID;
	CUString			m_strExtension;
	BOOL			m_bRiffWavSupport;
	INT				m_nVersion;
	DOUBLE			m_dResampleRatio;
	BOOL			m_bDownMixToMono;
	BOOL			m_bUpMixToStereo;

	CTaskInfo*		m_pTask;

	BOOL			m_bCanWriteTagV1;
	BOOL			m_bCanWriteTagV2;

	DWORD			m_dwId3PadSize;
	double			m_dNormalizationFactor;
	BOOL			m_bAvailable;

public:
	CUString	GetExtension() const {return m_strExtension;}
	void	SetExtention( const CUString& strValue ) { m_strExtension = strValue;}
	virtual	CEncoderDlg*	GetSettingsDlg() =0;

	CDEX_ERR	InitInputStream();

	DWORD		DownMixToMono( PSHORT psData, DWORD dwNumSamples);
	DWORD		UpMixToStereo( PSHORT psData, PSHORT psOutData,DWORD dwNumSamples);
	void		Normalize( PSHORT psData,DWORD dwNumSamples );
	DWORD		ReSample( PSHORT psData, DWORD dwNumSamples );

	CUString	GetIniKey();

	virtual void	SaveSettings();
	virtual void	LoadSettings();

	INT		GetNumChannels() { if ( m_nMode == BE_MP3_MODE_MONO) return 1;else return 2;}

	BOOL	GetChunkSupport() {return m_bChunkSupport;}
	void	SetChunkSupport(BOOL bValue) {m_bChunkSupport=bValue;}
	BOOL	GetRiffWavSupport() {return m_bRiffWavSupport;}

	INT		GetID() { return m_nEncoderID; }
	void	SetID( INT nValue ) { m_nEncoderID = nValue; }

	INT		GetMode() const { return m_nMode; }
	void	SetMode(INT nValue) { m_nMode=nValue; }

	BOOL	GetOnTheFlyEncoding() { return m_bOnTheFlyEncoding; }
	void	SetOnTheFlyEncoding(BOOL bValue) { m_bOnTheFlyEncoding=bValue; }

	CUString GetEncoderID() {return m_strEncoderID;}
	void	SetEncoderID(CUString strValue) {m_strEncoderID=strValue;}

	CUString GetEncoderPath() {return m_strEncoderPath;}
	void	SetEncoderPath(CUString strValue) {m_strEncoderPath=strValue;}

	INT		GetUserN1() {return m_nUser1;}
	void	SetUserN1(INT nValue) {m_nUser1=nValue;}

	INT		GetUserN2() {return m_nUser2;}
	void	SetUserN2(INT nValue) {m_nUser2=nValue;}

	CUString GetUser1() {return m_strUser1;}
	void	SetUser1(CUString strValue) {m_strUser1=strValue;}

	CUString GetUser2() {return m_strUser2;}
	void	SetUser2(CUString strValue) {m_strUser2=strValue;}

	BOOL	GetPrivate() {return m_bPrivate;}
	void	SetPrivate(BOOL bValue) {m_bPrivate=bValue;}

	BOOL	GetCopyRight() {return m_bCopyRight;}
	void	SetCopyRight(BOOL bValue) {m_bCopyRight=bValue;}

	BOOL	GetOriginal() {return m_bOriginal;}
	void	SetOriginal(BOOL bValue) {m_bOriginal=bValue;}

	BOOL	GetCRC() {return m_bCRC;}
	void	SetCRC(BOOL bValue) {m_bCRC=bValue;}

	INT		GetBitrate() {return m_nBitrate;}
	void	SetBitrate(INT nValue) {m_nBitrate=nValue;}

	INT		GetMaxBitrate() {return m_nMaxBitrate;}
	void	SetMaxBitrate(INT nValue) {m_nMaxBitrate=nValue;}

	INT		GetMinBitrate() {return m_nMinBitrate;}
	void	SetMinBitrate(INT nValue) {m_nMinBitrate=nValue;}

	INT		GetVersion() {return m_nVersion;}
	void	SetVersion(INT nValue) {m_nVersion=nValue;}

	INT		GetSampleRate() {return m_nSampleRate;}
	void	SetSampleRate(INT nValue) {m_nSampleRate=nValue;}

	INT		GetOutSampleRate() {return m_nOutSampleRate;}
	void	SetOutSampleRate(INT nValue) {m_nOutSampleRate=nValue;}

	INT		GetInputChannels() {return m_nInputChannels;}
	void	SetInputChannels(INT nValue) {m_nInputChannels=nValue;}

	void	SetCTaskInfo(CTaskInfo* pValue) {m_pTask=pValue;}
	CTaskInfo* GetTaskInfo() {return m_pTask;}

	BOOL	GetCanWriteTagV1() {return m_bCanWriteTagV1;}
	BOOL	GetCanWriteTagV2() {return m_bCanWriteTagV2;}

	void	SetId3V2PadSize( DWORD dwValue ) { m_dwId3PadSize = dwValue; }
	DWORD	GetId3V2PadSize( ) const { return m_dwId3PadSize; }


	void	SetNormalizationFactor( double dValue ) { m_dNormalizationFactor = dValue; }
	double	GetNormalizationFactor( ) const { return m_dNormalizationFactor; }

	DWORD	ProcessData( SHORT* psData, DWORD dwNumSamples );

	void	SetAvailable( BOOL bValue ) { m_bAvailable = bValue; }
	BOOL	GetAvailable( ) const { return m_bAvailable; }
	
	// CONSRUCTOR
	CEncoder();
	// DESTRUCTOR
	virtual ~CEncoder();


	// METHODS
	DWORD	GetSampleBufferSize() {return m_dwInBufferSize;}
	void	SetSampleBufferSize(DWORD Value) {m_dwInBufferSize=Value;}

	virtual CDEX_ERR InitEncoder( CTaskInfo* pRecItems );
	virtual CDEX_ERR OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels) =0;
	virtual CDEX_ERR EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples) =0;
	virtual CDEX_ERR CloseStream() =0;
	virtual CDEX_ERR DeInitEncoder() =0;
	virtual CDEX_ERR DosEncode(	const CUString& strInFileName,
								const CUString& strInFileExt, 
								const CUString& strOutFileName,
								const CUString& strOutDir,
								INT&  nPercent,
								BOOL& bAbort);

	virtual BOOL	IsOutputStereo() const {return ( 1 == GetMode() )? FALSE : TRUE ;}
	virtual BOOL	IsInputStereo() const {return ( 2 == m_nInputChannels )? TRUE : FALSE ;}
};

class CDLLEncoder:public CEncoder
{
protected:
	BEINITSTREAM	m_InitStream;
	BEENCODECHUNK	m_EncodeChunk;
	BEDEINITSTREAM	m_DeinitStream;
	BECLOSESTREAM	m_CloseStream;
	BEVERSION		m_Version;
	BE_CONFIG		m_beConfig;
	BEWRITEVBRHEADER	m_WriteVbrTag;
	HINSTANCE		m_hDLL;
	HBE_STREAM		m_hbeStream;
	CUString			m_strStreamFileName;
public:
	virtual	CEncoderDlg*	GetSettingsDlg() =0;

	// CONSTRUCTOR
	CDLLEncoder();

	// DESTRUCTOR
	virtual ~CDLLEncoder();

	// METHODS
	void GetDLLVersionInfo();
	virtual CDEX_ERR InitEncoder( CTaskInfo* pRecItems );
	virtual CDEX_ERR OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels);
	virtual CDEX_ERR EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples);
	virtual CDEX_ERR CloseStream();
	virtual CDEX_ERR DeInitEncoder();
};

extern CAcmCodec g_AcmCodec;

#endif /* ENCODER_H_INCLUDED */
