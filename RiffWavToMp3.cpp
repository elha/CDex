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
#include "RiffWavToMP3.h"
#include "AudioFile.h"


#define TIMERID 2
#define TIMERSLOT 100


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CRiffWavToMP3 dialog

// CONSTRUCTOR
CRiffWavToMP3::CRiffWavToMP3(CWnd* pParent /*=NULL*/)
	: CDialog(CRiffWavToMP3::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRiffWavToMP3)
	//}}AFX_DATA_INIT
	m_pFileDlg=NULL;
}

// DESCTURCTOR
CRiffWavToMP3::~CRiffWavToMP3()
{
	delete m_pFileDlg;
}


//	ON_WM_TIMER()

void CRiffWavToMP3::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CRiffWavToMP3)
    DDX_Control(pDX, IDC_PROGRESS, m_Progress);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_PROCESSFILENAME, m_processingFile);
}


BEGIN_MESSAGE_MAP(CRiffWavToMP3, CDialog)
	//{{AFX_MSG_MAP(CRiffWavToMP3)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDABORT, OnAbort)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRiffWavToMP3 message handlers

void CRiffWavToMP3::OnCancel() 
{
	KillTimer(TIMERID);
	CDialog::OnCancel();
}

void CRiffWavToMP3::OnTimer(UINT nIDEvent) 
{
	// Obtain the number of files
	if (m_Pos!=NULL)
	{
		// Get the file name to be converted
		CUString strFileName=m_pFileDlg->GetNextPathName(m_Pos);
		CUString strOrig(strFileName);

		// Update progress bar
		m_Progress.SetPos(m_nCurFile++);

		// Set current file name
        CUStringConvert strCnv;
        m_processingFile.SetWindowText( strCnv.ToT( strFileName ) );
		
		// Update controls
		UpdateData(FALSE);


		CWAV myWav;

		// Strip extension
		int nPos=strFileName.ReverseFind('.');

		// Stip it
		if (nPos>0)
			strFileName=strFileName.Left(nPos);

		// Do the conversion
		if (myWav.StripRiffWavHeader(strFileName)==CDEX_OK)
		{
			// delete the original file?
			if (m_pFileDlg->m_bDeleteOriginal)
				CDexDeleteFile(strOrig);
		}
	}
	else
	{
		OnCancel();
	}

	CDialog::OnTimer(nIDEvent);

}

BOOL CRiffWavToMP3::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// Get start position
	m_Pos=m_pFileDlg->GetStartPosition();

	m_nTotalFiles=0;

	// Obtain the number of files
	while (m_Pos!=NULL)
	{
		m_nTotalFiles++;
		m_pFileDlg->GetNextPathName(m_Pos);
	}	
			
	// Get start position again
	m_Pos=m_pFileDlg->GetStartPosition();
	
	m_Progress.SetRange(0,m_nTotalFiles);

	m_nCurFile=0;

	SetTimer(TIMERID,TIMERSLOT,NULL);
	
	return TRUE;
}

void CRiffWavToMP3::OnAbort() 
{
	OnCancel();
}
