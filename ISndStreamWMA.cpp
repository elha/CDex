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


#include "StdAfx.h"
#include "ISndStreamWMA.h"
#include "Config.h"

#define WAVFORMATSIZE 1024
#define WMABUFFERSIZE (1024*256)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



INITTRACE( _T( "ISndStreamWMA" ) );

///////////////////////////////////////////////////////////////////////////////
CWmaDecoder::CWmaDecoder()
{
    m_cRef			= 1;
	m_pData			= NULL;
	m_bEOF			= FALSE;
//	m_hBufferAvailable.ResetEvent();
	m_hDll			= NULL;

    memset(m_pszUrl,0,sizeof(m_pszUrl));
	
	m_nDataSize= 0;


	m_dwBitRate=0;
	m_dwTotalTime=0;
	m_dwCurrentTime=0;

	m_pReaderFunc		= NULL;

	m_bBufferFilled		= FALSE;
	
	// Default not in OnSample
	m_hNotInOnSample.SetEvent();
	/*
    m_cBuffersOutstanding = 0;

    m_pReader = NULL;
    m_hwo = NULL;

    m_bEOF = FALSE;
*/
	m_bStop = FALSE;

#ifdef SUPPORT_DRM
    m_pDRMCallback = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////////////
CWmaDecoder::~CWmaDecoder()
{
    if( m_pReader != NULL )
    {
        m_pReader->Release();
        m_pReader = NULL;
    }

#ifdef SUPPORT_DRM
    if( NULL != m_pDRMCallback )
    {
        m_pDRMCallback->Release();
        m_pDRMCallback = NULL;
    }
#endif
	// free the DLL
	if (m_hDll)
		FreeLibrary(m_hDll);

}

void CWmaDecoder::SetDataBuffer(PBYTE pData, INT nSize)
{
	// set data pointer, where the data will be copied to 
	m_pData=pData;

	// set the data size of the buffer
	m_nDataSize=nSize;

	// clear the buffer filled flag
	m_bBufferFilled = FALSE;

	LTRACE( _T( "Set data buffer, size is %d (set event)" ), nSize );

	// Set event, to indicate that the buffer is available
	m_hBufferAvailable.SetEvent();
}


VOID CWmaDecoder::Seek( DWORD dwSeekTimeInMs )
{
	ENTRY_TRACE( _T( "CWmaDecoder::Seek ( %d )" ), dwSeekTimeInMs );

	if ( m_pReader )
	{
		m_bStop = TRUE;

		// Set event, to indicate that the buffer is available
		m_hBufferAvailable.SetEvent();

		// Make sure were not in the OnSample routine
		m_hNotInOnSample.Lock();

		// Stop the WMA decoder
		m_pReader->Stop();

		// Seek to the desired position
		m_pReader->Seek( dwSeekTimeInMs );

		m_bStop = FALSE;

		m_pReader->Start( );

		// Set the event again
		m_hNotInOnSample.SetEvent();
	}

	EXIT_TRACE( _T( "CWmaDecoder::Seek(  )" ) );
}

VOID CWmaDecoder::Stop( )
{
	ENTRY_TRACE( _T( "CWmaDecoder::Stop(  )" ) );

	m_bStop = TRUE;

	if ( m_pReader )
	{
		// set trigger to get out of the OnSample function
		SetDataBuffer( NULL, 0 );

		// Make sure were not in the OnSample routine
		m_hNotInOnSample.Lock();

		// Stop the WMA decoder
		m_pReader->Stop();

		// Set the event again
		m_hNotInOnSample.SetEvent();
	}

//	m_pReader= NULL;

	EXIT_TRACE( _T( "CWmaDecoder::Stop(  )" ) );
}

VOID CWmaDecoder::Start( )
{
	ENTRY_TRACE( _T( "CWmaDecoder::Start(  )" ) );

	m_bStop = FALSE;
	
	// start the WMA reader decoder
	m_pReader->Start( );

	EXIT_TRACE( _T( "CWmaDecoder::Start(  )" ) );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CWmaDecoder::QueryInterface(
    REFIID riid,
    void **ppvObject )
{
    return( E_NOINTERFACE );
}


///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CWmaDecoder::AddRef()
{

    return( InterlockedIncrement( &m_cRef ) );
}


///////////////////////////////////////////////////////////////////////////////
ULONG STDMETHODCALLTYPE CWmaDecoder::Release()
{
    ULONG uRet = InterlockedDecrement( &m_cRef );

    if( 0 == uRet )
    {
        delete this;
    }

    return( uRet );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CWmaDecoder::OnSample( 
        /* [in] */ const BYTE *pData,
        /* [in] */ DWORD cbData,
        /* [in] */ DWORD dwMsTime )
{
    HRESULT hr = S_OK;

    ENTRY_TRACE( _T( "CWmaDecoder::OnSample( %08X, %d, %d ), tick count %d " ), pData, cbData, dwMsTime,GetTickCount() );

	// Indicate that were in OnSample function
	m_hNotInOnSample.ResetEvent();

	if ( FALSE == m_bStop )
	{
		LTRACE( _T( "Copy data to buffer ( wait for buffer)" ) );

		// Wait till buffer is available
		m_hBufferAvailable.Lock();

		LTRACE( _T( "Copy data to buffer ( passed the buffer lock), buffer size %d cbData=%d" ), m_nDataSize,cbData);

		// Set data size
		m_nDataSize = min ( (INT)cbData, m_nDataSize );

		// Fill buffer with the decoded data
		if ( m_pData && m_nDataSize > 0 )
		{
			CopyMemory( m_pData, pData, m_nDataSize );
		}

		// Set current time
		m_dwCurrentTime= dwMsTime;

		m_bBufferFilled = TRUE;

		// Clear buffer available event
		m_hBufferAvailable.ResetEvent();


	}
	else
	{
		// Clear data size
		m_nDataSize=0;
	}

	// Indicate that we leave the OnSample function
	m_hNotInOnSample.SetEvent();

    return( S_OK );
}

///////////////////////////////////////////////////////////////////////////////
HRESULT CWmaDecoder::Init( const CUString& strFileName, WAVEFORMATEX* pWavFormat)
{
    HRESULT hr;

	if ( m_hDll == NULL )
	{
		m_hDll = LoadLibrary( _T( "WmAudSDK.dll" ) );
	}

	if ( m_hDll==NULL )
	{
		LTRACE( _T( "Could not find the WmAudSDK.DLL file" ) ); 
		CDexMessageBox( _W( "Could not find the WmAudSDK.DLL file" ) ); 
		return !S_OK;
	}

	m_pReaderFunc = (WMAUDIOCREATEREADER*) GetProcAddress(m_hDll, "WMAudioCreateReader" );
	if ( m_pReaderFunc == NULL)
	{
		LTRACE( _T( "Could not access the WMAudioCreateReader function" ) ); 
		return !S_OK;
	}

/*
	m_pGetAttrCount = (GETATTRIBUTECOUNT*) GetProcAddress(m_hDll,"GetAttributeCount");

	if ( m_pGetAttrCount == NULL)
	{
		LTRACE("Could not access the GetAttributeCount function"); 
		return !S_OK;
	}
*/

    CUStringConvert strCnv;
    _tcsnccpy( m_pszUrl, strCnv.ToT( strFileName ), MAX_PATH );


#ifdef SUPPORT_DRM
    //
    // Make a DRM callback object
    //
    hr = WMAudioCreateDRMCallback( this, WMT_RIGHT_PLAYBACK, &m_pDRMCallback );

    if( FAILED( hr ) )
    {
        LTRACE( "failed to create callback (hr=%#X)", hr );
        return( hr );
    }

    hr = WMAudioCreateReader( m_pszUrl, m_pDRMCallback, &m_pReader, NULL );

#else
    // Use ourselves as the callback
// AF TODO FIXME    hr = m_pReaderFunc( m_pszUrl, this, &m_pReader, NULL );
#endif
    
    if( FAILED( hr ) )
    {
        LTRACE( _T( "failed to create audio reader (hr=%#X) " ), hr );
        return( hr );
    }

    //
    // It worked!  Display various attributes
    //

    WORD i, wAttrCnt;

    hr = m_pReader->GetAttributeCount( &wAttrCnt );

    if ( FAILED( hr ) )
    {
        LTRACE( _T( " GetAttributeCount Faied %x" ), hr );
        return( hr );
    }
    for ( i = 0; i < wAttrCnt ; i++ )
    {

        WCHAR  wName[512];
        WORD cbNamelen = sizeof ( wName );
        WMT_ATTR_DATATYPE type;
        BYTE pValue[512];
        WORD cbLength = sizeof( pValue );

        hr = m_pReader->GetAttributeByIndex( i, wName, &cbNamelen, &type, pValue, &cbLength );
        if ( FAILED( hr ) ) 
        {
            if ( hr == E_NOTIMPL )
            {
                continue;
            }
            printf( "GetAttributeByIndex Failed %x", hr );
            return( hr );
        }

		if ( memcmp( wName, g_wszWMADuration, cbNamelen ) == 0 )
		{
			m_dwTotalTime= *((DWORD *) pValue);
		}
		if ( memcmp( wName,  g_wszWMABitrate , cbNamelen ) == 0 )
		{
			m_dwBitRate= *((DWORD *) pValue);
		}
		m_dwCurrentTime= 0;


        switch ( type )
        {
        case WMT_TYPE_DWORD:
            LTRACE( _T( "%S:  %d" ), wName, *((DWORD *) pValue) );
            break;
        case WMT_TYPE_STRING:
            LTRACE( _T( "%S:   %S" ), wName, (WCHAR *) pValue );
            break;
        case WMT_TYPE_BINARY:
            LTRACE( _T( "%S:   Type = Binary of Length %d" ), wName, cbLength );
            break;
        case WMT_TYPE_BOOL:
            LTRACE( _T( "%S:   %s" ), wName, ( * ( ( BOOL * ) pValue) ? _T( "true" ) : _T( "false" ) ) );
            break;
        default:
            break;
        }
    }

	LTRACE( _T( "GlobalBitRate = %d\n" ), g_wszWMABitrate );

    // Set up for audio playback
    hr = m_pReader->GetOutputFormat( pWavFormat, WAVFORMATSIZE );

    if( FAILED( hr ) )
    {
        LTRACE( _T( "failed GetOutputFormat(), hr=0x%lX" ), hr );
        return( hr );
    }

    // Start reading the data (and rendering the audio)
    hr = m_pReader->Start( );

	::Sleep( 10 );

    if( FAILED( hr ) )
    {
        LTRACE( _T( "failed Start(), hr=0x%lX" ) , hr );
        return( hr );
    }

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE CWmaDecoder::OnStatus( 
        /* [in] */ WMT_STATUS Status, 
        /* [in] */ HRESULT hr,
        /* [in] */ const VARIANT *pParam )
{
    switch( Status )
    {
		case WMT_ERROR:
			LTRACE( _T( "OnStatus( WMT_ERROR )" ) );
        break;

		case WMT_BUFFERING_START:
			LTRACE( _T( "OnStatus( WMT_BUFFERING START)" ) );
        break;

		case WMT_BUFFERING_STOP:
			LTRACE( _T( "OnStatus( WMT_BUFFERING STOP)" ) );
        break;

		case WMT_EOF:
			LTRACE( _T( "OnStatus( WMT_EOF ), set EOF flag " ) );

			// set EOF flag
	        m_bEOF = TRUE;

			// Release buffer if it is waiting for more data
			//	m_hBufferAvailable.SetEvent();
        break;

    case WMT_LOCATING:
        LTRACE( _T( "OnStatus( WMT_LOCATING )" ) );
        break;

    case WMT_CONNECTING:
        LTRACE( _T( "OnStatus( WMT_CONNECTING )" ) );
        break;

    case WMT_NO_RIGHTS:
        {
            LPWSTR pszEscapedURL = NULL;

//            hr = MakeEscapedURL( m_pszUrl, &pszEscapedURL );

            if( SUCCEEDED( hr ) )
            {
                WCHAR szURL[ 0x1000 ];

                swprintf( szURL, L"%s&filename=%s&embedded=false", pParam->bstrVal, pszEscapedURL );

//                hr = LaunchURL( szURL );

                if( FAILED( hr ) )
                {
                    LTRACE( _T( "Unable to launch web browser to retrieve playback license (err = %#X)" ), hr );
                }

                delete [] pszEscapedURL;
            }
        }
        break;

    case WMT_MISSING_CODEC:

        //
        // find the guid for the missing codec
        //
        GUID    guidCodecID;

        hr = CLSIDFromString( pParam->bstrVal, &guidCodecID );
        if( FAILED( hr ) )
        {
            return( hr );
        }

//        hr = DoCodecDownload( &guidCodecID );
        if( FAILED( hr ) )
        {
            return( hr );
        }

        break;

        break;
    };

    return( S_OK );
}


/*
///////////////////////////////////////////////////////////////////////////////
HRESULT CWmaDecoder::DoCodecDownload( GUID* pguidCodecID )
{
    HRESULT hr;

    CoInitialize( NULL );

    //
    // Create a DownloadCallback object to manage the codec download
    // process for us (see codecdownload.h/cpp)
    //
    CDownloadCallback*  pCallback = new CDownloadCallback();
    if( NULL == pCallback )
    {
        CoUninitialize();
        return( E_OUTOFMEMORY );
    }

    IBindStatusCallback *pBSCallback;
    hr = pCallback->QueryInterface( IID_IBindStatusCallback, (void **) &pBSCallback );
    assert( hr == S_OK );

    //
    // Do a LoadLibrary()/GetProcAddress() to grab the urlmon DLL and some functions out of it.
    // We do this so that we can dynamically bind to the internet stuff and not require it to 
    // be a part of our base SDK download package.  By dynamically binding, we still allow
    // all the other functions to work even if the codec download stuff can't.
    // 
    HINSTANCE   hinstUrlmonDLL;

    hinstUrlmonDLL = LoadLibrary( "urlmon.dll" );
    if( NULL == hinstUrlmonDLL )
    {
        CoUninitialize();
        return( E_FAIL );
    }

    //
    // CreateAsyncBindCtx to create the bind context
    //
    FARPROC     lpfnCreateAsyncBindCtx = NULL;

    lpfnCreateAsyncBindCtx = GetProcAddress( hinstUrlmonDLL, "CreateAsyncBindCtx" );
    if( NULL == lpfnCreateAsyncBindCtx )
    {
        FreeLibrary( hinstUrlmonDLL );
        CoUninitialize();
        return( E_FAIL );
    }

    IBindCtx*   pBindCtx = NULL;

    hr = (*((HRESULT(STDAPICALLTYPE *)(DWORD,IBindStatusCallback*,IEnumFORMATETC*,IBindCtx**))lpfnCreateAsyncBindCtx))( 
                        0, 
                        pBSCallback, 
                        NULL, 
                        &pBindCtx );
    if( FAILED( hr ) ) 
    {
        FreeLibrary( hinstUrlmonDLL );
    	CoUninitialize();
        return hr;
    }

    //
    // CoGetClassObjectFromURL to grab the object from the across the network
    //
    FARPROC     lpfnCoGetClassObjectFromURL = NULL;

    lpfnCoGetClassObjectFromURL = GetProcAddress( hinstUrlmonDLL, "CoGetClassObjectFromURL" );
    if( NULL == lpfnCoGetClassObjectFromURL )
    {
        FreeLibrary( hinstUrlmonDLL );
        CoUninitialize();
        return( E_FAIL );
    }

    IUnknown*   pUnkObject = NULL;

    hr = (*((HRESULT(STDAPICALLTYPE *)(REFCLSID,LPCWSTR,DWORD,DWORD,LPCWSTR,LPBINDCTX,DWORD,LPVOID,REFIID,VOID**))lpfnCoGetClassObjectFromURL))(
                        *pguidCodecID,          // the clsid we want
                        NULL,                   // the URL we're downloading from.  NULL means go to the registry to find the URL
                        0x1,                    // major version number.  ACM codecs aren't versioned, so we send zero for "any"
                        0x0,                    // minor version number.
                        NULL,                   // mime type
				        pBindCtx,               // bind ctx used for downloading/installing
                        CLSCTX_INPROC,          // execution context
                        NULL,                   // reserved, must be NULL
                        IID_IUnknown,           // interface to obtain
				        (void **)&pUnkObject);  // pointer to store the new object

    if( hr == S_ASYNCHRONOUS ) 
    {
        //
        // Turn an asynchronous call into a synchronous call by
        // sitting here and waiting for the call to finish.
        //
	    for (;;) 
        {
	        HANDLE ev = pCallback->m_evFinished;
	        
	        DWORD dwResult = MsgWaitForMultipleObjects(
			                         1,
			                         &ev,
			                         FALSE,
			                         INFINITE,
			                         QS_ALLINPUT );

	        if( dwResult == WAIT_OBJECT_0 )
            {
		        break;
            }
	        
	        assert( dwResult == WAIT_OBJECT_0 + 1 );

            //
	        // Eat messages and go round again
            //
	        MSG Message;
	        while( PeekMessage( &Message,NULL,0,0,PM_REMOVE ) ) 
            {
		        TranslateMessage(&Message);
		        DispatchMessage(&Message);
	        }
	    }

	    hr = pCallback->m_hrBinding;
    }

    //
    // Do some cleaning
    //
    if( pBindCtx )
    {
        pBindCtx->Release();
        pBindCtx = NULL;
    }

    if( pBSCallback )
    {
        pBSCallback->Release();
        pBSCallback = NULL;
    }

    if( pUnkObject )
    {
        pUnkObject->Release();
        pBindCtx = NULL;
    }

    FreeLibrary( hinstUrlmonDLL );

    //
    // We treat REGDB_E_CLASSNOTREG as a success because ACM components are
    // not COM objects -- therefore they'll get downloaded just fine, but the
    // call to instantiate them will fail with REGDB_E_CLASSNOTREG.
    //
    if( REGDB_E_CLASSNOTREG == hr )
    {
        hr = S_OK;
    }

    CoUninitialize();
    return( hr );
}
*/



// CONSTRUCTOR
ISndStreamWMA::ISndStreamWMA()
	:ISndStream()
{
	SetFileExtention( _T( "wma" ) );
	SetStreamType( SNDSTREAM_WMA );

	m_dwSamples	= 0;
	m_bAbort	= FALSE;
}

// DESRUCTOR
ISndStreamWMA::~ISndStreamWMA()
{
	CloseStream();
}


BOOL ISndStreamWMA::OpenStream(const CUString& strFileName)
{
	DWORD			dwRead=0;
	WAVEFORMATEX*	pWavFormat=NULL;

	// Set file name 
	SetFileName( strFileName );

	// Make buffer large enough to hold the decoded data
	SetBufferSize( WMABUFFERSIZE );


    // Allocate space for WAVEFORMATEX structure, don't be skimpy
    pWavFormat = (WAVEFORMATEX*) new BYTE[ WAVFORMATSIZE ];

	// Open the decoder
	if ( FAILED( m_Decoder.Init( strFileName, pWavFormat ) ) )
	{
		delete [] pWavFormat;
		return FALSE;
	}

	// Get WFX info
//	if (m_decInfo.nBitRate>0)
//		SetBitRate(m_decInfo.nBitRate);

	SetChannels( pWavFormat->nChannels );
	SetSampleRate( pWavFormat->nSamplesPerSec );
	SetBitRate( m_Decoder.GetBitrate() );
	
	// delete WFX structure
	delete [] pWavFormat;

	// return Success
	return TRUE;
}




DWORD ISndStreamWMA::GetTotalTime()
{
	return m_Decoder.GetTotalTime();
}

DWORD ISndStreamWMA::GetCurrentTime()
{
	return m_Decoder.GetCurrentTime();
}

DWORD ISndStreamWMA::Read( PBYTE pbData, DWORD dwNumBytes )
{
	DWORD	dwWrite = 0;
	INT		nReturnSize = 0;
		
	ENTRY_TRACE( _T( "ISndStreamWMA::Read( %x, %d )" ), pbData, dwNumBytes );

	// Set decoder buffer
	m_Decoder.SetDataBuffer( pbData, dwNumBytes );

	SetPercent( (INT) ( (double) m_Decoder.GetCurrentTime() * 100.0 / m_Decoder.GetTotalTime() ) );

	// EOF reached ?
	if ( m_Decoder.IsEOF() )
	{
		int i;

		LTRACE( _T( "EOF REACHED" ) );

		nReturnSize = 0;

		// is there still something in the buffer ?
		for (i=0;i<5;i++)
		{
			LTRACE( _T( "ISndStreamWMA::Read, reached EOF, poll for last data %d" ), i );

			if ( !m_Decoder.IsBufferFilled() )
			{
				LTRACE( _T( "SLEEP" ) );
				::Sleep(20);
			}
			else
			{
				// Set proper return size (number of valid bytes in the buffer)
				nReturnSize=m_Decoder.GetDataSize();
				LTRACE( _T( "EOF BUFFER EMPTY SIZE " ), nReturnSize );
				break;
			}
		}
	}
	else
	{
		// Poll till data is available
		while ( ! m_Decoder.IsBufferFilled() )
		{
			if ( m_Decoder.IsEOF() )
			{
				break;
			}
			::Sleep( 10 );
		}

		if ( m_Decoder.IsEOF() )
		{
			LTRACE( _T( "ISndStreamWMA::Read(), EOF detected" ) );
			nReturnSize = 0;
		}
		else
		{
			LTRACE( _T( "ISndStreamWMA::Read(), Data available " ) );

			// Set proper return size (number of valid bytes in the buffer)
			nReturnSize = m_Decoder.GetDataSize();
		}
	}

	EXIT_TRACE( _T( "ISndStreamWMA::Read(), return value is %d" ), nReturnSize );

	return nReturnSize;
}


BOOL ISndStreamWMA::CloseStream()
{
	m_Decoder.Stop( );
	return TRUE;
}

LONG ISndStreamWMA::Seek( LONG lOff, UINT nFrom )
{
	LONG lSeekPosition=0;

	int nPercent=lOff;

	switch( nFrom )
	{
		case SEEK_PERCENT:
			if (nPercent<0 ) nPercent= 0;
			if (nPercent>99) nPercent=99;
			lSeekPosition = (LONG) ( lOff / 100.0 * m_Decoder.GetTotalTime() );
		break;
		case SEEK_TIME:
			lSeekPosition = lOff;
			break;
		default:
			ASSERT( FALSE );
		break;
	}

	// Do seek
	m_Decoder.Seek( lSeekPosition );

	// We're done, return seek position
	return lSeekPosition;
}


void ISndStreamWMA::Flush()
{
}

void ISndStreamWMA::Pause()
{
}

void ISndStreamWMA::InfoBox( HWND hWnd )
{
}
