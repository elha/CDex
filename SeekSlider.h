#ifndef SEEKSLIDER_H_INCLUDED
#define SEEKSLIDER_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SeekSlider.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CSeekSlider window

class CSeekSlider : public CSliderCtrl
{
// Construction
public:
	CSeekSlider();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSeekSlider)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSeekSlider();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSeekSlider)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
