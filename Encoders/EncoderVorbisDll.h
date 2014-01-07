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


#ifndef ENCODERVORBISDLL_H_INCLUDED
#define ENCODERVORBISDLL_H_INCLUDED

#include "Encode.h"
#include "vorbis/vorbisenc.h"

class CEncoderVorbisDll : public CEncoder
{
protected:
	HINSTANCE		m_hLibVorbisDLL;
	HINSTANCE		m_hOggDLL;

	CUString			m_strStreamFileName;
	const char*		m_lasterror;
	int				m_brmode;
	float			m_base_quality;
	bool			m_eos;
	ogg_stream_state m_os;
	ogg_page         m_og;
	ogg_packet       m_op;
	vorbis_info      m_vi;
	vorbis_comment   m_vc;
	vorbis_dsp_state m_vd;
	vorbis_block     m_vb;

public:
	virtual	CEncoderDlg*	GetSettingsDlg();

	// CONSTRUCTOR
	CEncoderVorbisDll();

	// DESTRUCTOR
	virtual ~CEncoderVorbisDll();

	// METHODS
	void GetDLLVersionInfo();
	virtual CDEX_ERR InitEncoder( CTaskInfo* pTask );
	virtual CDEX_ERR OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels);
	virtual CDEX_ERR EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples);
	virtual CDEX_ERR CloseStream();
	virtual CDEX_ERR DeInitEncoder();
private:
	CDEX_ERR CloseDlls();
	CDEX_ERR OpenDlls( BOOL bWarning );
	CDEX_ERR AddUtfTag( vorbis_comment *vc, const CUString& strField, const CUString& strFieldValue );
	CDEX_ERR WriteVorbisFrame();

	// vorbis.dll function pointer
	int (*vorbis_encode_init_func)(vorbis_info *vi,long channels,long rate,
		long max_bitrate,long nominal_bitrate,long min_bitrate);
	int (*vorbis_encode_init_vbr_func)(vorbis_info *vi,long channels,
		long rate,float base_quality);

	void (*vorbis_info_init_func)(vorbis_info *vi);
	void (*vorbis_info_clear_func)(vorbis_info *vi);
	void (*vorbis_dsp_clear_func)(vorbis_dsp_state *v);
	int  (*vorbis_block_init_func)(vorbis_dsp_state *v, vorbis_block *vb);
	int  (*vorbis_block_clear_func)(vorbis_block *vb);

	void (*vorbis_comment_init_func)(vorbis_comment *vc);
	void (*vorbis_comment_add_func)(vorbis_comment *vc, char *comment);
	void (*vorbis_comment_add_tag_func)(vorbis_comment *vc, char *tag, char *contents);
	void (*vorbis_comment_clear_func)(vorbis_comment *vc);

	int (*vorbis_analysis_init_func)(vorbis_dsp_state *v,vorbis_info *vi);
	int (*vorbis_analysis_func)(vorbis_block *vb,ogg_packet *op);
	int (*vorbis_analysis_headerout_func)(vorbis_dsp_state *v,vorbis_comment *vc,
		ogg_packet *op,ogg_packet *op_comm,ogg_packet *op_code);
	float **(*vorbis_analysis_buffer_func)(vorbis_dsp_state *v,int vals);
	int (*vorbis_analysis_wrote_func)(vorbis_dsp_state *v,int vals);
	int (*vorbis_analysis_blockout_func)(vorbis_dsp_state *v,vorbis_block *vb);
	int (*vorbis_synthesis_headerin_func)(vorbis_info *vi,vorbis_comment *vc,ogg_packet *op);
	int (*vorbis_bitrate_addblock_func)(vorbis_block *vb);
	int (*vorbis_bitrate_flushpacket_func)(vorbis_dsp_state *vd,ogg_packet *op);

	// ogg.dll function pointer
	int (*ogg_stream_init_func)(ogg_stream_state *os,int serialno);
	int (*ogg_stream_packetin_func)(ogg_stream_state *os, ogg_packet *op);
	int (*ogg_stream_flush_func)(ogg_stream_state *os, ogg_page *og);
	int (*ogg_stream_pageout_func)(ogg_stream_state *os, ogg_page *og);
	int (*ogg_page_eos_func)(ogg_page *og);
	int (*ogg_stream_clear_func)(ogg_stream_state *os);

};


#endif
