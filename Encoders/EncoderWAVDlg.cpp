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


#include "stdafx.h"
#include "cdex.h"
#include "EncoderWAVDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int wavSampleFreq[3] = { 44100,22050,11025 };


// WAV OPTION DIALOG
CEncoderWAVDlg::CEncoderWAVDlg() 
	: CEncoderDlg(CEncoderWAVDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderWAVDlg)
	m_nChannels = -1;
	m_bEncDuringRead = FALSE;
	//}}AFX_DATA_INIT
}

void CEncoderWAVDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderWAVDlg)
	DDX_Control(pDX, IDC_SAMPLERATE, m_SampleRate);
	DDX_Control(pDX, IDC_COMPRESSION, m_Compression);
	DDX_Radio(pDX, IDC_CHANNELS, m_nChannels );
	DDX_Control(pDX, IDC_COMPRESSION, m_Compression);
	DDX_Control(pDX, IDC_FORMAT, m_Format);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	//}}AFX_DATA_MAP
}

void CEncoderWAVDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	pEncoder->SetUserN1( m_Compression.GetCurSel() );
	pEncoder->SetUserN2( m_Format.GetCurSel() );

	pEncoder->SetMode( m_nChannels );
	pEncoder->SetSampleRate( wavSampleFreq[ m_SampleRate.GetCurSel() ] );
	pEncoder->SetOnTheFlyEncoding( m_bEncDuringRead );

}

void CEncoderWAVDlg::SetCompressionField()
{
	if ( m_Compression.m_hWnd && m_Format.m_hWnd )
	{
		m_Compression.ResetContent();

		m_Compression.AddString( _T( "None (PCM)" ) );
		m_Compression.SetCurSel( 0 );

		switch ( m_Format.GetCurSel() )
		{
			case 0:
				m_Compression.AddString( _T( "u-Law" ) );
				m_Compression.AddString( _T( "A-Law" ) );
				m_Compression.AddString( _T( "IMA ADPCM" ) );
				m_Compression.AddString( _T( "MS ADPCM" ) );
			break;
			case 1:
			case 4:
			case 5:
			case 6:
			case 7:
				m_Compression.SetCurSel( min(m_Compression.GetCurSel(), 0 ) );
			break;
			case 2:
			case 3:
				m_Compression.SetCurSel( min( m_Compression.GetCurSel(), 2 ) );
				m_Compression.AddString( _T( "u-Law" ) );
				m_Compression.AddString( _T( "A-Law" ) );
			break;
			default:
				ASSERT(FALSE);
		
		}
	}
}

void CEncoderWAVDlg::SetControls(CEncoder* pEncoder)
{

	if ( m_Format.m_hWnd )
	{
		m_Format.SetCurSel( pEncoder->GetUserN2() );
	}

	m_nChannels = pEncoder->GetMode();
	m_bEncDuringRead = pEncoder->GetOnTheFlyEncoding();

	SetCompressionField();

	if ( m_Compression.m_hWnd )
	{
		m_Compression.SetCurSel( pEncoder->GetUserN1() );
	}

	m_SampleRate.SetCurSel(0);
	for (int i=0;i<sizeof(wavSampleFreq)/sizeof(wavSampleFreq[0]);i++)
	{
		if (pEncoder->GetSampleRate()==wavSampleFreq[i])
		{
			m_SampleRate.SetCurSel(i);
		}
	}

	// Set data to controls
	UpdateData( FALSE );
}


BEGIN_MESSAGE_MAP(CEncoderWAVDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderWAVDlg)
	ON_CBN_SELCHANGE(IDC_FORMAT, OnSelchangeFormat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderWAVDlg message handlers

BOOL CEncoderWAVDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	CUString strTmp;

	for (int i=0;i<sizeof(wavSampleFreq)/sizeof(wavSampleFreq[0]);i++)
	{
        CUStringConvert strCnv;
		strTmp.Format( _W( "%d Hz" ), wavSampleFreq[ i ] );
		m_SampleRate.AddString( strCnv.ToT( strTmp ) );
	}

	return TRUE;
}

void CEncoderWAVDlg::OnSelchangeFormat() 
{
	UpdateData(TRUE);
	SetCompressionField();
	UpdateData(FALSE);
}

UINT CEncoderWAVDlg::GetIDD()
{
	return IDD;
}
