/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
** Copyright (C) Jack Moffit (jack@icecast.org)
** Copyright (C) Michael Smith (msmith@labyrinth.net.au)
** Copyright (C) Aaron Porter (aaron@javasource.org)
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


#define WIN32_MEAN_AND_LEAN
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <commctrl.h>
#include "in_vorbis_config.h"
#include "in_vorbis_util.h"
#include "in_vorbis_tags.h"
#include "resource.h"

#define NUM_PAGES       2

static char szTempBuffer[ 256 ];

#define RI(x, def)          (x = GetPrivateProfileInt("FLAC", #x, def, ini_name))
#define WI(x)               WritePrivateProfileString("FLAC", #x, itoa(x, szTempBuffer, 10), ini_name)
#define RS(x, n, def)       GetPrivateProfileString("FLAC", #x, def, x, n, ini_name)
#define WS(x)               WritePrivateProfileString("FLAC", #x, x, ini_name)

#define PREAMP_RANGE            24

#define Check(x,y)              CheckDlgButton(hwnd, x, y ? BST_CHECKED : BST_UNCHECKED)
#define GetCheck(x)             (IsDlgButtonChecked(hwnd, x)==BST_CHECKED)
#define GetSel(x)               SendDlgItemMessage(hwnd, x, CB_GETCURSEL, 0, 0)
#define GetPos(x)               SendDlgItemMessage(hwnd, x, TBM_GETPOS, 0, 0)
#define Enable(x,y)             EnableWindow(GetDlgItem(hwnd, x), y)


typedef struct
{
    HWND htab;
    HWND hdlg;
    RECT r;
    HWND all[NUM_PAGES];
} LOCALDATA;

static const char default_format[] = "[%artist% - ]$if2(%title%,%filename%)";


static BOOL CALLBACK ConfigDlgProc( HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam );
static INT_PTR CALLBACK GeneralProc( HWND hwnd, UINT dwMessage, WPARAM wParam, LPARAM lParam );
static void UpdatePreamp( HWND hwnd, HWND hamp );
static void UpdateRG( HWND hwnd );
static void UpdateDither( HWND hwnd );


int DoConfig( HINSTANCE hInstance, HWND parent )
{
	InitCommonControls();

    return DialogBoxParam(	hInstance,
							MAKEINTRESOURCE( IDD_CONFIG ),
							parent,
							ConfigDlgProc,
							(LONG)hInstance) == IDOK;
}


static INT_PTR CALLBACK GeneralProc(HWND hwnd, UINT dwMessage, WPARAM wParam, LPARAM lParam)
{
    switch ( dwMessage )
    {
		// init
		case WM_INITDIALOG:
			//	SetDlgItemText(hwnd, IDC_TITLE, flac_cfg.title.tag_format);
        return TRUE;

		// commands
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				// ok
				case IDOK:
		//            GetDlgItemText(hwnd, IDC_TITLE, flac_cfg.title.tag_format, sizeof(flac_cfg.title.tag_format));
				break;

				// reset
			   case IDC_RESET:
					SetDlgItemText(hwnd, IDC_TITLE, default_format);
				break;

				// show help message 
				case IDC_TAGZ_HELP:
					MessageBox(hwnd, in_vorbis_tags_manual, "Help", 0);
				break;
        }
        break;
    }

    return 0;
}

static void UpdatePreamp( HWND hwnd, HWND hamp )
{
    int pos = SendMessage( hamp, TBM_GETPOS, 0, 0 ) - PREAMP_RANGE;
    sprintf( szTempBuffer, "%d dB", pos );
    SetDlgItemText( hwnd, IDC_PA, szTempBuffer );
}

static void UpdateRG( HWND hwnd )
{
    int on = GetCheck(IDC_ENABLE);
    Enable(IDC_ALBUM, on);
    Enable(IDC_LIMITER, on);
    Enable(IDC_PREAMP, on);
    Enable(IDC_PA, on);
}

static void UpdateDither( HWND hwnd )
{
    int on = GetCheck( IDC_DITHERRG );
    Enable( IDC_SHAPE, on );
}


static INT_PTR CALLBACK OutputProc(HWND hwnd, UINT dwMessage, WPARAM wParam, LPARAM lParam)
{
    switch ( dwMessage )
    {
	    case WM_INITDIALOG:
			// init
			Check( IDC_ENABLE, ( REPLAYGAIN_MODE_AUDIOPHILE == vpi_config.bUseReplayGain  ) ? TRUE : FALSE );
			Check( IDC_ALBUM, vpi_config.nReplayGainMode );
			Check( IDC_LIMITER, vpi_config.bUseReplayGainBooster );
//			Check( IDC_DITHER, flac_cfg.output.resolution.normal.dither_24_to_16 );
//			Check( IDC_DITHERRG, flac_cfg.output.resolution.replaygain.dither );

			// prepare preamp slider
			{
				HWND hamp = GetDlgItem(hwnd, IDC_PREAMP);
				SendMessage(hamp, TBM_SETRANGE, 1, MAKELONG(0, PREAMP_RANGE*2));
				SendMessage(hamp, TBM_SETPOS, 1, vpi_config.nReplayGainPreAmp + PREAMP_RANGE );
				UpdatePreamp( hwnd, hamp );
			}
			// fill comboboxes
			{
				HWND hlist = GetDlgItem(hwnd, IDC_TO);
				SendMessage(hlist, CB_ADDSTRING, 0, (LPARAM)"16 bps");
				SendMessage(hlist, CB_ADDSTRING, 0, (LPARAM)"24 bps");
// FIXME				SendMessage(hlist, CB_SETCURSEL, flac_cfg.output.resolution.replaygain.bps_out/8 - 2, 0);
  
				hlist = GetDlgItem(hwnd, IDC_SHAPE);
				SendMessage(hlist, CB_ADDSTRING, 0, (LPARAM)"None");
				SendMessage(hlist, CB_ADDSTRING, 0, (LPARAM)"Low");
				SendMessage(hlist, CB_ADDSTRING, 0, (LPARAM)"Medium");
				SendMessage(hlist, CB_ADDSTRING, 0, (LPARAM)"High");
// FIXME				SendMessage(hlist, CB_SETCURSEL, flac_cfg.output.resolution.replaygain.noise_shaping, 0);
			}

			UpdateRG( hwnd );

			UpdateDither( hwnd );

			return TRUE;
		break;

		// commands
		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
				// ok
				case IDOK:
					vpi_config.bUseReplayGain = GetCheck( IDC_ENABLE );
					vpi_config.nReplayGainMode = GetCheck( IDC_ALBUM ) ? REPLAYGAIN_MODE_AUDIOPHILE : REPLAYGAIN_MODE_RADIO ;
	//				flac_cfg.output.replaygain.hard_limit = GetCheck(IDC_LIMITER);
	//				flac_cfg.output.replaygain.preamp = GetPos(IDC_PREAMP) - PREAMP_RANGE;
	//				flac_cfg.output.resolution.normal.dither_24_to_16 = GetCheck(IDC_DITHER);
	//				flac_cfg.output.resolution.replaygain.dither = GetCheck(IDC_DITHERRG);
	//				flac_cfg.output.resolution.replaygain.noise_shaping = GetSel(IDC_SHAPE);
	//				flac_cfg.output.resolution.replaygain.bps_out = (GetSel(IDC_TO)+2)*8;
				break;

			// reset
			case IDC_RESET:
				Check(IDC_ENABLE, 1);
				Check(IDC_ALBUM, 0);
				Check(IDC_LIMITER, 0);
				Check(IDC_DITHER, 0);
				Check(IDC_DITHERRG, 0);

				SendDlgItemMessage(hwnd, IDC_PREAMP, TBM_SETPOS, 1, PREAMP_RANGE);
				SendDlgItemMessage(hwnd, IDC_TO, CB_SETCURSEL, 0, 0);
				SendDlgItemMessage(hwnd, IDC_SHAPE, CB_SETCURSEL, 1, 0);

				UpdatePreamp(hwnd, GetDlgItem(hwnd, IDC_PREAMP));
				UpdateRG(hwnd);
				UpdateDither(hwnd);
				break;
			// active check-boxes
			case IDC_ENABLE:
				UpdateRG(hwnd);
				break;
			case IDC_DITHERRG:
				UpdateDither(hwnd);
				break;
			}
			break;
		// scroller
		case WM_HSCROLL:
			if (GetDlgCtrlID((HWND)lParam)==IDC_PREAMP)
				UpdatePreamp(hwnd, (HWND)lParam);
			return 0;
	}


    return 0;
}


static void ScreenToClientRect(HWND hwnd, RECT *rect)
{
    POINT pt = { rect->left, rect->top };
    ScreenToClient(hwnd, &pt);
    rect->left = pt.x;
    rect->top  = pt.y;

    pt.x = rect->right;
    pt.y = rect->bottom;
    ScreenToClient(hwnd, &pt);
    rect->right  = pt.x;
    rect->bottom = pt.y;
}

static void SendCommand(HWND hwnd, int command)
{
    LOCALDATA *data = (LOCALDATA*)GetWindowLong(hwnd, GWL_USERDATA);
    SendMessage(data->hdlg, WM_COMMAND, command, 0);
}

static void BroadcastCommand(HWND hwnd, int command)
{
    LOCALDATA *data = (LOCALDATA*)GetWindowLong(hwnd, GWL_USERDATA);
    int i;

    for (i=0; i<NUM_PAGES; i++)
        SendMessage(data->all[i], WM_COMMAND, command, 0);
}

static void OnSelChange(HWND hwnd)
{
    LOCALDATA *data = (LOCALDATA*)GetWindowLong(hwnd, GWL_USERDATA);
    int index = TabCtrl_GetCurSel(data->htab);
    if (index < 0) return;
    // hide previous
    if (data->hdlg)
        ShowWindow(data->hdlg, SW_HIDE);
    // display
    data->hdlg = data->all[index];
    SetWindowPos(data->hdlg, HWND_TOP, data->r.left, data->r.top, data->r.right-data->r.left, data->r.bottom-data->r.top, SWP_SHOWWINDOW);
}

static BOOL CALLBACK ConfigDlgProc(HWND hWndDlg, UINT dwMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bReturn = FALSE;
	static activePage = 0;


    switch ( dwMessage )
	{
		case WM_INITDIALOG:
		{
            HINSTANCE hInstance = (HINSTANCE)lParam;
            LOCALDATA *data = LocalAlloc(LPTR, sizeof(LOCALDATA));
            TCITEM item;

            // init
            SetWindowLong( hWndDlg, GWL_USERDATA, (LONG)data );
            data->htab = GetDlgItem( hWndDlg, IDC_TABS );
            data->hdlg = NULL;

            // add pages
            item.mask = TCIF_TEXT;
            data->all[0] = CreateDialog(	hInstance,
											MAKEINTRESOURCE( IDD_CONFIG_GENERAL ),
											hWndDlg,
											GeneralProc );
            item.pszText = "General";
            TabCtrl_InsertItem( data->htab, 0, &item );

            data->all[1] = CreateDialog(	hInstance,
											MAKEINTRESOURCE( IDD_CONFIG_OUTPUT ),
											hWndDlg,
											OutputProc );
            item.pszText = "Output";
            TabCtrl_InsertItem( data->htab, 1, &item );

            // get rect (after adding pages)
            GetWindowRect( data->htab, &data->r );
            ScreenToClientRect( hWndDlg, &data->r );
            TabCtrl_AdjustRect( data->htab, 0, &data->r );

            // simulate item change
            TabCtrl_SetCurSel( data->htab, activePage );
            OnSelChange( hWndDlg );
			bReturn = TRUE;
        }
		break;

	    // destory
		case WM_DESTROY:
		{
            int i = 0;
			LOCALDATA *data = (LOCALDATA*)GetWindowLong(hWndDlg, GWL_USERDATA);


            activePage = TabCtrl_GetCurSel(data->htab);

            for (i=0; i<NUM_PAGES; i++)
			{
                DestroyWindow(data->all[i]);
			}

			LocalFree(data);
        }
        break;

		case WM_NOTIFY:
			if (LOWORD(wParam) == IDC_TABS)
			{
				NMHDR *hdr = (NMHDR*)lParam;

				switch (hdr->code)
				{
					case TCN_SELCHANGE:
						OnSelChange( hWndDlg );
					break;

				}
			}
		break;

		case WM_COMMAND:
		{
	        switch ( LOWORD( wParam ) )
			{

//				case IDC_RESET:
//					SendCommand( hWndDlg, IDC_RESET );
//				break;

				case ID_CLOSE:
					store_vpi_config( );
		            BroadcastCommand( hWndDlg, IDOK );
		            /* fall through */
				case IDCANCEL:
					// if ( bFileChanged == TRUE )
					{
		//                if (MessageBox(hWndDlg, "Save changes?", "Save changes", MB_YESNO) == IDYES)
		  //                  List_SaveID3(hWndDlg, GetDlgItem(hWndDlg, IDC_ID3LIST), info_fn);
					}
					EndDialog( hWndDlg, wParam );
					bReturn = TRUE;
				break;
			}
		}
		break;
	}

	return bReturn;
}




