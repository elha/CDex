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


//=============================================================================
//=============================================================================
//                         CMixerBase Class
//
// Base class of all the mixer classes
// Takes care of opening the mixer device to get a valid handler.
// This handle, along with the number of channels are stored in
// member variables.
// You can use the 4 inline functions to retrieve the name and
// short name of a mixer control or mixer line:
//
//	        GetControlName()
//	        GetControlShortName()
//	        GetLineName()
//	        GetLineShortName()
//
//=============================================================================
//=============================================================================
#include <stdarg.h>
#include "stdafx.h"
#include "MixerBase.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "MixerBase" ) );

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : CMixerBase
//          
// Descr. : 
//          
//-----------------------------------------------------------------------------
CMixerBase::CMixerBase()
{
	m_HMixer = 0;
	m_val = NULL;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : ~CMixerBase
//          
// Descr. : 
//          
//-----------------------------------------------------------------------------
CMixerBase::~CMixerBase()
{
	CloseMixer();
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : Init
//          
// Descr. : Opens the mixer device and get a handle. Queries the mixer device for
//          the desired control type. This control should belong either to a
//          destination line or to a source line. 
//        - If the control belongs to a destination line, the 'DstType' parameter
//          should specify its component type, and the 'SrcType' parameter should
//          be set to NO_SOURCE.
//        - If the control belongs to a source line, the 'DstType' parameter
//          should specify the component type of the line that owns the source line,
//          and the 'SrcType' parameter should specify the component type of the
//          source line that owns the control.
//
//          Called by the derived classes only ( protected ).
//          If the desired mixer line is found, the specified control is queried,
//          and memory il allocated to hold the values that will be sent to and
//          queried from the mixer device.
//          
// Return : int 1 if successful, 0 otherwise
//
// Arg    : DWORD DstType : destination line component type
//          
//          These are valid arguments ( if available ):
//
//              MIXERLINE_COMPONENTTYPE_DST_DIGITAL 
//              MIXERLINE_COMPONENTTYPE_DST_LINE
//              MIXERLINE_COMPONENTTYPE_DST_MONITOR
//              MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
//              MIXERLINE_COMPONENTTYPE_DST_HEADPHONES
//              MIXERLINE_COMPONENTTYPE_DST_TELEPHONE 
//              MIXERLINE_COMPONENTTYPE_DST_WAVEIN
//              MIXERLINE_COMPONENTTYPE_DST_MONITOR
//              MIXERLINE_COMPONENTTYPE_DST_VOICEIN
//
// Arg    : DWORD SrcType : source line component type
//          
//          These are valid arguments ( if available ):
//
//              NO_SOURCE ( use when the desired control belongs to a destination line )
//
//              MIXERLINE_COMPONENTTYPE_SRC_DIGITAL 
//              MIXERLINE_COMPONENTTYPE_SRC_LINE
//              MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE
//              MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER
//              MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC
//              MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE 
//              MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER
//              MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT
//              MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY
//              MIXERLINE_COMPONENTTYPE_SRC_ANALOG
//
// Arg    : DWORD ControlType : desired control type
//
// Arg    : HWND hwnd : handle of window that will handle mixer specific messages
//                      this handle is supplied by the derived classes.
//
//-----------------------------------------------------------------------------
int CMixerBase::Init( DWORD DstType, DWORD SrcType, DWORD ControlType, HWND hwnd )
{	
	UINT nbMixers = mixerGetNumDevs();
	if( nbMixers < 1 )
	{
		LTRACE( _T( "CMixerBase::Init No mixer device present" ) );
		return 0;
	}

	// ---- open the mixer device ----

	if ( m_HMixer )
		mixerClose( m_HMixer );

	if( mixerOpen( &m_HMixer, 0, ( DWORD )hwnd, 0, CALLBACK_WINDOW ) != MMSYSERR_NOERROR )
	{
		LTRACE( _T( "CMixerBase::Init Could not open mixer device" ) );
		return 0;
	}

	// ----- find the desired destination line -----
	
	MIXERLINE line;

	if( GetDestinLineIndex( line, DstType ) == -1 )
	{
		LTRACE( _T( "CMixerBase::Init Warning: the destination line \n%s\nwas not found." ), GetLineTypeString( DstType ) );
		CloseMixer();
		return 0;
	}
	
	m_DestLineName = line.szName;

	// ----- find the desired source line -----

	if( SrcType != NO_SOURCE )
	{
		if(  GetSourceLineIndex( line, SrcType ) == -1 )
		{
			LTRACE( _T( "CMixerBase::Init Warning: the source line \n" ) );
			CloseMixer();
			return 0;
		}
	}

	// we found the line! let's see if it has any controls...

	m_nChannels     = line.cChannels;
	m_LineName      = line.szName;
	m_LineShortName = line.szShortName;

	//TRACE( "Number of controls available: %d\n", line.cControls );
	if( line.cControls == 0 )
	{
		LTRACE( _T( "CMixerBase::Init No controls available for this audio line: %s" ), line.szName );
		CloseMixer();
		return 0;
	}

	// ---------- We try and find the desired control for the mixer line ---------
    
	MIXERLINECONTROLS   mixerLineControl;

	mixerLineControl.cbStruct       = sizeof( MIXERLINECONTROLS );
	mixerLineControl.dwLineID       = line.dwLineID;
	mixerLineControl.dwControlType  = ControlType;
	mixerLineControl.cControls      = 1;
	mixerLineControl.cbmxctrl       = sizeof( MIXERCONTROL );
	mixerLineControl.pamxctrl       = &m_Control;
	
	if( mixerGetLineControls( ( HMIXEROBJ )m_HMixer, &mixerLineControl, 
		MIXER_GETLINECONTROLSF_ONEBYTYPE ) != MMSYSERR_NOERROR )
	{
		LTRACE( _T( "CMixerBase::Init - mixerGetLineControls failed\n" ) );
		CloseMixer();
		return 0;
	}

	// we got the control, now store some values for later queries

	if( !InitMXDetails() )
		return 0;
	
	return 1;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetDestinLineIndex
//          
// Descr. : Loops through all possible destination lines, and returns the
//          index of the line that matches the type passed as argument.
//          The MIXERLINE structure is also filled along the way by the call
//          to mixerGetLineInfo().
//          
// Return : int : index of line found, otherwise -1
//
// Arg    : MIXERLINE &line : 
// Arg    : DWORD lineType  : 
//-----------------------------------------------------------------------------
int CMixerBase::GetDestinLineIndex( MIXERLINE &line, DWORD lineType )
{
	// ----- find out how many destination lines are available -----

	MIXERCAPS caps;
	if( mixerGetDevCaps( (UINT)m_HMixer, &caps, sizeof(MIXERCAPS) ) != MMSYSERR_NOERROR )
	{
		return -1;
	}

	int ndest = caps.cDestinations;
	for( int i = 0; i < ndest; i++ )
	{
		line.cbStruct = sizeof( MIXERLINE );
		line.dwSource = 0;
		line.dwDestination = i;
		if( mixerGetLineInfo( ( HMIXEROBJ )m_HMixer, &line, 
							  MIXER_GETLINEINFOF_DESTINATION ) != MMSYSERR_NOERROR )
		{
			return -1;
		}
		
		if( line.dwComponentType == lineType )
		{
			return( i );
		}		
	}
	return -1;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetSourceLineIndex
//          
// Descr. : Loops through all possible source lines associated with a given
//          destination line, and returns the index of the line that matches
//          the type passed as argument. The destination line index is taken
//          from the dwDestination member of the MIXERLINE structure, so this
//          member must be properly set before calling this function.
//          The MIXERLINE structure is also filled along the way by the call
//          to mixerGetLineInfo().
//          
// Return : int
// Arg    : MIXERLINE &line : 
// Arg    : DWORD lineType  : 
//-----------------------------------------------------------------------------
int CMixerBase::GetSourceLineIndex( MIXERLINE &line, DWORD lineType )
{
	UINT  nconn    = line.cConnections;
	DWORD DstIndex = line.dwDestination;

	for( UINT j = 0; j < nconn; j++ )
	{
		line.cbStruct = sizeof( MIXERLINE );
		line.dwSource = j;
		line.dwDestination = DstIndex;
		if( mixerGetLineInfo( ( HMIXEROBJ )m_HMixer, &line, 
							  MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR )
		{
			return -1;
		}
		
		if( line.dwComponentType == lineType )
		{
			return( j );
		}			
	}
	return -1;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : InitMXDetails
//          
// Descr. : prepare the MIXERCONTROLDETAILS structure for queries, and allocate
//          the proper amount of memory to hold the values that will be sent to
//          and queried from the mixer device.
//
// Return : void
//-----------------------------------------------------------------------------
int CMixerBase::InitMXDetails()
{
	m_Details.cbStruct       = sizeof( MIXERCONTROLDETAILS );
    m_Details.dwControlID    = m_Control.dwControlID;
	m_Details.cMultipleItems = m_Control.cMultipleItems;
	
	if( m_Control.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM )
	{
		m_Details.cChannels = 1;
	}
	else
	{
		m_Details.cChannels = m_nChannels;
	}

	switch( m_Control.dwControlType & MIXERCONTROL_CT_UNITS_MASK )
	{
		case MIXERCONTROL_CT_UNITS_UNSIGNED:
		case MIXERCONTROL_CT_UNITS_DECIBELS:
			m_Details.cbDetails = sizeof( MIXERCONTROLDETAILS_UNSIGNED );
			break;
		
		case MIXERCONTROL_CT_UNITS_PERCENT:
		case MIXERCONTROL_CT_UNITS_SIGNED:
			m_Details.cbDetails = sizeof( MIXERCONTROLDETAILS_SIGNED );
			break;

		case MIXERCONTROL_CT_UNITS_BOOLEAN:
			m_Details.cbDetails = sizeof( MIXERCONTROLDETAILS_BOOLEAN );
			break;
		
		default:
			LTRACE( _T( "CMixerBase::InitMXDetails Control uses unknown units." ) );
			CloseMixer();
			return 0;
	}

	int nItems = 1;
	if( m_Control.fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE )
	{
		nItems = m_Control.cMultipleItems;
	}
	nItems *= m_Details.cChannels;

	if( m_val ) delete[] m_val;
	m_val = new char[ nItems* m_Details.cbDetails ];
	if( m_val == NULL )
	{
		LTRACE( _T( "CMixerBase::InitMXDetails Memory allocation error." ) );
		CloseMixer();
		return 0;
	}

	m_Details.paDetails = m_val;

	return 1;
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : SetMixerControlValue
//          
// Descr. : 
//          
// Return : int
// Arg    : MIXERCONTROL &ctrl    : 
// Arg    : CMControlInfo &ctInfo : 
// Arg    : LONG newVal           : 
//-----------------------------------------------------------------------------
int CMixerBase::SetMixerControlValue( )
{
	if( mixerSetControlDetails( ( HMIXEROBJ )m_HMixer, &m_Details, 
			                      MIXER_SETCONTROLDETAILSF_VALUE ) 
		!= MMSYSERR_NOERROR )
	{
		TRACE( _T( "CMixerBase::SetMixerControlValue mixerSetControlDetails error\n" ) );
		return 0;
	}

	return 1;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetMixerControlValue
//          
// Descr. : 
//          
// Return : int
// Arg    : CMControlInfo &ctInfo : 
//-----------------------------------------------------------------------------
int CMixerBase::GetMixerControlValue( )
{
	if( mixerGetControlDetails( ( HMIXEROBJ )m_HMixer, &m_Details, 
			                      MIXER_SETCONTROLDETAILSF_VALUE ) 
		!= MMSYSERR_NOERROR )
	{
		TRACE( _T( "CMixerDialog::GetMixerControlValue mixerGetControlDetails error\n" ) );
		return 0;
	}
	
	return 1;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
//
// Utility functions. These are declared static, so they can be used outside of
// an object instance, with the namespace being CMixerBase::
//
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetControlTypeString
//          
// Descr. : 
//          
// Return : LPCTSTR
// Arg    : DWORD type : 
//-----------------------------------------------------------------------------
LPCTSTR CMixerBase::GetControlTypeString( DWORD type )
{
	switch( type )
	{
		case MIXERCONTROL_CONTROLTYPE_CUSTOM:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_CUSTOM" ) );
		case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_BOOLEANMETER" ) );
		case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_SIGNEDMETER" ) );
		case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER" ) );
		case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_PEAKMETER" ) );
		case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_BOOLEAN" ) );
		case MIXERCONTROL_CONTROLTYPE_ONOFF:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_ONOFF" ) );
		case MIXERCONTROL_CONTROLTYPE_MUTE:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_MUTE" ) );
		case MIXERCONTROL_CONTROLTYPE_MONO:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_MONO" ) );
		case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_LOUDNESS" ) );
		case MIXERCONTROL_CONTROLTYPE_STEREOENH:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_STEREOENH" ) );
		case MIXERCONTROL_CONTROLTYPE_BUTTON:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_BUTTON" ) );
		case MIXERCONTROL_CONTROLTYPE_DECIBELS:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_DECIBELS" ) );
		case MIXERCONTROL_CONTROLTYPE_SIGNED:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_SIGNED" ) );
		case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_UNSIGNED" ) );
		case MIXERCONTROL_CONTROLTYPE_PERCENT:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_PERCENT" ) );
		case MIXERCONTROL_CONTROLTYPE_SLIDER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_SLIDER" ) );
		case MIXERCONTROL_CONTROLTYPE_PAN:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_PAN" ) );
		case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_QSOUNDPAN" ) );
		case MIXERCONTROL_CONTROLTYPE_FADER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_FADER" ) );
		case MIXERCONTROL_CONTROLTYPE_VOLUME:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_VOLUME" ) );
		case MIXERCONTROL_CONTROLTYPE_BASS:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_BASS" ) );
		case MIXERCONTROL_CONTROLTYPE_TREBLE:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_TREBLE" ) );
		case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_EQUALIZER" ) );
		case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_SINGLESELECT" ) );
		case MIXERCONTROL_CONTROLTYPE_MUX:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_MUX" ) );
		case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT" ) );
		case MIXERCONTROL_CONTROLTYPE_MIXER:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_MIXER" ) );
		case MIXERCONTROL_CONTROLTYPE_MICROTIME:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_MICROTIME" ) );
		case MIXERCONTROL_CONTROLTYPE_MILLITIME:
			return (  _T( "MIXERCONTROL_CONTROLTYPE_MILLITIME" ) );
			
		default:
			return (  _T( "---- UNKNOWN CONTROL TYPE ----" ) );
	}
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetLineTypeString
//          
// Descr. : 
//          
// Return : LPCTSTR
// Arg    : DWORD type : 
//-----------------------------------------------------------------------------
LPCTSTR CMixerBase::GetLineTypeString( DWORD type )
{
	switch( type )
	{
		case MIXERLINE_COMPONENTTYPE_DST_DIGITAL:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_DIGITAL" );
		case MIXERLINE_COMPONENTTYPE_DST_UNDEFINED:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_UNDEFINED" );
		case MIXERLINE_COMPONENTTYPE_DST_LINE:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_LINE");
		case MIXERLINE_COMPONENTTYPE_DST_MONITOR:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_MONITOR");
		case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_SPEAKERS");
		case MIXERLINE_COMPONENTTYPE_DST_HEADPHONES:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_HEADPHONES");
		case MIXERLINE_COMPONENTTYPE_DST_TELEPHONE:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_TELEPHONE");
		case MIXERLINE_COMPONENTTYPE_DST_WAVEIN:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_WAVEIN");
		case MIXERLINE_COMPONENTTYPE_DST_VOICEIN:
			return _T( "MIXERLINE_COMPONENTTYPE_DST_VOICEIN");
		case MIXERLINE_COMPONENTTYPE_SRC_ANALOG:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_ANALOG");
		case MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY");
		case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC");
		case MIXERLINE_COMPONENTTYPE_SRC_DIGITAL:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_DIGITAL");
		case MIXERLINE_COMPONENTTYPE_SRC_LINE:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_LINE");
		case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE");
		case MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER");
		case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER");
		case MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE");
		case MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED");
		case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT:
			return _T( "MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT");
		case NO_SOURCE:
			return _T( "No source line");
		default:
			return _T(  "---- UNKNOWN LINE TYPE ----" );
	}
}
