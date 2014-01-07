/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2007 Georgy Berdyshev
** Copyright (C) 2007 Albert L. Faber
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


#ifndef TAGDATA_H_INCLUDED
#define TAGDATA_H_INCLUDED

#include "Config.h"
#include <iostream>
#include <stdio.h>

#include "UString.h"
#include "taglib\fileref.h"
#include "taglib\tag.h"

using namespace std;

class CTagData 
{
public:

	// CONSTRUCTOR
	CTagData();

	// MUTATORS
	CUString GetTitle() const { return m_strTitle; }
	CUString GetArtist() const { return m_strArtist; }
	CUString GetAlbum() const { return m_strAlbum; }
	CUString GetYear() const { return m_strYear; }
	DWORD   GetTrackNumber() const { return m_dwTrackNumber; }
	DWORD   GetTotalTracks() const { return m_dwTotalTracks; }

	CUString	GetComment() const		{ return m_strComment; }
	CUString	GetGenre() const		{ return m_strGenre; }
	CUString	GetEncodedBy() const	{ return m_strEncodedBy; }
	// CUString	GetCommentDescr() const { return m_strCommentDescr; }
	CUString	GetMCDI() const			{ return m_strMCDI; }
	CUString GetLengthInMs() const	{ return m_strLengthInMs;}
	BYTE*	GetRawToc() const		{ return (BYTE*)m_btaRawToc; }
	
	void Init( );

	void SetTitle( const CUString& strNew )			{ m_strTitle = strNew; }
	void SetArtist( const CUString& strNew )			{ m_strArtist = strNew;}
	void SetAlbum( const CUString& strNew )			{ m_strAlbum = strNew; }
	void SetYear( const CUString& strNew ) 			{ m_strYear = strNew; }
	void SetComment( const CUString& strNew )		{ m_strComment = strNew; }
	void SetGenre( const CUString& strNew )			{ m_strGenre = strNew; }
	void SetEncodedBy( const CUString& strNew )		{ m_strEncodedBy = strNew; }
	void SetTrackNumber( DWORD value )				{ m_dwTrackNumber=value; }
	void SetTotalTracks( DWORD value )				{ m_dwTotalTracks=value; }
	// void SetCommentDescr( const CUString& strNew )	{ m_strCommentDescr = strNew; }
	void SetMCDI( const CUString& strNew )			{ m_strMCDI = strNew; }
	void SetLengthInMs( const CUString& strNew )		{ m_strLengthInMs = strNew; } 
	void SetRawToc( BYTE* btaRawToc )				{ memcpy( m_btaRawToc, btaRawToc, sizeof( RAW_TOC_TYPE ) ); }

	DWORD GetCDBID( ) const { return m_dwCDBID; } 
    DWORD GetVOLID( ) const { return m_dwVOLID; }

	void SetCDBID( DWORD value ) { m_dwCDBID = value; } 
    void SetVOLID( DWORD value ) { m_dwVOLID = value; }

    LONG GetTrackOffset() const {return m_lTrackOffset;}
	void SetTrackOffset( LONG lValue ) { m_lTrackOffset = lValue; }

private:
	CUString	m_strTitle;
	CUString m_strArtist;
	CUString m_strAlbum;
	CUString m_strYear;
	CUString m_strComment;
	CUString m_strEncodedBy;
	CUString m_strMCDI;
	CUString m_strGenre;
	DWORD   m_dwTrackNumber;
	DWORD   m_dwTotalTracks;
	DWORD   m_dwCDBID;
	DWORD   m_dwVOLID;
	CUString	m_strLengthInMs;
	RAW_TOC_TYPE m_btaRawToc;
	LONG    m_lTrackOffset;
};

#endif