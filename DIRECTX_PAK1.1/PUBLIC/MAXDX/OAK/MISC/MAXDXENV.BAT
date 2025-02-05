@echo off
if "%_WINCEROOT%" == "" goto noroot
set DDHAL_IGS5000=1
set DSWAVE_IGS5000=1
echo maxdx build window: default setting are for IGS5000 display and audio driver
call %_WINCEROOT%\public\common\oak\misc\wince.bat %1 %2 %3 %4 %5 %6 %7 %8 %9

goto done

:noroot
echo You did not set the required environment variables correctly.
echo _WINCEROOT *must* be set to the fully qualified path to your 
echo platform builder installation (e.g. d:\wince212)

:done
