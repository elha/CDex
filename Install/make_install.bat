call clean.bat
"c:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv" /out releasebuild.txt /rebuild Release "..\CDex.sln"call update cdex_170b3 lang_enu Release
del release_install.txt
call update cdex_170b3 lang_enu Release >release_install.txt


"c:\Program Files\Microsoft Visual Studio .NET 2003\Common7\IDE\devenv" /out release_unicode_build.txt /rebuild Release_UNICODE "..\CDex.sln"
call update cdex_170b3 lang_enu Release
call update cdex_170b3_unicode lang_enu Release_UNICODE >release_unicode_install.txt

