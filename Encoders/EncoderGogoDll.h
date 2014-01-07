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


#ifndef ENCODERGOGODLL_H_INCLUDED
#define ENCODERGOGODLL_H_INCLUDED

#include "./libraries/gogo/include/gogo.h"
#include "Encode.h"

typedef MERET	(*ME_INIT)(void);
typedef MERET	(*ME_SETCONF)(MPARAM mode, UPARAM dwPara1, UPARAM dwPara2 );  
typedef MERET	(*ME_GETCONF)(MPARAM mode, void *para1 ); 
typedef MERET	(*ME_DETECT)(); 
typedef MERET	(*ME_PROCFRAME)(); 
typedef MERET	(*ME_CLOSE)(); 
typedef MERET	(*ME_END)(); 
typedef MERET	(*ME_GETVER)( unsigned long *vercode, char *verstring ); 
typedef MERET	(*ME_HAVEUNIT)( unsigned long *unit ); 



class CEncoderGogoDll:public CEncoder
{
protected:
	ME_INIT					me_init;
	ME_SETCONF				me_setconf;
	ME_GETCONF				me_getconf;
	ME_DETECT				me_detect;
	ME_PROCFRAME			me_procframe; 
	ME_CLOSE				me_close; 
	ME_END					me_end; 
	ME_GETVER				me_getver;
	ME_HAVEUNIT				me_haveunit;
	HINSTANCE				m_hDLL;
	CUString					m_strStreamFileName;
public:
	virtual	CEncoderDlg*	GetSettingsDlg();
//	static MPGE_USERFUNC	Callback;
	static MERET			CallBack(void *buf, unsigned long nLength );
	// CONSTRUCTOR
	CEncoderGogoDll();

	// DESTRUCTOR
	virtual ~CEncoderGogoDll();

	// METHODS
	void GetDLLVersionInfo();
	virtual CDEX_ERR InitEncoder( CTaskInfo* pTask );
	virtual CDEX_ERR OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels);
	virtual CDEX_ERR EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples);
	virtual CDEX_ERR CloseStream();
	virtual CDEX_ERR DeInitEncoder();

};



#endif
