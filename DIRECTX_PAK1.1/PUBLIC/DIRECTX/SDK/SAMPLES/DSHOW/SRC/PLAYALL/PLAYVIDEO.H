//-----------------------------------------------------------------------------
// File: PlayVideo.h
//
// Desc: Header file for for DirectShow sample
//
//
// Copyright (c) 1998-2000 Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------

#ifndef PLAY_VID_H
#define PLAY_VID_H

//-----------------------------------------------------------------------------
// External function-prototypes
//-----------------------------------------------------------------------------
HRESULT InitDirectShow();
HRESULT FreeDirectShow();
HRESULT PrepareMedia(LPTSTR szPath, unsigned int nRepeats);
HRESULT CheckMovieState(BOOL *pbComplete);
HRESULT RunMedia();
HRESULT StopMedia();
HRESULT PlayFile(LPTSTR pszFile, int nPlayCount);

void RetailOutput(TCHAR *tszErr, ...);

#endif // !defined(PLAY_VID_H)
