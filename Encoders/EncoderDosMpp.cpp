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


#include "StdAfx.h"
#include "TaskInfo.h"
#include "EncoderDosMpp.h"
#include "EncoderDosMppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderDosMpp" ) );


CEncoderDosMpp::CEncoderDosMpp():
	CEncoderDos()
{
	ENTRY_TRACE( _T( "CEncoderDosMpp::CEncoderDosMpp()" ) );

	m_bRiffWavSupport = FALSE;
	m_strEncoderPath = _T( "" );
	m_nEncoderID = ENCODER_MPP;

	// OLD musepack string m_strUser2 = _T( "%1 %2 -v" );
	m_strUser2 = _T( "--verbose %1 %2" );


	LoadSettings();

	m_bSendStdinWavHeader = TRUE;
	m_strExtension = _T( "mpc" );
	m_strEncoderID = _T( "External Musepack Encoder" );

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = FALSE;

	SetChunkSupport ( GetOnTheFlyEncoding () ) ;

	EXIT_TRACE( _T( "CEncoderDosMpp::CEncoderDosMpp()" ) );
}

CEncoderDosMpp::~CEncoderDosMpp()
{
}

CEncoderDlg* CEncoderDosMpp::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg = new CEncoderDosMPPDlg;
	}
	return m_pSettingsDlg;
}


CDEX_ERR CEncoderDosMpp::DosEncode(	const CUString& strInFileName,
									const CUString& strInFileExt, 
									const CUString& strOutFileName,
									const CUString& strOutDir,
									INT&  nPercent,
									BOOL& bAbort)
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoderDosMpp::DosEncode()" ) );

	// Call parent method
	bReturn = CEncoderDos::DosEncode(	strInFileName,
										strInFileExt,
										strOutFileName,
										strOutDir,
										nPercent,
										bAbort );

	EXIT_TRACE( _T( "CEncoderDosMpp::DosEncode(), return value %d" ), bReturn );

	return bReturn;
}
