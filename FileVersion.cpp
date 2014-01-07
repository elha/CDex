/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) Manuel Laflamme
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


// FileVersion.cpp: implementation of the CFileVersion class.
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "FileVersion.h"

#pragma comment(lib, "version")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

CFileVersion::CFileVersion() 
{ 
    m_lpVersionData = NULL;
    m_dwLangCharset = 0;
}

CFileVersion::~CFileVersion() 
{ 
    Close();
} 

void CFileVersion::Close()
{
    delete[] m_lpVersionData; 
    m_lpVersionData = NULL;
    m_dwLangCharset = 0;
}

BOOL CFileVersion::Open( LPCTSTR lpszModuleName )
{
    ASSERT(_tcslen(lpszModuleName) > 0);
    ASSERT(m_lpVersionData == NULL);

    // Get the version information size for allocate the buffer
    DWORD dwHandle;     
    DWORD dwDataSize = ::GetFileVersionInfoSize((LPTSTR)lpszModuleName, &dwHandle); 
    if ( dwDataSize == 0 ) 
        return FALSE;

    // Allocate buffer and retrieve version information
    m_lpVersionData = new BYTE[dwDataSize]; 
    if (!::GetFileVersionInfo((LPTSTR)lpszModuleName, dwHandle, dwDataSize, 
	                          (void**)m_lpVersionData) )
    {
        Close();
        return FALSE;
    }

    // Retrieve the first language and character-set identifier
    UINT nQuerySize;
    DWORD* pTransTable;
    if (!::VerQueryValue(m_lpVersionData, _T("\\VarFileInfo\\Translation"),
                         (void **)&pTransTable, &nQuerySize) )
    {
        Close();
        return FALSE;
    }

    // Swap the words to have lang-charset in the correct format
    m_dwLangCharset = MAKELONG(HIWORD(pTransTable[0]), LOWORD(pTransTable[0]));

    return TRUE;
}

CUString CFileVersion::QueryValue(  LPCTSTR lpszValueName, 
                                    DWORD dwLangCharset /* = 0*/)
{
    CUString strValue;
    // Must call Open() first

    ASSERT(m_lpVersionData != NULL);

    if ( m_lpVersionData == NULL )
        return (CUString)_W("");

    // If no lang-charset specified use default
    if ( dwLangCharset == 0 )
        dwLangCharset = m_dwLangCharset;

    // Query version information value
    UINT nQuerySize;
    LPVOID lpData;
    // CString strValue, 
    CString strBlockName;
    strBlockName.Format(_T("\\StringFileInfo\\%08lx\\%s"), 
	                     dwLangCharset, lpszValueName);
    if ( ::VerQueryValue((void **)m_lpVersionData, strBlockName.GetBuffer(0), 
	                     &lpData, &nQuerySize) )

    strBlockName.ReleaseBuffer();

    strValue = (LPCTSTR)lpData;
    
    return strValue;
}

BOOL CFileVersion::GetFixedInfo(VS_FIXEDFILEINFO& vsffi)
{
    // Must call Open() first
    ASSERT(m_lpVersionData != NULL);
    if ( m_lpVersionData == NULL )
        return FALSE;

    UINT nQuerySize;
	VS_FIXEDFILEINFO* pVsffi;
    if ( ::VerQueryValue((void **)m_lpVersionData, _T("\\"),
                         (void**)&pVsffi, &nQuerySize) )
    {
        vsffi = *pVsffi;
        return TRUE;
    }

    return FALSE;
}

CUString CFileVersion::GetFixedFileVersion()
{
    CUString strVersion;
	VS_FIXEDFILEINFO vsffi;

    if ( GetFixedInfo(vsffi) )
    {
        strVersion.Format ( _W( "%u,%u,%u,%u" ), HIWORD( vsffi.dwFileVersionMS ), LOWORD( vsffi.dwFileVersionMS ), HIWORD( vsffi.dwFileVersionLS ), LOWORD( vsffi.dwFileVersionLS ) );
    }
    return strVersion;
}

CUString CFileVersion::GetFixedProductVersion()
{
    CUString strVersion;
	VS_FIXEDFILEINFO vsffi;

    if ( GetFixedInfo( vsffi ) )
    {
        strVersion.Format ( _W( "%u,%u,%u,%u" ), HIWORD( vsffi.dwProductVersionMS ), LOWORD( vsffi.dwProductVersionMS ), HIWORD( vsffi.dwProductVersionLS ), LOWORD( vsffi.dwProductVersionLS ) );
    }

    return strVersion;
}
