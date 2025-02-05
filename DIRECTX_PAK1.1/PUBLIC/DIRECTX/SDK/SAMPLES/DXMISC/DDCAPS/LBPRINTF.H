//=========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
// Copyright (c) 1995 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//=========================================================================

#ifndef __LBPRINTF_INCLUDED__
#define __LBPRINTF_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#define ID_PRINTF_LISTBOX   0xfefe

extern HWND LBCreate( HWND hWnd, HINSTANCE hInst, DWORD pos, BOOL debug );
extern void LBSize( DWORD dwWidth, DWORD dwHeight );
extern void LBClear( void );
extern void __cdecl LBPrintf( LPTSTR fmt, ... );
extern void __cdecl LBPrintfRC( HRESULT rc, LPTSTR fmt, ... );

extern HWND     hWndListBox;

#ifdef __cplusplus
}
#endif

#endif
