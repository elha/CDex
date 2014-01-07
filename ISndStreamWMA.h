/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 - 2007 Albert L. Faber
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


#ifndef ISNDSTREAMWMA_H_INCLUDED
#define ISNDSTREAMWMA_H_INCLUDED

#include "ISndStream.h"

#include "wmaudiosdk.h"

#include <mmsystem.h>
#include <vfw.h>

typedef HRESULT (_stdcall WMAUDIOCREATEREADER)(
  LPCWSTR pszFilename,
  IWMAudioReadCallback* pIWMReadCallback,
  IWMAudioReader** ppIWMAudioReader,
  void* pvReserved
); 


///////////////////////////////////////////////////////////////////////////////
class CWmaDecoder :  public IWMAudioReadCallback
{
private:
	CEvent					m_hBufferAvailable;
	CEvent					m_hNotInOnSample;
	BOOL					m_bBufferFilled;
    LONG					m_cRef;
	PBYTE					m_pData;
    volatile BOOL			m_bEOF;
    TCHAR					m_pszUrl[MAX_PATH];
    IWMAudioReader*			m_pReader;
	WMAUDIOCREATEREADER*	m_pReaderFunc;
//	GETATTRIBUTEBYNAME*		m_pGetAttrByName;
//	GETATTRIBUTECOUNT*		m_pGetAttrCount;
	INT						m_nDataSize;
	HINSTANCE				m_hDll;
	DWORD					m_dwBitRate;
	DWORD					m_dwTotalTime;
	DWORD					m_dwCurrentTime;
	BOOL					m_bStop;
public:
    CWmaDecoder();
    virtual ~CWmaDecoder();

	// ACCESSOR
	virtual BOOL GetDataSize() {return m_nDataSize;}
	virtual BOOL IsEOF() {return m_bEOF;}
	virtual	DWORD GetBitrate() const {return m_dwBitRate;}
	virtual DWORD GetTotalTime() const {return m_dwTotalTime;}
	virtual DWORD GetCurrentTime() const {return m_dwCurrentTime;}
	virtual BOOL IsBufferFilled() {return m_bBufferFilled;}
	virtual VOID Seek( DWORD dwSeekTimeInMs );
	virtual VOID Stop( );
	virtual VOID Start( );
	// MUTATORS
	virtual HRESULT CWmaDecoder::Init( const CUString& strFileName, WAVEFORMATEX* pWavFormat);
//	virtual void SetBufferAvailable() {m_hBufferAvailable.PulseEvent();}
//	virtual void ClearBufferFull() {m_bBufferFilled=FALSE;}
	virtual void SetDataBuffer(PBYTE pData, INT nSize);

    // IUnknown
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void **ppvObject );
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

    // IWMAudioReadCallback
    virtual HRESULT STDMETHODCALLTYPE OnSample( 
        /* [in] */ const BYTE * pData,
        /* [in] */ DWORD cbData,
        /* [in] */ DWORD dwMsTime );

    virtual HRESULT STDMETHODCALLTYPE OnStatus( 
        /* [in] */ WMT_STATUS Status, 
        /* [in] */ HRESULT hr,
        /* [in] */ const VARIANT *pParam );

protected:

//    void OnWaveOutMsg( UINT uMsg, DWORD dwParam1, DWORD dwParam2 );
//    HRESULT DoCodecDownload( GUID*  pCodecID );
//    LONG    m_cBuffersOutstanding;
//    HANDLE  m_hCompletionEvent;
//    HRESULT *m_phrCompletion;


#ifdef SUPPORT_DRM
    IWMAudioReadCallback *m_pDRMCallback;
#endif
};



class ISndStreamWMA : public ISndStream
{
private:
	CWmaDecoder			m_Decoder;
	DWORD				m_dwSamples;
	FILE*				m_pFile;
	BOOL				m_bAbort;
public:
	ISndStreamWMA();
	virtual ~ISndStreamWMA();
	virtual BOOL	OpenStream( const CUString& strFileName);
	virtual DWORD	Read(PBYTE pbData,DWORD dwNumBytes);
	virtual BOOL	CloseStream();
	virtual LONG	Seek( LONG lOff, UINT nFrom );
	virtual void	Flush();
	virtual void	Pause();
	virtual VOID	InfoBox( HWND hWnd );

	virtual DWORD	GetTotalTime();
	virtual DWORD	GetCurrentTime();

	
};




#endif


