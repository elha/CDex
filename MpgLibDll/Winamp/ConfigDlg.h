#ifndef CONFIGDLG_H_INCLUDED
#define CONFIGDLG_H_INCLUDED

BOOL CALLBACK ConfigDlgProc(	HWND hwndDlg, 
								UINT uMsg, 
								WPARAM wParam, 
								LPARAM lParam );

extern char xed_title[256];
extern int xch_enable;
extern int xch_supzero;
extern int xch_force;
extern int xch_dispavg;
extern int xch_24bit;
extern int xch_volume;
extern int xch_reverse;
extern int xch_invert;
extern int xstrbuflen;
extern char xed_proxy[256];


#endif

