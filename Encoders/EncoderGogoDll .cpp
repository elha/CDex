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


#include "StdAfx.h"
#include "EncoderGogoDll.h"
#include "EncoderGogoDllDlg.h"
#include "Encode.h"
#include "config.h"
#include "TaskInfo.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderGogoDll" ) );

int			g_nGgoBufSize = 0;
BYTE*		g_pGgoBuf = NULL;


// CONSTRUCTOR
CEncoderGogoDll::CEncoderGogoDll() : CEncoder()
{
	me_init			=NULL;
	me_setconf		=NULL;
	me_getconf		=NULL;
	me_detect		=NULL;
	me_procframe	=NULL;
	me_close		=NULL;
	me_end			=NULL;
	me_getver		=NULL;
	me_haveunit		=NULL;

	m_hDLL			=NULL;

	m_strStreamFileName = _T( "" );

	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding = TRUE;

	m_strEncoderPath = _T( "Gogo.dll" );
	m_strEncoderID = _T( "Gogo-no-coda DLL Encoder " );
	m_strExtension = _T( "mp3" );

	m_bChunkSupport = TRUE;

	m_bRiffWavSupport = TRUE;

	m_dResampleRatio = 1.0;

	m_nEncoderID = ENCODER_GGO;

	// get current settings from ini file
	LoadSettings();

	// Get encoder ID to get proper version info
	GetDLLVersionInfo();

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = TRUE;

}


// DESTRUCTOR
CEncoderGogoDll::~CEncoderGogoDll()
{
	if ( m_hDLL )
	{
		ASSERT( FALSE );
		FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}
	if ( m_fpOut )
	{
		ASSERT( FALSE );
		fclose( m_fpOut );
		m_fpOut = NULL;
	}
}



CDEX_ERR CEncoderGogoDll::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR	bReturn = CDEX_OK;
	CUString		strLang;

	LTRACE( _T( "CEncoderGogoDll::InitEncoder" ) );

	CEncoder::InitEncoder( pTask );

	if ( LoadDLL(  m_strEncoderPath, _W( "\\encoders\\libraries\\gogo\\Dll" ), m_hDLL, TRUE, FALSE ) )
	{
		me_init		= (ME_INIT)GetProcAddress( m_hDLL, "MPGE_initializeWork" );
		me_setconf	= (ME_SETCONF)GetProcAddress( m_hDLL, "MPGE_setConfigure" );
		me_getconf	= (ME_GETCONF)GetProcAddress( m_hDLL, "MPGE_getConfigure" );
		me_detect	= (ME_DETECT)GetProcAddress( m_hDLL, "MPGE_detectConfigure" );
		me_procframe= (ME_PROCFRAME)GetProcAddress( m_hDLL, "MPGE_processFrame" );
		me_close	= (ME_CLOSE)GetProcAddress( m_hDLL, "MPGE_closeCoder" );
		me_end		= (ME_END)GetProcAddress( m_hDLL, "MPGE_endCoder" );
		me_getver	= (ME_GETVER)GetProcAddress( m_hDLL, "MPGE_getVersion" );
		me_haveunit	= (ME_HAVEUNIT)GetProcAddress( m_hDLL, "MPGE_getUnitStates" );

		// check if functions are all present
		if (	me_init && 
				me_setconf && 
				me_getconf && 
				me_detect && 
				me_procframe &&
				me_close && 
				me_end && 
				me_getver && 
				me_haveunit )
		{
			MERET retval;
			retval = me_init();

			if (retval==ME_NOFPU)
			{
				LTRACE( _T( "CEncoderGogoDll::InitEncoder, Error:No FPU Present" ) );

				strLang = g_language.GetString( IDS_ENCODER_ERROR_GGODLLNEEDSFPU );

				CDexMessageBox( strLang );

				bReturn = CDEX_ERROR;
			}
		}
		else
		{
			CUString strMsg;
			strLang = g_language.GetString( IDS_LOADLIBRARY_FAILED );
			strMsg.Format(	strLang , (LPCWSTR)m_strEncoderPath );
			CDexMessageBox( strMsg );
			bReturn = CDEX_ERROR;
		}
	}
	else
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CEncoderGogoDll::InitEncoder, return value %d"), bReturn );

	return bReturn;
}


MERET CEncoderGogoDll::CallBack(void *buf, unsigned long nLength )
{
	memcpy( buf, g_pGgoBuf, nLength );

	return 0;
}



CDEX_ERR CEncoderGogoDll::OpenStream( CUString strOutFileName, DWORD dwSampleRate, WORD nChannels )
{
	CDEX_ERR bReturn = CDEX_OK;

    CUStringConvert strCnv;

	ENTRY_TRACE( _T( "CEncoderGogoDll::OpenStream( %s, %d %d" ),
				strCnv.ToT( strOutFileName + _W( "." ) + GetExtension() ),
				dwSampleRate,
				nChannels );

	int nVBR = ( GetCRC() >> 12 ) & 0x0F;

	CUString strFileName = strOutFileName + _W( ".mp3" );

	// setup number of input channels
	m_nInputChannels = nChannels;

	MCP_INPDEV_USERFUNC cbStruct;

	cbStruct.pUserFunc	= CallBack;
	cbStruct.nSize		= MC_INPDEV_MEMORY_NOSIZE;
	cbStruct.nBit		= 16;
	cbStruct.nChn		= nChannels;
	cbStruct.nFreq		= dwSampleRate;

	MERET retval;

	retval = me_setconf( MC_INPUTFILE	, MC_INPDEV_USERFUNC,(UPARAM)&cbStruct);
    retval = me_setconf( MC_OUTPUTFILE	, MC_OUTDEV_FILE,(UPARAM)strCnv.ToT( strFileName ) );

	retval = me_setconf( MC_ENCODEMODE		,(UPARAM)m_nMode, ( UPARAM )0 ); 
	retval = me_setconf( MC_BITRATE			,(UPARAM)m_nBitrate,( UPARAM ) 0 ); 

	retval = me_setconf( MC_INPFREQ			,(UPARAM)dwSampleRate, ( UPARAM ) 0 ); 

	// Setup properly for MPEG2 frequencies
	if ( ( 1 == GetVersion() ) && dwSampleRate > 32000 )
	{
		dwSampleRate /= 2;
	}

	retval = me_setconf( MC_OUTFREQ			,(UPARAM)dwSampleRate, ( UPARAM ) 0 ); 

	retval = me_setconf( MC_USEPSY			, (GetUserN1()&0x02)?TRUE:FALSE,( UPARAM ) 0 ); 
	retval = me_setconf( MC_USELPF16		, (GetUserN1()&0x04)?TRUE:FALSE,( UPARAM ) 0 ); 

	retval = me_setconf( MC_USEMMX			, FALSE, ( UPARAM ) 0 ); 
	retval = me_setconf( MC_USE3DNOW		, FALSE, ( UPARAM ) 0 ); 
	retval = me_setconf( MC_USEKNI			, FALSE, ( UPARAM ) 0 ); 
	retval = me_setconf( MC_USEE3DNOW		, FALSE, ( UPARAM ) 0 ); 


	if (nVBR>0)
	{
		retval = me_setconf( MC_VBR, (UPARAM)(nVBR-1), (UPARAM)0);
	}

	if ( (GetUserN1()&0x01) )
	{
		unsigned long nUnitCode;
		me_haveunit( &nUnitCode );

		#define tFPU (1<<0)
		#define tMMX (1<<1)
		#define t3DN (1<<2)
		#define tSSE (1<<3)
		#define tCMOV (1<<4
		#define tE3DN (1<<5)
		#define tEMMX (1<<6)
		#define tINTEL (1<<8)
		#define tAMD (1<<9)
		#define tCYRIX (1<<10)
		#define tIDT (1<<11)
		#define tUNKNOWN (1<<15)

		if (nUnitCode & tE3DN)
		{
			retval = me_setconf( MC_USEE3DNOW	, TRUE, ( UPARAM ) 0 ); 
		}
		else if (nUnitCode & t3DN)
		{
			retval = me_setconf( MC_USE3DNOW	, TRUE, ( UPARAM ) 0 ); 
		}
		else if (nUnitCode & tSSE)
		{
			retval = me_setconf( MC_USEKNI		, TRUE, ( UPARAM ) 0 ); 
		}
		else if (nUnitCode & tMMX)
		{
			retval = me_setconf( MC_USEMMX		, TRUE, ( UPARAM ) 0 ); 
		}
	}

	if ( MC_MODE_MONO == m_nMode )
	{
		retval = me_setconf( MC_MONO_PCM		, FALSE, ( UPARAM ) 0 ); 
	}

	// has to be called once before we can encode the stream
	retval = me_detect();

	// set input buffer size
	m_dwInBufferSize = 1152 * nChannels;

	g_nGgoBufSize = 0;
	g_pGgoBuf = NULL;

	if ( CDEX_OK == bReturn  )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	EXIT_TRACE( _T( "CEncoderGogoDll::OpenStream, return value %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderGogoDll::CloseStream()
{
	ENTRY_TRACE( _T( "CEncoderGogoDll::CloseStream" ) );

	MERET bResult = me_close();

	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CEncoderGogoDll::CloseStream" ) );

	return CDEX_OK;

}


CDEX_ERR CEncoderGogoDll::DeInitEncoder()
{
	MERET bResult=me_end();

	if ( m_hDLL )
	{
		FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}

	(void)CEncoder::DeInitEncoder();

	return CDEX_OK;
}



CDEX_ERR CEncoderGogoDll::EncodeChunk( PSHORT pbsInSamples, DWORD dwNumSamples )
{
	CDEX_ERR bReturn = CDEX_OK;
	DWORD nBytesToWrite = 0;

	LTRACE2( _T( "Entering CEncoderGogoDll::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

	g_nGgoBufSize = dwNumSamples * sizeof( SHORT );

	g_pGgoBuf = (BYTE*)m_psInputStream;

	// encode the frame
	if ( ME_NOERR != me_procframe() )
	{
		bReturn = CDEX_ERROR;
	}

	LTRACE2( _T( "Leaving CEncoderGogoDll::EncodeChunk(), return status %d" ),
					bReturn );

	return bReturn;

}

void CEncoderGogoDll::GetDLLVersionInfo()
{
	CUString strVersion;

    m_bAvailable = TRUE;

	if ( LoadDLL(	m_strEncoderPath, 
					_W( "\\encoders\\libraries\\gogo\\Dll" ),
					m_hDLL,
					FALSE,
					FALSE ) )
	{
		me_getver=(ME_GETVER)GetProcAddress(m_hDLL, "MPGE_getVersion" );
		if (me_getver!=NULL)
		{
			CUString			strLang;
			unsigned long	nVerCode;
			char			buf[ 260 ];

			me_getver( &nVerCode, buf );

			strLang = g_language.GetString( IDS_ENCODER_VERSIONSTRING );

			// Build file name from it
			strVersion.Format( _W( "(%s %s)" ), (LPCWSTR)strLang, (LPCWSTR)CUString( buf, CP_UTF8 ) );

		}

		// Free the library
		::FreeLibrary( m_hDLL );

		// Indicate that DLL has been freed
		m_hDLL = NULL;
	}
	else
	{
		strVersion = g_language.GetString( IDS_ENCODER_VERSIONNOTAVAILABLE );
		m_bAvailable = FALSE;

	}

	// Add version info the encoder ID
	m_strEncoderID += strVersion;
}


CEncoderDlg* CEncoderGogoDll::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg = new CEncoderGogoDllDlg;
	}
	return m_pSettingsDlg;
}

