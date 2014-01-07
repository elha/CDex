/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2001 Albert L. Faber
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


#ifndef TEXTFILE_H_INCLUDED
#define TEXTFILE_H_INCLUDED


class CTextFile : public CObject
{
	DECLARE_DYNAMIC(CTextFile)
	
public:
	enum TEXTFILE_MODE { modeRead, modeWrite, modeAppend };
	enum TEXTFILE_SEEK { seekBegin, seekEnd, seekCurrent };


	CTextFile( );
	virtual ~CTextFile();
	virtual BOOL  ReadString( CUString& rString );
	virtual void  WriteString( const CUString& qString );
	virtual void  SetWriteAsDosFile( bool bValue  );
	virtual void  Close( );
	virtual BOOL  Open( LPCTSTR lpszFileName, TEXTFILE_MODE nOpenFlags );
	virtual int   Seek( long offset, TEXTFILE_SEEK seekMode );
	virtual int   GetPosition();
private:
	bool			m_bWriteAsDosFile;
	bool			m_bWriteEof;
	FILE*			m_pFile;
	static const CUString	m_strCRLF;
	static const CUString	m_strLF;
};

#endif
