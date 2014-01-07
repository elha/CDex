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


#ifndef ENCODERWMA8DLL_H_INCLUDED
#define ENCODERWMA8DLL_H_INCLUDED

#include "Encode.h"

#include "WmaEncoder.h"
#include "WavToWma.h"

#define WMA8 (1)

class CEncoderWMA8Dll : public CEncoder
{
	WavToWma	m_Encoder;
    ContentDesc m_CntDesc;

	void*							m_pCODEC;
	LPCreateNewCompressionObject	m_pCreateNewCompressionObject;
	LPDeleteCompressionObject		m_pDeleteCompressionObject;
	LPANewDataBlockArrived			m_pANewDataBlockArrived;
	LPShowConfigBit					m_pShowConfigBit;
	LPRemoveConfigBit				m_pRemoveConfigBit;
	LPGetExtensionYouCreate			m_pGetExtensionYouCreate;
	LPSetIDTagElement				m_pSetIDTagElement;
	LPSetSettings					m_pSetSettings;
	HINSTANCE						m_hDLL;

	CUString							m_strStreamFileName;

public:
	// CONSTRUCTOR
	CEncoderWMA8Dll();
	virtual ~CEncoderWMA8Dll();
	virtual	CEncoderDlg*	GetSettingsDlg();
	virtual CDEX_ERR InitEncoder( CTaskInfo* pTask );
	virtual CDEX_ERR OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels);
	virtual CDEX_ERR EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples);
	virtual CDEX_ERR CloseStream();
	virtual CDEX_ERR DeInitEncoder();
};


#endif
