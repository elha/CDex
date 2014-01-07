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
#include "Mp3toRiffWavDlg.h"
#include "AudioFile.h"
#include "MPEGHeader.h"
#include "Config.h"
#include "ID3Tag.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



#define TIMERID 2
#define TIMERSLOT 100

BOOL CMp3toRiffWavDlg::ReadMP3Header(CUString& strFileName,int& nChannels, DWORD &dwSample, DWORD& dwBitRate,int& nLayer)
{
	BOOL	bReturn=FALSE;
	int		nExtLayer = 3;
	FILE*	fp=NULL;
	DWORD	dwID3TagOffset=0;
	
	CUString	strInFileName = strFileName + _W( ".mp3" );

	fp = CDexOpenFile( strInFileName, _W( "rb" ) );

	if ( NULL == fp )
	{
		nExtLayer = 2;
		strInFileName = strFileName + _W( ".mp2" );
	}
	else
	{
		nExtLayer = 3;
		fclose( fp );
	}

	// Check presense of ID3V2 tag
	// Get MP3 ID3V2 Tag offset
	CID3Tag	ID3Tag;

	ID3Tag.Init();
	ID3Tag.OpenFile( strInFileName );
	ID3Tag.LoadTagProperties();


	if ( ID3Tag.IsV2Tag() )  
	{
		dwID3TagOffset = ID3Tag.GetTagSize();
	}

	fp = CDexOpenFile( strInFileName, _W( "rb" ) );

	// check presense of file name
	if ( NULL == fp )
	{
		return FALSE;
	}


	// skip ID3V2 tag
	fseek( fp, dwID3TagOffset, SEEK_SET );


	MP3FRAMEHEADER myHeader;

	// Clear the stuff
	memset(&myHeader,0x00,sizeof(myHeader));

	BYTE bTmp[4];

	// Read header
	if ( 1 == fread(bTmp,4,1,fp ) )
	{
		memcpy(&myHeader,bTmp,4);

		int nData;

		memcpy(&nData,bTmp,4);
		fclose(fp);

		// Fudge the Layer setting, to get the real layer value
		myHeader.Layer=4-myHeader.Layer;

		if (nExtLayer!=myHeader.Layer)
		{
			CUString strTmp;
			strTmp.Format( _W( "Extention of file %s \r\ndoes not match MPEG Layer Tag, File conversion has been aborted" ), (LPCWSTR)strFileName );
            CUStringConvert strCnv;
            CDexMessageBox( strTmp );
			return FALSE;
		}

		// Okay, got the header
		if (myHeader.FrameSyncH!=0xff || myHeader.FrameSyncL<0xe) 
		{
			CUString strTmp;
			strTmp.Format( _W( "Could not find MPEG sync header\r\nConversion of file %s has been aborted" ), (LPCWSTR)strFileName );
            CUStringConvert strCnv;
            CDexMessageBox( strTmp );
			return FALSE;
		}

		// Get proper bitrate table based on version
		dwBitRate=MPEGBitrates[(myHeader.Version==TYPE_MPEG_I)?0:1][myHeader.Layer-1][myHeader.BitRateIndex];

		// Get proper samplrerate table based on version
		dwSample=MPEGSamplerates[(myHeader.Version==TYPE_MPEG_I)?0:1][myHeader.SampleRateIndex];
	
		// Adjust sample rate for lower bitrates
		if (myHeader.FrameSyncL==0xe)
			dwSample/=2;

		nLayer=myHeader.Layer;

		nChannels=(myHeader.ChannelMode==3)?1:2;


		return TRUE;
	}

	// close the file
	fclose(fp);

	return bReturn;
}

// CONSTRUCTOR
CMp3toRiffWavDlg::CMp3toRiffWavDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMp3toRiffWavDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMp3toRiffWavDlg)
	//}}AFX_DATA_INIT
	m_pFileDlg=NULL;
}


// DESTRUCTOR
CMp3toRiffWavDlg::~CMp3toRiffWavDlg()
{
	delete m_pFileDlg;
}

void CMp3toRiffWavDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CMp3toRiffWavDlg)
    DDX_Control(pDX, IDC_PROGRESS, m_Progress);
    //}}AFX_DATA_MAP
    DDX_Control(pDX, IDC_PROCESSFILENAME, m_processFileName);
    DDX_Control(pDX, IDC_STATIC_MPEGTORIFFWAV_PROCESSING, m_processingText);
}


BEGIN_MESSAGE_MAP(CMp3toRiffWavDlg, CDialog)
	//{{AFX_MSG_MAP(CMp3toRiffWavDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMp3toRiffWavDlg message handlers

void CMp3toRiffWavDlg::OnCancel() 
{
	MessageBeep( MB_OK );
	MessageBeep( 0xFFFFFFFF );


	KillTimer( TIMERID );
	
	CDialog::OnCancel();
}

void CMp3toRiffWavDlg::OnOK() 
{
	MessageBeep( MB_OK );
	MessageBeep( 0xFFFFFFFF );

	KillTimer( TIMERID );
	
	CDialog::OnOK();
}

void CMp3toRiffWavDlg::OnTimer(UINT nIDEvent) 
{
	int nLayer;

	// Obtain the number of files
	if (m_Pos!=NULL)
	{
		// Get the file name to be converted
		CUString strFileName=m_pFileDlg->GetNextPathName( m_Pos );
		CUString strOrig( strFileName );

		// Update progress bar
		m_Progress.SetPos( m_nCurFile++ );

		// Set current file name
        CUStringConvert strCnv;

        m_processFileName.SetWindowText( strCnv.ToT( strFileName ) );

        // Update controls
		UpdateData(FALSE);

		
		// Open the WAV file first to get the parameters
		CWAV myWav;

		int nWaveChannels=0;
		DWORD nWaveSampleRate=0;
		DWORD nMP3BitRate=0;


		// Strip extension
		int nPos=strFileName.ReverseFind( _W( '.' ) );

		// Stip it
		if (nPos>0)
			strFileName=strFileName.Left(nPos);

		if (ReadMP3Header(strFileName,nWaveChannels,nWaveSampleRate,nMP3BitRate,nLayer))
		{
			// Do the conversion
			// Do the conversion
			if (myWav.ConvertToRiffWav(strFileName,nMP3BitRate,nWaveSampleRate,nWaveChannels,nLayer)==CDEX_OK)
			{
				// delete the original file?
				if ( m_pFileDlg->m_bDeleteOriginal )
                {
					CDexDeleteFile(strOrig);
                }
			}

		}
		else
		{
			CDexMessageBox( _W( "Invalid MP3 file" ) );
		}
	}
	else
	{
		OnCancel();
	}

	CDialog::OnTimer(nIDEvent);

}

BOOL CMp3toRiffWavDlg::OnInitDialog() 
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

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
