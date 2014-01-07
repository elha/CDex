/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2007 Albert L. Faber
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


#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#define ForceCloseHandle(x) if (x) { CloseHandle(x); x = NULL; }

#include "CDRip/CDRip.h"
#include "Language.h"


void InitUtil(void);
DOUBLE GetMyFreeDiskSpace(CUString& strPath);
BOOL IsWindowsNTOS();

BOOL AsciiOnly( const CUString& strToTest );

BOOL CheckNoFileOverwrite( CWnd* pWnd, const CUString& strFileName, BOOL bAsk, BOOL& bYesToAll, BOOL& bNoToAll);

BOOL LoadDLL(	const CUString& strDllName,
				const CUString& strPath,
				HINSTANCE&		hDll,
				BOOL			bShowWarning,
				BOOL			bUnicodeDLL );

CDEX_ERR CalculateFileCrc32( const CUString& strFileName, DWORD& dwCrc32 );

CUString GetCDexAppPath();

INT QueryModuleDebugLevel( const CUString& strModule );


extern CLanguage g_language;
extern CUString g_lastErrorString;

void SetLastCDexErrorString( CUString& strValue );

CUString GetLastCDexErrorString();


BOOL UseFlatLookToolBar();

LPCSTR GetDosFileName( const CUString& strPath );
double PerformanceCounterGetDelta( __int64 startTime, __int64 endTime );
__int64 PerformanceCounterGetTime();

ULONG Calculate_CRC_Start( );
ULONG Calculate_CRC_Block( ULONG startCrc, PBYTE dataPtr, DWORD dataSize, BOOL ignoreZeroSamples );
ULONG Calculate_CRC_End( ULONG crc );


int CDexMessageBox( const CUString& strMsg, UINT nType = MB_OK );
FILE* CDexOpenFile( const CUString& strFileName, const CUString& strFlags );
BOOL CDexDeleteFile( const CUString& strFileName );
BOOL CDexCopyFile( const CUString& strSrc, const CUString& strDst , BOOL bFailIfExist = FALSE );
BOOL CDexMoveFile( const CUString& strSrc, const CUString& strDst );
HMODULE CDexLoadLibrary( const CUString& strLibraryFileName );

BOOL CDexFileExist( const CUString& strFileName);


#endif

