/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2002 Albert L. Faber
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
#include "TransBmp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CTransBmp::CTransBmp()
{
	
}

CTransBmp::~CTransBmp()
{
	
}

void CTransBmp::CreateMask(CDC *TargetDC,CTransBmp *Sprite, COLORREF TranspColor)
{
		
	CDC imageDC;
	imageDC.CreateCompatibleDC(TargetDC);
	CBitmap *pOldImageDC = (CBitmap *) imageDC.SelectObject(&(Sprite->m_bmPic));
	
	CDC maskDC;
	maskDC.CreateCompatibleDC(TargetDC);
	Sprite->m_bmMask.CreateBitmap(Sprite->m_iDimX,Sprite->m_iDimY,1,1,NULL);
	CBitmap *pOldMaskDC = (CBitmap *) maskDC.SelectObject(&(Sprite->m_bmMask));

	imageDC.SetBkColor(TranspColor);
	maskDC.BitBlt(0,0,Sprite->m_iDimX,Sprite->m_iDimY,&imageDC,0,0,SRCCOPY);

	CDC orDC;
	orDC.CreateCompatibleDC(TargetDC);
	Sprite->m_bmOr.CreateCompatibleBitmap(TargetDC,Sprite->m_iDimX,Sprite->m_iDimY);
	CBitmap *pOldOrDC = (CBitmap *) orDC.SelectObject(&(Sprite->m_bmOr));

	orDC.BitBlt(0,0,Sprite->m_iDimX,Sprite->m_iDimY,&imageDC,0,0,SRCCOPY);
	orDC.BitBlt(0,0,Sprite->m_iDimX,Sprite->m_iDimY,&maskDC,0,0,0x220326);

	// Clean up
	orDC.SelectObject(pOldOrDC);
	maskDC.SelectObject(pOldMaskDC);
	imageDC.SelectObject(pOldImageDC);
}

void CTransBmp::DrawTransparent(CTransBmp *Sprite, CDC *TargetDC, int xPos, int yPos)
{
	CDC memDC;
	memDC.CreateCompatibleDC(TargetDC);
	
	CBitmap memBM;
	memBM.CreateCompatibleBitmap(TargetDC,Sprite->m_iDimX,Sprite->m_iDimY);
	CBitmap *pOldMemDC = (CBitmap *) memDC.SelectObject(&memBM);

	CDC orDC;
	orDC.CreateCompatibleDC(TargetDC);
	CBitmap *pOldOrDC = (CBitmap *) orDC.SelectObject(&(Sprite->m_bmOr));

	CDC maskDC;
	maskDC.CreateCompatibleDC(TargetDC);
	CBitmap *pOldMaskDC = (CBitmap *) maskDC.SelectObject(&(Sprite->m_bmMask));

	memDC.BitBlt(0,0,Sprite->m_iDimX,Sprite->m_iDimY,TargetDC,xPos,yPos,SRCCOPY);
	memDC.BitBlt(0,0,Sprite->m_iDimX,Sprite->m_iDimY,&maskDC,0,0,SRCAND);
	memDC.BitBlt(0,0,Sprite->m_iDimX,Sprite->m_iDimY,&orDC,0,0,SRCPAINT);
	
	TargetDC->BitBlt(xPos,yPos,Sprite->m_iDimX,Sprite->m_iDimY,&memDC,0,0,SRCCOPY);

	// Clean up
	maskDC.SelectObject(pOldMaskDC);
	orDC.SelectObject(pOldOrDC);
	memDC.SelectObject(pOldMemDC);
}

CTransBmp::CTransBmp(CDC * TargetDC,UINT resPic, COLORREF TranspColor )
{
	Create( TargetDC, resPic, TranspColor );
}

void CTransBmp::Create( CDC * TargetDC,UINT resPic, COLORREF TranspColor )
{
	m_bmPic.LoadBitmap(resPic);

	BITMAP bm;
	m_bmPic.GetBitmap(&bm);

	m_iDimX = bm.bmWidth;
	m_iDimY = bm.bmHeight;

	m_iPosX = 0;
	m_iPosY = 0;

	CTransBmp::CreateMask(TargetDC,this, TranspColor );
}
