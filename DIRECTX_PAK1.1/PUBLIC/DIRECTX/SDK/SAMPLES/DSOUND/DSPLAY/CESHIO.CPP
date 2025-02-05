//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
// File: ceshio.cpp
//
// Desc: Wrapper functions for CESH file I/O
//

#define WINCEMACRO

#include <windows.h>
#include "ceshio.h"


int
pp_open(WCHAR *pFileName, UINT uiMode)
{
    return U_ropen(pFileName, uiMode);
}

int
pp_read(int h, BYTE * pBuffer, int iCount)
{
    return U_rread(h, pBuffer, iCount);
}

int
pp_write(int h, BYTE * pBuffer, int iCount)
{
    return U_rwrite(h, pBuffer, iCount);
}

int
pp_lseek(int h, int iOffset, int iMode)
{
    return U_rlseek(h, iOffset, iMode);
}

int
pp_close(int h)
{
    return U_rclose(h);
}
