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


DWORD S3VirgeFourCC[] = {
  MAKEFOURCC('Y', 'U', 'Y', 'V'),
  MAKEFOURCC('Y', 'U', 'Y', '2')
};

// callbacks from the DIRECTDRAW object

DDHAL_DDCALLBACKS cbDDCallbacks =
{
    sizeof( DDHAL_DDCALLBACKS ),        // dwSize
    DDHAL_CB32_DESTROYDRIVER |          // dwFlags
    DDHAL_CB32_CREATESURFACE |
    DDHAL_CB32_SETCOLORKEY |
    DDHAL_CB32_SETMODE |
    DDHAL_CB32_WAITFORVERTICALBLANK |
    DDHAL_CB32_CANCREATESURFACE |
    DDHAL_CB32_CREATEPALETTE |
    DDHAL_CB32_GETSCANLINE |
    DDHAL_CB32_SETEXCLUSIVEMODE |
    DDHAL_CB32_FLIPTOGDISURFACE |
    0,
    HalDestroyDriver,               // DestroyDriver
    HalCreateSurface,               // CreateSurface
    DDGPESetColorKeyDrv,            // SetColorKey
    DDGPESetMode,                   // SetMode
    HalWaitForVerticalBlank,        // WaitForVerticalBlank
    HalCanCreateSurface,            // CanCreateSurface
    DDGPECreatePalette,             // CreatePalette
    HalGetScanLine,                 // GetScanLine
    HalSetExclusiveMode,            // SetExclusiveMode
    DDGPEFlipToGDISurface           // FlipToGDISurface
};

// callbacks from the DIRECTDRAWCOLORCONTROL pseudo object

DDHAL_DDCOLORCONTROLCALLBACKS ColorControlCallbacks =
{
    sizeof(DDHAL_DDCOLORCONTROLCALLBACKS),
#ifdef ENABLE_OVERLAY
    DDHAL_COLOR_COLORCONTROL,
    &HalColorControl
#else
    0, // DDHAL_COLOR_COLORCONTROL,
    0 // &HalColorControl
#endif // ENABLE_OVERLAY
};

// callbacks from the DIRECTDRAWEXEBUF psuedo object

DDHAL_DDEXEBUFCALLBACKS cbDDExeBufCallbacks =
{
    sizeof( DDHAL_DDEXEBUFCALLBACKS ), // dwSize
    DDHAL_EXEBUFCB32_CANCREATEEXEBUF |
    DDHAL_EXEBUFCB32_CREATEEXEBUF    |
    DDHAL_EXEBUFCB32_DESTROYEXEBUF   |
    DDHAL_EXEBUFCB32_LOCKEXEBUF      |
    DDHAL_EXEBUFCB32_UNLOCKEXEBUF,      // dwFlags
    HalCanCreateSurface,                // CanCreateExecuteBuffer
    DDGPECreateExecuteBuffer,           // CreateExecuteBuffer
    DDGPEDestroyExecuteBuffer,          // DestroyExecuteBuffer
#ifndef NEWHAL
    DDGPELock,                          // Lock
    DDGPEUnlock                         // Unlock
#else
    HalLock,                            // Lock
    HalUnlock                           // Unlock
#endif
};

// callbacks from the DIRECTDRAWKERNEL psuedo object

DDHAL_DDKERNELCALLBACKS KernelCallbacks =
{
    sizeof(DDHAL_DDKERNELCALLBACKS),
//    DDHAL_KERNEL_SYNCSURFACEDATA |
//    DDHAL_KERNEL_SYNCVIDEOPORTDATA |
    0,
//    &HalSyncSurfaceData,
//    &HalSyncVideoPortData
    0
};

// callbacks from the DIRECTDRAWMISCELLANEOUS object

DDHAL_DDMISCELLANEOUSCALLBACKS MiscellaneousCallbacks =
{
    sizeof(DDHAL_DDMISCELLANEOUSCALLBACKS),
//    DDHAL_MISCCB32_GETAVAILDRIVERMEMORY |
//    DDHAL_MISCCB32_UPDATENONLOCALHEAP |
//    DDHAL_MISCCB32_GETHEAPALIGNMENT |
//    DDHAL_MISCCB32_GETSYSMEMBLTSTATUS |
    0,
    0, //HalGetAvailDriverMemory,
    0, //HalUpdateNonLocalHeap,
    0, //HalGetHeapAlignment,
    0  //HalGetSysmemBltStatus
};

// callbacks from the DIRECTDRAWPALETTE object

DDHAL_DDPALETTECALLBACKS cbDDPaletteCallbacks =
{
    sizeof( DDHAL_DDPALETTECALLBACKS ), // dwSize
    DDHAL_PALCB32_DESTROYPALETTE |      // dwFlags
    DDHAL_PALCB32_SETENTRIES |
    0,
    DDGPEDestroyPalette,                // DestroyPalette
    DDGPESetEntries                 // SetEntries
};


// callbacks from the DIRECTDRAWSURFACE object

DDHAL_DDSURFACECALLBACKS cbDDSurfaceCallbacks =
{
    sizeof( DDHAL_DDSURFACECALLBACKS ), // dwSize
    DDHAL_SURFCB32_DESTROYSURFACE |     // dwFlags
    DDHAL_SURFCB32_FLIP |
    DDHAL_SURFCB32_SETCLIPLIST |
    DDHAL_SURFCB32_LOCK |
    DDHAL_SURFCB32_UNLOCK |
    DDHAL_SURFCB32_BLT |
    DDHAL_SURFCB32_SETCOLORKEY |
//  DDHAL_SURFCB32_ADDATTACHEDSURFACE |
    DDHAL_SURFCB32_GETBLTSTATUS |
    DDHAL_SURFCB32_GETFLIPSTATUS |
    DDHAL_SURFCB32_UPDATEOVERLAY |
    DDHAL_SURFCB32_SETOVERLAYPOSITION |
//  DDHAL_SURFCB32_RESERVED4 |
    DDHAL_SURFCB32_SETPALETTE |
    0,
    DDGPEDestroySurface,            // DestroySurface
    HalFlip,                        // Flip
    HalSetClipList,                 // SetClipList
#ifndef NEWHAL
    DDGPELock,                      // Lock
    DDGPEUnlock,                    // Unlock
#else
    HalLock,                        // Lock
    HalUnlock,                      // Unlock
#endif
#ifndef NEWHAL
    DDGPEBlt,                       // Blt
#else
    HalBlt,                         // Blt
#endif
    DDGPESetColorKey,               // SetColorKey
    HalAddAttachedSurface,          // AddAttachedSurface
    HalGetBltStatus,                // GetBltStatus
#ifndef NEWHAL
    DDGPEGetFlipStatus,             // GetFlipStatus
#else
    HalGetFlipStatus,               // GetFlipStatus
#endif
    HalUpdateOverlay,               // UpdateOverlay
    HalSetOverlayPosition,          // SetOverlayPosition
    NULL,                           // reserved4
    DDGPESetPalette,                // SetPalette
};

DDHAL_DDHALMEMORYCALLBACKS HalMemoryCallbacks =
{
    sizeof(DDHAL_DDHALMEMORYCALLBACKS),
    DDHAL_KERNEL_HALGETVIDMEM |
//  DDHAL_KERNEL_HALSETSURFACEDESC |
    0,
    &HalGetVidMem,
//  &HalSetSurfaceDesc
};


// callbacks from the DIRECTDRAWVIDEOPORT pseudo object

DDHAL_DDVIDEOPORTCALLBACKS VideoPortCallbacks =
{
    sizeof(DDHAL_DDVIDEOPORTCALLBACKS),
//  DDHAL_VPORT32_CANCREATEVIDEOPORT |
    DDHAL_VPORT32_CREATEVIDEOPORT |
    DDHAL_VPORT32_FLIP |
    DDHAL_VPORT32_GETBANDWIDTH |
    DDHAL_VPORT32_GETINPUTFORMATS |
    DDHAL_VPORT32_GETOUTPUTFORMATS |
    DDHAL_VPORT32_GETFIELD |         
    DDHAL_VPORT32_GETLINE |                 
    DDHAL_VPORT32_GETCONNECT |             
    DDHAL_VPORT32_DESTROY |                 
    DDHAL_VPORT32_GETFLIPSTATUS |          
    DDHAL_VPORT32_UPDATE |                  
    DDHAL_VPORT32_WAITFORSYNC |             
    DDHAL_VPORT32_GETSIGNALSTATUS |         
    DDHAL_VPORT32_COLORCONTROL |           
    0,
    0, //&HalCanCreateVideoPort,
    0, //&HalCreateVideoPort,
    0, //&HalFlipVideoPort,
    0, //&HalGetVideoPortBandwidth,
    0, //&HalGetVideoPortInputFormats,
    0, //&HalGetVideoPortOutputFormats,
    NULL, //lpReserved1
    0, //&HalGetVideoPortField,
    0, //&HalGetVideoPortLine,
    0, //&HalGetVideoPortConnectInfo,
    0, //&HalDestroyVideoPort,
    0, //&HalGetVideoPortFlipStatus,
    0, //&HalUpdateVideoPort,
    0, //&HalWaitForVideoPortSync,
    0, //&HalGetVideoSignalStatus,
    0, //&HalColorControl,
};


const DDKERNELCAPS KernelCaps =
{
    sizeof(DDKERNELCAPS),
//    DDKERNELCAPS_SKIPFIELDS     |
//    DDKERNELCAPS_AUTOFLIP       |
//    DDKERNELCAPS_SETSTATE       |
//    DDKERNELCAPS_LOCK           |
//    DDKERNELCAPS_FLIPVIDEOPORT  |
//    DDKERNELCAPS_FLIPOVERLAY    |
//    DDKERNELCAPS_FIELDPOLARITY  |
    0,
//    DDIRQ_DISPLAY_VSYNC         |
//    DDIRQ_VPORT0_VSYNC          |
    0
};


DDHALMODEINFO modeInfoP[] =
{
//  width height pitch Pixel Palette Freq   Red     Green   Blue    Alpha
    { 640, 480,  1280,   16,   0,      60, 0xF800,   0x07E0, 0x001F,  0 },
    { 800, 600,  1600,   16,   0,      60, 0x7C00,   0x03E0, 0x001F,  0 },
};

DDHALMODEINFO *modeInfo;


#define SCREEN_WIDTH    (g_pGPE->ScreenWidth())
#define SCREEN_HEIGHT   (g_pGPE->ScreenHeight())


// set up by HalInit
// This global pointer is to be recorded in the DirectDraw structure
DDGPE*          g_pGPE                  = (DDGPE*)NULL;
DDGPESurf*      g_pDDrawPrimarySurface  = NULL;

// InitDDHALInfo must set up this information
unsigned long   g_nVideoMemorySize      = 0L;
unsigned char * g_pVideoMemory          = NULL; // virtual address of video 
                                                // memory from client's side
DWORD           g_nTransparentColor     = 0L;


EXTERN_C void buildDDHALInfo( LPDDHALINFO lpddhi, DWORD modeidx )
{
    // set modeInfo the appropriate table entry
    modeInfo = modeInfoP + ((GPEVGA *)g_pGPE)->m_nModeIndex;

    memset( lpddhi, 0, sizeof(DDHALINFO) );     // Clear the DDHALINFO structure

    if( !g_pVideoMemory )   // in case this is called more than once...
    {
        // g_pGPE and g_pDDrawPrimarySurface were already set up by HalInit

        unsigned long physicalVideoMemoryStart;
        g_pGPE->GetPhysicalVideoMemory( &physicalVideoMemoryStart, 
                                        &g_nVideoMemorySize );
        DEBUGMSG( GPE_ZONE_INIT,
           (TEXT("GetPhysicalVideoMemory returned phys=0x%08x size=%d\r\n"),
           physicalVideoMemoryStart, g_nVideoMemorySize));

        // Allocate some virtual memory space for video memory
        // this has already been done in ATI HAP so we don't 
        // need virtual alloc/copy
        g_pVideoMemory = (BYTE*)physicalVideoMemoryStart;

        DEBUGMSG( GPE_ZONE_INIT,(TEXT("gpVidMem=%08x\r\n"), g_pVideoMemory ));
    }

    // Populate the rest of the DDHALINFO structure:
    lpddhi->dwSize = sizeof(DDHALINFO);
    lpddhi->lpDDCallbacks = &cbDDCallbacks;
    lpddhi->lpDDSurfaceCallbacks = &cbDDSurfaceCallbacks;
    lpddhi->lpDDPaletteCallbacks = &cbDDPaletteCallbacks;
    lpddhi->lpDDExeBufCallbacks = &cbDDExeBufCallbacks;
    lpddhi->GetDriverInfo = HalGetDriverInfo;

    // pointer to primary surface
    lpddhi->vmiData.fpPrimary = (unsigned long)(g_pVideoMemory) + 
                                g_pDDrawPrimarySurface->OffsetInVideoMemory();

    lpddhi->vmiData.dwFlags = 0;                    // flags
    lpddhi->vmiData.dwDisplayWidth = SCREEN_WIDTH;  // current display width
    lpddhi->vmiData.dwDisplayHeight = SCREEN_HEIGHT;// current display height
    lpddhi->vmiData.lDisplayPitch = g_pDDrawPrimarySurface->Stride();
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("stride: %d\r\n"), 
              lpddhi->vmiData.lDisplayPitch ));
                                                // current display pitch
    lpddhi->vmiData.ddpfDisplay.dwSize = sizeof(DDPIXELFORMAT);
                                                // ... = 8bit/pixel palettized
    lpddhi->vmiData.ddpfDisplay.dwFlags = DDPF_RGB;
#if 0 // for palette indexed display
    lpddhi->vmiData.ddpfDisplay.dwFlags |= DDPF_PALETTEINDEXED8;
#endif
    lpddhi->vmiData.ddpfDisplay.dwRBitMask = modeInfo[0].dwRBitMask;
    lpddhi->vmiData.ddpfDisplay.dwGBitMask = modeInfo[0].dwGBitMask;
    lpddhi->vmiData.ddpfDisplay.dwBBitMask = modeInfo[0].dwBBitMask;
    lpddhi->vmiData.ddpfDisplay.dwRGBBitCount = modeInfo[0].dwBPP;

    lpddhi->vmiData.dwOffscreenAlign = 8;       // quadword alignment for 
                                                // offscreen surfaces
    lpddhi->vmiData.dwOverlayAlign = 8;         // quadword alignment for 
                                                // overlay surfaces
    lpddhi->vmiData.dwTextureAlign = 0;         // byte alignment for textures
    lpddhi->vmiData.dwZBufferAlign = 0;         // byte alignment for z buffers
    lpddhi->vmiData.dwAlphaAlign = 0;           // byte alignment for alpha
    lpddhi->vmiData.dwNumHeaps = 0;             // number of memory heaps 
                                                // in vmList
    lpddhi->vmiData.pvmList = (LPVIDMEM)NULL;   // array of heaps

    // hw specific caps:
    lpddhi->ddCaps.dwSize = sizeof(DDCAPS);     // size of the DDDRIVERCAPS 
                                                // structure
    lpddhi->ddCaps.dwCaps =             // driver specific capabilities
        // DDCAPS_3D |                  // Display hardware has 3D acceleration
        DDCAPS_BLT |                    // Display hardware is capable of 
                                        // blt operations
        // DDCAPS_BLTQUEUE |            // Display hardware is capable of 
                                        // asynchronous blt operations
        // DDCAPS_BLTFOURCC |           // Display hardware is capable of 
                                        // color space conversions 
                                        // during the blt operation
        // DDCAPS_BLTSTRETCH |          // Display hardware is capable of 
                                        // stretching during blt operations
        DDCAPS_GDI |                    // Display hardware is shared with GDI
        DDCAPS_OVERLAY |                // Display hardware can overlay
        DDCAPS_OVERLAYCANTCLIP |        // Set if display hardware supports 
                                        // overlays but can not clip them
        DDCAPS_OVERLAYFOURCC |          // overlay hardware is capable of 
                                        // color space conversions
        DDCAPS_OVERLAYSTRETCH |         // Indicates that stretching can be 
                                        // done by the overlay hardware
        // DDCAPS_PALETTE |             // unique DirectDrawPalettes can be 
                                        // created for DirectDrawSurfaces
        // DDCAPS_PALETTEVSYNC |        // palette changes can be syncd with 
                                        // the vertical
        // DDCAPS_READSCANLINE |        // Display hardware can return 
                                        // the current scan line
        // DDCAPS_STEREOVIEW |          // Display hardware has stereo 
                                        // vision capabilities
        // DDCAPS_VBI |                 // Display hardware is capable of 
                                        // generating a vertical blank interrupt
        // DDCAPS_ZBLTS |               // Supports the use of z buffers with 
                                        // blt operations
        // DDCAPS_ZOVERLAYS |           // Supports Z Ordering of overlays
        DDCAPS_COLORKEY |               // Supports color key
        // DDCAPS_ALPHA |               // Supports alpha surfaces
        DDCAPS_COLORKEYHWASSIST |       // colorkey is hardware assisted
        // DDCAPS_NOHARDWARE |          // no hardware support at all
        DDCAPS_BLTCOLORFILL |           // Display hardware is capable of 
                                        // color fill with bltter
        // DDCAPS_BANKSWITCHED |        // Display hardware is bank switched
        // DDCAPS_BLTDEPTHFILL |        // Display hardware is capable of 
                                        // depth filling Z-buffers with bltter
        // DDCAPS_CANCLIP |             // Display hardware is capable of 
                                        // clipping while bltting
        // DDCAPS_CANCLIPSTRETCHED |    // Display hardware is capable of 
                                        // clipping while stretch bltting
        DDCAPS_CANBLTSYSMEM |           // Display hardware is capable of 
                                        // bltting to or from system memory
        0;

    lpddhi->ddCaps.dwCaps2 =            // more driver specific capabilities
        // DDCAPS2_CERTIFIED            // Display hardware is certified
        // DDCAPS2_NO2DDURING3DSCENE |  // Driver cannot interleave 
                                        // 2D & 3D operations
        // DDCAPS2_VIDEOPORT |          // Display hardware contains 
                                        // a video port
        // DDCAPS2_AUTOFLIPOVERLAY |    // automatic doubled buffered 
                                        // display of video port
        // DDCAPS2_CANBOBINTERLEAVED |  // Overlay can display each field 
                                        // of interlaced data
        // DDCAPS2_CANBOBNONINTERLEAVED |   // As above but for 
                                            // non-interleaved data
        DDCAPS2_COLORCONTROLOVERLAY |   // The overlay surface contains 
                                        // color controls
        // DDCAPS2_COLORCONTROLPRIMARY |// The primary surface contains 
                                        // color controls
        // DDCAPS2_CANDROPZ16BIT |      // RGBZ -> RGB supported for 16:16 RGB:Z
        // DDCAPS2_NONLOCALVIDMEM |     // Driver supports non-local 
                                        // video memory
        // DDCAPS2_NONLOCALVIDMEMCAPS | // Dirver supports non-local video 
                                        // memory but has different capabilities
        DDCAPS2_NOPAGELOCKREQUIRED |    // Driver neither requires nor 
                                        // prefers surfaces to be pagelocked
        // DDCAPS2_WIDESURFACES |       // Driver can create surfaces which are
                                        // wider than the primary surface
        // DDCAPS2_CANFLIPODDEVEN |     // Driver supports bob without using 
                                        // a video port
        // DDCAPS2_CANBOBHARDWARE |     // Driver supports bob using hardware
        // DDCAPS2_COPYFOURCC |         // Driver supports bltting 
                                        // any FOURCC surface to another 
                                        // surface of the same FOURCC
        0 ;

    lpddhi->ddCaps.dwCKeyCaps =         // color key capabilities of the surface
        DDCKEYCAPS_DESTOVERLAY |        // Supports RGB overlaying with 
                                        // dest color key
        DDCKEYCAPS_DESTOVERLAYONEACTIVE |   // Supports only one active dest
                                            // color key value
        DDCKEYCAPS_SRCBLT |             // Hardware can use colorkey (cf source
                                        // only) ..for transparent blts
        DDCKEYCAPS_SRCOVERLAY |         // Supports RGB overlaying with
                                        // source color key
        DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV |  // Supports YUV overlaying using
                                            // a color space as source color key
        DDCKEYCAPS_SRCOVERLAYONEACTIVE |    // Supports only one active 
                                            // source color key value
        DDCKEYCAPS_SRCOVERLAYYUV |      // Supports YUV overlaying with
                                        // source color key
        0;

    lpddhi->ddCaps.dwFXCaps=            // driver specific stretching and 
                                        // effects capabilites
        // DDFXCAPS_BLTMIRRORUPDOWN |   // Supports vertical inversion Blts
        // DDFXCAPS_BLTSTRETCHY |       // Supports stretch blts in 
                                        // the Y-direction
        // DDFXCAPS_BLTSHRINKY |        // Supports shrink blts in 
                                        // the Y-direction
        // DDFXCAPS_BLTSTRETCHX |       // Supports stretch blts in 
                                        // the X-direction
        // DDFXCAPS_BLTSHRINKX |        // Supports shrink blts in 
                                        // the X-direction
        DDFXCAPS_OVERLAYSTRETCHX |      // Supports arbitrary stretching
                                        // along x-axis
        DDFXCAPS_OVERLAYSTRETCHY |      // Supports arbitrary stretching
                                        // along y-axis
        0;

    lpddhi->ddCaps.dwPalCaps;           // palette capabilities
        // DDPCAPS_1BIT |               // Simple 1-bit palette
        // DDPCAPS_2BIT |               // Simple 2-bit palette
        // DDPCAPS_4BIT |               // Simple 4-bit palette
        // DDPCAPS_8BITENTRIES |        // Palette indexes into 8 bit target
        DDPCAPS_8BIT |                  // Simple 8-bit palette
        DDPCAPS_INITIALIZE |            // DDraw should initalize palette
                                        //   ..from lpDDColorArray
        DDPCAPS_PRIMARYSURFACE |        // Palette is attached to 
                                        // primary surface
        // DDPCAPS_PRIMARYSURFACELEFT | // Palette is attached to 
                                        // left-eye primary surface
        DDPCAPS_ALLOW256 |              // All 256 entries may be set
        0;
    lpddhi->ddCaps.dwSVCaps= 0;         // Stereo vision capabilities (none)
    lpddhi->ddCaps.dwAlphaBltConstBitDepths = 0;// No Alpha Blt's
    lpddhi->ddCaps.dwAlphaBltPixelBitDepths = 0;
    lpddhi->ddCaps.dwAlphaBltSurfaceBitDepths = 0;
    lpddhi->ddCaps.dwZBufferBitDepths= 0;       // No z buffer
    lpddhi->ddCaps.dwVidMemTotal = g_nVideoMemorySize;  // total amount of 
                                                        // video memory
    lpddhi->ddCaps.dwVidMemFree = g_nVideoMemorySize;   // amount of free 
                                                        // video memory

    lpddhi->ddCaps.dwMaxVisibleOverlays = 1;    // max num of visible overlays
    lpddhi->ddCaps.dwCurrVisibleOverlays = 0;   // curr num of visible overlays
    lpddhi->ddCaps.dwNumFourCCCodes = 2;        // number of four cc codes
    lpddhi->ddCaps.dwAlignBoundarySrc = 0;      // source rectangle alignment
    lpddhi->ddCaps.dwAlignSizeSrc = 0;          // source rectangle byte size
    lpddhi->ddCaps.dwAlignBoundaryDest = 0;     // dest rectangle alignment
    lpddhi->ddCaps.dwAlignSizeDest = 0;         // dest rectangle byte size
    lpddhi->ddCaps.dwAlignStrideAlign = 0;      // stride alignment

    lpddhi->ddCaps.ddsCaps.dwCaps=              // DDSCAPS structure has 
                                                // all the general capabilities
        // DDSCAPS_ALPHA |              // Can create alpha-only surfaces
        DDSCAPS_BACKBUFFER |            // Can create backbuffer surfaces
        DDSCAPS_COMPLEX |               // Can create complex surfaces
        DDSCAPS_FLIP |                  // Can flip between surfaces
        DDSCAPS_FRONTBUFFER |           // Can create front-buffer surfaces
        DDSCAPS_OFFSCREENPLAIN |        // Can create off-screen bitmaps
        DDSCAPS_OVERLAY |               // Can create overlay surfaces
        DDSCAPS_PALETTE |               // Has one palette ???
        DDSCAPS_PRIMARYSURFACE |        // Has a primary surface
        // DDSCAPS_PRIMARYSURFACELEFT | // Has a left-eye primary surface
        // DDSCAPS_TEXTURE |            // Supports texture surrfaces
        // DDSCAPS_SYSTEMMEMORY |       // Surfaces are in system memory
        DDSCAPS_VIDEOMEMORY |           // Surfaces are in video memory 
        DDSCAPS_VISIBLE |               // Changes are instant ???
        // DDSCAPS_ZBUFFER |            // Can create (pseudo) Z buffer
        // DDSCAPS_EXECUTEBUFFER |      // Can create execute buffer
        // DDSCAPS_3DDEVICE |           // Surfaces can be 3d targets
        // DDSCAPS_WRITEONLY |          // Can create write-only surfaces
        // DDSCAPS_ALLOCONLOAD |        // Can create alloconload surfaces
        // DDSCAPS_MIPMAP |             // Can create mipmap
        0;

    SETROPBIT(lpddhi->ddCaps.dwRops,SRCCOPY);   // Set bits for ROPS supported
    SETROPBIT(lpddhi->ddCaps.dwRops,PATCOPY);
    SETROPBIT(lpddhi->ddCaps.dwRops,BLACKNESS);
    SETROPBIT(lpddhi->ddCaps.dwRops,WHITENESS);

    lpddhi->ddCaps.dwMinOverlayStretch = 1000;  // min overlay stretch factor
                                                // can't be scaled down
    lpddhi->ddCaps.dwMaxOverlayStretch = 9999;  // max overlay stretch factor

    // caps for system to video blts
    lpddhi->ddCaps.dwSVBCaps =          // driver specific capabilities
        DDCAPS_BLT |                    // Display hardware is capable of 
                                        // blt operations
        DDCAPS_COLORKEY |               // Supports color key
        DDCAPS_COLORKEYHWASSIST |       // colorkey is hardware assisted
        DDCAPS_BLTCOLORFILL |           // Display hardware is capable of 
                                        // color fill with bltter
        0;
    lpddhi->ddCaps.dwSVBCKeyCaps =      // color key capabilities of the surface
        DDCKEYCAPS_SRCBLT |             // Hardware can use colorkey (cf source
                                        // only) ..for transparent blts
        0;
    lpddhi->ddCaps.dwSVBFXCaps=         // driver specific stretching and 
                                        // effects capabilites
        0;
    SETROPBIT(lpddhi->ddCaps.dwSVBRops,SRCCOPY);
    SETROPBIT(lpddhi->ddCaps.dwSVBRops,PATCOPY);
    SETROPBIT(lpddhi->ddCaps.dwSVBRops,BLACKNESS);
    SETROPBIT(lpddhi->ddCaps.dwSVBRops,WHITENESS);
   
    lpddhi->dwMonitorFrequency = modeInfo[0].wRefreshRate;   
                                            // monitor frequency
    lpddhi->dwModeIndex = 0;                // current mode: index into array
    lpddhi->lpdwFourCC = S3VirgeFourCC;     // fourcc codes supported
    lpddhi->dwNumModes = 1;          //sizeof(modeInfo)/sizeof(DDHALMODEINFO);
                                            // number of modes supported
    lpddhi->lpModeInfo = modeInfo;          // mode information
    lpddhi->dwFlags =
                    DDHALINFO_MODEXILLEGAL |    // create flags
                    DDHALINFO_GETDRIVERINFOSET |
                    0;
    lpddhi->lpPDevice = (LPVOID)0;          // physical device ptr
    lpddhi->hInstance = (DWORD)0;           // instance handle of driver
}

