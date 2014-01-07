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
#include "EncoderFlacDll.h"
#include "EncoderFlacDllDlg.h"
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


INITTRACE( _T( "EncoderFlacDll" ) );

// #pragma comment(linker, "/delayload:libFlac.dll")
//#pragma comment(linker, "/delayload:libOggFlac.dll")



// CONSTRUCTOR
CEncoderFlacDll::CEncoderFlacDll() : 
	CEncoder(),
	m_hLibOggFlacDLL( NULL ),
	m_hFlacDll( NULL ),
	m_piInputStream( NULL )
{
	ENTRY_TRACE( _T( "CEncoderFlacDll::CEncoderFlacDll()" ) );

	m_pStreamEncoder = NULL;

	m_strStreamFileName = _T( "" );
	
	// DLL encoders do support ChunkEncoding
	m_bOnTheFlyEncoding = TRUE;

	m_strEncoderPath = _T( "libFlac.dll" );
	m_strEncoderID = _T( "FLAC Encoder DLL" );
	m_strExtension= _T( "flac" );

	// All DLL derivates support chunk support
	m_bChunkSupport = TRUE;

	m_bRiffWavSupport = FALSE;

	m_dResampleRatio = 1.0;

	m_nEncoderID = ENCODER_FLAC;

	SetMinBitrate( 0 );
	SetBitrate( 0 );
	SetMaxBitrate( 0 );

	SetUserN1( 8 );

	// get current settings from ini file
	LoadSettings();

	// Get encoder ID to get proper version info
	GetDLLVersionInfo();

	EXIT_TRACE( _T( "CEncoderFlacDll::CEncoderFlacDll()" ) );

}


// DESTRUCTOR
CEncoderFlacDll::~CEncoderFlacDll()
{
	ENTRY_TRACE( _T( "CEncoderFlacDll::~CEncoderFlacDll()" ) );

	if ( m_pStreamEncoder )
	{
		ASSERT( FALSE );
		FLAC__seekable_stream_encoder_delete( m_pStreamEncoder );
		m_pStreamEncoder = NULL;
	}

	CloseDlls();

	if ( NULL != m_fpOut )
	{
		ASSERT( FALSE );
		fclose( m_fpOut );
		m_fpOut = NULL;
	}
	EXIT_TRACE( _T( "CEncoderFlacDll::~CEncoderFlacDll()" ) );
}

CDEX_ERR CEncoderFlacDll::OpenDlls( BOOL bWarning )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T("CEncoderFlacDll::OpenDlls()" ) );

	// use standard DLLs
//	if ( FALSE == LoadDLL(  _W( "libOggFLAC.dll" ), _W( "\\encoders\\libraries\\libFlac\\Dll" ), m_hLibOggFlacDLL, bWarning, FALSE ) )
//	{
//		LTRACE( _T( "CEncoderFlacDll::InitEncoder Failed to load %s" ), _T( "oog.dll" ) );
//		bReturn = CDEX_ERROR;
//	} else 
		if ( FALSE == LoadDLL(  _W( "libFlac.dll" ), _W( "\\encoders\\libraries\\libFlac\\Dll" ), m_hFlacDll, bWarning, FALSE ) )
	{
		LTRACE( _T( "CEncoderFlacDll::OpenDLLs Failed to load %s" ), _T( "flac.dll" ) );
		bReturn = CDEX_ERROR;
	}

	EXIT_TRACE( _T( "CEncoderFlacDll::OpenDlls() with return %d" ), bReturn );

	return bReturn;
}


CDEX_ERR CEncoderFlacDll::InitEncoder( CTaskInfo* pTask )
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoderFlacDll::InitEncoder" ) );

	CEncoder::InitEncoder( pTask );

	bReturn = OpenDlls( TRUE );

	ASSERT( CDEX_OK == bReturn );

	if ( CDEX_OK == bReturn )
	{
		// create new stream encoder object
		m_pStreamEncoder = FLAC__seekable_stream_encoder_new();

		if( NULL == m_pStreamEncoder )
		{
			ASSERT( FALSE );
			bReturn = CDEX_ERROR;
		}
	}


	EXIT_TRACE( _T( "CEncoderFlacDll::InitEncoder, result %d" ), bReturn );

	return bReturn;
}



#if 0

void CEncoderFlacDll::stream_encoder_metadata_callback_(const FLAC__seekable_streamEncoder *encoder, const FLAC__seekable_streamMetadata *metadata, void *client_data)
{
	FLAC__SeekableStreamEncoder *seekable_stream_encoder = (FLAC__SeekableStreamEncoder*)client_data;
	FLAC__byte b[max(6, FLAC__seekable_stream_METADATA_SEEKPOINT_LENGTH)];
	const FLAC__uint64 samples = metadata->data.stream_info.total_samples;
	const unsigned min_framesize = metadata->data.stream_info.min_framesize;
	const unsigned max_framesize = metadata->data.stream_info.max_framesize;
	const unsigned bps = metadata->data.stream_info.bits_per_sample;

	FLAC__ASSERT(metadata->type == FLAC__METADATA_TYPE_STREAMINFO);



	/* We get called by the stream encoder when the encoding process
	 * has finished so that we can update the STREAMINFO and SEEKTABLE
	 * blocks.
	 */

	(void)encoder; /* silence compiler warning about unused parameter */

	/*@@@ reopen callback here?  The docs currently require user to open files in update mode from the start */

	/* All this is based on intimate knowledge of the stream header
	 * layout, but a change to the header format that would break this
	 * would also break all streams encoded in the previous format.
	 */

	/*
	 * Write MD5 signature
	 */
	if(seekable_stream_encoder->private_->seek_callback(seekable_stream_encoder, 26, seekable_stream_encoder->private_->client_data) != FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_OK) {
		seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_SEEK_ERROR;
		return;
	}
	if(seekable_stream_encoder->private_->write_callback(seekable_stream_encoder, metadata->data.stream_info.md5sum, 16, 0, 0, seekable_stream_encoder->private_->client_data) != FLAC__seekable_stream_ENCODER_WRITE_STATUS_OK) {
		seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_WRITE_ERROR;
		return;
	}

	/*
	 * Write total samples
	 */
	b[0] = ((FLAC__byte)(bps-1) << 4) | (FLAC__byte)((samples >> 32) & 0x0F);
	b[1] = (FLAC__byte)((samples >> 24) & 0xFF);
	b[2] = (FLAC__byte)((samples >> 16) & 0xFF);
	b[3] = (FLAC__byte)((samples >> 8) & 0xFF);
	b[4] = (FLAC__byte)(samples & 0xFF);
	if(seekable_stream_encoder->private_->seek_callback(seekable_stream_encoder, 21, seekable_stream_encoder->private_->client_data) != FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_OK) {
		seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_SEEK_ERROR;
		return;
	}
	if(seekable_stream_encoder->private_->write_callback(seekable_stream_encoder, b, 5, 0, 0, seekable_stream_encoder->private_->client_data) != FLAC__seekable_stream_ENCODER_WRITE_STATUS_OK) {
		seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_WRITE_ERROR;
		return;
	}

	/*
	 * Write min/max framesize
	 */
	b[0] = (FLAC__byte)((min_framesize >> 16) & 0xFF);
	b[1] = (FLAC__byte)((min_framesize >> 8) & 0xFF);
	b[2] = (FLAC__byte)(min_framesize & 0xFF);
	b[3] = (FLAC__byte)((max_framesize >> 16) & 0xFF);
	b[4] = (FLAC__byte)((max_framesize >> 8) & 0xFF);
	b[5] = (FLAC__byte)(max_framesize & 0xFF);
	if(seekable_stream_encoder->private_->seek_callback(seekable_stream_encoder, 12, seekable_stream_encoder->private_->client_data) != FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_OK) {
		seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_SEEK_ERROR;
		return;
	}
	if(seekable_stream_encoder->private_->write_callback(seekable_stream_encoder, b, 6, 0, 0, seekable_stream_encoder->private_->client_data) != FLAC__seekable_stream_ENCODER_WRITE_STATUS_OK) {
		seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_WRITE_ERROR;
		return;
	}

	/*
	 * Write seektable
	 */
	if(0 != seekable_stream_encoder->private_->seek_table && seekable_stream_encoder->private_->seek_table->num_points > 0 && seekable_stream_encoder->private_->seektable_offset > 0) {
		unsigned i;

		FLAC__format_seektable_sort(seekable_stream_encoder->private_->seek_table);

		FLAC__ASSERT(FLAC__format_seektable_is_legal(seekable_stream_encoder->private_->seek_table));

		if(seekable_stream_encoder->private_->seek_callback(seekable_stream_encoder, seekable_stream_encoder->private_->seektable_offset + FLAC__seekable_stream_METADATA_HEADER_LENGTH, seekable_stream_encoder->private_->client_data) != FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_OK) {
			seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_SEEK_ERROR;
			return;
		}

		for(i = 0; i < seekable_stream_encoder->private_->seek_table->num_points; i++) {
			FLAC__uint64 xx;
			unsigned x;
			xx = seekable_stream_encoder->private_->seek_table->points[i].sample_number;
			b[7] = (FLAC__byte)xx; xx >>= 8;
			b[6] = (FLAC__byte)xx; xx >>= 8;
			b[5] = (FLAC__byte)xx; xx >>= 8;
			b[4] = (FLAC__byte)xx; xx >>= 8;
			b[3] = (FLAC__byte)xx; xx >>= 8;
			b[2] = (FLAC__byte)xx; xx >>= 8;
			b[1] = (FLAC__byte)xx; xx >>= 8;
			b[0] = (FLAC__byte)xx; xx >>= 8;
			xx = seekable_stream_encoder->private_->seek_table->points[i].stream_offset;
			b[15] = (FLAC__byte)xx; xx >>= 8;
			b[14] = (FLAC__byte)xx; xx >>= 8;
			b[13] = (FLAC__byte)xx; xx >>= 8;
			b[12] = (FLAC__byte)xx; xx >>= 8;
			b[11] = (FLAC__byte)xx; xx >>= 8;
			b[10] = (FLAC__byte)xx; xx >>= 8;
			b[9] = (FLAC__byte)xx; xx >>= 8;
			b[8] = (FLAC__byte)xx; xx >>= 8;
			x = seekable_stream_encoder->private_->seek_table->points[i].frame_samples;
			b[17] = (FLAC__byte)x; x >>= 8;
			b[16] = (FLAC__byte)x; x >>= 8;
			if(seekable_stream_encoder->private_->write_callback(seekable_stream_encoder, b, 18, 0, 0, seekable_stream_encoder->private_->client_data) != FLAC__seekable_stream_ENCODER_WRITE_STATUS_OK) {
				seekable_stream_encoder->protected_->state = FLAC__SEEKABLE_STREAM_ENCODER_WRITE_ERROR;
				return;
			}
		}
	}
}
#endif

CDEX_ERR CEncoderFlacDll::OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels)
{
	CDEX_ERR	bReturn		= CDEX_OK;
	LONG		lMinBitRate	= GetMinBitrate() * 1000;
	LONG		lNomBitRate	= GetBitrate() * 1000;
	LONG		lMaxBitRate	= GetMaxBitrate() * 1000;
	DWORD		dwInfoMode	= 0;
	bool		bStereo		= true;
	float		fQuality	= 1.0f;
	int			nReturn     = 0;
	int			nFlacBlockSize = 1152;


    CUStringConvert strCnv;

	FLAC__SeekableStreamEncoderState nState;

	ENTRY_TRACE( _T( "CEncoderFlacDll::OpenStream( %s, %d %d" ),
				strCnv.ToT( strOutFileName + _W( "." ) + GetExtension() ),
				dwSampleRate,
				nChannels );

	m_strFullOutputName = strOutFileName + _W( "." ) + GetExtension();

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

	int nCompressionLevel = GetUserN1() & 0xFF;

	// INIT

	
	m_dwInBufferSize = nFlacBlockSize * nChannels;

	if ( m_bDownMixToMono )
	{
		m_dwInBufferSize *= 2;
	}

	// Set output buffer size, no output buffer needed, write directly to file
	m_dwOutBufferSize = 0;


	FLAC__bool bVerify = false;
	FLAC__bool bStreamableSubset = false;
	FLAC__bool do_exhaustive_model_search = false;
	FLAC__bool do_escape_coding = false;
	FLAC__bool do_mid_side = false;
	FLAC__bool loose_mid_side = false;
	int		qlp_coeff_precision = 0;
	int min_residual_partition_order = 2;
	int max_residual_partition_order = 2;
	int rice_parameter_search_dist = 0;
	int max_lpc_order = 0;

	int bitsPerSample = 16;

	switch ( nCompressionLevel )
	{
		case 0:
			do_exhaustive_model_search = false;
			do_escape_coding = false;
			do_mid_side = false;
			loose_mid_side = false;
			qlp_coeff_precision = 0;
			min_residual_partition_order = max_residual_partition_order = 2;
			rice_parameter_search_dist = 0;
			max_lpc_order = 0;
		break;

		case 1:
			do_exhaustive_model_search = false;
			do_escape_coding = false;
			do_mid_side = true;
			loose_mid_side = true;
			qlp_coeff_precision = 0;
			min_residual_partition_order = max_residual_partition_order = 2;
			rice_parameter_search_dist = 0;
			max_lpc_order = 0;
		break;

		case 2:
			do_exhaustive_model_search = false;
			do_escape_coding = false;
			do_mid_side = true;
			loose_mid_side = false;
			qlp_coeff_precision = 0;
			min_residual_partition_order = 0;
			max_residual_partition_order = 3;
			rice_parameter_search_dist = 0;
			max_lpc_order = 0;
		break;

		case 3:
			do_exhaustive_model_search = false;
			do_escape_coding = false;
			do_mid_side = false;
			loose_mid_side = false;
			qlp_coeff_precision = 0;
			min_residual_partition_order = max_residual_partition_order = 3;
			rice_parameter_search_dist = 0;
			max_lpc_order = 6;
		break;

		case 4:
			do_exhaustive_model_search = false;
			do_escape_coding = false;
			do_mid_side = true;
			loose_mid_side = true;
			qlp_coeff_precision = 0;
			min_residual_partition_order = max_residual_partition_order = 3;
			rice_parameter_search_dist = 0;
			max_lpc_order = 8;
		break;

		case 5:
			do_exhaustive_model_search = false;
			do_escape_coding = false;
			do_mid_side = true;
			loose_mid_side = false;
			qlp_coeff_precision = 0;
			min_residual_partition_order = max_residual_partition_order = 3;
			rice_parameter_search_dist = 0;
			max_lpc_order = 8;
		break;

		case 6:
			do_exhaustive_model_search = false;
			do_escape_coding = false;
			do_mid_side = true;
			loose_mid_side = false;
			qlp_coeff_precision = 0;
			min_residual_partition_order = 0;
			max_residual_partition_order = 4;
			rice_parameter_search_dist = 0;
			max_lpc_order = 8;
		break;

		case 7:
			do_exhaustive_model_search = true;
			do_escape_coding = false;
			do_mid_side = true;
			loose_mid_side = false;
			qlp_coeff_precision = 0;
			min_residual_partition_order = 0;
			max_residual_partition_order = 6;
			rice_parameter_search_dist = 0;
			max_lpc_order = 8;
		break;

		case 8:
			do_exhaustive_model_search = true;
			do_escape_coding = false;
			do_mid_side = true;
			loose_mid_side = false;
			qlp_coeff_precision = 0;
			min_residual_partition_order = 0;
			max_residual_partition_order = 6;
			rice_parameter_search_dist = 0;
			max_lpc_order = 12;
		break;

		default:
			ASSERT( FALSE );
		break;
	}

	FLAC__seekable_stream_encoder_set_verify( m_pStreamEncoder , bVerify );
	FLAC__seekable_stream_encoder_set_streamable_subset( m_pStreamEncoder , bStreamableSubset);
	FLAC__seekable_stream_encoder_set_channels( m_pStreamEncoder , bStereo?2:1 );

	// set only conditionally, otherwise stream won't be initialized by the FLAC encoder
	if ( bStereo )
	{
		FLAC__seekable_stream_encoder_set_loose_mid_side_stereo( m_pStreamEncoder , loose_mid_side );
		FLAC__seekable_stream_encoder_set_do_mid_side_stereo( m_pStreamEncoder , do_mid_side );
	}

	FLAC__seekable_stream_encoder_set_bits_per_sample( m_pStreamEncoder , bitsPerSample );
	FLAC__seekable_stream_encoder_set_sample_rate( m_pStreamEncoder , dwSampleRate);
	FLAC__seekable_stream_encoder_set_blocksize( m_pStreamEncoder , nFlacBlockSize );
	FLAC__seekable_stream_encoder_set_max_lpc_order( m_pStreamEncoder , max_lpc_order );
	FLAC__seekable_stream_encoder_set_qlp_coeff_precision( m_pStreamEncoder , qlp_coeff_precision);
//	FLAC__seekable_stream_encoder_set_do_qlp_coeff_prec_search( m_pStreamEncoder , do_qlp_coeff_prec_search);
	FLAC__seekable_stream_encoder_set_do_escape_coding( m_pStreamEncoder , do_escape_coding);
	FLAC__seekable_stream_encoder_set_do_exhaustive_model_search( m_pStreamEncoder , do_exhaustive_model_search );
	FLAC__seekable_stream_encoder_set_min_residual_partition_order( m_pStreamEncoder , min_residual_partition_order);
	FLAC__seekable_stream_encoder_set_max_residual_partition_order( m_pStreamEncoder , max_residual_partition_order);
	FLAC__seekable_stream_encoder_set_rice_parameter_search_dist( m_pStreamEncoder , rice_parameter_search_dist);

	int num_metadata = 0;

	FLAC__seekable_stream_encoder_set_client_data( m_pStreamEncoder, this );
	FLAC__seekable_stream_encoder_set_write_callback( m_pStreamEncoder , CEncoderFlacDll::EncoderWriteCallback );
	FLAC__seekable_stream_encoder_set_seek_callback( m_pStreamEncoder , CEncoderFlacDll::stream_encoder_seek_callback_ ) ;
	FLAC__seekable_stream_encoder_set_tell_callback( m_pStreamEncoder , CEncoderFlacDll::stream_encoder_tell_callback_ ) ;

	// Open output stream
	// must be done before the FLAC encoder is initialized
	// since meta data is written during this phase
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

	
	nState = FLAC__seekable_stream_encoder_init( m_pStreamEncoder );

	if( FLAC__SEEKABLE_STREAM_ENCODER_OK !=  nState )
	{
		ASSERT( FALSE );

		bReturn = CDEX_ERROR;
	}
	else
	{
	}


	if ( CDEX_OK == bReturn )
	{
		// Initialize input stream
		bReturn = InitInputStream();
	}

	m_piInputStream = new FLAC__int32[ m_dwInBufferSize ];

	EXIT_TRACE( _T( "CEncoderFlacDll::OpenStream, result %d" ), bReturn );
	return bReturn;
}


CDEX_ERR CEncoderFlacDll::EncodeChunk( PSHORT pbsInSamples,DWORD dwNumSamples )
{
	CDEX_ERR bReturn = CDEX_OK;
	DWORD nBytesToWrite=0;
	DWORD nSample = 0;

	LTRACE2( _T( "Entering CEncoderFlacDll::EncodeChunk(), dwNumSamples = %d " ),
					dwNumSamples );

	// Check the size of the input buffer
	ASSERT( dwNumSamples <= m_dwInBufferSize );

	dwNumSamples = ProcessData( pbsInSamples, dwNumSamples );


	for ( nSample = 0; nSample < dwNumSamples; nSample++ )
	{
		m_piInputStream[ nSample ] = m_psInputStream[ nSample ];
	}

	FLAC__bool bState;

	bState = FLAC__seekable_stream_encoder_process_interleaved(	m_pStreamEncoder, 
																m_piInputStream , 
																dwNumSamples / GetNumChannels() );
	if ( !bState )
	{
		bReturn = CDEX_ERROR;
	}

	LTRACE2( _T( "Leaving CEncoderFlacDll::EncodeChunk(), return status %d" ),
					bReturn );

	return bReturn;
}


CDEX_ERR CEncoderFlacDll::CloseStream()
{
	ENTRY_TRACE( _T( "CEncoderFlacDll::CloseStream" ) );

	// get the last samples out of the encoder
	// feed the encoder with zero samples, otherwhise we don't get 
	// the last frame
	EncodeChunk( NULL, 0 );

	if ( m_pStreamEncoder )
	{
		
		FLAC__seekable_stream_encoder_finish( m_pStreamEncoder );
	}

	delete [] m_piInputStream;
	m_piInputStream = NULL;

	if ( NULL != m_fpOut )
	{
		fclose( m_fpOut );
		m_fpOut = NULL;
	}

	WriteMetaData();


	return CDEX_OK;
}


CDEX_ERR CEncoderFlacDll::DeInitEncoder()
{
	(void)CEncoder::DeInitEncoder();

	if ( m_pStreamEncoder )
	{
		FLAC__seekable_stream_encoder_delete( m_pStreamEncoder );
		m_pStreamEncoder = NULL;
	}

	return CloseDlls();
}


CDEX_ERR CEncoderFlacDll::CloseDlls()
{
	ENTRY_TRACE( _T( "CEncoderFlacDll::CloseDlls() " ) );

	if ( m_hLibOggFlacDLL )
	{
		::FreeLibrary( m_hLibOggFlacDLL );
		m_hLibOggFlacDLL = NULL;
	}
	if ( m_hFlacDll )
	{
		::FreeLibrary( m_hFlacDll );
		m_hFlacDll = NULL;
	}

	EXIT_TRACE( _T( "CEncoderFlacDll::CloseDlls() " ) );

	return CDEX_OK;
}

void CEncoderFlacDll::GetDLLVersionInfo()
{

//	vorbis_block		vb;
//	vorbis_dsp_state	vd;
//	vorbis_info			vi;
//	vorbis_comment		vc;

//	ogg_packet			header;
//	ogg_packet			header_comm;
//	ogg_packet			header_code;

	CUString				strVersion;

	m_bAvailable = FALSE;

	ENTRY_TRACE( _T( "CEncoderFlacDll::GetDLLVersionInfo() " ) );

	// open the FLAC related DLLs
	if ( CDEX_OK == OpenDlls( FALSE ) )
	{
		m_bAvailable = TRUE;
#if 0
		LTRACE( _T( "CEncoderFlacDll::GetDLLVersionInfo(), DLLs found" ) );


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

		strVersion.Format( _W( "(%s)" ), (LPCWSTR)CUString( vc.vendor ) );

		vorbis_block_clear_func( &vb );

		vorbis_dsp_clear_func( &vd );

		vorbis_comment_clear_func( &vc );

		vorbis_info_clear_func( &vi );
#endif
	}
	else
	{
		LTRACE( _T( "CEncoderFlacDll::GetDLLVersionInfo(), DLLs not found" ) );
		strVersion = g_language.GetString( IDS_ENCODER_VERSIONNOTAVAILABLE );
	}

	m_strEncoderID += strVersion;

	CloseDlls();

	EXIT_TRACE( _T( "CEncoderFlacDll::GetDLLVersionInfo() " ) );
}

CEncoderDlg* CEncoderFlacDll::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg = new CEncoderFlacDllDlg;
	}
	return m_pSettingsDlg;
}



FLAC__StreamEncoderWriteStatus CEncoderFlacDll::EncoderWriteCallback(
	const FLAC__SeekableStreamEncoder *encoder, 
	const FLAC__byte buffer[],
	unsigned bytes,
	unsigned samples,
	unsigned current_frame,
	void *client_data )
{
	FLAC__StreamEncoderWriteStatus bReturn = FLAC__STREAM_ENCODER_WRITE_STATUS_OK;

	CEncoderFlacDll *pFlacEncoder = (CEncoderFlacDll*)client_data;

	if ( pFlacEncoder )
	{
		if ( pFlacEncoder->m_fpOut )
		{
			int nByteWritten = fwrite( buffer, sizeof(FLAC__byte), bytes, pFlacEncoder->m_fpOut );

			if( nByteWritten == bytes )
			{
				bReturn =  FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
			}
			else
			{
				bReturn = FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR;
			}
		}
	}

	return bReturn;

}


FLAC__SeekableStreamEncoderSeekStatus CEncoderFlacDll::stream_encoder_seek_callback_(const FLAC__SeekableStreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	CEncoderFlacDll *pFlacEncoder = (CEncoderFlacDll*)client_data;

	if ( pFlacEncoder->m_fpOut != NULL ) 
	{
		// AF TODO, USE 64 bits version
		fseek( pFlacEncoder->m_fpOut, (long)absolute_byte_offset, SEEK_SET );
		return FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_OK;
	} 

	return FLAC__SEEKABLE_STREAM_ENCODER_SEEK_STATUS_OK;
}
 
FLAC__SeekableStreamEncoderTellStatus CEncoderFlacDll::stream_encoder_tell_callback_ (const FLAC__SeekableStreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data)  
{
	*absolute_byte_offset = 0;
	CEncoderFlacDll *pFlacEncoder = (CEncoderFlacDll*)client_data;

	if ( pFlacEncoder->m_fpOut != NULL ) 
	{
		long lTellValue = ftell( pFlacEncoder->m_fpOut );
		*absolute_byte_offset = (__int64)lTellValue;
		return FLAC__SEEKABLE_STREAM_ENCODER_TELL_STATUS_OK;
	} 

	return FLAC__SEEKABLE_STREAM_ENCODER_TELL_STATUS_OK;
}



CDEX_ERR CEncoderFlacDll::AddUtfTag( FLAC__StreamMetadata *block, const CUString& strField, const CUString& strFieldValue )
{	
	CUString strTag( strField );
    
    strTag += _W( "=" );
    strTag += strFieldValue;


    // convert tag entry to UTF-8
    CUStringConvert strCnv;
    
    LPCSTR lpszValue = strCnv.ToUTF8( strTag );

	FLAC__ASSERT(block->type == FLAC__METADATA_TYPE_VORBIS_COMMENT);

    // create entry
	FLAC__StreamMetadata_VorbisComment_Entry entry;
	entry.entry = (FLAC__byte *)lpszValue ;
	entry.length = strlen( ( const char *)entry.entry );

    // set entry
	FLAC__metadata_object_vorbiscomment_append_comment( block, entry, true );

	return CDEX_OK;
}



void CEncoderFlacDll::WriteMetaData()
{
	if ( !m_strFullOutputName.IsEmpty() ) 
	{
		
		FLAC__Metadata_Chain*  metaChain = FLAC__metadata_chain_new();
		if ( NULL != metaChain )
		{
			LPCSTR dosFileName = GetDosFileName( m_strFullOutputName );
			FLAC__metadata_chain_read(  metaChain, dosFileName );  

			FLAC__StreamMetadata *block = 0;
			FLAC__Metadata_Iterator *metaIterator = FLAC__metadata_iterator_new();

			FLAC__metadata_iterator_init( metaIterator, metaChain );
			
			//block = FLAC__metadata_object_new( FLAC__METADATA_TYPE_VORBIS_COMMENT );

			// iterate to last block
			while( FLAC__metadata_iterator_next(metaIterator))
			{

			}

			// get reference to block
			block = FLAC__metadata_iterator_get_block( metaIterator );


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

                CTagData tagData( m_pTask->GetTagData() );

                LONG dwTrackNumber = (LONG)tagData.GetTrackNumber() + tagData.GetTrackOffset();
				DWORD	dwTotalTracks = tagData.GetTotalTracks( );

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
					AddUtfTag( block, _T( "COMMENT" ), strComment );
				}

				if ( !strTitle.IsEmpty() )
				{
					AddUtfTag( block, _T( "TITLE" ), strTitle );
				}

				if ( !strArtist.IsEmpty() )
				{
					AddUtfTag( block, _T( "ARTIST" ), strArtist );
				}

				if ( !strAlbum.IsEmpty() )
				{
					AddUtfTag( block, _T( "ALBUM" ), strAlbum );
				}

				if ( dwTrackNumber > 0 )
				{
						CUString strTrackNumber;

						switch ( g_config.GetID3V2TrackNumber() )
						{
							case 0:
								strTrackNumber.Format( _W( "%d"), dwTrackNumber);
							break;
							case 1:
								strTrackNumber.Format( _W( "%d/%d"), dwTrackNumber, dwTotalTracks  );
							break;
							case 2:
								strTrackNumber.Format( _W( "%02d"), dwTrackNumber );
							break;
							case 3:
								strTrackNumber.Format( _W( "%02d/%02d"), dwTrackNumber, dwTotalTracks  );
							break;
							default:
								ASSERT( FALSE );
						}

						AddUtfTag( block, _T( "TRACKNUMBER" ), strTrackNumber );		
				}

				if ( !strEncodedBy.IsEmpty() ) 
				{
					AddUtfTag( block, _T( "ENCODEDBY" ), strEncodedBy );
				}


				if ( !strGenre.IsEmpty() ) 
				{
					AddUtfTag( block, _T( "GENRE" ), strGenre );
				}

				if ( !strYear.IsEmpty() ) 
				{
					AddUtfTag( block, _T( "DATE" ), strYear );
				}
			}

			FLAC__metadata_iterator_delete( metaIterator );
			FLAC__metadata_chain_write(metaChain, true, true );
			FLAC__metadata_chain_delete( metaChain);
		
		}
	}
}

