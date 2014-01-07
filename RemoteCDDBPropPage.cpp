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
#include "RemoteCDDBPropPage.h"
#include "config.h"
#include "ConfigDlg.h"
#include "CDex.h"
#include "CDDBSubmitOpts.h"
#include "CDdbDlg.h"
#include "Cddb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT __cdecl QuerySitesThreadFunc( void* pParam, CWnd* pWnd, volatile BOOL& bAbort )
{
	return ((CRemoteCDDBPropPage*)pParam)->QuerySites( pWnd, bAbort );
}



/////////////////////////////////////////////////////////////////////////////
// CRemoteCDDBPropPage property page

IMPLEMENT_DYNCREATE(CRemoteCDDBPropPage, CPropertyPage)

CRemoteCDDBPropPage::CRemoteCDDBPropPage() : 
	CPropertyPage(CRemoteCDDBPropPage::IDD )
{
    static CUStringConvert strCnv;

	m_strTitle  = g_language.GetString( IDS_REMOTECDDBPROPPAGE );
    m_psp.pszTitle = strCnv.ToT( m_strTitle );
	m_psp.dwFlags |= PSP_USETITLE;

	//{{AFX_DATA_INIT(CRemoteCDDBPropPage)
	m_nCDDBPort = 0;
	m_strLocation = _T("");
	//}}AFX_DATA_INIT
	m_bAutoConnect = g_config.GetCDDBAutoConnect();
	m_strEmail=g_config.GetEmailAddress();
	m_nProxyPort = g_config.GetCDDBProxyPort();
	m_bUseProxy=g_config.GetCDDBUseProxy();
	m_strProxyAddress=g_config.GetProxyAddress();
	m_bUseAuthentication=g_config.GetCDDBUseAuthentication();
	m_strProxyUser = g_config.GetCDDBProxyUser();
	m_strProxyPassword = g_config.GetCDDBProxyPassword();
	m_nTimeOut = g_config.GetCDDBTimeOut();
	// Get Long or short CDDB directory names
}

CRemoteCDDBPropPage::~CRemoteCDDBPropPage()
{
}

void CRemoteCDDBPropPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRemoteCDDBPropPage)
	DDX_Control(pDX, IDC_PROTOCOL, m_cddbProtocol);
	DDX_Control(pDX, IDC_PROXYUSER, m_ProxyUser);
	DDX_Control(pDX, IDC_PROXYPASSWORD, m_ProxyPassword);
	DDX_Control(pDX, IDC_USEPROXY, m_UseProxy);
	DDX_Control(pDX, IDC_PROXYADDRESS, m_ProxyAddress);
	DDX_Control(pDX, IDC_PROXYPORT, m_ProxyPort);
	DDX_Control(pDX, IDC_SERVERLIST, m_ServerList);
	DDX_Text(pDX, IDC_EMAIL, m_strEmail);
	DDX_Text(pDX, IDC_PROXYADDRESS, m_strProxyAddress);
	DDX_Text(pDX, IDC_PROXYPORT, m_nProxyPort);
	DDX_Check(pDX, IDC_USEPROXY, m_bUseProxy);
	DDX_Check(pDX, IDC_USEAUTHENTICATION, m_bUseAuthentication);
	DDX_Text(pDX, IDC_PROXYUSER, m_strProxyUser);
	DDX_Text(pDX, IDC_PROXYPASSWORD, m_strProxyPassword);
	DDX_Text(pDX, IDC_TIMEOUT, m_nTimeOut);
	DDX_Check(pDX, IDC_AUTOCONNECT, m_bAutoConnect);
	DDX_Text(pDX, IDC_PATH, m_strPath);
	DDX_Text(pDX, IDC_ADDRESS, m_strAddress);
	DDX_Text(pDX, IDC_CDDBPORT, m_nCDDBPort);
	DDX_Text(pDX, IDC_LOCATION, m_strLocation);
	//}}AFX_DATA_MAP
	// Set Folder selection 
}


BEGIN_MESSAGE_MAP(CRemoteCDDBPropPage, CPropertyPage)
	//{{AFX_MSG_MAP(CRemoteCDDBPropPage)
	ON_EN_CHANGE(IDC_CDDBPATH, OnChange)
	ON_BN_CLICKED(IDC_QUERYSITES, OnQuerysites)
	ON_BN_CLICKED(IDC_SUBMITOPTS, OnSubmitopts)
	ON_BN_CLICKED(IDC_RESETBUTTON, OnResetbutton)
	ON_BN_CLICKED(IDC_ADDSITE, OnAddsite)
	ON_EN_CHANGE(IDC_LOCATION, OnChangeLocation)
	ON_EN_CHANGE(IDC_CDDBPORT, OnChangeLocation)
	ON_CBN_SELCHANGE(IDC_PROTOCOL, OnSelchangeProtocol)
	ON_CBN_SELCHANGE(IDC_SERVERLIST, OnSelchangeServerlist)
	ON_BN_CLICKED(IDC_USEPROXY, OnChange)
	ON_BN_CLICKED(IDC_PROTOCL, OnChange)
	ON_BN_CLICKED(IDC_CDDBTYPE, OnChange)
	ON_BN_CLICKED(IDC_STCDINI, OnChange)
	ON_BN_CLICKED(IDC_STLOCALCDDB, OnChange)
	ON_BN_CLICKED(IDC_PROTOCOLHTTP, OnChange)
	ON_BN_CLICKED(IDC_USEAUTHENTICATION, OnChange)
	ON_EN_CHANGE(IDC_ADDRESS, OnChangeAddress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CRemoteCDDBPropPage::UpdateControls()
{
	UpdateData(TRUE);

	// Get E-Mail address
	g_config.SetEmailAddress( CUString( m_strEmail ) );
	
	// Get selected server selection
	g_RemoteSites.SetActiveSite(m_nLastServerSelection);

	// Set Server Address
	g_RemoteSites.SetSite(	m_nLastServerSelection,
							CUString( m_strAddress ),
							CUString( m_strLocation ),
							CUString( m_strPath ),
							m_nCDDBPort,
							m_cddbProtocol.GetCurSel() );

	// Get proxy port 
	g_config.SetCDDBProxyPort(m_nProxyPort);

	// Get Use Proxy
	g_config.SetCDDBUseProxy(m_bUseProxy);

	// Get Proxy Address
	g_config.SetProxyAddress( CUString( m_strProxyAddress ));

	// Get Use Proxy Authentication
	g_config.SetCDDBUseAuthentication(m_bUseAuthentication);

	// Get Proxy User
	g_config.SetCDDBProxyUser( CUString( m_strProxyUser ));

	// Get Proxy Password
	g_config.SetCDDBProxyPassword( CUString( m_strProxyPassword ));

	// Get TimeOut
	g_config.SetCDDBTimeOut(m_nTimeOut);

	// Get Auto Connect
	g_config.SetCDDBAutoConnect(m_bAutoConnect);
}


void CRemoteCDDBPropPage::OnChange() 
{
	SetModified(TRUE);
	UpdateCtrls();
}

BOOL CRemoteCDDBPropPage::OnApply()
{
	// Call parent to update all property sheets
	((ConfigSheet*)GetParent())->OnApply();

	// Do default stuff
	return CPropertyPage::OnApply();
}

void CRemoteCDDBPropPage::UpdateCtrls()
{
	// Get data out of controls
	UpdateData(TRUE);


	// Do we have to use the proxy server?
	m_UseProxy.EnableWindow(TRUE);
	if (m_bUseProxy)
	{
		// Guess so, enable ProxyPort control
		m_ProxyPort.EnableWindow(TRUE);
		m_ProxyAddress.EnableWindow(TRUE);

		if (m_bUseAuthentication)
		{
			m_ProxyUser.EnableWindow(TRUE);
			m_ProxyPassword.EnableWindow(TRUE);
		}
	}
	else
	{
		// Nope, disable ProxyPort control
		m_ProxyPort.EnableWindow(FALSE);
		m_ProxyAddress.EnableWindow(FALSE);

		m_ProxyUser.EnableWindow(FALSE);
		m_ProxyPassword.EnableWindow(FALSE);
	}

	// Get the current settings
	int nCurrentServerSelection = m_ServerList.GetCurSel();
	m_strAddress	= g_RemoteSites.GetAddress( nCurrentServerSelection );
	m_strLocation	= g_RemoteSites.GetLocation( nCurrentServerSelection );
	m_strPath		= g_RemoteSites.GetPath( nCurrentServerSelection );
	m_nCDDBPort		= g_RemoteSites.GetPort( nCurrentServerSelection );
	m_cddbProtocol.SetCurSel(g_RemoteSites.GetProtocol( nCurrentServerSelection ));


	// Do we have to enable the CDDB server info edit windows?
	GetDlgItem(IDC_PATH)->EnableWindow(g_RemoteSites.IsEditable( nCurrentServerSelection ));
	GetDlgItem(IDC_ADDRESS)->EnableWindow(g_RemoteSites.IsEditable( nCurrentServerSelection ));
	GetDlgItem(IDC_CDDBPORT)->EnableWindow(g_RemoteSites.IsEditable( nCurrentServerSelection ));
	GetDlgItem(IDC_LOCATION)->EnableWindow(g_RemoteSites.IsEditable( nCurrentServerSelection ));
	GetDlgItem(IDC_PROTOCOL)->EnableWindow(g_RemoteSites.IsEditable( nCurrentServerSelection ));

	// And update controls again
	UpdateData(FALSE);
}


void CRemoteCDDBPropPage::GetRemoteServers() 
{
	// Clear all entries from the list
	m_ServerList.ResetContent();

	// Add remote sites
	int nNumSites=g_RemoteSites.GetNumSites();
	
	
	for (int i=0;i<nNumSites;i++)
	{
		CUString strTmp;
		strTmp=g_RemoteSites.GetAddress(i);
		if (g_RemoteSites.GetProtocol(i)==OPTIONS_CDDB_USEHTTP)
			strTmp += _W( " http " );
		else
			strTmp += _W( " cddbp " );

		CUString strPort;

		strPort.Format( _W( " %d " ), g_RemoteSites.GetPort( i ) );
		strTmp += strPort;

		strTmp += CUString( _W( " (" ) + g_RemoteSites.GetLocation( i ) + _W( ")" ) );

        CUStringConvert strCnv;

        m_ServerList.AddString( strCnv.ToT( strTmp ) );
	}

	// Update the selection in the list box
	m_ServerList.SetCurSel(g_RemoteSites.GetActiveSite());

	m_nLastServerSelection = g_RemoteSites.GetActiveSite();

}


BOOL CRemoteCDDBPropPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// Get the Remote Servers
	GetRemoteServers();

	// Do update the data controls
	UpdateCtrls();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRemoteCDDBPropPage::OnSubmitopts() 
{
	CCDDBSubmitOpts dlg;
	dlg.DoModal();
}

void CRemoteCDDBPropPage::OnQuerysites() 
{
	// Get latest setting out of controls
	UpdateControls();

	// Create Remote CDDB dialog box
	CCDdbDlg dlg( QuerySitesThreadFunc, this );
	dlg.DoModal();

	// Update controls
	UpdateData(FALSE);

}


/////////////////////////////////////////////////////////////////////////////
// CRemoteCDDBPropPage message handlers

/////////////////////////////////////////////////////////////////////////////

UINT CRemoteCDDBPropPage::QuerySites( CWnd* pWnd, volatile BOOL& bAbort ) 
{
	// Create a new CDDB object
	CDdb myCDDB( NULL, pWnd );

	// Do a query sites
	UINT nReturn = myCDDB.QuerySites();
	
	if ( TRUE == nReturn )
	{
		// Add new site to dialog box
		GetRemoteServers();

		// Set the active site to zero
		g_RemoteSites.SetActiveSite(0);
	}
	return nReturn;
}


void CRemoteCDDBPropPage::OnResetbutton() 
{
	// Clear all entries
	g_RemoteSites.ClearAll();

	// Add Default Entries
	g_RemoteSites.AddDefaultSites();

	GetRemoteServers();

	m_nLastServerSelection=0;

	// Set the new selection
	UpdateCtrls();
}

void CRemoteCDDBPropPage::OnAddsite() 
{

    CUStringConvert strCnv;

    UpdateControls();

	g_RemoteSites.AddSite( 
        _W( "localhost" ), 
        _W( "localserver" ), 
        _W( "/~cddb/cddb.cgi" ), 
        80, 
        OPTIONS_CDDB_USEHTTP, 
        TRUE );

	CUString strTmp( _W( "localhost http  80 ( localserver )" ) );

	int nNewSelection = m_ServerList.InsertString( -1, strCnv.ToT( strTmp ) );

	m_ServerList.SetCurSel( nNewSelection );

	// Set the new selection
	UpdateCtrls();

	m_nLastServerSelection = nNewSelection;

}

void CRemoteCDDBPropPage::OnChangeLocation() 
{
	OnChangeAddress();
}

void CRemoteCDDBPropPage::OnChangeHttppath() 
{
	OnChangeAddress();
}

void CRemoteCDDBPropPage::OnSelchangeProtocol() 
{
	OnChangeAddress();
}

void CRemoteCDDBPropPage::OnChangeAddress() 
{
	UpdateData( TRUE );
	
	int nCurrentSelection = m_ServerList.GetCurSel( );

	CUString strTmp( m_strAddress );

	if ( m_cddbProtocol.GetCurSel()==OPTIONS_CDDB_USEHTTP)
	{
		strTmp += _W( " http " );
	}
	else
	{
		strTmp += _W( " cddbp " );
	}

	CUString strPort;

	strPort.Format( _W( " %d " ), m_nCDDBPort );
	strTmp += strPort;

	strTmp += _W( " (" ) + CUString( m_strLocation ) + _W( ")" );

	// Replace combo string
	m_ServerList.DeleteString( nCurrentSelection );
    
    CUStringConvert strCnv;
	m_ServerList.InsertString( nCurrentSelection, strCnv.ToT( strTmp ) );

	m_ServerList.SetCurSel( nCurrentSelection );

	UpdateData( FALSE );
}

void CRemoteCDDBPropPage::OnSelchangeServerlist() 
{
	// update the current settings
	UpdateControls();

	// Set the new selection
	UpdateCtrls();

	// Set the last active server selection
	m_nLastServerSelection = m_ServerList.GetCurSel();
}





