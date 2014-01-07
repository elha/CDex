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


#ifndef TASK_H_INCLUDED
#define TASK_H_INCLUDED

#include "Encode.h"
#include "TagData.h"

class CTaskInfo
{
public:
static const double NORM_TUNE_FACTOR;
static const double NORM_DEFAULT_VALUE;

private:
    CTagData        m_tagData;

	CUString			m_strName;
	CUString			m_strNonSplitArtist;
	DWORD			m_dwStartSector;
	DWORD			m_dwEndSector;
	DWORD			m_dwTrack;
	DWORD			m_dwLengthInMs;
	CUString			m_strOutDir;
	CUString			m_strOutName;
	ENCODER_TYPES	m_nEncoderType;
	BOOL			m_bRip;
	BOOL			m_bNormalize;
	BOOL			m_bConvert;
	BOOL			m_bDeleteWav;
	BOOL			m_bRiffWav;
	int				m_nPeakValue;
	double			m_dNormalizationFactor;
	RAW_TOC_TYPE	m_btaRawToc;
	bool			m_bValidTagData;
public:

	// CONSTRUCTOR
	CTaskInfo();
	
	// DESTRUCTOR
	virtual ~CTaskInfo();

    CTagData& GetTagData() {return m_tagData;}
    void     SetTagData( const CTagData& value) {m_tagData = value;}

	// METHODS

	CUString		GetInDir( ) const ;
	void		SetInDir( CUString strDir );

	CUString		GetOutDir( ) const;

	CUString		GetFullFileName( ) const;
	CUString		GetFileName( ) const;
	CUString		GetFullFileNameNoExt( ) const ;
	CUString		GetFileNameNoExt( ) const ;
	CUString		GetFileExt( ) const ;
	void		SetFileExt( CUString str);

	CUString		GetOutFullFileName( ) const;
	CUString		GetOutFileName( ) const;
	CUString		GetOutFullFileNameNoExt( ) const ;
	CUString		GetOutFileNameNoExt( ) const ;
	CUString		GetOutFileExt( ) const ;
	void		SetOutFileExt( CUString str);

	CUString		GetNonSplitArtist( ) const;

    DWORD		GetStartSector( ) const;
	DWORD		GetEndSector( ) const;

	DWORD		GetLengthInMs( ) const;

	void		SetFullFileName( CUString str) { m_strName = str;}
	void		SetOutFullFileName( const CUString& str);
	void		SetOutDir( CUString str) { m_strOutDir = str;}
	
	void		SetNonSplitArtist( CUString str) { m_strNonSplitArtist=str;}

	void		SetStartSector( DWORD val) { m_dwStartSector=val;}
	void		SetEndSector( DWORD val) { m_dwEndSector=val;}
	void		SetLengthInMs( DWORD val) { m_dwLengthInMs=val;}

	void		BuildFileName( CUString& strName,CUString& strDir);

    ENCODER_TYPES	GetEncoderType( ) const { return  m_nEncoderType; }
	void			SetEncoderType(  ENCODER_TYPES nType );

	BOOL		GetRip() const			{ return m_bRip; }
	void		SetRip( BOOL nValue )			{ m_bRip = nValue; }

	BOOL		GetNormalize() const	{ return m_bNormalize; }
	void		SetNormalize( BOOL nValue )		{ m_bNormalize = nValue; }

	BOOL		GetConvert() const		{ return m_bConvert; }
	void		SetConvert( BOOL nValue )		{ m_bConvert = nValue; }

	BOOL		GetDeleteWav() const	{ return m_bDeleteWav; }
	void		SetDeleteWav( BOOL nValue )		{ m_bDeleteWav = nValue; }

	BOOL		GetRiffWav() const		{ return m_bRiffWav; }
	void		SetRiffWav( BOOL nValue )		{ m_bRiffWav = nValue; }

	void		SetPeakValue( int nValue )		{ m_nPeakValue = nValue; }
	int			GetPeakValue( ) const;

	void		SetNormalizationFactor( DOUBLE dValue ) { m_dNormalizationFactor = dValue;}
	DOUBLE		GetNormalizationFactor( ) const { return m_dNormalizationFactor;}

	void		SetValidTagData( bool value ) { m_bValidTagData = value;}
	bool		GetValidTagData( ) const { return m_bValidTagData;}

};

#endif
