#ifndef INFODLG_H_INCLUDED
#define INFODLG_H_INCLUDED

typedef struct id3tag {
	char tag[3];
	char title[30];
	char artist[30];
	char album[30];
	char year[4];
	char comment[30];
	unsigned char genre;
} id3tag;


BOOL CALLBACK InfoDlgProc(	HWND hwndDlg, 
							UINT uMsg, 
							WPARAM wParam, 
							LPARAM lParam );


extern id3tag xediting_id3;
extern char *xediting_id3fn;


#endif
