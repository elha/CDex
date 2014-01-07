Name "Supporting CDex DLLs for NTT VQF Audio encoder/decoder"
OutFile cdex_vqf.exe
CRCCheck on
ComponentText "This will install the supporting CDex DLLs for NTT VQF Audio encoder/decoder"
InstType Normal
AutoCloseWindow true
SetOverwrite on
SetDateSave on

InstallDir $PROGRAMFILES\Winamp
InstallDirRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "UninstallString"
DirShow show
DirText "The installer has detected the path to CDex. If it is not correct, please change."

Section "Supporting CDex DLLs for NTT VQF Audio encoder/decoder"
SectionIn 1
SetOutPath $INSTDIR\Plugins
  File "CDex\Plugins\in_vqf.dll"
  File "CDex\Plugins\tvqdec.dll"
  File "CDex\Plugins\in_vqf.txt"
SetOutPath $INSTDIR
  File "CDex\tvqenc.dll"
SectionEnd
