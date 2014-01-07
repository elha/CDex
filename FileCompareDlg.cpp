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
#include "FileCompareDlg.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FileCompareDlg dialog


FileCompareDlg::FileCompareDlg(CWnd* pParent /*=NULL*/)
	: CDialog(FileCompareDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(FileCompareDlg)
	m_strResult = _T("");
	m_strFileNameA = _T("");
	m_strFileNameB = _T("");
	m_strOffset = _T("");
	m_strFileSize = _T("");
	//}}AFX_DATA_INIT
}


void FileCompareDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(FileCompareDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Text(pDX, IDC_CHARDIFFERENT, m_strResult);
	DDX_Text(pDX, IDC_FILENAMEA, m_strFileNameA);
	DDX_Text(pDX, IDC_FILENAMEB, m_strFileNameB);
	DDX_Text(pDX, IDC_OFFSET, m_strOffset);
	DDX_Text(pDX, IDC_FILESIZE, m_strFileSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(FileCompareDlg, CDialog)
	//{{AFX_MSG_MAP(FileCompareDlg)
	ON_BN_CLICKED(IDC_SELFILEA, OnSelfilea)
	ON_BN_CLICKED(IDC_SELFILEB, OnSelfileb)
#ifdef _DEBUG_TEST
	ON_BN_CLICKED(IDC_COMPARE, OnCompareCRC)
#else
	ON_BN_CLICKED(IDC_COMPARE, OnCompare)
#endif

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FileCompareDlg message handlers

void FileCompareDlg::OnSelfilea() 
{
	TCHAR BASED_CODE szFilter[] = _T( "WAV files (*.wav)|*.wav|All Files (*.*)|*.*||" );
    
    CUStringConvert strCnv;
    CUString strFileName( g_config.GetCnvOutputDir() + _W( "*.wav" ) );

    CFileDialog dlg( TRUE, NULL, strCnv.ToT( strFileName ), OFN_HIDEREADONLY, szFilter, NULL);
	
	m_strFileNameA = _T( "" );
	
	if (dlg.DoModal() == IDOK)
	{
		m_strFileNameA = dlg.GetPathName();;	
	}
	UpdateData(FALSE);
}

void FileCompareDlg::OnSelfileb() 
{
	TCHAR BASED_CODE szFilter[] = _T( "WAV files (*.wav)|*.wav|All Files (*.*)|*.*||" );

    CUStringConvert strCnv;
    CUString strFileName( g_config.GetCnvOutputDir() + _W( "*.wav" ) );

	if ( !m_strFileNameA.IsEmpty())
	{
		strFileName= CUString( m_strFileNameA );
	}

    CFileDialog dlg(TRUE, NULL, strCnv.ToT( strFileName ), OFN_HIDEREADONLY, szFilter, NULL);
	
	m_strFileNameB = _T( "" );
	
	if (dlg.DoModal() == IDOK)
	{
		m_strFileNameB = dlg.GetPathName();;	
	}
	UpdateData(FALSE);
}

void FileCompareDlg::OnCompareCRC() 
{
////    FILE* file = CDexOpenFile( _W( "D:\\Albert\\EAC\\Track01.wav")
//  //      /* m_strFileNameA */ , _T( "rb" ) );
//
//    
//    fseek( file, 0, SEEK_END );
//    long fileSize = ftell( file );
//    fseek( file, 0, SEEK_SET );
//
//    BYTE* buffer = new BYTE[ 1024 ];
//
//    fread( buffer, 44, 1, file );
//
//    ULONG crcValue = Calculate_CRC_Start( );
//    
//    DWORD bytesRemaining = fileSize - 44;
//
//    while ( bytesRemaining > 0 )
//    {
//        DWORD readBytes = min( bytesRemaining, 1024 );
//
//        fread( buffer, readBytes , 1, file );
//        crcValue = Calculate_CRC_Block( crcValue, buffer, readBytes, TRUE );
//        bytesRemaining -= readBytes;
//    }
//
//    crcValue = Calculate_CRC_End( crcValue );
//    
//    fclose( file );
//
//    delete [] buffer;
}

void FileCompareDlg::OnCompare() 
{
	CUString		strLang;
    CUStringConvert strCnv;
	const DWORD dwBufferSize = 1024*1024;
	const DWORD dwHeaderSize = 50;
	const DWORD dwCompareSize = 1024;
	CDEX_ERR	bError = CDEX_OK;
	
	BYTE*		pBufferA = NULL;
	BYTE*		pBufferB = NULL;
	FILE*		pFileA = NULL;
	FILE*		pFileB = NULL;
	DWORD		dwOffset = 0;
	DWORD		dwBytesToRead = 0;
	DWORD		dwBytesToDo = 0;
	DWORD		dwDeltaPos = 0;
	BOOL		bMatch = FALSE;
	DWORD		dwFileSizeA = 0;
	DWORD		dwFileSizeB = 0;
	
	// Get the correct control values
	UpdateData(TRUE);
	
	// Clear result
	m_strOffset = "";
	m_strResult = "";
	
	// Update Controls
	UpdateData(FALSE);
	
	// Check if first file name field is filled out
	if (m_strFileNameA.IsEmpty())
	{
		strLang = g_language.GetString( IDS_FILECOMP_FIRSTFILENAMENOTVALID );
		CDexMessageBox( strLang );
		bError = CDEX_ERROR;
	}
	
	// Check if second file name field is filled out
	if ( m_strFileNameB.IsEmpty()  && ( CDEX_OK == bError ) )
	{
		strLang = g_language.GetString( IDS_FILECOMP_SECONDFILENAMENOTVALID );
		CDexMessageBox( strLang );
		bError = CDEX_ERROR;
	}
	
	if ( CDEX_OK == bError )
	{
		// Try to open the first file
		pFileA = CDexOpenFile( CUString( m_strFileNameA ), _W( "rb" ) );
	
		// Check if file could be opened correctly
		if ( NULL == pFileA )
		{
            CUStringConvert strCnv;

			strLang = g_language.GetString( IDS_ERROR_COULDNOTOPENFILE );
			m_strResult.Format( strCnv.ToT( strLang ) , m_strFileNameA );
			bError = CDEX_ERROR;
		}
	}
	
	if ( CDEX_OK == bError )
	{
		// Try to open the second file
		pFileB = CDexOpenFile( CUString( m_strFileNameB ), _W( "rb" ) );
		
		// Check if file could be opened correctly
		if ( NULL == pFileB )
		{
			strLang = g_language.GetString( IDS_ERROR_COULDNOTOPENFILE );

            
			m_strResult.Format( strCnv.ToT( strLang ), CUString( m_strFileNameB ) );

			bError = CDEX_ERROR;
		}
	}
	
	if ( CDEX_OK == bError )
	{
		// Seek to the end of bot file, in order to determine the file size
		fseek( pFileA, 0, SEEK_END );
		fseek( pFileB, 0, SEEK_END );
		
		// Determine the file size of both files
		dwFileSizeA = (DWORD)ftell( pFileA );
		dwFileSizeB = (DWORD)ftell( pFileB );
		
		// The number of bytes to compare equal to the minimum 
		// of the two files, minus the WAV header size
		dwBytesToDo = min(dwFileSizeA, dwFileSizeB) - dwHeaderSize;
		
		// Check if file sizes are big enough to do a comparison
		if ( dwBytesToDo < dwCompareSize )
		{
			// File size is too small, set result string 
			m_strResult = g_language.GetString( IDS_ERROR_FILES2SMALL2COMPARE );

			bError = CDEX_ERROR;
		}
	}
	
	if ( CDEX_OK == bError )
	{
		// Seek back to the beginning of both files, but skip the WAV header
		fseek(pFileA, dwHeaderSize, SEEK_SET);
		fseek(pFileB, dwHeaderSize, SEEK_SET);
		
		// Allocate space to hold the data that is gonna be read
		pBufferA = new BYTE[dwBufferSize];
		pBufferB = new BYTE[dwBufferSize];
	
		// Check if buffers are allocated properly
		if (pBufferA == NULL || pBufferB == NULL)
		{
			// Not enough memory I guess
			m_strResult = _T( "Could not allocate buffer space" );

			bError = CDEX_ERROR;
		}
	}

	if ( CDEX_OK == bError )
	{
		// Find offset in first block
		dwBytesToRead = min( dwBytesToDo, dwBufferSize );
		
		// Read the data into both buffers
		fread( pBufferA, dwBytesToRead, 1, pFileA );
		fread( pBufferB, dwBytesToRead, 1, pFileB );
		
		// Loop through the first block to determine the offset
		while (!bMatch &&(dwOffset < (dwBytesToRead - dwCompareSize)))
		{
            BOOL hasData = TRUE;

            //for ( DWORD index = 0; index < dwCompareSize; index++ )
            //{
            //    if ( ( pBufferA[ index + dwOffset ] != 0 )  && ( pBufferB[ index + dwOffset] != 0 ) )
            //    {
            //        hasData = TRUE;
            //        break;
            //    }
            //}

            if ( hasData )
            {
			    // First case, assume that fileB has an offset compare to fileA
			    if (memcmp(pBufferA, pBufferB + dwOffset, dwCompareSize) == 0)
			    {
				    strLang = g_language.GetString( IDS_FILECOMP_SECFILEOFFSET );

				    // That seems to be the right assumption
				    m_strOffset.Format( strCnv.ToT( strLang ), dwOffset );

				    bMatch = TRUE;
			    }
			    else
			    {
				    // Try next offset value
				    dwOffset++;
			    }
            }
		}
		
		// FileB has a positive offset compare to fileA
		if (bMatch)
		{
			// Rewind FileA pointer just after the WAV header
			fseek(pFileA, dwHeaderSize, SEEK_SET);

			// Rewind FileB pointer just after the WAV header + the offset
			fseek(pFileB, dwHeaderSize + dwOffset, SEEK_SET);
		}
		else
		{
			// Start again at the beginning
			dwOffset = 0;
			
			while (!bMatch &&(dwOffset < (dwBytesToRead - dwCompareSize)))
			{
				// Check if FileA has a positive offset compare to file B
				if (memcmp(pBufferA + dwOffset, pBufferB, dwCompareSize) == 0)
				{
					strLang = g_language.GetString( IDS_FILECOMP_FIRSTFILEOFFSET );

					// That seems to be the right assumption
					m_strOffset.Format( strCnv.ToT( strLang ), dwOffset );

					bMatch = TRUE;
				}
				else
				{
					// Try next offset value
					dwOffset++;
				}
			}
			
			// FileA has a positive offset compare to fileB
			if ( bMatch )
			{
				// Rewind FileA pointer just after the WAV header + the offset
				fseek(pFileA, dwHeaderSize + dwOffset, SEEK_SET);

				// Rewind FileB pointer just after the WAV header
				fseek(pFileB, dwHeaderSize, SEEK_SET);
			}
			else
			{

				// We are in trouble, offset is larger than block size
				m_strResult = g_language.GetString( IDS_FILECOMP_NOMATCHINGBLOCK );

				bError = CDEX_ERROR;
			}
		}
	}

	strLang = g_language.GetString( IDS_FILESIZE_FIRSTANDSECOND );

	// Update user info, inform about the file sizes
	m_strFileSize.Format( strCnv.ToT( strLang ), dwFileSizeA, dwFileSizeB );
	
	// Update controls with new info
	UpdateData( FALSE );
	
	// Reduce the comparison with the number of offset bytes, just too be sure that we are
	// not running out of the buffers
	dwBytesToDo -= dwOffset;
	
	// Re-use dwFileSizeA to hold the total number of bytes to compare
	dwFileSizeA = dwBytesToDo;
	
	// Start all over again, loop trough all the data
	while (dwBytesToDo>0)
	{
		// Determine how many byte to read
		dwBytesToRead = min(dwBytesToDo, dwBytesToRead);
		
		// Read the data from FileA into BufferA
		fread(pBufferA, dwBytesToRead, 1, pFileA);
		
		// Read the data from FileB into BufferB
		fread(pBufferB, dwBytesToRead, 1, pFileB);
		
		// Can we set the progress dialog box
		if (dwFileSizeA>0)
		{
			// Calculate percent completed
			DOUBLE dPercent = (DOUBLE)(dwFileSizeA - dwBytesToDo)/(DOUBLE)dwFileSizeA*100.0;
			// Set percent completed to progress bar
			m_Progress.SetPos((INT)dPercent);
		}
		
		// Do the memory comparison
		if (memcmp(pBufferA, pBufferB, dwBytesToRead) != 0)
		{
			// Find out where the difference did occur
			for (dwDeltaPos = 0; dwDeltaPos < dwBytesToRead; dwDeltaPos++)
			{
				// Check if this is the location where the difference is
				if (pBufferA[dwDeltaPos] != pBufferB[dwDeltaPos])
				{
					// Found it, break out of for loop
					break;
				}
			}
			
			// Determined that files are not equal
			bMatch = FALSE;
			
			// Break out of while loop, since further comparison does not make sense
			break;
		}
		
		// Decrease the number of bytes to compare, with the number we just compared
		dwBytesToDo -= dwBytesToRead;
	}
	
	// Check if there was a file difference
	if ( !bMatch )
	{
		strLang = g_language.GetString( IDS_FILECOMP_FILESAREDIFFERENT );

		// Yes there is. set the appropriate result string
		m_strResult.Format( strCnv.ToT( strLang ), dwDeltaPos );
	}
	else
	{
		strLang = g_language.GetString( IDS_FILECOMP_FILESARETHESAME );

		// Files do seem to match, set result that files are equal
		m_strResult.Format( strCnv.ToT( strLang ), dwFileSizeA );
	}
	
	// Update controls
	UpdateData( FALSE );
	
	// Delete the buffers
	delete[] pBufferA;
	delete[] pBufferB;
	
	// Close the files
	if ( NULL != pFileA )
	{
		fclose(pFileA);
	}

	if ( NULL != pFileB )
	{
		fclose( pFileB );
	}
}

BOOL FileCompareDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// translate dialog resources
	g_language.InitDialogStrings( this, IDD );

	// Set the range of the progress bar
	m_Progress.SetRange(0, 100);
	
	// Set the position of the progress bar to zero
	m_Progress.SetPos(0);
	
	return TRUE;
}

