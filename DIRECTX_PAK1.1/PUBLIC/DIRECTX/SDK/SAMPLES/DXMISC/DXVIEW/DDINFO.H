/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: ddinfo.h

    PURPOSE: DirectX Device Viewer DDraw-specific header file

****************************************************************************/


//
// Macros
//
#define MAKEMODE(xres,yres,bpp) (((DWORD)xres << 20) | ((DWORD)yres << 8) | bpp)
#define GETXRES(mode)           (int)((mode >> 20) & 0x0FFF)
#define GETYRES(mode)           (int)((mode >> 8)  & 0x0FFF)
#define GETCRES(mode)           (int)((mode >> 0)  & 0x00FF)

//
// Function prototypes
//
IDirectDraw * DDCreate(GUID *pid);

BOOL VidMemPrint(long lParam1, long lParam2, PRINTCBINFO *lpInfo);
VOID VidMem(long lParam1, long foo2);

HRESULT CALLBACK DDEnumCallBack(GUID *pid, LPTSTR lpDriverDesc,
                                LPTSTR lpDriverName, LPVOID lpContext);

HRESULT CALLBACK DDEnumCallBack2(GUID *pid, LPTSTR lpDriverDesc,
                                 LPTSTR lpDriverName, LPVOID lpContext,
                                 HMONITOR hMonitor);

//
//  Global data
//
extern IDirectDraw  *lpDD;                 // DirectDraw object
extern IDirectDraw4 *lpDD4;                // DirectDraw object
extern GUID * ddid;

