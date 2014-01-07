/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1998 Ferdinand V. Abne (fabne@bigfoot.com)
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
#include "MFECToolTip.h"
#include "Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMFECToolTip

CMFECToolTip::CMFECToolTip()
{
	m_nHeight				= 0;
	m_nWidth				= 0;
	m_nFontSize				= 14;		// original size
	m_currentControlID		= 0;
	m_pParentWnd			= NULL;

	m_aControlInfo.RemoveAll();
}

CMFECToolTip::~CMFECToolTip()
{
	ToolTipInfo *pInfo = NULL;
	int nSize = m_aControlInfo.GetSize();
	for( int i = 0; i < nSize; i++ )
	{
		pInfo = (ToolTipInfo*)m_aControlInfo.GetAt( i );
		delete pInfo;
	}

	m_aControlInfo.RemoveAll();
}

void CMFECToolTip::Create(CWnd* pWnd)
{
	m_pParentWnd = pWnd;
}

void CMFECToolTip::ErasePreviousToolTipDisplay( UINT nControlID )
{
	//This assertion fails because you did not call Create() first.
	//m_pParentWnd was not initialized.
	ASSERT(m_pParentWnd);

	//if erase already, do not do it again
	if((m_nHeight == 0) || (m_nWidth == 0))
		return;
	
	CRect		oInfoRect(0,0,0,0);

	CalculateInfoBoxRect(nControlID, &oInfoRect);
	m_pParentWnd->InvalidateRect(&oInfoRect);
	m_pParentWnd->UpdateWindow();

	m_nHeight = 0;
	m_nWidth = 0;
}


void CMFECToolTip::CalculateHeightAndWidth(CStringArray& straInfos)
{
	//This assertion fails because you did not call Create() first.
	//m_pParentWnd was not initialized.
	ASSERT(m_pParentWnd);

	int nMaxLength = 0;
	int nLength;
	int nLine = straInfos.GetSize();
	m_nTotalLine = nLine;		// holder for maximum line of information
	for(int i=0; i<nLine; i++)
	{
		nLength = (straInfos[i]).GetLength();
		if(nLength > nMaxLength)
			nMaxLength = nLength;
	}

	m_maxCharInLine = nMaxLength;		// holder for longest char info
	m_nHeight = 12 + nLine * (m_nFontSize - 1);	//m_nFontSize is the font size
	m_nWidth = 10 + (int)(7.5 * nMaxLength);	//aproximately 5 pixel per char

	const int nTabStops = 10;
	const int nTabSize  = 40;

	for (int i=0; i < sizeof( m_aTabStobs ) / sizeof( m_aTabStobs[0] ); i++ )
	{
		m_aTabStobs[ i ] = nTabSize * ( i + 1 );
	}

}


void CMFECToolTip::CalculateInfoBoxRect(UINT nControlID, CRect* pInfoRect)
{
	const int nVerticalOffset = 30;

	ASSERT(m_pParentWnd);
	
	CRect		oRect(0,0,0,0);
	CRect		oParentWindowRect(0,0,0,0);
	
	m_pParentWnd->GetWindowRect( &oParentWindowRect );
	m_pParentWnd->ScreenToClient( &oParentWindowRect );
	
	CWnd*		pWnd = m_pParentWnd->GetDlgItem( nControlID );

	ASSERT( pWnd );

	pWnd->GetWindowRect( &oRect );
	m_pParentWnd->ScreenToClient( &oRect );
	
	//check if the box fit in the parent dialog 
	SetFontSize( m_nFontSize );

	int nBottom = oRect.bottom + nVerticalOffset + m_nHeight;
	int nTop    = oRect.top - ( nVerticalOffset );

	// place below
	if( nBottom <= oParentWindowRect.bottom )
	{
		pInfoRect->top = oRect.bottom + nVerticalOffset;
		pInfoRect->bottom = pInfoRect->top + m_nHeight;
	}
	// place above
	else if( nTop >= oParentWindowRect.top )
	{
		pInfoRect->top = oRect.top - nVerticalOffset - m_nHeight;
		pInfoRect->bottom = pInfoRect->top + m_nHeight;
	}
	else
	{
		// change the size of the font as well as the info box if all
		// info data cannot be viewed
		if( m_nHeight > (oParentWindowRect.bottom - oParentWindowRect.top) )
		{
			SetFontSize( 12 );
			m_nHeight = 12 + m_nTotalLine * (m_nFontSize - 1);	//m_nFontSize is the font size
			m_nWidth = 10 + (int)(7 * m_maxCharInLine);	
		}
		// end
		pInfoRect->bottom = oParentWindowRect.bottom - 1;
		pInfoRect->top = pInfoRect->bottom - m_nHeight;
	}

	//check if the box fit in the parent dialog
	int nRight = (oRect.left + oRect.right)/2 + m_nWidth;
	if(nRight <= oParentWindowRect.right)
	{
		pInfoRect->left = (oRect.left + oRect.right)/2;
		pInfoRect->right = pInfoRect->left + m_nWidth;
	}
	else
	{
		int nLeft = (oRect.left + oRect.right)/2 - m_nWidth;
		if(nLeft <= oParentWindowRect.left)
		{
			pInfoRect->left = oParentWindowRect.left + 1;
			pInfoRect->right = pInfoRect->left + m_nWidth;
		}
		else
		{
			pInfoRect->right = (oRect.left + oRect.right)/2;
			pInfoRect->left = pInfoRect->right - m_nWidth;
		}
	}

	ASSERT(pInfoRect->top <= pInfoRect->bottom);
	ASSERT(pInfoRect->left <= pInfoRect->right);
}


void CMFECToolTip::ShowToolTip( UINT nControlID )
{						
	ToolTipInfo *pToolTip = IsControlIDExisting( nControlID );
	if( pToolTip == NULL )
		return;

	DisplayInfo( pToolTip );
}


void CMFECToolTip::ShowToolTip( CPoint& point )
{						
	CWnd*		  pWnd = m_pParentWnd->ChildWindowFromPoint(point);
	if( pWnd )
	{
		UINT	nControlID = (UINT)pWnd->GetDlgCtrlID();

		if( m_currentControlID != nControlID )
		{
			ErasePreviousToolTipDisplay( m_currentControlID );
			ShowToolTip( nControlID );
		}
	}
}

void CMFECToolTip::DisplayInfo( ToolTipInfo* pToolTip )
{
	if( pToolTip->nInfoSize <= 0 )
		return;

	ASSERT(m_pParentWnd);

	CDC* pDC = m_pParentWnd->GetDC();

	CRect		oInfoRect;
	CBrush		oBrush, *pOldBrush, oBorderBrush;
	int			nX, nY;
	TEXTMETRIC	TM;
	int			nTextHigh;
	CFont		oFont, *pOldFont;
	CWnd*		pWnd = NULL;

	pDC->SetBkMode(TRANSPARENT);
	
	oBrush.CreateSolidBrush( pToolTip->nBackColor );

	pOldBrush = pDC->SelectObject( &oBrush );
	pDC->SetTextColor( pToolTip->nTextColor );

	//calculate the width and height of the box dynamically
	CalculateHeightAndWidth( pToolTip->nControlInfo );
	CalculateInfoBoxRect( pToolTip->nControlID, &oInfoRect );

	oFont.CreateFont( m_nFontSize,			// nHeight
		              0,					// nWidth
					  0,					// nEscapement
					  0,					// nOrientation
					  FW_REGULAR,			// nWeight
					  0,					// bItalic
					  0,					// bUnderline
					  0,					// cStrikeOut
					  DEFAULT_CHARSET,		// nCharSet
					  0,					// nOutPrecision
					  0,					// nClipPrecision
					  0,					// nQuality
					  0,					// nPitchAndFamily
					  _T( "MS Shell Dlg" )	// lpszFacename 
					  );

	pOldFont = pDC->SelectObject(&oFont);
	
	pDC->FillRect(&oInfoRect, &oBrush);
	pDC->SelectObject(pOldBrush);
	oBrush.DeleteObject();

	oBorderBrush.CreateSolidBrush( pToolTip->nTextColor );
	pOldBrush = pDC->SelectObject(&oBorderBrush);
	DrawEdge(pDC->m_hDC, &oInfoRect, BDR_RAISEDINNER | BDR_SUNKENOUTER, BF_RECT);
	
	pDC->SetTextAlign(TA_LEFT);
	pDC->GetTextMetrics(&TM);
	nTextHigh = TM.tmHeight + TM.tmExternalLeading - 2;
	nX = oInfoRect.left + 8;
	nY = oInfoRect.top + 5; 

	for( register UINT i = 0; i < pToolTip->nInfoSize; i++)
	{
CUString strOut( pToolTip->nControlInfo[ i ] );
//		pDC->TextOut(nX, nY, pToolTip->nControlInfo[i]);
		pDC->TabbedTextOut( nX, 
							nY,
							pToolTip->nControlInfo[ i ],
							sizeof( m_aTabStobs ) / sizeof( m_aTabStobs[0] ),
							m_aTabStobs,
							nX );

		nY += m_nFontSize - 1;
		
	}

	pDC->SelectObject(pOldBrush);
	oBorderBrush.DeleteObject();	
	m_pParentWnd->ReleaseDC(pDC);
}	


BOOL CMFECToolTip::AddControlInfo( UINT contolID, CStringArray& straInfo, COLORREF back, COLORREF text )
{
	ToolTipInfo *pToolTip = new ToolTipInfo;
	ASSERT( pToolTip != NULL );

	int nSize = straInfo.GetSize();
	if( pToolTip <= 0 )	// no information, don't add to the list and return FALSE
	{
		delete pToolTip;
		return FALSE;
	}

	pToolTip->nControlInfo.RemoveAll();
	pToolTip->nInfoSize  = nSize;
	pToolTip->nControlID = contolID;

	for( int i = 0; i < nSize; i++ )
	{
		pToolTip->nControlInfo.Add( straInfo[ i ] );
	}
	// initialize colors, use default if not supplied
	pToolTip->nBackColor = back;
	pToolTip->nTextColor = text;

	// add to the list
	m_aControlInfo.Add( pToolTip );

	return TRUE;
}

BOOL CMFECToolTip::AddControlInfo( UINT contolID, UINT infoID, COLORREF back, COLORREF text )
{
	CUString strLang;
	CStringArray strArray;
	int nStart = 0;
	int nStrPos = 0;


	strLang = g_language.GetString( infoID );

    CUStringConvert strCnv;

	// Split language string into separate strings
	for ( nStrPos = 0; nStrPos < strLang.GetLength(); nStrPos++ )
	{
		if ( strLang[ nStrPos ] == '\n' )
		{

            strArray.Add( strCnv.ToT( strLang.Mid( nStart, nStrPos - nStart ) ) );
			nStart = nStrPos + 1;
		}
	}


	strArray.Add( strCnv.ToT( strLang.Mid( nStart, strLang.GetLength() - nStart ) ) );

	return AddControlInfo( contolID, strArray, back, text );
}



ToolTipInfo* CMFECToolTip::IsControlIDExisting( UINT controlID )
{
	ToolTipInfo *pToolTip = NULL;
	int nSize = m_aControlInfo.GetSize();
	for( register int i = 0; i < nSize; i++ )
	{
		pToolTip = (ToolTipInfo*)m_aControlInfo.GetAt( i );
		if( pToolTip->nControlID == controlID )		// if found!
		{
			m_currentControlID = controlID;
			return pToolTip;
		}
	}
	m_currentControlID = 0;		// if not found, reset the current control ID to refresh the display

	return NULL;
}

BOOL CMFECToolTip::RemoveControlInfo( UINT controlID )
{
	ToolTipInfo *pToolTip = NULL;
	int nSize = m_aControlInfo.GetSize();
	for( register int i = 0; i < nSize; i++ )
	{
		pToolTip = (ToolTipInfo*)m_aControlInfo.GetAt( i );
		if( pToolTip->nControlID == controlID )		// if found!
		{				
			if( m_currentControlID == controlID )
				ErasePreviousToolTipDisplay( m_currentControlID );

			m_aControlInfo.RemoveAt( i );	// remove the entry
			delete pToolTip;
			break;		// break from the loop
		}
	}

	return TRUE;
}

