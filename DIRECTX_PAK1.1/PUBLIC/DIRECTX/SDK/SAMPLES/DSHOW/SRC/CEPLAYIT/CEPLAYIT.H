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
//   Simple movie player console application
//

//
// Constants
//
#define SAMPLE_FREQ     100              // sample user input on an interval
#define NAME            TEXT("CEPlayit")
#define TITLE           TEXT("CEPlayit Sample")

#define kMoveAdjust     20
#define kSizeAdjust     20
#define kWidthAdjust    40
#define kHeightAdjust   30

#define kMinWidth       10
#define kMinHeight      10
#define kMaxWidth       640
#define kMaxHeight      480

#define UP              1
#define DOWN            0
#define NUM_QUADRANTS_SET   9

#define VOLUME_FULL     0L
#define VOLUME_SILENCE  -10000L


//
// Macros
//
#define RELEASE(i) {if (i) i->Release(); i = NULL;}

#define JIF(x) if (FAILED(hr=(x))) \
    {RETAILMSG(1, (TEXT("FAILED(0x%x) ") TEXT(#x) TEXT("\n"), hr)); goto CLEANUP;}

#define LIF(x) if (FAILED(hr=(x))) \
    {RETAILMSG(1, (TEXT("FAILED(0x%x) ") TEXT(#x) TEXT("\n"), hr)); }

#define HANDLE_EC(c)                                         \
    case c:                                                  \
        OutputDebugString(TEXT("  ") TEXT(#c) TEXT("\r\n")); \
        break;

#define ISBLANK(x) \
    (((x) == TEXT(' ')) || ((x) == TEXT('\t')) || \
     ((x) == TEXT('\r')) || ((x) == TEXT('\n')))

#define countof(a)    (sizeof(a)/sizeof(*(a)))


//
// Enumerated types
//
enum PLAYSTATE { psRUNNING, psPAUSED, psSTOPPED };
enum VIDEOSIZE { vsDEFAULT, vsFULLSCREEN };


//
// Global data
//
extern IGraphBuilder  *pGB;
extern IMediaControl  *pMC;
extern IMediaSeeking  *pMS;
extern IMediaPosition *pMP;
extern IVideoWindow   *pVW;
extern IBasicVideo    *pBV;
extern IMediaEvent    *pME;
extern IQualProp      *pQP;
extern IBaseFilter    *pBF_OvM;

extern PLAYSTATE g_psCurrent;
extern VIDEOSIZE g_vsCurrent;

extern HWND g_hwndMain;          // Window to receive messages
extern BOOL g_bContinue, g_bLooping, g_bAudioOnly, g_bWindowless;
extern BOOL g_bFullScreenStart, g_bCenterStart, g_bQuadrantStart;
extern BOOL g_bShowingMainWindow, g_bShowEvents;

extern int  g_nMaxX, g_nMaxY;

extern LONG leftSave, topSave, widthSave, heightSave;
extern LONG leftStart, topStart, widthStart, heightStart;

extern WORD g_wInitialQuadrant;


//
// Structures
//
typedef struct _quad
{
    int left;
    int top;
    int width;
    int height;

} QUAD, *PQUAD;

extern QUAD quadrants[NUM_QUADRANTS_SET];


//
// Function prototypes
//
HRESULT PlayMedia(LPTSTR lpszMovie, HINSTANCE hInstance);
HRESULT GetInterfaces(void);
HRESULT ProcessMoveKeys(WPARAM wParam);
HRESULT ToggleMute(void);
HRESULT ToggleFullScreen(void);

void Usage(void);
void CleanupInterfaces(void);
void CheckVisibility(void);
void ResetAudio(void);
void ResetVideo(void);
void ResetVideoMinimum(void);
void ToggleVideoMaximum(void);
void PauseClip(void);
void StopClip(void);
void CenterVideo(void);
void ModifyVideoSize(BOOL bUp);
void SetVideoQuadrant(WORD wQuad);
void ParseOptions(TCHAR *szFile, TCHAR *szCmd, TCHAR *szOpt);
void ProcessStartupOptions(void);
void SetInitialPosition(void);
void ShowCurrentVideoFile(LPTSTR lpszMovie);
void ShowCurrentAudioFile(LPTSTR lpszMovie);

int ParseCommandLine (TCHAR *lpCmdLine);

BOOL DoInit( HINSTANCE hInstance, int nCmdShow, TCHAR *szFile );
BOOL ConvertFilename(TCHAR *szFile, LPTSTR lpCmdLine);
BOOL ReadFromPC(TCHAR *szFile, LPTSTR lpCmdLine);
BOOL DeleteTemporaryFile(void);
