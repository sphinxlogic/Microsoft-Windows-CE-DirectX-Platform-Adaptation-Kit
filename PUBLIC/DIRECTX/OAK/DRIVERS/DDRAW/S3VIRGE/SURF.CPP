/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

Module Name:    surface allocation/manipulation/free routines

Abstract:  
  
Functions:


Notes: 


--*/

#include "precomp.h"

SCODE 
S3Virge::AllocSurface(
    GPESurf **ppSurf, 
    int width, 
    int height, 
    EGPEFormat format, 
    int surfaceFlags
    )
{
    SCODE hr = S_OK;
    EDDGPEPixelFormat pixelFormat;
    GPEModeEx modeInfoEx;

    GetModeInfoEx(&modeInfoEx, GetModeId());

    // BUGBUG
    // There is an issue with GDI where if you have a printer device context
    // that has the same number of bits per pixel as the display, but
    // different bit masks, you will get the display's masks.

    // This is because in this function, when we are passed
    // the gpeDeviceCompatible constant, it refers to the current device,
    // which we have no knowledge of, not the display.

    if (format == modeInfoEx.modeInfo.format ||
        format == gpeDeviceCompatible)
    {
        pixelFormat = modeInfoEx.ePixelFormat;

        DEBUGMSG(1, 
            (TEXT("Display Compatible requested: format, ")
             TEXT("pixelFormat = 0x%x, 0x%x\r\n"), format, pixelFormat));
    }
    else
    {
        pixelFormat = EGPEFormatToEDDGPEPixelFormat[format];
        DEBUGMSG(1, 
            (TEXT("Explicitly requested: format, ")
             TEXT("pixelFormat = 0x%x, 0x%x\r\n"), format, pixelFormat));
    }

    hr = AllocSurface((DDGPESurf**)ppSurf, width, height, 
                      format, pixelFormat, surfaceFlags);

    return hr;
}


SCODE 
S3Virge::AllocSurface(
    DDGPESurf **ppSurf, 
    int width, 
    int height, 
    EGPEFormat format, 
    EDDGPEPixelFormat pixelFormat,
    int surfaceFlags
    )
{
    DWORD bpp  = EGPEFormatToBpp[format];
    LONG align = 8; // quadword aligned
    DWORD alignedWidth = ((width + align - 1) & (- align));
    DWORD nSurfaceBytes = (bpp * (alignedWidth * height)) / 8;
    DWORD stride = ((alignedWidth * bpp) / 8);

    if ((surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY) || 
        (surfaceFlags & GPE_PREFER_VIDEO_MEMORY))
    {
        // Attempt to allocate from video memory
        SurfaceHeap *pHeap = m_pVideoMemoryHeap->Alloc(nSurfaceBytes);

        if (pHeap)
        {
            ULONG offset = pHeap->Address();
            *ppSurf = new S3VirgeSurf(width, height, offset, 
                                      m_pLAW + offset, stride, 
                                      format, pixelFormat, pHeap);
            if (!(*ppSurf))
            {
                pHeap->Free();
                return E_OUTOFMEMORY;
            }
            return S_OK;
        }

        if (surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY)
        {
            *ppSurf = (DDGPESurf *)NULL;
            return E_OUTOFMEMORY;
        }
    }

    // Allocate from system memory
    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("Creating a DDGPESurf in system memory. ")
         TEXT("EGPEFormat = %d, DDGPEFormat = %d\r\n"), 
         (int) format, (int) pixelFormat));

    *ppSurf = new S3VirgeSurf(width, height, stride, format, pixelFormat);
    if (*ppSurf)
    {
        // check we allocated bits succesfully
        if (!((*ppSurf)->Buffer()))
        {
            delete *ppSurf;
            return E_OUTOFMEMORY;
        }
    }
    return S_OK;
}


   
S3VirgeSurf::S3VirgeSurf(
    int width, 
    int height, 
    ULONG offset, 
    PVOID pBits, 
    int stride, 
    EGPEFormat format, 
    SurfaceHeap *pHeap
    )
    : DDGPESurf(width, height, pBits, stride, format)
{
    m_pHeap = pHeap;
    m_fInVideoMemory = TRUE;
    m_nOffsetInVideoMemory = offset;
} 

S3VirgeSurf::S3VirgeSurf(
    int width,
    int height,
    ULONG offset,
    PVOID pBits,
    int stride,
    EGPEFormat format,
    EDDGPEPixelFormat pixelFormat,
    SurfaceHeap *pHeap
    )
    : DDGPESurf(width, height, pBits, stride, format, pixelFormat)
{
    m_pHeap = pHeap;
    m_fInVideoMemory = TRUE;
    m_nOffsetInVideoMemory = offset;
}

S3VirgeSurf::S3VirgeSurf(
    int width,
    int height,
    int stride,
    EGPEFormat format,
    EDDGPEPixelFormat pixelFormat
    )
    : DDGPESurf(width, height, stride, format, pixelFormat)
{
    m_pHeap = NULL;
}


S3VirgeSurf::~S3VirgeSurf(void)
{
    if (m_pHeap)
    {
        m_pHeap->Free();
    }
}


HRESULT 
S3Virge::UpdateFlipStatus(ULONG ulVidMem)
{
    FLIPRECORD *pRec = &m_sFlipRecord;

    if ((pRec->bFlipFlag) &&
        ((ulVidMem == 0xFFFFFFFF) ||
         (ulVidMem == pRec->ulFlipFrom)))
    {
        if (IN_VBLANK)
        {
            if (pRec->bWasEverInDisplay)
            {
                pRec->bHaveEverCrossedVBlank = TRUE;
            }
        }
        else if (IN_DISPLAY)
        {
            if (pRec->bHaveEverCrossedVBlank)
            {
                pRec->bFlipFlag = FALSE;
                return(DD_OK);
            }
            pRec->bWasEverInDisplay = TRUE;
        }

        ULONG ulNow = GetTickCount();
        if (ulNow - pRec->ulFlipTime <= m_nTicksPerFrame)
        {
            return(DDERR_WASSTILLDRAWING);
        }

        pRec->bFlipFlag = FALSE;
    }

    return(DD_OK);
}

int 
S3Virge::SurfaceBusyFlipping(GPESurf *pSurf)
{
    return (UpdateFlipStatus(pSurf->OffsetInVideoMemory()) != DD_OK);
}

int 
S3Virge::FlipInProgress(void)
{
    return (UpdateFlipStatus(0xFFFFFFFF) != DD_OK);
}

void 
S3Virge::SetVisibleSurface(GPESurf *pSurfTarg)
{
    S3VirgeSurf *pSurf = (S3VirgeSurf *) pSurfTarg;
    int displayStart = pSurf->OffsetInVideoMemory();

#ifdef ENABLE_OVERLAY
    reg_MM81C0 = displayStart;                      // primary stream address
#else
    displayStart >>= 2;
    reg_EXT_SCTL_3 = (displayStart >> 16) & 0x0F;   // CR69 bits 19-16 of 
                                                    // display start 
    reg_STA_H = (displayStart >> 8) & 0xFF;         // CRxC
    reg_STA_L = displayStart & 0xFF;                // CRxD
#endif // ENABLE_OVERLAY

    // remember where and when we were when flip was requested
    FLIPRECORD *pRec = &m_sFlipRecord;
    pRec->ulFlipTime = GetTickCount();
    pRec->bFlipFlag = TRUE;
    pRec->bHaveEverCrossedVBlank = FALSE;
    pRec->bWasEverInDisplay = FALSE;
}

void 
S3Virge::SetVisibleSurface(GPESurf *pSurf, BOOL bWaitForVBlank)
{
    if (bWaitForVBlank)
    {
        while (IN_VBLANK) {}
        while (!IN_VBLANK) {}
    }
    SetVisibleSurface(pSurf);
}

DWORD
S3Virge::Lock(LPDDHAL_LOCKDATA pd)
{
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

    DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

    if (SurfaceBusyFlipping(pSurf))
    {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

    if (pd->dwFlags & DDLOCK_WAIT)
    {
        WaitForNotBusy();
    }
    else if (S3D_IS_BUSY)
    {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

    ULONG ulAddress = pd->lpDDSurface->lpGbl->fpVidMem;
    if (pd->bHasRect)
    {
        DWORD bitcnt;
        LPDDRAWI_DDRAWSURFACE_GBL pGbl = pd->lpDDSurface->lpGbl;

        if (pd->lpDDSurface->dwFlags & DDRAWISURF_HASPIXELFORMAT)
            bitcnt = pGbl->ddpfSurface.dwRGBBitCount;
        else
            bitcnt = pGbl->lpDD->vmiData.ddpfDisplay.dwRGBBitCount;

        ulAddress += ((DWORD)pd->rArea.top * pSurf->Stride() +
                      ((DWORD)pd->rArea.left * (UINT)(bitcnt)) / 8u);
    }

    *(int *)&(pd->lpSurfData) = ulAddress;

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

DWORD
S3Virge::Flip(LPDDHAL_FLIPDATA pd)
{
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

    //
    // is the current flip still in progress?
    //
    // don't want a flip to work until after the last flip is done,
    // so we ask for the general flip status and ignore the vmem.
    //
    // Also wait for S3 engine to be idle, allows blts to complete before
    // another Virge command is attempted. Failure to wait can cause unwanted
    // data to appear on the display.
    //
    if (FlipInProgress() || S3D_IS_BUSY)
    {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

    // warning: get surfaces before swap!!!
    DDGPESurf* surfTarg = DDGPESurf::GetDDGPESurf(pd->lpSurfTarg);
    DDGPESurf* surfCurr = DDGPESurf::GetDDGPESurf(pd->lpSurfCurr);

    // swap the surfaces - DDraw swaps the fpVidmem values itself
    DDGPESurf::Swap(pd->lpSurfCurr, pd->lpSurfTarg);

    // set up the flip
    int displayStart = surfTarg->OffsetInVideoMemory();

#ifdef ENABLE_OVERLAY
    if (pd->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE)
    {
        reg_MM81C0 = displayStart;                  // primary stream fb add
    }
    else if (pd->lpSurfCurr->ddsCaps.dwCaps & DDSCAPS_OVERLAY)
    {
        if (surfCurr == m_pVisibleOverlay)
        {
            reg_MM81D0 = displayStart + m_dwOverlayFlipOffset;
                                                    // secondary stream fb add
            m_pVisibleOverlay = surfTarg;
        }
        else
        {
            pd->ddRVal = DDERR_OUTOFCAPS;
            return DDHAL_DRIVER_HANDLED;
        }
    }
#else
    displayStart >>= 2;
    reg_EXT_SCTL_3 = (displayStart >> 16) & 0x0F;   // CR69 bits 19-16 of
                                                    // display start
    reg_STA_H = (displayStart >> 8) & 0xFF;         // CRxC
    reg_STA_L = displayStart & 0xFF;                // CRxD
#endif // ENABLE_OVERLAY

    // remember where and when we were when flip was requested
    FLIPRECORD *pRec = &m_sFlipRecord;
    pRec->ulFlipTime = GetTickCount();
    pRec->bFlipFlag = TRUE;
    pRec->bHaveEverCrossedVBlank = FALSE;
    pRec->bWasEverInDisplay = FALSE;

    // set the vid mem offset of the surface we flip from
    pRec->ulFlipFrom = surfCurr->OffsetInVideoMemory();

    DEBUGMSG(GPE_ZONE_FLIP,(TEXT("Flip done\r\n")));

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

DWORD
S3Virge::Blt(LPDDHAL_BLTDATA pd)
{
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

    DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);

    //
    // Wait for S3 engine to be idle, allows blts to complete before
    // another Virge command is attempted. Failure to wait can cause unwanted
    // data to appear on the display.
    //
    if (S3D_IS_BUSY || SurfaceBusyFlipping(pSurf))
    {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
        return DDHAL_DRIVER_HANDLED;
    }

    if (pd->dwFlags & DDBLT_ASYNC)
    {
        // If async, then only work if we won't have to wait on the
        // accelerator to start the command.
        //
        // The FIFO wait should account for the worst-case possible
        // blt that we would do:

        if (S3D_FIFO_HAS_ROOM(DDBLT_FIFO_COUNT))
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
            return DDHAL_DRIVER_HANDLED;
        }
    }

    DDGPESurf* pDstSurf = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);
    DDGPESurf* pSrcSurf = (DDGPESurf* )NULL;
    DDGPESurf* pPattern = (DDGPESurf* )NULL;
    RECT *prclDst = (RECT *)&(pd->rDest);
    RECT *prclSrc = (RECT *)&(pd->rSrc);
    ULONG solidColor;
    ULONG bltFlags = (pd->dwFlags & DDBLT_WAIT)?0x100:0;
    ULONG rop4;

    if(pd->lpDDSrcSurface)
    {
        pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);
    }
    else
    {
        pSrcSurf = (DDGPESurf*)NULL;
    }

    if ((pd->dwFlags & DDBLT_COLORFILL) ||
        ((pd->dwFlags & DDBLT_ROP) &&
         (pd->bltFX.dwROP == BLACKNESS || pd->bltFX.dwROP == WHITENESS)))
    {
        // FILL BLT
        DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("FILL BLT\n")));

        if ((pd->dwFlags & DDBLT_ROP) && (pd->bltFX.dwROP == BLACKNESS))
        {
            rop4 = 0x0000;
        }
        else if ((pd->dwFlags & DDBLT_ROP) && (pd->bltFX.dwROP == WHITENESS))
        {
            rop4 = 0xFFFF;
        }
        else
        {
            rop4 = 0xF0F0;  // PATCOPY
            solidColor = pd->bltFX.dwFillColor;
        }
    }
    else if (( pd->dwFlags & DDBLT_ROP) && (pd->bltFX.dwROP == PATCOPY))
    {
        // PAT BLT
        DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("PAT BLT\n")));
        rop4 = 0xF0F0;  // PATCOPY;
        solidColor = 0xFFFFFFFF;    // i.e. not a solid color blt
        pPattern = DDGPESurf::GetDDGPESurf(
                       (LPDDRAWI_DDRAWSURFACE_LCL)(pd->bltFX.lpDDSPattern));
    }
    else if (pd->dwFlags & (DDBLT_KEYSRCOVERRIDE | DDBLT_KEYSRC))
    {
        // TRANSPARENT BLT
        DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("TRANSPARENT BLT\n")));

        if (pd->dwFlags & DDBLT_KEYSRCOVERRIDE)
        {
            DEBUGMSG(GPE_ZONE_BLT_LO,
                     (TEXT("DDBLT_KEYSRCOVERRIDE, xparent = 0x%x\n"),
                      pd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue));
            solidColor = pd->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
        }
        else
        {
            DEBUGMSG(GPE_ZONE_BLT_LO,
                     (TEXT("DDBLT_KEYSRC, xparent = 0x%x\n"),
                      pd->lpDDSrcSurface->ddckCKSrcBlt.dwColorSpaceLowValue));
            solidColor = pd->lpDDSrcSurface->ddckCKSrcBlt.dwColorSpaceLowValue;
        }

        rop4 = 0xCCCC;  // SRCCOPY
        bltFlags |= BLT_TRANSPARENT;

        // BLT_STRETCH?
        if ((prclDst != NULL) && (prclSrc != NULL))
        {
            if (((prclDst->bottom - prclDst->top) !=
                 (prclSrc->bottom - prclSrc->top)) ||
                ((prclDst->right - prclDst->left) !=
                 (prclSrc->right - prclSrc->left) ))
            {
                bltFlags |= BLT_STRETCH;
            }
        }
    }
    else
    {
        // SIMPLE BLT
        DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("SIMPLE BLT\n")));

        // BLT_STRETCH?
        if ((prclDst != NULL) && (prclSrc != NULL))
        {
            if (((prclDst->bottom - prclDst->top) !=
                 (prclSrc->bottom - prclSrc->top)) ||
                ((prclDst->right - prclDst->left) !=
                 (prclSrc->right - prclSrc->left) ))
            {
                bltFlags |= BLT_STRETCH;
            }
        }
        rop4 = 0xCCCC;  // SRCCOPY
    }

    pd->ddRVal = BltExpanded(
                    pDstSurf,
                    pSrcSurf,
                    pPattern,
                    prclDst,
                    prclSrc,
                    solidColor,
                    bltFlags,
                    rop4 );

    return DDHAL_DRIVER_HANDLED;
}


DWORD
S3Virge::GetBltStatus(LPDDHAL_GETBLTSTATUSDATA pd)
{
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

    HRESULT ddRVal = DD_OK;

    if (pd->dwFlags == DDGBS_CANBLT)
    {
        // DDGBS_CANBLT case: can we add a blt?
        DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);

        if (SurfaceBusyFlipping(pSurf))
        {
            ddRVal = DDERR_WASSTILLDRAWING;
        }
        else
        {
            if (S3D_IS_BUSY)
            {
                ddRVal = DDERR_WASSTILLDRAWING;
            }
        }
    }
    else
    {
        // DDGBS_ISBLTDONE case: is a blt in progress?
        if (S3D_IS_BUSY)
        {
            ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    pd->ddRVal = ddRVal;

    return DDHAL_DRIVER_HANDLED;
}

DWORD
S3Virge::GetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA pd)
{
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

    pd->ddRVal = DD_OK;

    if (FlipInProgress())
    {
        pd->ddRVal = DDERR_WASSTILLDRAWING;
    }
    else if (pd->dwFlags & DDGFS_CANFLIP)
    {
        if (S3D_IS_BUSY)
        {
            pd->ddRVal = DDERR_WASSTILLDRAWING;
        }
    }

    return DDHAL_DRIVER_HANDLED;
}

#ifdef ENABLE_OVERLAY

#define DDOVER_ALPHA \
    (DDOVER_ALPHASRC | \
     DDOVER_ALPHASRCCONSTOVERRIDE | \
     DDOVER_ALPHADEST | \
     DDOVER_ALPHADESTCONSTOVERRIDE)

#define DDOVER_KEY \
    (DDOVER_KEYSRC | \
     DDOVER_KEYSRCOVERRIDE |\
     DDOVER_KEYDEST | \
     DDOVER_KEYDESTOVERRIDE)

#define KEYING          0x1
#define BLENDING        0x2
#define OPAQUEING       0x4

// #define OPQ_CTRL

DWORD
S3Virge::UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd)
{
    /*
    typedef struct _DDHAL_UPDATEOVERLAYDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDDestSurface;// dest surface
        RECTL                       rDest;          // dest rect
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSrcSurface; // src surface
        RECTL                       rSrc;           // src rect
        DWORD                       dwFlags;        // flags
        DDOVERLAYFX                 overlayFX;      // overlay FX
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_UPDATEOVERLAY UpdateOverlay;  // PRIVATE: ptr to callback
    } DDHAL_UPDATEOVERLAYDATA;
    */

    DDGPESurf* pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);

    // error if requested both keying and blending
    if ((pd->dwFlags & DDOVER_KEY) && (pd->dwFlags & DDOVER_ALPHA))
    {
        DEBUGMSG(1, 
                  (TEXT("Error: can't handle both keying and blending!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
    }

    // error if requested both source and dest color keying
    if ((pd->dwFlags & (DDOVER_KEYDEST | DDOVER_KEYDESTOVERRIDE)) &&
        (pd->dwFlags & (DDOVER_KEYSRC | DDOVER_KEYSRCOVERRIDE)))
    {
        DEBUGMSG(1, 
          (TEXT("Error: can't handle both source and dest color keying!\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return (DDHAL_DRIVER_HANDLED);
    }

    if (pd->dwFlags & DDOVER_HIDE)
    {
        if (pSrcSurf == m_pVisibleOverlay)
        {
            // hide the overlay
            reg_MM81A0 = POnS;          // blend control
#ifdef OPQ_CTRL
            reg_MM81DC = 0;             // opaque control
#endif // OPQ_CTRL

            // reset visible overlay
            m_pVisibleOverlay = NULL;
        }
        else
        {
            // the overlay is not currently visible
            // nothing we need to do here
        }

        pd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    DDGPESurf* pDstSurf = DDGPESurf::GetDDGPESurf(pd->lpDDDestSurface);

    if (pSrcSurf != m_pVisibleOverlay)
    {
        if (pd->dwFlags & DDOVER_SHOW)
        {
            if (m_pVisibleOverlay != NULL)
            {
                // some other overlay is already visible
                DEBUGMSG(1, 
                          (TEXT("Error: Other overaly already visible!\r\n")));
                pd->ddRVal = DDERR_OUTOFCAPS;
                return DDHAL_DRIVER_HANDLED;
            }
            else
            {
                // we are going to make the overlay visible
                // so mark it as such:
                m_pVisibleOverlay = pSrcSurf;
            }
        }
        else // DDOVER_SHOW not requested
        {
            // the overlay isn't visible, and we haven't been
            // asked to make it visible, so nothing we need to do
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }
    }
    
    DWORD dwSrcWidth = pd->rSrc.right - pd->rSrc.left;
    DWORD dwSrcHeight = pd->rSrc.bottom - pd->rSrc.top;
    DWORD dwDstWidth = pd->rDest.right - pd->rDest.left;
    DWORD dwDstHeight = pd->rDest.bottom - pd->rDest.top;
    DWORD dwSrcStride = pSrcSurf->Stride();

    DWORD dwSrcBytesPerPixel = EGPEFormatToBpp[pSrcSurf->Format()] / 8;
    m_dwOverlayFlipOffset = pd->rSrc.top * dwSrcStride + 
                            pd->rSrc.left * dwSrcBytesPerPixel;
    DWORD dwStartAdd = pSrcSurf->OffsetInVideoMemory() + m_dwOverlayFlipOffset;

    // set secondary stream control
    // 1. based on format
    DWORD dwSecCtrl = HDDA(dwSrcWidth, dwDstWidth);
    switch (pSrcSurf->PixelFormat())
    {
        case ddgpePixelFormat_YUYV422:
            dwSecCtrl |= S_YCrCb422;        // Not S_YUV422!  Dunno why...
            break;

#ifdef SUPPORT_UYVY422
        case ddgpePixelFormat_UYVY422:
            dwSecCtrl |= S_UYVY;
#endif
            break;

        case ddgpePixelFormat_5551:
        case ddgpePixelFormat_5550:
            dwSecCtrl |= S_RGB5551;
            break;

        case ddgpePixelFormat_565:
            dwSecCtrl |= S_RGB565;
            break;

        case ddgpePixelFormat_8880:
            dwSecCtrl |= S_RGB8880;
            break;

        case ddgpePixelFormat_8888:
            dwSecCtrl |= S_RGB8888;
            break;

        default:
            DEBUGMSG(1, (TEXT("Error: Unexpected overlay format!\r\n")));
            pd->ddRVal = DDERR_UNSUPPORTED;
            return DDHAL_DRIVER_HANDLED;
    }

    // 2. based on scaling factor
    if ((dwDstWidth != dwSrcWidth) || (dwDstHeight != dwSrcHeight))
    {
        if (dwDstWidth >= (dwSrcWidth << 2))
        {
            dwSecCtrl |= S_Beyond4x;
        }
        else if (dwDstWidth >= (dwSrcWidth << 1))
        {
            dwSecCtrl |= S_2xTo4x;
        }
        else
        {
            dwSecCtrl |= S_Upto2x;
        }
    }

    BYTE bSPOP = 0; // can be either KEYING, BLENDING or OPAQUEING
    DDGPESurf *pColorKeySurf = NULL;
    DWORD dwBlendCtrl = 0;
    DWORD dwKeyLow = 0;
    DWORD dwKeyHigh = 0;

    if (pd->dwFlags & DDOVER_KEYDEST)
    {
        bSPOP = KEYING;
        pColorKeySurf = pDstSurf;
        dwKeyLow = pd->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceLowValue;
        dwKeyHigh = 
                   pd->lpDDDestSurface->ddckCKDestOverlay.dwColorSpaceHighValue;
        dwBlendCtrl |= KeyOnP;
    }
    else if (pd->dwFlags & DDOVER_KEYDESTOVERRIDE)
    {
        bSPOP = KEYING;
        pColorKeySurf = pDstSurf;
        dwKeyLow  = pd->overlayFX.dckDestColorkey.dwColorSpaceLowValue;
        dwKeyHigh = pd->overlayFX.dckDestColorkey.dwColorSpaceHighValue;
        dwBlendCtrl |= KeyOnP;
    }
    else if (pd->dwFlags & DDOVER_KEYSRC)
    {
        bSPOP = KEYING;
        pColorKeySurf = pSrcSurf;
        dwKeyLow = pd->lpDDDestSurface->ddckCKSrcOverlay.dwColorSpaceLowValue;
        dwKeyHigh = pd->lpDDDestSurface->ddckCKSrcOverlay.dwColorSpaceHighValue;
        dwBlendCtrl |= KeyOnS;
    }
    else if (pd->dwFlags & DDOVER_KEYSRCOVERRIDE)
    {
        bSPOP = KEYING;
        pColorKeySurf = pSrcSurf;
        dwKeyLow = pd->overlayFX.dckSrcColorkey.dwColorSpaceLowValue;
        dwKeyHigh = pd->overlayFX.dckSrcColorkey.dwColorSpaceHighValue;
        dwBlendCtrl |= KeyOnS;
    }

    if (bSPOP == KEYING)
    {
        switch (pColorKeySurf->PixelFormat())
        {
            case ddgpePixelFormat_YUYV422:
#ifdef SUPPORT_UYVY422
            case ddgpePixelFormat_UYVY422:
#endif
                dwKeyLow = (dwKeyLow & 0x00ff7f7f) | (~(dwKeyLow | 0x00ff7f7f));
                dwKeyHigh = (dwKeyHigh & 0x00ff7f7f) | 
                            (~(dwKeyHigh | 0x00ff7f7f));
                break;

            case ddgpePixelFormat_5551:
            case ddgpePixelFormat_5550:
                dwKeyLow = RGB15to32(dwKeyLow);
                dwKeyHigh = RGB15to32(dwKeyHigh);
                dwKeyLow |= CompareBits3t7;
                break;

            case ddgpePixelFormat_565:
                dwKeyLow = RGB16to32(dwKeyLow);
                dwKeyHigh = RGB16to32(dwKeyHigh);
                dwKeyLow |= CompareBits3t7;
                break;

            case ddgpePixelFormat_8880:
            case ddgpePixelFormat_8888:
                dwKeyLow |= CompareBits0t7;
                break;

            default:
                DEBUGMSG(1, (TEXT("Error: Unexpected overlay format!\r\n")));
                pd->ddRVal = DDERR_UNSUPPORTED;
                return DDHAL_DRIVER_HANDLED;
        }
        dwKeyLow |= KeyFromCompare;
    }

    // take care of blending
    LPDDOVERLAYFX lpFX = (LPDDOVERLAYFX)&(pd->overlayFX);
    DWORD dwTmp;

    if (pd->dwFlags & DDOVER_ALPHASRC)
    {
        bSPOP = KEYING;
        dwBlendCtrl |= KeyOnS;
        dwKeyLow &= 0x00FFFFFF;
    }
    else if (pd->dwFlags & DDOVER_ALPHADEST)
    {
        bSPOP = KEYING;
        dwBlendCtrl |= KeyOnP;
        dwKeyLow &= 0x00FFFFFF;
    }
    else if (pd->dwFlags & DDOVER_ALPHASRCCONSTOVERRIDE)
    {
        bSPOP = BLENDING;
        dwTmp = lpFX->dwAlphaSrcConst;
        dwTmp = dwTmp & 0x07;   //Ks: only take 3 bits alpha value.
        if (!dwTmp)
        {
            // we want to see the primary only, hide the overlay
            reg_MM81A0 = POnS;          // blend control
#ifdef OPQ_CTRL
            reg_MM81DC = 0;             // opaque control
#endif // OPQ_CTRL

            // reset visible overlay
            m_pVisibleOverlay = NULL;
            pd->ddRVal = DD_OK;
            return DDHAL_DRIVER_HANDLED;
        }

        dwTmp = 8 - dwTmp;      //Kp
        dwBlendCtrl |= Dissolve | ((dwTmp << Kp_Shift) & Kp_Mask);
    }
    else if (pd->dwFlags & DDOVER_ALPHADESTCONSTOVERRIDE)
    {
        bSPOP = BLENDING;
        dwTmp = lpFX->dwAlphaDestConst;
        dwTmp = dwTmp & 0x07;   //Kp: only take 3 bits alpha value.
        if (!dwTmp)
            //we want to see secondary only, let it go thru opaque.
            pd->dwFlags &= ~DDOVER_ALPHADESTCONSTOVERRIDE;
        else
            dwBlendCtrl |= Dissolve | ((dwTmp << Kp_Shift) & Kp_Mask);
    }

#ifdef OPQ_CTRL
    DWORD dwOpaqueCtrl = 0;
    // if neither keying or blending, 
    // set enable opaque control to reduce bandwidth
    if (!(bSPOP & (BLENDING | KEYING)))
    {
        DWORD dwDstBytesPerPixel = EGPEFormatToBpp[pDstSurf->Format()] / 8;
        bSPOP = OPAQUEING;

        // pixel stop fetch
        dwTmp = pd->rDest.left * dwDstBytesPerPixel;
        if (dwTmp & 0x7)
            dwTmp = (dwTmp >> 3) + 2; // round up
        else
            dwTmp = (dwTmp >> 3) + 1;
        dwOpaqueCtrl = (1L << 31) | (dwTmp << 3);
        // pixel resume fetch
        dwTmp = dwTmp + ((dwDstWidth * dwDstBytesPerPixel) >> 3);
        dwOpaqueCtrl |= (dwTmp << 19);
    }
#endif // OPQ_CTRL

    // set the registers to show overlay
    reg_MM81D0 = dwStartAdd;                // Secondary FB Address 0
    reg_MM81D8 = dwSrcStride;               // Secondary Stream Stride
    reg_MM81F8 = XY(pd->rDest.left, pd->rDest.top);
                                            // Sec. Window Start Coordinates
    reg_MM81FC = WH(dwDstWidth, dwDstHeight);  
                                            // Secondary Window Size
    reg_MM8198 = HK1K2(dwSrcWidth, dwDstWidth);
                                            // Secondary Stream Stretch/
                                            // Filter Constants
    reg_MM81E0 = VK1(dwSrcHeight);          // K1 Vertical Scale Factor
    reg_MM81E4 = VK2(dwSrcHeight, dwDstHeight);
                                            // K2 Vertical Scale Factor
    reg_MM81E8 = VDDA(dwDstHeight);         // DDA Vert. Acc. Initial Value
    reg_MM8190 = dwSecCtrl;                 // Secondary Stream Control

    if (bSPOP == KEYING)
    {
        reg_MM8184 = dwKeyLow;              // Color Key Lower Bound
        reg_MM8194 = dwKeyHigh;             // Color Key Upper Bound
    }
    reg_MM81A0 = dwBlendCtrl;               // Blend Control
#ifdef OPQ_CTRL
    reg_MM81DC = dwOpaqueCtrl;              // Opaque Control
#endif // OPQ_CTRL

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

DWORD
S3Virge::SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd)
{
    /*
    typedef struct _DDHAL_SETOVERLAYPOSITIONDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL     lpDD;           // driver struct
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDSrcSurface; // src surface
        LPDDRAWI_DDRAWSURFACE_LCL   lpDDDestSurface;// dest surface
        LONG                        lXPos;          // x position
        LONG                        lYPos;          // y position
        HRESULT                     ddRVal;         // return value
        LPDDHALSURFCB_SETOVERLAYPOSITION SetOverlayPosition;
                                                    // PRIVATE: ptr to callback
    } DDHAL_SETOVERLAYPOSITIONDATA;
    */
    DDGPESurf* pSrcSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSrcSurface);

    if (pSrcSurf != m_pVisibleOverlay)
    {
        pd->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }

    reg_MM81F8 = XY(pd->lXPos, pd->lYPos);  // Sec. Window Start Coordinates

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}


//
// color conversion routines
//
LONG FitRangeAtoB(LONG Avalue, LONG Amin, LONG Amax, LONG Bmin, LONG Bmax)
{  
    return ((Avalue-Amin)*(Bmax-Bmin)/(Amax-Amin)) + Bmin;
}


LONG HWtoDD_Bright(DWORD dwReg)
{  
    LONG Avalue = dwReg & 0x000000FF;
    if (Avalue >= 128)
        Avalue -= 256;

    if (Avalue <= HW_DEF_BRIGHT)
    {  
        return FitRangeAtoB(Avalue, HW_MIN_BRIGHT, HW_DEF_BRIGHT, 
                            DD_MIN_BRIGHT, DD_DEF_BRIGHT);
    }
    else
    {  
        return FitRangeAtoB(Avalue, HW_DEF_BRIGHT, HW_MAX_BRIGHT, 
                            DD_DEF_BRIGHT, DD_MAX_BRIGHT);
    }
}


LONG HWtoDD_Cont(DWORD dwReg)
{  
    LONG Avalue = (dwReg & 0x00001F00) >> 8;

    if (Avalue <= HW_DEF_CONT)
    {  
        return FitRangeAtoB(Avalue, HW_MIN_CONT, HW_DEF_CONT, 
                            DD_MIN_CONT, DD_DEF_CONT);
    }
    else
    {  
        return FitRangeAtoB(Avalue, HW_DEF_CONT, HW_MAX_CONT, 
                            DD_DEF_CONT, DD_MAX_CONT);
    }
}


LONG HWtoDD_Hue(DWORD dwReg)
{  
    LONG lHue;
    LONG lFactor1 = (dwReg & 0x001F0000) >> 16;
    LONG lFactor2 = (dwReg & 0x1F000000) >> 24;
    float fFactor1, fFactor2, temp;

    // convert factors to signed
    if (lFactor1 >= 16)
        lFactor1 -= 32;
    if (lFactor2 >= 16)
        lFactor2 -= 32;

    fFactor1 = (FLOAT)lFactor1;   fFactor2 = (FLOAT)lFactor2;

    // calculate Sat
    temp = (float)sqrt(fFactor1*fFactor1 + fFactor2*fFactor2);
    temp = (float)((float)acos(fFactor1 / temp) * 180.0 / (float)PI);

    // round it
    if (temp < 0)
        lHue = (LONG)(temp-0.5);
    else
        lHue = (LONG)(temp+0.5);

    // set sign
    if (fFactor2 < 0)
        lHue = -lHue;

    if (lHue <= HW_DEF_HUE)
    {  
        return FitRangeAtoB(lHue, HW_MIN_HUE, HW_DEF_HUE, 
                            DD_MIN_HUE, DD_DEF_HUE);
    }
    else
    {  
        return FitRangeAtoB(lHue, HW_DEF_HUE, HW_MAX_HUE, 
                            DD_DEF_HUE, DD_MAX_HUE);
    }
}


LONG HWtoDD_Sat(DWORD dwReg)
{  
    LONG lSat;
    float temp;
    LONG factor1 = (dwReg & 0x001F0000) >> 16;
    LONG factor2 = (dwReg & 0x1F000000) >> 24;

    // convert factors to signed
    if (factor1 >= 16)
        factor1 -= 32;
    if (factor2 >= 16)
        factor2 -= 32;

    temp = (float)sqrt( factor1*factor1 + factor2*factor2 );
    // round it
    if (temp<0)
        lSat = (LONG)(temp-0.5);
    else
        lSat = (LONG)(temp+0.5);
 
    if (lSat <= HW_DEF_SAT)
    {  
        return FitRangeAtoB(lSat, HW_MIN_SAT, HW_DEF_SAT, 
                            DD_MIN_SAT, DD_DEF_SAT);
    }
    else
    {  
        return FitRangeAtoB(lSat, HW_DEF_SAT, HW_MAX_SAT, 
                            DD_DEF_SAT, DD_MAX_SAT);
    }
}


DWORD  DDtoHW_BrightCont(LONG lDDBright, LONG lDDCont)
{  
    DWORD dwHWBright, dwHWCont;

    if (lDDBright <= HW_DEF_BRIGHT)
    {  
        dwHWBright = FitRangeAtoB(lDDBright, DD_MIN_BRIGHT, DD_DEF_BRIGHT, 
                                  HW_MIN_BRIGHT, HW_DEF_BRIGHT);
    }
    else
    {  
        dwHWBright = FitRangeAtoB(lDDBright, DD_DEF_BRIGHT, DD_MAX_BRIGHT, 
                                  HW_DEF_BRIGHT, HW_MAX_BRIGHT);
    }
    if (lDDCont <= HW_DEF_CONT)
    {  
        dwHWCont = FitRangeAtoB(lDDCont, DD_MIN_CONT, DD_DEF_CONT, 
                                HW_MIN_CONT, HW_DEF_CONT);
    }
    else
    {  
        dwHWCont = FitRangeAtoB(lDDCont, DD_DEF_CONT, DD_MAX_CONT, 
                                HW_DEF_CONT, HW_MAX_CONT);
    }

    return ((dwHWCont & 0x0000001F) << 8) | (dwHWBright & 0x000000FF);
}


DWORD  DDtoHW_HueSat(LONG lDDHue, LONG lDDSat)
{  
    DWORD dwHWHue, dwHWSat;
    DWORD factor1, factor2;
    float temp;

    if (lDDHue <= HW_DEF_HUE)
    {  
        dwHWHue = FitRangeAtoB(lDDHue, DD_MIN_HUE, DD_DEF_HUE, 
                               HW_MIN_HUE, HW_DEF_HUE);
    }
    else
    {  
        dwHWHue = FitRangeAtoB(lDDHue, DD_DEF_HUE, DD_MAX_HUE, 
                               HW_DEF_HUE, HW_MAX_HUE);
    }
    if (lDDSat <= HW_DEF_SAT)
    {  
        dwHWSat = FitRangeAtoB(lDDSat, DD_MIN_SAT, DD_DEF_SAT, 
                               HW_MIN_SAT, HW_DEF_SAT);
    }
    else
    {  
        dwHWSat = FitRangeAtoB(lDDSat, DD_DEF_SAT, DD_MAX_SAT, 
                               HW_DEF_SAT, HW_MAX_SAT);
    }

    temp = dwHWSat * (float)cos((LONG)dwHWHue * PI / 180.0);
    // round it
    if (temp<0)
        factor1 = (LONG)(temp-0.5);
    else
        factor1 = (LONG)(temp+0.5);

    temp = dwHWSat * (float)sin((LONG)dwHWHue * PI / 180.0);
    // round it
    if (temp<0)
        factor2 = (LONG)(temp-0.5);
    else
        factor2 = (LONG)(temp+0.5);

    return ((factor1 & 0x0000001F) << 16) | ((factor2 & 0x0000001F) << 24);
}


DWORD
S3Virge::ColorControl(LPDDHAL_COLORCONTROLDATA pd)
{
    /*typedef struct _DDHAL_COLORCONTROLDATA
    {
        LPDDRAWI_DIRECTDRAW_GBL        lpDD;        // ptr to the ddraw GBL
        LPDDRAWI_DDRAWSURFACE_LCL      lpDDSurface; // ptr to the surface LCL
        LPDDCOLORCONTROL               lpColorData; // ptr to a DDCOLORCONTROL
        DWORD                          dwFlags;     // flags
        HRESULT                        ddRVal;      // return values
        LPDDHALCOLORCB_COLORCONTROL    ColorControl;// PRIVATE: ptr to callback
    } DDHAL_COLORCONTROLDATA;

    typedef struct _DDCOLORCONTROL
    {
        DWORD     dwSize;           // size of the structure
        DWORD     dwFlags;          // decides which members contain valid data
        LONG      lBrightness;      // Luminance intensity (Black Level) in 
                                    // IRE units*100. Range is 0 to 10,000. 
                                    // The default is 750 (7.5 IRE).
        LONG      lContrast;        // Relative difference between higher and 
                                    // lower intensity luminance values in 
                                    // IRE units*100. The valid range is 0 to 
                                    // 20,000. The default is 10,000 (100 IRE).
                                    // Higher values of contrast cause darker 
                                    // luminance values to tend toward black, 
                                    // and cause lighter luminance values to 
                                    // tend toward white. Lower values of 
                                    // contrast cause all luminance values to 
                                    // move toward the middle luminance values. 
        LONG      lHue;             // Controls the phase relationship of the 
                                    // chrominance components. Hue is specified 
                                    // in degrees and the valid range is -180 
                                    // to 180. The default is 0. 
        LONG      lSaturation;      // Specifies the color intensity in IRE 
                                    // units*100. The valid range is 0 to 
                                    // 20,000. The default value is 
                                    // 10,000 (100 IRE). 
        LONG      lSharpness;       // Specifies the sharpness in arbitrary 
                                    // units. The valid range is 0 to 10. 
                                    // The default value is 5.
        LONG      lGamma;           // Controls the amount of gamma correction 
                                    // applied to the luminance values. The 
                                    // valid range is 1 to 500 gamma units, 
                                    // with a default of 1. 
        LONG      lColorEnable;     // Enables or disables color data. If 
                                    // lColorEnable is 0, ignore the 
                                    // chrominance (color) data and only use 
                                    // the luminance data (that is, show 
                                    // everything in black and white). If set 
                                    // to any non-zero value, both chrominance 
                                    // and luminance data are used. 
        DWORD     dwReserved1;
    } DDCOLORCONTROL;*/

    DDGPESurf* pSurf = DDGPESurf::GetDDGPESurf(pd->lpDDSurface);
    EDDGPEPixelFormat pixelFormat = pSurf->PixelFormat();
    DWORD dwReg;

    if ((pixelFormat != ddgpePixelFormat_YUYV422)
#ifdef SUPPORT_UYVY422
		&& (pixelFormat != ddgpePixelFormat_UYVY422)
#endif
		)
    {
        DEBUGMSG(1, (TEXT("Error: color control ONLY for YUV surfaces\r\n")));
        pd->ddRVal = DDERR_INVALIDOBJECT;
        return DDHAL_DRIVER_HANDLED;
    }

    if (pd->dwFlags == DDRAWI_GETCOLOR)
    {
        dwReg = reg_MM819C & 0xFF1F9FFF;

        //if brightness and contrast enabled, use real values
        if (dwReg & 0x00008000)
        {  
            if (pd->lpColorData->dwFlags & DDCOLOR_BRIGHTNESS)
                pd->lpColorData->lBrightness = 
                    HWtoDD_Bright(dwReg & 0x000000FF);
            if (pd->lpColorData->dwFlags & DDCOLOR_CONTRAST)
                pd->lpColorData->lContrast = HWtoDD_Cont(dwReg & 0x00001F00);
        }
        // if not enabled, return 'no effect' values
        else                 
        {  
            pd->lpColorData->lBrightness = DD_DEF_BRIGHT;
            pd->lpColorData->lContrast  = DD_DEF_CONT;
        }

        // if saturation and hue enabled, use real values
        if (dwReg & 0x80000000)  
        {  
            if (pd->lpColorData->dwFlags & DDCOLOR_SATURATION)
                pd->lpColorData->lSaturation = HWtoDD_Sat(dwReg & 0x1F1F0000);
            if (pd->lpColorData->dwFlags & DDCOLOR_HUE)
                pd->lpColorData->lHue = HWtoDD_Hue(dwReg & 0x1F1F0000);
        }
        // if not enabled, return 'no effect' values
        else                 
        {  
            pd->lpColorData->lSaturation = DD_DEF_SAT;
            pd->lpColorData->lHue = DD_DEF_HUE;
        }

        // Turn off flags we do not support
        pd->lpColorData->dwFlags &=
            ~(DDCOLOR_GAMMA | DDCOLOR_COLORENABLE | DDCOLOR_SHARPNESS);
    }
    else if (pd->dwFlags == DDRAWI_SETCOLOR)
    {
        dwReg = reg_MM819C;

        // if Brightness or Contrast need set, do both together
        if (pd->lpColorData->dwFlags & (DDCOLOR_BRIGHTNESS | DDCOLOR_CONTRAST))
        {  
            // in case no value is given for one of them, use defaults
            LONG lDDBright = DD_DEF_BRIGHT;
            LONG lDDCont   = DD_DEF_CONT;

            if (pd->lpColorData->dwFlags & DDCOLOR_BRIGHTNESS)
                lDDBright = pd->lpColorData->lBrightness;
            if (pd->lpColorData->dwFlags & DDCOLOR_CONTRAST)
                lDDCont = pd->lpColorData->lContrast;

            reg_MM819C = (dwReg & 0xFFFF0000) |     // clear B and C
                         DDtoHW_BrightCont(lDDBright, lDDCont) |    // values
                         0x00008000 ;               // set enable bit
        }

        // if Hue or Saturation need set, do both together
        if (pd->lpColorData->dwFlags & (DDCOLOR_HUE | DDCOLOR_SATURATION))
        {  
            // in case no value is given for one of them, use defaults
            LONG lDDHue = DD_DEF_HUE;
            LONG lDDSat = DD_DEF_SAT;

            if (pd->lpColorData->dwFlags & DDCOLOR_HUE)
                lDDHue = pd->lpColorData->lHue;
            if (pd->lpColorData->dwFlags & DDCOLOR_SATURATION)
                lDDSat = pd->lpColorData->lSaturation;

            reg_MM819C = (dwReg & 0x0000FFFF) |     // clear H and S
                         DDtoHW_HueSat(lDDHue, lDDSat) |            // values
                         0x80000000 ;               // set enable bit
        }
    }
    else
    {
        DEBUGMSG(1, (TEXT("Error: bad control flags\r\n")));
        pd->ddRVal = DDERR_INVALIDPARAMS;
        return DDHAL_DRIVER_HANDLED;
    }

    pd->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
}

#endif // ENABLE_OVERLAY
