Name "Supporting CDex DLLs for Ogg Vorbis audio encoder/decoder"
OutFile cdex_ogg.exe
CRCCheck on
ComponentText "This will install the supporting CDex DLLs for Ogg Vorbis audio encoder/decoder"
InstType Normal
AutoCloseWindow true
SetOverwrite on
SetDateSave on

InstallDir $PROGRAMFILES\Winamp
InstallDirRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\CDex" "UninstallString"
DirShow show
DirText "The installer has detected the path to CDex. If it is not correct, please change."

Section "Supporting CDex DLLs for Ogg Vorbis audio encoder/decoder"
SectionIn 1
SetOutPath $INSTDIR\Plugins
  File "CDex\Plugins\in_vorbis.dll"
  File "CDex\Plugins\in_vorbis.txt"
SetOutPath $INSTDIR
  File "CDex\vorbis.dll"
  File "CDex\ogg.dll"
  File "CDex\vorbisenc.dll"
SectionEnd
