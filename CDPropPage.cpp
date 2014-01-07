/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
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


#include "StdAfx.h"
#include <limits.h>
#include "CDPropPage.h"
#include "config.h"
#include "ConfigDlg.h"
#include "AutoDetect.h"
#include "CDex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// GenPropPage message handlers
/////////////////////////////////////////////////////////////////////////////

// CDPropPage property page

IMPLEMENT_DYNCREATE(CDPropPage, CPropertyPage)

// CONSTRUCTOR of CDPropPage
CDPropPage::CDPropPage() : 
	CPropertyPage( CDPropPage::IDD )
{
    static CUStringConvert strCnv;

	m_strTitle  = g_language.GetString( IDS_CDPROPPAGE );
	m_psp.pszTitle = strCnv.ToT( m_strTitle );
	m_psp.dwFlags |= PSP_USETITLE;

	//{{AFX_DATA_INIT(CDPropPage)
	m_bUseNtScsi = FALSE;
	m_bFirstBlockOnly = FALSE;
	m_bMultipleRead = FALSE;
	m_nMultipleReadCount = 0;
	m_bLockDuringRead = FALSE;
	m_bUseCDText = FALSE;
	//}}AFX_DATA_INIT
	// Get the CD Parameters
	GetCDParams();
}


void CDPropPage::GetCDParams()
{



	// Construct CDROMPARAMS
	CDROMPARAMS cdParams;

	// Get cdParames of current selected CD-ROM
	CR_GetCDROMParameters(&cdParams);

	// m_nBlockCompare=cdParams.nNumCompareSectors;
	
	m_nSampleOffset = cdParams.nSampleOffset;
	
	if ( m_nSampleOffset == LONG_MIN )
	{
		m_nSampleOffset = 0;
	}

	m_nReadOverlap=cdParams.nNumOverlapSectors;
	m_nReadBlkSize=cdParams.nNumReadSectors;
	m_nCDSpeed = cdParams.nSpeed;
	m_nSpinUpTime=cdParams.nSpinUpTime;
	m_bSwapChannels = cdParams.bSwapLefRightChannel;
	m_bJitter=cdParams.bJitterCorrection;
	m_nStartOffset = cdParams.nOffsetStart;
	m_nEndOffset = cdParams.nOffsetEnd;
	m_bAspiPosting = cdParams.bAspiPosting;

	if ( m_RippingMethod.m_hWnd )
	{
		m_RippingMethod.SetCurSel( cdParams.nRippingMode + cdParams.nParanoiaMode );
	}		

	m_nRetries =	cdParams.nAspiRetries;
    m_bUseNtScsi = ( TRANSPLAYER_NTSCSI == g_config.GetTransportLayer() );
	m_bFirstBlockOnly = cdParams.bMultiReadFirstOnly;
	m_bMultipleRead = cdParams.bEnableMultiRead;
	m_nMultipleReadCount = cdParams.nMultiReadCount;
	m_bLockDuringRead = cdParams.bLockDuringRead;
	m_bUseCDText = cdParams.bUseCDText;

	// Set read method
	if (m_ReadMethod.m_hWnd)
		m_ReadMethod.SetCurSel((int)CR_GetCDROMType());

	m_bEjectWhenFinished = g_config.GetEjectWhenFinished();
	m_bSelectAll = g_config.GetSelectAllCDTracks();
}

void CDPropPage::SetCDParams()
{
	// Construct CDROMPARAMS
	CDROMPARAMS cdParams;

	// Get all values that have not been changed yet
	CR_GetCDROMParameters( &cdParams );

//	cdParams.nNumCompareSectors =	m_nBlockCompare;
	cdParams.nSampleOffset      =   m_nSampleOffset;
	cdParams.nNumOverlapSectors =	m_nReadOverlap;
	cdParams.nNumReadSectors =		m_nReadBlkSize;
	cdParams.nSpeed =				m_nCDSpeed;
	cdParams.nSpinUpTime =			m_nSpinUpTime;
	cdParams.bSwapLefRightChannel =	m_bSwapChannels;
	cdParams.bJitterCorrection =	m_bJitter;
	cdParams.nOffsetStart =			m_nStartOffset;
	cdParams.nOffsetEnd =			m_nEndOffset;
	cdParams.bAspiPosting =			m_bAspiPosting;
	cdParams.nAspiRetries =			m_nRetries;
	cdParams.bMultiReadFirstOnly =	m_bFirstBlockOnly;
	cdParams.bEnableMultiRead =		m_bMultipleRead;
	cdParams.nMultiReadCount =		m_nMultipleReadCount;
	cdParams.bLockDuringRead =		m_bLockDuringRead;
	cdParams.bUseCDText =			m_bUseCDText;
	cdParams.nRippingMode =			( m_RippingMethod.GetCurSel() > 0 ) ? 1 : 0 ;
	cdParams.nParanoiaMode =		m_RippingMethod.GetCurSel() - cdParams.nRippingMode ;

    // Get cdParames of current selected CD-ROM
	CR_SetCDROMParameters( &cdParams );

	// Set Drive Type (has to be after SetCDROm Parameters
	CR_SelectCDROMType( DRIVETYPE( m_ReadMethod.GetCurSel() ) );

	g_config.SetEjectWhenFinished( m_bEjectWhenFinished );
	g_config.SetSelectAllCDTracks( m_bSelectAll );

}


// CONSTRUCTOR of CDPropPage
CDPropPage::~CDPropPage()
{
}

void CDPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDPropPage)
	DDX_Control(pDX, IDC_USENTSCSI, m_UseNtScsi);
	DDX_Check(pDX, IDC_SWAPLEFTRIGHT, m_bSwapChannels);
	DDX_Check(pDX, IDC_JITTER, m_bJitter);
	DDX_Text(pDX, IDC_STARTOFFSET, m_nStartOffset);
	DDX_Text(pDX, IDC_ENDOFFSET, m_nEndOffset);
	DDX_Text(pDX, IDC_SPINUPTIME, m_nSpinUpTime);
	DDX_Text(pDX, IDC_READRETRIES, m_nRetries);
	DDX_Check(pDX, IDC_EJECTWHENFINISHED, m_bEjectWhenFinished);
	DDX_Check(pDX, IDC_SELECT_ALL_DEFAULT, m_bSelectAll);
	DDX_Check(pDX, IDC_USENTSCSI, m_bUseNtScsi);
	DDX_Check(pDX, IDC_LOCKDURINGREAD, m_bLockDuringRead);
	DDX_Control(pDX, IDC_RIPPINGMETHOD, m_RippingMethod);
	DDX_Check(pDX, IDC_USECDTEXT, m_bUseCDText);
	DDX_Control(pDX, IDC_READMETHOD, m_ReadMethod);
	DDX_Control(pDX, IDC_CDROMLIST, m_CDRomList);
	DDX_Text(pDX, IDC_EDIT1, m_nSampleOffset);
	DDX_Text(pDX, IDC_READOVERLAP, m_nReadOverlap);
	DDX_Text(pDX, IDC_READSIZE, m_nReadBlkSize);
	DDX_Text(pDX, IDC_CDSPEED, m_nCDSpeed);
	//}}AFX_DATA_MAP
}

void CDPropPage::UpdateControls()
{
	// Update controls
	UpdateData(TRUE);

	// Update the parameters
	SetCDParams();

}

BOOL CDPropPage::OnApply()
{
	// Call parent to update all property sheets
	((ConfigSheet*)GetParent())->OnApply();

	// Do default stuff
	return CPropertyPage::OnApply();
}


BEGIN_MESSAGE_MAP(CDPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDPropPage)
	ON_BN_CLICKED(IDC_DETECT, OnDetect)
	ON_CBN_SELCHANGE(IDC_CDROMLIST, OnSelchangeCdromlist)
	ON_BN_CLICKED(IDC_JITTER, OnJitter)
	ON_CBN_SELCHANGE(IDC_RIPPINGMETHOD, OnSelchangeRippingmethod)
	ON_BN_CLICKED(IDC_USENTSCSI, OnUsentscsi)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CDPropPage::FillCDRiveSelection()
{
	m_CDRomList.ResetContent();

	// Construct CDROMPARAMS
	CDROMPARAMS cdParams;

	// Obtain the active CDROM device
	int nSelCD = CR_GetActiveCDROM();

	// Fill list with CD-ROM devices
	for (int i = 0; i < CR_GetNumCDROM() ; i++ )
	{
		// Set the active CD ROM in order to get the information
		CR_SetActiveCDROM( i );

		// Get cdParames of current selected CD-ROM
		CR_GetCDROMParameters( &cdParams );


        CUStringConvert strCnv;
        
		// Add the CD-ROM ID to the combo box
		m_CDRomList.AddString( strCnv.ToT( CUString( (LPCSTR)cdParams.lpszCDROMID, CP_ACP ) ) );
	}
	
	// Select the original CD-ROM again
	CR_SetActiveCDROM( nSelCD );

	// Set selection in CD-ROM list
	m_CDRomList.SetCurSel( CR_GetActiveCDROM() );
}


BOOL CDPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );
/*
	CUString strTitle;
	GetWindowText( strTitle );

    CTabCtrl* pTab = GetTabControl();
    ASSERT (pTab);

    TC_ITEM ti;
    ti.mask = TCIF_TEXT;
    ti.pszText = pszText;
    VERIFY (pTab->SetItem (nPage, &ti));

*/

	// Update CD-Drive selection combo box
	FillCDRiveSelection();

	// Update the control parameters
	GetCDParams();

	if ( IsWindowsNTOS() )
	{
		m_UseNtScsi.EnableWindow( TRUE );
	}
	else
	{
		m_bUseNtScsi = FALSE;
		m_UseNtScsi.EnableWindow( FALSE );
	}

	EnDisAbleControls();

	UpdateData( FALSE );

	SetModified( TRUE );

	return TRUE;

}

void CDPropPage::OnDetect() 
{
	UpdateData( TRUE );

	SetModified( TRUE );	

	CUString strLang;
	strLang = g_language.GetString( IDS_AUTODEFECTWARNING );


	if ( IDYES == CDexMessageBox( strLang, MB_YESNO ) )
	{
		CAutoDetect dlg;
		if (dlg.DoModal()==IDOK)
		{
			m_ReadMethod.SetCurSel(CUSTOMDRIVE);
//			CR_SelectCDROMType(DRIVETYPE(m_ReadMethod.GetCurSel()));


			//((ConfigSheet*)GetParent())->m_pCdda->ChangeCDDevice(g_config.GetCDDevice());


			SetModified(TRUE);
			UpdateData(FALSE);
		}
	}
}


void CDPropPage::OnSelchangeRippingmethod() 
{
	UpdateData( TRUE );

	// Indicate page has been changed
	SetModified(TRUE);

	EnDisAbleControls();
}

void CDPropPage::EnDisAbleControls() 
{

	if ( m_RippingMethod.GetCurSel() > 0 )
	{
		GetDlgItem( IDC_JITTER )->EnableWindow( FALSE );
		GetDlgItem( IDC_READRETRIES )->EnableWindow( FALSE );
	}
	else
	{
		GetDlgItem( IDC_JITTER )->EnableWindow( TRUE );
		GetDlgItem( IDC_READRETRIES )->EnableWindow( TRUE );
	}


	// Set the controls
	if ( m_bJitter && 
		 ( 0 == m_RippingMethod.GetCurSel() ) )
	{
		GetDlgItem( IDC_READOVERLAP )->EnableWindow( TRUE );
		// GetDlgItem( IDC_BLOCKCOMPARE )->EnableWindow( TRUE );
	}
	else
	{
		GetDlgItem( IDC_READOVERLAP )->EnableWindow( FALSE );
		// GetDlgItem( IDC_BLOCKCOMPARE )->EnableWindow( FALSE );
	}
	
}

void CDPropPage::OnSelchangeCdromlist() 
{
	SetModified( TRUE );

	// Get current values out of controls
	UpdateData( TRUE );

	// Update current paramters
	SetCDParams();

	// Set the active CDROM
	CR_SetActiveCDROM( m_CDRomList.GetCurSel() );

	// Set parameters of newly selected CD-ROM
	GetCDParams();

	EnDisAbleControls();

	// Set new parameters into controls
	UpdateData( FALSE );
}

void CDPropPage::OnJitter() 
{
	// Get Control Values
	UpdateData( TRUE );

	// Indicate page has been changed
	SetModified( TRUE );

	EnDisAbleControls();
		   
}

void CDPropPage::OnUsentscsi() 
{
	CWaitCursor waitCursor;

	UpdateData( TRUE );

	// save settings
    g_config.SetTransportLayer( m_bUseNtScsi );
	g_config.SaveCDRomSettings();

	((CCDexApp*)AfxGetApp())->LoadCDRipLibrary();

    FillCDRiveSelection();

    m_bUseNtScsi = g_config.GetTransportLayer();

	UpdateData( FALSE );
}

