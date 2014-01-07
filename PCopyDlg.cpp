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
#include "PCopyDlg.h"
#include "CopyDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PCopyDlg dialog


PCopyDlg::PCopyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(PCopyDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(PCopyDlg)
	m_nOutputType = -1;
	m_strFileName = _T("");
	m_nStartF = 0;
	m_nStartM = 0;
	m_nStartS = 0;
	m_nEndF = 0;
	m_nEndM = 0;
	m_nEndS = 0;
	m_strInfo = _T("");
	//}}AFX_DATA_INIT
	m_bInit=FALSE;
}


void PCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(PCopyDlg)
	DDX_Control(pDX, IDC_MP3TYPE, m_MP3Type);
	DDX_Control(pDX, IDC_SPINSS, m_SpinSS);
	DDX_Control(pDX, IDC_SPINSM, m_SpinSM);
	DDX_Control(pDX, IDC_SPINSF, m_SpinSF);
	DDX_Control(pDX, IDC_SPINES, m_SpinES);
	DDX_Control(pDX, IDC_SPINEM, m_SpinEM);
	DDX_Control(pDX, IDC_SPINEF, m_SpinEF);
	DDX_Control(pDX, IDC_STARTTRACK, m_StartTrack);
	DDX_Control(pDX, IDC_ENDTRACK, m_EndTrack);
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	DDX_Radio(pDX, IDC_OUTPUTTYPE, m_nOutputType);
	DDX_Text(pDX, IDC_STARTF, m_nStartF);
	DDX_Text(pDX, IDC_STARTM, m_nStartM);
	DDX_Text(pDX, IDC_STARTS, m_nStartS);
	DDX_Text(pDX, IDC_ENDF, m_nEndF);
	DDX_Text(pDX, IDC_ENDM, m_nEndM);
	DDX_Text(pDX, IDC_ENDS, m_nEndS);
	DDX_Text(pDX, IDC_INFO, m_strInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PCopyDlg, CDialog)
	//{{AFX_MSG_MAP(PCopyDlg)
	ON_EN_CHANGE(IDC_ENDF, ValidateEndRange)
	ON_EN_CHANGE(IDC_STARTF, ValidateStartRange)
	ON_CBN_SELCHANGE(IDC_STARTTRACK, OnSelchangeStarttrack)
	ON_CBN_SELCHANGE(IDC_ENDTRACK, OnSelchangeEndtrack)
	ON_EN_CHANGE(IDC_ENDM, ValidateEndRange)
	ON_EN_CHANGE(IDC_ENDS, ValidateEndRange)
	ON_EN_CHANGE(IDC_STARTM, ValidateStartRange)
	ON_EN_CHANGE(IDC_STARTS, ValidateStartRange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PCopyDlg message handlers


void PCopyDlg::CalcSectors()
{
	CUString strLang;

	DWORD dwFirstOffSet = m_Tasks.GetTaskInfo( m_nStartT - 1 ).GetStartSector();
	DWORD dwLastOffSet = m_Tasks.GetTaskInfo(  m_nEndT - 1 ).GetStartSector();

	m_dwFirstSector = dwFirstOffSet + (DWORD)m_nStartF + ( (DWORD)m_nStartS+(DWORD)m_nStartM*60) * 75;
	m_dwLastSector = dwLastOffSet + (DWORD)m_nEndF + ( (DWORD)m_nEndS + (DWORD)m_nEndM * 60 ) * 75;

	DWORD dwDelta=m_dwLastSector-m_dwFirstSector;

	// Calculate sectors to time
	int nF = (int)( dwDelta % 75 );
	int nS = (int)( ( dwDelta / 75 ) % 60 );
	int nM = (int)( ( dwDelta / 75 ) / 60 );

	strLang = g_language.GetString( IDS_TOTALRECORDING_TIME );

    CUStringConvert strCnv;
	m_strInfo.Format( strCnv.ToT( strLang ), nM, nS, nF, dwDelta );

}


void PCopyDlg::ValidateEndRange() 
{
	if (m_bInit)
	{

		UpdateData(TRUE);
		
		// Start at the end of the track
		// So get length of current track in sectors
		DWORD dwDelta = m_Tasks.GetTaskInfo( m_nEndT-1 ).GetEndSector() - 
						m_Tasks.GetTaskInfo( m_nEndT-1 ).GetStartSector();

		// Calculate sectors to time
		int nF= (int)( dwDelta % 75 );
		int nS= (int)( (dwDelta / 75 ) %60 );
		int nM= (int)( (dwDelta / 75 ) /60 );

		// Set minute range
		m_nEndM=min(max(m_nEndM,0),nM);

		// Are we operating at the last minute of this track, so check seconds and frames
		if ( m_nEndM > nM )
		{
			// Make sure, the seconds don't exceed the track time
			m_nEndS = min( m_nEndS, nS );

			// Are we operating at the last second of this track, so check frames
			if ( m_nEndS > nS )
			{
				// Make sure, the frames don't exceed the track time
				m_nEndF= min( m_nEndF, nF );
			}
		}

		// Recalculate the number of sectors
		CalcSectors();

		UpdateData(FALSE);
	}
}

void PCopyDlg::ValidateStartRange() 
{
	if (m_bInit)
	{
		UpdateData(TRUE);

		if ( m_nStartT >= m_nStartT )
		{
			m_nStartT = m_nStartT;

		}

		// Get Offset in sectors
		DWORD dwFirstOffSet = m_Tasks.GetTaskInfo( m_nStartT-1 ).GetStartSector();

		// Get difference
		DWORD dwDelta = m_Tasks.GetTaskInfo( m_nStartT-1 ).GetEndSector() - 
						dwFirstOffSet;

		int nStartF = (int)( dwDelta % 75 );
		int nStartS = (int)( (dwDelta / 75) %60 );
		int nStartM = (int)( (dwDelta / 75) /60 );


		if ( m_nStartM > nStartM )
		{	
			m_nStartM = nStartM;
			m_nStartS = nStartS;
			m_nStartF = nStartF;
		}
		else if ( m_nStartM == nStartM )
		{
			if ( m_nStartS > nStartS )
			{
				m_nStartS = nStartS;
				m_nStartF = nStartF;
			}
			else if ( m_nStartS == nStartS )
			{
				if ( m_nStartF > nStartF )
				{
					m_nStartF = nStartF;
				}
			}
		}

		if ( m_nStartT == m_nEndT )
		{
			if ( m_nStartM > m_nEndM )
			{
				m_nStartM = m_nEndM;

				// Is start second equal to end second
				if ( m_nStartS > m_nEndS )
				{
					m_nStartS = m_nEndS;

					if ( m_nStartF > m_nEndF )
					{
						m_nStartF = m_nEndF;
					}
				}
			}
		}

		// Recalculate sectors
		CalcSectors();

		UpdateData(FALSE);
	}
}

BOOL PCopyDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	m_SpinEM.SetRange( 0,99 );
	m_SpinES.SetRange( 0,59 );
	m_SpinEF.SetRange( 0,74 );

	m_SpinSM.SetRange( 0,99 );
	m_SpinSS.SetRange( 0,59 );
	m_SpinSF.SetRange( 0,74 );

	m_dwFirstSector = m_Tasks.GetTaskInfo( m_wFirstTrack ).GetStartSector();
	m_dwLastSector = m_Tasks.GetTaskInfo( m_wLastTrack ).GetEndSector();

    m_nStartT = m_Tasks.GetTaskInfo( m_wFirstTrack ).GetTagData().GetTrackNumber( );
	m_nStartF = 0;
	m_nStartS = 0;
	m_nStartM = 0;

	DWORD dwDelta = m_dwLastSector - 
					m_Tasks.GetTaskInfo( m_wLastTrack ).GetStartSector();

    m_nEndT= m_Tasks.GetTaskInfo( m_wLastTrack ).GetTagData().GetTrackNumber();

	m_nEndF= (int)( dwDelta % 75 );
	m_nEndS= (int)( (dwDelta / 75) %60 );
	m_nEndM= (int)( (dwDelta / 75) /60 );


	for ( unsigned int i = 0 ; i < m_Tasks.GetNumTasks(); i++ )
	{
		CString strTmp;

        CUStringConvert strCnv;
    	strTmp = strCnv.ToT( m_Tasks.GetTaskInfo( i ).GetTagData().GetTitle() );

        if ( strTmp.GetLength() > 25 )
        {
			strTmp = _T( "..." ) + strTmp.Right( 21 );
        }

		m_StartTrack.AddString(strTmp);
		m_EndTrack.AddString(strTmp);
	}

	// Select proper Start Track 
	m_StartTrack.SetCurSel( m_nStartT-1 );

	// Select proper Start Track 
	m_EndTrack.SetCurSel( m_nEndT-1 );

	OnSelchangeStarttrack();
	OnSelchangeEndtrack();

	// Calculate sectors
	CalcSectors();

	// Update Controls
	UpdateData(FALSE);

	// Now everything is initialized, it OKAY to validate the stuff
	m_bInit=TRUE;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void PCopyDlg::OnSelchangeStarttrack() 
{
	// Get new Selection
	UpdateData(TRUE);

	// Set New Start Track
	m_nStartT = m_StartTrack.GetCurSel()+1;

	// Make sure that end track >= start track
	m_nStartT = min( m_nStartT, m_nEndT );

	// And set current selection again
	m_StartTrack.SetCurSel( m_nStartT-1 );

	m_wFirstTrack = m_StartTrack.GetCurSel();

	// Sanity check
	ASSERT( m_nStartT > 0 );
	ASSERT( m_nStartT <= (int)m_Tasks.GetNumTasks() );
	ASSERT( m_nEndT >= m_nStartT );

	// Start at the begining of the track
	m_nStartF = 0;
	m_nStartS = 0;
	m_nStartM = 0;

	// Recalculate sectors
	CalcSectors();

	// Update Controls
	UpdateData(FALSE);
}

void PCopyDlg::OnSelchangeEndtrack() 
{
	// Get new Selection
	UpdateData( TRUE );

	// Set New End Track
	m_nEndT = m_EndTrack.GetCurSel() + 1;

	// Make sure that end track >=start track
	m_nEndT = max( m_nStartT, m_nEndT );

	// And set current selection again
	m_EndTrack.SetCurSel( m_nEndT - 1 );

	m_wLastTrack = m_EndTrack.GetCurSel();

	// Sanity check
	ASSERT(m_nEndT>0 && m_nEndT<=(int)m_Tasks.GetNumTasks());
	ASSERT(m_nEndT>=m_nStartT);

	// Start at the end of the track
	// So get length of current track in sectors
	DWORD dwDelta = m_Tasks.GetTaskInfo( m_nEndT - 1  ).GetEndSector( ) - 
					m_Tasks.GetTaskInfo( m_nEndT - 1  ).GetStartSector( );

	// Calculate sectors to time
	m_nEndF=(int)(dwDelta % 75);
	m_nEndS=(int)((dwDelta / 75)%60);
	m_nEndM=(int)((dwDelta / 75)/60);

	// Recalculate sectors
	CalcSectors();

	// Update Controls
	UpdateData(FALSE);
}

