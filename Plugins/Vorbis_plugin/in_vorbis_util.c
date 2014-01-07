/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
** Copyright (C) Jack Moffitt (jack@icecast.org)
** Copyright (C) Michael Smith (msmith@labyrinth.net.au)
** Copyright (C) Aaron Porter (aaron@javasource.org)
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


#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <wchar.h>
#include <commctrl.h>
#include <stdio.h>
#include <assert.h>

#include "in_vorbis_util.h"

char info_fn[ 255 ];

VPI_INFO vpi_info = { 0,};
VPI_CONFIG vpi_config = { 0,};

static void GetConfigFileName( PCHAR pszFileName );


static void GetConfigFileName( PCHAR pszFileName )
{
	PCHAR	pLastSlash = NULL;

	// Get the full module (DLL) file name
	GetModuleFileName(	NULL, 
						pszFileName,
						MAX_PATH );

	pLastSlash = strrchr( pszFileName, '\\' );

	if (  pLastSlash )
	{
		pszFileName[ pLastSlash - pszFileName + 1 ] = '\0';
		
		pLastSlash = strstr( pszFileName, "\\Debug" );

		if (  !pLastSlash )
		{
			pLastSlash = strstr( pszFileName, "\\Release" );
		}
		if (  pLastSlash )
		{
			pszFileName[ pLastSlash - pszFileName + 1 ] = '\0';
		}

		strcat( pszFileName, "Plugins\\in_vorbis.ini" );
	}
}

void get_vpi_config( )
{
	int		nIndex = 0;
	char	szFileName[ MAX_PATH + 1 ];
	
	memset( szFileName, 0, sizeof( szFileName ) );

	vpi_config.bUseReplayGain = TRUE;
	vpi_config.bUseReplayGainBooster = FALSE;
	vpi_config.nReplayGainMode = REPLAYGAIN_MODE_AUDIOPHILE;
	
	GetConfigFileName( szFileName );

	vpi_config.bUseReplayGain = GetPrivateProfileInt("ReplayGain","bUseReplayGain", vpi_config.bUseReplayGain, szFileName );
	vpi_config.bUseReplayGainBooster = GetPrivateProfileInt("ReplayGain","bUseReplayGainBooster", vpi_config.bUseReplayGainBooster, szFileName );
	vpi_config.nReplayGainMode = GetPrivateProfileInt("ReplayGain","nReplayGainMode", vpi_config.nReplayGainMode, szFileName );

	store_vpi_config( );
}

void store_vpi_config( )
{
	char	szFileName[ MAX_PATH + 1 ];
	char	szTemp[ MAX_PATH + 1 ];
	
	memset( szFileName, 0, sizeof( szFileName ) );
	memset( szTemp, 0, sizeof( szTemp ) );

	GetConfigFileName( szFileName );

	sprintf( szTemp, "%d", vpi_config.bUseReplayGain );
	WritePrivateProfileString("ReplayGain","bUseReplayGain", szTemp, szFileName );

	sprintf( szTemp, "%d", vpi_config.bUseReplayGainBooster );
	WritePrivateProfileString("ReplayGain","bUseReplayGainBooster", szTemp, szFileName );

	sprintf( szTemp, "%d", vpi_config.nReplayGainMode );
	WritePrivateProfileString("ReplayGain","nReplayGainMode", szTemp, szFileName );
}

void clean_vpi_info()
{
	int	nComment = 0;

	if ( vpi_info.pszTitle )
	{
		free( vpi_info.pszTitle );
		vpi_info.pszTitle = NULL;
	}

	if ( vpi_info.pszVendor )
	{
		free( vpi_info.pszVendor );
		vpi_info.pszVendor = NULL;
	}



	for ( nComment = 0; nComment< IN_VORBIS_MAX_TAGS; nComment++ )
	{
		if ( vpi_info.pszTags[ nComment ] )
		{
			free( vpi_info.pszTags[ nComment ] );
			vpi_info.pszTags[ nComment ]= NULL;
		}

	}

	memset( &vpi_info, 0x00, sizeof( vpi_info ) );
}

PCHAR get_tag_value_by_name( const CHAR* pszTag )
{
	int nComment = 0;

	while ( nComment < IN_VORBIS_MAX_TAGS && 
			( NULL != vpi_info.pszTags[ nComment ] ) )
	{
		if ( 0 == _strnicmp( vpi_info.pszTags[ nComment ], pszTag, strlen( pszTag ) ) )
		{
			if ( vpi_info.pszTags[ nComment ][ strlen( pszTag ) ] == '='  )
			{
				return &vpi_info.pszTags[ nComment ][ strlen( pszTag ) + 1 ];
			}
		}

		nComment++;
	}
	return NULL;
}

PCHAR get_tag_value_by_number( int nComment )
{
	PCHAR pReturn = NULL;

	assert( nComment < IN_VORBIS_MAX_TAGS );

	pReturn = strchr( vpi_info.pszTags[ nComment ], '=' );

	if ( NULL != pReturn )
	{
		pReturn++;
	}

	return pReturn;
}

DWORD get_number_of_tags( )
{
	DWORD  dwNumberOfTags = 0;;

	while ( ( dwNumberOfTags < IN_VORBIS_MAX_TAGS ) &&
			( NULL != vpi_info.pszTags[ dwNumberOfTags ] ) )
	{
		dwNumberOfTags++;
	}

	return dwNumberOfTags;
}


/* Converts a UTF-8 character sequence to a UCS-4 character */
int _utf8_to_ucs4(unsigned int *target, const char *utf8, int n)
{
	unsigned int result = 0;
	int count;
	int i;

	/* Determine the number of characters in sequence */
	if ((*utf8 & 0x80) == 0)
		count = 1;
	else if ((*utf8 & 0xE0) == 0xC0)
		count = 2;
	else if ((*utf8 & 0xF0) == 0xE0)
		count = 3;
	else if ((*utf8 & 0xF8) == 0xF0)
		count = 4;
	else if ((*utf8 & 0xFC) == 0xF8)
		count = 5;
	else if ((*utf8 & 0xFE) == 0xFC)
		count = 6;
	else
		return UTF8_ILSEQ; /* Invalid start byte */

	if (n < count)
		return UTF8_ILSEQ; /* Not enough characters */

	if (count == 2 && (*utf8 & 0x1E) == 0)
		return UTF8_ILSEQ; /* Overlong sequence */

	/* Convert the first character */
	if (count == 1)
		result = *utf8;
	else
		result = (0xFF >> (count +1)) & *utf8;

	/* Convert the continuation bytes */
	for (i = 1; i < count; i++)
	{
		if ((utf8[i] & 0xC0) != 0x80)
			return UTF8_ILSEQ; /* Not a continuation byte */
		if (result == 0 &&
			i == 2 &&
			((utf8[i] & 0x7F) >> (7 - count)) == 0)
			return UTF8_ILSEQ; /* Overlong sequence */
		result = (result << 6) | (utf8[i] & 0x3F);
	}

	if (target != 0)
		*target = result;

	return count;
}

/* Converts a UTF-8 string to a WCHAR string */
int UTF8ToWideChar(LPWSTR target, LPCSTR utf8, WCHAR unknown)
{
	int wcount = 0;
	int conv;
	unsigned int ucs4;
	int count = lstrlenA(utf8) +1;

	while (count != 0)
	{
		conv = _utf8_to_ucs4(&ucs4, utf8, count);
		if (conv == UTF8_ILSEQ) return UTF8_ILSEQ;
		if (target != 0)
		{
			if (ucs4 > 0xFFFF)
				*target = unknown; /* Can only handle BMP */
			else
				*target = (WCHAR) ucs4;
			target++;
		}
		wcount++;
		count -= conv;
		utf8 += conv;
	}

	return wcount;
}

PCHAR UTT8_2_ASCII( CHAR* pszUTF, CHAR* pOutput, DWORD dwMaxSize )
{
	int len = UTF8_ILSEQ;

	pOutput[ 0 ] = '\0';

	if ( pszUTF )
	{
		/* Convert the UTF-8 title to the system code page */
		len = UTF8ToWideChar( NULL, pszUTF, '?' );

		if ( ( len == UTF8_ILSEQ ) || ( len >= (int)dwMaxSize ) )
		{
			/* Fallback to ascii */
			strncpy( pOutput, pszUTF, dwMaxSize );

		}
		else
		{
			/* Convert the UTF-8 string */
			PWCHAR titleW = calloc( len, sizeof( WCHAR ) );

			if ( NULL != titleW )
			{
				UTF8ToWideChar( titleW, pszUTF, '?' );

				WideCharToMultiByte(	CP_ACP, 
										0,
										titleW,
										-1,
										pOutput,
										len, 
										"?",
										NULL );
				free( titleW );
			}
		}
	}

	// be sure the string is properly terminated
	pOutput[ dwMaxSize - 1 ] = '\0';

	return pOutput;
}
