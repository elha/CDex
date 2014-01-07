cls
echo on
REM Batch file for making a CDex supporting packages
REM first parameter specifies the language, currently the following
REM languages are supported:
REM lang_enu for the english version
REM lang_deu for the german version (not yet supported)
REM lang_ita for the italian version (not yet supported)
REM lang_esp for the spanish version (not yet supported)

set LANG=%1

goto delete_files:

:delete_files

del CDex /s /q
rmdir CDex\Plugins
rmdir CDex

mkdir CDex

:copy_files

cd CDex
mkdir Plugins


:make_exe
del %1.exe

SET CDEX_PATH=..\..
SET LAME_PATH=..\..\..\lame

copy %CDEX_PATH%\encoders\libraries\libvorbis\dll\vorbis.dll
copy %CDEX_PATH%\encoders\libraries\libvorbis\dll\ogg.dll
copy %CDEX_PATH%\encoders\libraries\libvorbis\dll\vorbisenc.dll
copy %CDEX_PATH%\encoders\libraries\libape\MACDll.dll
copy %CDEX_PATH%\encoders\libraries\libnttvqf\dll\tvqenc.dll
copy %CDEX_PATH%\WMA8Connect.dll

copy %CDEX_PATH%\Plugins\*.* Plugins

cd ..


goto %1

:lang_esp
ECHO COPY SPANISH SPECIFIC LANGUAGE FILES
copy .\enu\*.nsi

goto lang_end


:lang_ita
ECHO COPY ITALIAN SPECIFIC LANGUAGE FILES
copy .\enu\*.nsi

goto lang_end


:lang_deu
ECHO COPY GERMAN SPECIFIC LANGUAGE FILES
copy .\enu\*.nsi

goto lang_end

:lang_enu
ECHO COPY ENGLISH SPECIFIC LANGUAGE FILES
copy .\enu\*.nsi

goto lang_end

:lang_end


C:\Progra~1\NSIS\makensis.exe /CD plugin_ape.nsi
C:\Progra~1\NSIS\makensis.exe /CD plugin_ogg.nsi
C:\Progra~1\NSIS\makensis.exe /CD plugin_vqf.nsi


