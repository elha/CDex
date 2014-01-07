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


#ifndef _MFECTOOLTIP_H_
#define _MFECTOOLTIP_H_


// default tooltip colors
#define IVORY       RGB( 255, 255, 225 )
#define BLACK		RGB( 0,     0,   0 )

// this structure holds all the tooltip information
struct ToolTipInfo : public CObject
{
	UINT			nControlID;		// ID of the control
	UINT			nInfoSize;		// number of lines in the info
	CStringArray	nControlInfo;	// container of the information
	COLORREF		nTextColor;		// text color
	COLORREF		nBackColor;		// background color
};

class CMFECToolTip : public CWnd
{
// Construction
public:
	CMFECToolTip();   // standard constructor
	virtual ~CMFECToolTip();	// destructor

	void		Create(CWnd* pWnd);
	void		ErasePreviousToolTipDisplay( UINT );
	void		ShowToolTip( UINT );		// explicitly shown the tooltip given the control ID

	// NOTE: the user must override the PreTranslateMessage in the calling window in order
	// to handle mousemovent. 
	void		ShowToolTip( CPoint& );		// called only during Mousemovement

	// tooltip functions
	BOOL		AddControlInfo( UINT, CStringArray&, COLORREF back = IVORY, COLORREF text=BLACK );
	BOOL		AddControlInfo( UINT, UINT, COLORREF back = IVORY, COLORREF text=BLACK );
	BOOL		RemoveControlInfo( UINT );

	// inline functions
	void		SetFontSize( int size ) { m_nFontSize = size; }

// Implementation
protected:
	CWnd*			m_pParentWnd;
	int				m_nHeight;
	int				m_nWidth;
	int				m_nFontSize;

	int				m_nTotalLine;
	int				m_maxCharInLine;   

	virtual void	CalculateInfoBoxRect(UINT nControlID, CRect* pInfoRect);
	virtual void	CalculateHeightAndWidth(CStringArray& straInfos);

private:
	ToolTipInfo*	IsControlIDExisting( UINT );
	void			DisplayInfo( ToolTipInfo* );

private:
	CObArray		m_aControlInfo;
	UINT			m_currentControlID;
	int				m_aTabStobs[ 10];
};


#endif
