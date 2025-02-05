/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998-2000  Microsoft Corporation

Module Name:  mainwnd.h

Abstract:  Implements the main window, the container for the webbrowser

Functions:

Notes: This class implements the container and its interaction with the webbrowser control,
        commandbar, statusbar etc.


--*/

#ifndef _MAINWND_H
#define _MAINWND_H_

#include "wmphlpie.h"

#define ADDRESSBARWIDTH		175

class CAnimThread;

class CMainWnd :    public IOleContainer,
    public IOleClientSite,
    public IOleInPlaceSite,
    public DWebBrowserEvents2
{

public:
    CMainWnd();
    ~CMainWnd();

    BOOL Create(LPTSTR lpszUrl, BOOL fFirstThread);

    //IUnknown methods
    STDMETHOD(QueryInterface) (REFIID riid, LPVOID * ppv)
    {
        if ((riid == IID_IOleContainer) || (riid == IID_IUnknown))
        {
            *ppv = (IOleContainer *) this;
        }
        else if (riid == IID_IOleClientSite)
        {
            *ppv = (IOleClientSite *)this;
        }
        else if (riid == IID_IOleInPlaceSite)
        {
            *ppv = (IOleInPlaceSite *)this;
        }
        else if (riid == DIID_DWebBrowserEvents2)
        {
            *ppv = (DWebBrowserEvents2 *)this;
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    STDMETHOD_(ULONG, AddRef) (void)
    {
        InterlockedIncrement((LONG*)&_ulRefs);
        return _ulRefs;
    }

    STDMETHOD_(ULONG, Release) (void)
    {
        ULONG ulRefs = _ulRefs;
        if (InterlockedDecrement((LONG*)&_ulRefs) == 0)
        {
            delete this;
            return 0;
        }
        return ulRefs - 1;
    }


    // IOleContainer methods
    STDMETHOD(ParseDisplayName)(IBindCtx *, LPOLESTR, ULONG *, IMoniker **) { return E_NOTIMPL;}
    STDMETHOD(EnumObjects)(DWORD, IEnumUnknown **)
    {
        return E_NOTIMPL;
    }
    STDMETHOD(LockContainer)(BOOL)                          { return S_OK;}

    // IOleClientSite methods
    STDMETHOD(SaveObject) (void)                            { return E_NOTIMPL;}
    STDMETHOD(GetMoniker) (DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER * ppmk) { return E_NOTIMPL;}
    STDMETHOD(GetContainer) (LPOLECONTAINER * ppContainer)
    {
        return E_NOINTERFACE;
    }
    STDMETHOD(ShowObject) (void)                            { return E_NOTIMPL;}
    STDMETHOD(OnShowWindow) (BOOL fShow)                    { return E_NOTIMPL;}
    STDMETHOD(RequestNewObjectLayout) (void)                { return E_NOTIMPL;}

    // IOleWindow methods
    STDMETHOD(GetWindow)(HWND *phwnd)
    {
        *phwnd = _hWnd;
        return S_OK;
    }
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode)        { return E_NOTIMPL;}

    // IOleInPlaceSite methods
    STDMETHOD(CanInPlaceActivate)   (void)                  { return S_OK;}
    STDMETHOD(OnInPlaceActivate)    (void)
    {
        _bInPlaceActive = TRUE;
        return S_OK;
    }

    STDMETHOD(OnUIActivate)         (void)                  { return E_NOTIMPL;}
    STDMETHOD(GetWindowContext) (
                                LPOLEINPLACEFRAME FAR *     lplpFrame,
                                LPOLEINPLACEUIWINDOW FAR *  lplpDoc,
                                LPRECT                      lprcPosRect,
                                LPRECT                      lprcClipRect,
                                LPOLEINPLACEFRAMEINFO       lpFrameInfo)
    {
        int nHeight = 0;
        GetClientRect(_hWnd, lprcPosRect);
        GetClientRect(_hWnd, lprcClipRect);

        RECT rc = {0,0,0,0};

#ifndef NOCOMMANDBAR
        GetClientRect(_hWndCmdbar, &rc);
        lprcPosRect->top += rc.bottom;
        lprcClipRect->top += rc.bottom;
#endif	//NOCOMMANDBAR

#ifndef NOSTATUSBAR
        GetClientRect(_hWndStatus, &rc);
        lprcPosRect->bottom -= rc.bottom;
        lprcClipRect->bottom -= rc.bottom;
#endif //NOSTATUSBAR

        return S_OK;
    }

    STDMETHOD(Scroll)               (SIZE scrollExtent)     { return E_NOTIMPL;}
    STDMETHOD(OnUIDeactivate)       (BOOL fUndoable)        { return E_NOTIMPL;}
    STDMETHOD(OnInPlaceDeactivate)  (void)
    {
        _bInPlaceActive = FALSE;
        return S_OK;
    }

    STDMETHOD(DiscardUndoState)     (void)                  { return E_NOTIMPL;}
    STDMETHOD(DeactivateAndUndo)    (void)                  { return E_NOTIMPL;}
    STDMETHOD(OnPosRectChange)      (LPCRECT lprcPosRect)   { return E_NOTIMPL;}

    //DWebBrowserEvents
    //IDispatch methods
    STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo)          { return E_NOTIMPL;}

    STDMETHOD(GetTypeInfo)(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo)   { return E_NOTIMPL;}

    STDMETHOD(GetIDsOfNames)(REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,
                             LCID lcid, DISPID FAR* rgdispid)                  { return E_NOTIMPL;}

    STDMETHOD(Invoke)(DISPID dispidMember,REFIID riid,LCID lcid,WORD wFlags,
                      DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
                      EXCEPINFO FAR* pexcepinfo,UINT FAR* puArgErr);

public:
    BOOL                    PreTranslateMessage(LPMSG pMsg);
    VOID                    Close();
    HWND                    GetWindow() {   return _hWnd;}

#ifndef NOCOMMANDBAR
    VOID                    UpdateUrlList(LPCTSTR lpszUrl);
    VOID                    SelChange();
    HWND                    GetCommandBand()    { return _hWndCmdband;}
#endif	//NOCOMMANDBAR

#ifndef NOPRINT
    VOID                    Print();
#endif	//NOPRINT

    HANDLE                  hEvent;

    IWebBrowser2            *_pBrowser;


protected:
    static LRESULT  CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#ifndef NOCOMMANDBAR
    static LRESULT  CALLBACK AddressEditProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
    WNDPROC         _lpfnAddressEditProc;

#endif	//NOCOMMANDBAR

#ifndef NOOPTIONS
    static BOOL     CALLBACK    OptionsDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif

#ifndef NOFIND
    //Added for HTML Find Dialogs
    HRESULT                 FindString();
#endif	//NOFIND


#ifndef NOCOMMANDBAR
    BOOL                    CreateCommandbar();
    LRESULT                 HandleCommand(WPARAM wParam, LPARAM lParam);
    VOID                    ChangeFontSize(WORD wSize);

    VOID                    LoadTypedUrls();
#endif //NOCOMMANDBAR

    LRESULT                 ProcessMediaNavigate(UINT message, DWORD dwProcess, WMPHELPERMSG *pWMPMsg);
    BOOL                    RegisterMainWnd();
    HWND                    CreateBrowser(HWND hParent, LPTSTR lpszUrl, BOOL fFirstThread);
    HRESULT                 Activate(IOleObject *pObject);
    HRESULT                 InitEvents();

#ifndef NOSTATUSBAR
    BOOL                    CreateStatusBar();
#endif	//NOSTATUSBAR

protected:
    HACCEL                  _hAccelTbl;     //a handle to the accelerator table
    HWND                    _hWnd;          //the main window

    ULONG                   _ulRefs;            //reference count for the interfaces supported by the container
    BOOL                    _bInPlaceActive;

    BOOL                    _bTyped;



#ifndef NOCOMMANDBAR
    HWND                    _hWndCmdband;
    HWND                    _hWndCmdbar;
    HWND                    _hWndAddressCombo;  //the combo box for typed urls
    HWND                    _hWndAddressEdit;   //the edit box of that combo box
#endif	//NOCOMMANDBAR

#ifndef	NOSTATUSBAR
    HWND                    _hWndStatus;        //handle to the status bar
    HWND                    _hWndProgress;      //handle to the progress bar
#endif	//NOSTATUSBAR

    HWND                    _hWndBrowser;       //handle to the browser window

    IOleObject              *_pObject;
    IConnectionPoint        *_pCP;
    IOleInPlaceActiveObject *_pIPActiveObj;
    DWORD                   _dwEventCookie;

    TCHAR                   _szTitle[MAXLEN];   //title of the current document
    HWND                    _hwndPrint;     //handle to the print status dialog
    CAnimThread             *_pthrdAnimation;   //the animation thread

#ifndef NOOFFLINE
	BOOL					m_fCheckedOffline;
#endif	//NOOFFLINE
};

#endif	//_MAINWND_H_
