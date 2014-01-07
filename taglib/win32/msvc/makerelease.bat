rem @echo off

SET release=1.3.1
SET toolkit=VCdotNet2003

SET releasedir=taglib-%release%-win32-bin

mkdir %releasedir%

cd %releasedir%
mkdir bin
mkdir lib
mkdir doc
mkdir include
cd ..

rem ***************************************
rem Copy binaries and libs
rem ***************************************
copy %toolkit%\Release\TagLib.dll %releasedir%\bin
copy %toolkit%\Release\TagLib.lib %releasedir%\lib
copy %toolkit%\Debug\TagLibd.dll %releasedir%\bin
copy %toolkit%\Debug\TagLibd.lib %releasedir%\lib

rem ***************************************
rem Copy header files
rem ***************************************
cd ..\..\taglib
for /R . %%F in (*.h) do (
	copy "%%F" ..\win32\msvc\%releasedir%\include
)

rem ***************************************
rem Make and copy documentation
rem ***************************************
cd ..\taglib-api
doxygen taglib.doxygen
copy html\*.* ..\win32\msvc\%releasedir%\doc

cd ..\win32\msvc