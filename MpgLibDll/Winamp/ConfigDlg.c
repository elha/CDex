#include <windows.h>
#include "ConfigDlg.h"
#include "resource.h"

char xed_title[256] = {'\0',};
int xch_enable		= 0;
int xch_supzero		= 0;
int xch_force		= 0;
int xch_dispavg		= 0;
int xch_24bit		= 0;
int xch_volume		= 0;
int xch_reverse		= 0;
int xch_invert		= 0;
int xstrbuflen		= 256;
char xed_proxy[ 256 ] = {'\0',};


BOOL CALLBACK ConfigDlgProc(	HWND hwndDlg, 
								UINT uMsg, 
								WPARAM wParam, 
								LPARAM lParam )
{
	switch ( uMsg )
	{
		case WM_INITDIALOG:

			SetWindowText( GetDlgItem( hwndDlg, IDC_ED_TITLE ), xed_title );

			SendMessage(	GetDlgItem( hwndDlg, IDC_CH_SUPZERO ),
							BM_SETCHECK, 
							(xch_supzero)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SendMessage(	GetDlgItem( hwndDlg, IDC_CH_ENABLE ),
							BM_SETCHECK, 
							(xch_enable)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SendMessage(	GetDlgItem( hwndDlg, IDC_CH_FORCE ),
							BM_SETCHECK, 
							(xch_force)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SendMessage(	GetDlgItem( hwndDlg, IDC_CH_DISPAVG ),
							BM_SETCHECK, 
							(xch_dispavg)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SendMessage(	GetDlgItem( hwndDlg, ID_CH_REV ),
							BM_SETCHECK, 
							(xch_reverse)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SendMessage(	GetDlgItem( hwndDlg, IDC_CH_INVERT ),
							BM_SETCHECK, 
							(xch_invert)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SendMessage(	GetDlgItem( hwndDlg, IDC_CH_VOLUME ),
							BM_SETCHECK, 
							(xch_volume)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SendMessage(	GetDlgItem( hwndDlg, IDC_CH_24BIT ),
							BM_SETCHECK, 
							(xch_24bit)?BST_CHECKED:BST_UNCHECKED,
							0L );
			SetDlgItemInt( hwndDlg, IDC_ED_STRBUF , xstrbuflen, FALSE );
			SetWindowText( GetDlgItem( hwndDlg, IDC_ED_PROXY ), xed_proxy );

			return TRUE;
		break;

		case WM_COMMAND:
			switch ( wParam )
			{
				case IDCANCEL:
					EndDialog( hwndDlg, IDCANCEL );
				break;
				case IDOK:
					GetWindowText( GetDlgItem( hwndDlg, IDC_ED_TITLE ), xed_title, sizeof( xed_title  ) );

					xch_supzero = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, IDC_CH_SUPZERO ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xch_enable = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, IDC_CH_ENABLE ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xch_force = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, IDC_CH_FORCE ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xch_dispavg = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, IDC_CH_DISPAVG ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xch_reverse = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, ID_CH_REV ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xch_invert = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, IDC_CH_INVERT ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xch_volume = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, IDC_CH_VOLUME ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xch_24bit = ( BST_CHECKED == SendMessage(	GetDlgItem( hwndDlg, IDC_CH_24BIT ), BM_GETCHECK, 0, 0L ) ) ? TRUE:FALSE ;
					xstrbuflen = GetDlgItemInt( hwndDlg, IDC_ED_STRBUF , NULL , FALSE );

					GetWindowText( GetDlgItem( hwndDlg, IDC_ED_PROXY ), xed_proxy, sizeof( xed_proxy ) );

					EndDialog( hwndDlg, IDOK );
				break;
			}
		break;

	}
	return FALSE;

}
 
 
