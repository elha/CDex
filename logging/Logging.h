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


#ifndef LOGING_INCLUDED
#define LOGGING_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_DEFAULT_FILENAME ".\\CDexLog.txt"

extern void LOG_SetLogFileName(const CHAR* lpszLogFileName );

extern void DebugPrintfA( const CHAR* pzFormat, ...);
extern void DebugPrintfW( const TCHAR* pzFormat, ...);

extern TCHAR* GetLastErrorString( );

#define CDexError(c) {ErrorCallBackFunc( __FILE__,__LINE__,c);}

#define INITTRACE( a ) static int gs_nDebug = QueryModuleDebugLevel( a );

#define LOG_FILE_LINE_STAMPA "#FILE_LINE#"
#define LOG_FILE_LINE_STAMPW _T( LOG_FILE_LINE_STAMPA )

#ifdef UNICODE
	#define DEBUGPRINTF DebugPrintfW
	#define LOG_FILE_LINE_STAMP LOG_FILE_LINE_STAMPW
	#define LOG_FILE_STRING _T( __FILE__ )
	#define LOG_ENTRY_FMTSTR  _T( "%s\t%s(%d):ENTRY  :\t")
	#define LOG_EXIT_FMTSTR   _T( "%s\t%s(%d):EXIT   :\t")
	#define LOG_TRACE0_FMTSTR _T( "%s\t%s(%d):TRACE0 :\t")
	#define LOG_TRACE1_FMTSTR _T( "%s\t%s(%d):TRACE1 :\t")
	#define LOG_TRACE2_FMTSTR _T( "%s\t%s(%d):TRACE2 :\t")
	#define LOG_TRACE3_FMTSTR _T( "%s\t%s(%d):TRACE3 :\t")

#else
	#define DEBUGPRINTF DebugPrintfA
	#define LOG_FILE_LINE_STAMP LOG_FILE_LINE_STAMPA
	#define LOG_FILE_STRING __FILE__
	#define LOG_ENTRY_FMTSTR  "%s\t%s(%d):ENTRY  :\t"
	#define LOG_EXIT_FMTSTR   "%s\t%s(%d):EXIT   :\t"
	#define LOG_TRACE0_FMTSTR "%s\t%s(%d):TRACE0 :\t"
	#define LOG_TRACE1_FMTSTR "%s\t%s(%d):TRACE1 :\t"
	#define LOG_TRACE2_FMTSTR "%s\t%s(%d):TRACE2 :\t"
	#define LOG_TRACE3_FMTSTR "%s\t%s(%d):TRACE3 :\t"
#endif


#define ENTRY_TRACE if (gs_nDebug&0x01) DEBUGPRINTF( LOG_ENTRY_FMTSTR , LOG_FILE_LINE_STAMP, LOG_FILE_STRING, __LINE__), DEBUGPRINTF
#define EXIT_TRACE  if (gs_nDebug&0x02) DEBUGPRINTF( LOG_EXIT_FMTSTR  , LOG_FILE_LINE_STAMP, LOG_FILE_STRING, __LINE__), DEBUGPRINTF
#define LTRACE      if (gs_nDebug&0x04) DEBUGPRINTF( LOG_TRACE0_FMTSTR, LOG_FILE_LINE_STAMP, LOG_FILE_STRING, __LINE__), DEBUGPRINTF
#define LTRACE1     if (gs_nDebug&0x08) DEBUGPRINTF( LOG_TRACE1_FMTSTR, LOG_FILE_LINE_STAMP, LOG_FILE_STRING, __LINE__), DEBUGPRINTF
#define LTRACE2     if (gs_nDebug&0x10) DEBUGPRINTF( LOG_TRACE2_FMTSTR, LOG_FILE_LINE_STAMP, LOG_FILE_STRING, __LINE__), DEBUGPRINTF
#define LTRACE3     if (gs_nDebug&0x20) DEBUGPRINTF( LOG_TRACE3_FMTSTR, LOG_FILE_LINE_STAMP, LOG_FILE_STRING, __LINE__), DEBUGPRINTF

	
#ifdef _DEBUG
#define DTRACE DEBUGPRINTF
#else
#define DTRACE
#endif


#ifdef __cplusplus
}
#endif


#endif /* LOGING_INCLUDED */
