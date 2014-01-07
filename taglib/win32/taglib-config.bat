@echo off
setlocal

Rem Adjust install prefix
set prefix=C:\Programme\Microsoft Visual Studio 8\VC

Rem Comment out if you're using mingw
set compiler=msvc

Rem No changes should be needed below this line


if "%1" == "" goto usage
set exec_prefix=%prefix%
set libdir=%exec_prefix%\lib
set includedir=%prefix%\include
set flags=

:loop
	if "%1"=="" goto printresult

	if "%1"=="--libs" (
		if %compiler%==msvc (
		set flags=%flags% tag.lib
		goto nextopt
		)
		set flags=%flags% -L%libdir% -ltag
		goto nextopt
	)

	if "%1"=="--cflags" (
		set flags=%flags% -I%includedir%\taglib
		goto nextopt
	)

	if "%1"=="--version" (
		echo 1.4
		goto nextopt
	)

	if "%1"=="--prefix" (
		echo %prefix%
		goto nextopt
	)

	echo %0: unknown option %1
	goto usage


:nextopt
	shift /1
	goto loop


:printresult
	if not "%flags%"=="" echo %flags%
	goto end


:usage
	echo usage: %0 [OPTIONS]
	echo options:
	echo    [--libs]
	echo    [--cflags]
	echo    [--version]
	echo    [--prefix]


:end
	endlocal
