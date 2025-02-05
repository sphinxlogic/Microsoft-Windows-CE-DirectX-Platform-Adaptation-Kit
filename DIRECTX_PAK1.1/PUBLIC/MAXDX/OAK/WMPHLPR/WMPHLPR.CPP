//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
// wmphlpr.cpp : Implementation of DLL Exports.

// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f wmphlprps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "wmphlpr.h"
#include "wmphlpr_i.c"
#include "wmphelpr.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_WMPHelper, WMPHelper)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

#ifdef UNDER_CE
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
#else
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
#endif /* UNDER_CE */
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
#ifdef UNDER_CE
        _Module.Init(ObjectMap, (HINSTANCE)hInstance);
#else
        _Module.Init(ObjectMap, hInstance, &LIBID_DXHLPRLib);
        DisableThreadLibraryCalls(hInstance);
#endif /* !UNDER_CE */
    }
    else if (dwReason == DLL_PROCESS_DETACH)
        _Module.Term();
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}



