//-----------------------------------------------------------------------------
// File: WinMain.cpp
//
// Desc: Windows management for DirectShow sample
//
//
// Copyright (c) 1999 Microsoft Corporation. All rights reserved.
//
//-----------------------------------------------------------------------------

#include <windows.h>
#include <strmif.h>
#include <uuids.h>
#include <control.h>
#include <stdio.h>
#include <vfwmsgs.h>
#include <evcode.h>

#include "PlayVideo.h"
#include "PlayAll.h"
#include "mediatypes.h"


// Function prototypes
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

//
// Global data
//
LPTSTR g_pszPath = DEFAULT_SEARCH_PATH;
LPTSTR g_pszFile=0;                     // No default file name
BOOL   g_bUsingSearchPath=FALSE, g_bRecursiveSearch=FALSE;
unsigned int g_nPlayCount=1;


/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

    RetailOutput

Description:

    Simple Retail Output mechanism.  If the function outputs
    the passed-in String and variables to the debug output stream.

Arguments:

    TCHAR *tszDest       - TEXT String to output

    ... (variable arg)   - List of variable arguments to embed in output
                           (similar to printf format)

Return Value:

    None

-------------------------------------------------------------------*/
void RetailOutput(TCHAR *tszErr, ...)
{
    TCHAR tszErrOut[MAX_PATH + 256];

    va_list valist;

    va_start(valist,tszErr);
    wvsprintf(tszErrOut, tszErr, valist);
    OutputDebugString(tszErrOut);
    va_end (valist);
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


void Usage()
{
    RetailOutput(TEXT("Usage: PlayAll [-p search path] [-m mediafile] [-l #loops] [-g #global loops] [-c] [-r] [-e]\r\n")
                 TEXT("       Default search path: \\CD-ROM\\\r\n")
                 TEXT("       Default media: %s\r\n\r\n")
                 TEXT("       Search path and media filename are mutually exclusive!\r\n\r\n")
                 TEXT("       -l specifies number of times to loop each media file.\r\n")
                 TEXT("       -g specifies number of times to loop ENTIRE selection.\r\n")
                 TEXT("       -c centers video at normal size instead of fullscreen.\r\n")
                 TEXT("       -r requests recursive media search (use with -p or with -m wildcards).\r\n")
                 TEXT("       -e displays DirectShow events as they are received.\r\n")
                 TEXT("       WILDCARDS ARE SUPPORTED for media file!\r\n\r\n")
                 TEXT("Important: See the readme.txt for this sample for more information.\r\n"),
                     DEFAULT_MEDIA_FILE);
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


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR pszCmdLine, int nCmdShow)
{
    HRESULT hr;

    RetailOutput(TEXT("\r\nPlayAll starting...\r\n"));

    // Parse command line for options
    if (!ParseCommandLine(pszCmdLine, &g_nPlayCount))
        return(1);

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
            for (int i=0; i<NUM_MEDIA_TYPES; i++)
            {
                PlayMediaType(g_pszPath, &TypeInfo[i]);
            }
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
            g_pszFile = DEFAULT_MEDIA_FILE;
            RetailOutput(TEXT("No media specified. Using default media file: %s\r\n"),
                         DEFAULT_MEDIA_FILE);
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

    // Release COM
    CoUninitialize();

    RetailOutput(TEXT("\r\nPlayAll played a total of %d files.  %d failed to render.\r\n"),
                 g_nFilesPlayed, g_nFilesFailed);
    RetailOutput(TEXT("  %d were audio-only or used unsupported video codecs (Indeo, RLE8, etc.).\r\n\r\n"),
                 g_nAudioOnly);

      return TRUE;
}


//
// Plays all the media of the requested type in a given directory
//
void PlayMediaType(LPTSTR pszRootDir, MEDIA_INFO *pInfo)
{
    HANDLE hFile;
    WIN32_FIND_DATA fd={0};
    TCHAR szFirstFile[MAX_PATH]={0};

    _tcscpy(szFirstFile, pszRootDir);
    _tcscat(szFirstFile, TEXT("\\"));

    _tcscat(szFirstFile, pInfo->pszType);
    RetailOutput(TEXT("Looking for %s files: [%s]\r\n"),
                 pInfo->pszName, szFirstFile);

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
    TCHAR szThisFile[MAX_PATH]={0};

    _tcscpy(szThisFile, pszRootDir);
    _tcscat(szThisFile, TEXT("\\"));
    _tcscat(szThisFile, pfd->cFileName);

    RetailOutput(TEXT("  Reading file [PLEASE WAIT]: %s\r\n"), szThisFile);

    PlayFile(szThisFile, g_nPlayCount);
}


void RecurseFromStruct(WIN32_FIND_DATA *pfd, MEDIA_INFO *pInfo,
                       LPTSTR pszRootDir, LPTSTR szFirstFile)
{
    TCHAR szDirName[MAX_PATH];
    DWORD dwAttributes;

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


