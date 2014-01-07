/*
** CDex - Open Source Digital Audio CD Extractor
**
** Copyright (C) 2006 - 2007 Georgy Berdyshev
** Copyright (C) 1999 - 2003 Albert L. Faber
** Copyright (C) Jack Moffitt (jack@icecast.org)
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
#include <commctrl.h>
#include <stdio.h>

#include "in_vorbis_info.h"
#include "in_vorbis_util.h"
#include "in_vorbis_tags.h"
#include "resource.h"

#define MAX_INF_LINE 4096

CHAR pszTemp[ MAX_INF_LINE ];

BOOL CALLBACK info_dialog_proc(HWND hwndDlg, UINT dwMessage, WPARAM wParam, LPARAM lParam)
{
	DWORD dwComment = 0;
	BOOL bReturn = FALSE;
	

    switch ( dwMessage )
	{
		case WM_INITDIALOG:

			SetDlgItemText(hwndDlg, IDC_TITLE, UTT8_2_ASCII( get_tag_value_by_name( VORBIS_TAG_TITLE ), pszTemp, MAX_INF_LINE )  );
			SetDlgItemText(hwndDlg, IDC_ARTIST, UTT8_2_ASCII( get_tag_value_by_name( VORBIS_TAG_ARTIST ), pszTemp, MAX_INF_LINE )  );
			SetDlgItemText(hwndDlg, IDC_ALBUM, UTT8_2_ASCII( get_tag_value_by_name( VORBIS_TAG_ALBUM ), pszTemp, MAX_INF_LINE )  );
			SetDlgItemText(hwndDlg, IDC_INFO_DATE, UTT8_2_ASCII( get_tag_value_by_name( VORBIS_TAG_DATE ), pszTemp, MAX_INF_LINE )  );
			SetDlgItemText(hwndDlg, IDC_INFO_COMMENT, UTT8_2_ASCII( get_tag_value_by_name( VORBIS_TAG_COMMENT ), pszTemp, MAX_INF_LINE )  );
			SetDlgItemText(hwndDlg, IDC_INFO_TRACKNUMBER, UTT8_2_ASCII( get_tag_value_by_name( VORBIS_TAG_TRACKNUMBER ), pszTemp, MAX_INF_LINE )  );

//	

			_snprintf( pszTemp, MAX_INF_LINE, "%02d:%02d:%02d", 
				( vpi_info.dwFileLengthInMs / 1000 ) / 3600,
				( ( vpi_info.dwFileLengthInMs / 1000 ) / 60 ) % 60,
				( ( vpi_info.dwFileLengthInMs / 1000 ) % 60 )  );
			SetDlgItemText(hwndDlg, IDC_FILE_LENGTH, pszTemp );

			_snprintf( pszTemp, MAX_INF_LINE, "%d", vpi_info.dwChannels );
			SetDlgItemText(hwndDlg, IDC_FILE_CHANNELS, pszTemp );

			_snprintf( pszTemp, MAX_INF_LINE, "%ld", vpi_info.dwFileSize );
			SetDlgItemText(hwndDlg, IDC_FILE_FILESIZE, pszTemp );

			_snprintf( pszTemp, MAX_INF_LINE, "%ld", vpi_info.dwSamplerate );
			SetDlgItemText(hwndDlg, IDC_FILE_SAMPLERATE, pszTemp );

			_snprintf( pszTemp, MAX_INF_LINE, "%ld", vpi_info.dwSerialNumber );
			SetDlgItemText(hwndDlg, IDC_FILE_SERIALNUMBER, pszTemp );

			_snprintf( pszTemp, MAX_INF_LINE, "%ld", vpi_info.dwBitrateNominal / 1000 );
			SetDlgItemText(hwndDlg, IDC_FILE_NOMBITRATE, pszTemp );

			_snprintf( pszTemp, MAX_INF_LINE, "%ld", vpi_info.dwBitrateAverage / 1000 );
			SetDlgItemText(hwndDlg, IDC_FILE_AVGBITRATE, pszTemp );

			dwComment = 0;

			while ( dwComment < get_number_of_tags() )
			{
				PCHAR pszTag = NULL;

				pszTag = vpi_info.pszTags[ dwComment ];

				if ( pszTag && !in_vorbis_tags_is_standard_tag( pszTag ) )
				{
					SendMessage(	GetDlgItem(	hwndDlg, IDC_INFO_TAGS ),
									LB_ADDSTRING,
									0,
									(LPARAM)pszTag );
				}
				dwComment++;
			}

		break;

	    case WM_COMMAND:
		{
	        switch (LOWORD(wParam))
			{

				case ID_CLOSE:
				case IDCANCEL:
					// if ( bFileChanged == TRUE )
					{
		//                if (MessageBox(hwndDlg, "Save changes?", "Save changes", MB_YESNO) == IDYES)
		  //                  List_SaveID3(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST), info_fn);
					}
					EndDialog(hwndDlg, wParam);
					bReturn = TRUE;
				break;
			}
		}
		break;
	}


	
	/*
    faadAACInfo format;
    char *tmp_string;
    char info[1024];
    LV_COLUMN lvc;
    BOOL bResult;

    switch (message)
	{
    case WM_INITDIALOG:

  IDC_FILE_LENGTH
        // Set up the list control for the ID3 tag
        
        // Initialize the LV_COLUMN structure.
        lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvc.fmt = LVCFMT_LEFT;

        // Add the columns.
        lvc.iSubItem = 0;
        lvc.cx = 100;
        lvc.pszText = "Frame";
        ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_ID3LIST), 0, &lvc);
        lvc.iSubItem = 1;
        lvc.cx = 250;
        lvc.pszText = "Data";
        ListView_InsertColumn(GetDlgItem(hwndDlg, IDC_ID3LIST), 1, &lvc);

        // get AAC info
        get_AAC_format(info_fn, &format, NULL, NULL);


        SetDlgItemText(hwndDlg, IDC_HEADER, tmp_string);

        if (format.object_type == 0 // Main 
            tmp_string = "Main";
        else if (format.object_type == 1 // Low Complexity
            tmp_string = "Low Complexity";
        else if (format.object_type == 2 // SSR 
            tmp_string = "SSR (unsupported)";
        else if (format.object_type == 3 // LTP
            tmp_string = "Main LTP";
        SetDlgItemText(hwndDlg, IDC_PROFILE, tmp_string);

        if (format.version == 2) tmp_string = "MPEG2";
        else tmp_string = "MPEG4";
        SetDlgItemText(hwndDlg, IDC_VERSION, tmp_string);

        wsprintf(info, "%d bps", format.bitrate);
        SetDlgItemText(hwndDlg, IDC_BITRATE, info);

        wsprintf(info, "%d Hz", format.sampling_rate);
        SetDlgItemText(hwndDlg, IDC_SAMPLERATE, info);

        wsprintf(info, "%d ch", format.channels);
        SetDlgItemText(hwndDlg, IDC_CHANNELS, info);

        FillID3List(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST), info_fn);
        if (ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_ID3LIST)) == 0)
            EnableWindow(GetDlgItem(hwndDlg, IDC_ID3V2TAG), FALSE);
        else
            EnableCheckbox(GetDlgItem(hwndDlg, IDC_ID3V2TAG), TRUE, TRUE);
        bFileChanged = FALSE;

        bReturn = TRUE;
		break;

    case WM_NOTIFY:
        
        // Branch depending on the specific notification message.
        switch (((LPNMHDR) lParam)->code) { 
            
        // Process LVN_GETDISPINFO to supply information about
        // callback items.
        case LVN_GETDISPINFO:
            List_OnGetDispInfo((LV_DISPINFO *)lParam);
            break;
        case NM_DBLCLK:
            bResult = List_EditData(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST));
            if (bResult)
                EnableCheckbox(GetDlgItem(hwndDlg, IDC_ID3V2TAG), TRUE, TRUE);
            bFileChanged = bFileChanged ? bFileChanged : bResult;
            break;
        }
        bReturn = TRUE;
		break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_ADDSTFRAMES:
            bResult = List_AddStandardFrames(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST));
            if (bResult)
                EnableCheckbox(GetDlgItem(hwndDlg, IDC_ID3V2TAG), TRUE, TRUE);
            bFileChanged = bFileChanged ? bFileChanged : bResult;
            bReturn = TRUE;
        case IDC_ADDFRAME:
            bResult = List_AddFrame(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST));
            if (bResult)
                EnableCheckbox(GetDlgItem(hwndDlg, IDC_ID3V2TAG), TRUE, TRUE);
            bFileChanged = bFileChanged ? bFileChanged : bResult;
            bReturn = TRUE;
        case IDC_DELFRAME:
            bResult = List_DeleteSelected(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST));
            if (ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_ID3LIST)) == 0)
                EnableCheckbox(GetDlgItem(hwndDlg, IDC_ID3V2TAG), FALSE, FALSE);
            bFileChanged = bFileChanged ? bFileChanged : bResult;
            bReturn = TRUE;
        case IDC_EDITFRAME:
            bResult = List_EditData(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST));
            if (bResult)
                EnableCheckbox(GetDlgItem(hwndDlg, IDC_ID3V2TAG), TRUE, TRUE);
            bFileChanged = bFileChanged ? bFileChanged : bResult;
            bReturn = TRUE;
        case IDC_ID3V2TAG:
            bFileChanged = TRUE;
            bReturn = TRUE;

        case IDC_CLOSE:
        case IDCANCEL:
            if (bFileChanged == TRUE) {
                if (MessageBox(hwndDlg, "Save changes?", "Save changes", MB_YESNO) == IDYES)
                    List_SaveID3(hwndDlg, GetDlgItem(hwndDlg, IDC_ID3LIST), info_fn);
            }
            EndDialog(hwndDlg, wParam);
            bReturn = TRUE;
        }
    }
    return FALSE;

*/
	return bReturn;
}
