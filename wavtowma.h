//
//  Microsoft Windows Media Technologies
//  Copyright (C) 1999 Microsoft Corporation.  All rights reserved.
//
// You have a royalty-free right to use, modify, reproduce and distribute
// the Sample Application Files (including modified versions) in any way 
// you determine to be useful, provided that you agree that Microsoft 
// Corporation provides no warrant or support, and has no obligation or
// liability resulting from the use of any Sample Application Files. 
//


#include <windows.h>
#include <wtypes.h>
#include "wmaudiosdk.h"

typedef HRESULT (_stdcall WMAUDIOCREATEWRITER)( LPCWSTR pszFilename, IWMAudioWriter **ppIWMAudioWriter ); 

typedef struct ContentDesc
{
    LPSTR pName;
    WMT_ATTR_DATATYPE type;
    LPSTR pValue;
} ContentDesc;

class WavToWma
{
	HINSTANCE			 m_hDll;
	WMAUDIOCREATEWRITER* m_pAudioWriter;

public:

    WavToWma();
    ~WavToWma();

    HRESULT  Init( LPCSTR pszOutFile,  DWORD bitrate, DWORD samplerate, DWORD nchannels, DWORD& dwBufferSize);
    
    HRESULT SetAttribute( ContentDesc *pCntDesc );
    HRESULT SetDRM( BOOL fUseDRM );
    
	HRESULT ConvertChunk(BYTE* pBuffer,DWORD dwBytes);
    HRESULT Cleanup();
protected:

//    HANDLE m_InFile;
    LPWSTR m_pszOutputFilename;
    IWMAudioWriter *m_pIWMAudioWriter;
//    WavFile *m_pWavFile;
};
