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


#ifndef MP3TORIFFWAVDLG_H_INCLUDED
#define MP3TORIFFWAVDLG_H_INCLUDED

#include "OpenFileDlg.h"
#include "afxwin.h"

#define SYNC 0xFFF

#pragma pack(push,1)

typedef struct MP3FRAMEHEADER_TAG
{
	// BYTE 0
	BYTE	FrameSyncH;						// should be all "1"

	// BYTE 1
	BYTE	ErrorProtection	: 1;
	BYTE	Layer			: 2;
	BYTE	Version			: 1;
	BYTE	FrameSyncL		: 4;

	// BYTE 2
	BYTE	Extention		: 1;
	BYTE	Padding			: 1;
	BYTE	SampleRateIndex	: 2;
	BYTE	BitRateIndex	: 4;

	// BYTE 3
	BYTE	emphasis		: 2;
	BYTE	Original		: 1;
	BYTE	Copyright		: 1;
	BYTE	ModeExtention	: 2;
	BYTE	ChannelMode		: 2;
} MP3FRAMEHEADER;


/*

	FILE STRUCTURE AUDIO MPEG FILE
--------------------------------------------------------------------------
file

	a file has one or more 'frames'.

	a frame has a 'frame header' and 'frame data'
--------------------------------------------------------------------------
frame length

	the frame length depends on the format and can vary per frame.
	the frame ends before the next sync header (see frame header).
--------------------------------------------------------------------------
frame header

	the frame header of an audio mpeg file consist of 4 bytes, which
	are at the beginning of a frame. They don't have to be the same
	for every frame. (p.e. the bit 'padding' can change per frame)

	byte       0        1        2        3
	binary 11111111 1111abbc ddddeefg hhjjkmpp		BIG ENDIAN    (Motorola)
           11111111 cbba1111 gfeedddd ppmkjjhh		LITTLE ENDIAN (Intel)

		1 - sync header
		a - version
		b - layer
		c - error protection
		d - bit rate index
		e - sample rate index
		f - padding
		g - extension
		h - channel mode
		j - mode extension
		k - copyright 
		m - original
		p - emphasis
--------------------------------------------------------------------------
1 - sync header

	must be 11111111 1111xxxx ($fffx)
--------------------------------------------------------------------------
a - version

	0 = mpg-ii
	1 = mpg-i
--------------------------------------------------------------------------
b - layer

	0 = layer-1
	3 = layer-2
	2 = layer-3
--------------------------------------------------------------------------
c - error protection

	0 = on
	1 = off
--------------------------------------------------------------------------
d - bit rate

	                     mpg-i
	(hex)   bps@38kHz  bps@44.1kHz  bps@48kHz
	$0          0            0           0
	$1         32           32          32
	$2         64           48          40
	$3         96           56          48
	$4        128           64          56
	$5        160           80          64
	$6        192           96          80
	$7        224          112          96
	$8        256          128         112
	$9        288          160         128
	$a        320          192         160
	$b        352          224         192
	$c        384          256         224
	$d        416          320         256
	$e        448          384         320
	
	                    mpg-ii
	(hex)   bps@16kHz  bps@22kHz  bps@24kHz
	$0          0            0           0
	$1         32            8           8
	$2         48           16          16
	$3         56           24          24
	$4         64           32          32
	$5         80           40          40
	$6         96           48          48
	$7        112           56          56
	$8        128           64          64
	$9        144           80          80
	$a        160           96          96
	$b        176          112         112
	$c        192          128         128
	$d        224          144         144
	$e        256          160         160
--------------------------------------------------------------------------
e - sample rate

	      mpg-i     mpg-ii  (version)
	0 = 44.1 kHz    22 kHz
	1 =   48 kHz    24 kHz
	2 =   38 kHz    16 kHz
--------------------------------------------------------------------------
f - padding

	1 = unused bits are filled (padding required)
	0 = all bits in frame are used
--------------------------------------------------------------------------
g - extension 

	0 = none
	1 = private
--------------------------------------------------------------------------
h - channel mode

	0 = stereo
	1 = joint stereo
	2 = dual channel
	3 = mono
--------------------------------------------------------------------------
j = mode extension

	when encoding with joint stereo, this specifies to which freq.band
	it is bound.

            layer-1&2  layer-3
	0 =     4         0
	1 =     8         4
        2 =    12         8
        3 =    16        16
--------------------------------------------------------------------------
k - copyright

	0 = no
	1 = yes
--------------------------------------------------------------------------
m - original

	0 = no
	1 = yes
--------------------------------------------------------------------------
p - emphasis

	0 = none
	1 = 50/15 microseconds
	3 = CITT j.17
--------------------------------------------------------------------------
*/





/////////////////////////////////////////////////////////////////////////////
// CMp3toRiffWavDlg dialog

class CMp3toRiffWavDlg : public CDialog
{
// Construction
public:
	COpenFileDlg*	m_pFileDlg;
	POSITION		m_Pos;
	int				m_nCurFile;
	int				m_nTotalFiles;

	// CONSTRUCTOR
	CMp3toRiffWavDlg(CWnd* pParent = NULL);
	// DESTRUCTOR
	virtual ~CMp3toRiffWavDlg();

	BOOL ReadMP3Header(CUString& strFileName,int& nChannels, DWORD &dwSampelRate, DWORD& dwBitRate,int& nLayer);

// Dialog Data
	//{{AFX_DATA(CMp3toRiffWavDlg)
	enum { IDD = IDD_MP3TORIFFWAV };
	CProgressCtrl	m_Progress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMp3toRiffWavDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMp3toRiffWavDlg)
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    CStatic m_processFileName;
    CStatic m_processingText;
};

#pragma pack(pop)

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MP3TORIFFWAVDLG_H__362FE020_4369_11D2_8704_D152B10A9D1F__INCLUDED_)
