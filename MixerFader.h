/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1998 Langis Pitre (lpitre@sympatico.ca)
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


#ifndef _LANGIS_PITRE_MIXER_FADER_H_
#define _LANGIS_PITRE_MIXER_FADER_H_

#include "MixerBase.h"

//=============================================================================
//                             CMixerFader Class
//
// Since this class is derived from CSliderCtrl, it allows the user to associate
// a slider resource with a mixer fader control. All messages are handled
// internally through message reflection.
// 
// How to use:
//
// 1) create a slider resource (either horizontal or vertical)
// 2) declare a variable of type CMixerFader in your CDialog-derived class
//    declaration
// 3) In your OnInitDialog, subclass the CMixerFader object to associate it
//    with the resource (or use DDE with a variable of type CMixerFader)
// 4) In your OnInitDialog, call the Init() member funcion.
//
// If the device has two channels, it is possible to either create one
// main volume and one balance slider, or to create sliders for the left
// and right channels.
//
// For example:  say you want a main volume slider and a cd audio
// volume slider, the call to Init() for the main and CD volumes would be:
//
//	m_mainVolume.Init( MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,   // destination line
//                     NO_SOURCE,                              // no source line
//	                   MIXERCONTROL_CONTROLTYPE_VOLUME,	       // control type
//					   CMixerFader::MAIN );
//
//	m_cdVolume.Init( MIXERLINE_COMPONENTTYPE_DST_SPEAKERS,     // destination line
//                   MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC,  // source line
//	                 MIXERCONTROL_CONTROLTYPE_VOLUME,          // control type
//					 CMixerFader::MAIN );
//
//=============================================================================

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
class CMixerFader : public CSliderCtrl, public CMixerBase
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
{
public:
	CMixerFader();
	virtual ~CMixerFader();
	
	enum { LEFT, RIGHT, MAIN, BALANCE };

	int Init( DWORD DstType, DWORD SrcType, DWORD ControlType, int subType );

	//{{AFX_VIRTUAL(CMixerFader)
	//}}AFX_VIRTUAL

protected:

	DWORD GetSliderValue();
	void SetFaderValue();

	void SetSliderPos( int value );
	void SetHVSliderPos();

	int m_FaderSpan;
	int m_FaderStep;
	int m_SliderSpan;
	int m_subType;
	float m_DeltaOverMax;

	MIXERCONTROLDETAILS_UNSIGNED *m_Value;

	//{{AFX_MSG(CMixerFader)
	afx_msg LONG OnMixerControlChanged( UINT, LONG );
	afx_msg void HScroll ( UINT nSBCode, UINT nPos );
	afx_msg void VScroll ( UINT nSBCode, UINT nPos );
	afx_msg void OnDestroy();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//===========================================================================

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _LANGIS_PITRE_MIXER_FADER_H_
