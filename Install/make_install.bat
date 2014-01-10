REM Build
"C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe" /out release_unicode_build.txt /rebuild Release_UNICODE "..\CDex.sln"

REM Create Installer/Archive
call update cdex_171 lang_enu Release_UNICODE

