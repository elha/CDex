/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1998 Jorge Lodos (lodos@cigb.edu.cu)
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


// DIBStatic.cpp : implementation file
//


#include "stdafx.h"
#include "Dib.h"
#include "DIBStatic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDIBStatic

CDIBStatic::CDIBStatic()
{
}

CDIBStatic::~CDIBStatic()
{
}


BEGIN_MESSAGE_MAP(CDIBStatic, CStatic)
	//{{AFX_MSG_MAP(CDIBStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_QUERYNEWPALETTE()
	ON_WM_PALETTECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CDIBStatic::LoadDib(LPCTSTR lpszFileName)
{
	try
	{
		CFile file(lpszFileName, CFile::modeRead);
		return LoadDib(file);
	}
	catch (CFileException* e)
	{
		e->Delete();
		return FALSE;
	}
}

BOOL CDIBStatic::LoadDib(CFile& file)
{
	ASSERT_VALID(this);
    
	BOOL bResult = TRUE;
		
	if (!m_DIB.Read(file))
		bResult = FALSE;

	DoRealizePalette(FALSE);
	UpdateDib();
	return bResult;
}

BOOL CDIBStatic::LoadFromResource(LPCTSTR lpszResourceName)
{
	ASSERT_VALID(this);
    
	BOOL bResult = TRUE;
		
	if (!m_DIB.LoadFromResource(lpszResourceName))
		bResult = FALSE;

	DoRealizePalette(FALSE);
	UpdateDib();
	return bResult;

}

void CDIBStatic::ClearDib()
{
	ASSERT_VALID(this);
	
	CClientDC dc(this);
	CRect rectPaint;
    
	GetClientRect(&rectPaint);    
	rectPaint.InflateRect(-1,-1);
	    
	CBrush* pBrushWhite;
	pBrushWhite = CBrush::FromHandle((HBRUSH)::GetStockObject(LTGRAY_BRUSH));	//	WHITE_BRUSH
    
	dc.FillRect(&rectPaint, pBrushWhite);
}

void CDIBStatic::PaintDib(BOOL bDibValid)
{
	ASSERT_VALID(this);
	ClearDib();
		
	CRect PaintRect;
	GetClientRect(&PaintRect);    
	PaintRect.InflateRect(-1, -1);
	CClientDC dc(this);

	if (bDibValid)
	{
		int nDestX, nDestY, nDestWidth, nDestHeight;
		if (m_DIB.Width() < (DWORD)PaintRect.Width() && m_DIB.Height() < (DWORD)PaintRect.Height())
		{ // If the image fits, just center it
			nDestX = PaintRect.left + (PaintRect.Width() - m_DIB.Width())/2;
			nDestY = PaintRect.top + (PaintRect.Height() - m_DIB.Height())/2;
// AF, Width and Height are mixed up 
//			nDestWidth = m_DIB.Height();
//			nDestHeight = m_DIB.Width();
// this seems to be the correct code
			nDestHeight = m_DIB.Height();
			nDestWidth  = m_DIB.Width();
		}
		else
		{ // The bitmap doesn't fit, scale to fit 
			if ((PaintRect.Width()/(float)m_DIB.Width()) <= (PaintRect.Height()/(float)m_DIB.Height()))
			{ // Width is constraint
				nDestWidth = PaintRect.Width();
				nDestHeight = (nDestWidth*m_DIB.Height()) / m_DIB.Width();
				nDestX = PaintRect.left;
				nDestY = PaintRect.top + (PaintRect.Height() - nDestHeight) /2;
			}
			else
			{ // Height is constraint		
				nDestHeight = PaintRect.Height();
				nDestWidth = (nDestHeight*m_DIB.Width()) / m_DIB.Height();
				nDestX = PaintRect.left + (PaintRect.Width() - nDestWidth) /2;
				nDestY = PaintRect.top;
			}
		}

		CRect RectDest(nDestX, nDestY, nDestX+nDestWidth, nDestY+nDestHeight);
		CRect RectDib(0, 0, m_DIB.Width(), m_DIB.Height());
		m_DIB.Paint(dc, &RectDest, &RectDib);     
	}
/*
	Trozo eliminado por mi ya que no me gusta que pinten estas lineas so desgraciado.
	else
	{
		dc.MoveTo(PaintRect.TopLeft());
		dc.LineTo(PaintRect.BottomRight());
		dc.MoveTo(PaintRect.right, PaintRect.top);
		dc.LineTo(PaintRect.left, PaintRect.bottom);
	}
*/	
	return;
}

void CDIBStatic::UpdateDib()
{
	ASSERT_VALID(this);
	PaintDib(IsValidDib());
}

/////////////////////////////////////////////////////////////////////////////
// CDIBStatic message handlers

HBRUSH CDIBStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	UpdateDib();
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

BOOL CDIBStatic::OnQueryNewPalette() 
{
	return DoRealizePalette(FALSE);
}

void CDIBStatic::OnPaletteChanged(CWnd* pFocusWnd) 
{
	DoRealizePalette(TRUE);
}

BOOL CDIBStatic::DoRealizePalette(BOOL bForceBackGround)
{
	if (IsValidDib())
	{
		CClientDC dc(this);
		if (!m_DIB.m_pPalette)
			return FALSE;
		HPALETTE hPal = (HPALETTE)m_DIB.m_pPalette->m_hObject;
		HPALETTE hOldPalette = SelectPalette(dc, hPal, bForceBackGround);
		UINT nChanged = dc.RealizePalette();
		SelectPalette(dc, hOldPalette, TRUE);

		if (nChanged == 0)	// no change to our mapping
			return FALSE;
		
		// some changes have been made; invalidate
		UpdateDib();
	}

	return TRUE;
}
