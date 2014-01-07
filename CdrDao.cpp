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
#include "CdrDao.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define ForceCloseHandle(x) if (x) { CloseHandle(x); x = NULL; }


/////////////////////////////////////////////////////////////////////////////
// CCdrDao dialog


CCdrDao::CCdrDao(CWnd* pParent /*=NULL*/)
	: CDialog(CCdrDao::IDD, pParent)
{
	m_pChildProcess = NULL;

	m_hInReadPipe = NULL;
	m_hInWritePipe = NULL;
	m_hOutReadPipe = NULL;
	m_hOutWritePipe = NULL;
	m_hErrPipe = NULL;


	//{{AFX_DATA_INIT(CCdrDao)
	//}}AFX_DATA_INIT
}


void CCdrDao::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCdrDao)
	DDX_Control(pDX, IDC_OUTPUT, m_OutputWnd);
	DDX_Control(pDX, IDC_CDDRIVE, m_cdDrives);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCdrDao, CDialog)
	//{{AFX_MSG_MAP(CCdrDao)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCdrDao message handlers



// The steps for redirecting child process's STDOUT: 
//     1. Save current STDOUT, to be restored later. 
//     2. Create anonymous pipe to be STDOUT for child process. 
//     3. Set STDOUT of the parent process to be write handle to 
//        the pipe, so it is inherited by the child process. 
//     4. Create a noninheritable duplicate of the read handle and
//        close the inheritable read handle. 



BOOL CCdrDao::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	HANDLE			hOutReadTmp = NULL;
	HANDLE			hInWriteTmp = NULL;

	BOOL				rc ;
	SECURITY_ATTRIBUTES sa;

	// Setup security attributes
	sa.nLength = sizeof ( sa ) ;
	sa.lpSecurityDescriptor = NULL ;
	sa.bInheritHandle = FALSE ;


	// Create the child output pipe.

	if ( !( rc = CreatePipe( &hOutReadTmp, &m_hOutWritePipe, &sa, 0 ) ) )
	{
		CDexMessageBox( _W( "ERROR: Cannot create pipe to external encoder" ) );
	}

	if ( ! DuplicateHandle ( GetCurrentProcess () , m_hOutWritePipe, 
						   GetCurrentProcess () , &m_hErrPipe , 
						   0 , TRUE ,
						   DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) )
	{
		CDexMessageBox( _W( "ERROR: Cannot duplicate read pipe to cdrdao" ) );
	}


	if ( !( rc = CreatePipe( &m_hInReadPipe, &hInWriteTmp, &sa, 0)) )
	{
		CDexMessageBox( _W( "ERROR: Cannot create pipe to cdrdao" ) );
	}

	if ( ! DuplicateHandle ( GetCurrentProcess () , hOutReadTmp, 
						   GetCurrentProcess () , &m_hOutReadPipe, 
						   0 , FALSE , // not inhereteable
						   DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) )
	{
		CDexMessageBox( _W( "ERROR: Cannot duplicate read pipe to cdrdao" ) );
	}

	if ( ! DuplicateHandle ( GetCurrentProcess () , hInWriteTmp, 
						   GetCurrentProcess () , &m_hInWritePipe, 
						   0 , FALSE , // not inhereteable
						   DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS ) )
	{
		CDexMessageBox( _W( "ERROR: Cannot duplicate read pipe to cdrdao" ) );
	}

    CloseHandle( hOutReadTmp );
    CloseHandle( hInWriteTmp );

	// call cdrdao to capture drives
	m_pChildProcess = new CChildProcess( TRUE, _W( "d:\\cvs\\cdexos\\cdrdao\\cdrdao.exe scanbus" ), NORMAL_PRIORITY_CLASS, m_hInReadPipe, m_hOutWritePipe, m_hErrPipe, FALSE ) ;


	m_pChildProcess->StartProcess();

	::Sleep(1000);

	m_pChildProcess->WaitForFinish( 10L);

	BYTE myBuff[10000];
	DWORD dwBytesRead=0;

	rc = ReadFile( m_hOutReadPipe, myBuff, sizeof(myBuff), &dwBytesRead, NULL);

	myBuff[dwBytesRead] = '\0';
	m_OutputWnd.AddString( CUString( (LPSTR)myBuff, CP_UTF8 ) );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCdrDao::OnCancel() 
{
	ForceCloseHandle ( m_hInReadPipe ) ;
	ForceCloseHandle ( m_hInWritePipe ) ;
	ForceCloseHandle ( m_hOutReadPipe ) ;
	ForceCloseHandle ( m_hOutWritePipe ) ;
	ForceCloseHandle ( m_hErrPipe ) ;

	if ( m_pChildProcess ) 
	{
		while ( ! m_pChildProcess->WaitForFinish ( 100L ) ) 
		{
		} ;
		delete m_pChildProcess ;
	}

	m_pChildProcess= NULL;;

	CDialog::OnCancel();
}
