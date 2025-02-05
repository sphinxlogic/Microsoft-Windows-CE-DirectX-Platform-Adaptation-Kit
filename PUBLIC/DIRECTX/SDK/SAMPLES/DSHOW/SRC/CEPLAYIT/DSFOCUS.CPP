//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 1999  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
//
//   Finds the instance of CEPlayit running a requested filename
//

#include <windows.h>
#include <strmif.h>
#include <uuids.h>
#include <control.h>
#include <stdio.h>
#include <vfwmsgs.h>
#include <evcode.h>
#include <amvideo.h>

#include "ceplayit.h"

// Function prototypes
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void SetNewFocus(HWND hwndApp);

// Global data
static int nInstances = 0;


//
// Main program code
//
int SetVideoFocus ( LPTSTR lpCmdLine )
{
    HWND hwndApp=0;
    TCHAR szTitle[MAX_PATH]={0};

    // If no filename passed in, just enumerate the current instances of CEPlayit
    if (!lstrcmpi(lpCmdLine, TEXT("")))
    {
        OutputDebugString(TEXT("No parameter.  Listing player instances...\r\n"));
        nInstances = 0;

        EnumWindows(EnumWindowsProc, (LPARAM) 0);

        if (!nInstances)
            OutputDebugString(TEXT("No player instances found.\r\n"));
        else
            RETAILMSG(1, (TEXT("%d player instance(s) found.\r\n"), nInstances));
        return 0;
    }

    RETAILMSG(1, (TEXT("CEPlayit: Searching for instance of %s playing file %s.\r\n"),
              NAME, lpCmdLine));

    // Look for media filename as passed in
    wsprintf(szTitle, TEXT("%s: %s"), NAME, lpCmdLine);

    hwndApp = FindWindow(NAME, szTitle);
    if (hwndApp)
    {
        SetNewFocus(hwndApp);
        return 0;
    }

    // Try \windows\<filename>
    wsprintf(szTitle, TEXT("%s: \\windows\\%s"), NAME, lpCmdLine);

    hwndApp = FindWindow(NAME, szTitle);
    if (hwndApp)
    {
        SetNewFocus(hwndApp);
        return 0;
    }

    // Try \<filename>
    wsprintf(szTitle, TEXT("%s: \\%s"), NAME, lpCmdLine);

    hwndApp = FindWindow(NAME, szTitle);
    if (hwndApp)
    {
        SetNewFocus(hwndApp);
        return 0;
    }

    RETAILMSG(1, (TEXT("CEPlayit: %s not found.\r\n"), lpCmdLine));
    return 0;
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    TCHAR szTitle[MAX_PATH]={0}, *szTarget = (TCHAR *)lParam;
    TCHAR szClassName[128];

    GetClassName(hwnd, szClassName, 128);

    if (!lstrcmpi(szClassName, NAME))
    {
        if (GetWindowText(hwnd, szTitle, MAX_PATH))
        {
            nInstances++;
            RETAILMSG(1, (TEXT("   CEPlayit: Title=%s\r\n"), szTitle));

            // Was a title passed in?  If so, set focus using callback window handle
            if (lParam != 0)
            {
                if (!lstrcmpi(szTitle, szTarget))
                {
                    SetNewFocus(hwnd);
                    return FALSE;
                }
            }
        }
        else
        {
            RETAILMSG(1, (TEXT("CEPlayit: Callback failed to read title!\r\n")));
            return FALSE;
        }
    }

	return TRUE;
}


void SetNewFocus(HWND hwndApp)
{
    RETAILMSG(1, (TEXT("CEPlayit: Setting focus...\r\n")));

    SetFocus(hwndApp);
    SetForegroundWindow(hwndApp);
}