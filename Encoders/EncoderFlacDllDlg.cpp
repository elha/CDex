/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
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


#include "stdafx.h"
#include "cdex.h"
#include "EncoderFlacDllDlg.h"
#include "Encode.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int oggBitrates[] = { 0,32,40,48,64,80,96,128,160,196,256,350};



CEncoderFlacDllDlg::CEncoderFlacDllDlg() 
	: CEncoderDlg(CEncoderFlacDllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderFlacDllDlg)
	m_nMode = -1;
	m_nChannels = -1;
	m_strQualitySetting = _T("");
	m_nCompressionLevel = 0;
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
}


void CEncoderFlacDllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderFlacDllDlg)
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Radio(pDX, IDC_CHANNELSELECTION, m_nChannels);
	DDX_CBIndex(pDX, IDC_COMPRESSION_LEVEL, m_nCompressionLevel);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEncoderFlacDllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderFlacDllDlg)
	ON_WM_HSCROLL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderFlacDllDlg message handlers

BOOL CEncoderFlacDllDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;
}


void CEncoderFlacDllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	pEncoder->SetOnTheFlyEncoding( m_bEncDuringRead );

	int nMode = ( m_nChannels  == 1 ) ? BE_MP3_MODE_MONO : BE_MP3_MODE_STEREO;

	pEncoder->SetMode( nMode );

	pEncoder->SetUserN1( m_nCompressionLevel  );



}

void CEncoderFlacDllDlg::SetControls(CEncoder* pEncoder)
{
	INT nMode = pEncoder->GetMode();

	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();

	m_nChannels = ( ( nMode & 0xFFFF ) == BE_MP3_MODE_MONO )? 1 : 0;

	// Fill Min bitrate table

	m_nCompressionLevel = pEncoder->GetUserN1() & 0xFF;
	

	// Set data to controls
	UpdateData(FALSE);
}



UINT CEncoderFlacDllDlg::GetIDD()
{
	return IDD;
}

