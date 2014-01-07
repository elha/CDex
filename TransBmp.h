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


#ifndef TRANSBMP_H_INCLUDED
#define TRANSBMP_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTransBmp : public CObject  
{
public:
	
	CTransBmp(CDC * TargetDC,UINT resPic, COLORREF TranspColor = RGB( 255, 0, 255 ) );
	void Create( CDC * TargetDC,UINT resPic, COLORREF TranspColor = RGB( 255, 0, 255 ) );

	static void DrawTransparent(CTransBmp *Sprite,CDC *TargetDC,int xPos, int yPos);
	CTransBmp();
	virtual ~CTransBmp();

protected:
	CBitmap m_bmMask;
	CBitmap m_bmPic;
	CBitmap m_bmOr;
	int m_iPosX;
	int m_iPosY;
	int m_iDimX;
	int m_iDimY;

	static void CreateMask(CDC *TargetDC, CTransBmp *Sprite,COLORREF TranspColor);
};

#endif // !defined(AFX_MYSPRITE_H__FCFEB62C_FB6B_11D3_9A30_F90D86097B62__INCLUDED_)
