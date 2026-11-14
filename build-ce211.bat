@echo off
setlocal

set "ROOT=%~dp0"
set "SDK=C:\evc4\sdk\STANDARDSDK_420"
set "BIN=C:\evc4\wince\bin"
set "OUT=%ROOT%build\ce211"
cd /d "%ROOT%"

if not exist "%BIN%\clarm.exe" (
    echo EVC4 ARM compiler not found at %BIN%\clarm.exe
    exit /b 1
)
if not exist "%SDK%\Include\Armv4\windows.h" (
    echo Standard SDK 4.2 ARMv4 headers not found at %SDK%
    exit /b 1
)
if not exist "%OUT%" mkdir "%OUT%"
del /q "%OUT%\*.obj" "%OUT%\*.res" "%OUT%\PocketPuTTY.exe" 2>nul

for %%F in (*.c) do (
    echo Compiling %%F
    "%BIN%\clarm.exe" /nologo /c /W3 /O2 /MC /GF /I "%ROOT%." /I "%SDK%\Include\Armv4" /I "%SDK%\Include" /D _WIN32_WCE=211 /D UNDER_CE=211 /D STANDARDSDK_420 /D ARM /D _ARM_ /D ARMV4 /D NDEBUG /D UNICODE /D _UNICODE /D NO_WINRESRC_H /D WIN32 /D WIN32_PLATFORM_HPCPRO /Fo"%OUT%\%%~nF.obj" "%%F"
    if errorlevel 1 exit /b 1
)

rem EVC4's SDK omits winver.h; the CE resource script uses no version fields.
if exist "%ROOT%winver.h" goto have_winver
type nul > "%ROOT%winver.h"
set "CREATED_WINVER=1"
:have_winver
"%BIN%\rc.exe" /i "%ROOT%." /i "%SDK%\Include\Armv4" /i "%SDK%\Include" /l 0x409 /d NDEBUG /d ARM /d _ARM_ /d ARMV4 /d UNDER_CE=211 /d _WIN32_WCE=211 /d UNICODE /d _UNICODE /d STANDARDSDK_420 /d WIN32_PLATFORM_HPCPRO /d NO_WINRESRC_H /fo "%OUT%\wce_res.res" "%ROOT%wce_res.rc"
if errorlevel 1 goto resource_failed
if defined CREATED_WINVER del "%ROOT%winver.h"

> "%OUT%\link.rsp" echo /nologo
>>"%OUT%\link.rsp" echo /base:0x00010000
>>"%OUT%\link.rsp" echo /stack:0x20000,0x1000
>>"%OUT%\link.rsp" echo /entry:WinMainCRTStartup
>>"%OUT%\link.rsp" echo /nodefaultlib:libc.lib
>>"%OUT%\link.rsp" echo /subsystem:windowsce,2.11
>>"%OUT%\link.rsp" echo /align:4096
>>"%OUT%\link.rsp" echo /MACHINE:ARM
>>"%OUT%\link.rsp" echo /out:"%OUT%\PocketPuTTY.exe"
for %%F in ("%OUT%\*.obj") do >>"%OUT%\link.rsp" echo "%%~fF"
>>"%OUT%\link.rsp" echo "%OUT%\wce_res.res"
>>"%OUT%\link.rsp" echo "%SDK%\Lib\Armv4\commctrl.lib"
>>"%OUT%\link.rsp" echo "%SDK%\Lib\Armv4\coredll.lib"
>>"%OUT%\link.rsp" echo "%SDK%\Lib\Armv4\winsock.lib"
>>"%OUT%\link.rsp" echo "%SDK%\Lib\Armv4\corelibc.lib"
>>"%OUT%\link.rsp" echo "%SDK%\Lib\Armv4\uuid.lib"

"%BIN%\link.exe" @"%OUT%\link.rsp"
if errorlevel 1 exit /b 1
echo Built %OUT%\PocketPuTTY.exe
exit /b 0

:resource_failed
if defined CREATED_WINVER del "%ROOT%winver.h"
exit /b 1
