/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2006 Albert L. Faber
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
#include <tchar.h>

#include "RipInfoDB.h"
#include "Config.h"

void CRipInfoDB::Init()
{
	m_pFile = NULL;
	m_nTrack = -1;
	m_strFileName= _T( "" );
}


CRipInfoDB::CRipInfoDB()
{
	Init();
}

CRipInfoDB::~CRipInfoDB()
{
	if ( m_pFile )
	{
		fclose( m_pFile );
		m_pFile = NULL;
	}
}

void CRipInfoDB::SetFileName( CUString strFileName )
{
	CUString strOutPath = g_config.GetCDDBPath() + _W( "Status\\" );

	// And test it, no dialog please
	DoesDirExist( strOutPath, FALSE );

	m_strFileName = strOutPath + strFileName + _W( ".txt" );
}

void CRipInfoDB::DeleteTrack( int nTrack )
{
	// Open file for append
	m_pFile = CDexOpenFile( m_strFileName, _W( "r" ) );
    CUStringConvert strCnv;

	if ( NULL != m_pFile )
	{
        fclose( m_pFile );

        CDexCopyFile( m_strFileName, m_strFileName + _W( ".bak") );

        CString test( GetLastErrorString() );

        m_pFile = CDexOpenFile( m_strFileName + _W( ".bak"), _W( "r" ) );
		FILE* pFileOut = CDexOpenFile( m_strFileName, _W( "w+" ) );

		while( ( !feof( m_pFile ) )  && ( pFileOut != NULL ) )
		{
			TCHAR	lpszLine[ 1024 ] = {'\0',};
			int		nCurrentTrack;

			// Clear string
			memset( lpszLine, 0x00, sizeof( lpszLine ) );

			// Read the next line from the file
			_fgetts( lpszLine, _countof( lpszLine ), m_pFile );

			nCurrentTrack= _ttoi( &lpszLine[ 1 ] );

			if ( nCurrentTrack != nTrack )
			{
				_fputts( lpszLine, pFileOut );
			}
		}

		fclose( m_pFile );
		m_pFile = NULL;
		fclose( pFileOut );

		m_pFile = NULL;


        //::DeleteFile( strCnv.ToT( m_strFileName ) );
        //::CopyFile( strCnv.ToT( CUString( m_strFileName + _T( ".bak") ) ) , strCnv.ToT( m_strFileName ), FALSE );
        CDexDeleteFile( m_strFileName + _W( ".bak") );
	}

    if ( m_pFile )
    {
        fclose( m_pFile );
		m_pFile = NULL;
    }

}

void CRipInfoDB::SetCurrentTrack( int nTrack )
{
	m_nTrack = nTrack;
}

BOOL CRipInfoDB::GetCRC(int nTrack, ULONG& crc)
{
    BOOL hasCRC = FALSE;
    
    crc = 0;

	// Open file for append
	m_pFile = CDexOpenFile( m_strFileName, _W( "r" ) );

	if ( NULL != m_pFile )
	{
		// Loop through all entries
		while( !feof( m_pFile ) )
		{
			TCHAR	lpszLine[ 1024 ] = {'\0',};
			int		nCurrentTrack;

			// Clear string
			memset(lpszLine,0x00,sizeof(lpszLine));

			// Read the next line from the file
			_fgetts(	lpszLine,
						_countof( lpszLine ),
						m_pFile );

			// Get track number
			nCurrentTrack = _ttoi( &lpszLine[ 1 ]  );

			if ( nCurrentTrack == nTrack ) 
			{
				switch ( lpszLine[ 6 ] )
				{
					case 'C':	
                        {
                            TCHAR* strCRC = _tcsstr( lpszLine, _T( "CRC=" ) );
                            if ( strCRC )
                            {
                                hasCRC = TRUE;
                                _stscanf( &strCRC[ 4 ], _T( "%08X\n" ), &crc );
                            }
                        }
                        break;
                    default:
                        break;
				}
			}
		}

		fclose( m_pFile );
		m_pFile = NULL;
	}

	return hasCRC;
}

CDEX_ERR CRipInfoDB::GetStatusInfo( int nTrack, DWORD& dwJitterErrors )
{
	BOOL	bFinishedProperly = FALSE;
	BOOL	bPresentInDB = FALSE;
	BOOL	bAborted = FALSE;
	CDEX_ERR result = CDEX_OK;
	
	dwJitterErrors = 0;

	// Open file for append
    m_pFile = CDexOpenFile( m_strFileName, _W( "r" ) );

	if ( NULL != m_pFile )
	{
		// Loop through all entries
		while( !feof( m_pFile ) )
		{
			TCHAR	lpszLine[ 1024 ] = {'\0',};
			int		nCurrentTrack;

			// Clear string
			memset(lpszLine,0x00,sizeof(lpszLine));

			// Read the next line from the file
			_fgetts(	lpszLine,
						_countof( lpszLine ),
						m_pFile );

			// Get track number
			nCurrentTrack = _ttoi( &lpszLine[ 1 ]  );

			if ( nCurrentTrack == nTrack ) 
			{
				bPresentInDB=TRUE;

				switch ( lpszLine[ 6 ] )
				{
					case 'J':	dwJitterErrors++ ; break;
					case 'A':	bAborted = TRUE; break;
					case 'C':	bFinishedProperly = TRUE; break;
				}
			}
		}

		fclose( m_pFile );

		m_pFile = NULL;
	}

	if ( dwJitterErrors > 0 )
	{
		result = CDEX_JITTER_ERROR;
	}
	else if ( bAborted )
	{
		result = CDEX_ERROR;
	}
	else if ( bPresentInDB && bFinishedProperly )
	{
		result = CDEX_RIPPING_DONE;
	}

	return result;
}

void CRipInfoDB::SetRipInfo(CUString strRipInfo)
{
	// Open file for append
    CUStringConvert strCnv;

    m_pFile = CDexOpenFile( m_strFileName, _W( "a+" ) );

	if ( NULL != m_pFile )
	{
        CUString strOut;

        strOut.Format( _W( "[%03d][I] : %s\n" ), m_nTrack, (LPCWSTR)strRipInfo  );

		_fputts( strCnv.ToT( strOut ), m_pFile );

		fclose( m_pFile );

		m_pFile = NULL;
	}
}



void CRipInfoDB::SetRipInfoFinshedOK(CUString strRipInfo, ULONG crc )
{
    CUStringConvert strCnv;

	// Open file for append
    m_pFile = CDexOpenFile( m_strFileName, _W( "a+" ) );

	if ( m_pFile )
	{
        CUString strOut;

        strOut.Format( _W( "[%03d][C] : %s CRC=%08X\n" ), m_nTrack, (LPCWSTR) strRipInfo, crc );

		_fputts( strCnv.ToT( strOut ), m_pFile );

		fclose( m_pFile );

		m_pFile = NULL;
	}
}

void CRipInfoDB::SetJitterError( int nSectorStart, int nSectorEnd, int nTrackSectorBegin )
{
	// Open file for append
	m_pFile = CDexOpenFile( m_strFileName, _W( "a+" ) );

	if ( NULL != m_pFile )
	{
		char lpszOut[1024] = {'\0',};

		sprintf(	lpszOut,
					"[%03d][J] : Jitter error detected (not corrected!) near %d:%02d [m:s] (range %d - %d [ms], absolute %d - %d [sectors])\n",
					m_nTrack,
					( ( nSectorStart - nTrackSectorBegin ) / 75 ) / 60,
					( ( nSectorStart - nTrackSectorBegin ) / 75 ) % 60,
					( nSectorStart - nTrackSectorBegin ) * 1000 / 75,
					( nSectorEnd - nTrackSectorBegin ) * 1000 / 75,
					nSectorStart,
					nSectorEnd );

		fputs( lpszOut, m_pFile );
		fclose( m_pFile );
		m_pFile = NULL;
	}
}


void CRipInfoDB::SetAbortError(CUString strAbortInfo)
{
	// Open file for append
	m_pFile = CDexOpenFile( m_strFileName, _W( "a+" ) );

	if ( NULL != m_pFile )
	{
		char lpszOut[ 1024 ] = { '\0',};

		sprintf(	lpszOut,
					"[%03d][A] : %s\n",
					m_nTrack,
					strAbortInfo );

		fputs( lpszOut, m_pFile );
		fclose( m_pFile );
		m_pFile = NULL;
	}
}

CUString CRipInfoDB::GetRipInfo( int nTrack )
{
	CUString strRet( _W( "" ) );

	// Open file for read
	m_pFile = CDexOpenFile( m_strFileName, _W( "r" ) );

	if ( NULL != m_pFile )
	{
		// Loop through all entries
		while( !feof( m_pFile ) )
		{
			TCHAR	lpszLine[1024];
			int		nCurrentTrack;

			// Clear string
			memset(lpszLine,0x00,sizeof(lpszLine));

			// Get the current line
			_fgetts(lpszLine, _countof(lpszLine), m_pFile);

			// Get track number
			nCurrentTrack = _ttoi(&lpszLine[1]);

			if ( nCurrentTrack == nTrack ) 
			{
				CUString strTmp( lpszLine );
				strTmp = strTmp.Mid( 11 );
				strRet += strTmp + _W( "\r\n" ) ;
			}
		}
		fclose( m_pFile );
		m_pFile = NULL;
	}

	if (!strRet.IsEmpty())
	{
		strRet += _W( "\r\n" );
	}
	return strRet;
}

void CRipInfoDB::RemoveAll()
{
	// Open file for append
    CUStringConvert strCnv;
    ::DeleteFile( strCnv.ToT( m_strFileName ) );
}