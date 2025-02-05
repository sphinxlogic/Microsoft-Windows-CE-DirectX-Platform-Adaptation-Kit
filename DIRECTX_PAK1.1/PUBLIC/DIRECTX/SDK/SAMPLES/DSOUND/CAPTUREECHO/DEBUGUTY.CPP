
//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 2000 Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;

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
