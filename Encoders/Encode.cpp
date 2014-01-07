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
#include "Encode.h"
#include "AudioFile.h"
#include "aacenc.h"
#include "TaskInfo.h"
#include "EncoderFaacDll.h"
#include <math.h>

#include "EncoderDlg.h"
#include "EncoderApeDllDlg.h"
#include "EncoderBladeDllDlg.h"
#include "EncoderDosDlg.h"
#include "EncoderDosMPPDlg.h"
#include "EncoderDosVqfDlg.h"
#include "EncoderDosXingDlg.h"
#include "EncoderFaacDllDlg.h"
#include "EncoderGogoDllDlg.h"
#include "EncoderLameDllDlg.h"
#include "EncoderNttVqfDllDlg.h"
#include "EncoderVorbisDllDlg.h"
#include "EncoderWAVDlg.h"
#include "EncoderWinCodecMP3Dlg.h"
#include "EncoderWMA8DllDlg.h"

#include "ID3Tag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "Encode" ) );

CAcmCodec g_AcmCodec;

CDEX_ERR CEncoder::DosEncode(	const CUString& strInFileName,
								const CUString& strInFileExt, 
								const CUString& strOutFileName,
								const CUString& strOutDir,
								INT&  nPercent,
								BOOL& bAbort)
{
	ASSERT(FALSE);
	return CDEX_ERROR;
}


// CEncoder CONSTRUCTOR
CEncoder::CEncoder()
{
	ENTRY_TRACE( _T( "CEncoder::CEncoder" ) );

	m_pbOutputStream=NULL;
	m_psInputStream=NULL;

	m_fpOut = NULL;
	m_dwOutBufferSize=0;
	m_dwInBufferSize=0;

	m_dwId3PadSize = 0;

	m_pSettingsDlg = NULL;


	m_bOnTheFlyEncoding=FALSE;
	m_strEncoderID = _W( "Not Assigned" );
	m_strEncoderPath= _W( "" );
	m_strUser1= _W( "" );
	m_strUser2= _W( "" );

	m_nUser1=0;
	m_nUser2=0;

	m_nBitrate = 128;
	m_nMaxBitrate = 224;
	m_nMinBitrate = 96;
	m_nVersion=0;
	m_nSampleRate = 44100;
	m_nOutSampleRate = 44100;
	m_nInputChannels = 2;
	m_nMode=BE_MP3_MODE_STEREO;

	m_bPrivate=FALSE;
	m_bCopyRight=FALSE;
	m_bOriginal=FALSE;
	m_bCRC=FALSE;
	m_nEncoderID=-1;
	m_bChunkSupport = FALSE;
	m_bRiffWavSupport = FALSE;
	m_bDownMixToMono = FALSE;
	m_bUpMixToStereo = FALSE;
	m_bCanWriteTagV1 = FALSE;
	m_bCanWriteTagV2 = FALSE;

	m_bAvailable = TRUE;

	m_dNormalizationFactor = 1.0;

	m_pTask = NULL;

	m_dResampleRatio = 1.0;

	EXIT_TRACE( _T( "CEncoder::CEncoder" ) );
}

// CEncoder CONSTRUCTOR
CEncoder::~CEncoder()
{
	ENTRY_TRACE( _T( "CEncoder::~CEncoder" ) );

	CEncoder::DeInitEncoder();

	delete m_pSettingsDlg;

	EXIT_TRACE( _T( "CEncoder::~CEncoder" ) );
}


DWORD CEncoder::DownMixToMono( SHORT* psData,DWORD dwNumSamples )
{
	DWORD dwSample;
	for (dwSample=0;dwSample<dwNumSamples/2;dwSample++)
	{
		psData[dwSample]=psData[ 2 * dwSample ] / 2 + psData[ 2 * dwSample + 1 ] / 2;
	}

	return dwNumSamples / 2;
}


void CEncoder::Normalize( PSHORT psData,DWORD dwNumSamples )
{
	DWORD dwSample = 0;

	if ( 1.0 != m_dNormalizationFactor )
	{
		for ( dwSample=0; dwSample< dwNumSamples; dwSample++ )
		{
			m_psInputStream[ dwSample ]= (SHORT)( ( double) psData[ dwSample] * m_dNormalizationFactor + 0.5 );
		}
	}
	else
	{
		for ( dwSample=0; dwSample< dwNumSamples; dwSample++ )
		{
			m_psInputStream[ dwSample ]= psData[ dwSample];
		}
	}
}


DWORD CEncoder::UpMixToStereo( SHORT* psData, SHORT* psOutData, DWORD dwNumSamples )
{
	int dwSample;
	for ( dwSample = dwNumSamples - 1; dwSample >= 0; dwSample-- )
	{
		psOutData[ 2 * dwSample + 0 ] = psData[ dwSample ];
		psOutData[ 2 * dwSample + 1 ] = psData[ dwSample ];
	}
	return dwNumSamples * 2;
}

DWORD CEncoder::ReSample( SHORT* psData,DWORD dwNumSamples )
{
	DWORD dwSample = 0;

	if ( m_dResampleRatio == 2.0 )
	{
		dwNumSamples /= 2; 

		if ( IsOutputStereo() )
		{
			for ( dwSample = 0; dwSample < dwNumSamples; dwSample += 2 )
			{
				psData[ dwSample + 0 ] = psData[ 2 * dwSample + 0 ];
				psData[ dwSample + 1 ] = psData[ 2 * dwSample + 1 ];
			}
		}
		else
		{
			for ( dwSample=0; dwSample < dwNumSamples; dwSample++ )
			{
				psData[ dwSample ] = psData[ 2 * dwSample ];
			}
		}
	}
	else if ( m_dResampleRatio == 4.0 )
	{
		dwNumSamples /= 4; 

		if ( IsOutputStereo() )
		{
			for ( dwSample = 0; dwSample < dwNumSamples; dwSample += 4 )
			{
				psData[ dwSample + 0 ] = psData[ 4 * dwSample + 0 ];
				psData[ dwSample + 1 ] = psData[ 4 * dwSample + 1 ];
				psData[ dwSample + 2 ] = psData[ 4 * dwSample + 2 ];
				psData[ dwSample + 3 ] = psData[ 4 * dwSample + 3 ];
			}
		}
		else
		{
			for ( dwSample=0; dwSample < dwNumSamples; dwSample++ )
			{
				psData[ dwSample ] = psData[ 4 * dwSample ];
			}
		}
	}


	return dwNumSamples;
}

DWORD CEncoder::ProcessData( SHORT* pbsInSamples, DWORD dwNumSamples )
{
	DWORD	dwSample = 0;
	PSHORT 	psData = pbsInSamples;
	BOOL	bSilence = FALSE;
	PSHORT	pFifoBuffer = NULL;


	// Downmix to mono?
	if ( m_bDownMixToMono )
	{
		dwNumSamples = DownMixToMono( pbsInSamples, dwNumSamples );
	} else if ( m_bUpMixToStereo )
	{
		dwNumSamples = UpMixToStereo( pbsInSamples, m_psInputStream, dwNumSamples );
		psData = m_psInputStream;
	}
	
	dwNumSamples = ReSample( psData, dwNumSamples );

	// normalize samples, result written to m_psInputStream
	Normalize( psData, dwNumSamples );


	return dwNumSamples;
}


CDEX_ERR CEncoder::InitInputStream()
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoder::InitInputStream m_dwInBufferSize = %d" ), m_dwInBufferSize );

	// allocate input stream
	m_psInputStream = new SHORT[ m_dwInBufferSize ];

	if ( m_bUpMixToStereo )
	{
		m_dwInBufferSize /= 2;
	}
	if ( NULL == m_psInputStream )
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CEncoder::InitInputStream m_dwInBufferSize = %d, return value %d" ), m_dwInBufferSize, bReturn );

	return bReturn;
}

/* resampling via FIR filter, blackman window */
inline static FLOAT blackman(FLOAT x,FLOAT fcn,int l)
{
  /* This algorithm from:
SIGNAL PROCESSING ALGORITHMS IN FORTRAN AND C
S.D. Stearns and R.A. David, Prentice-Hall, 1992
  */
  const double PI = 3.1415926535;

  FLOAT bkwn,x2;
  FLOAT wcn = (FLOAT)(PI * fcn);
  
  x /= l;
  if (x<0) x=0;
  if (x>1) x=1;
  x2 = x - .5f;

  bkwn = (FLOAT)( 0.42 - 0.5*cos(2*x*PI)  + 0.08*cos(4*x*PI) );
  if (fabs(x2)<1e-9) 
  {
	  return (FLOAT)(wcn/PI);
  }
  else 
  {
    return  (FLOAT)(  bkwn*sin(l*wcn*x2)  / (PI*l*x2)  );
  }


}


# if 0
int fill_buffer_resample(
       lame_global_flags *gfp,
       sample_t *outbuf,
       int desired_len,
       sample_t *inbuf,
       int len,
       int *num_used,
       int ch) 
{

  
  lame_internal_flags *gfc=gfp->internal_flags;
  int BLACKSIZE;
  FLOAT8 offset,xvalue;
  int i,j=0,k;
  int filter_l;
  FLOAT8 fcn,intratio;
  FLOAT *inbuf_old;
  int bpc;   /* number of convolution functions to pre-compute */
  bpc = gfp->out_samplerate/gcd(gfp->out_samplerate,gfp->in_samplerate);
  if (bpc>BPC) bpc = BPC;

  intratio=( fabs(gfc->resample_ratio - floor(.5+gfc->resample_ratio)) < .0001 );
  fcn = 1.00/gfc->resample_ratio;
  if (fcn>1.00) fcn=1.00;
  filter_l = gfp->quality < 7 ? 31 : 7;
  filter_l = 31;
  if (0==filter_l % 2 ) --filter_l;/* must be odd */
  filter_l += intratio;            /* unless resample_ratio=int, it must be even */


  BLACKSIZE = filter_l+1;  /* size of data needed for FIR */
  
  if ( gfc->fill_buffer_resample_init == 0 ) {
    gfc->inbuf_old[0]=calloc(BLACKSIZE,sizeof(gfc->inbuf_old[0][0]));
    gfc->inbuf_old[1]=calloc(BLACKSIZE,sizeof(gfc->inbuf_old[0][0]));
    for (i=0; i<=2*bpc; ++i)
      gfc->blackfilt[i]=calloc(BLACKSIZE,sizeof(gfc->blackfilt[0][0]));

    gfc->itime[0]=0;
    gfc->itime[1]=0;

    /* precompute blackman filter coefficients */
    for ( j = 0; j <= 2*bpc; j++ ) {
        FLOAT8 sum = 0.; 
        offset = (j-bpc) / (2.*bpc);
        for ( i = 0; i <= filter_l; i++ ) 
            sum += 
	    gfc->blackfilt[j][i]  = blackman(i-offset,fcn,filter_l);
	for ( i = 0; i <= filter_l; i++ ) 
	  gfc->blackfilt[j][i] /= sum;
    }
    gfc->fill_buffer_resample_init = 1;
  }
  
  inbuf_old=gfc->inbuf_old[ch];

  /* time of j'th element in inbuf = itime + j/ifreq; */
  /* time of k'th element in outbuf   =  j/ofreq */
  for (k=0;k<desired_len;k++) {
    FLOAT8 time0;
    int joff;

    time0 = k*gfc->resample_ratio;       /* time of k'th output sample */
    j = floor( time0 -gfc->itime[ch]  );

    /* check if we need more input data */
    if ((filter_l + j - filter_l/2) >= len) break;

    /* blackman filter.  by default, window centered at j+.5(filter_l%2) */
    /* but we want a window centered at time0.   */
    offset = ( time0 -gfc->itime[ch] - (j + .5*(filter_l%2)));
    assert(fabs(offset)<=.501);

    /* find the closest precomputed window for this offset: */
    joff = floor((offset*2*bpc) + bpc +.5);

    xvalue = 0.;
    for (i=0 ; i<=filter_l ; ++i) {
      int j2 = i+j-filter_l/2;
      int y;
      assert(j2<len);
      assert(j2+BLACKSIZE >= 0);
      y = (j2<0) ? inbuf_old[BLACKSIZE+j2] : inbuf[j2];
#define PRECOMPUTE
#ifdef PRECOMPUTE
      xvalue += y*gfc->blackfilt[joff][i];
#else
      xvalue += y*blackman(i-offset,fcn,filter_l);  /* very slow! */
#endif
    }
    outbuf[k]=xvalue;
  }

  
  /* k = number of samples added to outbuf */
  /* last k sample used data from [j-filter_l/2,j+filter_l-filter_l/2]  */

  /* how many samples of input data were used:  */
  *num_used = Min(len,filter_l+j-filter_l/2);

  /* adjust our input time counter.  Incriment by the number of samples used,
   * then normalize so that next output sample is at time 0, next
   * input buffer is at time itime[ch] */
  gfc->itime[ch] += *num_used - k*gfc->resample_ratio;

  /* save the last BLACKSIZE samples into the inbuf_old buffer */
  if (*num_used >= BLACKSIZE) {
      for (i=0;i<BLACKSIZE;i++)
	  inbuf_old[i]=inbuf[*num_used + i -BLACKSIZE];
  }else{
      /* shift in *num_used samples into inbuf_old  */
       int n_shift = BLACKSIZE-*num_used;  /* number of samples to shift */

       /* shift n_shift samples by *num_used, to make room for the
	* num_used new samples */
       for (i=0; i<n_shift; ++i ) 
	   inbuf_old[i] = inbuf_old[i+ *num_used];

       /* shift in the *num_used samples */
       for (j=0; i<BLACKSIZE; ++i, ++j ) 
	   inbuf_old[i] = inbuf[j];

       assert(j==*num_used);
  }
  return k;  /* return the number samples created at the new samplerate */
}


#endif /* ndef KLEMM_44 */


CDEX_ERR CEncoder::InitEncoder( CTaskInfo* pTask )
{
	ENTRY_TRACE( _T( "CEncoder::InitEncoder" ) );

	ASSERT( pTask );

	m_pTask = pTask;

	pTask->SetOutFullFileName(	pTask->GetOutDir( ) + 
								pTask->GetFileNameNoExt( ) + _W( "." ) + GetExtension() );

	EXIT_TRACE( _T( "CEncoder::InitEncoder" ) );

	return CDEX_OK;
}


CDEX_ERR CEncoder::CloseStream( )
{
	ENTRY_TRACE( _T( "CEncoder::CloseStream" ) );

	if ( m_fpOut )
	{
		fclose( m_fpOut );
		m_fpOut = NULL;
	}

	EXIT_TRACE( _T( "CEncoder::CloseStream" ) );

	return CDEX_OK;
}

CDEX_ERR CEncoder::DeInitEncoder( )
{
	ENTRY_TRACE( _T( "CEncoder::DeInitEncoder" ) );

	CEncoder::CloseStream();

	delete [] m_pbOutputStream;
	m_pbOutputStream = NULL;

	delete [] m_psInputStream;
	m_psInputStream = NULL;

	EXIT_TRACE( _T( "CEncoder::DeInitEncoder" ) );

	return CDEX_OK;
}


void CDLLEncoder::GetDLLVersionInfo()
{
	CUString strVersion;

	m_bAvailable = FALSE;

	if ( FALSE == LoadDLL( m_strEncoderPath, _W( "" ), m_hDLL, FALSE, FALSE ) )
	{
		if ( FALSE == LoadDLL( m_strEncoderPath, _W( "\\encoders\\libraries\\lame\\Dll" ), m_hDLL, FALSE, FALSE ) )
		{
			LoadDLL( m_strEncoderPath, _W( "" ), m_hDLL, FALSE, TRUE );
		}		
	}

	if ( NULL != m_hDLL)
	{
		CUString strLang;

		BE_VERSION myVersion={'\0',};

		m_Version = (BEVERSION)GetProcAddress(m_hDLL, TEXT_BEVERSION);

		if (m_Version!=NULL)
		{
			CUString strLang;
			CUString strLang1;

			// Get version info from DLL
			m_Version( &myVersion );

			strLang = g_language.GetString( IDS_ENCODER_VERSIONSTRING );

			strLang1 = g_language.GetString( IDS_ENCODER_ENGINESTRING );

			// Build file name from it
			strVersion.Format(	_W( "(%s %d.%02d, %s %d.%02d) " ),
								(LPCWSTR)strLang,
								myVersion.byDLLMajorVersion,
								myVersion.byDLLMinorVersion,
								(LPCWSTR)strLang1,
								myVersion.byMajorVersion,
								myVersion.byMinorVersion );


			if ( ( (m_nEncoderID == ENCODER_INT) ) ||
				  ( (m_nEncoderID == ENCODER_MP2) ) )
			{
				CUString strLang;
				CUString strLang1;
				CUString strRevLevel( _W( "" ) );
				CUString	strMMX;

				if (myVersion.byAlphaLevel)
				{
					strLang = g_language.GetString( IDS_ENCODER_VERSIONALPHA );

					strRevLevel.Format( strLang, myVersion.byAlphaLevel );
				}
				if (myVersion.byBetaLevel)
				{
					strLang = g_language.GetString( IDS_ENCODER_VERSIONBETA );

					strRevLevel.Format( strLang, myVersion.byBetaLevel );
				}

				strLang = g_language.GetString( IDS_ENCODER_VERSIONSTRING );

				strLang1 = g_language.GetString( IDS_ENCODER_ENGINESTRING );

				strVersion.Format(	_W( "(%s %d.%02d, %s %d.%02d%s%s) "),
									(LPCWSTR)strLang,
									myVersion.byDLLMajorVersion,
									myVersion.byDLLMinorVersion,
									(LPCWSTR)strLang1,
									myVersion.byMajorVersion,
									myVersion.byMinorVersion,
									(LPCWSTR)strRevLevel,
									myVersion.byMMXEnabled ? _W( " MMX" ): _W( "" ) );

			}

			m_bAvailable = TRUE;
		}

		// Free the library
		FreeLibrary( m_hDLL );

		// Indicate that DLL has been freed
		m_hDLL = NULL;
	}
	else
	{
		strVersion = g_language.GetString( IDS_ENCODER_VERSIONNOTAVAILABLE );
	}


	m_hDLL=NULL;

	m_strEncoderID += strVersion;
}

CUString CEncoder::GetIniKey()
{
	CUString strRet;
	strRet.Format( _W( "EncoderSetting_%02d" ), m_nEncoderID );
	return strRet;
}


// CEncoder CONSTRUCTOR
void CEncoder::SaveSettings()
{
	CUString strKey = GetIniKey();

	ENTRY_TRACE( _T( "CEncoder::SaveSettings()" ) );

	g_config.GetIni().SetValue( strKey, _T( "nMode" ), m_nMode );
	g_config.GetIni().SetValue( strKey, _T( "nBitRate" ), m_nBitrate );
	g_config.GetIni().SetValue( strKey, _T( "nMaxBitRate" ), m_nMaxBitrate );
	g_config.GetIni().SetValue( strKey, _T( "nMinBitRate" ), m_nMinBitrate );
	g_config.GetIni().SetValue( strKey, _T( "nVersion" ), m_nVersion );
	g_config.GetIni().SetValue( strKey, _T( "bOnTheFlyEncoding" ), m_bOnTheFlyEncoding );
	g_config.GetIni().SetValue( strKey, _T( "bPrivate" ), m_bPrivate );
	g_config.GetIni().SetValue( strKey, _T( "bCRC" ),m_bCRC );
	g_config.GetIni().SetValue( strKey, _T( "bCopyRight" ), m_bCopyRight );
	g_config.GetIni().SetValue( strKey, _T( "bOriginal" ), m_bOriginal );
	g_config.GetIni().SetValue( strKey, _T( "strUser1"), m_strUser1 );
	g_config.GetIni().SetValue( strKey, _T( "strUser2"), m_strUser2 );
	
	g_config.GetIni().SetValue( strKey, _T( "nUser1" ), m_nUser1 );
	g_config.GetIni().SetValue( strKey, _T( "nUser2" ), m_nUser2 );

	g_config.GetIni().SetValue( strKey, _T( "nSampleRate" ), m_nSampleRate );
	g_config.GetIni().SetValue( strKey, _T( "nOutSampleRate" ), m_nOutSampleRate );
	g_config.GetIni().SetValue( strKey, _T( "strExtention" ), m_strExtension );

	EXIT_TRACE( _T( "CEncoder::SaveSettings()" ) );

}

void CEncoder::LoadSettings()
{
	CUString strKey = GetIniKey();

	ENTRY_TRACE( _T( "CEncoder::LoadSettings()" ) );

	m_nMode=			g_config.GetIni().GetValue( strKey, _T( "nMode" ), m_nMode );
	m_nBitrate=			g_config.GetIni().GetValue( strKey, _T( "nBitRate" ), m_nBitrate );
	m_nMaxBitrate=		g_config.GetIni().GetValue( strKey, _T( "nMaxBitRate" ), m_nMaxBitrate );
	m_nMinBitrate=		g_config.GetIni().GetValue( strKey, _T( "nMinBitRate" ), m_nMinBitrate );
	m_nVersion=			g_config.GetIni().GetValue( strKey, _T( "nVersion" ), m_nVersion );
	m_bOnTheFlyEncoding=g_config.GetIni().GetValue( strKey, _T( "bOnTheFlyEncoding" ), m_bOnTheFlyEncoding );
	m_bPrivate=			g_config.GetIni().GetValue( strKey, _T( "bPrivate" ), m_bPrivate );
	m_bCRC=				g_config.GetIni().GetValue( strKey, _T( "bCRC" ), m_bCRC );
	m_bCopyRight=		g_config.GetIni().GetValue( strKey, _T( "bCopyRight" ), m_bCopyRight );
	m_bOriginal=		g_config.GetIni().GetValue( strKey, _T( "bOriginal" ), m_bOriginal );
	m_strUser1=			g_config.GetIni().GetValue( strKey, _T( "strUser1" ), m_strUser1 );
	m_strUser2=			g_config.GetIni().GetValue( strKey, _T( "strUser2" ), m_strUser2 );
	m_nUser1=			g_config.GetIni().GetValue( strKey, _T( "nUser1" ), m_nUser1 );
	m_nUser2=			g_config.GetIni().GetValue( strKey, _T( "nUser2" ), m_nUser2 );
	m_nSampleRate=		g_config.GetIni().GetValue( strKey, _T( "nSampleRate" ), m_nSampleRate );
	m_strExtension=		g_config.GetIni().GetValue( strKey, _T( "strExtention" ), m_strExtension );
	m_nOutSampleRate=	g_config.GetIni().GetValue( strKey, _T( "nOutSampleRate" ), m_nOutSampleRate );

	EXIT_TRACE( _T( "CEncoder::LoadSettings()" ) );
}



// CDLLEncoder CONSTRUCTOR
CDLLEncoder::CDLLEncoder() : CEncoder()
{
	m_InitStream	=NULL;
	m_EncodeChunk	=NULL;
	m_DeinitStream	=NULL;
	m_CloseStream	=NULL;
	m_Version		=NULL;
	m_WriteVbrTag	=NULL;
	m_hDLL			=NULL;
	m_hbeStream		=0;

	SetLastCDexErrorString( CUString( _T("") ) );
	
	m_strStreamFileName = _T( "" );

	// Clear Config structure
	memset(&m_beConfig,0x00,sizeof(BE_CONFIG));

	int nTest = sizeof( BE_CONFIG );

	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding = TRUE;

	m_strEncoderPath = _T( "lame_enc.dll" );

	m_strExtension = _T( "mp3" );

	// All DLL derivates support chunk support
	m_bChunkSupport = TRUE;

	m_bRiffWavSupport = TRUE;

	m_dResampleRatio = 1.0;

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = TRUE;
}


// CDLLEncoder DESTRUCTOR
CDLLEncoder::~CDLLEncoder()
{
	if ( m_hDLL )
	{
		ASSERT( FALSE );
		FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}
	CEncoder::DeInitEncoder();
}



CDEX_ERR CDLLEncoder::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR bReturn = CDEX_OK;

	CUString strLang;

	ENTRY_TRACE( _T( "CDLLEncoder::InitEncoder" ) );

	CEncoder::InitEncoder( pTask );

	// Try to open the DLL
	CUString strTest = g_config.GetAppPath();

    CUStringConvert strCnv;

    LTRACE( _T( "CDLLEncoder::InitEncoder Trying to load :%s:" ), strCnv.ToT( g_config.GetAppPath() + _W( "\\" ) + m_strEncoderPath ) );

	if ( FALSE == LoadDLL(  m_strEncoderPath, _W( "\\Encoders\\Libraries\\lame\\Dll" ), m_hDLL, FALSE, FALSE ) )
	{
		if ( FALSE == LoadDLL( m_strEncoderPath, _W( "" ), m_hDLL, TRUE, TRUE ) )
		{
			LTRACE( _T("CDLLEncoder::InitEncoder Failed to load %s" ) , m_strEncoderPath);
			return CDEX_ERROR;
		}
	}

	// Get the DLL functions
	m_InitStream	= (BEINITSTREAM)	GetProcAddress(m_hDLL, TEXT_BEINITSTREAM);
	m_EncodeChunk	= (BEENCODECHUNK)	GetProcAddress(m_hDLL, TEXT_BEENCODECHUNK);
	m_DeinitStream	= (BEDEINITSTREAM)	GetProcAddress(m_hDLL, TEXT_BEDEINITSTREAM);
	m_CloseStream	= (BECLOSESTREAM)	GetProcAddress(m_hDLL, TEXT_BECLOSESTREAM);
	m_Version		= (BEVERSION)		GetProcAddress(m_hDLL, TEXT_BEVERSION);
	m_WriteVbrTag	= (BEWRITEVBRHEADER) GetProcAddress(m_hDLL,TEXT_BEWRITEVBRHEADER);

	BE_VERSION beVersion;
	m_Version( &beVersion );

	// Reduce sample rate by a factor of two (if applicable)
	if ( !( m_InitStream && m_EncodeChunk && m_DeinitStream && m_CloseStream ) )
	{
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CDLLEncoder::InitEncoder, return value %d" ), bReturn );

	return bReturn;

}


CDEX_ERR CDLLEncoder::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	CDEX_ERR bReturn = CDEX_OK;

	CUString strLang;

    CUStringConvert strCnv;

	ENTRY_TRACE( _T( "CDLLEncoder::OpenStream, file name is: %s:" ),
                    strCnv.ToT( strOutFileName + _W( "." ) + GetExtension() ) );

	DWORD dwInSampleRate=dwSampleRate;

	m_strStreamFileName = strOutFileName;


	// clear all config settings
	memset( &m_beConfig, 0, sizeof(m_beConfig) );

	// set the number of input channels
	m_nInputChannels = nChannels;

	// Divide sample rate by two for MPEG2
	if ( ( GetVersion() >=1 ) && ( dwInSampleRate >= 32000 ) )
	{
		m_dResampleRatio=2.0;
		dwSampleRate = dwInSampleRate/2;
	}

	// mask mode, just to be sure (due to an old hack)
	m_nMode&=0x0F;

	// Do we have to downmix/upmix ?
	if ( BE_MP3_MODE_MONO == m_nMode )
	{
		if ( 2 == nChannels )
			m_bDownMixToMono=TRUE;
		else
			m_bDownMixToMono=FALSE;
	}
	else 
	{
		if ( 1 == nChannels )
			m_bUpMixToStereo=TRUE;
		else
			m_bUpMixToStereo=FALSE;
	}

	// Use the old blade interface or new Lame interface?
	if ( m_nEncoderID != ENCODER_INT )
	{
		m_beConfig.dwConfig = BE_CONFIG_MP3;
		m_beConfig.format.mp3.dwSampleRate	= dwSampleRate;
		m_beConfig.format.mp3.byMode=m_nMode;
		m_beConfig.format.mp3.wBitrate		= m_nBitrate;
		m_beConfig.format.mp3.bCopyright	= m_bCopyRight;
		m_beConfig.format.mp3.bCRC			= m_bCRC;
		m_beConfig.format.mp3.bOriginal		= m_bOriginal;
		m_beConfig.format.mp3.bPrivate		= m_bPrivate;
	}

    m_fpOut = CID3Tag::SaveInitialV2Tag( strOutFileName + _W( "." ) + GetExtension(), GetId3V2PadSize() );
    // m_fpOut = CDexOpenFile( strOutFileName + _W( "." ) + GetExtension(), _W( "wb+" ) );
	
    if ( NULL == m_fpOut )
	{
		CUString strErr;

		strLang = g_language.GetString( IDS_ENCODER_ERROR_COULDNOTOPENFILE );

		strErr.Format( strLang, (LPCWSTR)CUString( strOutFileName + _W( "." ) + GetExtension() ) );

		CDexMessageBox( strErr );

		bReturn  = CDEX_FILEOPEN_ERROR;
	}


	if ( CDEX_OK == bReturn )
	{
		int nReturn = m_InitStream(&m_beConfig, &m_dwInBufferSize,&m_dwOutBufferSize,&m_hbeStream);

		switch ( nReturn )
		{
			case CDEX_OK:
				// do nothing
			break;
			case -1:
				strLang = g_language.GetString( IDS_ENCODER_ERROR_SAMPLERATEBITRATEMISMATCH );
				CDexMessageBox( strLang );
				bReturn = CDEX_ERROR;
			break;
			case -2:
				strLang = g_language.GetString( IDS_ENCODER_ERROR_INVALIDINPUTSTREAM );
				CDexMessageBox( strLang );
				bReturn = CDEX_ERROR;
			break;
			default:
				strLang = g_language.GetString( IDS_ENCODER_ERROR_INVALIDINPUTSTREAM );
				CDexMessageBox( strLang );
				bReturn = CDEX_ERROR;
			break;

		}
	}

	if ( CDEX_OK == bReturn )
	{

		if ( (GetVersion() >= 1) && (dwInSampleRate>=32000) )
		{
			// Add multiplcation factor for resampling
			m_dwInBufferSize *= 2;
		}

		if (m_bDownMixToMono)
		{
			m_dwInBufferSize *= 2;
		}


		// Allocate Output Buffer size
		m_pbOutputStream=new BYTE[ m_dwOutBufferSize ];

		// Only expect halve the number of samples
		// in case we have to upsample
		if ( m_bUpMixToStereo )
		{
			// Allocate upsample Buffer size
			m_psInputStream = new SHORT[ m_dwInBufferSize ];

			m_dwInBufferSize /= 2;

		}
	}

	// Initialize the input stream
	bReturn = InitInputStream();


	EXIT_TRACE( _T( "CDLLEncoder::OpenStream, return value %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CDLLEncoder::CloseStream()
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CDLLEncoder::CloseStream" ) );

	DWORD nBytesToWrite = 0;

	// Get the last bytes out of the encoder

	if ( m_pbOutputStream )
	{
		if(m_DeinitStream(m_hbeStream, m_pbOutputStream, &nBytesToWrite) != CDEX_OK)	
		{
			ASSERT(FALSE);
			m_CloseStream(m_hbeStream);
			bReturn = CDEX_ERROR;
		}

		// Did we receive some output bytes
		if ( nBytesToWrite > 0 )
		{
			// Guess so, write it to the output file
			if ( 1 != fwrite(m_pbOutputStream,nBytesToWrite,1,m_fpOut) )
			{
				ASSERT(FALSE);
				bReturn = CDEX_FILEWRITE_ERROR;
			}
		}
	}

	if ( m_hbeStream )
	{
		// Close the Stream
		m_CloseStream( m_hbeStream );
	}

	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CDLLEncoder::CloseStream, return value %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CDLLEncoder::DeInitEncoder()
{
	ENTRY_TRACE( _T( "CDLLEncoder::DeInitEncoder()" ) );

	// Check if we have to write the VBR header
	if ( m_WriteVbrTag )
	{
		LPCSTR lpTemp = GetDosFileName( m_strStreamFileName + _W( "." ) + GetExtension() );
		BOOL bResult;
		if ( m_WriteVbrTag )
		{
			bResult = m_WriteVbrTag( lpTemp );
		}
	}

	// Release DLL if necessary
	if (m_hDLL)
	{
		FreeLibrary( m_hDLL );
		m_hDLL = NULL;
	}

	CEncoder::DeInitEncoder();

	EXIT_TRACE( _T( "CDLLEncoder::DeInitEncoder()" ) );

	return CDEX_OK;
}



CDEX_ERR CDLLEncoder::EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples)
{
	CDEX_ERR bReturn = CDEX_OK;

	DWORD nBytesToWrite = 0;

	LTRACE2( _T( "Entering CDLLEncoder::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	// Sanity check
	ASSERT( pbsInSamples );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );

	// Convert the input samples, as long as the user did not abort the thread
	if ( m_EncodeChunk( m_hbeStream,
						dwNumSamples,
						m_psInputStream,
						m_pbOutputStream,
						&nBytesToWrite) == CDEX_ERROR )
	{
		ASSERT(FALSE);
		return CDEX_ERROR;
	}

	// Did we receive some output bytes
	if ( nBytesToWrite > 0 )
	{
		// Guess so, write it to the output file
		if ( 1 != fwrite( m_pbOutputStream, nBytesToWrite, 1, m_fpOut ) )
		{
			ASSERT( FALSE );
			bReturn = CDEX_FILEWRITE_ERROR;
		}
	}

	return bReturn;
}
















