/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998-1999  Microsoft Corporation

Module Name:  mainwnd.cpp

Abstract:  Implements the main window, the container for the webbrowser

Functions:

Notes: Most of the code resides here. The container and its interaction with the webbrowser control,
        commandbar, statusbar etc.


--*/

#include "Precomp.h"
#include "resource.h"
#include "MainWnd.h"
#include "animthrd.h"

#define INITGUID
#include "initguid.h"


DEFINE_GUID(CLSID_WebBrowser,       0x8856F961L, 0x340A, 0x11D0, 0xA9, 0x6B, 0x00, 0xC0, 0x4F, 0xD7, 0x05, 0xA2);
DEFINE_GUID(IID_IWebBrowser,        0xEAB22AC1L, 0x30C1, 0x11CF, 0xA7, 0xEB, 0x00, 0x00, 0xC0, 0x5B, 0xAE, 0x0B);
DEFINE_GUID(IID_IWebBrowser2,       0xD30C1661L, 0xCDAF, 0x11D0, 0x8A, 0x3E, 0x00, 0xC0, 0x4F, 0xC9, 0xE2, 0x6E);
DEFINE_GUID(DIID_DWebBrowserEvents, 0xEAB22AC2L, 0x30C1, 0x11CF, 0xA7, 0xEB, 0x00, 0x00, 0xC0, 0x5B, 0xAE, 0x0B);
DEFINE_GUID(DIID_DWebBrowserEvents2, 0x34A715A0L, 0x6587, 0x11D0, 0x92, 0x4A, 0x00, 0x20, 0xAF, 0xC7, 0xAC, 0x4D);
DEFINE_GUID(IID_IWebBrowserApp,     0x0002DF05L, 0x0000, 0x0000, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46);

const GUID SID_SDocHost = { 0xc6504990, 0xd43e, 0x11cf, { 0x89, 0x3b, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x1a}};


static TBBUTTON tbBtns[] = {    {0, ID_GO_BACK, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {1, ID_GO_FORWARD, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {2, ID_VIEW_STOP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {3, ID_VIEW_REFRESH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {4, ID_GO_HOME, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
    {5, ID_GO_SEARCH, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0},
};

static TCHAR szTypedUrls[] = _T("Software\\Microsoft\\Internet Explorer\\TypedURLs");

//the indexes for the menu items
#define ID_FILE				0
#define ID_VIEW				1
#define ID_VIEW_FONTS		0

#define PROGRESSPANEWIDTH	200			//width of the progressbar
#define MAX_STATUSMSGLEN	127			//the maximum length for the statusbar message
#define ID_COMBOEDIT		1001		//ID for the edit box of a combo box
#define MAXURLS				25			//maximum number of typed url's.

HINSTANCE   g_hInstance = NULL;
LONG        glThreadCount = 0;
HANDLE      ghExitEvent = NULL;

HWND ghWndAddressEdit = NULL;


#ifdef OEMCONTROLS
BOOL RegisterOemControls();
#endif

static HRESULT FindString();


DWORD WINAPI NewWindow(LPVOID pParam)
{

    CMainWnd *pWnd = (CMainWnd *)pParam;
    MSG msg;
    BOOL fRet;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    fRet = pWnd->Create(NULL, FALSE);

    SetEvent(pWnd->hEvent);

    if (!fRet)
    {
        pWnd->_pBrowser = NULL;
        return 0;
    }

    while (GetMessage( &msg, NULL, 0, 0 ) )
    {
        if (!pWnd->PreTranslateMessage(&msg) && !(msg.message == WM_CHAR && msg.wParam == VK_TAB))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pWnd->Release();

    CoUninitialize();

    InterlockedDecrement(&glThreadCount);
    SetEvent(ghExitEvent);

    return msg.wParam;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow)
{
    CMainWnd *pMainWnd;
    INITCOMMONCONTROLSEX iccsex;
    MSG msg;

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr))
    {
        return FALSE;
    }

    ghExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!ghExitEvent)
    {
        return FALSE;
    }

    iccsex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccsex.dwICC = ICC_COOL_CLASSES;
    InitCommonControlsEx(&iccsex);

    g_hInstance = hInst;

    pMainWnd = new CMainWnd;

    if (!pMainWnd)
    {
        return FALSE;
    }

    if (!pMainWnd->Create(lpCmdLine, TRUE))
    {
        return FALSE;
    }

    while (GetMessage( &msg, NULL, 0, 0 ) )
    {
        if (!pMainWnd->PreTranslateMessage(&msg) && !(msg.message == WM_CHAR && msg.wParam == VK_TAB))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    pMainWnd->Release();

    while (glThreadCount > 0)
    {
        WaitForSingleObject(ghExitEvent, INFINITE);
    }

    CoUninitialize();

    return msg.wParam;
}


CMainWnd::CMainWnd()
{
    _ulRefs = 1;
    _hWnd = NULL;
    _pBrowser = NULL;
    _pObject = NULL;
    _pCP = NULL;

#ifndef NOCOMMANDBAR
    _lpfnAddressEditProc = NULL;
    _hWndCmdbar = NULL;
    _hWndBrowser = NULL;
#endif	//NOCOMMANDBAR

#ifndef NOSTATUSBAR
    _hWndStatus = NULL;
    _hWndProgress = NULL;
#endif	//NOSTATUSBAR

    _bTyped = FALSE;

    _szTitle[0] = 0;
}

CMainWnd::~CMainWnd()
{
}

BOOL CMainWnd::RegisterMainWnd()
{
    WNDCLASS        wc;
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)CMainWnd::MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInstance;
    wc.hIcon         = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_IE));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT("IESAMPLE");

    if (!(RegisterClass(&wc)))
        return FALSE;

    return TRUE;
}


BOOL CMainWnd::Create(LPTSTR lpszUrl, BOOL fFirstThread)
{
    RECT rcArea, rcWindow;

    DWORD dwStyle = WS_VISIBLE/*|WS_OVERLAPPED|WS_BORDER|WS_CAPTION
                        |WS_SYSMENU|WS_MINIMIZEBOX*/;
    DWORD dwExStyle = 0/*WS_EX_OVERLAPPEDWINDOW*/;

    HMENU hMenu = NULL;


    if (fFirstThread)
    {
#ifdef OEMCONTROLS
        if (!RegisterOemControls())
            return FALSE;
#endif	//OEMCONTROLS

        if (!RegisterMainWnd())
            return FALSE;
    }

    _hWnd = ::CreateWindowEx(dwExStyle,
                             TEXT("IESAMPLE"),
                             LOADSTRING(IDS_IE),
                             dwStyle,
                             CW_USEDEFAULT,
                             0,
                             CW_USEDEFAULT,
                             0,
                             NULL, hMenu, g_hInstance, 0);
    if (!_hWnd)
        return FALSE;

    SetWindowLong(_hWnd, GWL_USERDATA, (DWORD)this);

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcArea, 0);
    GetWindowRect(_hWnd, &rcWindow);

#ifndef NOCOMMANDBAR
    if (!CreateCommandbar())
        return FALSE;

    _pthrdAnimation = new CAnimThread(this);
    if (!_pthrdAnimation)
        return FALSE;

    _pthrdAnimation->StartThread();

#endif	//NOCOMMANDBAR

#ifndef NOSTATUSBAR
    if (!CreateStatusBar())
        return FALSE;
#endif	//NOSTATUSBAR

    if (!(_hWndBrowser = CreateBrowser(_hWnd, lpszUrl, fFirstThread)))
        return FALSE;

    LONG lStyle = GetWindowLong(_hWndBrowser, GWL_STYLE);
    SetWindowLong(_hWndBrowser, GWL_STYLE, lStyle|WS_BORDER);
    SetFocus(_hWndBrowser);

    _hAccelTbl = LoadAccelerators(g_hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

    return TRUE;
}

#ifndef NOCOMMANDBAR
BOOL CMainWnd::CreateCommandbar()
{
    TBBUTTON tbSep = {0, IDC_STATIC, 0, TBSTYLE_SEP, 0, 0};

    RECT rc;
    GetClientRect(_hWnd, &rc);

    _hWndCmdband = CommandBands_Create(g_hInstance, _hWnd, ID_COMMANDBAND,
                                       0, NULL);
    REBARBANDINFO rbbi[1];
    rbbi[0].cbSize = sizeof(REBARBANDINFO);
    rbbi[0].fMask = RBBIM_ID|RBBIM_STYLE|RBBIM_SIZE;
    rbbi[0].cx = rc.right - ANIMATIONWIDTH;
    rbbi[0].wID = ID_COMMANDBAR;
    rbbi[0].fStyle = RBBS_NOGRIPPER ;

    if (!CommandBands_AddBands(_hWndCmdband, g_hInstance, 1, rbbi))
        return FALSE;

    _hWndCmdbar = CommandBands_GetCommandBar(_hWndCmdband, 0);


    if (!CommandBar_InsertMenubar(_hWndCmdbar, g_hInstance, IDR_MAINMENU, 0))
        return FALSE;

    if (!CommandBar_InsertButton(_hWndCmdbar, 1, &tbSep))
        return FALSE;

    if (!(_hWndAddressCombo = CommandBar_InsertComboBox(_hWndCmdbar,
                                                        g_hInstance,
                                                        ADDRESSBARWIDTH,
                                                        WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWN|CBS_AUTOHSCROLL,
                                                        ID_ADDRESSBAR, 2)))
        return FALSE;


    LoadTypedUrls();

    _hWndAddressEdit = GetDlgItem(_hWndAddressCombo, ID_COMBOEDIT);
	ghWndAddressEdit = _hWndAddressEdit;

    SetWindowLong(_hWndAddressEdit, GWL_USERDATA, (DWORD)this);

    _lpfnAddressEditProc = (WNDPROC )SetWindowLong(_hWndAddressEdit,
                                                   GWL_WNDPROC, (LONG)AddressEditProc);
    SendMessage(_hWndAddressEdit, EM_LIMITTEXT, MAXLEN-1, 0L);

    if ((CommandBar_AddBitmap(_hWndCmdbar, g_hInstance, IDB_TBBITMAP,
                              sizeof(tbBtns)/sizeof(TBBUTTON), 16, 16)) < 0)
        return FALSE;

    if (!CommandBar_AddButtons(_hWndCmdbar, _countof(tbBtns), tbBtns))
        return FALSE;

    if (!CommandBar_InsertButton(_hWndCmdbar, 3, &tbSep))
        return FALSE;

    return TRUE;
}
#endif	//NOCOMMANDBAR


#ifndef NOSTATUSBAR
BOOL CMainWnd::CreateStatusBar()
{
    if (!(_hWndStatus = CreateStatusWindow(WS_CHILD|WS_VISIBLE, _T("Ready"), _hWnd, ID_STATUSBAR)))
        return FALSE;

    RECT rc;
    GetClientRect(_hWndStatus, &rc);
    int naParts[2];
    naParts[0] = rc.right - PROGRESSPANEWIDTH;
    naParts[1] = rc.right;
    SendMessage(_hWndStatus, SB_SETPARTS, (WPARAM )2, (LPARAM)naParts);

    SendMessage(_hWndStatus, SB_GETRECT, 1, (LPARAM )&rc);
    _hWndProgress = CreateWindowEx(0, PROGRESS_CLASS, _T(""), WS_CHILD|WS_VISIBLE|PBS_SMOOTH,
                                   rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
                                   _hWndStatus, NULL, g_hInstance, NULL);
    if (!_hWndProgress)
        return FALSE;
    SendMessage(_hWndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
    return TRUE;
}
#endif	//NOSTATUSBAR

HWND CMainWnd::CreateBrowser(HWND hParent, LPTSTR lpszUrl, BOOL fFirstThread)
{
    HRESULT hr;
    IUnknown *pUnk = NULL;
    IOleObject *pObject = NULL;

    if (!_pBrowser)
    {
        hr = CoCreateInstance(CLSID_WebBrowser, NULL,
                              CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
                              IID_IUnknown, (LPVOID *)(&pUnk));
        if (hr)
            return FALSE;

        hr = pUnk->QueryInterface(IID_IOleObject, (LPVOID *)(&pObject));
        if (hr)
            goto Cleanup;

        DWORD dwFlags;
        hr = pObject->GetMiscStatus(DVASPECT_CONTENT, &dwFlags);
        if (hr)
            goto Cleanup;

        if (dwFlags & OLEMISC_SETCLIENTSITEFIRST)
        {
            IOleClientSite *pClientSite;
            hr = QueryInterface(IID_IOleClientSite, (LPVOID *)(&pClientSite));
            if (hr)
                goto Cleanup;

            hr = pObject->SetClientSite(pClientSite);
            pClientSite->Release();
            if (hr)
                goto Cleanup;
        }

        hr = Activate(pObject);
        if (hr)
            goto Cleanup;

        hr = _pObject->QueryInterface(IID_IWebBrowser2, (void **)&_pBrowser);
        if (hr)
            goto Cleanup;

        // See if there might be a url in lpszUrl
        hr = S_FALSE;

        if (fFirstThread)
        {
            if (lpszUrl)
            {
                LPTSTR lpszUrlAlpha = lpszUrl;

                while (hr == S_FALSE && *lpszUrlAlpha != _T('\0'))
                {
                    if (_istalpha(*lpszUrlAlpha))
                        hr = S_OK;

                    lpszUrlAlpha++;
                }
            }

            if (hr == S_OK)
                _pBrowser->Navigate(lpszUrl, NULL, NULL, NULL, NULL);
            else
                _pBrowser->GoHome();
        }

        hr = InitEvents();
        if (hr)
            goto Cleanup;

        hr = pUnk->QueryInterface(IID_IOleInPlaceActiveObject, (LPVOID *)(&_pIPActiveObj));
        if (hr)
            goto Cleanup;
    }

    Cleanup:
    if (pUnk)
        pUnk->Release();
    if (pObject)
        pObject->Release();

    IOleWindow *pWnd = NULL;
    HWND hwndBrowser = NULL;

    if (_pBrowser)
    {
        hr = _pBrowser->QueryInterface(IID_IOleWindow, (LPVOID *)(&pWnd));
        if (hr)
            return NULL;
    }

    if (pWnd)
    {
        hr = pWnd->GetWindow(&hwndBrowser);
        pWnd->Release();
    }
    return hwndBrowser;
}

HRESULT CMainWnd::Activate(IOleObject *pObject)
{
    _pObject = pObject;
    _pObject->AddRef();

    RECT rc;
    ::GetClientRect(_hWnd, &rc);

    HRESULT hr;
    hr    =    _pObject->DoVerb(   OLEIVERB_UIACTIVATE,    NULL,    this,    0,    _hWnd,    &rc);

    if (hr)
        goto Cleanup;

    Cleanup:
    return hr;
}

HRESULT CMainWnd::InitEvents()
{
    HRESULT                     hr;
    IConnectionPointContainer  *pCPCont = NULL;
    DWebBrowserEvents          *pEvents = NULL;

    if (!_pBrowser)
        return S_FALSE;
    hr = _pBrowser->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&pCPCont);
    if (hr)
        return S_FALSE;
    hr = pCPCont->FindConnectionPoint(DIID_DWebBrowserEvents2, &_pCP);
    if (hr)
        goto Cleanup;

    hr = QueryInterface(DIID_DWebBrowserEvents2, (LPVOID *)(&pEvents));
    if (hr)
        goto Cleanup;
    hr = _pCP->Advise(pEvents, &(_dwEventCookie));
    if (hr)
        goto Cleanup;


    Cleanup:
    if (pCPCont)
        pCPCont->Release();
    if (pEvents)
        pEvents->Release();
    return hr;
}

STDMETHODIMP CMainWnd::Invoke (
                              DISPID dispidMember,
                              REFIID riid,
                              LCID lcid,
                              WORD wFlags,
                              DISPPARAMS FAR* pdparams,
                              VARIANT FAR* pvarResult,
                              EXCEPINFO FAR* pexcepinfo,
                              UINT FAR* puArgErr
                              )
{
    switch (dispidMember)
    {
#ifndef NOSTATUSBAR
    case DISPID_STATUSTEXTCHANGE:
        {
            if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR)
            {
                // NULL terminate the string
                TCHAR   szStatusText[128];

                wcsncpy(szStatusText, pdparams->rgvarg[0].bstrVal, 128);
                szStatusText[128 - 1] = 0;
                SendMessage(_hWndStatus, SB_SETTEXT, 0, (LPARAM)szStatusText);
            }
            break;
        }

    case DISPID_PROGRESSCHANGE:
        {
            LONG lProgMax = pdparams->rgvarg[0].lVal;
            LONG lProg = pdparams->rgvarg[1].lVal;
            UINT nPercent = (lProg == -1) ? 0 : (lProgMax) ? lProg*100/lProgMax : 0;
            SendMessage(_hWndProgress, PBM_SETPOS, nPercent, 0);
        }
        break;
#endif	//NOSTATUSBAR

#ifndef NOCOMMANDBAR
    case DISPID_NAVIGATECOMPLETE2:
        if (_bTyped)
        {
            UpdateUrlList(pdparams->rgvarg[0].pvarVal->bstrVal);
            _bTyped = FALSE;
        }
        else
        {
            // add the URL which was navigated to by clicking to the URLList combo-box.
            BSTR bstrUrl = NULL;
            _pBrowser->get_LocationURL(&bstrUrl);
            UpdateUrlList((LPCTSTR)bstrUrl);
            SysFreeString(bstrUrl);
        }
        break;

#endif	//NOCOMMANDBAR

    case DISPID_TITLECHANGE:
        {
            if (pdparams && pdparams->rgvarg[0].vt == VT_BSTR)
            {
                TCHAR szTitle[MAXLEN*2];
                _tcsncpy(_szTitle, pdparams->rgvarg[0].bstrVal, MAXLEN);
                _tcsncpy(szTitle, pdparams->rgvarg[0].bstrVal, MAXLEN);
                _tcscat(szTitle, LOADSTRING(IDS_HYPHEN));
                _tcscat(szTitle, LOADSTRING(IDS_IE));

                SetWindowText(_hWnd, szTitle);
            }
            break;
        }

    case DISPID_ONQUIT:
        PostMessage(_hWnd, WM_CLOSE, 0, 0L);
        break;

#ifndef NOCOMMANDBAR
    case DISPID_DOWNLOADBEGIN:
        _pthrdAnimation->StartAnimation();
        break;
    case DISPID_DOWNLOADCOMPLETE:
    case DISPID_DOCUMENTCOMPLETE:
        _pthrdAnimation->StopAnimation();
        SendMessage(_hWndProgress, PBM_SETPOS, 0, 0);
        break;
#endif	//NOCOMMANDBAR

    case DISPID_NEWWINDOW2:

        HANDLE hThread;
        CMainWnd *pNewWnd;
        IDispatch *pDispatch;
        HRESULT hr;

        pNewWnd = new CMainWnd;

        if (!pNewWnd)
        {
            return E_OUTOFMEMORY;
        }

        pNewWnd->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!pNewWnd->hEvent)
        {

        }

        InterlockedIncrement(&glThreadCount);

        hThread = CreateThread(NULL, 0, NewWindow, (LPVOID)pNewWnd, 0, NULL);

        if (!hThread)
        {
            delete pNewWnd;
            InterlockedDecrement(&glThreadCount);
            return E_OUTOFMEMORY;
        }

        WaitForSingleObject(pNewWnd->hEvent, INFINITE);

        if (pNewWnd->_pBrowser)
        {
            hr = pNewWnd->_pBrowser->QueryInterface(IID_IDispatch, (LPVOID *)(&pDispatch));
        }
        else
        {
            hr = E_FAIL;
            pDispatch = NULL;
        }

        *(pdparams->rgvarg[0].pboolVal) = 0;
        *(pdparams->rgvarg[1].ppdispVal) = pDispatch;

        return hr;

        break;


    case DISPID_PROPERTYCHANGE:
    case DISPID_COMMANDSTATECHANGE:
    case DISPID_BEFORENAVIGATE2:
        break;
    default:
        break;
    }
    return S_OK;
}

#ifndef NOCOMMANDBAR
EXTERN_C const GUID CGID_MSHTML;
LRESULT CMainWnd::HandleCommand(WPARAM wParam, LPARAM lParam)
{
    UINT nID = LOWORD(wParam);
    switch (nID)
    {
    case ID_ADDRESSBAR:
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            SelChange();
            return 0;
        }
        break;
#ifndef NOFIND
    case ID_FILE_FIND:
        {
            FindString();
            return 0;
        }
#endif	//NOFIND

#ifndef NOPRINT
    case ID_FILE_PRINT:
        {
			_pBrowser->ExecWB(OLECMDID_PRINT, OLECMDEXECOPT_DODEFAULT, NULL, NULL);	
            return 0;
        }
#endif	//NOPRINT

#ifndef NOOFFLINE
	case ID_FILE_OFFLINE:
		{
			INTERNET_CONNECTED_INFO ci;
            DWORD dwBufferLength;

			memset(&ci, 0, sizeof(ci));
            dwBufferLength = sizeof(ci);
			InternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, &dwBufferLength);
            ci.dwConnectedState = (ci.dwConnectedState & INTERNET_STATE_DISCONNECTED_BY_USER) ? INTERNET_STATE_CONNECTED : INTERNET_STATE_DISCONNECTED_BY_USER;
			InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
			return 0;
		}
#endif //NOOFLINE

	case ID_FILE_CLOSE:
		PostMessage(_hWnd, WM_CLOSE, 0, 0L);
		return 0;

    case ID_EDIT_CUT:
		if (GetFocus()== _hWndAddressEdit)
			SendMessage(_hWndAddressEdit, WM_CUT, WPARAM(0), LPARAM(0));
		else
			_pBrowser->ExecWB(OLECMDID_CUT, OLECMDEXECOPT_DONTPROMPTUSER,
                          NULL, NULL);
        return 0;

    case ID_EDIT_COPY:
		if (GetFocus()== _hWndAddressEdit)
			SendMessage(_hWndAddressEdit, WM_COPY, WPARAM(0), LPARAM(0));
		else
    	    _pBrowser->ExecWB(OLECMDID_COPY, OLECMDEXECOPT_DONTPROMPTUSER,
                          NULL, NULL);
        return 0;
    case ID_EDIT_PASTE:
		if (GetFocus() == _hWndAddressEdit)
			SendMessage(_hWndAddressEdit, WM_PASTE, WPARAM(0), LPARAM(0));
		else
	        _pBrowser->ExecWB(OLECMDID_PASTE, OLECMDEXECOPT_DONTPROMPTUSER,
                          NULL, NULL);
        return 0;



    case ID_VIEW_FONTSLARGEST:
    case ID_VIEW_FONTSLARGER:
    case ID_VIEW_FONTSMEDIUM:
    case ID_VIEW_FONTSSMALLER:
    case ID_VIEW_FONTSSMALLEST:
        ChangeFontSize(LOWORD(wParam));
        return 0;

#ifndef NOOPTIONS
    case ID_VIEW_OPTIONS:
        DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_OPTIONS), _hWnd, (DLGPROC)OptionsDlgProc);
        return 0;
#endif	//NOOPTIONS

    case ID_GO_BACK:
        _pBrowser->GoBack();
        return 0;
    case ID_GO_FORWARD:
        _pBrowser->GoForward();
        return 0;
    case ID_GO_HOME:
        _pBrowser->GoHome();
        return 0;
    case ID_GO_SEARCH:
        _pBrowser->GoSearch();
        return 0;
    case ID_VIEW_REFRESH:
        _pBrowser->Refresh();
        return 0;
    case ID_VIEW_STOP:
        _pBrowser->Stop();
        return 0;
    case ID_FOCUS_URL:
        if (_hWndAddressEdit)
        {
            SetFocus(_hWndAddressEdit);
        }
        return 0;
    default:
        // if this is from the encoding popup
        if (nID > ID_VIEW_ENCODING)
        {
		    if (_pBrowser)
            {
                IOleCommandTarget* pcmdt;
	            if (SUCCEEDED(_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) &pcmdt)))
	            {
                    pcmdt->Exec(&CGID_MSHTML, nID - ID_VIEW_ENCODING, 0, NULL, NULL);
	                pcmdt->Release();
                }
            }
        }
        return 0;
    }
    return TRUE;
}

VOID CMainWnd::ChangeFontSize(WORD wSize)
{
    VARIANT vaSize;
    vaSize.vt = VT_I4;
    vaSize.lVal = wSize - ID_VIEW_FONTSSMALLEST;
    HRESULT hr = _pBrowser->ExecWB(OLECMDID_ZOOM, OLECMDEXECOPT_DONTPROMPTUSER,
                                   &vaSize, NULL);
    HMENU hMenu = CommandBar_GetMenu(_hWndCmdbar, 0);
    HMENU hmenuView = GetSubMenu(hMenu, ID_VIEW);
    HMENU hmenuFonts = GetSubMenu(hMenu, ID_VIEW_FONTS);
    BOOL bRet = CheckMenuRadioItem(hmenuFonts, ID_VIEW_FONTSLARGEST, ID_VIEW_FONTSSMALLEST, wSize, MF_BYCOMMAND);
    CommandBar_DrawMenuBar(_hWndCmdbar, 0);
}
#endif	//NOCOMMANDBAR



EXTERN_C const GUID CGID_ShellDocView;

LRESULT CALLBACK CMainWnd::MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CMainWnd *pMainWnd = (CMainWnd*)GetWindowLong(hwnd, GWL_USERDATA);

    if (pMainWnd)
    {
        switch (message)
        {
        case WM_CLOSE:
            pMainWnd->Close();
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            break;
		case WM_SETFOCUS:
			if (pMainWnd->_pIPActiveObj)
			{
				pMainWnd->_pIPActiveObj->OnFrameWindowActivate(LOWORD(wParam) != WA_INACTIVE);
			}
			return 0;

#ifndef NOCOMMANDBAR
        case WM_COMMAND:
            return pMainWnd->HandleCommand(wParam, lParam);
            break;
#endif  //NOCOMMANDBAR

        case WM_INITMENUPOPUP:

            {
                // for WM_INITMENUPOPUP
                OLECMD rgcmd[] = {
                   { OLECMDID_CUT, 0 },
                   { OLECMDID_COPY, 0 },
                   { OLECMDID_PASTE, 0 }
                };

                // check/uncheck the offline state
                INTERNET_CONNECTED_INFO ci;
                DWORD dwBufferLength;

                memset(&ci, 0, sizeof(ci));
                dwBufferLength = sizeof(ci);
                InternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, &dwBufferLength);
                CheckMenuItem((HMENU)wParam, ID_FILE_OFFLINE, MF_BYCOMMAND | (ci.dwConnectedState & INTERNET_STATE_DISCONNECTED_BY_USER) ? MF_CHECKED : MF_UNCHECKED);

                // enable / disable edit menu commands
                IOleCommandTarget* pcmdt;
                int i;

				if (GetFocus()==ghWndAddressEdit)
				{
					DWORD dwStart, dwEnd;
					
					SendMessage(ghWndAddressEdit, EM_GETSEL, (WPARAM)(&dwStart), (LPARAM)(&dwEnd));
					if (dwEnd - dwStart > 1)
					{
						EnableMenuItem((HMENU)wParam, ID_EDIT_CUT, MF_BYCOMMAND | MF_ENABLED);
						EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, MF_BYCOMMAND | MF_ENABLED);
					}
					else
					{
						EnableMenuItem((HMENU)wParam, ID_EDIT_CUT, MF_BYCOMMAND | MF_GRAYED);
						EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, MF_BYCOMMAND | MF_GRAYED);
					}					
					if (IsClipboardFormatAvailable(CF_UNICODETEXT) || IsClipboardFormatAvailable(CF_TEXT))
					{
						EnableMenuItem((HMENU)wParam, ID_EDIT_PASTE, MF_BYCOMMAND | MF_ENABLED);
					}					
					else
					{
						EnableMenuItem((HMENU)wParam, ID_EDIT_PASTE, MF_BYCOMMAND | MF_GRAYED);
					}					
				}
				else
				{
				   if (!pMainWnd->_pBrowser)
    	               break;

	               	if (SUCCEEDED(pMainWnd->_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) & pcmdt)))
	              	{
	                    pcmdt->QueryStatus(NULL, _countof(rgcmd), rgcmd, NULL);
	                    pcmdt->Release();
	   	            }
	                for (i=0; i<_countof(rgcmd); i++)
	                {
	                    EnableMenuItem((HMENU)wParam, ID_EDIT_CUT + i, rgcmd[i].cmdf & OLECMDF_ENABLED ?
	                        (MF_BYCOMMAND | MF_ENABLED) : ( MF_BYCOMMAND| MF_GRAYED));
	                }
				}

                // provide encoding menu
			    if (!pMainWnd->_pBrowser)
    	            break;

                // if this is not the view menu, bail out early
                MENUITEMINFO mii;
                mii.cbSize = sizeof(mii);
                mii.fMask = MIIM_ID;
                GetMenuItemInfo((HMENU)wParam, 3, TRUE, &mii);
                if (mii.wID != ID_VIEW_STOP)
                    break;

	            if (SUCCEEDED(pMainWnd->_pBrowser->QueryInterface(IID_IOleCommandTarget, (LPVOID*) & pcmdt)))
	            {
                    VARIANTARG v = {0};
                    HRESULT hr;

                    hr = pcmdt->Exec(&CGID_ShellDocView, 27/*SHDVID_GETMIMECSETMENU*/, 0, NULL, &v);
                    if (S_OK == hr)
                    {
                        MENUITEMINFO mii;
                        UINT uItem = 0;
                        HMENU hmenuFonts = (HMENU)v.lVal;

                        // offset all of the command IDs into a known range
                        mii.cbSize = sizeof(mii);
                        mii.fMask = MIIM_ID;
                        while (GetMenuItemInfo(hmenuFonts, uItem, TRUE, &mii))
                        {
                            mii.wID += ID_VIEW_ENCODING;
                            SetMenuItemInfo(hmenuFonts, uItem++, TRUE, &mii);
                        }

                        HMENU hmenuOldFonts = GetSubMenu((HMENU)wParam, 1);
                        RemoveMenu((HMENU)wParam, 1, MF_BYPOSITION);
                        DestroyMenu(hmenuOldFonts);
                        InsertMenu((HMENU)wParam, 1, MF_POPUP|MF_BYPOSITION, (UINT)hmenuFonts, TEXT("Enco&ding"));
                    }

	                pcmdt->Release();
                }

			}
            break;

        default:
            break;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}


#ifndef NOCOMMANDBAR
LRESULT CALLBACK CMainWnd::AddressEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{


    CMainWnd *pMainWnd = (CMainWnd*)GetWindowLong(hwnd, GWL_USERDATA);

    if (!pMainWnd)
    {
        return 0;
    }

    if ((message == WM_CHAR) && (wParam == VK_RETURN))
    {
        TCHAR szString[MAXLEN];
	int iSpaces = 0;
        GetWindowText(hwnd, szString, MAXLEN);
        // Remove preceding spaces.
        for (iSpaces=0; szString[iSpaces] == _T(' '); iSpaces++);
        pMainWnd->_pBrowser->Navigate(&szString[iSpaces], NULL, NULL, NULL, NULL);
        pMainWnd->_bTyped = TRUE;
        return 0;
    }
    else
        return CallWindowProc(pMainWnd->_lpfnAddressEditProc, hwnd, message, wParam, lParam);
}
#endif	//NOCOMMANDBAR

BOOL CMainWnd::PreTranslateMessage(LPMSG pMsg)
{
    if (TranslateAccelerator(_hWnd, _hAccelTbl, pMsg))  //first shot to the main frame
        return TRUE;

#ifndef NOCOMMANDBAR
    if (pMsg->hwnd == _hWndAddressEdit)
        return FALSE;
#endif	//NOCOMMANDBAR

    //then to mshtml
    if (_pIPActiveObj && pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST)
    {
        HRESULT hr = _pIPActiveObj->TranslateAccelerator(pMsg);
        return (hr != S_FALSE);
    }
    return FALSE;
}

#ifndef NOCOMMANDBAR
VOID CMainWnd::LoadTypedUrls()
{
    DWORD dwType = 0;
    HKEY hKey = NULL;

    LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, szTypedUrls, 0, KEY_QUERY_VALUE, &hKey);
    if (hKey && lRes==ERROR_SUCCESS)
    {
        DWORD dwValNames, dwMaxValNameLen, dwMaxValDataLen;
        lRes = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL,
                               &dwValNames, &dwMaxValNameLen, &dwMaxValDataLen, NULL, NULL);
        if (lRes==ERROR_SUCCESS)
        {
            TCHAR pszValName[10];
            TCHAR pszValData[MAXLEN];
            DWORD cbValName, cbData;
            for (DWORD i=dwValNames; i>0; i--)
            {
                cbData = sizeof(TCHAR)*MAXLEN;
                lRes = RegEnumValue(hKey, i-1, pszValName, &cbValName, 0,
                                    &dwType, (BYTE *)pszValData, &cbData);
				if (lRes ==ERROR_SUCCESS)
					SendMessage(_hWndAddressCombo, CB_INSERTSTRING, 0, (LPARAM )pszValData);
            }
        }
    }

}

VOID CMainWnd::UpdateUrlList(LPCTSTR lpszUrl)
{

    int i = SendMessage(_hWndAddressCombo, CB_FINDSTRINGEXACT,
                        (WPARAM )-1, (LPARAM )lpszUrl);
    if (i == CB_ERR)
    {
        SendMessage(_hWndAddressCombo, CB_INSERTSTRING, 0, (LPARAM )lpszUrl);
        SendMessage(_hWndAddressCombo, CB_SETCURSEL,
                    (WPARAM)0, (LPARAM)0);
    }
    else
    {
        SendMessage(_hWndAddressCombo, CB_SETCURSEL,
                    (WPARAM)i, (LPARAM)0);
    }

    if ( (SendMessage(_hWndAddressCombo, CB_GETCOUNT, 0, 0L)) > MAXURLS)
    {
        SendMessage(_hWndAddressCombo, CB_DELETESTRING, MAXURLS-1, 0L);
    }
}

VOID CMainWnd::SelChange()
{
    TCHAR *szUrl;
    int nRet, nRet1 = SendMessage(_hWndAddressCombo, CB_GETCURSEL, 0, 0L);
    if (nRet1==CB_ERR)
        return ;
	nRet = SendMessage(_hWndAddressCombo, CB_GETLBTEXTLEN, nRet1, 0L); 
	if (nRet==CB_ERR)
		return;
	szUrl = new TCHAR[nRet+1];
	if (szUrl)  {
		SendMessage(_hWndAddressCombo, CB_GETLBTEXT, nRet1, (LPARAM )szUrl);
		_pBrowser->Navigate(szUrl, NULL, NULL, NULL, NULL);
		delete szUrl;
	}
}
#endif	//NOCOMMANDBAR

VOID CMainWnd::Close()
{

#ifndef NOCOMMANDBAR
    HKEY hKey = NULL;
    TCHAR *szString=NULL;	int cbString = 0;
    DWORD dwDisp;

    LONG lRes = RegCreateKeyEx(HKEY_CURRENT_USER, szTypedUrls, 0, NULL, 0, 0, 0, &hKey, &dwDisp);
    if (hKey && lRes==ERROR_SUCCESS)
    {
        int nCount = SendMessage(_hWndAddressCombo, CB_GETCOUNT, 0, 0L);
        int nLen;
        TCHAR szValueName[7];
        for (int i=0; i<nCount; i++)
        {
			nLen = SendMessage(_hWndAddressCombo, CB_GETLBTEXTLEN, i, 0L); 
			if (nLen==CB_ERR)  {
				delete szString;  cbString = 0;
				return;
			}
			if (nLen > cbString)  {
				delete szString;  cbString = 0;
				szString = new TCHAR[nLen+1];
				if (!szString)
					break;
				cbString = nLen;
			}
            nLen = SendMessage(_hWndAddressCombo, CB_GETLBTEXT, i, (LPARAM )szString);
            wsprintf(szValueName, _T("url%d"), i+1);
            lRes = RegSetValueEx(hKey, szValueName, 0, REG_SZ, (BYTE *)szString, (_tcslen(szString)+1)*sizeof(TCHAR));
        }
    }
	delete szString; cbString = 0;

    SetWindowLong(_hWndAddressEdit, GWL_WNDPROC, (LONG)_lpfnAddressEditProc);
#endif	//NOCOMMANDBAR

    _pIPActiveObj->Release();
    _pIPActiveObj = NULL;
    _pCP->Unadvise(_dwEventCookie);
    _pCP->Release();
    _pCP = NULL;
    _pObject->Close(FALSE);
    _pObject->Release();
    _pObject = NULL;
    _pBrowser->Release();
    _pBrowser = NULL;
}

#ifdef OEMCONTROLS
LRESULT CALLBACK OemComboProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            HDC hDC = GetDC(hwnd);
            SelectObject(hDC, GetStockObject(BLACK_PEN));
            SelectObject(hDC, GetStockObject(SYSTEM_FONT));
            RECT rc;
            GetClientRect(hwnd, &rc);
            TCHAR szText[] = _T("OEM Combo Box");
            DrawText(hDC, szText, _tcslen(szText),&rc, DT_LEFT|DT_TOP);
            EndPaint(hwnd, &ps);
            return 0;
        }
    default:
        break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);

}

LRESULT CALLBACK OemListBoxProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            HDC hDC = GetDC(hwnd);
            SelectObject(hDC, GetStockObject(BLACK_PEN));
            SelectObject(hDC, GetStockObject(SYSTEM_FONT));
            RECT rc;
            GetClientRect(hwnd, &rc);
            TCHAR szText[] = _T("OEM Combo Box");
            DrawText(hDC, szText, _tcslen(szText),&rc, DT_LEFT|DT_TOP);
            EndPaint(hwnd, &ps);
            return 0;
        }
    default:
        break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);

}


BOOL RegisterOemControls()
{
    WNDCLASS        wc;
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)OemComboProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = TEXT("combobox");

    if (!(RegisterClass(&wc)))
        return FALSE;

    wc.lpfnWndProc = (WNDPROC )OemListBoxProc;
    wc.lpszClassName = TEXT("listbox");

    if (!(RegisterClass(&wc)))
        return FALSE;

    return TRUE;

}

#endif	//OEMCONTROLS

#ifndef NOFIND
HRESULT CMainWnd::FindString()
{
    LPDISPATCH pDisp = NULL;
    LPOLECOMMANDTARGET pCmdTarg = NULL;
    HRESULT sts = S_OK;
    VARIANTARG var;

#define GOERROR_S(bNotCond)  {if (!(bNotCond)) goto errorS; }

    if (!_pBrowser)
        return S_OK;
    sts = _pBrowser->get_Document(&pDisp);
    GOERROR_S(pDisp);
    sts = pDisp->QueryInterface(IID_IOleCommandTarget, (LPVOID*)&pCmdTarg);
    GOERROR_S(pCmdTarg);

    var.vt   = VT_I4;
    var.lVal = 0;
    sts = pCmdTarg->Exec(
                        NULL,
                        OLECMDID_FIND,
                        MSOCMDEXECOPT_PROMPTUSER,
                        NULL,
                        &var);

    errorS:
    if (pCmdTarg)
        pCmdTarg->Release(); // release document's command target
    if (pDisp)
        pDisp->Release();    // release document's dispatch interface
    return sts;
}
#endif	//NOFIND
