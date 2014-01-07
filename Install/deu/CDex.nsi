Name "CDex"
Caption "CDex Installation"
ComponentText "Wählen Sie die zu installierenden Komponenten"

OutFile "cdex_setup.exe"
InstallDir $PROGRAMFILES\CDex_150
ShowInstDetails show

icon CDex.ico
UninstallIcon Uninstall.ico
EnabledBitmap cdex1.bmp
DisabledBitmap cdex2.bmp

UninstallText "CDex wird nun deinstalliert. Den NEXT Button zum Fortfahren klicken."
InstallDirRegKey HKEY_LOCAL_MACHINE "Software\CDex" "Instpath"

LicenseText "Bitte vor dem Fortfahren lesen:"
LicenseData "CDex.txt"
SubCaption 0 " "

InstType "CDex Vollständig (empfohlen)"
InstType "Keine Verknüpfungen"

SetOverwrite on

DirText "Zu installierende Dateien auswählen:"
AutoCloseWindow true
SetDatablockOptimize on
SetCompress force


Section "CDex (notwendig)"
  SectionIn 1 2

DetailPrint " - Installation der notwendigen Dateien"

IfFileExists $INSTDIR\CDex.exe exexist
goto noexe
exexist:
  MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_TOPMOST \
  "Eine alte Installation von CDex wurde gefunden. Dies führt normalerweise zu keinem Problem. \
  $\r$\rWollen Sie die alte Installation überschreiben?"\
  IDYES noexe

MessageBox MB_OKCANCEL|MB_ICONSTOP|MB_TOPMOST "Installation fortfahren?" IDCANCEL noexe
Abort

noexe:
  SetOutPath $INSTDIR
  File "CDex\CDex.exe"
  File "CDex\CDRip.DLL"
  File "CDex\Libsndfile.DLL"
  File "CDex\LangDll.DLL"
  File "CDex\CDex.cnt"
  File "CDex\CDex.hlp"
  File "CDex\changes.txt"
  File "CDex\liesmich.txt"
  File "CDex\CDex.exe.manifest"

  WriteUninstaller "$INSTDIR\uninstall.exe"

  DetailPrint " - Installation der Kompressions DLL(s)"
  File "CDex\lame_enc.dll"
  File "CDex\vorbis.dll"
  File "CDex\ogg.dll"
  File "CDex\vorbisenc.dll"
  File "CDex\MP2Enc.DLL"
  File "CDex\WMA8Connect.dll"
  File "CDex\MACDll.dll"

  SetOutPath $INSTDIR\Plugins
  File "CDex\Plugins\in_mp3.dll"
  File "CDex\Plugins\in_vorbis.dll"
  File "CDex\Plugins\in_vorbis.txt"

  SetOutPath $INSTDIR

  DetailPrint " - Adding uninstall information to the registry file"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\CDex" "Instpath" "$INSTDIR"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "DisplayName" "CDex extraction audio"
  WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "UninstallString" '"$INSTDIR\uninstall.exe"'

SectionEnd

Section "Automatische Konfiguration"
  SectionIn 1 

DetailPrint " - Automatische Konfiguration"

IfFileExists $INSTDIR\cdex.ini iniexist
goto noini
iniexist:
  MessageBox MB_YESNO|MB_ICONEXCLAMATION|MB_TOPMOST \
  "Eine vorhandene Konfiguration wurde gefunden.$\r$\rWollen Sie die Konfiguration überschreiben?" \
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

SectionDivider


Section "Desktopverknüpfung erstellen"
  SectionIn 1 

DetailPrint " - Eine Verknüpfung auf dem Desktop erstellen"

  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\CDex.lnk" "$INSTDIR\CDex.exe"
SectionEnd


Section "Startmenu Verknüpfungen erstellen"
  SectionIn 1 

DetailPrint " - Verknüpfungen im Startmenü erstellen"

  CreateDirectory $SMPROGRAMS\CDex
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\CDex\CDex.lnk" "$INSTDIR\CDex.exe"
  CreateShortCut "$SMPROGRAMS\CDex\CDex Help.lnk" "$INSTDIR\CDex.hlp"
  CreateShortCut "$SMPROGRAMS\CDex\CDex Uninstall.lnk" "$INSTDIR\uninstall.exe"
  CreateShortCut "$SMPROGRAMS\CDex\CDex Site.lnk" "http://cdexos.sourceforge.net/"
SectionEnd

Function .onInstSuccess
   MessageBox MB_YESNO|MB_ICONQUESTION|MB_TOPMOST "Herzlichen Glückwunsch! Die CDex Installation ist abgeschlossen.$\r$\rWollen Sie die Liesmich Datei lesen (empfohlen)?" IDNO nohelp
   ExecShell open "$INSTDIR\liesmich.txt"
goto fxend
   nohelp:
	fxend:
FunctionEnd

Section "Uninstall"

DetailPrint " - Die folgenden Dateien wurden gelöscht..."
  Delete /rebootok "$INSTDIR\CDex.exe"
  Delete /rebootok "$INSTDIR\CDRip.DLL"
  Delete /rebootok "$INSTDIR\lame_enc.dll"
  Delete /rebootok "$INSTDIR\libsndfile.dll"
  Delete /rebootok "$INSTDIR\LangDll.DLL"
  Delete /rebootok "$INSTDIR\MP2Enc.DLL"
  Delete /rebootok "$INSTDIR\CDex.cnt"
  Delete /rebootok "$INSTDIR\CDex.hlp"
  Delete /rebootok "$INSTDIR\Changes.txt"
  Delete /rebootok "$INSTDIR\liesmich.txt"
  Delete /rebootok "$INSTDIR\vorbis.DLL"
  Delete /rebootok "$INSTDIR\ogg.DLL"
  Delete /rebootok "$INSTDIR\vorbisenc.DLL"
  Delete /rebootok "$INSTDIR\WMA8Connect.DLL"
  Delete /rebootok "$INSTDIR\MACDll.dll"
  Delete /rebootok "$INSTDIR\CDex.exe.manifest"
  Delete /rebootok "$INSTDIR\Plugins\in_mp3.dll"
  Delete /rebootok "$INSTDIR\Plugins\in_vorbis.dll"
  Delete /rebootok "$INSTDIR\Plugins\in_vorbis.txt"

DetailPrint " - Die Verknüpfungen wurden gelöscht..."
  Delete /rebootok "$DESKTOP\CDex.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex\CDex.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex\CDex Help.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex Uninstall.lnk"
  Delete /rebootok "$SMPROGRAMS\CDex\Internet\CDex Site.lnk"
  rmdir /r "$SMPROGRAMS\CDex"
DetailPrint " - Die Registrierungseinstellungen wurden gelöscht..."
  DeleteRegValue HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "DisplayName"
  DeleteRegValue HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "UninstallString"

MessageBox MB_YESNO|MB_ICONQUESTION|MB_TOPMOST \
"Die Desinstallation ist abgeschlossen. Wollen Sie auch die Konfigurationsdatei löschen? (nicht empfohlen)" \
IDNO nodel
DetailPrint " - The CDex Datei(en) und Einstellungen wurden gelöscht..."
  rmdir /r "$INSTDIR"
  Delete /rebootok "$WINDIR\CDex.ini"
nodel:

DetailPrint " - Deinstallation ist abgeschlossen!"

SectionEnd
