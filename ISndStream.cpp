/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 Albert L. Faber
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
#include "ISndStream.h"


// CONSTRUCTOR
ISndStream::ISndStream()
	:SndStream()
{
	m_dwBitRate=0;
}

// DESRUCTOR
ISndStream::~ISndStream()
{
}


DWORD ISndStream::Read(PBYTE pbData,DWORD dwNumBytes)
{
	ASSERT(FALSE);
	return 0;
}

DWORD ISndStream::GetTotalTime()
{
	/* total file length in ms */
	return 0;
}

DWORD ISndStream::GetCurrentTime()
{
	/* total file length in ms */
	return 0;
}

LONG ISndStream::Seek( LONG lOff, UINT nFrom )
{
	return -1;
}

void ISndStream::Flush()
{
	return ;
}

void ISndStream::InfoBox( HWND hWnd)
{
	return ;
}

void ISndStream::Pause()
{
	return ;
}

