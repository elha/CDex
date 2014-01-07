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
#include "EncoderWinCodecMP3.h"
#include "EncoderWinCodecMP3Dlg.h"
#include "AudioFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderWinCodecMP3" ) );

CEncoderWinCodecMP3::CEncoderWinCodecMP3() : CEncoderWinCodec()
{
	m_strEncoderPath = _T( "WindowsCodec" );
	m_strEncoderID = _T( "Windows MP3 Codec" );
	m_nEncoderID = ENCODER_WIN;

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = TRUE;

	// Get settings from registry
	LoadSettings();

	// Set bitrate based on Format setting
	if ( m_strUser2.GetLength() > 1 )
	{
		m_nBitrate = _wtoi( m_strUser2 );
		int nPos = m_strUser2.Find( _W( "Mono" ) );

		if ( nPos >= 0 )
		{
			m_nMode = BE_MP3_MODE_MONO;
		}
		else
		{
			m_nMode = BE_MP3_MODE_STEREO;
		}
	}
}


CEncoderDlg* CEncoderWinCodecMP3::GetSettingsDlg()
{
	if (!m_pSettingsDlg)
	{
		m_pSettingsDlg = new CEncoderWinCodecMP3Dlg;
	}
	return m_pSettingsDlg;
}

CWinULawEncoder::CWinULawEncoder()
	: CEncoderWinCodec()
{
	m_strEncoderPath = _T( "WindowsCodec" );
	m_strEncoderID = _T( "Windows ADPCM Codec" );
//	m_nEncoderID=ENCODER_WINULAW;
	m_strExtension= _T( "wav" );
	m_strUser1= _T( "Microsoft ADPCM" );
	m_strUser2= _T( "44" );
	m_nCodecID = 0x02;
	LoadSettings();
}

#ifdef _DEBUG
CEncoderDlg* CWinSCXEncoder::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg=new CWinEncOptsULaw;
	}
	return m_pSettingsDlg;
}
#endif

CEncoderDlg* CWinULawEncoder::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg=new CWinEncOptsULaw;
	}
	return m_pSettingsDlg;
}

CWinEncOptsULaw::CWinEncOptsULaw() 
	: CEncoderWinCodecMP3Dlg()
{
	m_nCodecID = 0x02;
}


#ifdef ENCODER_TYPES_EXPERIMENTAL
CWinSCXEncoder::CWinSCXEncoder()
	: CEncoderWinCodec()
{
	m_strEncoderPath = _T( "WindowsCodec" );
	m_strEncoderID = _T( "Windows Atrac Codec" );
	m_strExtension = _T( "scx" );
	m_strUser1= _T( "Atrac" );
	m_strUser2= _T( "44" );

	m_nCodecID = WAVE_FORMAT_SONY_SCX;
	m_nEncoderID = ENCODER_SCX;

	m_bCanWriteTagV1 = FALSE;
	m_bCanWriteTagV2 = FALSE;

	LoadSettings();
}

CWinEncOptsScx::CWinEncOptsScx()
	: CEncoderWinCodecMP3Dlg()
{
	m_nCodecID = WAVE_FORMAT_SONY_SCX;
}

#endif


