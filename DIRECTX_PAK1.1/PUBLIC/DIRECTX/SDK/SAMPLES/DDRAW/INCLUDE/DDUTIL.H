/*==========================================================================
 *
 *  Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
 *
 *  File:       ddutil.cpp
 *  Content:    Routines for loading bitmap and palettes from resources
 *
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

extern IDirectDrawPalette *     DDLoadPalette(HINSTANCE hInstance, IDirectDraw4 * pdd, LPCTSTR szBitmap);
extern IDirectDrawSurface4 *    DDLoadBitmap(HINSTANCE hInstance, IDirectDraw4 * pdd, LPCTSTR szBitmap);
extern HBITMAP                  DDGetBitmapHandle(HINSTANCE hInstance, LPCTSTR szBitmap);
extern HRESULT                  DDReLoadBitmap(HINSTANCE hInstance, IDirectDrawSurface4 * pdds, LPCTSTR szBitmap);
extern HRESULT                  DDCopyBitmap(IDirectDrawSurface4 * pdds, HBITMAP hbm, int x, int y, int dx, int dy);
extern DWORD                    DDColorMatch(IDirectDrawSurface4 * pdds, COLORREF rgb);
extern HRESULT                  DDSetColorKey(IDirectDrawSurface4 * pdds, COLORREF rgb);
extern HRESULT                  DDSetColorKey2(IDirectDrawSurface4 * pdds, COLORREF rgb);

#ifdef __cplusplus
}
#endif	/* __cplusplus */

