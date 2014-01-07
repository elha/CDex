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


#include <wchar.h>
#include <tchar.h>
#include <windows.h>
#include <string.h>
#include <assert.h>
#include <Stdio.h>
#include <Stdlib.h>
#include <TIME.h>
#include "vector"
#include "AspiDebug.h"

using namespace std ;

#include "AspiDebug.h"


int gs_nDebug = 1;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void SetDebugLevel( int nValue )
{
	gs_nDebug= nValue;
}



void ErrorCallBackFunc(char* lpszFile,int nLine,char* lpszError)
{
/*
	CTime myTime=CTime::GetCurrentTime();

	FILE* fpError=NULL;


	// Open error log file
	fpError=fopen( "c:\\CDexErrorLog.txt","a");

	if (fpError==NULL)
	{
		AfxMessageBox("Can't open CDexErrorLog.txt file");
		return;
	}
	
	CString strFileName=lpszFile;
	int nPos=strFileName.ReverseFind('\\');
	if (nPos)
		strFileName=strFileName.Right(strFileName.GetLength()-nPos-1);

	CString strLog,strTmp;
//	strTmp.Format("Error occured in CDex version %s",g_config.GetVersion());strLog=strTmp;
	strTmp.Format("date %04d:%02d:%02d ",myTime.GetYear(),myTime.GetMonth(),myTime.GetDay());strLog+=strTmp;
	strTmp.Format("time %02d:%02d:%02d ",myTime.GetHour(),myTime.GetMinute(),myTime.GetSecond());strLog+=strTmp;
	strTmp.Format("Error:%s (file:%s line %05d)",lpszError,strFileName,nLine);strLog+=strTmp;

//	AfxMessageBox(strTmp);

	fprintf(fpError,"%s\n",strLog);
	fclose(fpError);
*/
}
