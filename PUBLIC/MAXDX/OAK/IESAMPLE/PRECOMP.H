#include <windows.h>
#include <windowsx.h>
#include <winbase.h>

#include <commctrl.h>


// Component includes
//#include <docobj.h>
//#include <olectl.h>
//#include <mshtml.h>

// Site includes
//#include <mshtmcid.h>
//#include <mshtmdid.h>

// Include the automation definitions...
#include <exdisp.h>
#include <exdispid.h>

#include <objbase.h>

#include <tchar.h>

#include <wininet.h>
#include <afdfunc.h>	//ras stuff

#include <mshtml.h>

extern HINSTANCE g_hInstance;


#define MAXLEN 512
#define ANIMATIONWIDTH		22
#define ANIMATIONHEIGHT		22

//from commdlg.h
#define FR_DOWN                         0x00000001
#define FR_WHOLEWORD					0x00000002
#define FR_MATCHCASE                    0x00000004

#define RELEASE_INTERFACE(x)	if (x)	x->Release();

#define LOADSTRING(id) (LPCTSTR )LoadString(g_hInstance, id, NULL, 0)

#define _countof(x) (sizeof(x) / sizeof(x[0]))