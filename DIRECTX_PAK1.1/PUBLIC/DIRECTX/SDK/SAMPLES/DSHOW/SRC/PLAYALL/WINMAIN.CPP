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
//
//   Movie player console application
//
//   Function:
//       Plays a single movie file. Supports recursion, wildcards, and
//       automatic playing of all supported media files for a given system.
//
//       See Usage() or readme.txt for detail.
//

#include <windows.h>
#include <strmif.h>
#include <uuids.h>
#include <control.h>
#include <stdio.h>
#include <vfwmsgs.h>
#include <evcode.h>

#include "PlayAll.h"
#include "PlayVideo.h"
#include "mediatypes.h"


//
// Function prototypes
//
int ParseCommandLine(LPTSTR pszCmdLine, unsigned int *pnRepeats);
void DisplayMediaTypes(void);
void PlayMediaType(LPTSTR pszRootDir, MEDIA_INFO *pInfo);
void PlayFromStruct(WIN32_FIND_DATA *pfd, LPTSTR pszRootDir);
void Usage();
void RecurseFromStruct(WIN32_FIND_DATA *pfd, MEDIA_INFO *pInfo,
                           LPTSTR pszRootDir, LPTSTR szFirstFile);
void PlayWildcardMedia(LPTSTR pszRootDir, LPTSTR lpExtension);
void RecurseWildcardFromStruct(WIN32_FIND_DATA *pfd,
                               LPTSTR pszRootDir, LPTSTR szFirstFile);
void ShowStatistics(void);
void CloseApplication(void);

BOOL DoInit( HINSTANCE hInstance);
long ProcessKey(WORD wParam);
long FAR PASCAL WindowProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

//
// Global data
//
LPTSTR g_pszPath = DEFAULT_SEARCH_PATH;
LPTSTR g_pszFile=0;                     // No default file name
BOOL   g_bUsingSearchPath=FALSE, g_bRecursiveSearch=FALSE;
unsigned int g_nPlayCount=1;
int    g_nInitialMediaType=0;
HANDLE g_hMem=NULL;



int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR pszCmdLine, int nCmdShow)
{
    HRESULT hr;

    RetailOutput(TEXT("\r\nPlayAll starting...\r\n"));

    // Parse command line for options
    if (!ParseCommandLine(pszCmdLine, &g_nPlayCount))
        return(1);

    // Create the main window to allow for keyboard input
    DoInit(hInstance);

    // Initialize COM
    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    if(FAILED(hr))
    {
        RetailOutput(TEXT("Could not initialize COM!  hr=0x%x\r\n"), hr);
        CoUninitialize();
        return FALSE;
    }

    hr = InitDirectShow();
    if(FAILED(hr))
    {
        RetailOutput(TEXT("Failed to initialize DirectShow!  hr=0x%x\r\n"), hr);
        goto Cleanup;
    }

    //
    // If a search path was specified, use it.
    //
    if (g_bUsingSearchPath)
    {
        RetailOutput(TEXT("Using Media base directory: %s\r\n"), g_pszPath);

        // Handle global repeat count
        for (int i=0; i < g_nGlobalRepeats; i++)
        {
            DisplayMediaTypes();

            if (g_nGlobalRepeats > 1)
            {
                RetailOutput(TEXT("============================================\r\n"));
                RetailOutput(TEXT("    Running GLOBAL Pass #%d of %d\r\n"),
                             i+1, g_nGlobalRepeats);
                RetailOutput(TEXT("============================================\r\n"));
            }

            // Begin the search!  Play all specified file types in a directory
            for (int i=g_nInitialMediaType; i<NUM_MEDIA_TYPES; i++)
            {
                PlayMediaType(g_pszPath, &TypeInfo[i]);

                if (g_bShutdown)
                    break;
            }

            // Reset initial media type if command line option set it nonzero
            g_nInitialMediaType = 0;
        }
    }

    //
    // If no search path specified, play the specified media file(s)
    //
    else
    {
        BOOL bUsingWildcard=FALSE;

        // If no media file specified, use default
        if (g_pszFile == 0)
        {
            Usage();
            goto Cleanup;
        }

        if ((_tcsstr(g_pszFile, TEXT("*"))) || (_tcsstr(g_pszFile, TEXT("?"))))
        {
            bUsingWildcard = TRUE;
        }
        else
        {
            //
            // Make sure a directory wasn't specified by accident (like \cd-rom)
            // which should be used with the -p option (path) instead.
            //
            DWORD dwAttributes = GetFileAttributes(g_pszFile);

            if (dwAttributes == 0xFFFFFFFF)
            {
                RetailOutput(TEXT("Error 0x%x reading file attributes!\r\n"),
                             GetLastError());
            }
            else if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                RetailOutput(TEXT("\r\nWhoops! This is a directory. Use '-p' (not '-m') for directory paths!\r\n"));
                goto Cleanup;
            }
        }

        //
        // Play the selected file as many times as requested
        //

        // Handle global repeat count
        for (int i=0; i < g_nGlobalRepeats; i++)
        {
            if (g_nGlobalRepeats > 1)
            {
                RetailOutput(TEXT("============================================\r\n"));
                RetailOutput(TEXT("    Running GLOBAL Pass #%d of %d\r\n"),
                             i+1, g_nGlobalRepeats);
                RetailOutput(TEXT("============================================\r\n"));
            }

            // Does the filename have a wildcard?
            if (bUsingWildcard)
            {
                RetailOutput(TEXT("Wildcard in filename. Searching for: %s\r\n"),
                             g_pszFile);

                PlayWildcardMedia(TEXT("\0"), g_pszFile);
            }
            else
            {
                PlayFile(g_pszFile, g_nPlayCount);
            }
        }
    }

Cleanup:
    // If the DirectShow movie is still playing, stop it
    StopMedia();

    // Release COM interfaces and clean up
    FreeDirectShow();

    PostMessage(g_hwndMain, WM_CLOSE, 0, 0);

    // Release COM
    CoUninitialize();

    ShowStatistics();
    return TRUE;
}


void ShowStatistics(void)
{
    RetailOutput(TEXT("\r\nPlayAll played a total of %d files.  %d failed to render.  %d were skipped.\r\n"),
                 g_nFilesPlayed, g_nFilesFailed, g_nSkipped);
    RetailOutput(TEXT("  %d were audio-only or used unsupported video codecs (Indeo, RLE8, etc.).\r\n\r\n"),
                 g_nAudioOnly);
}


BOOL DoInit( HINSTANCE hInstance)
{
    ATOM     atomWndClass;
    TCHAR    szTitle[MAX_PATH];

    // set up and register window class
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = NAME;
    atomWndClass = RegisterClass( &wc );

    wsprintf(szTitle, TEXT("%s"), NAME);

    // Create a window of zero size that will serve as the sink for
    // keyboard input.  If this media file has a video component, then
    // a second ActiveMovie window will be displayed in which the video
    // will be rendered.  Setting keyboard focus on this application window
    // will allow the user to move the video window around the screen, make
    // it full screen, resize, center, etc. independent of the application
    // window.  If the media file has only an audio component, then this will
    // be the only window created.
    g_hwndMain = CreateWindowEx(
        0,
        NAME,
        szTitle,
        0,  // not visible
        0, 0,
        0, 0,
        NULL,
        NULL,
        hInstance,
        NULL );

    return (g_hwndMain != NULL);
}


long FAR PASCAL WindowProc( HWND hWnd, UINT message,
                           WPARAM wParam, LPARAM lParam )
{
    switch( message )
    {
        // Monitor keystrokes for manipulating video window
        // and program options
        case WM_KEYDOWN:
            ProcessKey(wParam);
            break;

        case WM_CLOSE:
            FreeDirectShow();
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}


long ProcessKey(WORD wParam)
{
    switch( wParam )
    {
        case VK_ESCAPE:
        case VK_F12:
        case 'Q':
        case 'X':
            // Quit
            CloseApplication();
            break;

            // Allow user to skip the current file by pressing SPACE or ENTER
        case VK_SPACE:
        case VK_RETURN:
        case 'S':
        case 'N':
            g_bUserSkip = TRUE;
            break;

        case 'P':
            PauseMedia();
            break;

        case 'M':
            ToggleMute();
            g_bGlobalMute ^= 1;
            break;

        case 'F':
            g_bFullScreen ^= 1;
            RetailOutput(TEXT("Toggling screen mode to %s for next media file.\r\n\r\n"),
                         g_bFullScreen ? TEXT("FULLSCREEN") : TEXT("CENTERED"));
            break;

        case 'T':
            g_bUsingMaxPlayTime ^= 1;
            RetailOutput(TEXT("Toggling maximum play time capability to %s.\r\n\r\n"),
                         g_bUsingMaxPlayTime ? TEXT("ON") : TEXT("OFF"));
            break;

    }

    return 0;
}


void CloseApplication(void)
{
    // Set global flags to interrupt loops, global loops, etc.
    g_bShutdown = TRUE;
    g_bUserSkip = TRUE;
    g_bContinue = FALSE;
    g_nGlobalRepeats = 0;

    RetailOutput(TEXT("PlayAll is exiting (user request).\r\n"));
    FreeDirectShow();
    PostMessage(g_hwndMain, WM_CLOSE, 0, 0);
}


int ParseCommandLine(LPTSTR pszCmdLine, unsigned int *pnRepeats)
{
    // Convert to lowercase for parsing
    pszCmdLine = _tcslwr(pszCmdLine);

    while(*pszCmdLine == TEXT('-'))
    {
        ++pszCmdLine;

        switch(*pszCmdLine)
        {
            case TEXT('p'):                 // set the DShow scan directory
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_bUsingSearchPath = TRUE;
                    g_pszPath = pszCmdLine;
                }
                break;

            case TEXT('m'):                 // specify a media file to loop
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_pszFile = pszCmdLine;
                }
                break;

            case TEXT('o'):                 // specify a media file to omit
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    _tcscpy(g_pszOmitFile, pszCmdLine);
                    LPTSTR pSpace = (LPTSTR) g_pszOmitFile;
                    SKIP_NON_WHITESPACE(pSpace);
                    *pSpace = L'\0';
                    RetailOutput(TEXT("Set to OMIT file %s.\r\n"), g_pszOmitFile);
                }
                break;

            case TEXT('l'):                 // set the number of per-file loops
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    *pnRepeats = _ttoi(pszCmdLine);
                    if (! *pnRepeats)
                    {
                        Usage();
                        return(0);
                    }
                    RetailOutput(TEXT("Set for %d repeats of each media file.\r\n"), *pnRepeats);
                }
                break;

            case TEXT('g'):                 // set the number of GLOBAL loops
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_nGlobalRepeats = _ttoi(pszCmdLine);
                    if (! g_nGlobalRepeats)
                    {
                        Usage();
                        return(0);
                    }
                    RetailOutput(TEXT("Set for %d global repeats of all selected media.\r\n"),
                                 g_nGlobalRepeats);
                }
                break;

            case TEXT('c'):                 // Center instead of fullscreen
                {
                    g_bFullScreen = FALSE;
                    RetailOutput(TEXT("Centering video at default size.\r\n"));
                }
                break;

            case TEXT('f'):                 // Fullscreen
                {
                    g_bFullScreen = TRUE;
                }
                break;

            case TEXT('e'):                 // Display DShow events
                {
                    g_bDisplayEvents = TRUE;
                    RetailOutput(TEXT("Displaying DirectShow events.\r\n"));
                }
                break;

            case TEXT('r'):                 // Recursively search
                {
                    g_bRecursiveSearch = TRUE;
                    RetailOutput(TEXT("Using recursive directory search.\r\n"));
                }
                break;

            case TEXT('k'):                 // set the number of per-file loops
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_nInitialSkipCount = _ttoi(pszCmdLine);
                    if (!g_nInitialSkipCount)
                    {
                        Usage();
                        return(0);
                    }
                    RetailOutput(TEXT("Set to skip the first %d media files.\r\n"), g_nInitialSkipCount);
                }
                break;

            case TEXT('n'):                 // set the min size of files to render
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_uMinFileSize = _ttoi(pszCmdLine);
                    if (! g_uMinFileSize)
                    {
                        Usage();
                        return(0);
                    }
                    RetailOutput(TEXT("Set to skip files smaller than %d bytes.\r\n"), g_uMinFileSize);
                }
                break;

            case TEXT('x'):                 // set the max size of files to render
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_uMaxFileSize = _ttoi(pszCmdLine);
                    if (! g_uMaxFileSize)
                    {
                        Usage();
                        return(0);
                    }
                    RetailOutput(TEXT("Set to skip files larger than %d bytes.\r\n"), g_uMaxFileSize);
                }
                break;

            case TEXT('t'):                 // set the max time (seconds) to play each file
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_nMaxPlayTime = _ttoi(pszCmdLine);
                    if (! g_nMaxPlayTime)
                    {
                        Usage();
                        return(0);
                    }
                    g_bUsingMaxPlayTime = TRUE;
                    RetailOutput(TEXT("Set to play maximum of %d seconds per file.\r\n"), g_nMaxPlayTime);
                }
                break;

            case TEXT('i'):                 // set initial media type to nonzero
                {
                    // Skip over any non-whitespace we ignored from the last token
                    SKIP_NON_WHITESPACE(pszCmdLine);
                    SKIP_WHITESPACE(pszCmdLine);

                    g_nInitialMediaType = _ttoi(pszCmdLine);
                    if (! g_nInitialMediaType)
                    {
                        Usage();
                        return(0);
                    }
                    if (g_nInitialMediaType >= NUM_MEDIA_TYPES)
                    {
                        RetailOutput(TEXT("There are only %d media types.  Please reselect!\r\n"), NUM_MEDIA_TYPES);
                        return(0);
                    }
                    RetailOutput(TEXT("Set to start with media type: %s (%s)\r\n"),
                                 TypeInfo[g_nInitialMediaType].pszName,
                                 TypeInfo[g_nInitialMediaType].pszType);
                }
                break;

            case TEXT('?'):
            case TEXT('h'):
            default:
                {
                    Usage();
                    return 0;
                }
                break;
        }

        // Skip to next token
        // Skip over any non-whitespace we ignored from the last token
        SKIP_NON_WHITESPACE(pszCmdLine);

        if(*pszCmdLine == TEXT('\0'))
        {
            break;
        }
        *pszCmdLine++ = TEXT('\0');

        // Skip over white space between tokens
        while(IS_SPACE(*pszCmdLine))
        {
            ++pszCmdLine;
        }
    }

    return TRUE;
}


void RetailOutput(TCHAR *tszErr, ...)
{
    TCHAR tszErrOut[MAX_PATH + 256];

    va_list valist;

    va_start(valist,tszErr);
    wvsprintf(tszErrOut, tszErr, valist);
    OutputDebugString(tszErrOut);
    va_end (valist);
}


void Usage()
{
    RetailOutput(TEXT("Usage: PlayAll [-p search path] [-m mediafile] [-l #loops] [-g #global loops] [-c] [-r] [-e]\r\n")
                 TEXT("       [-x max file size] [-n min file size] [-k # files to skip] [-i initial type]\r\n\r\n"));

    RetailOutput(TEXT("       WILDCARDS ARE SUPPORTED for media file!\r\n")
                 TEXT("       Search path and media filename are mutually exclusive!\r\n\r\n"));

    RetailOutput(TEXT("       -i specifies optional media type for -p startup (0 to %d).\r\n")
                 TEXT("       -l specifies number of times to loop each media file.\r\n"), NUM_MEDIA_TYPES);

    RetailOutput(TEXT("       -g specifies number of times to loop ENTIRE selection.\r\n")
                 TEXT("       -k specifies optional number of files to skip at startup (for wildcards).\r\n"));

    RetailOutput(TEXT("       -c centers video at normal size instead of fullscreen.\r\n")
                 TEXT("       -r requests recursive media search (use with -p or with -m wildcards).\r\n"));

    RetailOutput(TEXT("       -e displays DirectShow events as they are received.\r\n")
                 TEXT("       -x specifies the max file size to play (upper bound).\r\n"));

    RetailOutput(TEXT("       -n specifies the min file size to play (lower bound).\r\n")
                 TEXT("       -t specifies the max number of seconds to play each file.\r\n\r\n"));

    RetailOutput(TEXT("Important: See the readme.txt for this sample for more information.\r\n"));
}


void DisplayMediaTypes(void)
{
    RetailOutput(TEXT("PlayAll will search for the following media:\r\n"));
    RetailOutput(TEXT("--------------------------------------------\r\n"));

    for (int i=0; i<NUM_MEDIA_TYPES; i++)
    {
        RetailOutput(TEXT("%15s   %s\r\n"),
            TypeInfo[i].pszName, TypeInfo[i].pszType);
    }
    RetailOutput(TEXT("--------------------------------------------\r\n"));
}



//
// Plays all the media of the requested type in a given directory
//
void PlayMediaType(LPTSTR pszRootDir, MEDIA_INFO *pInfo)
{
    HANDLE hFile;
    WIN32_FIND_DATA fd={0};
    TCHAR szFirstFile[MAX_PATH]={0};

    if (g_bShutdown)
        return;

    _tcscpy(szFirstFile, pszRootDir);
    _tcscat(szFirstFile, TEXT("\\"));

    _tcscat(szFirstFile, pInfo->pszType);

    // If we're shutting down, don't bother with reporting next file
    if (!g_bShutdown)
    {
        RetailOutput(TEXT("Looking for %s files: [%s]\r\n"),
                     pInfo->pszName, szFirstFile);
    }

    hFile = FindFirstFile(szFirstFile, &fd);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        PlayFromStruct(&fd, pszRootDir);

        while (FindNextFile(hFile, &fd))
        {
            PlayFromStruct(&fd, pszRootDir);
        }

        FindClose(hFile);
    }

    if (g_bRecursiveSearch)
    {
        // Now look for subdirectories
        _tcscpy(szFirstFile, pszRootDir);
        _tcscat(szFirstFile, TEXT("\\*.*"));

        hFile = FindFirstFile(szFirstFile, &fd);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            RecurseFromStruct(&fd, pInfo, pszRootDir, szFirstFile);

            while (FindNextFile(hFile, &fd))
            {
                RecurseFromStruct(&fd, pInfo, pszRootDir, szFirstFile);
            }

            FindClose(hFile);
        }
    }
}


void PlayFromStruct(WIN32_FIND_DATA *pfd, LPTSTR pszRootDir)
{
    SYSTEMTIME st={0};
    TCHAR szThisFile[MAX_PATH]={0};

    if (g_bShutdown)
        return;

    _tcscpy(szThisFile, pszRootDir);
    _tcscat(szThisFile, TEXT("\\"));
    _tcscat(szThisFile, pfd->cFileName);

    GetLocalTime(&st);
    RetailOutput(TEXT("Reading file [%d/%02d/%02d at %02d:%02d:%02d]:  "),
                 st.wMonth, st.wDay, st.wYear % 100,
                 st.wHour, st.wMinute, st.wSecond);

    RetailOutput(TEXT("%s\r\n"), szThisFile);

    PlayFile(szThisFile, g_nPlayCount);
}


void RecurseFromStruct(WIN32_FIND_DATA *pfd, MEDIA_INFO *pInfo,
                       LPTSTR pszRootDir, LPTSTR szFirstFile)
{
    TCHAR szDirName[MAX_PATH];
    DWORD dwAttributes;

    if (g_bShutdown)
        return;

    _tcscpy(szDirName, pszRootDir);
    _tcscat(szDirName, TEXT("\\"));
    _tcscat(szDirName, pfd->cFileName);

    // Is this a directory?
    dwAttributes = GetFileAttributes(szDirName);

    if (dwAttributes == 0xFFFFFFFF)
    {
        RetailOutput(TEXT("Error 0x%x reading file attributes!\r\n"),
                     GetLastError());
        return;
    }

    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        // Reuse file name memory space
        _tcscpy(szFirstFile, pszRootDir);
        _tcscat(szFirstFile, TEXT("\\"));
        _tcscat(szFirstFile, pfd->cFileName);

        PlayMediaType(szFirstFile, pInfo);
    }
}


//
// Plays all the media of the requested type in a given directory
//
void PlayWildcardMedia(LPTSTR pszRootDir, LPTSTR pszWildcard)
{
    HANDLE hFile;
    WIN32_FIND_DATA fd={0};
    TCHAR szFirstFile[MAX_PATH]={0};
    TCHAR szDir[MAX_PATH]={0};
    LPTSTR pszExtension=0;
    int nLength;

    if (g_bShutdown)
        return;

    _tcscpy(szFirstFile, pszRootDir);
    if (szFirstFile[0] != '\0')
        _tcscat(szFirstFile, TEXT("\\"));
    _tcscat(szFirstFile, pszWildcard);

    // Get file's directory name
    _tcscpy(szDir, szFirstFile);
    nLength = _tcslen(szDir);
    for (int i=nLength-1; i; i--)
    {
        if (szDir[i] == '\\')
        {
            szDir[i] = '\0';
            pszExtension = &szDir[i+1];
            break;
        }
    }

    hFile = FindFirstFile(szFirstFile, &fd);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        PlayFromStruct(&fd, szDir);

        while (FindNextFile(hFile, &fd))
        {
            PlayFromStruct(&fd, szDir);
        }

        FindClose(hFile);
    }


    if (g_bRecursiveSearch)
    {
        // Show some groovy progress output...
        if (pszRootDir[0] != '\0')
        {
            _tcscpy(szFirstFile, szDir);
            _tcscat(szFirstFile, TEXT("\\"));
            _tcscat(szFirstFile, pszWildcard);
            RetailOutput(TEXT("PlayAll: Recursing %s...\r\n"), szFirstFile);
        }

        // Now look for subdirectories
        _tcscpy(szFirstFile, szDir);
        _tcscat(szFirstFile, TEXT("\\*.*"));

        hFile = FindFirstFile(szFirstFile, &fd);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            RecurseWildcardFromStruct(&fd, szDir, pszExtension);

            while (FindNextFile(hFile, &fd))
            {
                RecurseWildcardFromStruct(&fd, szDir, pszExtension);
            }

            FindClose(hFile);
        }
    }
}


void RecurseWildcardFromStruct(WIN32_FIND_DATA *pfd,
                               LPTSTR pszRootDir, LPTSTR szExtension)
{
    TCHAR szDirName[MAX_PATH];
    DWORD dwAttributes;

    if (g_bShutdown)
        return;

    _tcscpy(szDirName, pszRootDir);
    _tcscat(szDirName, TEXT("\\"));
    _tcscat(szDirName, pfd->cFileName);

    // Is this a directory?
    dwAttributes = GetFileAttributes(szDirName);

    if (dwAttributes == 0xFFFFFFFF)
    {
        RetailOutput(TEXT("Error 0x%x reading file attributes!\r\n"),
                     GetLastError());
        return;
    }

    if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        PlayWildcardMedia(szDirName, szExtension);
    }
}


