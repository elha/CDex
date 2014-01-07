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


#ifndef CTasks_H_INCLUDED
#define CTasks_H_INCLUDED

#include "ID3Tag.h"
#include "vector"
using namespace std ;


#include "TaskInfo.h"


typedef vector<CTaskInfo> RECTP;
typedef vector<CTaskInfo>::iterator	RECIT;

class CTasks
{
private:
	RECTP			m_vTasks;
//	DWORD			m_dwCurrentTrack;
	CUString			m_strLastError;

public:
	// CONSTRUCTOR
	CTasks();

	// DESTRUCTOR
	virtual ~CTasks();

	// METHODS
	CTaskInfo&		GetTaskInfo( DWORD dwTask );
	DWORD			GetNumTasks() const {return m_vTasks.size();}
	INT				AddTaskInfo( CTaskInfo& newTask );
	void			ClearAll() { m_vTasks.clear(); }
};

#endif