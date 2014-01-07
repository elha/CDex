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
#include "EncoderDosVqf.h"
#include "EncoderDosVqfDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderDosVqf" ) );

CEncoderDosVqf::CEncoderDosVqf()
	: CEncoderDos()
{
	ENTRY_TRACE( _T( "CEncoderDosVqf::CEncoderDosVqf()" ) );

	m_strEncoderPath = _T( "" );
	m_strEncoderID = _T( "Yamaha VQF encoder" );
	m_nEncoderID = ENCODER_VQF;
	m_strExtension = _T( "vqf" );
	m_strUser2 = _T( "44KHz-48Kbps" );
	LoadSettings();
	m_bRiffWavSupport = FALSE;

	EXIT_TRACE( _T( "CEncoderDosVqf::CEncoderDosVqf()" ) );
}

CEncoderDosVqf::~CEncoderDosVqf()
{
}


CEncoderDlg* CEncoderDosVqf::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg=new CEncoderDosVqfDlg;
	}
	return m_pSettingsDlg;
}


CDEX_ERR CEncoderDosVqf::CreateVQFScript(CUString strWavFile,CUString strVqfFile)
{
	CDEX_ERR bReturn = CDEX_OK;

	CUString	strLang;
	FILE*	fOut = NULL;

	fOut = CDexOpenFile( _W( "cdex.scr" ), _W( "w" ) );

    CUStringConvert strCnv;
	if ( NULL != fOut )
	{
		fprintf( fOut, "[begin]\n" );
		fprintf( fOut, "input = %s\n", strWavFile );
		fprintf( fOut, "output = %s\n", strVqfFile );
		fprintf( fOut, "title = \"-\"\n" );
		fprintf( fOut, "author = \"-\"\n" );
		fprintf( fOut, "copyright = \"-\"\n" );
		fprintf( fOut, "comment = \"-\"\n" );
		fprintf( fOut, "mode = %s\n" , strCnv.ToUTF8( GetUser2() ) );
		fprintf( fOut, "channel = 2\n" );
		fprintf( fOut, "quality = High\n" );
//		fprintf( fOut, "folder = same\n" );
		fprintf( fOut, "[end]\n" );

		fclose(fOut);
	}
	else
	{
		strLang = g_language.GetString( IDS_ENCODER_ERROR_CANT_CREATEVQFSCRIPT );
		CDexMessageBox( strLang );
		bReturn = CDEX_ERROR;
	}
	return bReturn;
}



CDEX_ERR CEncoderDosVqf::DosEncode(	const CUString& strInFileName,
									const CUString& strInFileExt, 
									const CUString& strOutFileName,
									const CUString& strOutDir,
									INT&  nPercent,
									BOOL& bAbort)
{
	CDEX_ERR bReturn = CDEX_OK;

	ENTRY_TRACE( _T( "CEncoderDosVqf::DosEncode()" ) );

	// Create script file

	CreateVQFScript( strInFileName + CUString( _W( ".wav" ) ),
						strOutDir + CUString( _W( "temp.vqf" ) ) );

	m_strEncoderPath.Format( _W( "%s cdex.scr -delete" ), (LPCWSTR)GetUser1() );

	// Call the encoder
	bReturn = CEncoderDos::DosEncode(strInFileName,strInFileExt, strOutFileName, strOutDir,nPercent,bAbort);

	EXIT_TRACE( _T( "CEncoderDosVqf::DosEncode(), return value %d" ), bReturn );

	return bReturn;
}

