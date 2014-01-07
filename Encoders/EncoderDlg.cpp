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


#include "stdafx.h"
#include "cdex.h"
#include "config.h"
#include "EncoderDlg.h"
#include "MPEGHeader.h"
#include "AudioFile.h"
#include "EncoderBladeDllDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int MP3SampleRates[3][3] =
{
	{ 48000, 44100, 32000 },
	{ 24000, 22050, 16000 },
	{  8000, 11025, 11025 } 
};

 
CEncoderDlg::CEncoderDlg(UINT nIDTemplate,CWnd* pParentWnd):CDialog(nIDTemplate,pParentWnd)
{
	m_nVersion=0;
	m_nLayer=2;
}


void CEncoderDlg::FillMaxBitrateTable()
{
	// Depending on the settings, fill the bit-rate tables
	m_MaxBitrate.ResetContent();

	int nItems=sizeof(MPEGBitrates[m_nVersion][m_nLayer])/sizeof(MPEGBitrates[m_nVersion][m_nLayer][0]);
	
	int i;

	// DO NOT DISPLAY ITEM 0
	for (i=1;i<nItems;i++)
	{
		CUString strItem;
        CUStringConvert strCnv;
		strItem.Format( _W( "%d kbps" ), MPEGBitrates[ m_nVersion ][ m_nLayer ][ i ] );
        m_MaxBitrate.AddString( strCnv.ToT( strItem ) );
	}
}

void CEncoderDlg::FillOutSampleRateCtrl( )
{
	int		i = 0;
	CUString	strLang;
    CUStringConvert strCnv;

	m_OutSampleRate.ResetContent();

	strLang = g_language.GetString( IDS_AUTO );
    m_OutSampleRate.AddString( strCnv.ToT( strLang ) );

	for ( i=0; i< sizeof( MP3SampleRates[m_nVersion] ) / sizeof( MP3SampleRates[m_nVersion][0] ); i++ )
	{
		CUString strAdd;
		strAdd.Format( _W( "%d"), MP3SampleRates[m_nVersion][ i ] );
        m_OutSampleRate.AddString( strCnv.ToT( strAdd ) );
	}
	m_OutSampleRate.SetCurSel( 0 );
}

void CEncoderDlg::FillMinBitrateTable()
{
	// Depending on the settings, fill the bit-rate tables
	m_MinBitrate.ResetContent();

	int nItems=sizeof(MPEGBitrates[m_nVersion][m_nLayer])/sizeof(MPEGBitrates[m_nVersion][m_nLayer][0]);
	
	int i;

	// DO NOT DISPLAY ITEM 0
	for (i=1;i<nItems;i++)
	{
		CUString strItem;
        CUStringConvert strCnv;
		strItem.Format( _W( "%d kbps" ), MPEGBitrates[ m_nVersion ][ m_nLayer ][ i ] );
        m_MinBitrate.AddString( strCnv.ToT( strItem ) );
	}
}

int CEncoderDlg::GetMinBitrate()
{
	int nItems=sizeof(MPEGBitrates[m_nVersion][m_nLayer])/sizeof(MPEGBitrates[m_nVersion][m_nLayer][0]);
	return MPEGBitrates[m_nVersion][m_nLayer][m_MinBitrate.GetCurSel()+1];
}

int CEncoderDlg::GetMaxBitrate()
{
	int nItems=sizeof(MPEGBitrates[m_nVersion][m_nLayer])/sizeof(MPEGBitrates[m_nVersion][m_nLayer][0]);
	return MPEGBitrates[m_nVersion][m_nLayer][m_MaxBitrate.GetCurSel()+1];
}


void CEncoderDlg::SetMaxBitrate(int nBitrate)
{
	int nItems=sizeof(MPEGBitrates[m_nVersion][m_nLayer])/sizeof(MPEGBitrates[m_nVersion][m_nLayer][0]);
	
	int i;

	for (i=1;i<nItems;i++)
	{
		if (nBitrate==MPEGBitrates[m_nVersion][m_nLayer][i])
		{
			m_MaxBitrate.SetCurSel(i-1);
			return;
		}
	}

	// default return if not found
	m_MaxBitrate.SetCurSel(9);
}

void CEncoderDlg::SetMinBitrate(int nBitrate)
{
	int nItems=sizeof(MPEGBitrates[m_nVersion][m_nLayer])/sizeof(MPEGBitrates[m_nVersion][m_nLayer][0]);
	
	int i;

	for (i=1;i<nItems;i++)
	{
		if (nBitrate==MPEGBitrates[m_nVersion][m_nLayer][i])
		{
			m_MinBitrate.SetCurSel(i-1);
			return;
		}
	}

	// default return if not found
	m_MinBitrate.SetCurSel(9);
}


