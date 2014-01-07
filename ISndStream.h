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


#ifndef ISndStream_H_INCLUDED
#define ISndStream_H_INCLUDED

#include "SndStream.h"

class ISndStream:public SndStream
{
public:
	// DESTRUCTOR
	virtual ~ISndStream();

	// MUTATORS
	virtual DWORD	Read(PBYTE pbData,DWORD dwNumBytes);
	virtual DWORD	GetTotalTime();						/* total file length in ms */
	virtual DWORD	GetCurrentTime();						/* total file length in ms */
	virtual LONG	Seek( LONG lOff, UINT nFrom );
	virtual void	Flush();
	virtual void	Pause();
	virtual VOID	InfoBox( HWND hWnd );

	// ACCESSORS
	virtual DWORD	GetBitRate(void) {return m_dwBitRate;}
	virtual void	SetBitRate(DWORD nValue) {m_dwBitRate=nValue;}

protected:
	// CONSTRUCTOR, ABSTRACT CLASS
	ISndStream();
private:
	DWORD			m_dwBitRate;
};




#endif