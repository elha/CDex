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


// This article was contributed by Manuel Laflamme . 
// Roberto Rocco already give us a class to get version information from module resources (.exe, .dll, etc.) it works great for the fixed information but you have to know the language-charset of the module for retrieving language dependant information. 
// My class, CFileVersion, allow you to get fixed and the language dependant information. The class use the first language-charset available from the module, so you don't have to worry about it. 
//
// Some of the most use information have predefined method like GetFileVersion(), GetCompanyName(), etc. You can also use QueryValue() if you need to get uncommon or custom data, or if you really want to get data from a specific language-charset. 
//
// You don't have to explicitly add version.lib on your project setting, a pragma directive take care of it. 
//
// FileVersion.h: interface for the CFileVersion class.
// by Manuel Laflamme
//////////////////////////////////////////////////////////////////////

#ifndef __FILEVERSION_H_
#define __FILEVERSION_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CFileVersion
{ 
// Construction
public: 
    CFileVersion();

// Operations	
public: 
    BOOL    Open(LPCTSTR lpszModuleName);
    void    Close();

    CUString QueryValue(LPCTSTR lpszValueName, DWORD dwLangCharset = 0);
    CUString GetFileDescription()  {return QueryValue(_T("FileDescription")); };
    CUString GetFileVersion()      {return QueryValue(_T("FileVersion"));     };
    CUString GetInternalName()     {return QueryValue(_T("InternalName"));    };
    CUString GetCompanyName()      {return QueryValue(_T("CompanyName"));     }; 
    CUString GetLegalCopyright()   {return QueryValue(_T("LegalCopyright"));  };
    CUString GetOriginalFilename() {return QueryValue(_T("OriginalFilename"));};
    CUString GetProductName()      {return QueryValue(_T("ProductName"));     };
    CUString GetProductVersion()   {return QueryValue(_T("ProductVersion"));  };

    BOOL    GetFixedInfo(VS_FIXEDFILEINFO& vsffi);
    CUString GetFixedFileVersion();
    CUString GetFixedProductVersion();

// Attributes
protected:
    LPBYTE  m_lpVersionData; 
    DWORD   m_dwLangCharset; 

// Implementation
public:
    ~CFileVersion(); 
}; 

#endif  // __FILEVERSION_H_

