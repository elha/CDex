/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
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


#ifndef ENCODERDOSMPP_H_INCLUDED
#define ENCODERDOSMPP_H_INCLUDED

#include "EncoderDos.h"

class CEncoderDosMpp:public CEncoderDos
{
public:
	// CONSTRUCTOR
	CEncoderDosMpp();

	// DESTRUCTOR
	virtual ~CEncoderDosMpp();

	// OPERATIONS
	virtual CEncoderDlg* GetSettingsDlg();
	virtual CDEX_ERR DosEncode(	const CUString& strInFileName,
								const CUString& strInFileExt, 
								const CUString& strOutFileName,
								const CUString& strOutDir,
								INT&  nPercent,
								BOOL& bAbort);
};


#endif /* ENCODERDOSMPP_H_INCLUDED */

