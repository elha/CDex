/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
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


#ifndef ENCODERFLACDLL_H_INCLUDED
#define ENCODERFLACDLL_H_INCLUDED

#include "Encode.h"
// #include "vorbis/vorbisenc.h"

#include "FLAC/assert.h"
#include "FLAC/file_encoder.h"
#include "FLAC/seekable_stream_encoder.h"
#include "FLAC/stream_encoder.h"
#include "FLAC/metadata.h"

class CEncoderFlacDll : public CEncoder
{
protected:
	HINSTANCE		m_hLibOggFlacDLL;
	HINSTANCE		m_hFlacDll;

	CUString			m_strStreamFileName;

	FLAC__SeekableStreamEncoder*			m_pStreamEncoder;
	FLAC__SeekableStreamEncoderState		m_StreamEncoderState;
//	FLAC__SeekableStreamEncoderState state_;
//	FLAC__StreamEncoderState state__;
//	FLAC__StreamDecoderState dstate;
	FLAC__int32* m_piInputStream;
	CUString m_strFullOutputName;

public:
	virtual	CEncoderDlg*	GetSettingsDlg();
//	void stream_encoder_metadata_callback_(const FLAC__seekable_streamEncoder *encoder, const FLAC__seekable_streamMetadata *metadata, void *client_data);

	// CONSTRUCTOR
	CEncoderFlacDll();

	// DESTRUCTOR
	virtual ~CEncoderFlacDll();

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
	CDEX_ERR AddUtfTag( FLAC__StreamMetadata *block, const CUString& strField, const CUString& strFieldValue );

	static FLAC__StreamEncoderWriteStatus EncoderWriteCallback(
		const FLAC__SeekableStreamEncoder *encoder, 
		const FLAC__byte buffer[],
		unsigned bytes,
		unsigned samples,
		unsigned current_frame,
		void *client_data );

	static FLAC__SeekableStreamEncoderSeekStatus stream_encoder_seek_callback_(const FLAC__SeekableStreamEncoder *encoder, FLAC__uint64 absolute_byte_offset, void *client_data);
	static FLAC__SeekableStreamEncoderTellStatus stream_encoder_tell_callback_(const FLAC__SeekableStreamEncoder *encoder, FLAC__uint64 *absolute_byte_offset, void *client_data);
	void  WriteMetaData();

};


#endif
