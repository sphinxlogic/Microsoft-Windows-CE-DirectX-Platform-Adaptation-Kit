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
// WMPHelper.cpp : 

#include "stdafx.h"
#include "wmphlpr.h"
#include "wmphelpr.h"
#include <wininet.h>


HRESULT WMPHelper::PlaybackMedia()
{
    HRESULT hr = S_OK;

    // This code demonstrates 2 ways to playback media when a <a href="..."> anchor is
    // clicked on in the browser. The if (1) part communicates to the running instance
    // of iesample.exe to cause a navigate to a media playback page in the browser.
    // The else part of the code can be used to launch a seperate playback application
    // that the OEM/ISV could provide.
    if (1)
    {
        // copy the URL to global memory
        WMPHELPERMSG *pWMPMsg = (WMPHELPERMSG*) CoTaskMemAlloc(((wcslen(m_wszMoniker)
            + WMPHELPER_URL_LEN) * sizeof(WCHAR)) + sizeof(WMPHELPERMSG));
        if (pWMPMsg)
        {
            pWMPMsg->dwThreadId = GetCurrentThreadId();
            // the url of the media page with the embedded control
            wcscpy(pWMPMsg->wszURL, WMPHELPER_URL_STR);
            // append the URL
            wcscat(pWMPMsg->wszURL, m_wszMoniker);
            // make sure we have a unique message id for our private communication
            UINT uiMsgId = RegisterWindowMessage(WMP_USER_MESSAGE_STR);
            DEBUGMSG(1, (L"WMP message PID: %08X, THRD: %08X, URL: %s\n",
                GetOwnerProcess(), GetCurrentThreadId(), pWMPMsg->wszURL));
            // now broadcast the message to all top level windows
            PostMessage(HWND_BROADCAST, uiMsgId, (WPARAM) GetOwnerProcess(), (LPARAM) pWMPMsg);
            // the memory is freed by the IESample wndproc
        }
    }
    else
    {
        PROCESS_INFORMATION pi;

        if (CreateProcess(L"mplayer2.exe", m_wszMoniker, NULL,
                NULL, FALSE, 0, NULL, NULL, NULL, &pi))
        {
            hr = S_OK;
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}


STDMETHODIMP WMPHelper::GetClassID(CLSID *pclsid)
{
	DEBUGMSG(1,(TEXT("Entering:GetClassID\n")));
	if (pclsid == NULL)
	{
		return (E_INVALIDARG);
	}
    *pclsid = CLSID_WMPHelper;
	return S_OK;
}

STDMETHODIMP WMPHelper::IsDirty()
{
	DEBUGMSG(1,(TEXT("Entering:IsDirty\n")));
    return E_NOTIMPL;
}

STDMETHODIMP WMPHelper::Load(BOOL fFullyAvailable, IMoniker *pmkName, IBindCtx *pbctx, DWORD grfMode)
{
    DEBUGMSG(1,(TEXT("Entering:Load\n")));
    HRESULT hr;
    TCHAR   *pchURL = NULL;

    if (pmkName == NULL)
    {
        return E_INVALIDARG;
    }

    // Get the URL from the display name of the moniker:
    hr = pmkName->GetDisplayName(pbctx, NULL, &pchURL);
    if (SUCCEEDED(hr))
    {
        // save the URL
        wcscpy(m_wszMoniker, pchURL);
        // play the media
        hr = PlaybackMedia();
        // free the URL memory
        CoTaskMemFree(pchURL);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

STDMETHODIMP WMPHelper::Save(IMoniker *pmkName, LPBC pBCtx, BOOL fRemember)
{
    DEBUGMSG(1,(TEXT("Entering:Save\n")));
	return E_NOTIMPL;
}

STDMETHODIMP WMPHelper::SaveCompleted(IMoniker *pmkName, LPBC pBCtx)
{
    DEBUGMSG(1,(TEXT("Entering:SaveCompleted\n")));
    return E_NOTIMPL;
}

STDMETHODIMP WMPHelper::GetCurMoniker(IMoniker **ppmkName)
{
    DEBUGMSG(1,(TEXT("Entering:GetCurMoniker\n")));
    return E_NOTIMPL;
}
