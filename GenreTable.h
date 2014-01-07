/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2001 - 2007 Albert L. Faber
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


#ifndef GENRETABLE_H_INCLUDED
#define GENRETABLE_H_INCLUDED

class GENRETABLEENTRY
{
public:
	BYTE		nID3V1ID;
	CUString    strGenre;
	CUString	strCDDBGenre;
	bool		bCanBeModified;
	bool		operator < ( const GENRETABLEENTRY& rhs ) const { if ( strGenre.Compare( rhs.strGenre) > 0 ) return false; else return true; }
};

class CGenreTable : public CObject
{
public:
	// CONSTRUCTOR
	CGenreTable();
	// DESTRUCTOR
	~CGenreTable();

	int Load( CUString strFileName );
	void Save( CUString strFileName );

	DWORD GetNumGenres() const { return v_Entries.size(); }

	BYTE	GetID3V1ID( WORD wIdx ) const;
	void	SetID3V1ID( WORD wIdx, BYTE nValue );

	CUString GetGenre( WORD wIdx ) const;
	void	SetGenre( WORD wIdx, CUString strValue );

	CUString GetCDDBGenre( WORD wIdx ) const;
	void	SetCDDBGenre( WORD wIdx, CUString strValue );

	int		SearchGenre( CUString strSearch ) const;

	bool	CanBeModified( WORD wIdx );

	CUString GetID3V1GenreString( int nID3TagIdx ) const;

	int		SearchID3V1ID( int btSearch ) const;

	void	DeleteAll() { v_Entries.clear(); }
	void	Sort();

	void	AddDefaults();
	int		AddEntry( INT nID3V1ID, const CUString& strGenre, const CUString& strCDDBGenre, bool bCanBeModified  );

	void	FreeWinampRange( DWORD& dwStart, DWORD& dwEnd );

private:
	vector<GENRETABLEENTRY> v_Entries;
};

#endif
