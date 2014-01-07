/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 Albert L. Faber
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
#include "GenreTable.h"
#include <ALGORITHM>
#include "resource.h"
#include <limits.h>
#include "Util.h"


static CUString WinAmpGenres[][2] =
{
	// GENRE                    CDDB GENRE          NUMBER
	_T("Blues" ),				_T("blues"),		/*   0 */
	_T("Classic Rock" ),		_T("rock"),			/*   1 */
	_T("Country" ),				_T("country"),		/*   2 */
	_T("Dance" ),				_T("rock"),			/*   3 */
	_T("Disco" ),				_T("rock"),			/*   4 */
	_T("Funk" ),				_T("rock"),			/*   5 */
	_T("Grunge" ),				_T("misc"),			/*   6 */
	_T("Hip-Hop" ),				_T("rock"),			/*   7 */
	_T("Jazz" ),				_T("blues"),		/*   8 */
	_T("Metal" ),				_T("jazz"),			/*   9 */
	_T("New Age" ),				_T("blues"),		/*  10 */
	_T("Oldies" ),				_T("newage"),		/*  11 */
	_T("Other" ),				_T("rock"),			/*  12 */
	_T("Pop" ),					_T("rock"),			/*  13 */
	_T("R&B" ),					_T("rock"),			/*  14 */
	_T("Rap" ),					_T("rock"),			/*  15 */
	_T("Reggae" ),				_T("reggae"),		/*  16 */
	_T("Rock" ),				_T("rock"),			/*  17 */
	_T("Techno" ),				_T("rock"),			/*  18 */
	_T("Industrial" ),			_T("misc"),			/*  19 */
	_T("Alternative" ),			_T("newage"),		/*  20 */
	_T("Ska" ),					_T("misc"),			/*  21 */
	_T("Death Metal" ),			_T("rock"),			/*  22 */
	_T("Pranks" ),				_T("misc"),			/*  23 */
	_T("Soundtrack" ),			_T("soundtrack"),	/*  24 */
	_T("Euro-Techno" ),			_T("newage"),		/*  25 */
	_T("Ambient" ),				_T("misc"),			/*  26 */
	_T("Trip-Hop" ),			_T("misc"),			/*  27 */
	_T("Vocal" ),				_T("misc"),			/*  28 */
	_T("Jazz+Funk" ),			_T("jazz"),			/*  29 */
	_T("Fusion" ),				_T("misc"),			/*  30 */
	_T("Trance" ),				_T("misc"),			/*  31 */
	_T("Classical" ),			_T("classical"),	/*  32 */
	_T("Instrumental" ),		_T("rock"),			/*  33 */
	_T("Acid" ),				_T("newage"),		/*  34 */
	_T("House" ),				_T("misc"),			/*  35 */
	_T("Game" ),				_T("misc"),			/*  36 */
	_T("Sound Clip" ),			_T("soundtrack"),	/*  37 */
	_T("Gospel" ),				_T("folk"),			/*  38 */
	_T("Noise" ),				_T("misc"),			/*  39 */
	_T("Alt Rock" ),			_T("rock"),			/*  40 */
	_T("Bass" ),				_T("misc"),			/*  41 */
	_T("Soul" ),				_T("rock"),			/*  42 */
	_T("Punk" ),				_T("newage"),		/*  43 */
	_T("Space" ),				_T("misc"),			/*  44 */
	_T("Meditative" ),			_T("misc"),			/*  45 */
	_T("Instrumental Pop" ),	_T("rock"),			/*  46 */
	_T("Instrumental Rock" ),	_T("rock"),			/*  47 */
	_T("Ethnic" ),				_T("misc"),			/*  48 */
	_T("Gothic" ),				_T("folk"),			/*  49 */
	_T("Darkwave" ),			_T("misc"),			/*  50 */
	_T("Techno-Industrial" ),	_T("rock"),			/*  51 */
	_T("Electronic" ),			_T("rock"),			/*  52 */
	_T("Pop-Folk" ),			_T("folk"),			/*  53 */
	_T("Eurodance" ),			_T("rock"),			/*  54 */
	_T("Dream" ),				_T("misc"),			/*  55 */
	_T("Southern Rock" ),		_T("rock"),			/*  56 */
	_T("Comedy" ),				_T("misc"),			/*  57 */
	_T("Cult" ),				_T("misc"),			/*  58 */
	_T("Gangsta Rap" ),			_T("rock"),			/*  59 */
	_T("Top 40" ),				_T("rock"),			/*  60 */
	_T("Christian Rap" ),		_T("misc"),			/*  61 */
	_T("Pop/Funk" ),			_T("rock"),			/*  62 */
	_T("Jungle" ),				_T("misc"),			/*  63 */
	_T("Native American" ),		_T("folk"),			/*  64 */
	_T("Cabaret" ),				_T("misc"),			/*  65 */
	_T("New Wave" ),			_T("newage"),		/*  66 */
	_T("Psychedelic" ),			_T("misc"),			/*  67 */
	_T("Rave" ),				_T("newage"),		/*  68 */
	_T("Showtunes" ),			_T("misc"),			/*  69 */
	_T("Trailer" ),				_T("misc"),			/*  70 */
	_T("Lo-Fi" ),				_T("misc"),			/*  71 */
	_T("Tribal" ),				_T("misc"),			/*  72 */
	_T("Acid Punk" ),			_T("newage"),		/*  73 */
	_T("Acid Jazz" ),			_T("newage"),		/*  74 */
	_T("Polka" ),				_T("folk"),			/*  75 */
	_T("Retro" ),				_T("rock"),			/*  76 */
	_T("Musical" ),				_T("misc"),			/*  77 */
	_T("Rock & Roll" ),			_T("rock"),			/*  78 */
	_T("Hard Rock" ),			_T("rock"),			/*  79 */
	_T("Folk" ),				_T("folk"),			/*  80 */
	_T("Folk-Rock" ),			_T("folk"),			/*  81 */
	_T("National Folk" ),		_T("folk"),			/*  82 */
	_T("Swing" ),				_T("misc"),			/*  83 */
	_T("Fast Fusion" ),			_T("misc"),			/*  84 */
	_T("Bebob" ),				_T("misc"),			/*  85 */
	_T("Latin" ),				_T("rock"),			/*  86 */
	_T("Revival" ),				_T("misc"),			/*  87 */
	_T("Celtic" ),				_T("folk"),			/*  88 */
	_T("Bluegrass" ),			_T("misc"),			/*  89 */
	_T("Avantgarde" ),			_T("misc"),			/*  90 */
	_T("Gothic Rock" ),			_T("rock"),			/*  91 */
	_T("Progressive Rock" ),	_T("rock"),			/*  92 */
	_T("Psychedelic Rock" ),	_T("rock"),			/*  93 */
	_T("Symphonic Rock" ),		_T("rock"),			/*  94 */
	_T("Slow Rock" ),			_T("rock"),			/*  95 */
	_T("Big Band" ),			_T("misc"),			/*  96 */
	_T("Chorus" ),				_T("misc"),			/*  97 */
	_T("Easy Listening" ),		_T("rock"),			/*  98 */
	_T("Acoustic" ),			_T("misc"),			/*  99 */
	_T("Humour" ),				_T("misc"),			/*  100 */
	_T("Speech" ),				_T("misc"),			/*  101 */
	_T("Chanson" ),				_T("misc"),			/*  102 */
	_T("Opera" ),				_T("classical"),	/*  103 */
	_T("Chamber Music" ),		_T("misc"),			/*  104 */
	_T("Sonata" ),				_T("classical"),	/*  105 */
	_T("Symphony" ),			_T("classical"),	/*  106 */
	_T("Booty Bass" ),			_T("misc"),			/*  107 */
	_T("Primus" ),				_T("misc"),			/*  108 */
	_T("Porn Groove" ),			_T("misc"),			/*  109 */
	_T("Satire" ),				_T("misc"),			/*  110 */
	_T("Slow Jam" ),			_T("misc"),			/*  111 */
	_T("Club" ),				_T("misc"),			/*  112 */
	_T("Tango" ),				_T("misc"),			/*  113 */
	_T("Samba" ),				_T("rock"),			/*  114 */
	_T("Folklore" ),			_T("folk"),			/*  115 */
	_T("Ballad" ),				_T("rock"),			/*  116 */
	_T("Power Ballad" ),		_T("rock"),			/*  117 */
	_T("Rhythmic Soul" ),		_T("blues"),		/*  118 */
	_T("Freestyle" ),			_T("rock"),			/*  119 */
	_T("Duet" ),				_T("classical"),	/*  120 */
	_T("Punk Rock" ),			_T("rock"),			/*  121 */
	_T("Drum Solo" ),			_T("misc"),			/*  122 */
	_T("Acapella" ),			_T("misc"),			/*  123 */
	_T("Euro-House" ),			_T("rock"),			/*  124 */
	_T("Dance Hall" ),			_T("rock"),			/*  125 */
	_T("Goa" ),					_T("misc"),			/*  126 */
	_T("Drum" ),				_T("rock"),			/*  127 */
	_T("Club-House" ),			_T("rock"),			/*  128 */
	_T("Hardcore" ),			_T("rock"),			/*  129 */
	_T("Terror" ),				_T("misc"),			/*  130 */
	_T("Indie" ),				_T("folk"),			/*  131 */
	_T("Britpop" ),				_T("rock"),			/*  132 */
	_T("Negerpunk" ),			_T("newage"),		/*  133 */
	_T("Polsk Punk" ),			_T("rock"),			/*  134 */
	_T("Beat" ),				_T("rock"),			/*  135 */
	_T("Christian Gangsta" ),	_T("misc"),			/*  136 */
	_T("Heavy Metal" ),			_T("rock"),			/*  137 */
	_T("Black Metal" ),			_T("rock"),			/*  138 */
	_T("Crossover" ),			_T("misc"),			/*  139 */
	_T("Contemporary Christian" ),_T("rock"),		/*  140 */
	_T("Christian Rock" ),		_T("rock"),			/*  141 */
	_T("Merengue" ),			_T("blues"),		/*  142 */
	_T("Salsa" ),				_T("blues"),		/*  143 */
	_T("Trash Metal" ),			_T("rock"),			/*  144 */
	_T("Anime" ),				_T("misc"),			/*  145 */
	_T("JPop" ),				_T("rock"),			/*  146 */
	_T("Synthpop" ),			_T("rock"),			/*  147 */
};


// CONSTRUCTOR
CGenreTable::CGenreTable()
{
}

// DESTRUCTOR
CGenreTable::~CGenreTable()
{
	v_Entries.clear();
}



void CGenreTable::AddDefaults( )
{
	CUString strLang;
	strLang = g_language.GetString( IDS_UNKNOWN );

	int nGenre;

	for ( nGenre = 0; nGenre < sizeof( WinAmpGenres ) / sizeof( WinAmpGenres[0] ); nGenre++ )
	{
		AddEntry( nGenre, WinAmpGenres[ nGenre ][0], WinAmpGenres[ nGenre ][1], false );
	}

	AddEntry( 254,_T( "Data" ),_T( "data" ), false );
	AddEntry( 255,_T( "Unknown" ),_T( "misc" ), false );
}


BYTE CGenreTable::GetID3V1ID( WORD wIdx ) const
{
	ASSERT( wIdx < v_Entries.size() );
	return v_Entries[ wIdx ].nID3V1ID;
}

void CGenreTable::SetID3V1ID( WORD wIdx, BYTE nValue )
{
	ASSERT( wIdx < v_Entries.size() );
	v_Entries[ wIdx ].nID3V1ID = nValue;
}

CUString CGenreTable::GetGenre( WORD wIdx ) const 
{ 
	ASSERT( wIdx < v_Entries.size() );
	return v_Entries[ wIdx ].strGenre;
}

void CGenreTable::SetGenre( WORD wIdx, CUString strValue )
{
	ASSERT( wIdx < v_Entries.size() );
	v_Entries[ wIdx ].strGenre = strValue;
}

CUString CGenreTable::GetCDDBGenre( WORD wIdx ) const
{
	ASSERT( wIdx < v_Entries.size() );
	return v_Entries[ wIdx ].strCDDBGenre;
}

void CGenreTable::SetCDDBGenre( WORD wIdx, CUString strValue )
{
	ASSERT( wIdx < v_Entries.size() );
	v_Entries[ wIdx ].strCDDBGenre = strValue;
}


bool CGenreTable::CanBeModified( WORD wIdx )
{
	ASSERT( wIdx < v_Entries.size() );
	return v_Entries[ wIdx ].bCanBeModified;
}


int CGenreTable::Load( CUString strFileName )
{
	FILE*	pFile = NULL;
	TCHAR	lpszLine[ 255 ] = { '\0',};
	CUString strGenre;
	CUString strCDDBGenre;
	INT		nID3V1ID = -1;

	AddDefaults( );

	pFile = CDexOpenFile( strFileName , _W( "r" ) );

	int g_nNumGenres = 0;

	if ( pFile )
	{
		while ( NULL != _fgetts( lpszLine, sizeof( lpszLine ), pFile ) )
		{
			int i = 0;
			TCHAR* lpszToken = NULL;

			int		nIndex = -1;
			CUString strCategory;
			CUString strCDDBCategory;

			lpszToken = _tcstok( lpszLine, _T( "\t" ) );

			nID3V1ID = -1;

			while ( lpszToken != NULL )
			{
				switch ( i )
				{
					case 0:
						nID3V1ID = _ttoi( lpszToken );
						i++;
					break;
					case 1:
						strGenre = lpszToken;
						i++;
					break;
					case 2:
						strCDDBGenre = lpszToken;
						i++;
					break;
				}
				lpszToken = _tcstok( NULL, _T( "\t" ) );
			}

			if ( (  nID3V1ID ) >= 0 && ( i >= 3 ) )
			{
				AddEntry( nID3V1ID, strGenre, strCDDBGenre, true );
			}
		}

		fclose( pFile );

	}

//	Sort( );

	Save( strFileName );

	return v_Entries.size() ;
}


void CGenreTable::Save( CUString strFileName )
{
	FILE* pFile = NULL;
	TCHAR lpszLine[ 255 ] = { '\0',};
	int i = 0;

    CUStringConvert strCnv;
	
    pFile = CDexOpenFile( strFileName, _W( "w" ) );

	if ( NULL != pFile )
	{
		for ( i = 0 ; i < (int)v_Entries.size(); i++ )
		{
			if ( CanBeModified( i ) )
			{
				CUString strLine;
                CUStringConvert strCnv;

				GENRETABLEENTRY newEntry = v_Entries[i] ;

				strLine.Format( _W( "%d\t%s\t%s\n" ), newEntry.nID3V1ID, (LPCWSTR)newEntry.strGenre, (LPCWSTR)newEntry.strCDDBGenre );

				_fputts( strCnv.ToT( strLine ), pFile );
			}
		}

		fclose( pFile );
	}
}


int CGenreTable::AddEntry( INT nID3V1ID, const CUString& strGenre, const CUString& strCDDBGenre, bool bCanBeModified  )
{
	bool	bAdd = true;
	int		i = 0;

	GENRETABLEENTRY newEntry;

	newEntry.nID3V1ID = nID3V1ID;
	newEntry.strGenre = strGenre;
	newEntry.bCanBeModified = bCanBeModified;
	newEntry.strCDDBGenre = strCDDBGenre;

	newEntry.strGenre.TrimLeft();
	newEntry.strGenre.TrimRight();

	newEntry.strCDDBGenre.TrimLeft();
	newEntry.strCDDBGenre.TrimRight();

	// check if entry is already in this list
	for ( i = 0 ; i < (int)v_Entries.size(); i++ )
	{
		if ( GetID3V1ID( i )  == nID3V1ID  && 
			( 0 == strGenre.CompareNoCase( GetGenre( i ) ) ) )
		{
			bAdd = false;
		}
	}

	if ( true == bAdd )
	{
		v_Entries.push_back( newEntry );
//		Sort();
	}


	return v_Entries.size();
}

int CGenreTable::SearchGenre( CUString strSearch ) const
{
	int i;

	strSearch.TrimLeft();
	strSearch.TrimRight();

	for ( i = 0 ; i < (int)v_Entries.size(); i++ )
	{
		if ( 0 == strSearch.CompareNoCase( GetGenre( i ) ) )
		{
			return i;
		}
	}
	return -1;
}

CUString CGenreTable::GetID3V1GenreString( int nID3TagIdx ) const
{
	int i;
	CUString strRet;

	for ( i = 0 ; i < (int)v_Entries.size(); i++ )
	{
		if ( nID3TagIdx == GetID3V1ID( i ) )
		{
			return GetGenre( i );
		}
	}

	strRet = g_language.GetString( IDS_UNKNOWN );

	return strRet;
}


void CGenreTable::Sort(  )
{
	sort( v_Entries.begin(), v_Entries.end() );
}


int CGenreTable::SearchID3V1ID( int btSearch ) const
{
	int i;

	for ( i = 0 ; i < (int)v_Entries.size(); i++ )
	{
		if ( GetID3V1ID( i )  == btSearch )
		{
			return i;
		}
	}
	return -1;
}

void CGenreTable::FreeWinampRange( DWORD& dwStart, DWORD& dwEnd )
{
	dwEnd = UCHAR_MAX;
	dwStart = sizeof( WinAmpGenres ) / sizeof( WinAmpGenres[0] ) + 1;
}
