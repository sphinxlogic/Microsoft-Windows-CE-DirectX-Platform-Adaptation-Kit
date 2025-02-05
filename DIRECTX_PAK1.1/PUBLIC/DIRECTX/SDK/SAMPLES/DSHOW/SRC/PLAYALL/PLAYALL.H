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

//
// Constants
//
#define SAMPLE_FREQ     100              // sample user input on an interval
#define NAME            TEXT("PlayAll")
#define TITLE           TEXT("PlayAll Sample")
#define kMaxWidth       640
#define kMaxHeight      480

#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L

//
// Macros
//
#define RELEASE(i) {if (i) i->Release(); i = NULL;}

#define JIF(x) if (FAILED(hr=(x))) \
    {RETAILMSG(1, (TEXT("FAILED(0x%x) ") TEXT(#x) TEXT("\n"), hr)); goto CLEANUP;}

#define HANDLE_EC(c) \
    case c: \
        OutputDebugString(TEXT("  ") TEXT(#c) TEXT("\r\n")); \
        break;

#define ISBLANK(x) \
    (((x) == TEXT(' ')) || ((x) == TEXT('\t')) || \
     ((x) == TEXT('\r')) || ((x) == TEXT('\n')))

#define countof(a)    (sizeof(a)/sizeof(*(a)))

#define IS_SPACE(c)   ((c) == TEXT(' ') || (c) == TEXT('\r') || (c) == TEXT('\n') || (c) == TEXT('\t'))
#define SKIP_NON_WHITESPACE(psz) {  while(!IS_SPACE(*psz) && (*psz != TEXT('\0')))  ++psz; }
#define SKIP_WHITESPACE(psz)     {  while(IS_SPACE(*psz)  && (*psz != TEXT('\0')))  ++psz; }

//
// Enumerated types
//
enum PLAYSTATE { psRUNNING, psPAUSED, psSTOPPED };
enum VIDEOSIZE { vsDEFAULT, vsFULLSCREEN };

//
// Global data
//
extern IGraphBuilder *pGB;
extern IMediaSeeking *pMS;
extern IMediaControl *pMC;
extern IMediaEvent   *pME;
extern IBasicVideo   *pBV;
extern IVideoWindow  *pVW;

extern PLAYSTATE g_psCurrent;
extern VIDEOSIZE g_vsCurrent;

extern HWND g_hwndMain;          // Window to receive messages

extern int  g_nMaxX, g_nMaxY;
extern int  g_nInitialSkipCount, g_nSkipped;
extern int  g_nFilesPlayed, g_nFilesFailed, g_nAudioOnly, g_nGlobalRepeats;

extern unsigned int g_uMaxFileSize, g_uMinFileSize, g_nMaxPlayTime;

extern BOOL g_bContinue, g_bLooping, g_bAudioOnly;
extern BOOL g_bFullScreenStart, g_bCenterStart, g_bUsingMaxPlayTime;
extern BOOL g_bUserSkip, g_bShutdown;
extern BOOL g_bFullScreen, g_bDisplayEvents, g_bGlobalMute;

extern DWORD g_dwInitialFreeMem, g_dwInitialFragments;

extern TCHAR  g_pszOmitFile[128];


//
// Function prototypes
//
HRESULT PlayMedia(LPTSTR lpszMovie, HINSTANCE hInstance);
HRESULT GetInterfaces(void);
HRESULT PauseMedia(void);
HRESULT ToggleMute(void);
void Usage(void);
void CleanupInterfaces(void);
void CheckVisibility(void);
void ToggleFullscreen(void);
void ResetAudio(void);
void ResetVideo(void);
void ParseOptions(TCHAR *szFile, TCHAR *szCmd, TCHAR *szOpt);
void ProcessStartupOptions(void);
void SetInitialPosition(void);
BOOL DoInit( HINSTANCE hInstance, int nCmdShow, TCHAR *szFile );
BOOL ConvertFilename(TCHAR *szFile, LPTSTR lpCmdLine);
int SetVideoFocus (LPTSTR lpCmdLine);
int ParseCommandLine (TCHAR *lpCmdLine);
