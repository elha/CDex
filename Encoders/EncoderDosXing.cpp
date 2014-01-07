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
#include "EncoderDosXing.h"
#include "EncoderDosXingDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _W( "EncoderDosXing" ) );


CEncoderDosXing::CEncoderDosXing():CEncoderDos()
{
	m_bRiffWavSupport = TRUE;
	m_strEncoderPath = _W( "" );
	m_strExtension = _W( "mp3" );
	m_strEncoderID = _W( "External Xing Encoder" );
	m_nEncoderID = ENCODER_EXTXING;
	LoadSettings();

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = TRUE;

}

CEncoderDosXing::~CEncoderDosXing()
{
}

CEncoderDlg* CEncoderDosXing::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg=new CEncoderDosXingDlg;
	}
	return m_pSettingsDlg;
}


CDEX_ERR CEncoderDosXing::DosEncode(const CUString& strInFileName,
									const CUString& strInFileExt, 
									const CUString& strOutFileName,
									const CUString& strOutDir,
									INT&  nPercent,
									BOOL& bAbort)
{
	// Prepare user string based on the Xing encoder settings
	m_strUser2 = _W( "%1 %2" );

	if (GetOriginal()==0)		// Turn off Original flag
	{
		m_strUser2 += _W( " -o" );
	}

	if (GetCopyRight()==0)	// Turn off copyright flag
	{
		m_strUser2 += _W( " -c" );
	}

	if ( (GetUserN1()>>0)&0x01 )	 //Downmix To Mono
	{
		m_strUser2 += _W( " -d" );
	}

	if ( (GetUserN1()>>1)&0x01 )	// FilterHighFreq
	{
		m_strUser2 += _W( " -l" );
	}

	if ( (GetUserN1()>>2)&0x01 )	// SimpleStereo
	{
		m_strUser2 += _W( " -s" );
	}

	if ( (GetUserN1()>>3)&0x01 )	// Let Encoder choose sample rate
	{
		m_strUser2 += _W( " -r" );
	}

	if ( GetMode() == 0 )		// is CBR or VBR
	{
		// CBR
		m_strUser2 += _W( " -b %3" );
		
	}
	else
	{
		// VBR
		CUString strTmp;
		strTmp.Format( _W( " -v %d" ), GetUserN2() );
		m_strUser2+=strTmp;
	}

	// Call parent method
	return CEncoderDos::DosEncode(strInFileName, strInFileExt, strOutFileName, strOutDir, nPercent, bAbort );
}

