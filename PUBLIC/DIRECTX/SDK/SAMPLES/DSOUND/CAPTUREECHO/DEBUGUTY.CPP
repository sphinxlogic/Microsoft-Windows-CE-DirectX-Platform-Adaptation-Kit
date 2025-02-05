#include "main.h"
#include "DebugUty.h"

void dbgout(LPCTSTR pszFormat, ...)
{
    va_list args;
    va_start(args,pszFormat);
    TCHAR szBuffer[1024];
    _vstprintf(szBuffer, pszFormat, args);
    OutputDebugString(szBuffer);
    va_end(args);		
}
