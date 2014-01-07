#include <windows.h>
#include "InfoDlg.h"
#include "resource.h"

id3tag xediting_id3;

char *xediting_id3fn;


BOOL CALLBACK InfoDlgProc(	HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:

			SetWindowText( GetDlgItem( hwndDlg, IDC_ED_ARTIST ), xediting_id3.artist );
/*
			SendMessage(	GetDlgItem( hwndDlg, IDC_ARTIST ),
							BM_SETCHECK, 
							(xch_supzero)?BST_CHECKED:BST_UNCHECKED,
							0L );
*/
			return TRUE;
		break;
		case WM_COMMAND:
			switch ( wParam )
			{
				case IDOK:
					EndDialog( hwndDlg, IDOK );
				break;
				case IDCANCEL:
					EndDialog( hwndDlg, IDCANCEL );
				break;
			}
	}
	return FALSE;
}