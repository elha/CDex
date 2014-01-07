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


#include "stdafx.h"
#include "cdex.h"
#include "AutoDetect.h"
#include "Config.h"
#include "Util.h"
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CDROMPARAMS cdOldParams;


int CheckDensities[]={0x00,0x04,0x82};

#define AUTODETECT_OUTPUT_FILENAME (g_config.GetAppPath()+ _W( "\\CDexAutoDetect.txt"))

CADebug	dbgFile;

static CWinThread*	g_pThread=NULL;
BOOL	bThreadActive=FALSE;


const int NUMDENSITIES=sizeof(CheckDensities)/sizeof(CheckDensities[0]);
const int NUMATAPI=2;

const int nNumTableEntries=NUMATAPI*NUMREADMETHODS*NUMENABLEMODES*NUMDENSITIES;

static DRIVETABLE myDriveTable[nNumTableEntries];
int		nCurrentDrive=0;

INITTRACE( _T( "AutoDetect" ) );

static short SWAPSHORT(short sSwap)
{
	return ((sSwap>>8)&0xFF) + ((sSwap&0xFF) <<8);
}


/////////////////////////////////////////////////////////////////////////////
// CAutoDetect dialog


CAutoDetect::CAutoDetect(CWnd* pParent /*=NULL*/)
	: CDialog(CAutoDetect::IDD, pParent)
{
	g_pThread=NULL;
	m_pWaitCursor=NULL;
	m_bAbort=FALSE;

	dbgFile.Open( AUTODETECT_OUTPUT_FILENAME, TRUE );

	//{{AFX_DATA_INIT(CAutoDetect)
	// m_strMMC = _T("");
	//}}AFX_DATA_INIT
}

CAutoDetect::~CAutoDetect()
{
	// Delete wait cursor if it still does exist
	delete m_pWaitCursor;

	// Delete thread
	if (g_pThread)
		::TerminateThread(g_pThread->m_hThread,0);

	dbgFile.Close();
}


void CAutoDetect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAutoDetect)
	DDX_Control(pDX, IDC_LIST, m_List);
	// DDX_Text(pDX, IDC_MMCSTATUS, m_strMMC);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAutoDetect, CDialog)
	//{{AFX_MSG_MAP(CAutoDetect)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_COMMAND(ID_INITDETECTION, OnInitDetection)
	ON_BN_CLICKED(IDC_ABORT, OnAbort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAutoDetect message handlers

BOOL CAutoDetect::OnInitDialog() 
{
	bThreadActive=FALSE;

	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// No drives found yet
	nCurrentDrive=0;

	CRect rcClient;

	// Get size of list control
	m_List.GetClientRect(rcClient);

	int iColWidth=rcClient.Width() / 6;

	// Create columns of track list
    CUStringConvert strCnv;
 
    m_List.InsertColumn( 0, strCnv.ToT( g_language.GetString( IDS_READ   )), LVCFMT_LEFT,  iColWidth, 0 );
	m_List.InsertColumn( 1, strCnv.ToT( g_language.GetString( IDS_ATAPI  )), LVCFMT_RIGHT, iColWidth, 0 );
	m_List.InsertColumn( 2, strCnv.ToT( g_language.GetString( IDS_ENABLE )), LVCFMT_RIGHT, iColWidth, 0 );
	m_List.InsertColumn( 3, strCnv.ToT( g_language.GetString( IDS_DENSITY)), LVCFMT_RIGHT, iColWidth*3/4, 0 );
	m_List.InsertColumn( 4, strCnv.ToT( g_language.GetString( IDS_ENDIAN )), LVCFMT_RIGHT, iColWidth*3/4, 0 );
	m_List.InsertColumn( 5, strCnv.ToT( g_language.GetString( IDS_RESULT )), LVCFMT_RIGHT, iColWidth, 0 );

	// Construct CDROMPARAMS
	CDROMPARAMS cdParams;

	// Get cdParames of current selected CD-ROM
	CR_GetCDROMParameters(&cdParams);

	dbgFile.printf("Start Auto probing info for drive %s \n\n",cdParams.lpszCDROMID);


	UpdateData(FALSE);

	// Disable OK button
	GetDlgItem(IDOK)->EnableWindow(FALSE);

	PostMessage(WM_COMMAND,ID_INITDETECTION);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CAutoDetect::OnInitDetection()
{

	// Get the CDROM parameters of the active drive
	CR_GetCDROMParameters(&cdOldParams);

	// Update Controls
	UpdateData(FALSE);

//	m_pWaitCursor=new CWaitCursor();
//	BeginWaitCursor();

	//PostMessage(WM_COMMAND,ID_STARTDETECTION);
	g_pThread=AfxBeginThread(OnStartDetection,this);

	// g_pThread=AfxBeginThread(OnStartDetectionCache,this);

	// Get MMC status of drive
//	m_pCdda->IsMMC(m_strMMC);
//	dbgFile.printf("MMC status is reporting: %s\n",m_strMMC);
	UpdateData(FALSE);
}

CDEX_ERR CAutoDetect::TestDrive(CDROMPARAMS& cdTestParams)
{
	BOOL bAbort = FALSE;

	// Step 1, setup the ripper
	LONG nBufferSize=0;
	LONG nNumBytesRead=0;

	cdTestParams.nNumReadSectors=10;
	cdTestParams.nNumOverlapSectors=1;
	cdTestParams.DriveTable.DriveType=CUSTOMDRIVE;
	cdTestParams.DriveTable.SetSpeed=SPEEDNONE;
	cdTestParams.nAspiTimeOut=3000;
	cdTestParams.nAspiRetries=0;

	CR_SetCDROMParameters(&cdTestParams);

	// Get the TOC information:
	// Get the TOC entry

	CR_ReadToc();

	TOCENTRY myToc=CR_GetTocEntry(0);
	TOCENTRY myToc1=CR_GetTocEntry(1);

	DWORD dwStartSector=(myToc.dwStartSector+myToc1.dwStartSector)/2;
	DWORD dwEndSector= dwStartSector+5;


#ifdef _DEBUG_NO
	// Test C2 error detect
	DWORD dwSector = myToc.dwStartSector;

	DWORD dwNumSectors = 20;

	while ( dwSector < myToc1.dwStartSector )
	{
		DWORD adwErrorSectors[ 20 ];
		DWORD dwErrors = 0;
		CDEX_ERR bReturn = CR_ScanForC2Errors(	
								dwSector,
								dwNumSectors,
								dwErrors,
								adwErrorSectors );
		dwSector += dwNumSectors;
		if ( dwErrors > 0 )
		{
			LTRACE( _T( "C2 errors detected in sector %d : %d\n" ), dwSector, dwErrors );
		}
	}

#endif

	// create the stream buffer
    long numSectors = 5;
    nBufferSize = ( 2352 + 300 ) * numSectors;

	vector<BYTE> pbtStream( nBufferSize );

    memset( &pbtStream[0], 0, nBufferSize );
    
    nNumBytesRead = 2352 * numSectors;

	// Read one chunk
    if ( CDEX_ERROR == CR_ReadBlockDirect( &pbtStream[ 0 ], pbtStream.size(), dwStartSector, numSectors ) )
	{
		return CDEX_ERROR;
	}

	BOOL bHasData=FALSE;

	// Analyze data
	for (int i=0;i<nNumBytesRead;i++)
	{
		if (pbtStream[i]>0)
		{
			bHasData=TRUE;
		}
	}

	if (bHasData==FALSE)
	{
		// Close the Ripper session
        ULONG lCRC = 0;
        //CR_CloseRipper( &lCRC);
		return CDEX_ERROR;
	}

	int iBigEndian=0;
	int iLitleEndian=0;

	// Two bytes for each sample
	short *psBuffer=(short*)&pbtStream[ 0 ];

	// Init the stuff to calculate delta samples
	short sPrevLitle=pbtStream[0];
	short sPrevBig=SWAPSHORT(sPrevLitle);

	// Test Endian
	for (int i=0;i<nNumBytesRead/2;i++)
	{
		short sLitle=psBuffer[i];
		short sBig=SWAPSHORT(sLitle);

		if ( abs((int)sLitle-(int)sPrevLitle)> abs((int)sBig-(int)sPrevBig))
		{
			iBigEndian++;
		}
		else
		{
			iLitleEndian++;
		}
		sPrevLitle=sLitle;
		sPrevBig=sBig;

	}

	if (iBigEndian>iLitleEndian)
	{
		cdTestParams.DriveTable.Endian=ENDIAN_BIG;
	}
	else
	{
		cdTestParams.DriveTable.Endian=ENDIAN_LITTLE;
	}

	// Close the Ripper session
    ULONG lCRC = 0;
    CR_CloseRipper( &lCRC);

	return CDEX_OK;
}


UINT CAutoDetect::OnStartDetectionCache(void *lpParam)
{
    const int bufferSize = 65535;
	// create the stream buffer
	BYTE* pbtStream = new BYTE[ bufferSize ];

	// Read one chunk
    for ( DWORD loop = 1; loop < 100; loop++ )
    {
        DWORD numSectors = loop;
        LONG  sector = 0;

        __int64 startTime = PerformanceCounterGetTime();
        

        while ( numSectors > 0 )
        {
            DWORD readSectors = min( numSectors, 20 );
	        if	( CDEX_ERROR == CR_ReadBlockDirect( pbtStream, bufferSize, sector,  readSectors  ) )
	        {
		        ASSERT( FALSE );
		        return CDEX_ERROR;
	        }
            numSectors -= readSectors;
            sector += readSectors;
        }

        double deltaTime = PerformanceCounterGetDelta( startTime, PerformanceCounterGetTime() );

        CUString strOut;
        strOut.Format( _W( "sectors: %d  bytes: %d time(ms)=%10.3f transfer rate: %12.5f\n" ), loop, loop * 2352,deltaTime,(double)loop * 2352.0 / deltaTime / 1024.0 / 1024.0 );
        CUStringConvert strCnv; 
        OutputDebugString( strCnv.ToT( strOut ) );
    }
    delete [] pbtStream;
    return CDEX_OK;
}

UINT CAutoDetect::OnStartDetection(void *lpParam)
{
	bThreadActive=TRUE;
	int nInsertItem=0;
	int Endian=0;
	CUString strTmp;
	CUString strOk;
	CUString strRead;
	CUString strEbl;
	CUString strEnd;
	CUString strDens;
	CUString strAtapi;

	g_pThread->BeginWaitCursor();

	CAutoDetect* pDlg=(CAutoDetect*)lpParam;

	CDROMPARAMS cdTest;
	CR_GetCDROMParameters(&cdTest);


	CUString ReadMethodDescr[]={	_W("MMC     "),
								_W("Standard"),
								_W("Nec     "),
								_W("Sony    "),
								_W("MMC1    "),
								_W("MMC2    "),
								_W("MMC3    "),
								_W("Read C1 "),
								_W("Read C2 "),
								_W("Read C3 "),
								_W("MMC4    "),
								_W("????????")};

	CUString EnableDescr[]={		g_language.GetString( IDS_NONE ),
								g_language.GetString( IDS_STANDARD ),
								_W( "????" )};

	// Write header to Log file
	dbgFile.printf( "ATAPI\tREAD\t\tENABLE\t\t%DENS\tENDIAN\tRESULT\n" );

	for (int atapi = 0; atapi < NUMATAPI; atapi++ )
	{
		if ( 0 == atapi )
		{
			cdTest.DriveTable.bAtapi = FALSE;
			strAtapi = g_language.GetString( IDNO );
		}
		else
		{
			cdTest.DriveTable.bAtapi = TRUE;
			strAtapi = g_language.GetString( IDYES );
		}


		for (int rm = 0; rm < NUMREADMETHODS; rm++ )
		{
			strRead=ReadMethodDescr[ rm ];

			for (int en=0;en<NUMENABLEMODES;en++)
			{
				strEbl = EnableDescr[ en ];

				for (int ds=0;ds<NUMDENSITIES;ds++)
				{
					cdTest.DriveTable.nDensity = CheckDensities[ds];

					strDens.Format( _W( "%2x" ), cdTest.DriveTable.nDensity );

                    CUStringConvert strCnv;
                    CUStringConvert strCnv1;
                    CUStringConvert strCnv2;
                    CUStringConvert strCnv3;

					// Log what we are doing to file
					dbgFile.printf("%s\t%s\t%s\t%s\t",
                        strCnv.ToACP( strAtapi ),
                        strCnv1.ToACP( strRead ),
                        strCnv2.ToACP( strEbl ),
                        strCnv3.ToACP( strDens ) );

					cdTest.DriveTable.ReadMethod= READMETHOD( rm );

					switch ( cdTest.DriveTable.ReadMethod )  
					{
						case READMMC:
						case READMMC2:
						case READMMC3:
						case READMMC4:
							cdTest.DriveTable.SetSpeed= SPEEDMMC;
						break;
						case READSONY:
							cdTest.DriveTable.SetSpeed= SPEEDSONY;
						break;
						case READ10:
						case READNEC:
						case READC1:
						case READC2:
						case READC3:
						default:
							cdTest.DriveTable.SetSpeed= SPEEDNONE;
						break;
					}

					cdTest.DriveTable.EnableMode= ENABLEMODE(en);
					cdTest.DriveTable.Endian = ENDIAN_LITTLE;

					CDEX_ERR err = TestDrive( cdTest );

					strEnd = _T( "-" );

					if (err==CDEX_OK)
					{
						strOk = g_language.GetString( IDS_PASS );

						if ( ENDIAN_LITTLE == cdTest.DriveTable.Endian )
						{
							strEnd = g_language.GetString( IDS_LITTLEENDIAN );
						}
						else
						{
							strEnd = g_language.GetString( IDS_BIGENDIAN );
						}
					}
					else
					{
						strOk = g_language.GetString( IDS_FAIL );
					}


 					pDlg->m_List.InsertItem( nInsertItem, strCnv.ToT( strRead ) );
					pDlg->m_List.SetItemText( nInsertItem,1,strCnv.ToT( strAtapi ) );
					pDlg->m_List.SetItemText( nInsertItem,2,strCnv.ToT( strEbl ) );
					pDlg->m_List.SetItemText( nInsertItem,3,strCnv.ToT( strDens ) );
					pDlg->m_List.SetItemText( nInsertItem,4,strCnv.ToT( strEnd ) );
					pDlg->m_List.SetItemText( nInsertItem,5,strCnv.ToT( strOk ) );
					
					myDriveTable[nCurrentDrive++]=cdTest.DriveTable;

					ASSERT(nCurrentDrive<=nNumTableEntries);

					// Log success
					dbgFile.printf("%s\t%s\n",strEnd,strOk);

					nInsertItem++;

					::Sleep(0);

					if (pDlg->m_bAbort)
					{
						pDlg->	GetDlgItem(IDC_ABORT)->EnableWindow(FALSE);
						pDlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
						g_pThread->EndWaitCursor();
						return 0;
					}
				}
			}
		}
	}

	pDlg->	GetDlgItem(IDC_ABORT)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
	g_pThread->EndWaitCursor();

	return 0;
}

void CAutoDetect::OnOK() 
{
	int nSelection=m_List.GetNextItem(-1,LVNI_FOCUSED);

	if (nSelection>=0)
	{
		CDROMPARAMS cdTest;
		
		// Get CDROM Parameters
		CR_GetCDROMParameters(&cdTest);
		
		// Set drive table settings
		cdTest.DriveTable=myDriveTable[nSelection];

		// Set CDROM parameters
		CR_SetCDROMParameters(&cdTest);

		CDialog::OnOK();
	}
	else
	{
		CUString strLang;
		strLang = g_language.GetString( IDS_PLEASE_SELECTLISTEDITEM );

		CDexMessageBox( strLang );
	}
}

void CAutoDetect::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CAutoDetect::OnCancel() 
{
	// Get previous selected item
	CR_SetCDROMParameters(&cdOldParams);

	// Close dialog
	CDialog::OnCancel();
}

void CAutoDetect::OnAbort() 
{
	m_bAbort=TRUE;


	if (g_pThread)
	{
		DWORD dwStillAlive=0;
		dwStillAlive=WaitForSingleObject(g_pThread->m_hThread,2000);
		if (dwStillAlive==WAIT_TIMEOUT)
		{
			// Kill Thread
//			KillThread
		}
	}

}
