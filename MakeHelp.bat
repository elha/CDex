@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by CDEX.HPJ. >"hlp\CDex.hm"
echo. >>"hlp\CDex.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\CDex.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\CDex.hm"
echo. >>"hlp\CDex.hm"
echo // Prompts (IDP_*) >>"hlp\CDex.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\CDex.hm"
echo. >>"hlp\CDex.hm"
echo // Resources (IDR_*) >>"hlp\CDex.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\CDex.hm"
echo. >>"hlp\CDex.hm"
echo // Dialogs (IDD_*) >>"hlp\CDex.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\CDex.hm"
echo. >>"hlp\CDex.hm"
echo // Frame Controls (IDW_*) >>"hlp\CDex.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\CDex.hm"
REM -- Make help for Project CDEX


echo Building Win32 Help files
start /wait hcw /C /E /M "hlp\CDex.hpj"
if errorlevel 1 goto :Error
if not exist "hlp\CDex.hlp" goto :Error
if not exist "hlp\CDex.cnt" goto :Error
echo.
if exist Debug\nul copy "hlp\CDex.hlp" Debug
if exist Debug\nul copy "hlp\CDex.cnt" Debug
if exist Release\nul copy "hlp\CDex.hlp" Release
if exist Release\nul copy "hlp\CDex.cnt" Release
echo.
goto :done

:Error
echo hlp\CDex.hpj(1) : error: Problem encountered creating help file

:done
echo.
