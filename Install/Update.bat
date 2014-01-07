cls
@echo on
@REM Batch file for making a CDex release version
@REM First parameter specifies the name, like cdex_140b6
@REM Second parameter specifies the language, currently the following
@REM languages are supported:
@REM lang_enu for the english version
@REM lang_deu for the german version
@REM lang_ita for the italian version
@REM lang_esp for the spanish version

@set INSTALL_DIR=%CD%
@set PACOMP=..\..\..\cdex_tools\pacomp\pacomp.EXE
@set UPX=..\..\..\cdex_tools\upx.exe

@set LANG=%2
@SET CDEX_PATH=..\..
@SET LAME_PATH=..\..\..\lame
@set REL_OR_RELUNI=%3



@echo =============================================================
@echo DELETE OLD FILES
@echo =============================================================

del %1.exe /s /q /f ,2>null
del CDex /s /q /f ,2>null
rmdir CDex\Plugins ,2>null
rmdir CDex\lang ,2>null
rmdir CDex ,2>null



@echo =============================================================
@echo COPY FILES
@echo =============================================================

:copy_files

mkdir CDex
cd CDex
mkdir Plugins
mkdir lang

xcopy %CDEX_PATH%\%REL_OR_RELUNI%\*.exe /s
xcopy %CDEX_PATH%\%REL_OR_RELUNI%\*.dll /s

copy %CDEX_PATH%\cdrom_drive_offsets.txt
copy %CDEX_PATH%\encoders\libraries\lame\Dll\lame_enc.dll
copy %CDEX_PATH%\encoders\libraries\libvorbis\dll\vorbis.dll
copy %CDEX_PATH%\encoders\libraries\libvorbis\dll\ogg.dll
copy %CDEX_PATH%\encoders\libraries\libape\MACDll.dll
copy %CDEX_PATH%\encoders\libraries\libFlac\Dll\libFLAC.dll
copy %CDEX_PATH%\MusicBrainz\libmusicbrainz.dll
copy %CDEX_PATH%\WMA8Connect.dll
copy %CDEX_PATH%\CDex.exe.manifest
copy %CDEX_PATH%\lang\*.cdex.lang .\lang
copy ..\CDex.ini
copy %CDEX_PATH%\hlp\cdex.hlp
copy %CDEX_PATH%\hlp\cdex.cnt
copy %CDEX_PATH%\hlp\readme.txt
copy %CDEX_PATH%\hlp\changes.txt
copy %CDEX_PATH%\Plugins\*.* Plugins


goto %2

:lang_esp
ECHO COPY SPANISH SPECIFIC LANGUAGE FILES
set LANG_EXT=_esp
cd ..
copy .\esp\in_mp3.dll .\CDex\Plugins
copy .\esp\in_vorbis.dll .\CDex\Plugins
copy .\esp\CDex.txt
copy .\esp\CDex.nsi

goto lang_end


:lang_ita
ECHO COPY ITALIAN SPECIFIC LANGUAGE FILES
set LANG_EXT=_ita
cd ..
copy .\ita\Leggimi.txt .\CDex
copy .\ita\cdexguida.mht .\CDex
copy .\ita\Leggimi-ITA.txt .\CDex
copy .\ita\CDex.txt
copy .\ita\CDex.nsi

goto lang_end


:lang_deu
ECHO COPY GERMAN SPECIFIC LANGUAGE FILES
set LANG_EXT=_deu
cd ..
copy .\deu\liesmich.txt .\CDex
copy .\deu\CDex.txt
copy .\deu\CDex.nsi


goto lang_end

:lang_enu
ECHO COPY ENGLISH SPECIFIC LANGUAGE FILES
set LANG_EXT=_enu
cd ..
copy .\enu\CDex.txt
copy .\enu\CDex.nsi

goto lang_end


:lang_enu_unicode
ECHO COPY ENGLISH SPECIFIC LANGUAGE FILES FOR ENU UNICODE
set LANG_EXT=_enu

cd ..
copy .\enu\CDex.txt
copy .\enu\CDex_UNICODE.nsi CDex.nsi

goto lang_end

:lang_end

cd CDex

%UPX% -9 *.dll
cd Plugins
%UPX% -9 *.dll
cd %INSTALL_DIR%


C:\Progra~1\NSIS\makensis-bz2.exe /CD CDex.nsi
copy cdex_setup.exe %1%LANG_EXT%.exe


:make_zip

echo =============================================================
echo MAKE ZIP FILE
echo =============================================================

cd CDex
del %INSTALL_DIR%\%1%LANG_EXT%.zip ,2>null
%PACOMP% -c2 -a -p %INSTALL_DIR%\%1%LANG_EXT%.zip *.*
cd %INSTALL_DIR%


goto %LANG_EXT%


:_esp
goto finish

:_deu

%PACOMP% -c2 -a -p ..\%1%LANG_EXT%.zip iesmich.txt
goto finish

:_enu
goto finish

:_ita
%PACOMP% -c2 -a -p %INSTALL_DIR%\%1%LANG_EXT%.zip cdexguida.mht
%PACOMP% -c2 -a -p %INSTALL_DIR%\%1%LANG_EXT%.zip Leggimi-ITA.txt
%PACOMP% -c2 -a -p %INSTALL_DIR%\%1%LANG_EXT%.zip Leggimi.txt
goto finish


:finish

cd %INSTALL_DIR%
