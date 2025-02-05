// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  Copyright (c) 1997-2000  Microsoft Corporation
//
// -----------------------------------------------------------------------------

#include "precomp.h"

DWORD WINAPI HalFlipToGDISurface( LPDDHAL_FLIPTOGDISURFACEDATA pd )
{
    DEBUGENTER( HalFlipToGDISurface );
    /*
    typedef struct _DDHAL_FLIPTOGDISURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL    lpDD;             // driver struct
        DWORD                      dwToGDI;          // TRUE if flipping to the
                                                     // GDI surface, FALSE if 
                                                     // flipping away
        DWORD                      dwReserved;       // reserved for future use
        HRESULT                    ddRVal;           // return value
        LPDDHAL_FLIPTOGDISURFACE   FlipToGDISurface; // PRIVATE: ptr to callback
    } DDHAL_FLIPTOGDISURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

// retrieve pFormat and pPixelFormat based on pDDPF
SCODE 
DetectPixelFormat(
    DDPIXELFORMAT *pDDPF,
    EGPEFormat *pFormat,
    EDDGPEPixelFormat *pPixelFormat
    )
{
    if( pDDPF->dwFlags & DDPF_RGB )
    {
        if( !(pDDPF->dwFlags & DDPF_ALPHAPIXELS) ) 
		{
            pDDPF->dwRGBAlphaBitMask = 0;
		}
        if( pDDPF->dwRGBBitCount == 4 ) 
		{
            *pPixelFormat = ddgpePixelFormat_4bpp;
		}
        else if( pDDPF->dwRGBBitCount == 8 ) 
		{
            *pPixelFormat = ddgpePixelFormat_8bpp;
		}
        else if( pDDPF->dwRGBBitCount == 16 )
        {
            if( pDDPF->dwRGBAlphaBitMask == 0x0000 && 
                pDDPF->dwRBitMask == 0xF800 )
            {
                DEBUGMSG(1, (TEXT("565 surface detected\r\n") ));
                *pPixelFormat = ddgpePixelFormat_565;
            }
            else if( pDDPF->dwRGBAlphaBitMask == 0x8000 && 
                     pDDPF->dwRBitMask == 0x7C00 )
            {
                DEBUGMSG(1, (TEXT("1555 surface detected\r\n") ));
                *pPixelFormat = ddgpePixelFormat_5551;
            }
            else if( pDDPF->dwRGBAlphaBitMask == 0xF000 && 
				pDDPF->dwRBitMask == 0x0F00 ) 
			{
                DEBUGMSG(1, (TEXT("4444 surface detected\r\n") ));
                *pPixelFormat = ddgpePixelFormat_4444;
			}
            else if( pDDPF->dwRGBAlphaBitMask == 0x0000 && 
                     pDDPF->dwRBitMask == 0x7C00 )
			{
                DEBUGMSG(1, (TEXT("0555 surface detected\r\n") ));
                *pPixelFormat = ddgpePixelFormat_5550;
			}
            else {
                DEBUGMSG(1, (TEXT("DetectFormat: Unsupported 16-bit format\r\n") ));
                return DDERR_UNSUPPORTEDFORMAT;
			}
        }
        else if( pDDPF->dwRGBBitCount == 24 )
        {
            if( pDDPF->dwRGBAlphaBitMask == 0x000000 && 
                pDDPF->dwRBitMask == 0xFF0000 ) 
			{
                DEBUGMSG(1, (TEXT("8880 surface detected\r\n") ));
                *pPixelFormat = ddgpePixelFormat_8880;
			}
            else 
			{
                DEBUGMSG(1, (TEXT("DetectFormat: Unsupported 24-bit format\r\n") ));
                return DDERR_UNSUPPORTEDFORMAT;
			}
        }
        else if( pDDPF->dwRGBBitCount == 32 )
        {
            if( pDDPF->dwRGBAlphaBitMask == 0xFF000000 && 
                pDDPF->dwRBitMask == 0x00FF0000 ) 
			{
                DEBUGMSG(1, (TEXT("8888 surface detected\r\n") ));
                *pPixelFormat = ddgpePixelFormat_8888;
			}
            else
			{
                DEBUGMSG(1, (TEXT("DetectFormat: Unsupported 32-bit format\r\n") ));
                return DDERR_UNSUPPORTEDFORMAT;
			}
        }
        else
		{
            DEBUGMSG(1, (TEXT("DetectFormat: Unsupported bit-depth (%d)\r\n"), pDDPF->dwRGBBitCount));
            return DDERR_UNSUPPORTEDFORMAT;
		}
    }
    else if( pDDPF->dwFlags & DDPF_ZBUFFER )
    {
        //*pPixelFormat = ddgpePixelFormat_raw;
        return DDERR_UNSUPPORTEDFORMAT;
    }
    else if( pDDPF->dwFlags & DDPF_FOURCC )
    {
        if( pDDPF->dwFourCC == FOURCC_YUYV422 ) {
             DEBUGMSG(1, (TEXT("DetectFormat: YUYV422 surface detected\r\n") ));
            *pPixelFormat = ddgpePixelFormat_YUYV422;
		}
        else if( pDDPF->dwFourCC == FOURCC_YUY2422 ) {
             DEBUGMSG(1, (TEXT("DetectFormat: YUY2422 surface detected\r\n") ));
            *pPixelFormat = ddgpePixelFormat_YUY2422;
		}
#ifdef SUPPORT_UYVY422
        else if( pDDPF->dwFourCC == FOURCC_UYVY422 ) {
             DEBUGMSG(1, (TEXT("DetectFormat: YUYV422 surface detected\r\n") ));
            *pPixelFormat = ddgpePixelFormat_UYVY422;
		}
#endif
        else {
            DEBUGMSG(1, (TEXT("DetectFormat: Unsupported FOURCC %08x\r\n"), pDDPF->dwFourCC ));
            return DDERR_UNSUPPORTEDFORMAT;
		}
    }
    else
    {
        DEBUGMSG(1, (TEXT("DetectFormat: Unsupported Non-RGB, Non-FOURCC surface.\r\n")));
        return DDERR_UNSUPPORTEDFORMAT;
    }
    *pFormat = EDDGPEPixelFormatToEGPEFormat[*pPixelFormat];

    DEBUGMSG(1, (TEXT("Detected surface pixelFormat, format = %d, %d\r\n"), 
                  *pPixelFormat, *pFormat));

    return DD_OK;
}

DWORD WINAPI HalCreateSurface( LPDDHAL_CREATESURFACEDATA pd )
{
    DEBUGENTER( HalCreateSurface );
    /*
    typedef struct _DDHAL_CREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDSURFACEDESC             lpDDSurfaceDesc;// description of surface 
                                                    // being created
        LPDDRAWI_DDRAWSURFACE_LCL   FAR *lplpSList; // list of created surface 
                                                    // objects
        DWORD                       dwSCnt;         // number of surfaces in 
                                                    // SList
        HRESULT                     ddRVal;         // return value
        LPDDHAL_CREATESURFACE       CreateSurface;  // PRIVATE: ptr to callback
    } DDHAL_CREATESURFACEDATA;
    */
    EGPEFormat format;
    EDDGPEPixelFormat pixelFormat;
    EDDGPEPixelFormat primaryPixelFormat;
    DWORD dwFlags = pd->lpDDSurfaceDesc->dwFlags;
    DWORD dwCaps = pd->lpDDSurfaceDesc->ddsCaps.dwCaps;

    // get the pixel format for the primary surface
    GPEMode modeInfo;
    SCODE sc = g_pGPE->GetModeInfo(&modeInfo, g_pGPE->GetPhysicalModeId());
    if (FAILED(sc))
    {
        DEBUGMSG(1, (TEXT("Error: can't get mode info\r\n")));
        pd->ddRVal = DDERR_GENERIC;
        return DDHAL_DRIVER_HANDLED;
    }

    GPEModeEx modeInfoEx;
    sc = g_pGPE->GetModeInfoEx(&modeInfoEx, g_pGPE->GetPhysicalModeId());
    if (FAILED(sc))
    {
        primaryPixelFormat = EGPEFormatToEDDGPEPixelFormat[modeInfo.format];
    }
    else
    {
        primaryPixelFormat = modeInfoEx.ePixelFormat;
    }

    // get the pixel format for the surface to be created
    if (dwFlags & DDSD_PIXELFORMAT)
    {
        pd->ddRVal = DetectPixelFormat(
                         &pd->lpDDSurfaceDesc->ddpfPixelFormat,
                         &format,
                         &pixelFormat);
        if (FAILED(pd->ddRVal))
        {
            DEBUGMSG(1, (TEXT("HalCreateSurface: unsupported format\r\n")));
            return DDHAL_DRIVER_HANDLED;
        }
    }
    else
    {
        pixelFormat = primaryPixelFormat;
    }

    // return failure if requested a non-overlay surface
    // in video memory but with a different pixel format
    // from the primary surface
    if ((!(dwCaps & DDSCAPS_OVERLAY)) && 
        (dwCaps & DDSCAPS_VIDEOMEMORY) &&
        (pixelFormat != primaryPixelFormat))
    {
        DEBUGMSG(1, (TEXT("HalCreateSurface: non-overlay video memory surface must have same format as primary surface\r\n")));
        pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
        return DDHAL_DRIVER_HANDLED;
    }

    return DDGPECreateSurface(pd);
}

//////////////////////////// DDHAL_DDEXEBUFCALLBACKS ////////////////////////////

DWORD WINAPI HalCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pd )
{
    DEBUGENTER( HalCanCreateSurface );
    /*
    typedef struct _DDHAL_CANCREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL lpDD;               // driver struct
        LPDDSURFACEDESC         lpDDSurfaceDesc;    // description of surface 
                                                    // being created
        DWORD                   bIsDifferentPixelFormat;
                                                    // pixel format differs 
                                                    // from primary surface
        HRESULT                 ddRVal;             // return value
        LPDDHAL_CANCREATESURFACE CanCreateSurface;  // PRIVATE: ptr to callback
    } DDHAL_CANCREATESURFACEDATA;
    */

	DDPIXELFORMAT	*pddpf	= &pd->lpDDSurfaceDesc->ddpfPixelFormat;
	DWORD			caps	= pd->lpDDSurfaceDesc->ddsCaps.dwCaps;


	// BUGBUG
	// Apparently the S3 Virge STATUS_1 register is broken and
	// doesn't really tell us when we're in Vertical Blank.
	// This means we can't really flip without horrendous artifacts
	// For now, we simply disable flipping surfaces.
	// Someday, go off and look at the streams processor to see if
	// we can get true HW Flipping surfaces to work
	if ((caps & DDSCAPS_FLIP) && (caps & DDSCAPS_OVERLAY)) {
		DEBUGMSG(1, (TEXT("Flipping overlays not supported\r\n")));
		goto CannotCreate;
	}

        // We do not allow system memory primaries.

        if ((caps & DDSCAPS_PRIMARYSURFACE) &&
            (caps & DDSCAPS_SYSTEMMEMORY)) {
          goto CannotCreate;
        }

	if (pd->bIsDifferentPixelFormat) {
		if (caps & DDSCAPS_OVERLAY) {			
			if (pddpf->dwFlags & DDPF_FOURCC) {
			DEBUGMSG(1, (TEXT("HalCanCreateSurface FOURCC = %08x ('%c','%c','%c','%c')\r\n")
				,  pddpf->dwFourCC      
				, (pddpf->dwFourCC      ) & 0xff
				, (pddpf->dwFourCC >>  8) & 0xff
				, (pddpf->dwFourCC >> 16) & 0xff
				, (pddpf->dwFourCC >> 24) & 0xff
				));
				switch (pddpf->dwFourCC) {
#ifdef SUPPORT_UYVY422
				case FOURCC_UYVY422:
#endif
                                case FOURCC_YUY2422:
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
			DEBUGMSG(1, (TEXT("HalCanCreateSurface RGBBitCount = %d\r\n"), pddpf->dwRGBBitCount));
				switch (pddpf->dwRGBBitCount) {
				case 8 :
				case 16:
					goto CanCreate;
				default:
					goto CannotCreate;
				}		
			}
		}
	}

CanCreate:
	DEBUGMSG(1, (TEXT("HalCanCreateSurface: OK\r\n")));
	pd->ddRVal = DD_OK;
	return DDHAL_DRIVER_HANDLED;

CannotCreate:
	DEBUGMSG(1, (TEXT("HalCanCreateSurface: Unsupported\r\n")));
	pd->ddRVal = DDERR_UNSUPPORTEDFORMAT;
	return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalCreateExecuteBuffer( LPDDHAL_CREATESURFACEDATA pd )
{
    DEBUGENTER( HalCreateExecuteBuffer );
    /*
    typedef struct _DDHAL_CREATESURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDSURFACEDESC             lpDDSurfaceDesc;// description of surface 
                                                    // being created
        LPDDRAWI_DDRAWSURFACE_LCL   FAR *lplpSList; // list of created surface 
                                                    // objects
        DWORD                       dwSCnt;         // number of surfaces in 
                                                    // SList
        HRESULT                     ddRVal;         // return value
        LPDDHAL_CREATESURFACE       CreateSurface;  // PRIVATE: ptr to callback
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
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_DESTROYSURFACE DestroySurface;// PRIVATE: ptr to callback
        BOOL                        fDestroyGlobal;
    } DDHAL_DESTROYSURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

#ifndef NEWHAL

DWORD WINAPI HalLock( LPDDHAL_LOCKDATA pd )
{
    DEBUGENTER( HalLock );
    /*
    typedef struct _DDHAL_LOCKDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       bHasRect;       // rArea is valid
        RECTL                       rArea;          // area being locked
        LPVOID                      lpSurfData;     // pointer to screen memory
                                                    // (return value)
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_LOCK          Lock;           // PRIVATE: ptr to callback
    } DDHAL_LOCKDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

#else

DWORD WINAPI HalLock( LPDDHAL_LOCKDATA pd )
{
    DEBUGENTER( HalLock );

    return ((S3Virge *)g_pGPE)->Lock(pd);
}

#endif

DWORD WINAPI HalUnlock( LPDDHAL_UNLOCKDATA pd )
{
    DEBUGENTER( HalUnlock );
    /*
    typedef struct _DDHAL_UNLOCKDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_UNLOCK        Unlock;         // PRIVATE: ptr to callback
    } DDHAL_UNLOCKDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

//////////////////////////// DDHAL_DDSURFACECALLBACKS /////////////////////////

DWORD WINAPI HalDestroySurface( LPDDHAL_DESTROYSURFACEDATA pd )
{
    DEBUGENTER( HalDestroySurface );
    /*
    typedef struct _DDHAL_DESTROYSURFACEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_DESTROYSURFACE DestroySurface;// PRIVATE: ptr to callback
        BOOL                        fDestroyGlobal;
    } DDHAL_DESTROYSURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

DWORD WINAPI HalSetSurfaceDesc(LPDDHAL_HALSETSURFACEDESCDATA pd)
{
    DEBUGENTER( HalSetSurfaceDesc );
    /*
        typedef struct _DDHAL_HALSETSURFACEDESCDATA
        {
            DWORD           dwSize;                 // Size of this structure
            LPDDRAWI_DDRAWSURFACE_LCL  lpDDSurface; // Surface
            LPDDSURFACEDESC lpddsd;                 // Description of surface
            HRESULT         ddrval;
        } DDHAL_HALSETSURFACEDESCDATA;
    */

    // Implementation
    pd->ddrval = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

#ifndef NEWHAL

DWORD WINAPI HalFlip( LPDDHAL_FLIPDATA pd )
{
    DEBUGENTER( HalFlip );
    /*
    typedef struct _DDHAL_FLIPDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfCurr;     // current surface
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfTarg;     // target surface 
                                                    // (to flip to)
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_FLIP          Flip;           // PRIVATE: ptr to callback
    } DDHAL_FLIPDATA;
    */

    if( IS_BUSY )
    {
        if( pd->dwFlags & DDFLIP_WAIT )
        {
            WAIT_FOR_NOT_BUSY;
        }
        else
        {
            DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Graphics engine busy\r\n")));
            pd->ddRVal = DDERR_WASSTILLDRAWING;
            return DDHAL_DRIVER_HANDLED;
        }
    }

    //////
    //WAIT_FOR_VBLANK;
    //////

    DDGPEFlip(pd);


    //DEBUGMSG(1,(TEXT("Flip!\r\n")));
    DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Flip done\r\n")));

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

#else

DWORD WINAPI HalFlip( LPDDHAL_FLIPDATA pd )
{
    DEBUGENTER( HalFlip );

    return ((S3Virge *)g_pGPE)->Flip(pd);
}

#endif

DWORD WINAPI HalSetClipList( LPDDHAL_SETCLIPLISTDATA pd )
{
    DEBUGENTER( HalSetClipList );
    /*
    typedef struct _DDHAL_SETCLIPLISTDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_SETCLIPLIST   SetClipList;    // PRIVATE: ptr to callback
    } DDHAL_SETCLIPLISTDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}


#ifndef NEWHAL

DWORD WINAPI HalBlt( LPDDHAL_BLTDATA pd )
{
    DEBUGENTER( HalBlt );
    /*
    typedef struct _DDHAL_BLTDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDDestSurface;// dest surface
        RECTL                       rDest;          // dest rect
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSrcSurface; // src surface
        RECTL                       rSrc;           // src rect
        DWORD                       dwFlags;        // blt flags
        DWORD                       dwROPFlags;     // ROP flags 
                                                    // (valid for ROPS only)
        DDBLTFX                     bltFX;          // blt FX
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_BLT           Blt;            // PRIVATE: ptr to callback
    } DDHAL_BLTDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

#else

DWORD WINAPI HalBlt( LPDDHAL_BLTDATA pd )
{
    DEBUGENTER( HalBlt );

    return ((S3Virge *)g_pGPE)->Blt(pd);
}

#endif

// not to be confused with
// DWORD WINAPI HalSetColorKey( LPDDHAL_DRVSETCOLORKEYDATA pd )
DWORD WINAPI HalSetColorKey( LPDDHAL_SETCOLORKEYDATA pd )
{
    DEBUGENTER( HalSetColorKey );
    /*
    typedef struct _DDHAL_DRVSETCOLORKEYDATA
    {
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       dwFlags;        // flags
        DDCOLORKEY                  ckNew;          // new color key
        HRESULT                     ddRVal;         // return value
        LPDDHAL_SETCOLORKEY         SetColorKey;    // PRIVATE: ptr to callback
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
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpSurfAttached; // surface to attach
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_ADDATTACHEDSURFACE AddAttachedSurface;
                                                    // PRIVATE: ptr to callback
    } DDHAL_ADDATTACHEDSURFACEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

#ifndef NEWHAL

DWORD WINAPI HalGetBltStatus( LPDDHAL_GETBLTSTATUSDATA pd )
{
    DEBUGENTER( HalGetBltStatus );
    /*
    typedef struct _DDHAL_GETBLTSTATUSDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_GETBLTSTATUS  GetBltStatus;   // PRIVATE: ptr to callback
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
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        DWORD                       dwFlags;        // flags
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_GETFLIPSTATUS GetFlipStatus;  // PRIVATE: ptr to callback
    } DDHAL_GETFLIPSTATUSDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}

#else

DWORD WINAPI HalGetBltStatus( LPDDHAL_GETBLTSTATUSDATA pd )
{
    DEBUGENTER( HalGetBltStatus );

    return ((S3Virge *)g_pGPE)->GetBltStatus(pd);
}

DWORD WINAPI HalGetFlipStatus( LPDDHAL_GETFLIPSTATUSDATA pd )
{
    DEBUGENTER( HalGetFlipStatus );

    return ((S3Virge *)g_pGPE)->GetFlipStatus(pd);
}

#endif

DWORD WINAPI HalUpdateOverlay( LPDDHAL_UPDATEOVERLAYDATA pd )
{
    DEBUGENTER( HalUpdateOverlay );

#ifdef ENABLE_OVERLAY
    return ((S3Virge *)g_pGPE)->UpdateOverlay(pd);
#else
    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
#endif // ENABLE_OVERLAY
}

DWORD WINAPI HalSetOverlayPosition( LPDDHAL_SETOVERLAYPOSITIONDATA pd )
{
    DEBUGENTER( HalSetOverlayPosition );

#ifdef ENABLE_OVERLAY
    return ((S3Virge *)g_pGPE)->SetOverlayPosition(pd);
#else
    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
#endif // ENABLE_OVERLAY
}

DWORD WINAPI HalSetPalette( LPDDHAL_SETPALETTEDATA pd )
{
    DEBUGENTER( HalSetPalette );
    /*
    typedef struct _DDHAL_SETPALETTEDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSurface;    // surface struct
        LPDDRAWI_DDRAWPALETTE_GBL   lpDDPalette;    // palette to set to surface
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_SETPALETTE    SetPalette;     // PRIVATE: ptr to callback
        BOOL                        Attach;         // attach this palette?
    } DDHAL_SETPALETTEDATA;
    */

    // Implementation
    pd->ddRVal = DD_OK;

    return DDHAL_DRIVER_HANDLED;
}


#ifdef ENABLE_OVERLAY

DWORD WINAPI HalColorControl( LPDDHAL_COLORCONTROLDATA pd )
{
    DEBUGENTER( HalColorControl );

    return ((S3Virge *)g_pGPE)->ColorControl(pd);
}

#endif // ENABLE_OVERLAY
