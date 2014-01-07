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


#include "StdAfx.h"
#include <limits.h>
#include <math.h>
#include <float.h>
#include "Plot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define sRound(x) ((SHORT)(x+0.5))

void CPlotTrace::ClearTrace()
{
	m_pData=NULL;

	m_strName=_W( "" );
	m_color=RGB(0,0,0);

	m_dMin=0.0;
	m_dMax=0.0;
	m_dWMin=0.0;
	m_dWMax=0.0;
	m_traceType=NOTRACE;
}


CPlotTrace::CPlotTrace()
{
	ClearTrace();
}

CPlotTrace::~CPlotTrace()
{
	delete [] m_pData;
	ClearTrace();
}

BOOL CPlotTrace::AddTrace(double *pData,long traceType,int nPoints,CUString strName,COLORREF color)
{
	int i;
	if (pData)
	{
		m_pData=new double[nPoints];
		for (i=0;i<nPoints;i++)
			m_pData[i]=pData[i];
	}

	m_traceType=traceType;
	m_strName=strName;
	m_color=color;
	m_nPoints=nPoints;
	CalcMinMax();
	return 0;
}


BOOL CPlotTrace::AddTrace(short *pData,long traceType,int nPoints,CUString strName,COLORREF color)
{
	int i;
	if (pData)
	{
		m_pData=new double[nPoints];
		for (i=0;i<nPoints;i++)
			m_pData[i]=(double)pData[i];
	}

	m_traceType=traceType;
	m_strName=strName;
	m_color=color;
	m_nPoints=nPoints;
	CalcMinMax();
	return 0;
}


BOOL CPlotTrace::CalcMinMax()
{
	int i;

	m_dMax=-DBL_MAX;
	m_dMin=DBL_MAX;

	for (i=0;i<m_nPoints;i++)
	{
		m_dMin=min(m_dMin,m_pData[i]);
		m_dMax=max(m_dMax,m_pData[i]);
	}
	m_dWMin=m_dMin;
	m_dWMax=m_dMax;

	return 0;
}


/////////////////////////////////////////////////////////////////////////////
// CPlot

CPlot::CPlot()
{
	m_nXTrace=-1;
	m_nTraces=0;
	m_nMajGridLinesX=8;
	m_nMajGridLinesY=8;
	m_BkColor=RGB(0xFF,0xFF,0xFF);
	m_WindowBkColor=RGB(0xFF,0xFF,0xFF);

	m_dMax=-DBL_MAX;
	m_dMin=DBL_MAX;
}

CPlot::~CPlot()
{
}


BEGIN_MESSAGE_MAP(CPlot, CWnd)
	//{{AFX_MSG_MAP(CPlot)
	ON_WM_SIZE()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPlot message handlers

void CPlot::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	m_rcWnd=CRect(0,0,cx,cy);
}

void CPlot::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	int iTrace;

	if (m_nTraces>0)
	{
		// Clear Background Color;
		CBrush bkBrush(m_BkColor);
		CBrush *pOldBrush=(CBrush *)dc.SelectObject(&bkBrush);
		dc.Rectangle(m_rcWnd);
		dc.SelectObject(pOldBrush);

		m_rcPlotArea=CalcPlotArea();

		// Clear Plot Area Color;
		CBrush plotBrush(m_WindowBkColor);
		pOldBrush=(CBrush *)dc.SelectObject(&plotBrush);
		dc.Rectangle(m_rcPlotArea);
		dc.SelectObject(pOldBrush);

		// Set the number of tick_marks.
		//CalcTickMarksX(m_Traces[m_nXTrace].m_nPoints);

		// Plot X grid
		CPen majorGridPen(PS_SOLID,1,RGB(0x00,0x80,0x00));
		CPen *pOldPen=dc.SelectObject(&majorGridPen);
		
		// Plot X grid
		for (int iGridLine=1;iGridLine<=m_nMajGridLinesX;iGridLine++)
		{
			int xPos=m_rcPlotArea.left+m_rcPlotArea.Width()*iGridLine/(m_nMajGridLinesX+1);

			dc.MoveTo(xPos,m_rcPlotArea.top);
			dc.LineTo(xPos,m_rcPlotArea.bottom);
		}

		// Plot Y grid
		for (int iGridLine=1;iGridLine<=m_nMajGridLinesY;iGridLine++)
		{
			int yPos=m_rcPlotArea.top+m_rcPlotArea.Height()*iGridLine/(m_nMajGridLinesY+1);
			dc.MoveTo(m_rcPlotArea.left,yPos);
			dc.LineTo(m_rcPlotArea.right,yPos);
		}

		// Select proper font
		CFont* pOldFont=(CFont *)dc.SelectObject(&m_ScaleFont);


		DrawScaleX(&dc);
		DrawScaleY(&dc,0);


		// Select Old Font.
		dc.SelectObject(pOldFont);

		// Select OldPen
		dc.SelectObject(pOldPen);


		for (iTrace=0;iTrace<m_nTraces;iTrace++)
		{
			int iPoint;
			double dYScale=1.0;
			double dXScale=1.0;
			int iHeight=m_rcPlotArea.Height()-1;
			int iWidth =m_rcPlotArea.Width()-1;

			if ((m_Traces[iTrace].m_traceType & YTRACE)==YTRACE)
			{
	//			if (m_Traces[iTrace].m_dWMax>m_Traces[iTrace].m_dWMin)
	//				dYScale=iHeight/(m_Traces[iTrace].m_dWMax-m_Traces[iTrace].m_dMin);
				if (m_dMax>m_dMin)
					dYScale=iHeight/(m_dMax-m_dMin);
				dXScale=(double)iWidth/(double)m_nScalePoints;

				CPen tracePen(PS_SOLID,1,m_Traces[iTrace].m_color);
				CPen *pOldPen=dc.SelectObject(&tracePen);
				
				CPoint ptNew;
				CPoint ptPrev;


				for (iPoint=0;iPoint<m_Traces[iTrace].m_nPoints;iPoint++)
				{
					double dNewY=m_rcPlotArea.bottom-1-((m_Traces[iTrace].m_pData[iPoint]-m_dMin)*dYScale)+0.5;
					double dNewX=m_rcPlotArea.left+iPoint*dXScale+0.5;

					ptNew=CPoint(sRound(dNewX),sRound(dNewY));
					
					if ( ptNew.y<=m_rcPlotArea.bottom && ptNew.y>=m_rcPlotArea.top)
					{
						if ((m_Traces[iTrace].m_traceType & TONEXTLINETYPE)==TONEXTLINETYPE)
						{

							if (iPoint==0)
							{
								dc.MoveTo(ptNew);
							}
							dc.LineTo(ptNew);
						} else
						if ((m_Traces[iTrace].m_traceType & STEPLINETYPE)==STEPLINETYPE)
						{
							if (iPoint==0)
							{
								ptPrev=ptNew;
							}
							dc.MoveTo(ptPrev);
							dc.LineTo(ptNew.x,ptPrev.y);
							dc.LineTo(ptNew);
						} else
						if ((m_Traces[iTrace].m_traceType & POINTTYPE)==POINTTYPE)
						{
							dc.MoveTo(ptNew);
							dc.LineTo(ptNew.x+1,ptNew.y);
						} else
						if ((m_Traces[iTrace].m_traceType & BARTYPE)==BARTYPE)
						{
						}
					}
					else
					{
						ptNew.y=min(max(ptNew.y,m_rcPlotArea.bottom),m_rcPlotArea.top);
						dc.MoveTo(ptNew);
					}
					ptPrev=ptNew;
				}
				dc.SelectObject(pOldPen);
			}
		}
	}	
}

BOOL CPlot::AddTrace(double *pData,long traceType,int nPoints,CUString strName,COLORREF color)
{
	BOOL bRet=0;

	if ( (traceType & XTRACE)==XTRACE)
		m_nXTrace=m_nTraces;
	m_nTraces++;
	bRet=m_Traces[m_nTraces-1].AddTrace(pData,traceType,nPoints,strName,color);

	// Determine min/max of y scales
	CalcAutoScale();

	m_nScalePoints=nPoints;

	return bRet;
}


void CPlot::SetBkColor(COLORREF ColorRef)
{
	m_BkColor=ColorRef;
}

void CPlot::SetWindowBkColor(COLORREF ColorRef)
{
	m_WindowBkColor=ColorRef;
}

void CPlot::SetMajorGridTicks(int x,int y)
{
	m_nMajGridLinesX=x;
	m_nMajGridLinesY=y;
}

BOOL CPlot::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	BOOL bRet;
	bRet=CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);

	m_ScaleFont.CreateFont ( -10, 0, 0, 0, FW_NORMAL,0,0, 0, 
							DEFAULT_CHARSET, 
	    					OUT_CHARACTER_PRECIS,
							CLIP_CHARACTER_PRECIS,
						PROOF_QUALITY,
							DEFAULT_PITCH|FF_DONTCARE,
							_T( "Courier New" ) );



	SetFont(&m_ScaleFont);
	return bRet;
}

CRect CPlot::CalcPlotArea()
{
	CRect rcRet;
	GetClientRect(&rcRet);
	rcRet.left  +=55;
	rcRet.right -=20;
	rcRet.top   +=10;
	rcRet.bottom-=30;
	return rcRet;
}

CUString CPlot::GetScaleLabel(double dValue,int nMaxExp10)
{
	int nAbsExp10=abs(nMaxExp10);

	CUString strRet;

	if (nAbsExp10<1)
		strRet.Format( _W( "%5.2f" ), dValue );
	else
	if (nAbsExp10<2)
		strRet.Format( _W( "%5.1f" ), dValue );
	else
	if (nAbsExp10<3)
		strRet.Format( _W( "%3d" ), (int)dValue );
	else
	if (nAbsExp10<4)
		strRet.Format( _W( "%4d" ), (int)dValue );
	else
	if (nAbsExp10<5)
		strRet.Format( _W( "%5d" ), (int)dValue );
	else
	{
		double dPow10=pow((double)10, nMaxExp10);
		strRet.Format( _W( "%5.2f" ), dValue * dPow10 );
	}

	return strRet;
}

void CPlot::DrawScaleX(CDC* pDC)
{
	int	iGridLine;
	double dMinMax=max(fabs(m_Traces[m_nXTrace].m_dWMax),fabs(m_Traces[m_nXTrace].m_dWMin));
	int nMaxExp10=(int)log10(dMinMax);


	pDC->SetTextAlign(TA_TOP|TA_CENTER);

	// Plot scale text for each marker, including the boundaries.
	for (iGridLine=0;iGridLine<=m_nMajGridLinesX+1;iGridLine++)
	{
		if (m_bPlotScaleX)
		{
			// Create Temporary string to hold scale label.
			CString strTmp;

			// Plot GridLine.
			int xPos=m_rcPlotArea.left+m_rcPlotArea.Width()*iGridLine/(m_nMajGridLinesX+1);
			
			// Calculate index into X trace.
			int nPoint=(int)((double)iGridLine*(m_nScalePoints-1)/(double)(m_nMajGridLinesX+1));

			// Clip max point, since space between last marker and right edge can be
			// different from the rest.
			nPoint=min(nPoint,m_Traces[m_nXTrace].m_nPoints-1);

			// Get value at postiont nPoint.
			double dValue=m_Traces[m_nXTrace].m_pData[nPoint];

			// Get the apprioprate string label.
			strTmp=GetScaleLabel(dValue,nMaxExp10);

			// And plot the scale text.
			pDC->TextOut(xPos,m_rcPlotArea.bottom+10,strTmp);
		}
	}
}

void CPlot::DrawScaleY(CDC* pDC,int nTrace)
{
	int	iGridLine;
	double dMinMax=max(fabs(m_Traces[nTrace].m_dWMax),fabs(m_Traces[nTrace].m_dWMin));
	int nMaxExp10=(int)log10(dMinMax);

	pDC->SetTextAlign(TA_BASELINE|TA_LEFT);

	// Calculate scale factor based on the window leveling.
	double dInc=(m_dMax-m_dMin)/(double)(m_nMajGridLinesY+1);
	// Get offset position.
	double dStart=m_dMin;

	// Plot scale text for each marker, including the boundaries.
	for (iGridLine=0;iGridLine<=m_nMajGridLinesY+1;iGridLine++)
	{
		if (m_bPlotScaleX)
		{
			CString strTmp;

			// Plot GridLine.
			int yPos=m_rcPlotArea.bottom-m_rcPlotArea.Height()*iGridLine/(m_nMajGridLinesY+1);

			// Get value at postion iGridLine, based in window min/max.
			double dValue=dStart+dInc*iGridLine;

			// Get the apprioprate string label.
			strTmp=GetScaleLabel(dValue,nMaxExp10);

			// And plot the scale text.
			pDC->TextOut(m_rcPlotArea.left-40,yPos,strTmp);
		}
	}
}


void CPlot::CalcAutoScale()
{
	m_dMax=-DBL_MAX;
	m_dMin=DBL_MAX;

	int iTrace;

	for (iTrace=0;iTrace<m_nTraces;iTrace++)
	{
		if ((m_Traces[iTrace].m_traceType & YTRACE)==YTRACE)
		{
			m_dMin=min(m_dMin,m_Traces[iTrace].m_dMin);
			m_dMax=max(m_dMax,m_Traces[iTrace].m_dMax);
		}
	}
}


// Get minimum Y value of all traces.
double CPlot::GetMinY()
{
	return m_dMin;
}

// Get maximum Y value of all traces.
double CPlot::GetMaxY()
{
	return m_dMax;
}


// Set the minimum Y display value to overrule the auto-scaling.
void CPlot::SetMinY(double dValue)
{
	m_dMin=dValue;
}

// Set the maximum Y display value to overrule the auto-scaling.
void CPlot::SetMaxY(double dValue)
{
	m_dMax=dValue;
}

	

void CPlot::CalcTickMarksX(int nPoints)
{
	int i;


	BOOL bFound=FALSE;

	if (nPoints<=10)
	{
		m_nMajGridLinesX=nPoints-1;
		return;
	}


	// Above ten, we wanne have at least 6 markers.
	for (i=10;i>=6 && !bFound;i--)
	{
		int iMod=nPoints%i;
		if (iMod==0)
		{
			bFound=TRUE;
			m_nMajGridLinesX=i-1;
			m_nScalePoints=nPoints;
		}
	}
	// Oops, we got ourselfs a number that's not dividable by 6..10
	// do the good old recursion to try the next one.
	if (!bFound)
		CalcTickMarksX(nPoints+1);
}

