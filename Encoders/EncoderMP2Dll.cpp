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
#include "EncoderMP2Dll.h"
#include "EncoderMP2DllDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "EncoderMP2Dll" ) );

CEncoderMP2Dll::CEncoderMP2Dll():CDLLEncoder()
{
	m_strEncoderPath = _T( "MP2Enc.dll" );
	m_strExtension = _T( "mp2" );
	m_strEncoderID = _T( "Internal MP2 Encoder ");
	m_nEncoderID = ENCODER_MP2;

	LoadSettings();
	// Get encoder ID to get proper version info
	GetDLLVersionInfo();

	m_bCanWriteTagV1 = TRUE;
	m_bCanWriteTagV2 = TRUE;
}





CEncoderDlg* CEncoderMP2Dll::GetSettingsDlg()
{
	if ( !m_pSettingsDlg )
	{
		m_pSettingsDlg = new CEncoderMP2DllDlg;
	}
	return m_pSettingsDlg;
}
