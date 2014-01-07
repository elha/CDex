// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8F4F6B57_3F3C_11D2_8704_A9E3FCB3091B__INCLUDED_)
#define AFX_STDAFX_H__8F4F6B57_3F3C_11D2_8704_A9E3FCB3091B__INCLUDED_

#define WINVER 0x0400

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define OEMRESOURCE

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC OLE automation classes
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "AfxMT.h"
#include "AfxTempl.h"
#include "AFXPRIV.H"

#include "CDexUserMsg.h"
#include "Logging/Logging.h"

#include <iostream>
#include <vector>
#include <list>
using namespace std ;

#include "UString.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8F4F6B57_3F3C_11D2_8704_A9E3FCB3091B__INCLUDED_)
