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


#ifndef PLOT_H_INCLUDED
#define PLOT_H_INCLUDED

// Define trace options constants

#define	YTRACE			0x10000000
#define	NOTRACE			0x00000000
#define	XTRACE			0x00000000
#define	TONEXTLINETYPE  0x00000001
#define STEPLINETYPE	0x00000010
#define POINTTYPE       0x00000020
#define BARTYPE         0x00000040
#define LOGSCALEX		0x00000080
#define LOGSCALEY		0x00000100

class CPlotTrace : public CObject
{
public:
	int			m_nPoints;
	double		*m_pData;
	long		m_traceType;
	CUString		m_strName;
	COLORREF	m_color;
	double		m_dMax;
	double		m_dMin;
	double		m_dWMax;
	double		m_dWMin;


public:
	BOOL CalcMinMax();
	CPlotTrace();
	~CPlotTrace();
	BOOL AddTrace(short *pData,long traceType,int nPoints,CUString strName,COLORREF color);
	BOOL AddTrace(double *pData,long traceType,int nPoints,CUString strName,COLORREF color);
	void ClearTrace();
};


#define MAXTRACES 20

/////////////////////////////////////////////////////////////////////////////
// CPlot window

class CPlot : public CWnd
{
private:
	int			m_nScalePoints;
	CFont		m_ScaleFont;
	CRect		m_rcWnd;
	int			m_nTraces;
	CPlotTrace	m_Traces[MAXTRACES];
	COLORREF	m_BkColor;
	COLORREF	m_WindowBkColor;
	int			m_nMajGridLinesX;
	int			m_nMajGridLinesY;
	BOOL		m_bPlotScaleX;
	BOOL		m_bPlotScaleY;
	CRect		m_rcPlotArea;
	double		m_dMin;
	double		m_dMax;
	int			m_nXTrace;

public:
	// CONSTRUCTORS
	CPlot();
	virtual ~CPlot();

	// ACCESSORS
	CUString GetScaleLabel(double dValue,int nMaxExp10);
	double GetMaxY();
	double GetMinY();

	// MANIPULATORS
	void	SetMaxY(double dValue);
	void	SetMinY(double dValue);
	void	SetMajorGridTicks(int x,int y);

	virtual void SetBkColor(COLORREF ColorRef);
	virtual void SetWindowBkColor(COLORREF ColorRef);
	BOOL	AddTrace(double *pData,long traceType,int nPoints,CUString strName,COLORREF color);
	BOOL	AddTrace(short *pData,long traceType,int nPoints,CUString strName,COLORREF color);
	void	DrawScaleY(CDC* pPaintDC,int nTrace);
	void	DrawScaleX(CDC* pPaintDC);
	CRect	CalcPlotArea();
	void	CalcTickMarksX(int nPoints);
	void	CalcAutoScale();

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPlot)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL


	// Generated message map functions
protected:
	//{{AFX_MSG(CPlot)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#endif
