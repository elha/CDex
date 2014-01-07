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
#include "EncoderWMA8DllDlg.h"
#include "Encode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int wmaBitRates[10] = { 22,32,36,40,48,64,80,96,128,160};


/////////////////////////////////////////////////////////////////////////////
// CEncoderWMA8DllDlg dialog

CEncoderWMA8DllDlg::CEncoderWMA8DllDlg() 
	: CEncoderDlg(CEncoderWMA8DllDlg::IDD)
{
	//{{AFX_DATA_INIT(CEncoderWMA8DllDlg)
	//}}AFX_DATA_INIT
	m_bEncDuringRead = FALSE;
	m_bUseDRM = FALSE;
	m_nMode = 1;
}


void CEncoderWMA8DllDlg::DoDataExchange(CDataExchange* pDX)
{
	CEncoderDlg::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderWMA8DllDlg)
	DDX_Control(pDX, IDC_BITRATE, m_Bitrate);
	DDX_Check(pDX, IDC_RTENCODING, m_bEncDuringRead);
	DDX_Check(pDX, IDC_USEDRM, m_bUseDRM);
	DDX_Radio(pDX, IDC_CHANNELS, m_nMode);
	//}}AFX_DATA_MAP
}




BEGIN_MESSAGE_MAP(CEncoderWMA8DllDlg, CEncoderDlg)
	//{{AFX_MSG_MAP(CEncoderWMA8DllDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEncoderWMA8DllDlg message handlers

BOOL CEncoderWMA8DllDlg::OnInitDialog() 
{
	CEncoderDlg::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CEncoderWMA8DllDlg::GetControls(CEncoder* pEncoder)
{
	// Get data out of Controls
	UpdateData(TRUE);

	// Set items
	pEncoder->SetOnTheFlyEncoding(m_bEncDuringRead);
	pEncoder->SetCopyRight(m_bUseDRM);

	// Get current bitrate selection
	int iSelection=m_Bitrate.GetCurSel();

	// Validate selection value
	ASSERT(iSelection<sizeof(wmaBitRates)/sizeof(wmaBitRates[0]));
	ASSERT(iSelection>=0);

	// Get/Set bitrate value from wmaBitRate Table
	pEncoder->SetBitrate(wmaBitRates[iSelection]);

	pEncoder->SetMode( m_nMode );

}

void CEncoderWMA8DllDlg::SetControls(CEncoder* pEncoder)
{
	// Set control items based in pEncoder information
	m_bEncDuringRead=pEncoder->GetOnTheFlyEncoding();
	m_bUseDRM=pEncoder->GetCopyRight();

	for (int i=0;i<sizeof(wmaBitRates)/sizeof(wmaBitRates[0]);i++)
	{	
		if (pEncoder->GetBitrate()==wmaBitRates[i])
			m_Bitrate.SetCurSel(i);
	}

	m_nMode = pEncoder->GetMode();

	// Set data to controls
	UpdateData(FALSE);
}

UINT CEncoderWMA8DllDlg::GetIDD()
{
	return IDD;
}
