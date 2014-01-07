REM set PRGNAME=.\Debug\CDex
REM set DLLNAME=.\MP2Enc\Dll\Debug\MP2Enc

set PRGNAME=CDex
set DLLNAME=..\MP2Enc\Dll\Debug\MP2Enc


call c:\vcvars32.bat

copy %DLLNAME%.dll %DLLNAME%.save
PREP /OM %DLLNAME%.dll

copy %DLLNAME%._ll %DLLNAME%.dll

PROFILE /I %DLLNAME% /O %DLLNAME% %PRGNAME%

copy %DLLNAME%.save %DLLNAME%.dll

PREP /M  %DLLNAME%
PLIST %DLLNAME%