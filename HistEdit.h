/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
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


#ifndef HIST_EDIT_H_INCLUDED
#define HIST_EDIT_H_INCLUDED


class CHistEdit : public CEdit
{
	CUString m_strHist;
	int		m_nLines;
	int		m_nHistLines;
	CUString	m_strLogFileName;
	bool	m_bLogEnabled;

public:
	// CONSTRUCTOR
	CHistEdit();

	// DESTRUCTOR
	virtual ~CHistEdit();

	// METHODS
	void AddString(const CUString& strAdd, bool bAddReturn= TRUE);

	void SetNumHistLines(int nValue) {m_nHistLines=nValue;}
	int  GetNumHistLines() const {return m_nHistLines;}
	void SetLogging( const CUString& strFileName, bool bEnable );

};


#endif
