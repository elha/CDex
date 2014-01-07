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
#include <windows.h>
#include <wtypes.h>
#include <comdef.h>
#include <stdio.h>
#include <mmsystem.h>
#include "wavtowma.h"
#include "Config.h"


///////////////////////////////////////////////////////////////

WavToWma::WavToWma()
{
	m_pszOutputFilename = NULL;
	m_pIWMAudioWriter = NULL;
	m_hDll=NULL;
	m_pAudioWriter=NULL;
}

////////////////////////////////////////////////////////////////

WavToWma::~WavToWma()
{
	if (m_hDll)
		FreeLibrary(m_hDll);

    if ( m_pszOutputFilename )
        delete m_pszOutputFilename;
    if ( m_pIWMAudioWriter )
    {
        m_pIWMAudioWriter->Flush();
        m_pIWMAudioWriter->Release();
    }
}


////////////////////////////////////////////////////////////////

HRESULT WavToWma::Init( 
                        LPCSTR pszOutFile,
                        DWORD  dwBitrate,
                        DWORD dwSampleRate,
                        DWORD dwChannels,
						DWORD& dwBufferSize
                        )
{
    HRESULT hr=S_OK;

	m_hDll=LoadLibrary( _T( "WmAudSDK.dll" ) );

	if (m_hDll==NULL)
	{
		CDexMessageBox( _W( "Could not found WmAudSDK.DLL file" ) ); 
		return !S_OK;
	}

	m_pAudioWriter=(WMAUDIOCREATEWRITER*)GetProcAddress(m_hDll,"WMAudioCreateWriter");
	if (m_hDll==NULL)
	{
		CDexMessageBox( _W( "Could not found WMAudioCreateWriter library function" ) ); 
		return -1;
	}

    WCHAR pszwOutFile[1024];

	WAVEFORMATEX wfmt;

	memset(&wfmt,0x00,sizeof(wfmt));

	wfmt.wFormatTag		= WAVE_FORMAT_PCM;
	wfmt.nChannels		= (WORD)dwChannels;
	wfmt.nSamplesPerSec	= dwSampleRate;
	wfmt.wBitsPerSample = 16;
	wfmt.nBlockAlign	= wfmt.nChannels * ((wfmt.wBitsPerSample + 7) / 8);
	wfmt.nAvgBytesPerSec= wfmt.nBlockAlign * wfmt.nSamplesPerSec;

	dwBufferSize=0;

    if( 0 == MultiByteToWideChar( CP_ACP, 0, pszOutFile, strlen( pszOutFile ) + 1, pszwOutFile, sizeof( pszwOutFile ) ) )
    {
        CDexMessageBox( _W( "internal error" ) );
		ASSERT(FALSE);
        return( E_UNEXPECTED );
    }

//    if ( m_pWavFile->Open( pszInFile ) < 0 )
//    {
//        printf("Error Failed to open Wav File %s\n", pszInFile ); 
//        return( E_FAIL );
//    }


	
    hr = m_pAudioWriter( pszwOutFile, &m_pIWMAudioWriter ); 
    if ( FAILED( hr ) )
    {
        CDexMessageBox( _W( "Failed to Create Audio Writer" ) );
		ASSERT(FALSE);
        return( hr );
    }

    hr = m_pIWMAudioWriter->SetInputFormat( &wfmt );
    if ( FAILED( hr ) )
    {
        m_pIWMAudioWriter->Release();
        m_pIWMAudioWriter = NULL;
        CDexMessageBox( _W( "Failed to Set Input Format" ) );
		ASSERT(FALSE);
        return( hr );
    }

    hr = m_pIWMAudioWriter->SetOutputFormat( dwBitrate, dwSampleRate, dwChannels,0);
    if ( FAILED( hr ) )
    {
        m_pIWMAudioWriter->Release();
        m_pIWMAudioWriter = NULL;
        CDexMessageBox( _W( "Failed to Set Output Format" ) );
		ASSERT(FALSE);
        return( hr );
    }

	dwBufferSize=16 * 1024;

    return( hr );
}

//////////////////////////////////////////////////////////////////////

HRESULT WavToWma::SetAttribute( ContentDesc *pCntDesc )
{
    HRESULT hr;
    WCHAR wName[512];
    WCHAR wValue[512];
    BYTE *pValue;
    DWORD dwValue;
    WORD wLen;
    
    if  ( 0 == MultiByteToWideChar( CP_ACP, 0, pCntDesc->pName, strlen( pCntDesc->pName ) + 1,
                              wName, sizeof( wName ) ) )
    {
		ASSERT(FALSE);
        printf(" invalid args SetTitlen\n" );
        return( E_INVALIDARG );
    }
    if ( pCntDesc->type == WMT_TYPE_STRING) 
    {
          if  ( 0 == MultiByteToWideChar( CP_ACP, 0, pCntDesc->pValue, 
                                          strlen( pCntDesc->pValue ) + 1,
                                          wValue, sizeof( wValue ) ) )
          {
               printf(" invalid args SetTitlen\n" );
               return( E_INVALIDARG );
          }
          wLen = sizeof( WCHAR ) * ( wcslen( wValue ) + 1 );
          pValue = ( BYTE * ) wValue;
    }
    else if ( pCntDesc->type == WMT_TYPE_DWORD )
    {
        wLen = sizeof( DWORD );
        dwValue = atol( pCntDesc->pValue );
        pValue = ( BYTE * ) &dwValue;
    }
    else
    {
        wLen = strlen( pCntDesc->pValue );
        pValue = ( BYTE * ) pCntDesc->pValue;
    }

    hr = m_pIWMAudioWriter->SetAttribute( wName, pCntDesc->type, pValue, wLen );

    if ( FAILED( hr ) )
    {
		ASSERT(FALSE);
        printf(" SetAttribute %s Failed %x\n", pCntDesc->pName, hr );
    }
	return hr;
}


/////////////////////////////////////////////////////////////////////
HRESULT WavToWma::SetDRM( BOOL fUseDRM )
{
    HRESULT hr;

    hr = m_pIWMAudioWriter->SetAttribute( L"use_drm", WMT_TYPE_BOOL, (BYTE *) &fUseDRM, sizeof( BOOL ) );

    if( FAILED( hr ) )
    {
        CDexMessageBox( _W( "SetAttribute use_drm failed" ) );
    }

    return( hr );
}
/////////////////////////////////////////////////////////////////////
HRESULT WavToWma::ConvertChunk(BYTE* pBuffer,DWORD dwBytes)
{
    HRESULT hr = S_OK;
//    DWORD cbBuffer = ( 16 * 1024 );
//    BYTE pBuffer[ 16 * 1024 ];
//    DWORD dwCurrent = 0;
//    DWORD dwBytes = m_pWavFile->GetDataLen();

    if ( dwBytes )
    {
//        DWORD cbRead = cbBuffer;

//        if ( cbRead > dwBytes )
//        {
//            cbRead = dwBytes;
//        }

//        if ( m_pWavFile->ReadData( dwCurrent, cbRead, pBuffer ) < 0 )
//        {
//            hr = E_FAIL;
//            break;
//        }
        hr = m_pIWMAudioWriter->WriteSample( pBuffer, dwBytes );
        if ( FAILED( hr ) )
        {
            return CDEX_ERROR;
        }

//        dwBytes -= cbRead;
//        dwCurrent += cbRead;
    }
	return hr;
}

////////////////////////////////////////////////////////////////////////
HRESULT WavToWma::Cleanup()
{
    HRESULT hr;

	if (m_pIWMAudioWriter)
	{
		hr = m_pIWMAudioWriter->Flush();
		if (FAILED( hr ) )
		{
			return( hr );
		}
		delete m_pszOutputFilename;
		m_pszOutputFilename = NULL;
		m_pIWMAudioWriter->Release();
		m_pIWMAudioWriter = NULL;
	}
    return( S_OK );

}

