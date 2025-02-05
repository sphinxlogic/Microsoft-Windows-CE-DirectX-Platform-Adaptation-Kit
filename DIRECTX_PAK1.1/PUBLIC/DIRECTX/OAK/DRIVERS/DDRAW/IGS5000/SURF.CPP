// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-2000  Microsoft Corporation
//
//  Module Name:
//
//	surf.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#include "precomp.h"


//
//  AllocSurface -
//  inputs:
//  outputs:
//
SCODE IGS5000::AllocSurface(
    GPESurf **ppSurf,
    int width,
    int height,
    EGPEFormat format,
    int surfaceFlags )
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


//
//  AllocSurface -
//  inputs:
//  outputs:
//
SCODE IGS5000::AllocSurface(
    DDGPESurf **ppSurf,
    int width,
    int height,
    EGPEFormat format,
    EDDGPEPixelFormat pixelFormat,
    int surfaceFlags )
{

	
    if( surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY || 
        surfaceFlags & GPE_PREFER_VIDEO_MEMORY )    {

		WORD bytepp = (EGPEFormatToBpp[format]/8);
		ulong stride = ((width+7)&~0x7)*bytepp;
		ulong heapsize = stride*height;
		SurfaceHeap *pHeap = m_pLinearVideoMemory->Alloc(heapsize);
		if (pHeap) {
			ulong ofs = pHeap->Address() - (DWORD)m_pLAW;
	        *ppSurf = new IGS5000Surf(
		                        width,
		                        height,
								ofs,
		                        (void*) pHeap->Address(),		                  
		                        stride,
		                        format,
                                pixelFormat,
		                        pHeap);
            if (!(*ppSurf)) {
                DEBUGMSG(TRUE, (TEXT("*ppSurf is NULL\n\r") ));
                pHeap->Free();
            }
			return S_OK;
		}
        DEBUGMSG(1, (TEXT("pHeap is NULL\n\r") ));
	    if( surfaceFlags & GPE_REQUIRE_VIDEO_MEMORY ) {
	        *ppSurf = (DDGPESurf *)NULL;
	        return E_OUTOFMEMORY;
	    }

    }
    // Allocate from system memory
    DEBUGMSG(GPE_ZONE_CREATE,(TEXT("Creating a GPESurf in system memory. EGPEFormat = %d\r\n"), (int)format ));
    *ppSurf = new DDGPESurf( width, height, format );
    if(*ppSurf) 
    {
	    // check we allocated bits succesfully
	    if(!((*ppSurf)->Buffer())) 
        {
	        delete *ppSurf; // and then return E_OUTOFMEMORY
	    } 
        else 
        {
	        return S_OK;
	    }
    }
    return E_OUTOFMEMORY;
}

//
//  IGS5000Surf - constructor
//  inputs:
//  outputs:
//
IGS5000Surf::IGS5000Surf(
    int width,
    int height,
    unsigned long offset,
    void *pBits,	    // virtual address of allocated bits
    int stride,
    EGPEFormat format,
    SurfaceHeap *pHeap )
    : DDGPESurf( width, height, pBits, stride, format ) {

    m_pSurfHeap = pHeap;
    m_fInVideoMemory = TRUE;
    m_nOffsetInVideoMemory = offset;

	m_SurfWidth = (width+7) & (int) ~0x07;
#ifdef FB16BPP
	m_CopOfs = offset/2;
#else
	m_CopOfs = offset;
#endif

}


//
//  IGS5000Surf - constructor
//  inputs:
//  outputs:
//
IGS5000Surf::IGS5000Surf(
    int width,
    int height,
    unsigned long offset,
    void *pBits,        // virtual address of allocated bits
    int stride,
    EGPEFormat format,
    EDDGPEPixelFormat pixelFormat,
    SurfaceHeap *pHeap )
    : DDGPESurf( width, height, pBits, stride, format, pixelFormat )
{
    m_pSurfHeap = pHeap;
    m_fInVideoMemory = TRUE;
    m_nOffsetInVideoMemory = offset;

	m_SurfWidth = (width+7) & (int) ~0x07;
#ifdef FB16BPP
	m_CopOfs = offset/2;
#else
	m_CopOfs = offset;
#endif

}


//
//  ~IGS5000Surf - destructor
//  inputs:
//  outputs:
//
IGS5000Surf::~IGS5000Surf() {

	m_pSurfHeap->Free();

}

//
//  CheckVisibleSurface -
//  inputs:
//  outputs:
//
void IGS5000::CheckVisibleSurface()
{
    unsigned long tickCount = GetTickCount();
    if( tickCount - m_nTicksAtFlip > m_nTicksPerFrame + 1 ) {
	m_pOldVisibleSurface = m_pNewVisibleSurface;
	return;
    }
    unsigned int phase = ( tickCount - m_nTicksAtResync ) % m_nTicksPerFrame;
    unsigned int flipPhase = ( m_nTicksAtFlip - m_nTicksAtResync ) % m_nTicksPerFrame;
    if( (phase >= m_nTicksPerFrame * 7 / 8) ||	// near end of frame, may as well wait
	( tickCount - m_nTicksAtResync > m_nTicksPerFrame * 20 ) || // overdue for resync
	( tickCount < m_nTicksAtFlip ) ||   // Time went backwards
	( m_nTicksAtFlip < m_nTicksAtResync ) ) { // Time went backwards
	WaitForVBlank();
	return;
    }
    if( phase < flipPhase ) {
	m_pOldVisibleSurface = m_pNewVisibleSurface;
	return;
    }
    DEBUGMSG( GPE_ZONE_FLIP,
	( TEXT("Not flipped. Ticks at flip:%d, now %d, "),
 	m_nTicksAtFlip,
	tickCount ) );
    DEBUGMSG( GPE_ZONE_FLIP,
	( TEXT("per frame %d resync %d\r\n"),
	m_nTicksPerFrame,
	m_nTicksAtResync ) );
}

//
//  SurfaceBusyFlipping -
//  inputs:
//  outputs:
//
int IGS5000::SurfaceBusyFlipping( IGS5000Surf *pSurf )
{

    if( pSurf == m_pOldVisibleSurface || pSurf == m_pNewVisibleSurface ) {
	CheckVisibleSurface();
    }

    return ( pSurf == m_pOldVisibleSurface || pSurf == m_pNewVisibleSurface );
}

//
//  FlipInProgress -
//  inputs:
//  outputs:
//
int IGS5000::FlipInProgress() {

    if( m_pOldVisibleSurface != m_pNewVisibleSurface ) {
	CheckVisibleSurface();
    } else {
	return 0;
    }

    if( m_pOldVisibleSurface != m_pNewVisibleSurface ) {
	DEBUGMSG( GPE_ZONE_FLIP, ( TEXT("Flip from surf 0x%08x to 0x%08x in progress\r\n"),
	    m_pOldVisibleSurface, m_pNewVisibleSurface ));
    }

    return ( m_pOldVisibleSurface != m_pNewVisibleSurface );
}

#if DEBUG
int tickCountValid = 0;
unsigned int lastTickCount;
int frameCount = 0;
#endif

//
//  SetVisibleSurface -
//  inputs:
//  outputs:
//
void IGS5000::SetVisibleSurface( GPESurf *pTempSurf ) {

    IGS5000Surf *pSurf = (IGS5000Surf *) pTempSurf;
	int displayStart = pSurf->OffsetInVideoMemory();

    m_pNewVisibleSurface = pSurf;
    displayStart >>= 2;

    reg_GR[0x10] = 0x10 | ((displayStart>>16) & 0x0F);
    reg_STA_H = (displayStart>>8) & 0xFF;
    reg_STA_L = displayStart & 0xFF;
    m_nTicksAtFlip = GetTickCount();
#if DEBUG
    if( tickCountValid ) {
	frameCount++;
	if( m_nTicksAtFlip - lastTickCount > 10000 ) {
	    int framesPerTenSecs = frameCount * 10000 / ( m_nTicksAtFlip - lastTickCount );
	    if( framesPerTenSecs < 2000 ) {
		DEBUGMSG( GPE_ZONE_PERF, (TEXT("Frames per second: %d.%1d\r\n"),
		    framesPerTenSecs/10, framesPerTenSecs%10 ));
	    }
	    lastTickCount = m_nTicksAtFlip;
	    frameCount = 0;
	}
    } else {
	tickCountValid = 1;
	lastTickCount = m_nTicksAtFlip;
	frameCount = 0;
    }
#endif
}

void
IGS5000::SetVisibleSurface(GPESurf *pSurf, BOOL bWaitForVBlank) {

	if (bWaitForVBlank) {
		while (InVBlank()) {}
		while (!InVBlank()) {}
	}
    SetVisibleSurface(pSurf);
}

//
//  VBlankReceived -
//  inputs:
//  outputs:
//
void IGS5000::VBlankReceived() {
    m_nTicksAtFlip = m_nTicksAtResync = GetTickCount();
    m_pOldVisibleSurface = m_pNewVisibleSurface;
}

