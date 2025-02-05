/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1998-1999  Microsoft Corporation

Module Name:  animthrd.h

Abstract:  handles the animation thread

Functions:

Notes: This class encapsulates the thread that runs the animation


--*/

#ifndef _ANIMTHRD_H_
#define _ANIMTHRD_H_

#ifndef NOCOMMANDBAR

#define ANIMATIONINTERVAL	400

class CAnimThread
{
public:
	CAnimThread(CMainWnd *pMainWnd);
	~CAnimThread();

public:
	VOID StartThread();
	VOID StartAnimation();
	VOID StopAnimation();

private:
	DWORD Run();
	VOID Paint();
	static DWORD WINAPI AnimationThreadProc(LPVOID lpParam);
	static LRESULT CALLBACK AnimationWndProc(HWND hwnd, UINT message,
												WPARAM wParam, LPARAM lParam);

private:
	DWORD		_dwThreadID;	//the thread id
	LONG 		_nIndex;		//the index of the frame in the bitmap currently being shown
	HBITMAP		_hBitmap;		//handle to the animation bitmap
	HWND		_hwndCmdband;	//the command band on which to create the animation window
	HWND		_hwndAnimation;	//the animation window
	LONG		_bRun;			//is the animation being run

	WNDPROC		_lpfnAnimationProc;

    CMainWnd    *_pMainWnd;

};

#endif	//NOCOMMANDBAR
#endif	//_ANIMTHRD_H_
