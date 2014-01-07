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


#ifndef _LANGIS_PITRE_MIXER_BASE_H_
#define _LANGIS_PITRE_MIXER_BASE_H_

#include <mmsystem.h>

// Comment-out the next line to prevent warnings to popup. If there was a problem
// initializing a mixer control, your program won't crash since the classes check
// the validity of values, and your controls will simply be disabled.
// For debugging, it does help to find out where the problem is though.

//#define DO_MIXER_WARNING


const DWORD NO_SOURCE = MIXERLINE_COMPONENTTYPE_SRC_LAST + 1;

#ifdef _DEBUG
#define DO_MIXER_WARNING 1
#endif

//=============================================================================
//                         CMixerBase Class
//
// Base class of all the mixer classes
// Takes care of opening the mixer device to get a valid handler.
// This handle, along with the number of channels are stored in
// member variables. The Init() method queries the mixer interface
// for the desired mixer control.
//
// You don't use this class directly, unless you want to derive new classes
// from it, but you can use the 4 inline functions to retrieve the name and
// short name of a mixer control or mixer line:
//
//	        GetControlName()
//	        GetControlShortName()
//	        GetLineName()
//	        GetLineShortName()
//
//===========================================================================

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
class CMixerBase  
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
{
public:

	CMixerBase();
	virtual ~CMixerBase();

	void* GetValuePtr() { return m_val; }

	int GetDestinLineIndex( MIXERLINE &line, DWORD lineType );
	int GetSourceLineIndex( MIXERLINE &line, DWORD lineType );

	int GetMixerControlValue();
	int SetMixerControlValue();
	
	CUString GetControlName() const;
	CUString GetControlShortName() const;
	CUString GetLineName() const;
	CUString GetLineShortName() const;

	static LPCTSTR GetControlTypeString( DWORD type );
	static LPCTSTR GetLineTypeString( DWORD type );


protected:
	
	int  Init( DWORD DstType, DWORD SrcType, DWORD ControlType, HWND hwnd );
	
	void CloseMixer();

	CUString m_LineName;
	CUString m_DestLineName;
	CUString m_LineShortName;

	HMIXER       m_HMixer;
    MIXERCONTROL m_Control;
	int          m_nChannels;

private:
	int InitMXDetails();

    MIXERCONTROLDETAILS  m_Details;
	void *m_val;
};
	

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
inline CUString CMixerBase::GetControlName() const
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
{
	if( m_HMixer )
		return CUString(m_Control.szName);
	else
		return CUString( _W( "" ) );
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
inline CUString CMixerBase::GetControlShortName() const
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
{
	if( m_HMixer )
		return CUString(m_Control.szShortName);
	else
		return CUString(_W( "" ));
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
inline CUString CMixerBase::GetLineName() const
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
{
	return m_LineName;
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
inline CUString CMixerBase::GetLineShortName() const
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
{
	return m_LineShortName;
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
inline void CMixerBase::CloseMixer()
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
{
	if ( m_HMixer ) mixerClose( m_HMixer );
	m_HMixer = 0;
	
	if( m_val ) delete[] m_val;
	m_val = NULL;
}



//===========================================================================
//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // _LANGIS_PITRE_MIXER_BASE_H_

//===========================================================================
