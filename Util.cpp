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


#include "StdAfx.h"
#include "Config.h"
#include "Util.h"
#include "FileExistWarning.h"
#include <limits.h>
#include "ID3Lib/Zlib/include/zlib.h"
#include "FileVersion.h"
//#include "machine.h"

CLanguage		g_language;
CUString			g_lastErrorString;

/*
typedef union _ULARGE_INTEGER_WIN
{
   struct
   {
      DWORD LowPart;
      DWORD HighPart;
   }; DWORDLONG QuadPart;
} ULARGE_INTEGER_WIN;
*/

typedef BOOL (PASCAL *GFDPEX )(LPCTSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER);


// Local variables
static GFDPEX		GetDiskFreeSpaceOSR2=NULL;
static	HINSTANCE	gs_hDLLKernel=NULL;
static	BOOL		gs_bDetermineOSVersion=FALSE;
static	BOOL		gs_bNT=FALSE;

BOOL IsWindowsNTOS()
{
	return gs_bNT;
}

// return the amount of free disk space in KB!
DOUBLE GetMyFreeDiskSpace(CUString& strPath)
{
	CUString strRootDir;

	// Check input parameter
	if (strPath.IsEmpty())
	{
		ASSERT(FALSE);
		return 0;
	}

	// Copy driverletter, colon and back slash
	strRootDir=strPath.Left(3);

	DOUBLE dFreeDiskSpace=0.0;

	// Is this NT or OSR2, then call extented free disk space routine
	if ( NULL != GetDiskFreeSpaceOSR2 )
	{
		ULARGE_INTEGER	FreeBytes; 
		ULARGE_INTEGER	TotalNumberOfBytes;
		ULARGE_INTEGER	TotalNumberOfFreeBytes;

        CUStringConvert strCnv;
		// Make call to kernel 32 dll
		BOOL bReturn = GetDiskFreeSpaceOSR2(	strCnv.ToT( strRootDir ),
												&FreeBytes,
												&TotalNumberOfBytes,
												&TotalNumberOfFreeBytes );

		// If result is OK, then calculate result
		if (bReturn)
		{
			dFreeDiskSpace=(DOUBLE)FreeBytes.LowPart;
			dFreeDiskSpace+=((DOUBLE)FreeBytes.HighPart)*ULONG_MAX;
		}
	}
	else
	{
		// Standard Windows 95
 		DWORD dwSectorsPerCluster; 
		DWORD dwBytesPerSector; 
		DWORD dwNumberOfFreeClusters; 
		DWORD dwTotalNumberOfClusters;

        CUStringConvert strCnv;
        // Try the good old GetDiskFreeSpace
		GetDiskFreeSpace( strCnv.ToT( strRootDir ),&dwSectorsPerCluster,&dwBytesPerSector,&dwNumberOfFreeClusters,&dwTotalNumberOfClusters);
 
		// Calculate free disk space
		dFreeDiskSpace=(DOUBLE)dwNumberOfFreeClusters*(DOUBLE)dwBytesPerSector*(DOUBLE)dwSectorsPerCluster;

	// Get the free disc space as you already do.
	}
	return dFreeDiskSpace;
}


// Local prototypes
static BOOL DetermineOSVersion(void);



void InitUtil(void)
{
	DetermineOSVersion();
}

static BOOL DetermineOSVersion(void)
{
	if (!gs_bDetermineOSVersion)
	{
		OSVERSIONINFO osVersionInfo;

		// Set structure size filed of osVersionInfo
		osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		// Get the actual Version
		GetVersionEx(&osVersionInfo);

		// Check if is OSR2
		if (	osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS &&
				( osVersionInfo.dwBuildNumber & 0xFFFF) > 1000)
		{
			gs_bDetermineOSVersion= TRUE;
		}

		// Always true for Windows NT
		if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			gs_bNT=TRUE;
			gs_bDetermineOSVersion= TRUE;
		}


		if (gs_bDetermineOSVersion)
		{
			gs_hDLLKernel = LoadLibrary( _T( "kernel32.dll" ) );
			if ( gs_hDLLKernel == NULL)
			{
				gs_bDetermineOSVersion= TRUE;
			}
			else
			{
				GetDiskFreeSpaceOSR2 = (GFDPEX)GetProcAddress( gs_hDLLKernel, "GetDiskFreeSpaceEx" );

				if ( NULL == GetDiskFreeSpaceOSR2 )
				{
					gs_bDetermineOSVersion= TRUE;
					FreeLibrary( gs_hDLLKernel );
				}
			}
		}
	}
	return gs_bDetermineOSVersion;
}


BOOL CDexFileExist( const CUString& strFileName)
{
	BOOL bFileExist = FALSE;
	
    FILE* fp= CDexOpenFile( strFileName, _W( "r" ) );

	// check if the file does exist
	if ( NULL != fp )
	{
		bFileExist = TRUE;
		fclose( fp );
	}
    return bFileExist;
}

BOOL CheckNoFileOverwrite( CWnd* pWnd, const CUString& strFileName, BOOL bAsk, BOOL& bYesToAll, BOOL& bNoToAll )
{
	BOOL bFileExist = CDexFileExist( strFileName );
	BOOL bOverwriteFile = FALSE;

	FILE* fp= CDexOpenFile( strFileName, _W( "r" ) );

	// check if the file does exist
	if ( NULL != fp )
	{
		bFileExist = TRUE;
		fclose( fp );
	}

	if ( ( g_config.GetOverwriteExisting() == 1 ) || 
		 ( TRUE == bYesToAll ) )
	{
		return FALSE;
	}

	if ( bFileExist )
	{
		// pop up the overwrite dialog message?
		if ( ( TRUE == bAsk ) && ( bNoToAll == FALSE ) )
		{
			CFileExistWarning dlg;
			dlg.SetFileName( CUString( strFileName ) );

			if ( IDOK == dlg.DoModal() )
			{
				// Did the user request to overwrite all the files?
				bYesToAll = dlg.GetYesToAll();
				return FALSE;
			}
			else
			{
				bNoToAll = dlg.GetNoToAll();
				return TRUE;
			}
		}
		else
		{
			return TRUE;
		}
	}
	return FALSE;
}

BOOL LoadDLL(	const CUString&	strDllName,
				const CUString&	strPath,
				HINSTANCE&		hDll,
				BOOL			bShowWarning,
				BOOL			bUnicodeDLL )
{		
	BOOL bReturn = TRUE;
	CUString strFullPathName;

	hDll = NULL;

	strFullPathName = g_config.GetAppPath() + _W( "\\" ) + strDllName;

    CUStringConvert strCnv;
    hDll = CDexLoadLibrary( strFullPathName );

	CUString strError = GetLastErrorString();


	if ( NULL == hDll )
	{
		strFullPathName = g_config.GetAppPath() + strPath + _W( "\\" ) + strDllName;
		hDll = CDexLoadLibrary( strFullPathName );
	}

	// make an exception for the DBUG environment
	if ( NULL == hDll )
	{
		// Load the correct resource depending on the BUILD settings
#if _DEBUG
#if _UNICODE
		if ( bUnicodeDLL )
		{
			strFullPathName = g_config.GetAppPath() + strPath + _W( "\\Debug_UNICODE\\" ) + strDllName;
		}
		else
#endif
		{
			strFullPathName = g_config.GetAppPath() + strPath + _W( "\\Debug\\" ) + strDllName;
		}
#else
#if _UNICODE
		if ( bUnicodeDLL )
		{
			strFullPathName = g_config.GetAppPath() + strPath + _W( "\\Release_UNICODE\\" ) + strDllName;
		}
		else
#endif
		{
			strFullPathName = g_config.GetAppPath() + strPath + _W( "\\Release\\" ) + strDllName;
		}
#endif
		hDll = CDexLoadLibrary( strFullPathName );
	}

	if ( NULL == hDll )
	{
		if ( TRUE == bShowWarning )
		{
			CUString strLang;
			CUString strMsg;

			strLang = g_language.GetString( IDS_LOADLIBRARY_FAILED );
            strMsg.Format( strLang, (LPCWSTR)strDllName );

			CDexMessageBox( strMsg );
		}

		bReturn = FALSE;
	}

	return bReturn;
}

INT QueryModuleDebugLevel( const CUString& strModule )
{
	int nReturn = 0;

    CUStringConvert strCnv;
    // Get the debug level for the specified module
	// from the CDex.ini file in the Debug section
	nReturn = ::GetPrivateProfileInt(	_T( "Debug" ),
										strCnv.ToT( strModule ),
										0,
										strCnv.ToT( GetCDexAppPath() + _W( "\\CDex.ini" ) ) );

	return nReturn;

}

CUString GetCDexAppPath()
{
	TCHAR  pszAppPath[ MAX_PATH + 1 ];
	TCHAR* pszSearch = NULL;

	memset( pszAppPath, 0, sizeof( pszAppPath ) );

	GetModuleFileName( NULL, pszAppPath, sizeof( pszAppPath ) );

	// find last \ character
	pszSearch = _tcsrchr( pszAppPath, _T( '\\' ) );

	if ( pszSearch )
	{
		// strip everything after the last \ char, \ char including 
		pszSearch[ 0 ] = '\0';
	}

	// Check if this running from a debug environment
#ifdef _UNICODE
	if (_tcsicmp(&pszSearch[ -14], _T( "\\DEBUG_UNICODE"  ) ) == 0 )
	{
		pszSearch[ -14 ] = '\0';
	}
	if (_tcsicmp(&pszSearch[ -16], _T( "\\RELEASE_UNICODE"  ) ) == 0 )
	{
		pszSearch[ -16 ] = '\0';
	}
		OutputDebugString( pszSearch );
#else
	if (_tcsicmp(&pszSearch[ -6], _T( "\\DEBUG"  ) ) == 0 )
	{
		pszSearch[ -6 ] = '\0';
	}
	if (_tcsicmp(&pszSearch[-8], _T( "\\RELEASE" ) ) == 0 )
	{
		pszSearch[ -8 ] = '\0';
	}
#endif
	return pszAppPath;
}

CDEX_ERR CalculateFileCrc32( const CUString& strFileName, DWORD& dwCrc32 )
{
	CDEX_ERR	bReturn = CDEX_OK;
	const int	nBufSize = 1024;
	BYTE		buf[ nBufSize ];

	dwCrc32 = 0;

    CUStringConvert strCnv;
	FILE* pFile= CDexOpenFile( strFileName, _W( "rb" ) );

	if ( NULL == pFile )
	{
		ASSERT( FALSE );
		bReturn = CDEX_FILEOPEN_ERROR;
	}
	else
	{
		bool bDone = false;
		while ( !bDone )
		{
			int nRead = fread( buf, 1, nBufSize, pFile );
			if ( nRead > 0 )
			{
				dwCrc32 = crc32( dwCrc32, buf, nRead );
			}
			if ( nRead < nBufSize )
			{
				bDone = true;
			}
		}

		fclose( pFile );
	}

	return bReturn;
}


BOOL UseFlatLookToolBar()
{
	BOOL bReturn = FALSE;

	// Get COMCTL32 version
	CFileVersion myVersion;

	TCHAR lpszModuleName[ MAX_PATH + 1 ] = { '\0',};

	// Get Comctl32.dll product version
	GetSystemDirectory( lpszModuleName,
						MAX_PATH );
	
    CUStringConvert strCnv;

	myVersion.Open( strCnv.ToT( CUString( lpszModuleName ) + _W( "\\COMCTL32.DLL" ) ) );

	CUString strCompany( myVersion.GetCompanyName() );
	CUString strVersion( myVersion.GetProductVersion() );
	CUString strProductName( myVersion.GetProductName() );
	CUString strFileVersion( myVersion.GetFileVersion() );


	float fVersion = 0.0f;

    _stscanf( strCnv.ToT( strVersion ), _T( "%f" ), & fVersion );

	BOOL	bDisableFlatLook=AfxGetApp()->GetProfileInt( _T( "Debug" ), _T( "DisableFlatLook" ),FALSE);

	if ( ( fVersion >= 4.70 ) && ( FALSE == bDisableFlatLook ) )
	{
		bReturn = TRUE;
	}

	return bReturn;
}

LPCSTR GetDosFileName( const CUString& strPath )
{
	static TCHAR lpszPath[ MAX_PATH + 1 ];
	static CHAR lpszRet[ MAX_PATH + 1 ];
	static CHAR* lpszAscii;

	memset( lpszPath, 0, sizeof( lpszPath ) );

    CUStringConvert strCnv;
	// Get short directory name for output path
	GetShortPathName(	strCnv.ToT( strPath ),
						lpszPath,
						MAX_PATH ); 

	lpszAscii = strCnv.ToACP( CUString( lpszPath, CP_ACP ) );
	strncpy( lpszRet, lpszAscii, MAX_PATH );

	return lpszRet;
}

BOOL AsciiOnly( const CUString& strToTest )
{
#if _UNICODE
  for (int i=0; i<strToTest.GetLength();i++)
    {
		if ( strToTest[ i ] >= 0x80 )
        return false;
    }
#endif
  return true;
}

void SetLastCDexErrorString( CUString& strValue ) 
{ 
	g_lastErrorString = strValue; 
}

CUString GetLastCDexErrorString() { 
	return g_lastErrorString; 
}

__int64 PerformanceCounterGetTime()
{
    __int64 returnValue = 0;
    QueryPerformanceCounter( (LARGE_INTEGER*)&returnValue );
    return returnValue;
}

double PerformanceCounterGetDelta( __int64 startTime, __int64 endTime )
{
    double returnValue = 0;
    static __int64 frequencyDivider = 0;

	if ( frequencyDivider == 0 )
	{
		QueryPerformanceFrequency( (LARGE_INTEGER*)&frequencyDivider );
	}

    if ( frequencyDivider > 0 )
    {
        __int64 deltaTime = endTime - startTime;
        returnValue = (double)deltaTime / (double)(frequencyDivider);
    }

    return returnValue;
}


// Reflection is a requirement for the official CRC-32 standard.
// You can create CRCs without it, but they won't conform to the standard.
static ULONG CRC32Reflect(ULONG ref, char ch)
{
	ULONG value(0);
	// Swap bit 0 for bit 7
	// bit 1 for bit 6, etc.
	for(int i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}


static ULONG crc32Table[ 256 ];

static void CRC32InitTable()
{// Called by OnInitDialog()

	// This is the official polynomial used by CRC-32 
	// in PKZip, WinZip and Ethernet. 
	ULONG ulPolynomial = 0x04c11db7;

	// 256 values representing ASCII character codes.
	for(int i = 0; i <= 0xFF; i++)
	{
		crc32Table[ i ] = CRC32Reflect(i, 8) << 24;
		for (int j = 0; j < 8; j++)
        {
			crc32Table[i] = (crc32Table[i] << 1) ^ (crc32Table[i] & (1 << 31) ? ulPolynomial : 0);
        }
		crc32Table[i] = CRC32Reflect( crc32Table[i], 32 );
	}
}

ULONG Calculate_CRC_Start( )
{
	// Be sure to use unsigned variables,
	// because negative values introduce high bits
	// where zero bits are required.
    return 0xffffffff;
}


// This function uses the crc32_table lookup table
// to generate a CRC for csData
ULONG Calculate_CRC_Block( ULONG startCrc, PBYTE dataPtr, DWORD dataSize, BOOL ignoreZeroSamples )
{
	ULONG crc = startCrc;

    static bool crc32TableInit = false;

    if ( !crc32TableInit )
    {
        CRC32InitTable();
        crc32TableInit = true;
    }


    if ( TRUE == ignoreZeroSamples )
    {
        SHORT* pShort = (SHORT*)dataPtr;

        // Perform the algorithm on each character
	    // in the string, using the lookup table values.
	    for ( DWORD index = 0; index < dataSize / sizeof( SHORT ); index++ )
        {
            if ( pShort[ index ] != 0 )
            {
		        crc = (crc >> 8) ^ crc32Table[ (crc & 0xFF) ^ dataPtr[ index * 2 + 0 ] ];
		        crc = (crc >> 8) ^ crc32Table[ (crc & 0xFF) ^ dataPtr[ index * 2 + 1 ] ];
            }
        }
    }
    else
    {
        // Perform the algorithm on each character
	    // in the string, using the lookup table values.
	    for ( DWORD index = 0; index < dataSize; index++ )
        {
            crc = (crc >> 8) ^ crc32Table[ (crc & 0xFF) ^ dataPtr[ index ] ];
        }
    }

    return crc;
}

ULONG Calculate_CRC_End( ULONG crc )
{
	// Exclusive OR the result with the beginning value.
	return crc^0xffffffff;
}

int CDexMessageBox( const CUString& strMsg, UINT nType )
{
    CUStringConvert strCnv;
    return AfxMessageBox( strCnv.ToT( strMsg ), nType );
}

FILE* CDexOpenFile( const CUString& strFileName, const CUString& strFlags )
{
    CUStringConvert strCnv;
    CUStringConvert strCnv1;
    return _tfopen( strCnv.ToT( strFileName ), strCnv1.ToT( strFlags ) );
}

BOOL CDexDeleteFile( const CUString& strFileName )
{
    BOOL bResult = FALSE;
    CUStringConvert strCnv;
    bResult = ::DeleteFile( strCnv.ToT( strFileName ) );
    CUString strErr = GetLastErrorString();
    return bResult;
}

BOOL CDexCopyFile( const CUString& strSrc, const CUString& strDst , BOOL bFailIfExist )
{
        CUStringConvert strCnv;
        CUStringConvert strCnv1;

        return ::CopyFile( strCnv.ToT( strSrc ), strCnv1.ToT( strDst ), bFailIfExist );
}

BOOL CDexMoveFile( const CUString& strSrc, const CUString& strDst )
{
        CUStringConvert strCnv;
        CUStringConvert strCnv1;

        return ::MoveFile( strCnv.ToT( strSrc ), strCnv1.ToT( strDst ) );
}


HMODULE CDexLoadLibrary( const CUString& strLibraryFileName )
{
    CUStringConvert strCnv;    
    return LoadLibrary( strCnv.ToT( strLibraryFileName ) );
}
