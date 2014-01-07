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


#ifndef ENCODERDOS_H_INCLUDED
#define ENCODERDOS_H_INCLUDED

#include "Encode.h"

class CEncoderDos : public CEncoder
{
public:
	// CONSTRUCTOR
	CEncoderDos();

	// DESTRUCTOR
	virtual ~CEncoderDos();

	// METHODS
	virtual CDEX_ERR InitEncoder( CTaskInfo* pTask );
	virtual CDEX_ERR OpenStream(CUString strOutFileName,DWORD dwSampleRate,WORD nChannels);
	virtual CDEX_ERR EncodeChunk(PSHORT pbsInSamples,DWORD dwNumSamples);
	virtual CDEX_ERR CloseStream();
	virtual CDEX_ERR DeInitEncoder();
	virtual CDEX_ERR DosEncode(	const CUString& strInFileName,
								const CUString& strInFileExt, 
								const CUString& strOutFileName,
								const CUString& strOutDir,
								INT&  nPercent,
								BOOL& bAbort);
	virtual	CEncoderDlg*	GetSettingsDlg();

	virtual void SaveSettings();
	virtual void LoadSettings();

	BOOL GetAddStdinWavHeader() const { return m_bSendStdinWavHeader;}
	void SetAddStdinWavHeader( BOOL bValue ) { m_bSendStdinWavHeader = bValue;}

protected:
	DWORD CalculateChildPriorityClass();

	// MEMBERS
	HANDLE			m_hReadPipe;
	HANDLE			m_hWritePipe;
	CChildProcess*	m_pChildProcess;
	CUString			m_strTempFileName;
	CUString			m_strOutFileName;
	BOOL			m_bSendStdinWavHeader;
	// METHODS
	CUString PrepareCommandLine();

};

class CAacEncoder:public CEncoderDos
{
public:
	// CONSTRUCTOR
	CAacEncoder();

	// DESTRUCTOR
	virtual ~CAacEncoder();
	virtual CEncoderDlg* GetSettingsDlg();
	virtual CDEX_ERR DosEncode(	const CUString& strInFileName,
								const CUString& strInFileExt, 
								const CUString& strOutFileName,
								const CUString& strOutDir,
								INT&  nPercent,
								BOOL& bAbort);
};

class CAacPsyEncoder:public CAacEncoder
{
public:
	// CONSTRUCTOR
	CAacPsyEncoder();

	// DESTRUCTOR
	virtual ~CAacPsyEncoder();
};


#endif /* ENCODERDOS_H_INCLUDED */

