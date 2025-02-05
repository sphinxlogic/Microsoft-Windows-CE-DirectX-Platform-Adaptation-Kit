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
// wmphelpr.h: Definition of the WMPHelper class
//
//////////////////////////////////////////////////////////////////////

#if !defined(_WMPHELPER_H__)
#define _WMPHELPER_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "wmphlpie.h"
#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// WMPHelper

class WMPHelper : 
	public IWMPHelper,
	public CComObjectRoot,
	public CComCoClass<WMPHelper,&CLSID_WMPHelper>,
    public IOleObject,
	public IPersistMoniker
{
public:
	WMPHelper()
    {
        m_wszMoniker[0] = 0;
    }

BEGIN_COM_MAP(WMPHelper)
	COM_INTERFACE_ENTRY(IWMPHelper)
	COM_INTERFACE_ENTRY(IPersistMoniker)
    COM_INTERFACE_ENTRY(IOleObject)
END_COM_MAP()

//DECLARE_NOT_AGGREGATABLE(WMPHelper) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation.

BEGIN_MSG_MAP(CWMPHelper)
END_MSG_MAP()

DECLARE_REGISTRY_RESOURCEID(IDR_WMPHelper)

public:
// IPersistMoniker
	STDMETHOD(GetCurMoniker)(IMoniker **ppmkName);
	STDMETHOD(SaveCompleted)(IMoniker *pmkName, LPBC pBCtx);
	STDMETHOD(Save)(IMoniker *pmkName, LPBC pBCtx, BOOL fRemember);
	STDMETHOD(Load)(BOOL fFullyAvailable, IMoniker *pmkName, IBindCtx *pbctx, DWORD grfMode);
	STDMETHOD(IsDirty)();
	STDMETHOD(GetClassID)(CLSID* pclsid);

// IOleObject
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite) { return E_NOTIMPL; }
    STDMETHOD(GetClientSite)(IOleClientSite *__RPC_FAR *ppClientSite) { return E_NOTIMPL; }
	STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj) { return E_NOTIMPL; }
	STDMETHOD(Close)(DWORD dwSaveOption) { return E_NOTIMPL; }
	STDMETHOD(SetMoniker)(DWORD dwWhichMoniker, IMoniker *pmk) { return E_NOTIMPL; }
	STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker *__RPC_FAR *ppmk) { return E_NOTIMPL; }
	STDMETHOD(InitFromData)(IDataObject *pDataObject, BOOL fCreation, DWORD dwReserved) { return E_NOTIMPL; }
	STDMETHOD(GetClipboardData)(DWORD dwReserved, IDataObject *__RPC_FAR *ppDataObject) { return E_NOTIMPL; }
    STDMETHOD(DoVerb)(LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect) { return E_NOTIMPL; }
	STDMETHOD(EnumVerbs)(IEnumOLEVERB *__RPC_FAR *ppEnumOleVerb) { return E_NOTIMPL; }
	STDMETHOD(Update)() { return E_NOTIMPL; }
	STDMETHOD(IsUpToDate)() { return E_NOTIMPL; }
	STDMETHOD(GetUserClassID)(CLSID *pClsid) { return E_NOTIMPL; }
	STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType) { return E_NOTIMPL; }
	STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel) { return E_NOTIMPL; }
	STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel) { return E_NOTIMPL; }
    STDMETHOD(Advise)(IAdviseSink *pAdvSink, DWORD *pdwConnection) { return E_NOTIMPL; }
	STDMETHOD(Unadvise)(DWORD dwConnection) { return E_NOTIMPL; }
	STDMETHOD(EnumAdvise)(IEnumSTATDATA *__RPC_FAR *ppenumAdvise) { return E_NOTIMPL; }
	STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus) { return E_NOTIMPL; }
	STDMETHOD(SetColorScheme)(LOGPALETTE *pLogpal) { return E_NOTIMPL; }

private:
    HRESULT PlaybackMedia();

private:
    WCHAR           m_wszMoniker[4096]; // large enough for max URL
};

#endif // !defined(_WMPHELPER_H__)
