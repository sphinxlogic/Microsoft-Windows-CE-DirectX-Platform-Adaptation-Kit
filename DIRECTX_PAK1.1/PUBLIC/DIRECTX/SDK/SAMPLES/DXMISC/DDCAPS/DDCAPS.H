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

//
//  Function prototypes
//
BOOL Init(HINSTANCE hInstance, HINSTANCE hPrevInstance,
          LPTSTR lpszCmdLine,  int CmdShow);

int  DoMain(HINSTANCE hInstance);
void CleanUp(void);

LRESULT WINAPI OverlappedWinProc(HWND hWnd, UINT wMsgID,
                                 UINT wParam, LPARAM lParam);
