@echo off
REM simple install script for Windows CE OS QFE 1,2,3,4,5,6,7,9,10,11 for SH4

REM check for valid installation
if exist %1\public\common\oak\lib\SHx\SH4\ce\retail\nk.lib goto copyfiles
goto usage

:copyfiles
REM copy the files
echo updating OS components with QFEs...
copy /y public\common\oak\lib\SHx\SH4\ce\retail\*.*    %1\public\common\oak\lib\SHx\SH4\ce\retail
copy /y public\common\oak\lib\SHx\SH4\ce\debug\*.*     %1\public\common\oak\lib\SHx\SH4\ce\debug
copy /y public\common\oak\target\SHx\SH4\ce\retail\*.* %1\public\common\oak\target\SHx\SH4\ce\retail
copy /y public\common\oak\target\SHx\SH4\ce\debug\*.*  %1\public\common\oak\target\SHx\SH4\ce\debug
echo done.
goto done

:usage
echo instSH4 _WINCEROOT CPU (ie. instSH4 d:\wince212)
goto done

:done
