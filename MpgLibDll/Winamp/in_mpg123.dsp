# Microsoft Developer Studio Project File - Name="in_mpg123" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 5.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=in_mpg123 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "in_mpg123.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "in_mpg123.mak" CFG="in_mpg123 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "in_mpg123 - Win32 Release" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE "in_mpg123 - Win32 Debug" (based on\
 "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "in_mpg123 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../Plugins"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../" /I "../../MP3Enc" /I "../../MP3Enc/libmp3lame" /I "../../MP3Enc/include" /D "NDEBUG" /D "HAVE_MPGLIB" /D "_WINDOWS" /D "USE_LAYER_2" /D "USE_LAYER_1" /D "WIN32" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "in_mpg123 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../Plugins"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "../" /I "../../MP3Enc" /I "../../MP3Enc/libmp3lame" /I "../../MP3Enc/include" /D "_DEBUG" /D "_WINDOWS" /D "USE_LAYER_2" /D "USE_LAYER_1" /D "HAVE_MPGLIB" /D "WIN32" /D "HAVE_CONFIG_H" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o NUL /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "in_mpg123 - Win32 Release"
# Name "in_mpg123 - Win32 Debug"
# Begin Group "Source File"

# PROP Default_Filter "c,def,rc"
# Begin Source File

SOURCE=..\common.c
# End Source File
# Begin Source File

SOURCE=.\ConfigDlg.c
# End Source File
# Begin Source File

SOURCE=..\dct64_i386.c
# End Source File
# Begin Source File

SOURCE=..\decode_i386.c
# End Source File
# Begin Source File

SOURCE=.\httpstream.c
# End Source File
# Begin Source File

SOURCE=.\in_mpg123.c
# End Source File
# Begin Source File

SOURCE=.\in_mpg123.rc
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.c
# End Source File
# Begin Source File

SOURCE=..\interface.c
# End Source File
# Begin Source File

SOURCE=..\layer1.c
# End Source File
# Begin Source File

SOURCE=..\layer2.c
# End Source File
# Begin Source File

SOURCE=..\layer3.c
# End Source File
# Begin Source File

SOURCE=..\mpglib_error.c
# End Source File
# Begin Source File

SOURCE=..\tabinit.c
# End Source File
# Begin Source File

SOURCE=.\VbrTag.c
# End Source File
# End Group
# Begin Group "Include Files"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=..\common.h
# End Source File
# Begin Source File

SOURCE=.\ConfigDlg.h
# End Source File
# Begin Source File

SOURCE=..\dct64_i386.h
# End Source File
# Begin Source File

SOURCE=..\decode_i386.h
# End Source File
# Begin Source File

SOURCE=.\httpstream.h
# End Source File
# Begin Source File

SOURCE=..\huffman.h
# End Source File
# Begin Source File

SOURCE=.\in_mpg123.h
# End Source File
# Begin Source File

SOURCE=.\InfoDlg.h
# End Source File
# Begin Source File

SOURCE=..\interface.h
# End Source File
# Begin Source File

SOURCE=..\l2tables.h
# End Source File
# Begin Source File

SOURCE=..\layer1.h
# End Source File
# Begin Source File

SOURCE=..\layer2.h
# End Source File
# Begin Source File

SOURCE=..\layer3.h
# End Source File
# Begin Source File

SOURCE=..\mpg123.h
# End Source File
# Begin Source File

SOURCE=..\mpglib.h
# End Source File
# Begin Source File

SOURCE=..\mpglib_error.h
# End Source File
# Begin Source File

SOURCE=..\tabinit.h
# End Source File
# Begin Source File

SOURCE=.\VbrTag.h
# End Source File
# End Group
# End Target
# End Project
