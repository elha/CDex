/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
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


#ifndef SNDSTREAM_H_INCLUDED
#define SNDSTREAM_H_INCLUDED

#define SEEK_FRAME		100
#define SEEK_PERCENT	101
#define SEEK_TIME		102

#define SNDSTREAM_BUFFER_SIZE 4096

class SndStream
{
public:
	enum SNDSTREAMTYPES {SNDSTREAM_WAV,SNDSTREAM_CD,SNDSTREAM_ADPCM,SNDSTREAM_MPEG, SNDSTREAM_OGG, SNDSTREAM_WMA };
	
	// CONSTRUCTORS
	SndStream( const SndStream& rhs );
	void	CopyStreamInfo( const SndStream& rhs );
	virtual ~SndStream();


	// ACCESSORS
	int		GetPercent() const {return m_nPercent;}
	int		GetBufferSize() const {return m_nBufferSize;}
	int		GetSampleRate() const {return m_nSampleRate;}
	int		GetChannels() const {return m_nChannels;}
	CUString	GetFileExtention() const {return m_strFileExt;}
	CUString	GetFileName() const {return m_strFileName;}
	SNDSTREAMTYPES GetStreamType() const {return m_StreamType;}
	BYTE*	GetModifyBuffer() const { return m_pModifyBuffer;}
	DWORD	GetModifyBufferSize() const { return m_dwModifyBufferSize;}
	int		GetBytesPerSample() const {return m_nBytesPerSample;}
	int		GetPeakValue() const {return m_nPeakValue;}
	
	// SET METHODS
	void	SetPercent( int nValue ) { m_nPercent = nValue; }
	void	SetBufferSize( int nValue ) { m_nBufferSize = nValue; }
	void	SetSampleRate( int nValue ) { m_nSampleRate = nValue; }
	void	SetChannels( int nValue ) { m_nChannels = nValue; }
	void 	SetFileExtention( CUString strValue ) { m_strFileExt=strValue; }
	void	SetFileName( CUString strValue ) { m_strFileName=strValue; }
	void	SetStreamType( SNDSTREAMTYPES nValue ) { m_StreamType = nValue; }
	void	SetBytesPerSample( int nValue ) { m_nBytesPerSample = nValue;}

	// MUTATORS
	virtual BOOL	OpenStream( const CUString& strFileName);
	virtual BOOL	CloseStream();
	virtual LONG	Seek( LONG lOff, UINT nFrom );
			DWORD	ModifyStreamData( BYTE* psData, DWORD dwBytes, double dUpSampleRatio, double dScale );
			void	CalcPeakValue( BYTE* pbtData, DWORD dwBytes );

protected:
	// ABSTRACT CLASS, not allowed to instantiate this class
	SndStream();
private:
	SNDSTREAMTYPES m_StreamType;
	int		m_nBufferSize;
	int		m_nChannels;
	int		m_nSampleRate;
	int		m_nPercent;
	CUString	m_strFileName;
	CUString	m_strFileExt;

	BYTE*	m_pModifyBuffer;
	DWORD	m_dwModifyBufferSize;
	INT		m_nPeakValue;
	INT		m_nBytesPerSample;
};




#endif