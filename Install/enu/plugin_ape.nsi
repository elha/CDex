Name "Monkey's APE CDex plugins"
OutFile cdex_ape.exe
CRCCheck on
ComponentText "This will install the supporting CDex DLLs for Monkey's APE audio format."
InstType Normal
AutoCloseWindow true
SetOverwrite on
SetDateSave on

InstallDir $PROGRAMFILES\Winamp
InstallDirRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "UninstallString"
DirShow show
DirText "The installer has detected the path to CDex. If it is not correct, please change."

Section "Supporting CDex DLLs for Monkey's APE Audio format"
SectionIn 1
SetOutPath $INSTDIR\Plugins
  File "CDex\Plugins\in_ape.dll"
SetOutPath $INSTDIR
  File "CDex\MACDll.dll"
SectionEnd
