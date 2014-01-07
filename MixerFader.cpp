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
//                             CMixerFader
//
// Since this class is derived from CSliderCtrl, it allows the user to associate
// a slider resource with a mixer fader control. All messages are handled
// internally through message reflection.
// 
// How to use:
//
// 1 ) create a slider resource ( either horizontal or vertical )
// 2 ) declare a variable of type CMixerFader in your CDialog-derived class
//    declaration
// 3 ) In your OnInitDialog, subclass the CMixerFader object to associate it
//    with the resource ( or use DDE with a variable of type CMixerFader )
// 4 ) In your OnInitDialog, call the Init() member funcion.
//
// If the device has two channels, it is possible to either create one
// main volume and one balance slider, or to create sliders for the left
// and right channels.
//
//=============================================================================
//=============================================================================
//=============================================================================
#include "stdafx.h"
#include "MixerFader.h"
#include "config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

INITTRACE( _T( "MixerFader " ) );


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : CMixerFader
//          
// Descr. : 
//          
//-----------------------------------------------------------------------------
CMixerFader::CMixerFader() : CMixerBase()
{
	m_Value = NULL;
	m_DeltaOverMax = 0;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : ~CMixerFader
//          
// Descr. : 
//          
//-----------------------------------------------------------------------------
CMixerFader::~CMixerFader()
{
}

BEGIN_MESSAGE_MAP( CMixerFader, CSliderCtrl )
	//{{AFX_MSG_MAP( CMixerFader )
	ON_MESSAGE( MM_MIXM_CONTROL_CHANGE, OnMixerControlChanged )
	ON_WM_HSCROLL_REFLECT( )
	ON_WM_VSCROLL_REFLECT( )
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : Init
//          
// Descr. : Initializes the mixer control. Most of the work is done in the
//          base class' implementation of Init.
//          Init queries the mixer device for the specified mixer control type.
//          If such a control is found, its current value is queried, and used
//          to initialize the windows control.
//        - If the call to CMixerBase::Init() is succesfull, m_Value is set to
//          the address of the memory allocated by CMixerBase.
//          
// Return : int 0 if failed, 1 otherwise
//
// Arg    : DWORD DstType    : see definition of CMixerBase::Init() for a description
// Arg    : DWORD SrcType    : see definition of CMixerBase::Init() for a description
// Arg    : DWORD ControlType : type of desired fader.
//
//          These are valid arguments:
//
//              MIXERCONTROL_CONTROLTYPE_FADER 
//              MIXERCONTROL_CONTROLTYPE_VOLUME
//              MIXERCONTROL_CONTROLTYPE_BASS
//              MIXERCONTROL_CONTROLTYPE_TREBLE
//              MIXERCONTROL_CONTROLTYPE_EQUALIZER
//
// Arg    : int subType: If the mixer line has two channels, the subtype can be used to
//                       create a fader for the left or right channel, or as a balance
//                       between the channels, or as a main fader that controls both
//                       channels.
//                       If only one channel is available, only the MAIN subtype may be
//                       specified.
//                       These are valid arguments:
//                       
//                       CMixerFader::LEFT
//                       CMixerFader::RIGHT
//                       CMixerFader::MAIN
//                       CMixerFader::BALANCE
//                       
//-----------------------------------------------------------------------------
int CMixerFader::Init( DWORD DstType, DWORD SrcType, DWORD ControlType, int subType )
{	
	if ( m_hWnd == NULL )
	{
		LTRACE( _T( "CMixerFader::Init No Window attached. Did you create or subclass one?" ) );
		return 0;
	}

	EnableWindow( FALSE );

	if( subType == BALANCE )
	{
		int min, max;
		GetRange( min, max );
		SetSliderPos( ( max - min )/2 );
	}

	// start by checking the type and subtypes 

	if( ( ControlType & MIXERCONTROL_CT_CLASS_MASK ) != MIXERCONTROL_CT_CLASS_FADER )
	{
		LTRACE( _T( "CMixerFader::Init Wrong control type for a fader control: %s" ), 
		              GetControlTypeString( ControlType ) );
		return 0;
	}
	if( subType < LEFT || subType > BALANCE )
	{
		LTRACE( _T( "CMixerFader::Init Wrong sub-type for a fader control: %d" ), subType );
		return 0;
	}

	m_subType = subType;

	if( CMixerBase::Init( DstType, SrcType, ControlType, m_hWnd ) == 0 )
		return 0;

	// set the slider range and number of steps from the information obtained
	// on the mixer control
	m_FaderSpan  = m_Control.Bounds.dwMaximum - m_Control.Bounds.dwMinimum;
	m_FaderStep  = m_FaderSpan/m_Control.Metrics.cSteps;
	m_SliderSpan = m_Control.Metrics.cSteps;
	
	SetRange( 0, m_SliderSpan );

	if( m_nChannels < 2 )
	{
		// a balance, left, or right control on a single channel line doesn't make sense
		// abort and set the balance slider to mid-course
		if( m_subType == LEFT || m_subType == RIGHT || m_subType == BALANCE )
		{
			LTRACE( _T( "CMixerFader::Init Only one channel available on this line. Use Fader type MAIN only.\n Windows control will be disabled.\n \nItem: %s.\n" ),
						  GetControlName() );

			if( subType == BALANCE )
				SetSliderPos( m_SliderSpan/2 );
			CloseMixer();
			return 0;
		}
	}

	m_Value = (MIXERCONTROLDETAILS_UNSIGNED *)GetValuePtr();
	if( m_Value == NULL )
	{
		LTRACE( _T( "CMixerFader::Init m_Value NULL." ) );
		CloseMixer();
		return 0;
	}

	if( GetMixerControlValue() )
	{
		EnableWindow( TRUE );
		SetHVSliderPos();
	}

	return 1;
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : SetHVSliderPos
//          
// Descr. : Updates the slider position according to the subtype of fader used.
//          Since a fader can be used as a balance between the two channels, this function
//          and SetFaderValue() had to work in conjunction to ensure proper behavior of all
//          sliders. For example, if the balance slider is shifted to one side, the right
//          and left channels sliders must react accordingly. And if the main fader slider
//          is moved, the relative position of the right and left channels must be kept
//          constant.
//          You don't have to read the rest of this description. It was not written to
//          impress anybody; I just wanted to keep it logged somewhere!
//
//            For the BALANCE slider position P, the maximum value must be obtained at mid-course,
//          the value decreasing linearly to 0 at both ends.
//            Defining D ( Delta ) as R - L, the difference in value between the Right and
//          Left channel, and M the greatest value between the the Right and Left channel,
//          D / M is a function that is defined in the range [-1, 1], with a maximum at 0.
//            So, m_SliderSpan / 2 * ( D/M + 1 ) will have the range [0, m_SliderSpan] with a
//          maximum at half the slider range.
//
//          -> P = S / 2 * ( DOM + 1 ), where DOM = D/M ( Delta-Over-Max ),and S = m_SliderSpan
//          
//          Whenever a new set of values are sent to the mixer device, it sends back a
//          notification message to all the mixer controls. The CMixerFader handler
//          for this message ( see below ) calls SetHVSliderPos() to reposition the affected
//          sliders. This was a headache because the current value of DOM had to be
//          protected from unwanted changes. I used a member variable ( m_DeltaOverMax ) that
//          could be used by both this function and SetFaderValue(). This looked simple on
//          paper, but it took me a lot of debugging to find the right way to do this.
//
// Return : void
//-----------------------------------------------------------------------------
void CMixerFader::SetHVSliderPos()
{
	float MaxVal;

	if( m_nChannels > 1 )
	{
		MaxVal = ( float )max( m_Value[RIGHT].dwValue, m_Value[LEFT].dwValue );
		if( MaxVal > 0 )
		{
			m_DeltaOverMax = ( ( float )m_Value[RIGHT].dwValue - ( float )m_Value[LEFT].dwValue )/MaxVal;
		}
	}
	else
	{
		MaxVal = ( float )m_Value[0].dwValue;
	}
		
	switch ( m_subType )
	{
		case MAIN: // set the main fader to the maximum value
			SetSliderPos( int( MaxVal/m_FaderStep ) );
			break;

		case BALANCE: // pos = m_SliderSpan/2 * ( delta/ max + 1 )
			if( MaxVal > 0 )
			{
				SetSliderPos( int( m_SliderSpan*( m_DeltaOverMax + 1.0 )/2.0 ) );
			}
			break;

		case LEFT: // only set the left channel slider
			SetSliderPos( int( ( float )m_Value[LEFT].dwValue/m_FaderStep ) );
			break;

		case RIGHT: // only set the right channel slider
			SetSliderPos( int( ( float )m_Value[RIGHT].dwValue/m_FaderStep ) );
			break;

		default:
			LTRACE( _T( "CMixerFader::SetHVSliderPos Wrong sub-type for a fader control." ) );
	}
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : SetFaderValue
//          
// Descr. : the slider position changed, update the values and send to mixer device.
//
//          To determine the values to send to the mixer device, this algorithm is used:
//
//          - If the MAIN slider is moved:
//          The idea is to keep D = R - L constant. In other words, D1/M1 = D2/M2 where
//          D2 and M2 are the new values.
//          Now, if D1 > 0, this means that R1 = M1. This, in the abvove equality, gives:
//          -> L2 = R2( 1 - D1/R1 ) =  R2( 1 - DOM1 )
//          If D1 < 0,  L1 = M1, which leads to:
//          ->  R2 = L2( 1 + DOM1 )
//
//          - If the BALANCE slider is moved:
//          This will change D. Starting from the equation P = S / 2 * ( D/M + 1 )
//          and rearranging the terms, we obtain respectively for the rigth and left channels:
//
//          -> R = M ( 2P/S - 1 ) + L
//          -> L = R - M ( 2P/S - 1 )
//
//          If D > 0, R = M. This gives: L = 2R ( 1 - P/S )
//          If D < 0, L = M. This gives: R = 2LP/S
//
// Return : void
//-----------------------------------------------------------------------------
void CMixerFader::SetFaderValue()
{
	DWORD newVal = GetSliderValue();
	
	float MaxVal;

	if( m_nChannels > 1 )
	{
		MaxVal = ( float )max( m_Value[RIGHT].dwValue, m_Value[LEFT].dwValue );
	}
	else
	{
		MaxVal = ( float )m_Value[0].dwValue;
	}

	switch ( m_subType )
	{
		case MAIN:
			if( m_nChannels > 1 )
			{
				if( m_DeltaOverMax > 0 )
				{
					m_Value[RIGHT].dwValue = newVal;
					m_Value[LEFT].dwValue = DWORD( newVal*( 1.0f - m_DeltaOverMax ) );
				}
				else
				{
					m_Value[LEFT].dwValue = newVal;
					m_Value[RIGHT].dwValue = DWORD( newVal*( 1.0f + m_DeltaOverMax ) );
				}
			}
			else
			{
				m_Value[0].dwValue = newVal;
			}
			break;

		case BALANCE:
			if( ( int )newVal > m_FaderSpan/2 )
			{
				m_Value[RIGHT].dwValue = DWORD( MaxVal );
				m_Value[LEFT].dwValue = DWORD( 2*MaxVal*( 1.0 - ( float )newVal/m_FaderSpan ) );
			}
			else
			{
				m_Value[RIGHT].dwValue = DWORD( 2*MaxVal*( float )newVal/m_FaderSpan );
				m_Value[LEFT].dwValue = DWORD( MaxVal );
			}
			break;

		case LEFT:
			m_Value[LEFT].dwValue = newVal;
			break;

		case RIGHT:
			m_Value[RIGHT].dwValue = newVal;
			break;

		default:
			LTRACE( _T( "CMixerFader::SetFaderValue Wrong sub-type for a fader control." ) );
			return;
	}
	
	SetMixerControlValue();
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetSliderValue
//          
// Descr. : gets the current slider position. The value returned depends on the orientation
//          of the slider.
//          
// Return : DWORD value corresponding to slider position
//-----------------------------------------------------------------------------
DWORD CMixerFader::GetSliderValue()
{
	if( ( GetStyle( ) & TBS_VERT ) == TBS_VERT )
	{
		return ( ( m_SliderSpan - GetPos() ) * m_FaderStep );
	}
	else
	{
		return ( GetPos() * m_FaderStep );
	}
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : OnMixerControlChanged
//          
// Descr. : Message handler called whenever the MCI device sends the MM_MIXM_CONTROL_CHANGE
//          message. The MCI mixer device will send this message for any changing
//          setting, so we need to check that the MCI control that sent the message
//          corresponds to the current fader control.
//          
// Return : LONG
// Arg    : UINT         : 
// Arg    : LONG message : 
//-----------------------------------------------------------------------------
LONG CMixerFader::OnMixerControlChanged( UINT hMix, LONG message )
{
	UINT id = ( UINT )message;

	if( id == m_Control.dwControlID && m_HMixer == ( HMIXER )hMix )
	{
		if( GetMixerControlValue() )
		{
			SetHVSliderPos();
		}
		else
		{
			TRACE( _T( "CMixerFader::OnMixerControlChanged --- error\n" ) );
		}
	}

	return 0L;
}


//=============================================================================
//
// Descr. : Handler for scrolling event message sent whenever the slider is used. 
//          Called by a horizontal slider
//
//=============================================================================
void CMixerFader::HScroll ( UINT nSBCode, UINT /*nPos*/ )
{
	if( m_Value == NULL )
	{
		LTRACE(  _T( "CMixerFader::HScroll Error: Did you call Init() first?" ) );
		return;
	}
	if( nSBCode == TB_THUMBTRACK || nSBCode == TB_ENDTRACK || nSBCode == TB_PAGEDOWN || nSBCode == TB_PAGEUP )
	{
		SetFaderValue();
	}
}

//=============================================================================
//
// Descr. : Handler for scrolling event message sent whenever the slider is used. 
//          Called by a vertical slider
//
//=============================================================================
void CMixerFader::VScroll ( UINT nSBCode, UINT /*nPos*/ )
{
	if( m_Value == NULL )
	{
		LTRACE(  _T( "CMixerFader::VScroll Error: Did you call Init() first?" ) );
		return;
	}

	if( nSBCode == TB_THUMBTRACK || nSBCode == TB_ENDTRACK || nSBCode == TB_PAGEDOWN || nSBCode == TB_PAGEUP )
	{
		SetFaderValue();
	}
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : SetSliderPos
//          
// Descr. : we need to reverse the value for a vertical scrollbar
//          
// Return : void
// Arg    : int value : 
//-----------------------------------------------------------------------------
void CMixerFader::SetSliderPos( int value )
{
	if( ( GetStyle( ) & TBS_VERT ) == TBS_VERT )
	{
		SetPos( m_SliderSpan - value );
	}
	else
	{
		SetPos( value );
	}
}


//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : OnDestroy
//          
// Descr. : 
//          
// Return : void
//-----------------------------------------------------------------------------
void CMixerFader::OnDestroy() 
{
	CloseMixer();
	CSliderCtrl ::OnDestroy();
}
