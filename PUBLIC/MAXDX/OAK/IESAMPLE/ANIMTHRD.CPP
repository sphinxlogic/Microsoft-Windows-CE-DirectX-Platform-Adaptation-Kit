/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998-1999  Microsoft Corporation

Module Name:  animthrd.cpp

Abstract:  handles the animation thread

Functions:

Notes: This class encapsulates the thread that runs the animation


--*/

#include "Precomp.h"
#include "resource.h"
#include "mainwnd.h"
#include "animthrd.h"

#ifndef NOCOMMANDBAR

#define MAXFRAMES	46		//the number of frames in the bitmap

CAnimThread::CAnimThread(CMainWnd *pMainWnd)
{
	_nIndex = 0;
	_hBitmap = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_ANIMATION));
	_bRun = FALSE;
    _pMainWnd = pMainWnd;

}

CAnimThread::~CAnimThread()
{
	DeleteObject(_hBitmap);
}

VOID CAnimThread::StartThread()
{
	_hwndCmdband = _pMainWnd->GetCommandBand();
	CreateThread(NULL, 0, AnimationThreadProc, this, 0, &_dwThreadID);
}


DWORD WINAPI CAnimThread::AnimationThreadProc(LPVOID lpParam)
{
	CAnimThread *pAnimation = (CAnimThread *)lpParam;
	return pAnimation->Run();
}

DWORD CAnimThread::Run()
{
	_hwndAnimation = CreateWindowEx(0, _T("STATIC"), NULL, WS_CHILD|WS_VISIBLE|WS_DISABLED,
					0, 0, ANIMATIONWIDTH, ANIMATIONHEIGHT, _hwndCmdband, NULL,
					g_hInstance, NULL);

	if (!_hwndAnimation)
		return FALSE;

	SetWindowLong(_hwndAnimation, GWL_USERDATA, (DWORD)this);

	REBARBANDINFO rbbi[1];
	rbbi[0].cbSize = sizeof(REBARBANDINFO);
	rbbi[0].fMask = RBBIM_ID|RBBIM_STYLE|RBBIM_CHILD|RBBIM_SIZE;
	rbbi[0].cx = ANIMATIONWIDTH;
	rbbi[0].wID = ID_ANIMATIONWINDOW;
	rbbi[0].fStyle = RBBS_NOGRIPPER ;
	rbbi[0].hwndChild = _hwndAnimation;

	if (!CommandBands_AddBands(_hwndCmdband, g_hInstance, 1, rbbi))
		return FALSE;
	
	if (0 == SetTimer(_hwndAnimation, ID_ANIMATIONTIMER, ANIMATIONINTERVAL, NULL))
		return FALSE;

	_lpfnAnimationProc = (WNDPROC )SetWindowLong(_hwndAnimation, GWL_WNDPROC, (LONG )AnimationWndProc);

	MSG msg;
	while(GetMessage(&msg, _hwndAnimation, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	KillTimer(_hwndAnimation, ID_ANIMATIONTIMER);

	return msg.wParam;
}

LRESULT CALLBACK CAnimThread::AnimationWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    CAnimThread *pthrdAnimation = (CAnimThread*)GetWindowLong(hwnd, GWL_USERDATA);

	switch(message)
	{
	case WM_TIMER:
		pthrdAnimation->Paint();
		return 0L;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}	
	return CallWindowProc(pthrdAnimation->_lpfnAnimationProc, hwnd, message, wParam, lParam);
}


VOID CAnimThread::Paint()
{
	HDC hDC = GetDC(_hwndAnimation);
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hBmpOld = (HBITMAP )SelectObject(hMemDC, _hBitmap);
	BitBlt(hDC, 0, 0, ANIMATIONWIDTH, ANIMATIONHEIGHT,
					hMemDC, 0, _nIndex*ANIMATIONHEIGHT, SRCCOPY);
	SelectObject(hMemDC, hBmpOld);
	DeleteDC(hMemDC);
	ReleaseDC(_hwndAnimation, hDC);
	if (_bRun)
	{
		InterlockedIncrement(&_nIndex);
		if(_nIndex == MAXFRAMES)
		{
			LONG lVal = 0;
			InterlockedExchange(&_nIndex, lVal);
		}
	}
}

//StartAnimation and StopAnimation are called from the main thread, to control the animation
VOID CAnimThread::StartAnimation()
{
	LONG dwVal = TRUE;
	InterlockedExchange(&_bRun, dwVal);
}

VOID CAnimThread::StopAnimation()
{
	LONG lVal = 0;
	InterlockedExchange(&_bRun, lVal);
	InterlockedExchange(&_nIndex, lVal);
}

#endif	//NOCOMMANDBAR
