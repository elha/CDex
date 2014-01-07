/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006, 2007, 2008  Georgy Berdyshev
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


#include "stdafx.h"
#include "cdex.h"
#include "EncoderWinCodecMP3Dlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEncoderWinCodecMP3Dlg dialog

CEncoderWinCodecMP3Dlg::CEncoderWinCodecMP3Dlg() 
	: CEncoderDlg(CEncoderWinCodecMP3Dlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderWinCodecMP3Dlg)
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
	m_bHighQuality = FALSE;
	m_bInit = FALSE;
	m_pEncoder = NULL;
	m_nCodecID = WAVE_FORMAT_MPEGLAYER3;
}


void CEncoderWinCodecMP3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderWinCodecMP3Dlg)
	DDX_Control(pDX, IDC_WINCODECFORMAT, m_Formats);
	DDX_Control(pDX, IDC_WINCODEC, m_Codecs);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Check(pDX, IDC_HIQUALITY, m_bHighQuality);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEncoderWinCodecMP3Dlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderWinCodecMP3Dlg)
	ON_BN_CLICKED(IDC_CODECOPTIONS, OnCodecoptions)
	ON_CBN_SELCHANGE(IDC_WINCODEC, OnSelchangeWincodec)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderWinCodecMP3Dlg message handlers

BOOL CEncoderWinCodecMP3Dlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	m_bInit=TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//	CUString strTmp;

//	m_Codecs.GetLBText(m_Codecs.GetCurSel(),strTmp);
//	g_config.SetWinCodec(strTmp);

//	m_Formats.GetLBText(m_Formats.GetCurSel(),strTmp);
//	g_config.SetWinCodecFmt(strTmp);

//	UpdateData(TRUE);



void CEncoderWinCodecMP3Dlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData( TRUE );

	// Set items
	pEncoder->SetOnTheFlyEncoding( m_bEncDuringRead );

	CString strTmp;

	if ( m_Codecs.GetCurSel() >= 0 )
	{
		m_Codecs.GetLBText( m_Codecs.GetCurSel(), strTmp );
		pEncoder->SetUser1( CUString( strTmp ) );


		m_Formats.GetLBText( m_Formats.GetCurSel(), strTmp );
		pEncoder->SetUser2( CUString( strTmp ) );
	}

	pEncoder->SetUserN1( m_bHighQuality );
}

void CEncoderWinCodecMP3Dlg::SetControls(CEncoder* pEncoder)
{
	m_pEncoder=pEncoder;

	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();

	// Set high quality boolean
	m_bHighQuality=pEncoder->GetUserN1();

	CUString strEncoder=pEncoder->GetUser1();
	CUString strFmt=pEncoder->GetUser2();

	if (m_bInit)
	{
		int nCodecIndex=0;
		int	iCodec=0;

		m_Codecs.SetCurSel( 0 );

		for (int i=0;i<g_AcmCodec.GetNumCodecs();i++)
		{

			CUString strName(g_AcmCodec.GetACMCodecName( i, m_nCodecID ) );

			if ( !strName.IsEmpty() )
			{
                CUStringConvert strCnv;
				m_Codecs.AddString( strCnv.ToT( strName ));
				if (strName.Find(strEncoder)>=0)
				{
					nCodecIndex=i;
					m_Codecs.SetCurSel(iCodec);
				}
				iCodec++;
			}
		}

		m_bInit=FALSE;
	}



	m_Formats.ResetContent();

	// Get Codec Text
	CString strSelected;
	int nCodecIndex = m_Codecs.GetCurSel();

	if ( nCodecIndex >= 0 )
    {
		m_Codecs.GetLBText( m_Codecs.GetCurSel() ,strSelected );
    }

	int iCodec=0;

	nCodecIndex =0;

	for (int i=0;i<g_AcmCodec.GetNumCodecs();i++)
	{

		CUString strName(g_AcmCodec.GetACMCodecName(i,m_nCodecID));

		if (!strName.IsEmpty())
		{

			if (strName.Find( CUString( strSelected ) ) >=0 )
			{
				nCodecIndex=i;
			}
			iCodec++;
		}
	}
	
	iCodec=0;

	BOOL bFound=FALSE;

	for (int i=0;i<g_AcmCodec.GetNumDetails(nCodecIndex);i++)
	{
		CUString strFormat=g_AcmCodec.GetFormatString(nCodecIndex,i,m_nCodecID);
//		CUString strFormat=g_AcmCodec.GetFormatString(nCodecIndex,i );
		if (!strFormat.IsEmpty())
		{
            CUStringConvert strCnv;
			m_Formats.AddString( strCnv.ToT( strFormat ));
			if (strFormat.Find(strFmt)>=0)
			{
				bFound=TRUE;
				m_Formats.SetCurSel(iCodec);
			}
			iCodec++;
		}
	}

	if (!bFound)
		m_Formats.SetCurSel(0);


//	m_bPrivate=pEncoder->GetPrivate();
//	m_bCRC=pEncoder->GetCRC();
//	m_bOriginal=pEncoder->GetOriginal();
//	m_bCopyRight=pEncoder->GetCopyRight();
//	m_nMode=pEncoder->GetMode();

	// Set data to controls
	UpdateData(FALSE);
}

void CEncoderWinCodecMP3Dlg::OnCodecoptions() 
{
}

void CEncoderWinCodecMP3Dlg::OnSelchangeWincodec() 
{
	SetControls(m_pEncoder);
}

UINT CEncoderWinCodecMP3Dlg::GetIDD()
{
	return IDD;
}
