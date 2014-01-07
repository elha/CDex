/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 2000 - 2007 Albert L. Faber
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


#ifndef ISNDSTREAMWINAMP_H_INCLUDED
#define ISNDSTREAMWINAMP_H_INCLUDED

#include "ISndStream.h"

extern "C"
{
	#include "in2.h"
//	#include "out.h"
}

typedef struct WINAMPPLUGINPROP_TAG
{
	CUString		strExt;
	CUString		strFileName;
	HINSTANCE	hDll;
	In_Module*	pModule;

}	WINAMPPLUGINPROP;


class ISndStreamWinAmp : public ISndStream
{
	DWORD				m_dwSamples;
	WINAMPPLUGINPROP	m_WinAmpProp;
public:
	ISndStreamWinAmp();
	virtual ~ISndStreamWinAmp();
	virtual BOOL	OpenStream( const CUString& strFileName);
	virtual DWORD	Read(PBYTE pbData,DWORD dwNumBytes);
	virtual BOOL	CloseStream();
	virtual __int64	Seek( __int64 ddwOffset, UINT nFrom );
	virtual void	Flush();
	virtual void	Pause();
	
	virtual DWORD	GetTotalTime();
	virtual DWORD	GetCurrentTime();
	virtual VOID	InfoBox( HWND hWnd );

private:
	int		m_dwCurrentFilePos;
	int		m_dwTotalFileSize;

	void	FlushFIFO( );


};


void InitWinAmpPlugins( HWND hWnd );
void DeInitWinAmpPlugins(  );

void WinampPlugInFinished();
int GetNumWinampPlugins();
CUString GetWinampPluginInfo( int i );
CUString GetWinampPluginFileName( int i );
void WinampPluginAbout( int i, HWND hWnd );
void WinampPluginConfigure( int i, HWND hWnd );
CUString GetWinampFileNameExt();

void WinampPluginInfoBox( LPSTR lpszFileName, HWND hWnd );

#endif