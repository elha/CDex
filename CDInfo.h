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


#ifndef CDINFO_INCLUDED
#define CDINFO_INCLUDED

#define TRACKSPERSEC 75
#define CB_CDDASECTORSIZE 2352


#include "config.h"
#include "TagData.h"

#pragma pack(push,1)


enum CDTEXT_PACK_TYPE
{
	CDT_TRACK_TITLE	=0x80, // Title of Album name(ID2=00h) or Track Titles (ID2=01h...63h)
	CDT_PERFORMER	=0x81, // Name(s) of the performer(s) (in ASCII)
	CDT_SONGWRITER	=0x82, // Name(s) of the songwriter(s) (in ASCII)
	CDT_COMPOSER	=0x83, // Name(s) of the composer(s) (in ASCII)
	CDT_ARRANGER	=0x84, // Name(s) of the arranger(s) (in ASCII)
	CDT_MESSAGE		=0x85, // Message(s) from content provider and/or artist (in ASCII)
	CDT_DISC_ID		=0x86, // Disc Identification information
	CDT_GENRE		=0x87, // Genre Identification and Genre information
	CDT_TOC_INFO	=0x88, // Table of Content information
	CDT_SEC_TOC_INFO=0x89, // Second Table of Content information
	CDT_RESERVED_1	=0x8A, // Reserved
	CDT_RESERVED_2	=0x8B, // Reserved
	CDT_RESERVED_3	=0x8C, // Reserved
	CDT_RESERVED_4	=0x8D, // Reserved for content provider only
	CDT_UPC_EAN		=0x8E, // UPC/EAN code of the album and ISRC code of each track
	CDT_SIZE_BLOCK	=0x8F  // Size information of the Block
};

typedef struct CDTEXTPACK_TAG
{
  BYTE packType;
  BYTE trackNumber;
  BYTE sequenceNumber;

  BYTE characterPosition:4;		// character position
  BYTE block			:3;		// block number 0..7
  BYTE bDBC				:1;		// double byte character

  BYTE data[12];
  BYTE crc0;
  BYTE crc1;
} CDTEXTPACK;

#pragma pack(pop)


class PlayList:public CObject
{
private:
	CUString m_strFileName;
	CUString m_strOutDir;
	CUString m_strPlayListDir;
	CUString m_strFileDir;

public:
	PlayList( const CUString& strPlayListFileName );
	void AddEntry( CUString Entry );
	void AddM3UEntry( CUString Entry );
	void AddPLSEntry( CUString Entry );

private:
	CUString MakeEntry(	const CUString& strListName, 
						const CUString& strEntry );

};


class CCDTrackInfo: public CObject
{
//private:
public:
	BYTE	m_reserved1;
	BYTE	m_btFlags;
	BYTE	m_btTrack;
	BYTE	m_reserved2;
	DWORD	m_dwStartSector;
	CUString	m_strTrackName;
	CUString	m_strExtTrackName;
	CUString	m_strISRC;
	int		m_nCDDBType;
    DWORD   m_dwGap;

public:
	CCDTrackInfo();
	DWORD	GetStartSector()	const {return m_dwStartSector;}
	BYTE	GetFlags()			const {return m_btFlags;}
	BYTE	GetTrack()			const {return m_btTrack;}

	CUString	GetTrackName()		const {return m_strTrackName;}
	CUString	GetExtTrackName()	const {return m_strExtTrackName;}
	CUString	GetISRC()	        const {return m_strISRC;}
    DWORD   GetGap()            const {return m_dwGap;}

	void	SetTrack(BYTE btTrack)				    {m_btTrack=btTrack;}
	void	SetTrackName(const CUString& strValue)   {m_strTrackName=strValue;}
	void	SetExtTrackName(const CUString& strValue){m_strExtTrackName=strValue;}
	void	SetStartSector(DWORD dwStartSector)	    {m_dwStartSector=dwStartSector;}
	void    SetISRC(const CUString& strValue)        {m_strISRC=strValue;}
	void	SetGap(DWORD value) 				    {m_dwGap = value;}
private:
};


class CDInfo: public CObject
{
private:
	INT				m_nNumTracks;
	CCDTrackInfo	m_TrackInfo[MAXTRK];
	CUString			m_strArtist;
	CUString			m_strTitle;
	CUString			m_strExtTitle;
	DWORD			m_dwVolID;
	DWORD			m_dwDiscID;
	CUString			m_strYear;
	CUString			m_strGenre;
	DWORD			m_dwTotalSecs;
	INT				m_nRevision;
	CUString			m_strSubmission;
	CUString			m_strVolID;
	CUString			m_strCDDBCat;
	CUString			m_strMCDI;
	CUString			m_strMCN;

	RAW_TOC_TYPE	m_btaRawToc;
    vector<CTagData>    m_vTagData;
public:
	// CONSTRUCTORS
	CDInfo();
	// DESTRUCTOR
	~CDInfo();

	CUString			GetCDDBCat();
	void			SetCDDBCat( const CUString& strCDDBCat );

	INT				GetNumTracks() const {return m_nNumTracks;}
	VOID			SetNumTracks(WORD wValue) {m_nNumTracks=wValue;}
	void			DumpToc(LPBYTE pbtBuffer,int nSize);
	LONG			GetEndSector(int p_track);
	LONG			GetStartSector(int p_track);
	CUString			GetTrackDuration(int nTrack);
	CUString			GetStartTime(int nTrack);
	DWORD			GetSize(int nTrack);
	BOOL			IsAudioTrack(int p_track);
	DWORD			CalculateDiscID();

	WORD			GetTrack(int iTrackIndex) const {return m_TrackInfo[iTrackIndex].GetTrack();}
	void			SetTrack(int iTrackIndex, BYTE btTrack) { m_TrackInfo[iTrackIndex].SetTrack(btTrack);}

	DWORD			GetGap(int iTrackIndex) const {return m_TrackInfo[iTrackIndex].GetGap();}
	void			SetGap(int iTrackIndex, DWORD value ) { m_TrackInfo[iTrackIndex].SetGap(value);}

	CUString			GetTrackName(int iTrackIndex) const {return m_TrackInfo[iTrackIndex].GetTrackName();}
	void 			SetTrackName( const CUString& strValue, int iTrackIndex);

    CUString			GetISRC(int iTrackIndex) const {return m_TrackInfo[iTrackIndex].GetISRC();}

	CUString			GetExtTrackName(int iExtTrackIndex) const {return m_TrackInfo[iExtTrackIndex].GetExtTrackName();}
	void 			SetExtTrackName( const CUString& strValue,int iExtTrackIndex );

	CUString			GetArtist() const {return m_strArtist;}
	void			SetArtist( const CUString& strValue );
	
	CUString			GetTitle() const {return m_strTitle;}
	void			SetTitle( const CUString& strValue);

	CUString			GetExtTitle() const {return m_strExtTitle;}
	void			SetExtTitle( const CUString& strValue )	  {m_strExtTitle=strValue;}

	CUString			GetYear()	const {return m_strYear;}
	void			SetYear(CUString strValue) { m_strYear = strValue; }

	CUString			GetGenre()	const {return m_strGenre;}
	void			SetGenre(CUString strValue);

	int				GetRevision()	const {return m_nRevision;}
	void			SetRevision(int nValue)	  {m_nRevision=nValue;}

	CUString			GetSubmission()	const {return m_strSubmission;}
	void			SetSubmission(CUString strValue)	  {m_strSubmission=strValue;}

	void			SetStartSector(	int p_track, DWORD dwOffset);

	void			SetTotalSecs( DWORD dwSecs) {m_dwTotalSecs=dwSecs;}

	int				FirstTrack();

	void			ReadCDInfo();
	void			SaveCDInfo();

	DWORD			GetDiscID() const {return m_dwDiscID;}
	void			SetDiscID(DWORD dwID) {m_dwDiscID=dwID;}
	DWORD			GetVolID() const {return m_dwVolID;}

	CUString			GetMCDI() const {
		CUString strTmp;
		strTmp = m_strMCDI;
		return strTmp; }

    CUString         GetMCN() const { return m_strMCN; };

	BYTE*			GetRawToc() const { return (BYTE*)m_btaRawToc; }
	void			SetRawToc( BYTE* btaRawToc ) { memcpy( m_btaRawToc, btaRawToc, sizeof( RAW_TOC_TYPE ) ); }

	BOOL			IsLastAudioTrack( int p_track );
	LONG			GetSaveEndSector(int p_track);

	void			Init();
	BOOL			ReadCDPlayerIni();
	BOOL			ReadCDText();
	void			SaveCDPlayerIni();
	BOOL			ReadLocalCDDB( );
	BOOL			ReadRemoteCDDB( CWnd* pWnd, volatile BOOL& bAbort  );
	BOOL			SubmitRemoteCDDB( CWnd* pWnd, volatile BOOL& bAbort  );
	BOOL			ReadRemoteCDDBBatch( CWnd* pWnd, volatile BOOL& bAbort );
	DWORD			GetTotalSecs() const {return m_dwTotalSecs;}
	CDEX_ERR		ReadToc();
	BOOL			ReadFromWinampDB();
    CDEX_ERR        ReadUpcAndISRC();
private:
	BOOL			ReadWinampDBString( CFile& cFile, CUString& strRead );


};

UINT ReadRemoteCDDBThreadFunc( void* pParam, CWnd* pWnd, volatile BOOL& bAbort );
UINT SubmitRemoteCDDBThreadFunc( void* pParam, CWnd* pWnd, volatile BOOL& bAbort );
UINT BatchRemoteCDDBThreadFunc( void* pParam, CWnd* pWnd, volatile BOOL& bAbort );


#endif
