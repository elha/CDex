/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2006 Albert L. Faber
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
#include "EncoderVorbisDll.h"
#include "EncoderVorbisDllDlg.h"
#include "Encode.h"
#include "config.h"
#include "utf8.h"
#include "TaskInfo.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


INITTRACE( _T( "EncoderVorbisDll" ) );

// CONSTRUCTOR
CEncoderVorbisDll::CEncoderVorbisDll() : 
	CEncoder(),
	m_hLibVorbisDLL( NULL ),
	m_hOggDLL( NULL )

{
	ENTRY_TRACE( _T( "CEncoderVorbisDll::CEncoderVorbisDll()" ) );

	m_strStreamFileName = _T( "" );
	
	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding = TRUE;

	m_strEncoderPath = _T( "libvorbis.dll" );
	m_strEncoderID = _T( "Ogg Vorbis DLL Encoder " );
	m_strExtension= _T( "ogg" );

	// All DLL derivates support chunk support
	m_bChunkSupport = TRUE;

	m_bRiffWavSupport = FALSE;

	m_dResampleRatio = 1.0;

	m_nEncoderID = ENCODER_OGG;

	SetMinBitrate( 0 );
	SetBitrate( 0 );
	SetMaxBitrate( 0 );

	SetUserN1( MAKELPARAM( 500, 1 ) );

	// get current settings from ini file
	LoadSettings();

	// Get encoder ID to get proper version info
	GetDLLVersionInfo();

	// clear end of stream flag
	m_eos = false;

	(void)memset( &m_os, 0, sizeof( m_os ) );
	(void)memset( &m_og, 0, sizeof( m_og ) );
	(void)memset( &m_op, 0, sizeof( m_op ) );
	(void)memset( &m_vi, 0, sizeof( m_vi ) );
	(void)memset( &m_vc, 0, sizeof( m_vc ) );
	(void)memset( &m_vd, 0, sizeof( m_vd ) );
	(void)memset( &m_vb, 0, sizeof( m_vb ) );

	EXIT_TRACE( _T( "CEncoderVorbisDll::CEncoderVorbisDll()" ) );

}


// DESTRUCTOR
CEncoderVorbisDll::~CEncoderVorbisDll()
{
	ENTRY_TRACE( _T( "CEncoderVorbisDll::~CEncoderVorbisDll()" ) );

	CloseDlls();

	if ( NULL != m_fpOut )
	{
		ASSERT( FALSE );
		fclose( m_fpOut );
		m_fpOut = NULL;
	}
	EXIT_TRACE( _T( "CEncoderVorbisDll::~CEncoderVorbisDll()" ) );
}

CDEX_ERR CEncoderVorbisDll::OpenDlls( BOOL bWarning )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T("CEncoderVorbisDll::OpenDlls()" ) );

    if ( FALSE == LoadDLL(  m_strEncoderPath, _W( "\\encoders\\libraries\\libvorbis\\Dll" ), m_hLibVorbisDLL, bWarning, FALSE ) )
	{
		// use standard DLLs
		if ( FALSE == LoadDLL(  _W( "ogg.dll" ), _W( "\\encoders\\libraries\\libvorbis\\Dll" ), m_hOggDLL, bWarning, FALSE ) )
		{
			LTRACE( _T( "CEncoderVorbisDll::InitEncoder Failed to load %s" ), _T( "ogg.dll" ) );
			bReturn = CDEX_ERROR;
		} else
		if ( FALSE == LoadDLL(  _W( "vorbis.dll" ) , _W( "\\encoders\\libraries\\libvorbis\\Dll" ), m_hLibVorbisDLL, bWarning, FALSE ) )
		{
			LTRACE( _T( "CEncoderVorbisDll::InitEncoder Failed to load %s" ), m_strEncoderPath );
			bReturn = CDEX_ERROR;
		}
	}
    else
    {
		// use standard DLLs
		if ( FALSE == LoadDLL(  _W( "ogg.dll" ), _W( "\\encoders\\libraries\\libvorbis\\Dll" ), m_hOggDLL, bWarning, FALSE ) )
		{
			LTRACE( _T( "CEncoderVorbisDll::InitEncoder Failed to load %s" ), _T( "ogg.dll" ) );
			bReturn = CDEX_ERROR;
		}
    }


	if ( NULL == m_hOggDLL  || NULL == m_hLibVorbisDLL )
	{
		bReturn = CDEX_ERROR;
	}
	else
	{

		// Get the function pointers of the vorbis DLL
		vorbis_encode_init_func = (int (*)(vorbis_info *vi,long channels,long rate,long max_bitrate,long nominal_bitrate,long min_bitrate))
			::GetProcAddress( m_hLibVorbisDLL, "vorbis_encode_init" );
		vorbis_encode_init_vbr_func = (int (*)(vorbis_info *vi,long channels,long rate,float base_quality))
			::GetProcAddress( m_hLibVorbisDLL, "vorbis_encode_init_vbr" );


		vorbis_info_init_func = (void (*)(vorbis_info *vi))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_info_init" );

		vorbis_info_clear_func = (void (*)(vorbis_info *vi))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_info_clear" );

		vorbis_dsp_clear_func = (void (*)(vorbis_dsp_state *v))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_dsp_clear" );

		vorbis_block_init_func = (int  (*)(vorbis_dsp_state *v, vorbis_block *vb))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_block_init" );

		vorbis_block_clear_func = (int  (*)(vorbis_block *vb))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_block_clear" );

		vorbis_comment_init_func = (void (*)(vorbis_comment *vc))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_comment_init" );

		vorbis_comment_add_func = (void (*)(vorbis_comment *vc, char *comment))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_comment_add" );

		vorbis_comment_add_tag_func = (void (*)(vorbis_comment *vc, char *tag, char *contents))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_comment_add_tag" );

		vorbis_comment_clear_func = (void (*)(vorbis_comment *vc))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_comment_clear" );

		vorbis_analysis_init_func = (int (*)(vorbis_dsp_state *v,vorbis_info *vi))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_analysis_init" );

		vorbis_analysis_func = (int (*)(vorbis_block *vb,ogg_packet *op))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_analysis" );

		vorbis_analysis_headerout_func = (int (*)(vorbis_dsp_state *v,vorbis_comment *vc,ogg_packet *op,ogg_packet *op_comm,ogg_packet *op_code))
			::GetProcAddress(m_hLibVorbisDLL, "vorbis_analysis_headerout" );

		vorbis_analysis_buffer_func = (float **(*)(vorbis_dsp_state *v,int vals))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_analysis_buffer" );

		vorbis_analysis_wrote_func = (int (*)(vorbis_dsp_state *v,int vals))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_analysis_wrote" );

		vorbis_analysis_blockout_func = (int (*)(vorbis_dsp_state *v,vorbis_block *vb))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_analysis_blockout" );

		vorbis_synthesis_headerin_func = (int (*)(vorbis_info *vi,vorbis_comment *vc,ogg_packet *op))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_synthesis_headerin" );

		vorbis_bitrate_addblock_func = (int (*)(vorbis_block *vb))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_bitrate_addblock" );

		vorbis_bitrate_flushpacket_func = (int (*)(vorbis_dsp_state *vd,ogg_packet *op))
			::GetProcAddress( m_hLibVorbisDLL,  "vorbis_bitrate_flushpacket" );

		// Get the function pointers of the ogg DLL
		ogg_stream_init_func = (int (*)(ogg_stream_state *os,int serialno))
			::GetProcAddress( m_hOggDLL,  "ogg_stream_init" );
		ogg_stream_packetin_func = (int (*)(ogg_stream_state *os, ogg_packet *op))
			::GetProcAddress( m_hOggDLL,  "ogg_stream_packetin" );
		ogg_stream_flush_func = (int (*)(ogg_stream_state *os, ogg_page *og))
			::GetProcAddress( m_hOggDLL,  "ogg_stream_flush" );
		ogg_stream_pageout_func = (int (*)(ogg_stream_state *os, ogg_page *og))
			::GetProcAddress(m_hOggDLL,  "ogg_stream_pageout" );
		ogg_page_eos_func = (int (*)(ogg_page *og))
			::GetProcAddress( m_hOggDLL, "ogg_page_eos" );
		ogg_stream_clear_func = (int (*)(ogg_stream_state *os))
			::GetProcAddress( m_hOggDLL, "ogg_stream_clear" );


		if ( !(
			vorbis_encode_init_func != NULL && 
			vorbis_encode_init_vbr_func != NULL && 
			vorbis_info_init_func  != NULL && 
			vorbis_block_init_func != NULL &&
			vorbis_block_clear_func != NULL &&
			vorbis_comment_init_func != NULL &&
			vorbis_comment_add_func != NULL && 
			vorbis_comment_add_tag_func != NULL &&
			vorbis_comment_clear_func != NULL && 
			vorbis_analysis_init_func != NULL &&
			vorbis_analysis_func != NULL && 
			vorbis_analysis_headerout_func != NULL &&
			vorbis_analysis_buffer_func != NULL && 
			vorbis_analysis_wrote_func != NULL &&
			vorbis_analysis_blockout_func != NULL && 
			vorbis_synthesis_headerin_func != NULL &&
			vorbis_dsp_clear_func != NULL && 
			vorbis_info_clear_func != NULL &&
			ogg_stream_init_func != NULL && 
			ogg_stream_packetin_func != NULL &&
			ogg_stream_flush_func != NULL && 
			ogg_stream_pageout_func != NULL &&
			ogg_page_eos_func != NULL && 
			ogg_stream_clear_func != NULL ) )
		{
				ASSERT( FALSE );
				bReturn = CDEX_ERROR;
		}

	}

	EXIT_TRACE( _T( "CEncoderVorbisDll::OpenDlls() with return %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderVorbisDll::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoderVorbisDll::InitEncoder" ) );

	CEncoder::InitEncoder( pTask );

    LTRACE( _T( "CEncoderVorbisDll::InitEncoder Trying to load :%s.%s:" ), g_config.GetAppPath(), m_strEncoderPath );

	bReturn = OpenDlls( FALSE );

	EXIT_TRACE( _T( "CEncoderVorbisDll::InitEncoder, result %d" ), bReturn );

	return bReturn;
}

CDEX_ERR CEncoderVorbisDll::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	CDEX_ERR	bReturn		= CDEX_OK;
	LONG		lMinBitRate	= GetMinBitrate() * 1000;
	LONG		lNomBitRate	= GetBitrate() * 1000;
	LONG		lMaxBitRate	= GetMaxBitrate() * 1000;
	DWORD		dwInfoMode	= 0;
	bool		bStereo		= true;
	float		fQuality	= 1.0f;
	int			nReturn     = 0;

	ENTRY_TRACE( _T( "CEncoderVorbisDll::OpenStream( %s.%s, %d %d" ),
				strOutFileName, 
                GetExtension(),
				dwSampleRate,
				nChannels );

	// clear end of stream flag
	m_eos = false;

	// setup bitrates
	if ( lMaxBitRate <= 0 ) 
	{
		lMaxBitRate = -1;
	}

	if ( lMinBitRate <= 0 ) 
	{
		lMinBitRate = -1;
	}
	
	if ( lNomBitRate <= 0 ) 
	{
		lNomBitRate = -1;
	}

	if ( -1 == lNomBitRate && -1 == lMinBitRate && -1 == lMaxBitRate )
	{
		lNomBitRate = 160000;
	}

	// setup number of input channels
	m_nInputChannels = nChannels;

	// setup number of output channels
	if ( ( m_nMode & 0xFFFF ) == BE_MP3_MODE_MONO )
	{
		bStereo = false;
	}


	// mixer setup
	if ( ( false == bStereo ) && ( 2 == nChannels ) )
	{
		m_bDownMixToMono  = TRUE;
	}
	else
	{
		m_bDownMixToMono = FALSE;
	}

	if (  ( true == bStereo ) && ( 1 == nChannels ) )
	{
		m_bUpMixToStereo = TRUE;
	}
	else
	{
		m_bUpMixToStereo = FALSE;
	}

	// setup quality
	fQuality = ( (float) ((SHORT)LOWORD( GetUserN1() ) ) / 1000.0f );

	if ( fQuality < -0.1f )
	{
		fQuality = -0.1f;
	}

	if (  fQuality > 1.0f ) 
	{
		fQuality = 1.0f;
	}

	// Have vorbisenc choose a mode for us
	vorbis_info_init_func( &m_vi );

	// choose proper encoder, based on user settings
	if ( 0 == HIWORD( GetUserN1() ) )
	{
		LTRACE2( _T( "CEncoderVorbisDll::OpenStream, managed bit rate, %d %d %d" ),
				lMinBitRate, lNomBitRate, lMaxBitRate );

		/* setup the encoder parameters */
		nReturn = vorbis_encode_init_func(	&m_vi,
											(bStereo == true )? 2 : 1,
											dwSampleRate,
											lMaxBitRate,
											lNomBitRate,
											lMinBitRate );

	}
	else
	{
		LTRACE2( _T( "CEncoderVorbisDll::OpenStream, using quality option %f" ),
				fQuality );

		/* setup the encoder parameters for VBR encoding using quality parameter */
		nReturn = vorbis_encode_init_vbr_func(	&m_vi,
												(bStereo == true )? 2 : 1,
												dwSampleRate,
												fQuality );
	}


	if ( nReturn != 0 )
	{
		CUString strLang;
		bReturn = CDEX_ERROR;
		strLang = g_language.GetString( IDS_ENCODER_OGGINITFAILED );

		CDexMessageBox( strLang );
	}
	else
	{

		// add a comment
	   vorbis_comment_init_func( &m_vc );

		// Add tag info
		if ( m_pTask )
		{
			CUString strArtist;
			CUString strAlbum;
			CUString strTitle;
			CUString strComment;
			CUString strEncodedBy;
			CUString	strYear;
			CUString	strGenre;
			CUString	strLang;

            CTagData& tagData( m_pTask->GetTagData() );

            LONG	trackNumber = tagData.GetTrackNumber() + tagData.GetTrackOffset();
            DWORD	dwTotalTracks = tagData.GetTotalTracks();

			strArtist = tagData.GetArtist();
			strAlbum = tagData.GetAlbum();
            strTitle = tagData.GetTitle();
            strComment = tagData.GetComment();
            strEncodedBy = tagData.GetEncodedBy();
			strYear  = tagData.GetYear();
			strGenre = tagData.GetGenre();

			// only add comment string when not empty
			if ( !strComment.IsEmpty() )
			{
				AddUtfTag( &m_vc, _T( "COMMENT" ), strComment );
			}

			if ( !strTitle.IsEmpty() )
			{
				AddUtfTag( &m_vc, _T( "TITLE" ), strTitle );
			}

			if ( !strArtist.IsEmpty() )
			{
				AddUtfTag(&m_vc, _T( "ARTIST" ), strArtist );
			}

			if ( !strAlbum.IsEmpty() )
			{
				AddUtfTag(&m_vc, _T( "ALBUM" ), strAlbum );
			}

			if ( trackNumber > 0 )
			{
					CUString strTrackNumber;

					switch ( g_config.GetID3V2TrackNumber() )
					{
						case 0:
							strTrackNumber.Format( _W( "%d"), trackNumber);
						break;
						case 1:
							strTrackNumber.Format( _W( "%d/%d"), trackNumber, dwTotalTracks  );
						break;
						case 2:
							strTrackNumber.Format( _W( "%02d"), trackNumber );
						break;
						case 3:
							strTrackNumber.Format( _W( "%02d/%02d"), trackNumber, dwTotalTracks  );
						break;
						default:
							ASSERT( FALSE );
					}

					AddUtfTag( &m_vc, _T( "TRACKNUMBER" ), strTrackNumber );		
			}

			if ( !strEncodedBy.IsEmpty() ) 
			{
				AddUtfTag(&m_vc, _T( "ENCODEDBY" ), strEncodedBy );
			}


			if ( !strGenre.IsEmpty() ) 
			{
				AddUtfTag(&m_vc, _T( "GENRE" ), strGenre );
			}

			if ( !strYear.IsEmpty() ) 
			{
				AddUtfTag(&m_vc, _T( "DATE" ), strYear );
			}
		}

		// set up the analysis state and auxiliary encoding storage
		vorbis_analysis_init_func( &m_vd, &m_vi );
		vorbis_block_init_func( &m_vd, &m_vb );


		// set up our packet->stream encoder 
		// pick a random serial number; that way we can more likely build
		// chained streams just by concatenation

		srand( time( NULL ) );

		ogg_stream_init_func( &m_os, GetTickCount() );

		// Open output stream
		m_fpOut = CDexOpenFile( strOutFileName + _W( "." ) + GetExtension(), _W( "wb+" ) );

		if ( NULL == m_fpOut )
		{
			CUString strLang;
			CUString strErr;

			strLang = g_language.GetString( IDS_ENCODER_ERROR_COULDNOTOPENFILE );

			strErr.Format( strLang, (LPCWSTR)CUString( strOutFileName + _W( "." ) + GetExtension() ) );

			CDexMessageBox( strErr );

			bReturn = CDEX_FILEOPEN_ERROR;
		}


		// Vorbis streams begin with three headers; the initial header (with
		// most of the codec setup parameters) which is mandated by the Ogg
		// bitstream spec.  The second header holds any comment fields.  The
		// third header holds the bitstream codebook.  We merely need to
		// make the headers, then pass them to libvorbis one at a time;
		// libvorbis handles the additional Ogg bitstream constraints */
		if ( CDEX_OK == bReturn  )
		{

			ogg_packet header;
			ogg_packet header_comm;
			ogg_packet header_code;

			// build packets
			vorbis_analysis_headerout_func( &m_vd,&m_vc,&header,&header_comm,&header_code );

			// stream them out
			ogg_stream_packetin_func(&m_os,&header ); 
			ogg_stream_packetin_func( &m_os, &header_comm );
			ogg_stream_packetin_func( &m_os, &header_code );

			// We don't have to write out here, but doing so makes streaming 
			// much easier, so we do, flushing ALL pages. This ensures the actual
			// audio data will start on a new page

			while( !m_eos && ( CDEX_OK == bReturn ) )
			{
				int result = ogg_stream_flush_func( &m_os, &m_og );

				if( 0 == result )
					break;

				bReturn = WriteVorbisFrame();
			}
		}


		m_dwInBufferSize = 1024 * nChannels;

		if ( m_bDownMixToMono )
		{
			m_dwInBufferSize *= 2;
		}
	}

	// Set output buffer size, no output buffer needed, write directly to file
	m_dwOutBufferSize = 0;

	if ( CDEX_OK == bReturn )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	EXIT_TRACE( _T( "CEncoderVorbisDll::OpenStream, result %d" ), bReturn );
	return bReturn;
}


CDEX_ERR CEncoderVorbisDll::WriteVorbisFrame()
{
	CDEX_ERR bReturn = CDEX_OK;

	ASSERT( m_fpOut );

	// write header to output file
	if ( 1 != fwrite( m_og.header, m_og.header_len, 1, m_fpOut ) )
	{
		ASSERT( FALSE );
		bReturn = CDEX_FILEWRITE_ERROR;
		m_eos = true;
	}

	// write body to output file
	if ( 1 != fwrite( m_og.body, m_og.body_len, 1, m_fpOut ) )
	{
		ASSERT( FALSE );
		bReturn = CDEX_FILEWRITE_ERROR;
		m_eos = true;
	}

	return bReturn;
}

CDEX_ERR CEncoderVorbisDll::EncodeChunk( PSHORT pbsInSamples,DWORD dwNumSamples )
{
	CDEX_ERR bReturn = CDEX_OK;
	DWORD nBytesToWrite=0;

	LTRACE2( _T( "Entering CEncoderVorbisDll::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );


	// end of stream?
	if ( m_eos )
	{
		bReturn = CDEX_ERROR;
	}
	else
	{
		dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );
	
		FLOAT**	pfInputbuffer = NULL;
		DWORD	dwSample = 0;
		PSHORT	pSamples = m_psInputStream;

		// expose the buffer to submit data
		pfInputbuffer = vorbis_analysis_buffer_func( &m_vd, dwNumSamples/ GetNumChannels() ); 

		// get samples
		// uninterleave and copy the samples
		for( dwSample = 0; dwSample < dwNumSamples / GetNumChannels(); dwSample++ )
		{
			pfInputbuffer[0][dwSample] = (float)(*pSamples++)/32768.0f;

			if ( 2 == GetNumChannels() )
			{
				pfInputbuffer[1][dwSample] = (float)(*pSamples++)/32768.0f;
			}
		}

		// tell the library how much we actually submitted
		vorbis_analysis_wrote_func( &m_vd, dwNumSamples / GetNumChannels() );

		LTRACE2( _T( "CEncoderVorbisDll::EncodeChunk() Fed Encoder %d samples" ),
				dwNumSamples );


		// vorbis does some data preanalysis, then divvies up blocks for
		// more involved (potentially parallel) processing.  Get a single
		// block for encoding now
		while( 1 == vorbis_analysis_blockout_func( &m_vd, &m_vb ) )
		{
			// analysis
			vorbis_analysis_func( &m_vb, NULL );

			vorbis_bitrate_addblock_func( &m_vb);

			// EXTRA LOOP ADDED 20011225 FOR RC3 BITRATE MANAGEMENT
			while( vorbis_bitrate_flushpacket_func( &m_vd, &m_op ) )
			{
				// weld the packet into the bitstream
				ogg_stream_packetin_func( &m_os, &m_op );
    
				// write out pages (if any)
				do
				{
					int nResult = 0;

					nResult = ogg_stream_pageout_func( &m_os, &m_og );
					if ( 0 == nResult ) 
					{
						break;
					}
		
					// write the data
					bReturn = WriteVorbisFrame();

					if( ogg_page_eos_func( &m_og ) )
					{
						m_eos = true;;
					}

				} while( CDEX_OK == bReturn );
			}
		}
	}

	LTRACE2( _T( "Leaving CEncoderVorbisDll::EncodeChunk(), return status %d" ),
					bReturn );

	return bReturn;
}


CDEX_ERR CEncoderVorbisDll::CloseStream()
{
	ENTRY_TRACE( _T( "CEncoderVorbisDll::CloseStream" ) );

	// get the last samples out of the encoder
	// feed the encoder with zero samples, otherwhise we don't get 
	// the last frame
	EncodeChunk( NULL, 0 );


   /* clean up and exit.  vorbis_info_clear() must be called last */
	ogg_stream_clear_func( &m_os );
	vorbis_block_clear_func( &m_vb );
	vorbis_dsp_clear_func( &m_vd );
	vorbis_comment_clear_func( &m_vc );
//	vorbis_info_clear_func( &m_vi );

	(void)CEncoder::CloseStream();

	EXIT_TRACE( _T( "CEncoderVorbisDll::CloseStream" ) );

	// No Errors
	return CDEX_OK;
}


CDEX_ERR CEncoderVorbisDll::DeInitEncoder()
{
	(void)CEncoder::DeInitEncoder();

	return CloseDlls();
}


CDEX_ERR CEncoderVorbisDll::CloseDlls()
{
	ENTRY_TRACE( _T( "CEncoderVorbisDll::CloseDlls() " ) );

	if ( m_hLibVorbisDLL )
	{
		::FreeLibrary( m_hLibVorbisDLL );
		m_hLibVorbisDLL = NULL;
	}
	if ( m_hOggDLL )
	{
		::FreeLibrary( m_hOggDLL );
		m_hOggDLL = NULL;
	}

	EXIT_TRACE( _T( "CEncoderVorbisDll::CloseDlls() " ) );

	return CDEX_OK;
}

void CEncoderVorbisDll::GetDLLVersionInfo()
{
	vorbis_block		vb;
	vorbis_dsp_state	vd;
	vorbis_info			vi;
	vorbis_comment		vc;

	ogg_packet			header;
	ogg_packet			header_comm;
	ogg_packet			header_code;

	CUString				strVersion;

	m_bAvailable = FALSE;

	ENTRY_TRACE( _T( "CEncoderVorbisDll::GetDLLVersionInfo() " ) );

	// open the vorbis related DLLs
	if ( CDEX_OK == OpenDlls( FALSE ) )
	{
		LTRACE( _T( "CEncoderVorbisDll::GetDLLVersionInfo(), DLLs found" ) );

		m_bAvailable = TRUE;

		// to get build version, create first 3 ogg packets,
		// take the second and read it back in; the vc.vendor
		// then is filled with build version
		vorbis_info_init_func( &vi );

		vorbis_encode_init_func(&vi,2,44100, -1, 192000, -1);

		vorbis_analysis_init_func(&vd,&vi);

		vorbis_block_init_func( &vd, &vb );

		vorbis_comment_init_func( &vc );

		vorbis_analysis_headerout_func( &vd, &vc, &header,&header_comm,&header_code );

		vorbis_synthesis_headerin_func( &vi, &vc, &header_comm );

		strVersion.Format( _W( "(%s)" ), (LPCWSTR)CUString( vc.vendor, CP_UTF8 ) );

		vorbis_block_clear_func( &vb );

		vorbis_dsp_clear_func( &vd );

		vorbis_comment_clear_func( &vc );

//		vorbis_info_clear_func( &vi );
	}
	else
	{
		LTRACE( _T( "CEncoderVorbisDll::GetDLLVersionInfo(), DLLs not found" ) );
		strVersion = g_language.GetString( IDS_ENCODER_VERSIONNOTAVAILABLE );
	}

	m_strEncoderID += strVersion;

	CloseDlls();

	EXIT_TRACE( _T( "CEncoderVorbisDll::GetDLLVersionInfo() " ) );

}

CEncoderDlg* CEncoderVorbisDll::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg=new CEncoderVorbidDllDlg;
	}
	return m_pSettingsDlg;
}


CDEX_ERR CEncoderVorbisDll::AddUtfTag( vorbis_comment *vc, const CUString& strField, const CUString& strFieldValue )
{
	// add comment
    CUStringConvert strCnv;
    CUStringConvert strCnv1;

    vorbis_comment_add_tag_func( vc, strCnv.ToUTF8( strField ), strCnv1.ToUTF8( strFieldValue ) );

	return CDEX_OK;
}
