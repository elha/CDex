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
#include "EncoderPropPage.h"
#include "config.h"
#include "ConfigDlg.h"
#include "CDex.h"
#include "CodecDlg.h"

#include "EncoderDlg.h"
#include "EncoderApeDllDlg.h"
#include "EncoderBladeDllDlg.h"
#include "EncoderDosDlg.h"
#include "EncoderDosMPPDlg.h"
#include "EncoderDosVqfDlg.h"
#include "EncoderDosXingDlg.h"
#include "EncoderFaacDllDlg.h"
#include "EncoderGogoDllDlg.h"
#include "EncoderLameDllDlg.h"
#include "EncoderNttVqfDllDlg.h"
#include "EncoderMP2DllDlg.h"
#include "EncoderVorbisDllDlg.h"
#include "EncoderWAVDlg.h"
#include "EncoderWinCodecMP3Dlg.h"
#include "EncoderWMA8DllDlg.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEncoderPropPage property page

IMPLEMENT_DYNCREATE(CEncoderPropPage, CPropertyPage)


// CONSTRUCTOR of CEncoderPropPage
CEncoderPropPage::CEncoderPropPage() : CPropertyPage(CEncoderPropPage::IDD)
{
    static CUStringConvert strCnv;

    m_strTitle  = g_language.GetString( IDS_ENCODERPROPPAGE );
	m_psp.pszTitle = strCnv.ToT( m_strTitle );
	m_psp.dwFlags |= PSP_USETITLE;

	//{{AFX_DATA_INIT(CEncoderPropPage)
	//}}AFX_DATA_INIT
	m_bRiffWav = g_config.GetRiffWav();
	m_bRetainWAVFile = g_config.GetRetainWavFile();

	for ( int i = 0; i < NUMENCODERS; i++ )
	{
		m_pEncoder[ i ] = NULL;
	}

	m_nEncoders = 0;
	m_nOldSel = -1;
}

// DESTRUCTOR of CEncoderPropPage
CEncoderPropPage::~CEncoderPropPage()
{
	for ( int i = 0; i < NUMENCODERS; i++ )
	{
		delete m_pEncoder[ i ];
		m_pEncoder[ i ] = NULL;
	}
}

void CEncoderPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEncoderPropPage)
	DDX_Control(pDX, IDC_ENCODERSEL, m_EncoderSel);
	DDX_Control(pDX, IDC_THREADPRIORITY, m_ThreadPriority);
	DDX_Check(pDX, IDC_RIFFWAV, m_bRiffWav);
	DDX_Check(pDX, IDC_RETAINWAVFILE, m_bRetainWAVFile);
	//}}AFX_DATA_MAP
}

BOOL CEncoderPropPage::OnInitDialog() 
{
	m_nEncoders = 0;

	CPropertyPage::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// Get active encoder type
	m_nEncoderType = g_config.GetEncoderType();

	// Clear number of encoders
	m_nEncoders = 0;

	// add all encoder
	for ( int i = 0; i< NUMENCODERS; i++ )
	{
		m_pEncoder[ m_nEncoders ] = EncoderObjectFactory( ENCODER_TYPES( i ) );

		if ( m_pEncoder[ m_nEncoders ] && m_pEncoder[ m_nEncoders ]->GetAvailable() )
		{
            CUStringConvert strCnv;

			// Get the encoder string ID from the encoder
			m_EncoderSel.AddString( strCnv.ToT( m_pEncoder[ m_nEncoders ]->GetEncoderID() ) );

			// Is this the selected encoder ?
			if ( g_config.GetEncoderType() == m_pEncoder[ m_nEncoders ]->GetID() )
			{
				m_EncoderSel.SetCurSel( m_nEncoders  );
			}

			m_nEncoders++;
		}
		else
		{
			// we don't use the created object, so delete it to avoid memory leaks
			delete m_pEncoder[ m_nEncoders ];
			m_pEncoder[ m_nEncoders ] = NULL;
		}
	}
	
	// Get the location of the dialog place holder
	CRect rcSheet;
	GetDlgItem( IDC_ENCOPTS )->GetWindowRect( &rcSheet );
	ScreenToClient( &rcSheet );
	rcSheet.left-=7;
	rcSheet.top-=7;

	// Create all the encoder dialog boxes
	for (int i = 0; i < m_nEncoders; i++ )
	{
		// Make sure that the encoder does exist
		ASSERT( m_pEncoder[ i ] );

		if ( m_pEncoder[ i ]  )
		{
			// Get a pointer to the dialog option string
			CEncoderDlg* pOptsDlg = m_pEncoder[ i ]->GetSettingsDlg();

			// Make sure it is valid
			ASSERT( pOptsDlg );

			if ( pOptsDlg )
			{
				// Create the dialog box
				pOptsDlg->Create( pOptsDlg->GetIDD(), this );
				// Set it to the proper location

				// get the previous tab order control
				UINT nFlags = SWP_NOSIZE|SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER ;

				pOptsDlg->SetWindowPos( NULL, rcSheet.left, rcSheet.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE );

				// Update controls
				pOptsDlg->SetControls( m_pEncoder[ i ] );
			}
		}
	}

	// Enable the right dialog box
	SelectEncOptionDlg();

	m_ThreadPriority.SetCurSel( g_config.GetThreadPriority() - THREAD_PRIORITY_LOWEST );

	// Update Controls
	UpdateData( FALSE );

	UpdateCtrls();

	return TRUE;
}


BOOL CEncoderPropPage::OnApply()
{
	// Call parent to update all property sheets
	((ConfigSheet*)GetParent())->OnApply();

	// Do default stuff
	return CPropertyPage::OnApply();
}

void CEncoderPropPage::UpdateControls()
{
	int nIndex = 0;

	UpdateData( TRUE );

	// Set Riff WAV output
	g_config.SetRiffWav( m_bRiffWav );

	g_config.SetRetainWavFile( m_bRetainWAVFile );

	// Get active page based on encoder selections
	nIndex = m_EncoderSel.GetCurSel();

	if ( ( nIndex < 0 )  || ( nIndex >= m_nEncoders ) )
	{
		nIndex = 0;
	}

	ASSERT( m_pEncoder[ nIndex ] );

	g_config.SetEncoderType( m_pEncoder[ nIndex ]->GetID() );

	CEncoderDlg* pOptsDlg=m_pEncoder[ nIndex ]->GetSettingsDlg();

	ASSERT( pOptsDlg );

	// Update controls
	pOptsDlg->GetControls( m_pEncoder[ nIndex ] );

	// Save the settings
	m_pEncoder[ nIndex ]->SaveSettings();

	g_config.SetThreadPriority( m_ThreadPriority.GetCurSel() + THREAD_PRIORITY_LOWEST );
}

void CEncoderPropPage::SetNormControl() 
{
	UpdateData(TRUE);
	SetModified(TRUE);
}




BEGIN_MESSAGE_MAP(CEncoderPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CEncoderPropPage)
	ON_BN_CLICKED(IDC_SELCODEC, OnSelcodec)
	ON_BN_CLICKED(IDC_RTENCODING, OnRtencoding)
	ON_CBN_SELCHANGE(IDC_BITRATE, OnChange)
	ON_BN_CLICKED(IDC_ENCTYPE, OnEnctype)
	ON_BN_CLICKED(IDC_CHANNELSELECTION, OnChange)
	ON_CBN_SELCHANGE(IDC_EMPHASIS, OnChange)
	ON_BN_CLICKED(IDC_NORMTRACK, OnChange)
	ON_BN_CLICKED(IDC_RADIO2, OnChange)
	ON_BN_CLICKED(IDC_RADIO4, OnChange)
	ON_BN_CLICKED(IDC_RADIO5, OnChange)
	ON_BN_CLICKED(IDC_RIFFWAV, OnChange)
	ON_BN_CLICKED(IDC_ENCTYPE1, OnEnctype)
	ON_EN_CHANGE(IDC_EXTENCOPTS, OnChange)
	ON_BN_CLICKED(IDC_LAYER, OnChange)
	ON_BN_CLICKED(IDC_LAYER1, OnChange)
	ON_BN_CLICKED(IDC_EXTENCTYPE, OnChange)
	ON_CBN_SELCHANGE(IDC_THREADPRIORITY, OnChange)
	ON_CBN_SELCHANGE(IDC_ENCODERSEL, OnSelchangeEncodersel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// MP3 message handler functions

void CEncoderPropPage::OnRtencoding() 
{
	SetNormControl();
}

void CEncoderPropPage::OnSelcodec() 
{
	CCodecDlg dlg;
	dlg.DoModal();
}

void CEncoderPropPage::OnChange() 
{
	SetModified(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// CEncoderPropPage message handlers


void CEncoderPropPage::UpdateCtrls() 
{
	UpdateData(TRUE);
	UpdateData(FALSE);
}

void CEncoderPropPage::OnEnctype() 
{
	SetModified(TRUE);
	UpdateCtrls();
}


void CEncoderPropPage::OnSelchangeEncodersel() 
{
	SelectEncOptionDlg();
}


void CEncoderPropPage::SelectEncOptionDlg() 
{
	int nSel = m_EncoderSel.GetCurSel();

	if ( nSel < 0 || nSel > m_nEncoders )
	{
		nSel = 0;
	}

	// Hide all dialog boxes, and save the settings
	for ( int i = 0; i < m_nEncoders; i++)
	{
		CEncoderDlg* pOptsDlg = m_pEncoder[ i ]->GetSettingsDlg();

		ASSERT( pOptsDlg );

		pOptsDlg->ShowWindow( SW_HIDE );

		// Update controls
		pOptsDlg->GetControls( m_pEncoder[ i ] );

		if ( m_nOldSel == i )
		{
			// Save the settings
			m_pEncoder[ i ]->SaveSettings();
		}
	}

	CEncoderDlg* pOptsDlg = m_pEncoder[ nSel ]->GetSettingsDlg();
	ASSERT( pOptsDlg );

	if (m_pEncoder[ nSel ]->GetRiffWavSupport())
	{
		::EnableWindow( ::GetDlgItem( this->m_hWnd, IDC_RIFFWAV ), TRUE );
	}
	else
	{
		::EnableWindow( ::GetDlgItem( this->m_hWnd, IDC_RIFFWAV ), FALSE );
	}


	pOptsDlg->ShowWindow( SW_SHOW );

	m_nOldSel = nSel;
}


