Name "CDex"
Caption "CDex Installation"
ComponentText "Select the components to be installed"

OutFile "cdex_setup.exe"
InstallDir "$PROGRAMFILES\CDex_170b3"
ShowInstDetails show

#icon CDex.ico
#UninstallIcon Uninstall.ico
#SetBrandingImage /RESIZETOFIT cdex1.bmp
#DisabledBitmap cdex2.bmp

UninstallText "Deinstall CDex, click on the NEXT button to continue "
#InstallDirRegKey HKEY_LOCAL_MACHINE "Software\CDex" "Instpath"


LicenseText "Please read this before continuing:" "Next"
LicenseData "CDex.txt"
SubCaption 0 " "

InstType "CDex Full (recommended)"
InstType "CDex Full, No Shortcuts"

SetOverwrite on

DirText "Select a file(s) to install:"
AutoCloseWindow true
SetDatablockOptimize on
SetCompress force


Section "CDex (essential)"
  SectionIn 1 2

DetailPrint " - Installation of the files base"

IfFileExists $INSTDIR\CDex.exe exexist
goto noexe
exexist:
  MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_TOPMOST \
  "An old installation of CDex was detected. This poses normally no problem, \
  $\r$\rDo you want to overwrite the old installation?"\
  IDYES noexe

MessageBox MB_OKCANCEL|MB_ICONSTOP|MB_TOPMOST "Continue installation ?" IDCANCEL noexe
Abort

noexe:
  SetOutPath $INSTDIR
  DetailPrint " - Installation of the base files"
  File "CDex\CDex.*"
  File "CDex\*.DLL"
  File "CDex\*.txt"

  WriteUninstaller "$INSTDIR\uninstall.exe"

  DetailPrint " - Installation of the compression DLL(s)"

  File "CDex\lame_enc.dll"
  File "CDex\vorbis.dll"
  File "CDex\ogg.dll"
  File "CDex\MP2Enc.DLL"
  File "CDex\WMA8Connect.dll"
  File "CDex\MACDll.dll"
  File "CDex\libFLAC.dll"

  DetailPrint " - Installation of language packs"
  SetOutPath $INSTDIR\lang
  File "CDex\lang\*.lang"

  DetailPrint " - Installation of the WinAmp Input Plugins"
  SetOutPath $INSTDIR\Plugins
  File "CDex\Plugins\*.dll"

  SetOutPath $INSTDIR

  DetailPrint " - Adding uninstall information to the registry file"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\CDex" "Instpath" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "DisplayName" "CDex extraction audio"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "UninstallString" '"$INSTDIR\uninstall.exe"'
SectionEnd

Section "Automatic Configuration"
  SectionIn 1 

DetailPrint " - Configuration"

IfFileExists $INSTDIR\cdex.ini iniexist
goto noini
iniexist:
  MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_TOPMOST \
  "An existing configuration file was found.$\r$\rDo you want to delete the old configuration file?" \
  IDNO noauto
delete $INSTDIR\cdex.ini

noini:
  ReadRegStr $0 HKEY_CURRENT_USER "Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders" "Personal"
  SetOutPath "$INSTDIR"
  File "CDex\CDex.INI"

  WriteINIStr "$INSTDIR\CDex.ini" "General" "MP3OutputDir" "$0\My Music\MP3"
  WriteINIStr "$INSTDIR\CDex.ini" "General" "CnvOutputDir" "$0\My Music\MP3"
  WriteINIStr "$INSTDIR\CDex.ini" "General" "PlayDir" "$0\My Music"
  WriteINIStr "$INSTDIR\CDex.ini" "General" "WAVInputDir" "$0\My Music\WAV"
  WriteINIStr "$INSTDIR\CDex.ini" "General" "CDDBPath" "$0\My Music\CDDB"
  WriteINIStr "$INSTDIR\CDex.ini" "General" "MP3InputDir" "$0\My Music\MP3"
noauto:
SectionEnd

# SectionDivider


Section "Creating Desktop short cut"
  SectionIn 1 

DetailPrint " - Creating desktop short cut"

  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\CDex.lnk" "$INSTDIR\CDex.exe"
SectionEnd


Section "Creating Start Menu short cuts"
  SectionIn 1 

DetailPrint " - Creating Start Menu short cut(s)"

  CreateDirectory $SMPROGRAMS\CDex
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\CDex\CDex.lnk" "$INSTDIR\CDex.exe"
  CreateShortCut "$SMPROGRAMS\CDex\CDexPlayer.lnk" "$INSTDIR\CDex.exe" /PLAY
  CreateShortCut "$SMPROGRAMS\CDex\CDex Help.lnk" "$INSTDIR\CDex.hlp"
  CreateShortCut "$SMPROGRAMS\CDex\CDex Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortCut "$SMPROGRAMS\CDex\CDex Site.lnk" "http://cdexos.sourceforge.net"
SectionEnd

Function .onInstSuccess
   MessageBox MB_YESNO|MB_ICONQUESTION|MB_TOPMOST "Congratulations ! CDex installation has been completed.$\r$\rView the readme file (recommended) ?" IDNO nohelp
   ExecShell open "$INSTDIR\readme.txt"
goto fxend
   nohelp:
	fxend:
FunctionEnd

Section "Uninstall"

DetailPrint " - The following files are deleted ..."
  Delete /rebootok "$INSTDIR\*.exe"
  Delete /rebootok "$INSTDIR\*.dll"
  Delete /rebootok "$INSTDIR\CDex.*"
  Delete /rebootok "$INSTDIR\lang\*"
  Delete /rebootok "$INSTDIR\*.txt"
  Delete /rebootok "$INSTDIR\CDex.exe.manifest"
  Delete /rebootok "$INSTDIR\Plugins\*"

DetailPrint " - The short cut(s) are deleted ..."
  Delete /rebootok "$DESKTOP\CDex.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex\CDex.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex\CDexPlayer.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex\CDex Help.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex Uninstall.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex\Internet\CDex Site.lnk"
  rmdir /r "$SMPROGRAMS\CDex"
DetailPrint " - The registry entries are deleted ..."
  DeleteRegValue HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "DisplayName"
  DeleteRegValue HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "UninstallString"
  DeleteRegValue HKEY_LOCAL_MACHINE "Software\CDex" "Instpath"

MessageBox MB_YESNO|MB_ICONQUESTION|MB_TOPMOST \
"The desinstallation is finished. Do you wish to erase all the contents of the file of CDex as well as the configuration files? (not recommended)" \
IDNO nodel
DetailPrint " - The CDex file(s) and preferences are deleted ..."
  rmdir /r "$INSTDIR"
  Delete /rebootok "$WINDIR\CDex.ini"
nodel:

DetailPrint " - Uninstall has been finished !"

SectionEnd
