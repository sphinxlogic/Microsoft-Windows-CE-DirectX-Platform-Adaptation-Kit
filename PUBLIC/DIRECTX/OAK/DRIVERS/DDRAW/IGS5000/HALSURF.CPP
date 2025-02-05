// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  Copyright (c) 1997-1998  Microsoft Corporation
//  COPYRIGHT (C) 1999, IGS TECHNOLOGIES, INC. (iGST)
//
// -----------------------------------------------------------------------------

#include "precomp.h"

OVERLAYSURF ovl[2];
FLIPRECORD	flipRec;
int			iOvlCnt=0;
int			iOvlMax=1;


HRESULT UpdateFlipStatus() {

	if (!flipRec.dwInFlip) {
		return DD_OK;
	}

	WORD curr = GetScanline();
	if (curr<flipRec.dwCurrLine) {
FlipOK:
		flipRec.dwInFlip = 0;
		return DD_OK;
	}
	else {
		flipRec.dwCurrLine = curr;
		DWORD dwCurrTicks=GetTickCount();
		if ((dwCurrTicks - flipRec.dwTicksAtFlip)> flipRec.dwTicksPerFrame) {
			goto FlipOK;
		}
	}

	return DDERR_WASSTILLDRAWING;
}


DWORD WINAPI HalFlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA pd )
{
	DEBUGENTER( HalFlipToGDISurface );
	/*
	typedef struct _DDHAL_FLIPTOGDISURFACEDATA
	{
		LPDDRAWI_DIRECTDRAW_GBL    lpDD;	     // driver struct
		DWORD			   dwToGDI;	     // TRUE if flipping to the GDI surface, FALSE if flipping away
		DWORD			   dwReserved;	     // reserved for future use
		HRESULT 		   ddRVal;	     // return value
		LPDDHAL_FLIPTOGDISURFACE   FlipToGDISurface; // PRIVATE: ptr to callback
	} DDHAL_FLIPTOGDISURFACEDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalCreateSurface( LPDDHAL_CREATESURFACEDATA pd )
{
	DEBUGENTER( HalCreateSurface );
	/*
	typedef struct _DDHAL_CREATESURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDSURFACEDESC		lpDDSurfaceDesc;// description of surface being created
	    LPDDRAWI_DDRAWSURFACE_LCL	FAR *lplpSList; // list of created surface objects
	    DWORD			dwSCnt; 	// number of surfaces in SList
	    HRESULT			ddRVal; 	// return value
	    LPDDHAL_CREATESURFACE	CreateSurface;	// PRIVATE: ptr to callback
	} DDHAL_CREATESURFACEDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

//////////////////////////// DDHAL_DDEXEBUFCALLBACKS ////////////////////////////

DWORD WINAPI HalCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pd )
{
	DEBUGENTER( HalCanCreateSurface );
	/*
	typedef struct _DDHAL_CANCREATESURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;				// driver struct
	    LPDDSURFACEDESC			lpDDSurfaceDesc;	// description of surface being created
	    DWORD					bIsDifferentPixelFormat;
												// pixel format differs from primary surface
	    HRESULT					ddRVal; 			// return value
	    LPDDHAL_CANCREATESURFACE	CanCreateSurface;
												// PRIVATE: ptr to callback
	} DDHAL_CANCREATESURFACEDATA;
	*/

	// Implementation

	/*
	 *  - Well, nothing if surf has color format match screen format.
	 *  - For Overlay: we support RGB8/16 and YUV422, doesn't care about
	 *    screen format.
	 *  - In DDRAWI.H, lpDDSurfaceDesc is declared as SURFACEDESC2 ???
	 */


	DDPIXELFORMAT	*pddpf	= &pd->lpDDSurfaceDesc->ddpfPixelFormat;
	DWORD			caps	= pd->lpDDSurfaceDesc->ddsCaps.dwCaps;

	if (pd->bIsDifferentPixelFormat) {
		if (caps & DDSCAPS_OVERLAY) {
			if (pddpf->dwFlags & DDPF_FOURCC) {
				switch (pddpf->dwFourCC) {
				case FOURCC_YUYV422:
					/* just in case App doesn't fill this */
					pddpf->dwYUVBitCount=16;
					goto CanCreate;
					break;
				default:
					goto CannotCreate;

				}
			}
			else if (pddpf->dwFlags & DDPF_RGB) {
				switch (pddpf->dwRGBBitCount) {
				case 8 :
				case 16:
					goto CanCreate;
				default:
					goto CannotCreate;
				}
			}
		}
		else {
		/*
		 * nav: Create plain surface???
		 * we better check pixel format here 'cos some games, they
		 * try to create 555 format 1st, if failed, they will create 565.
		 *
		 */

#ifdef FB16BPP
			if ((pddpf->dwRGBBitCount!=16)||
				(pddpf->dwRBitMask!=0xF800)) {
				goto CannotCreate;
			}
#else
			if (pddpf->dwRGBBitCount!=8) {
				goto CannotCreate;
			}
#endif
			goto CanCreate;
		}
	}


CanCreate:
	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
CannotCreate:
	pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalCreateExecuteBuffer( LPDDHAL_CREATESURFACEDATA pd )
{
	DEBUGENTER( HalCreateExecuteBuffer );
	/*
	typedef struct _DDHAL_CREATESURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDSURFACEDESC		lpDDSurfaceDesc;// description of surface being created
	    LPDDRAWI_DDRAWSURFACE_LCL	FAR *lplpSList; // list of created surface objects
	    DWORD			dwSCnt; 	// number of surfaces in SList
	    HRESULT			ddRVal; 	// return value
	    LPDDHAL_CREATESURFACE	CreateSurface;	// PRIVATE: ptr to callback
	} DDHAL_CREATESURFACEDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}


DWORD WINAPI HalDestroyExecuteBuffer( LPDDHAL_DESTROYSURFACEDATA pd )
{
	DEBUGENTER( HalDestroyExecutebuffer );
	/*
	typedef struct _DDHAL_DESTROYSURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_DESTROYSURFACE DestroySurface;// PRIVATE: ptr to callback
		BOOL						fDestroyGlobal;
	} DDHAL_DESTROYSURFACEDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalLock( LPDDHAL_LOCKDATA pd )
{
	DEBUGENTER( HalLock );
	/*
	typedef struct _DDHAL_LOCKDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    DWORD			bHasRect;	// rArea is valid
	    RECTL			rArea;		// area being locked
	    LPVOID			lpSurfData;	// pointer to screen memory (return value)
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_LOCK		Lock;		// PRIVATE: ptr to callback
	} DDHAL_LOCKDATA;
	*/

	// Implementation
	/*
	pd->ddRVal = UpdateFlipStatus();
	if (pd->ddRVal==DDERR_WASSTILLDRAWING) {
		if (pd->dwFlags & DDLOCK_WAIT) {
			while (UpdateFlipStatus()!=DD_OK);
		}
		else {
			return DDHAL_DRIVER_HANDLED;
		}
	}
	*/

	return DDGPELock(pd);

	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalUnlock( LPDDHAL_UNLOCKDATA pd )
{
	DEBUGENTER( HalUnlock );
	/*
	typedef struct _DDHAL_UNLOCKDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    HRESULT			ddRVal; 		// return value
	    LPDDHALSURFCB_UNLOCK	Unlock; 	// PRIVATE: ptr to callback
	} DDHAL_UNLOCKDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

//////////////////////////// DDHAL_DDSURFACECALLBACKS ////////////////////////////

DWORD WINAPI HalDestroySurface( LPDDHAL_DESTROYSURFACEDATA pd )
{
	DEBUGENTER( HalDestroySurface );
	/*
	typedef struct _DDHAL_DESTROYSURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_DESTROYSURFACE DestroySurface;// PRIVATE: ptr to callback
		BOOL						fDestroyGlobal;
	} DDHAL_DESTROYSURFACEDATA;
	*/

	// Implementation
    LPDDRAWI_DIRECTDRAW_GBL	pdrv;
    LPDDRAWI_DDRAWSURFACE_LCL	psurf;
    LPDDRAWI_DDRAWSURFACE_GBL	psurf_gbl;
	int e;

    pdrv      = pd->lpDD;
    psurf     = pd->lpDDSurface;
    psurf_gbl = psurf->lpGbl;



    if (psurf->ddsCaps.dwCaps & DDSCAPS_OVERLAY) {
		for (e=0; e<iOvlMax; e++) {
			if (psurf == ovl[e].pSrcSurf) {
				//if(e == 0)
					//VPE_EnableOverDoubleBuffer(0);
				if (iOvlCnt) {
					ovl[e].pSrcSurf=0;
					iOvlCnt--;
				}
			}
	}
	};
	return DDGPEDestroySurface(pd);
	//pd->ddRVal = DD_OK;
	//return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetSurfaceDesc(LPDDHAL_HALSETSURFACEDESCDATA pd)
{
	DEBUGENTER( HalSetSurfaceDesc );
	/*
		typedef struct _DDHAL_HALSETSURFACEDESCDATA
		{
		    DWORD		dwSize; 		// Size of this structure
		    LPDDRAWI_DDRAWSURFACE_LCL  lpDDSurface; // Surface
			LPDDSURFACEDESC lpddsd; 				// Description of surface
		    HRESULT		ddrval;
		} DDHAL_HALSETSURFACEDESCDATA;
	*/

	// Implementation
	pd->ddrval = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalFlip( LPDDHAL_FLIPDATA pd ) {
	int  e;
	long lOffset;
	int  scrheight = g_pGPE->ScreenHeight();

	DEBUGENTER( HalFlip );
	/*
	typedef struct _DDHAL_FLIPDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpSurfCurr;	// current surface
	    LPDDRAWI_DDRAWSURFACE_LCL	lpSurfTarg;	// target surface (to flip to)
	    DWORD			dwFlags;	// flags
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_FLIP		Flip;		// PRIVATE: ptr to callback
	} DDHAL_FLIPDATA;
	*/

	if (IS_BUSY) {
		if (pd->dwFlags & DDFLIP_WAIT) {
			WAIT_FOR_NOT_BUSY;
		}
		else {
			DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Graphics engine busy\r\n")));
			pd->ddRVal = DDERR_WASSTILLDRAWING;
			return DDHAL_DRIVER_HANDLED;
		}
	}

	//////
	//WAIT_FOR_VBLANK;
	//////
	if (!(pd->dwFlags & DDFLIP_WAIT)) {
		pd->ddRVal = UpdateFlipStatus();
		if (pd->ddRVal==DDERR_WASSTILLDRAWING) {
			return DDHAL_DRIVER_HANDLED;
		}
	}

	lOffset = pd->lpSurfTarg->lpGbl->fpVidMem - (DWORD)g_pVideoMemory;

	if (pd->lpSurfTarg->ddsCaps.dwCaps & DDSCAPS_OVERLAY) {
		for (e=0; e<iOvlMax; e++) {
			if (ovl[e].pSrcSurf == pd->lpSurfCurr) {
				break;
			}
		}
		if (e>=iOvlMax) {
			/* can not flip this surf */
			pd->ddRVal = DD_OK;
			return DDHAL_DRIVER_HANDLED;
		}
		/* we should flip if surf is current active one */
		while ( GetScanline()>=scrheight);		/* we can not set during VBlank*/
		while ( GetScanline()<(scrheight-5));	/* wait right before VBlank */
	SetVideoSrc(e, lOffset, (WORD)ovl[e].dwSrcPitch, (WORD)ovl[e].wFetch);
	}
	else {
		while ( GetScanline()>=scrheight);		/* we can not set during VBlank*/
		//while ( GetScanline()<100);				/* skip 1st certain scanlines */
		while ( GetScanline()<(scrheight-5));		/* we can not set during VBlank*/
		pd->dwFlags&=~DDFLIP_WAIT;				/* we wait already */
		DDGPEFlip(pd);							/* if you want to flip by yourself, remember swap 2 lcl surfaces */
	};

	flipRec.dwInFlip = 1;
	flipRec.dwTicksAtFlip = GetTickCount();
	flipRec.dwCurrLine = GetScanline();
	//RETAILMSG(1,(TEXT("Flip!\r\n")));
	DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Flip done\r\n")));

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetClipList( LPDDHAL_SETCLIPLISTDATA pd )
{
	DEBUGENTER( HalSetClipList );
	/*
	typedef struct _DDHAL_SETCLIPLISTDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_SETCLIPLIST	SetClipList;	// PRIVATE: ptr to callback
	} DDHAL_SETCLIPLISTDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}


DWORD WINAPI HalBlt( LPDDHAL_BLTDATA pd )
{
	DEBUGENTER( HalBlt );
	/*
	typedef struct _DDHAL_BLTDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDDestSurface;// dest surface
	    RECTL			rDest;		// dest rect
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSrcSurface; // src surface
	    RECTL			rSrc;		// src rect
	    DWORD			dwFlags;	// blt flags
	    DWORD			dwROPFlags;	// ROP flags (valid for ROPS only)
	    DDBLTFX			bltFX;		// blt FX
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_BLT		Blt;		// PRIVATE: ptr to callback
	} DDHAL_BLTDATA;
	*/

	// Implementation
	return DDGPEBlt(pd);
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

// not to be confused with
// DWORD WINAPI HalSetColorKey( LPDDHAL_DRVSETCOLORKEYDATA pd )
DWORD WINAPI HalSetColorKey( LPDDHAL_SETCOLORKEYDATA pd )
{
	DEBUGENTER( HalSetColorKey );
	/*
	typedef struct _DDHAL_DRVSETCOLORKEYDATA
	{
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    DWORD			dwFlags;	// flags
	    DDCOLORKEY			ckNew;		// new color key
	    HRESULT			ddRVal; 	// return value
	    LPDDHAL_SETCOLORKEY 	SetColorKey;	// PRIVATE: ptr to callback
	} DDHAL_DRVSETCOLORKEYDATA;
	*/

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalAddAttachedSurface( LPDDHAL_ADDATTACHEDSURFACEDATA pd )
{
	DEBUGENTER( HalAddAttachedSurface );
	/*
	typedef struct _DDHAL_ADDATTACHEDSURFACEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL		lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpSurfAttached; // surface to attach
	    HRESULT						ddRVal; 	// return value
	    LPDDHALSURFCB_ADDATTACHEDSURFACE AddAttachedSurface;
												// PRIVATE: ptr to callback
	} DDHAL_ADDATTACHEDSURFACEDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalGetBltStatus( LPDDHAL_GETBLTSTATUSDATA pd )
{
	DEBUGENTER( HalGetBltStatus );
	/*
	typedef struct _DDHAL_GETBLTSTATUSDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    DWORD			dwFlags;	// flags
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_GETBLTSTATUS	GetBltStatus;	// PRIVATE: ptr to callback
	} DDHAL_GETBLTSTATUSDATA;
	*/

	// Implementation

	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pd )
{
	DEBUGENTER( HalGetFlipStatus );
	/*
	typedef struct _DDHAL_GETFLIPSTATUSDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    DWORD			dwFlags;	// flags
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_GETFLIPSTATUS GetFlipStatus;	// PRIVATE: ptr to callback
	} DDHAL_GETFLIPSTATUSDATA;
	*/

	// Implementation
	pd->ddRVal = UpdateFlipStatus();
	//pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalUpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA pd ) {
	DEBUGENTER( HalUpdateOverlay );
	/*
//	typedef struct _DDHAL_UPDATEOVERLAYDATA
//	{
//	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
//	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDDestSurface;// dest surface
//	    RECTL			rDest;		// dest rect
//	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSrcSurface; // src surface
//	    RECTL			rSrc;		// src rect
//	    DWORD			dwFlags;	// flags
//	    DDOVERLAYFX 		overlayFX;	// overlay FX
//	    HRESULT			ddRVal; 	// return value
//	    LPDDHALSURFCB_UPDATEOVERLAY UpdateOverlay;	// PRIVATE: ptr to callback
//	} DDHAL_UPDATEOVERLAYDATA;
//	*/
//

#ifdef FB16BPP
	DWORD	dwScreenBpp=2;
#else
	DWORD	dwScreenBpp=1;
#endif
    LPDDRAWI_DDRAWSURFACE_LCL	    pSrcSurf;
    LPDDRAWI_DDRAWSURFACE_GBL	    pSrcSurf_Gbl;
    LPDDPIXELFORMAT		    pddpf;
    WORD			    wVCtrl,wVFmt;

    WORD    wo_dx1;
    WORD    wo_dy1;
    WORD    wo_dx2;
    WORD    wo_dy2;
    WORD    wo_dw;
    WORD    wo_dh;
    WORD    wo_sw;
    WORD    wo_sh;
    WORD    wo_fetch;
    WORD    wo_srcpitch;
    DWORD   po_srcptr,po_srcptr1;
	int		e;

	// Implementation
    pSrcSurf	 = pd->lpDDSrcSurface;
    pSrcSurf_Gbl = pSrcSurf->lpGbl;

	/*
	 * find out whether this surf is already initialized or new. If new
	 * try to alloc video engine for it.
	 */
	for (e=0; e<iOvlMax; e++) {
		if (ovl[e].pSrcSurf==pSrcSurf) {
			break;
		}
	}

	if (e>=iOvlMax) {
		/* well, we need alloc another video engine for it */
		if (iOvlCnt>=iOvlMax) {
			pd->ddRVal=DDERR_CURRENTLYNOTAVAIL;
			return DDHAL_DRIVER_HANDLED;
		}
		else {
			for (e=0; e<iOvlMax; e++) {
				if (!ovl[e].pSrcSurf)
					break;
			}
			ovl[e].pSrcSurf  = pSrcSurf;
			ovl[e].dwDstX1	 = ovl[e].dwDstY1   = 0xFFFF;
			ovl[e].dwDstW	 = ovl[e].dwDstH    = 0xFFFF;
			ovl[e].dwDstCKHi = ovl[e].dwDstCKLo = 0xFFFFFFFF;
			ovl[e].dwSrcCKHi = ovl[e].dwSrcCKLo = 0xFFFFFFFF;
			ovl[e].dwSrcPitch= pSrcSurf_Gbl->lPitch;
			ovl[e].dwSrcW	 = (DWORD) pSrcSurf_Gbl->wWidth;
			ovl[e].dwSrcH	 = (DWORD) pSrcSurf_Gbl->wHeight;
			ovl[e].dwFlags	 = 0;
			ovl[e].dwFlipOfs = 0;
			if (pSrcSurf->dwFlags & DDRAWISURF_HASPIXELFORMAT) {
				ovl[e].dwSurfBpp = (pSrcSurf_Gbl->ddpfSurface.dwRGBBitCount+1)>>3;
			}
			else {
				ovl[e].dwSurfBpp = dwScreenBpp;
			}
			iOvlCnt++;
		}
	}


	if (pd->dwFlags & DDOVER_HIDE) {
		VideoOff(e);
	}
	else {
		if (pd->dwFlags & DDOVER_SHOW) {
			VideoOn(e);
		}

		/* get src format if avail */
		if (pSrcSurf->dwFlags & DDRAWISURF_HASPIXELFORMAT) {
			pddpf=&pSrcSurf->lpGbl->ddpfSurface;
		}


		/* setup video src and dst */
		po_srcptr    = pSrcSurf_Gbl->fpVidMem - (DWORD)g_pVideoMemory;
		wo_srcpitch  = (WORD) pSrcSurf_Gbl->lPitch;
		po_srcptr   += pd->rSrc.top * wo_srcpitch + pd->rSrc.left * ovl[e].dwSurfBpp;
	wo_dx1	     = (WORD) pd->rDest.left;
	wo_dy1	     = (WORD) pd->rDest.top;
	wo_dx2	     = (WORD) pd->rDest.right;
	wo_dy2	     = (WORD) pd->rDest.bottom;
	wo_dw	     = (WORD) (wo_dx2-wo_dx1);
	wo_dh	     = (WORD) (wo_dy2-wo_dy1);
	wo_sw	     = (WORD) (pd->rSrc.right-pd->rSrc.left);
	wo_sh	     = (WORD) (pd->rSrc.bottom-pd->rSrc.top);
	wo_fetch     = (WORD) (wo_sw * ovl[e].dwSurfBpp);

		/* remember these value */
		ovl[e].dwSrcPitch	= wo_srcpitch;
		ovl[e].pSrcStart	= po_srcptr;
		ovl[e].wFetch		= wo_fetch;
		ovl[e].dwDstW		= wo_dw;
		ovl[e].dwDstH		= wo_dh;
		ovl[e].dwDstX1		= wo_dx1;
		ovl[e].dwDstY1		= wo_dy1;
		ovl[e].dwSrcW		= wo_sw;
		ovl[e].dwSrcH		= wo_sh;

		SetVideoSrc(e,po_srcptr,wo_srcpitch, wo_fetch);

    if (pd->dwFlags & DDOVER_AUTOFLIP)
    {
	    po_srcptr1	 = pSrcSurf->lpAttachList->lpAttached->lpGbl->fpVidMem - (DWORD)g_pVideoMemory;
		po_srcptr   += pd->rSrc.top * wo_srcpitch + pd->rSrc.left * ovl[e].dwSurfBpp;
		SetVideoBackBufferSrc(po_srcptr1);
		VPE_EnableOverDoubleBuffer(1);
    }
	else
		VPE_EnableOverDoubleBuffer(0);

		SetVideoWindow(e, wo_dx1, wo_dy1, wo_dx2, wo_dy2);
		SetVideoScale(e, wo_sw,wo_dw, wo_sh,wo_dh);

		/* setup video format */
	wVCtrl = WINKEY;
	if ((pSrcSurf->dwFlags&DDRAWISURF_HASPIXELFORMAT)&&
	    (pddpf->dwFlags & DDPF_FOURCC)) {
	    wVFmt = VFMT_YUV422;
	}
	else if ((pSrcSurf->dwFlags&DDRAWISURF_HASPIXELFORMAT)&&
		 (pddpf->dwFlags & DDPF_RGB)) {
	    switch (ovl[e].dwSurfBpp) {
	    case 1:
		wVFmt=VFMT_RGB8;
				wVCtrl|=GOTHRU_DAC;
		break;
	    case 2:
		if (pddpf->dwRBitMask == 0x00007C00L)
		    wVFmt=VFMT_RGB555;
		else
		    wVFmt=VFMT_RGB565;
		break;
	    case 3:
		wVFmt=VFMT_RGB888;
		break;
	    case 4:
		wVFmt=VFMT_RGB8888;
		break;
	    }
	}
	else {
	    switch (ovl[e].dwSurfBpp) {
	    case 1:
		wVFmt=VFMT_RGB8;
		wVCtrl|=GOTHRU_DAC;
		break;
	    case 2:
		wVFmt=VFMT_RGB565;
		break;	    //our DRV support 16bpp
	    case 3:
		wVFmt=VFMT_RGB888;
		break;
	    case 4:
		wVFmt=VFMT_RGB8888;
		break;
	    }
	}
		VideoFormat(e,wVFmt);
		VideoControl(e,wVCtrl);
	}

	/* check colorkey here */

    /* for dest colorkey */
    if (pd->dwFlags & DDOVER_KEYDESTOVERRIDE) {
	BYTE	  bR, bG, bB;
	DWORD	  dwKey;

	dwKey=pd->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
		ovl[e].dwDstCKLo = dwKey;
		switch (dwScreenBpp) {
		case 1:
			bG = bB = bR = (BYTE) (dwKey);
			break;
		case 3:
		case 4:
			bR = (BYTE) ( dwKey );
			bG = (BYTE) (( dwKey & 0xff00L) >> 8 );
			bB = (BYTE) (( dwKey >> 16 ));
			break;
		case 2:
			bR = (BYTE) ( ( dwKey & 0xf800L) >> 8 );
			bG = (BYTE) ( ( dwKey & 0x7e0L) >> 3 );
			bB = (BYTE) ( ( dwKey & 0x1fL ) << 3 );
			break;
		}
	SetColorKey(e,bR,bG,bB);
	ColorkeyOn(e);
    }
    else {
		ovl[e].dwDstCKLo = 0xFFFFFFFF;
	ColorkeyOff(e);
    }


	/* for source color key */
    if (pd->dwFlags & DDOVER_KEYSRCOVERRIDE) {
	BYTE	  bHiR, bHiG, bHiB;
	BYTE	  bLoR, bLoG, bLoB;
	DWORD	  dwLoKey=pd->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
	DWORD	  dwHiKey=pd->overlayFX.dckSrcColorkey.dwColorSpaceHighValue;

		ovl[e].dwSrcCKHi = dwHiKey;
		ovl[e].dwSrcCKLo = dwLoKey;
		switch (dwScreenBpp) {
		case 1:
			bHiG = bHiB = bHiR = (BYTE) (dwHiKey);
			bLoG = bLoB = bLoR = (BYTE) (dwLoKey);
			break;
		case 3:
		case 4:
			bHiR = (BYTE) ( dwHiKey );
			bHiG = (BYTE) ((dwHiKey & 0xff00L) >> 8 );
			bHiB = (BYTE) ((dwHiKey >> 16 ));
			bLoR = (BYTE) ( dwLoKey );
			bLoG = (BYTE) ((dwLoKey & 0xff00L) >> 8 );
			bLoB = (BYTE) ((dwLoKey >> 16 ));
			break;
		case 2:
			bHiR = (BYTE) ((dwHiKey & 0xf800L) >> 8 );
			bHiG = (BYTE) ((dwHiKey & 0x7e0L) >> 3 );
			bHiB = (BYTE) ((dwHiKey & 0x1fL ) << 3 );
			bLoR = (BYTE) ((dwLoKey & 0xf800L) >> 8 );
			bLoG = (BYTE) ((dwLoKey & 0x7e0L) >> 3 );
			bLoB = (BYTE) ((dwLoKey & 0x1fL ) << 3 );
			break;
		}
		SetChromaKey(e,bHiR, bHiG, bHiB, bLoR, bLoG, bLoB);
		ChromaOn(e);
	}
    else {
		ChromaOff(e);
		ovl[e].dwSrcCKHi = 0xFFFFFFFF;
		ovl[e].dwSrcCKLo = 0xFFFFFFFF;
	}


	/* new *** new *** new: check alpha stuff */
	if (pd->dwFlags&DDOVER_ALPHASRCCONSTOVERRIDE) {
		DEBUGMSG(1,(TEXT("AlphaSrcConst supported: %d\r\n"),pd->overlayFX.dwAlphaSrcConst));
//		DebugBreak();
	}


	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA pd ) {

	int e;
	int scrheight = g_pGPE->ScreenHeight();

	DEBUGENTER( HalSetOverlayPosition );
	/*
	typedef struct _DDHAL_SETOVERLAYPOSITIONDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSrcSurface; // src surface
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDDestSurface;// dest surface
	    LONG			lXPos;		// x position
	    LONG			lYPos;		// y position
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_SETOVERLAYPOSITION SetOverlayPosition;
												// PRIVATE: ptr to callback
	} DDHAL_SETOVERLAYPOSITIONDATA;
	*/

	// Implementation
	for (e=0; e<iOvlMax; e++) {
		if (ovl[e].pSrcSurf==pd->lpDDSrcSurface) {
			break;
		}
	}
	if (e>=iOvlMax) {
		pd->ddRVal = DD_OK;
		return DDHAL_DRIVER_HANDLED;
	}
	ovl[e].dwDstX1 = (DWORD) pd->lXPos;
	ovl[e].dwDstY1 = (DWORD) pd->lYPos;
	while (GetScanline() >= scrheight);
	while (GetScanline() < (scrheight-5));
	SetVideoWindow(e, (WORD) pd->lXPos, (WORD) pd->lYPos, (WORD) (ovl[e].dwDstX1+ovl[e].dwDstW),
					(WORD) (ovl[e].dwDstY1+ovl[e].dwDstH));

	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetPalette( LPDDHAL_SETPALETTEDATA pd )
{
	DEBUGENTER( HalSetPalette );
	/*
	typedef struct _DDHAL_SETPALETTEDATA
	{
	    LPDDRAWI_DIRECTDRAW_GBL	lpDD;		// driver struct
	    LPDDRAWI_DDRAWSURFACE_LCL	lpDDSurface;	// surface struct
	    LPDDRAWI_DDRAWPALETTE_GBL	lpDDPalette;	// palette to set to surface
	    HRESULT			ddRVal; 	// return value
	    LPDDHALSURFCB_SETPALETTE	SetPalette;	// PRIVATE: ptr to callback
	    BOOL			Attach; 	// attach this palette?
	} DDHAL_SETPALETTEDATA;
	*/

	// Implementation
	pd->ddRVal = DD_OK;

	return DDHAL_DRIVER_HANDLED;
}

