@echo off

if not exist %_WINCEROOT%\public\maxdx\internal.bat goto endif_1
call %_WINCEROOT%\public\maxdx\internal.bat
:endif_1

REM make the image size as small as possible
set MAXALL_NOHELP=1
set MAXALL_NOPIE=1
set MAXALL_NOPWORD=1
set MAXALL_NOPMAIL=1
set MAXDX_FULL=

REM turn off default wavedev driver.
set ODO_NOAUDIO=1

if not "%DSHAL_ES1371%"=="1" set DSHAL_WAVEDEV=1

if not "%_TGTPLAT%"=="CEPC" goto end_cepc
    REM only CEPC supports real cursor and USB
    set MAXALL2_NOCURSOR=
    set IMGUSB=
    if "%MAXDX_FULL%"=="1" set IMGUSB=1
    REM turn off default CEPC DDI
    set ODO_NODISPLAY=1
    goto end_cepc
:end_cepc
