/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 Albert L. Faber
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
#include "config.h"
#include "FileName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TCHAR g_SepChar[3]={ _T( ' ' ), _T( '-' ),_T( '\0' )};

//                                        000000000001
//                                        123 4 567890
const CUString ILLEGAL_FNAME_CHARS(   _T( "<>:\"\\/|*? " ) );


CUString FormatTrackNameString( CUString strFormat, ENUM_FILENAME_CONVERTS nType ) 
{
	int i;

	switch ( nType )
	{
		case FILENAME_CONVERT_NONE:
			// do noting
		break;
		case FILENAME_CONVERT_LOWER:
			strFormat.MakeLower();
		break;

		case FILENAME_CONVERT_UPPER:
			strFormat.MakeUpper();
		break;

		case FILENAME_CONVERT_FC_UPPER:
			strFormat.MakeLower();
			if ( strFormat.GetLength() > 0 )
			{
				CUString strTmp( strFormat.Left( 1 ) );
				strTmp.MakeUpper();
				strFormat.SetAt( 0, strTmp.GetAt( 0 ) );
			}
		break;
		case FILENAME_CONVERT_FCEW_UPPER:
			strFormat.MakeLower();

			for ( i=0; i < strFormat.GetLength()-1; i++ )
			{
				if ( i == 0 )
				{
					CUString strTmp( strFormat.Mid( 0, 1 ) );
					strTmp.MakeUpper();
					strFormat.SetAt( 0, strTmp.GetAt( 0 ) );
				} else
				{
					if (  ( 0 == _istalnum( strFormat.GetAt( i ) )  ) &&
						  ( _T( '\'' ) != strFormat.GetAt( i ) ) )
					{
						CUString strTmp( strFormat.Mid( i+1, 1 ) );
						strTmp.MakeUpper();
						strFormat.SetAt( i+1, strTmp.GetAt( 0 ) );
					}
				}
			}
		break;
		case FILENAME_CONVERT_SWAP_ARTIST_TRACK:
			i = strFormat.Find( _W( " / " ) );

			if ( i > 0 )
			{
				CUString str1;
				CUString str2;

				str1 = strFormat.Left( i );
				str2 = strFormat.Mid( i + 3 );

				strFormat = str2 + _W( " / " ) + str1;
			}
		break;
		default:
			ASSERT( FALSE );
	}

	return strFormat;
}



/////////////////////////////////////////////////////////////////////////////
// CFNButton

CFNButton::CFNButton()
{
    m_clrText = RGB( 255,255,255 );
    m_clrBkgnd = RGB( 0, 0, 192 );
    m_brBkgnd.CreateSolidBrush( m_clrBkgnd );
	m_bActivated=FALSE;
}


CFNButton::~CFNButton()
{
}


BEGIN_MESSAGE_MAP(CFNButton, CButton)
	//{{AFX_MSG_MAP(CFNButton)
	ON_WM_CTLCOLOR_REFLECT()
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFNButton message handlers


HBRUSH CFNButton::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	if (m_bActivated)
	{
		pDC->SetTextColor( m_clrText );		// text
		pDC->SetBkColor( m_clrBkgnd );		// text bkgnd
		return m_brBkgnd;					// ctl bkgnd
	}
	else
		return NULL;						// ctl bkgnd
}

void CFNButton::OnClicked() 
{	
}

void CFNButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if (!m_bActivated)
	{
		// Get the focus to this button
		SetFocus();

		// Redraw button
		Invalidate();
		
		// Don't Set the button during button up
		m_bSet=FALSE;
	}
	else
	{
		CButton::OnLButtonDown(nFlags, point);

		// Set the button during button up
		m_bSet=TRUE;
	}

	// Notify Parent
//	GetParent()->SendMessage(WM_CTRLCHANGED,0,(LPARAM)this);
}


void BuildFileName(	CUString strFormat,
					CUString strArray[NUMFILENAME_IDS],
					CUString& strName,CUString& strDir)
{

	CUString strTmp( strFormat );
	int		nPos = 0;
	int		i;

	for ( i=0; i<NUMFILENAME_IDS; i++ )
	{
		strArray[i] = FixFileNameString( strArray[i] );
	}

	// loop through all the tokens
	while ( (( nPos = strTmp.Find( _T( '%' ),nPos ) ) >=0 ) && ( nPos + 1 < strTmp.GetLength() ) )
	{
		ENUM_FILENAME_CONVERTS	convertType = FILENAME_CONVERT_NONE;
		int						nPercentID = 0;
		int						nSkipChars = 2;

		WCHAR chEval = strTmp.GetAt( nPos + 1 );

		if ( _W( 'l' ) == chEval )
		{
			convertType = FILENAME_CONVERT_LOWER;
			chEval = strTmp.GetAt( nPos + 2 );
			nSkipChars++;
		}
		if ( _W( 'u' ) == chEval )
		{
			convertType = FILENAME_CONVERT_UPPER;
			chEval = strTmp.GetAt( nPos + 2 );
			nSkipChars++;
		}
		if ( _W( 'c' ) == chEval )
		{
			convertType = FILENAME_CONVERT_FCEW_UPPER;
			chEval = strTmp.GetAt( nPos + 2 );
			nSkipChars++;
		}
		if ( _W( 'f' ) == chEval )
		{
			convertType = FILENAME_CONVERT_FC_UPPER;
			chEval = strTmp.GetAt( nPos + 2 );
			nSkipChars++;
		}

		// Get %ID number
		switch ( chEval )
		{
			case '1': nPercentID =  0; break;
			case '2': nPercentID =  1; break;
			case '3': nPercentID =  2; break;
			case '4': nPercentID =  3; break;
			case '5': nPercentID =  4; break;
			case '6': nPercentID =  5; break;
			case '7': nPercentID =  6; break;
			case '8': nPercentID =  7; break;
			case 'Y': nPercentID =  8; break;
			case 'G': nPercentID =  9; break;
			case 'A': nPercentID = 10; break;

			default:
				nPercentID = -1;
				nPos++;
			break;
		}

	
		if ( nPercentID >= 0 && nPercentID < NUMFILENAME_IDS )
		{
			CUString strLeft( strTmp.Left( nPos ) );
			CUString strMid( strArray[ nPercentID ] );
			CUString strRight( strTmp.Right( strTmp.GetLength() - nPos - nSkipChars ) );

			// do filename mangling
			strMid = FormatTrackNameString( strMid, convertType );

			strTmp = strLeft + strMid + strRight;

		}
		else
		{
			// Delete token
//			strTmp = strTmp.Left( nPos ) + strTmp.Right( strTmp.GetLength() - nPos - 1 );
		}
	}

	// Split result in a directory and file name portion
	// Search for last backslash
	nPos = strTmp.ReverseFind( _T( '\\' ) );

	if ( nPos >= 2 )
	{
		// OK, we've found the backsplash, do the split
		strDir = strTmp.Left( nPos + 1 );
		strName = strTmp.Right( strTmp.GetLength() - nPos - 1 );
	}
	else
	{
		// Only a filename, just assign the string to the filename and clear the directory
		strDir = _T( "" );
		strName = strTmp;
	}

	strDir.TrimLeft();
	strDir.TrimRight();

	// remove spaces before the directory separator and double backslashes
	INT	nIndex = 0;
	while ( nIndex < strDir.GetLength() )
	{
		if ( _T( '\\' ) == strDir[ nIndex ] )
		{
			if ( nIndex > 0 )
			{
				if ( ( _T( '\\' ) == strDir[ nIndex - 1 ] ) || 
					 ( _T( ' ' ) == strDir[ nIndex - 1 ] ))
				{
					strDir = strDir.Left( nIndex ) + strDir.Mid( nIndex + 1 );
					nIndex = 0;
					continue;
				}
			}
		}
		if ( _T( ' ' ) == strDir[ nIndex ] )
		{
			if ( nIndex > 0 )
			{
				if ( ( _T( '\\' ) == strDir[ nIndex - 1 ] ) )
				{
					strDir = strDir.Left( nIndex ) + strDir.Mid( nIndex + 1 );
					nIndex = 0;
					continue;
				}
			}
		}

		nIndex++;

	}

	strDir.TrimLeft();
	strDir.TrimRight();

	strName.TrimLeft();
	strName.TrimRight();

}	



CUString FixFileNameString(const CUString& strIn)
{
	int nLoc=0;
	int nOldPos=-1;
	CUString strRILC=g_config.GetRILC();
	CUString strToTest(strIn);

	// Remove any trailing spaces
	strToTest.TrimLeft();

	// Remove any leading spaces
	strToTest.TrimRight();

	for ( nLoc=0; nLoc< ILLEGAL_FNAME_CHARS.GetLength() ; nLoc++)
	{
		int nPos=0;

		int nChar = 0x20;

		int nRes = swscanf(strRILC.Mid(nLoc*3,2), _W( "%2X" ), &nChar );

		if ( ( EOF == nRes ) || ( 0 == nRes ) )
		{
			nChar = 0x20;
		}

		while ( nPos < strToTest.GetLength() )
		{
			if ( strToTest.GetAt( nPos ) ==  ILLEGAL_FNAME_CHARS[  nLoc ] )
			{
				// nChar can be zero, if this is the case, we have to strip
				// the illegal character, and not replace it
				if ( nChar )
				{
					// replace character
					strToTest.SetAt( nPos, (char)(nChar) );

					// advance to next character
					nPos++;
				}
				else
				{
					// No replacement available, thus
					// delete the illegal character from the string
					strToTest= strToTest.Left(nPos) + strToTest.Right(strToTest.GetLength() - nPos - 1 );
				}
			}
			else
			{
				// advance to next character
				nPos++;
			}

		}

		nPos = strToTest.GetLength() - 1 ;

		while ( nPos >= 0 )
		{
			if ( '.' != strToTest.GetAt( nPos ) )
				break;

			strToTest = strToTest.Left( strToTest.GetLength() -1 );
			nPos--;
		}
	}
	return strToTest;
}

void CFNButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bSet)
	{
		CButton::OnLButtonUp(nFlags, point);
		// Notify Parent
	}
		GetParent()->SendMessage(WM_CTRLCHANGED,0,(LPARAM)this);
}

DWORD GetNumReplaceCharacters()
{
	return ILLEGAL_FNAME_CHARS.GetLength();
}
