/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: ddraw.c

    PURPOSE: DirectX Device Viewer - DirectDraw module

****************************************************************************/

#include <stdio.h>
#include "dxview.h"
#include "support.h"
#include "ddinfo.h"

#pragma comment(lib, "ddraw")

// Suppress unary operator warning from BitCount()
#pragma warning(4: 4146)

/****************************************************************************
 ***************************************************************************/
CAPDEF OtherInfoDefs[] =
{
    DDVALDEF(TEXT("VidMemTotal"),                   dwVidMemTotal),
    DDVALDEF(TEXT("VidMemFree"),                    dwVidMemFree),
    DDHEXDEF(TEXT("AlphaBltConstBitDepths"),        dwAlphaBltConstBitDepths),
    DDCAPDEF(TEXT("  8"),                           dwAlphaBltConstBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                          dwAlphaBltConstBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                          dwAlphaBltConstBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                          dwAlphaBltConstBitDepths, DDBD_32),
    DDHEXDEF(TEXT("AlphaBltPixelBitDepths"),        dwAlphaBltPixelBitDepths),
    DDCAPDEF(TEXT("  8"),                           dwAlphaBltPixelBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                          dwAlphaBltPixelBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                          dwAlphaBltPixelBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                          dwAlphaBltPixelBitDepths, DDBD_32),
    DDHEXDEF(TEXT("AlphaBltSurfaceBitDepths"),      dwAlphaBltSurfaceBitDepths),
    DDCAPDEF(TEXT("  8"),                           dwAlphaBltSurfaceBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                          dwAlphaBltSurfaceBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                          dwAlphaBltSurfaceBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                          dwAlphaBltSurfaceBitDepths, DDBD_32),
    DDHEXDEF(TEXT("AlphaOverlayConstBitDepths"),    dwAlphaOverlayConstBitDepths),
    DDCAPDEF(TEXT("  8"),                           dwAlphaOverlayConstBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                          dwAlphaOverlayConstBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                          dwAlphaOverlayConstBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                          dwAlphaOverlayConstBitDepths, DDBD_32),
    DDHEXDEF(TEXT("AlphaOverlayPixelBitDepths"),    dwAlphaOverlayPixelBitDepths),
    DDCAPDEF(TEXT("  8"),                           dwAlphaOverlayPixelBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                          dwAlphaOverlayPixelBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                          dwAlphaOverlayPixelBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                          dwAlphaOverlayPixelBitDepths, DDBD_32),
    DDHEXDEF(TEXT("AlphaOverlaySurfaceBitDepths"),  dwAlphaOverlaySurfaceBitDepths),
    DDCAPDEF(TEXT("  8"),                           dwAlphaOverlaySurfaceBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                          dwAlphaOverlaySurfaceBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                          dwAlphaOverlaySurfaceBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                          dwAlphaOverlaySurfaceBitDepths, DDBD_32),
    DDHEXDEF(TEXT("ZBufferBitDepths"),              dwZBufferBitDepths),
    DDCAPDEF(TEXT("  8"),                           dwZBufferBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                          dwZBufferBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                          dwZBufferBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                          dwZBufferBitDepths, DDBD_32),
    DDVALDEF(TEXT("MaxVisibleOverlays"),            dwMaxVisibleOverlays),
    DDVALDEF(TEXT("CurrVisibleOverlays"),           dwCurrVisibleOverlays),
    DDVALDEF(TEXT("NumFourCCCodes"),                dwNumFourCCCodes),
    DDVALDEF(TEXT("AlignBoundarySrc"),              dwAlignBoundarySrc),
    DDVALDEF(TEXT("AlignSizeSrc"),                  dwAlignSizeSrc),
    DDVALDEF(TEXT("AlignBoundaryDest"),             dwAlignBoundaryDest),
    DDVALDEF(TEXT("AlignSizeDest"),                 dwAlignSizeDest),
    DDVALDEF(TEXT("AlignStrideAlign"),              dwAlignStrideAlign),
    DDVALDEF(TEXT("MinOverlayStretch"),             dwMinOverlayStretch),
    DDVALDEF(TEXT("MaxOverlayStretch"),             dwMaxOverlayStretch),
    DDVALDEF(TEXT("MinLiveVideoStretch"),           dwMinLiveVideoStretch),
    DDVALDEF(TEXT("MaxLiveVideoStretch"),           dwMaxLiveVideoStretch),
    DDVALDEF(TEXT("MinHwCodecStretch"),             dwMinHwCodecStretch),
    DDVALDEF(TEXT("MaxHwCodecStretch"),             dwMaxHwCodecStretch),
    DDVALDEF(TEXT("dwReserved1"),                   dwReserved1),
    DDVALDEF(TEXT("dwReserved2"),                   dwReserved2),
    DDVALDEF(TEXT("dwReserved3"),                   dwReserved3),
    DDVALDEF(TEXT("dwMaxVideoPorts"),               dwMaxVideoPorts),
    DDVALDEF(TEXT("dwCurrVideoPorts"),               dwCurrVideoPorts),
    { TEXT(""), 0, 0 }
};


/****************************************************************************
 ***************************************************************************/
CAPDEF GenCaps[] =
{
    GEN_CAPS(dwCaps)
    GEN_CAPS2(dwCaps2)
    { TEXT(""), 0, 0 }
};

//
// NOTE GenCaps and CapsDefs are the same
// because some are blt caps that apply to VRAM->VRAM blts and
// some are "general" caps...
//
CAPDEF CapsDefs[] =
{
    GEN_BLTCAPS(dwCaps)
    GEN_BLTCAPS2(dwCaps2)
    { TEXT(""), 0, 0 }
};
CAPDEF SVBCapsDefs[] =
{
    GEN_BLTCAPS(dwSVBCaps)
    GEN_BLTCAPS2(dwSVBCaps2)
    { TEXT(""), 0, 0 }
};
CAPDEF VSBCapsDefs[] =
{
    GEN_BLTCAPS(dwVSBCaps)
    { TEXT(""), 0, 0 }
};
CAPDEF SSBCapsDefs[] =
{
    GEN_BLTCAPS(dwSSBCaps)
    { TEXT(""), 0, 0 }
};
CAPDEF NLVBCapsDefs[] =
{
    GEN_BLTCAPS(dwNLVBCaps)
    GEN_BLTCAPS2(dwNLVBCaps2)
    { TEXT(""), 0, 0 }
};

CAPDEF CKeyCapsDefs[] =
{
    GEN_CKCAPS(dwCKeyCaps)
    { TEXT(""), 0, 0}
};
CAPDEF VSBCKeyCapsDefs[] =
{
    GEN_CKCAPS(dwVSBCKeyCaps)
    { TEXT(""), 0, 0}
};
CAPDEF SVBCKeyCapsDefs[] =
{
    GEN_CKCAPS(dwSVBCKeyCaps)
    { TEXT(""), 0, 0}
};
CAPDEF SSBCKeyCapsDefs[] =
{
    GEN_CKCAPS(dwSSBCKeyCaps)
    { TEXT(""), 0, 0}
};
CAPDEF NLVBCKeyCapsDefs[] =
{
    GEN_CKCAPS(dwNLVBCKeyCaps)
    { TEXT(""), 0, 0}
};


/****************************************************************************
 ***************************************************************************/

CAPDEF FXCapsDefs[] =
{
    GEN_FXCAPS(dwFXCaps)
    { TEXT(""), 0, 0}
};
CAPDEF VSBFXCapsDefs[] =
{
    GEN_FXCAPS(dwVSBFXCaps)
    { TEXT(""), 0, 0}
};
CAPDEF SVBFXCapsDefs[] =
{
    GEN_FXCAPS(dwSVBFXCaps)
    { TEXT(""), 0, 0}
};
CAPDEF SSBFXCapsDefs[] =
{
    GEN_FXCAPS(dwSSBFXCaps)
    { TEXT(""), 0, 0}
};
CAPDEF NLVBFXCapsDefs[] =
{
    GEN_FXCAPS(dwNLVBFXCaps)
    { TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/

CAPDEF FXAlphaCapsDefs[] =
{
    GEN_FXALPHACAPS(dwFXAlphaCaps)
    { TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF PalCapsDefs[] =
{
    DDCAPDEF(TEXT("1BIT"),              dwPalCaps, DDPCAPS_1BIT),
    DDCAPDEF(TEXT("2BIT"),              dwPalCaps, DDPCAPS_2BIT),
    DDCAPDEF(TEXT("4BIT"),              dwPalCaps, DDPCAPS_4BIT),
    DDCAPDEF(TEXT("8BITENTRIES"),       dwPalCaps, DDPCAPS_8BITENTRIES),
    DDCAPDEF(TEXT("8BIT"),              dwPalCaps, DDPCAPS_8BIT),
    DDCAPDEF(TEXT("ALLOW256"),          dwPalCaps, DDPCAPS_ALLOW256),
    DDCAPDEF(TEXT("ALPHA"),             dwPalCaps, DDPCAPS_ALPHA),
    DDCAPDEF(TEXT("INITIALIZE"),        dwPalCaps, DDPCAPS_INITIALIZE),
    DDCAPDEF(TEXT("PRIMARYSURFACE"),    dwPalCaps, DDPCAPS_PRIMARYSURFACE),
    DDCAPDEF(TEXT("PRIMARYSURFACELEFT"),dwPalCaps, DDPCAPS_PRIMARYSURFACELEFT),
    DDCAPDEF(TEXT("VSYNC"),             dwPalCaps, DDPCAPS_VSYNC),
    { TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF SurfCapsDefs[] =
{
    DDCAPDEF( TEXT("3DDEVICE"),             ddsCaps.dwCaps, DDSCAPS_3DDEVICE),
    DDCAPDEF( TEXT("ALPHA"),                ddsCaps.dwCaps, DDSCAPS_ALPHA),
    DDCAPDEF( TEXT("BACKBUFFER"),           ddsCaps.dwCaps, DDSCAPS_BACKBUFFER),
    DDCAPDEF( TEXT("COMPLEX"),              ddsCaps.dwCaps, DDSCAPS_COMPLEX),
    DDCAPDEF( TEXT("FLIP"),                 ddsCaps.dwCaps, DDSCAPS_FLIP),
    DDCAPDEF( TEXT("FRONTBUFFER"),          ddsCaps.dwCaps, DDSCAPS_FRONTBUFFER),
    DDCAPDEF( TEXT("HWCODEC"),              ddsCaps.dwCaps, DDSCAPS_HWCODEC),
    DDCAPDEF( TEXT("LIVEVIDEO"),            ddsCaps.dwCaps, DDSCAPS_LIVEVIDEO),
    DDCAPDEF( TEXT("MIPMAP"),               ddsCaps.dwCaps, DDSCAPS_MIPMAP),
    DDCAPDEF( TEXT("MODEX"),                ddsCaps.dwCaps, DDSCAPS_MODEX),
    DDCAPDEF( TEXT("OFFSCREENPLAIN"),       ddsCaps.dwCaps, DDSCAPS_OFFSCREENPLAIN),
    DDCAPDEF( TEXT("OVERLAY"),              ddsCaps.dwCaps, DDSCAPS_OVERLAY),
    DDCAPDEF( TEXT("OWNDC"),                ddsCaps.dwCaps, DDSCAPS_OWNDC),
    DDCAPDEF( TEXT("PALETTE"),              ddsCaps.dwCaps, DDSCAPS_PALETTE),
    DDCAPDEF( TEXT("PRIMARYSURFACE"),       ddsCaps.dwCaps, DDSCAPS_PRIMARYSURFACE),
    DDCAPDEF( TEXT("PRIMARYSURFACELEFT"),   ddsCaps.dwCaps, DDSCAPS_PRIMARYSURFACELEFT),
    DDCAPDEF( TEXT("SYSTEMMEMORY"),         ddsCaps.dwCaps, DDSCAPS_SYSTEMMEMORY),
    DDCAPDEF( TEXT("TEXTURE"),              ddsCaps.dwCaps, DDSCAPS_TEXTURE),
    DDCAPDEF( TEXT("VIDEOMEMORY"),          ddsCaps.dwCaps, DDSCAPS_VIDEOMEMORY),
    DDCAPDEF( TEXT("VIDEOPORT"),            ddsCaps.dwCaps, DDSCAPS_VIDEOPORT),
    DDCAPDEF( TEXT("VISIBLE"),              ddsCaps.dwCaps, DDSCAPS_VISIBLE),
    DDCAPDEF( TEXT("WRITEONLY"),            ddsCaps.dwCaps, DDSCAPS_WRITEONLY),
    DDCAPDEF( TEXT("ZBUFFER"),              ddsCaps.dwCaps, DDSCAPS_ZBUFFER),
    { TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF SVisionCapsDefs[] =
{
    DDCAPDEF( TEXT("ENIGMA"),  dwSVCaps, DDSVCAPS_ENIGMA),
    DDCAPDEF( TEXT("FLICKER"), dwSVCaps, DDSVCAPS_FLICKER),
    DDCAPDEF( TEXT("REDBLUE"), dwSVCaps, DDSVCAPS_REDBLUE),
    DDCAPDEF( TEXT("SPLIT"),   dwSVCaps, DDSVCAPS_SPLIT),
    { TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/

CAPDEF ROPCapsDefs[] =
{
    GEN_ROPS(dwRops)
    {TEXT(""), 0, 0}
};
CAPDEF VSBROPCapsDefs[] =
{
    GEN_ROPS(dwVSBRops)
    {TEXT(""), 0, 0}
};
CAPDEF SVBROPCapsDefs[] =
{
    GEN_ROPS(dwSVBRops)
    {TEXT(""), 0, 0}
};
CAPDEF SSBROPCapsDefs[] =
{
    GEN_ROPS(dwSSBRops)
    {TEXT(""), 0, 0}
};
CAPDEF NLVBROPCapsDefs[] =
{
    GEN_ROPS(dwNLVBRops)
    {TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF OverlayCapsDefs[] =
{
    DDCAPDEF(TEXT("OVERLAY"),                         dwCaps, DDSCAPS_OVERLAY),

    DDCAPDEF(TEXT("ALIGNBOUNDARYDEST"),               dwCaps, DDCAPS_ALIGNBOUNDARYDEST),
    DDVALDEF(TEXT("  AlignBoundaryDest"),             dwAlignBoundaryDest),
    DDCAPDEF(TEXT("ALIGNSIZEDEST"),                   dwCaps, DDCAPS_ALIGNSIZEDEST),
    DDVALDEF(TEXT("  AlignSizeDest"),                 dwAlignSizeDest),
    DDCAPDEF(TEXT("ALIGNBOUNDARYSRC"),                dwCaps, DDCAPS_ALIGNBOUNDARYSRC),
    DDVALDEF(TEXT("  AlignBoundarySrc"),              dwAlignBoundarySrc),
    DDCAPDEF(TEXT("ALIGNSIZESRC"),                    dwCaps, DDCAPS_ALIGNSIZESRC),
    DDVALDEF(TEXT("  AlignSizeSrc"),                  dwAlignSizeSrc),
    DDCAPDEF(TEXT("ALIGNSTRIDE"),                     dwCaps, DDCAPS_ALIGNSTRIDE),
    DDVALDEF(TEXT("  AlignStrideAlign"),              dwAlignStrideAlign),

    DDCAPDEF(TEXT("OVERLAYCANTCLIP"),                 dwCaps, DDCAPS_OVERLAYCANTCLIP),
    DDCAPDEF(TEXT("OVERLAYFOURCC"),                   dwCaps, DDCAPS_OVERLAYFOURCC),
    DDCAPDEF(TEXT("OVERLAYSTRETCH"),                  dwCaps, DDCAPS_OVERLAYSTRETCH),
    DDCAPDEF(TEXT("ZOVERLAYS"),                       dwCaps, DDCAPS_ZOVERLAYS),
    DDCAPDEF(TEXT("AUTOFLIPOVERLAY"),                 dwCaps2,DDCAPS2_AUTOFLIPOVERLAY),
    DDCAPDEF(TEXT("CANBOBINTERLEAVED"),               dwCaps2,DDCAPS2_CANBOBINTERLEAVED),
    DDCAPDEF(TEXT("CANBOBNONINTERLEAVED"),            dwCaps2,DDCAPS2_CANBOBNONINTERLEAVED),
    DDCAPDEF(TEXT("COLORCONTROLOVERLAY"),             dwCaps2,DDCAPS2_COLORCONTROLOVERLAY),
    DDCAPDEF(TEXT("DESTOVERLAY"),                     dwCKeyCaps, DDCKEYCAPS_DESTOVERLAY),
    DDCAPDEF(TEXT("DESTOVERLAYCLRSPACE"),             dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYCLRSPACE),
    DDCAPDEF(TEXT("DESTOVERLAYCLRSPACEYUV"),          dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV),
    DDCAPDEF(TEXT("DESTOVERLAYONEACTIVE"),            dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYONEACTIVE),
    DDCAPDEF(TEXT("DESTOVERLAYYUV"),                  dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYYUV),
    DDCAPDEF(TEXT("SRCOVERLAY"),                      dwCKeyCaps, DDCKEYCAPS_SRCOVERLAY),
    DDCAPDEF(TEXT("SRCOVERLAYCLRSPACE"),              dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYCLRSPACE),
    DDCAPDEF(TEXT("SRCOVERLAYCLRSPACEYUV"),           dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV),
    DDCAPDEF(TEXT("SRCOVERLAYONEACTIVE"),             dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYONEACTIVE),
    DDCAPDEF(TEXT("SRCOVERLAYYUV"),                   dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYYUV),
    DDCAPDEF(TEXT("OVERLAYALPHAEDGEBLEND"),           dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND),
    DDCAPDEF(TEXT("OVERLAYALPHAPIXELS"),              dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHAPIXELS),
    DDCAPDEF(TEXT("OVERLAYALPHAPIXELSNEG"),           dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG),
    DDCAPDEF(TEXT("OVERLAYALPHASURFACES"),            dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHASURFACES),
    DDCAPDEF(TEXT("OVERLAYALPHASURFACESNEG"),         dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHASURFACESNEG),
    DDCAPDEF(TEXT("OVERLAYALPHA"),                    dwFXCaps, DDFXCAPS_OVERLAYALPHA),
    DDCAPDEF(TEXT("OVERLAYARITHSTRETCHY"),            dwFXCaps, DDFXCAPS_OVERLAYARITHSTRETCHY),
    DDCAPDEF(TEXT("OVERLAYARITHSTRETCHYN"),           dwFXCaps, DDFXCAPS_OVERLAYARITHSTRETCHYN),
    DDCAPDEF(TEXT("OVERLAYFILTER"),                   dwFXCaps, DDFXCAPS_OVERLAYFILTER),
    DDCAPDEF(TEXT("OVERLAYMIRRORLEFTRIGHT"),          dwFXCaps, DDFXCAPS_OVERLAYMIRRORLEFTRIGHT),
    DDCAPDEF(TEXT("OVERLAYMIRRORUPDOWN"),             dwFXCaps, DDFXCAPS_OVERLAYMIRRORUPDOWN),
    DDCAPDEF(TEXT("OVERLAYSHRINKX"),                  dwFXCaps, DDFXCAPS_OVERLAYSHRINKX),
    DDCAPDEF(TEXT("OVERLAYSHRINKXN"),                 dwFXCaps, DDFXCAPS_OVERLAYSHRINKXN),
    DDCAPDEF(TEXT("OVERLAYSHRINKY"),                  dwFXCaps, DDFXCAPS_OVERLAYSHRINKY),
    DDCAPDEF(TEXT("OVERLAYSHRINKYN"),                 dwFXCaps, DDFXCAPS_OVERLAYSHRINKYN),
    DDCAPDEF(TEXT("OVERLAYSTRETCHX"),                 dwFXCaps, DDFXCAPS_OVERLAYSTRETCHX),
    DDCAPDEF(TEXT("OVERLAYSTRETCHXN"),                dwFXCaps, DDFXCAPS_OVERLAYSTRETCHXN),
    DDCAPDEF(TEXT("OVERLAYSTRETCHY"),                 dwFXCaps, DDFXCAPS_OVERLAYSTRETCHY),
    DDCAPDEF(TEXT("OVERLAYSTRETCHYN"),                dwFXCaps, DDFXCAPS_OVERLAYSTRETCHYN),
    DDCAPDEF(TEXT("OVERLAYTRANSFORM"),                dwFXCaps, DDFXCAPS_OVERLAYTRANSFORM),
    DDHEXDEF(TEXT("AlphaOverlayConstBitDepths"),      dwAlphaOverlayConstBitDepths),
    DDCAPDEF(TEXT("  8"),                             dwAlphaOverlayConstBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                            dwAlphaOverlayConstBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                            dwAlphaOverlayConstBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                            dwAlphaOverlayConstBitDepths, DDBD_32),
    DDHEXDEF(TEXT("AlphaOverlayPixelBitDepths"),      dwAlphaOverlayPixelBitDepths),
    DDCAPDEF(TEXT("  8"),                             dwAlphaOverlayPixelBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                            dwAlphaOverlayPixelBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                            dwAlphaOverlayPixelBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                            dwAlphaOverlayPixelBitDepths, DDBD_32),
    DDHEXDEF(TEXT("AlphaOverlaySurfaceBitDepths"),    dwAlphaOverlaySurfaceBitDepths),
    DDCAPDEF(TEXT("  8"),                             dwAlphaOverlaySurfaceBitDepths, DDBD_8),
    DDCAPDEF(TEXT("  16"),                            dwAlphaOverlaySurfaceBitDepths, DDBD_16),
    DDCAPDEF(TEXT("  24"),                            dwAlphaOverlaySurfaceBitDepths, DDBD_24),
    DDCAPDEF(TEXT("  32"),                            dwAlphaOverlaySurfaceBitDepths, DDBD_32),
    DDVALDEF(TEXT("MaxVisibleOverlays"),              dwMaxVisibleOverlays),
    DDVALDEF(TEXT("CurrVisibleOverlays"),             dwCurrVisibleOverlays),
    DDVALDEF(TEXT("MinOverlayStretch"),               dwMinOverlayStretch),
    DDVALDEF(TEXT("MaxOverlayStretch"),               dwMaxOverlayStretch),
    { TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEF VideoPortCapsDefs[] =
{
    DDCAPDEF(TEXT("DDCAPS2_VIDEOPORT"),               dwCaps2,DDCAPS2_VIDEOPORT),
    DDCAPDEF(TEXT("DDSCAPS_VIDEOPORT"),               ddsCaps.dwCaps, DDSCAPS_VIDEOPORT),
    DDCAPDEF(TEXT("LIVEVIDEO"),                       ddsCaps.dwCaps, DDSCAPS_LIVEVIDEO),
    DDCAPDEF(TEXT("HARDWAREDEINTERLACE"),             dwCaps2,DDSCAPS2_HARDWAREDEINTERLACE),
    DDCAPDEF(TEXT("AUTOFLIPOVERLAY"),                 dwCaps2,DDCAPS2_AUTOFLIPOVERLAY),
    DDVALDEF(TEXT("MinLiveVideoStretch"),             dwMinLiveVideoStretch),
    DDVALDEF(TEXT("MaxLiveVideoStretch"),             dwMaxLiveVideoStretch),
    DDVALDEF(TEXT("MaxVideoPorts"),                   dwMaxVideoPorts),
    DDVALDEF(TEXT("CurrVideoPorts"),                  dwCurrVideoPorts),
    { TEXT(""), 0, 0}
};

/****************************************************************************
 ***************************************************************************/
CAPDEFS DDCapDefs[] =
{
    {TEXT(""),                    NULL,               (LPARAM)0,                  NULL},
    {TEXT("Memory"),              VidMem,             (LPARAM)0,                  VidMemPrint},

    {TEXT("+Caps"),               NULL,               (LPARAM)0,                  NULL},
    {TEXT("General"),             DDAddCaps,          (LPARAM)GenCaps,            DDPrintCaps},
    {TEXT("FX Alpha Caps"),       DDAddCaps,          (LPARAM)FXAlphaCapsDefs,    DDPrintCaps},
    {TEXT("Palette Caps"),        DDAddCaps,          (LPARAM)PalCapsDefs,        DDPrintCaps},
    {TEXT("Overlay Caps"),        DDAddCaps,          (LPARAM)OverlayCapsDefs,    DDPrintCaps},
    {TEXT("Surface Caps"),        DDAddCaps,          (LPARAM)SurfCapsDefs,       DDPrintCaps},
    {TEXT("Stereo Vision Caps"),  DDAddCaps,          (LPARAM)SVisionCapsDefs,    DDPrintCaps},
    {TEXT("Video Port Caps"),     DDAddCaps,          (LPARAM)VideoPortCapsDefs,  DDPrintCaps},

    {TEXT("+BLT Caps"),           NULL,               (LPARAM)0,                  NULL},

    {TEXT("+Video - Video"),      NULL,               (LPARAM)0,                  NULL},
    {TEXT("General"),             DDAddCaps,          (LPARAM)CapsDefs,           DDPrintCaps},
    {TEXT("Color Key"),           DDAddCaps,          (LPARAM)CKeyCapsDefs,       DDPrintCaps},
    {TEXT("FX"),                  DDAddCaps,          (LPARAM)FXCapsDefs,         DDPrintCaps},
    {TEXT("ROPS"),                DDAddCaps,          (LPARAM)ROPCapsDefs,        DDPrintCaps},
    {TEXT("-"),                   NULL,               (LPARAM)0,                  NULL},

    {TEXT("+System - Video"),     NULL,               (LPARAM)0,                  NULL},
    {TEXT("General"),             DDAddCaps,          (LPARAM)SVBCapsDefs,        DDPrintCaps},
    {TEXT("Color Key"),           DDAddCaps,          (LPARAM)SVBCKeyCapsDefs,    DDPrintCaps},
    {TEXT("FX"),                  DDAddCaps,          (LPARAM)SVBFXCapsDefs,      DDPrintCaps},
    {TEXT("ROPS"),                DDAddCaps,          (LPARAM)SVBROPCapsDefs,     DDPrintCaps},
    {TEXT("-"),                   NULL,               (LPARAM)0,                  NULL},

    {TEXT("+Video - System"),     NULL,               (LPARAM)0,                  NULL},
    {TEXT("General"),             DDAddCaps,          (LPARAM)VSBCapsDefs,        DDPrintCaps},
    {TEXT("Color Key"),           DDAddCaps,          (LPARAM)SSBCKeyCapsDefs,    DDPrintCaps},
    {TEXT("FX"),                  DDAddCaps,          (LPARAM)SSBFXCapsDefs,      DDPrintCaps},
    {TEXT("ROPS"),                DDAddCaps,          (LPARAM)VSBROPCapsDefs,     DDPrintCaps},
    {TEXT("-"),                   NULL,               (LPARAM)0,                  NULL},

    {TEXT("+System - System"),    NULL,               (LPARAM)0,                  NULL},
    {TEXT("General"),             DDAddCaps,          (LPARAM)SSBCapsDefs,        DDPrintCaps},
    {TEXT("Color Key"),           DDAddCaps,          (LPARAM)SSBCKeyCapsDefs,    DDPrintCaps},
    {TEXT("FX"),                  DDAddCaps,          (LPARAM)SSBFXCapsDefs,      DDPrintCaps},
    {TEXT("ROPS"),                DDAddCaps,          (LPARAM)SSBROPCapsDefs,     DDPrintCaps},
    {TEXT("-"),                   NULL,               (LPARAM)0,                  NULL},

    {TEXT("+NonLocal - Video"),   NULL,               (LPARAM)0,                  NULL},
    {TEXT("General"),             DDAddCaps,          (LPARAM)NLVBCapsDefs,       DDPrintCaps},
    {TEXT("Color Key"),           DDAddCaps,          (LPARAM)NLVBCKeyCapsDefs,   DDPrintCaps},
    {TEXT("FX"),                  DDAddCaps,          (LPARAM)NLVBFXCapsDefs,     DDPrintCaps},
    {TEXT("ROPS"),                DDAddCaps,          (LPARAM)NLVBROPCapsDefs,    DDPrintCaps},
    {TEXT("-"),                   NULL,               (LPARAM)0,                  NULL},

    {TEXT("-"),                   NULL,               (LPARAM)0,                  NULL},
    {TEXT("-"),                   NULL,               (LPARAM)0,                  NULL},

    {TEXT("Video Modes"),         DDAddVideoModes,    (LPARAM)0,                  DDPrintVideoModes},
    {TEXT("FourCC Formats"),      DDFourCCFormat,     (LPARAM)0,                  DDPrintFourCCFormat},
    {TEXT("Other"),               DDAddCaps,          (LPARAM)OtherInfoDefs,      DDPrintCaps},

    { NULL, 0, 0, NULL}
};

//================================================================
//================================================================
IDirectDraw * DDCreate(GUID *pid)
{
    if (lpDD && pid == ddid)
        return lpDD;

    if (lpDD)
    {
        IDirectDraw_Release(lpDD);
        lpDD = NULL;
    }

    // There is no need to create DirectDraw emulation-only just to get
    // the HEL caps.  In fact, this will fail if there is another DirectDraw
    // app running and using the hardware.
    if( pid == (GUID *)DDCREATE_EMULATIONONLY )
    {
        pid = NULL;
    }
    if (DirectDrawCreate(pid, &lpDD, NULL) == DD_OK)
    {
        IDirectDraw_QueryInterface(lpDD,&IID_IDirectDraw4,&lpDD4);
        ddid = pid;
        return lpDD;
    }

    if(!g_PrintToFile)
        MessageBox(g_hwndMain, TEXT("DirectDrawCreate failed."), g_szAppName, MB_OK);

    return NULL;
}

//================================================================
//================================================================
void DDAddCaps(LPARAM lParam1, LPARAM lParam2)
{
    // lParam1 is the GUID for the driver we should open
    // lParam2 is the CAPDEF table we should use

    if (DDCreate((GUID*)lParam1))
    {
        DDCAPS ddcaps;

        ddcaps.dwSize = sizeof(ddcaps);

        if (lParam1 == DDCREATE_EMULATIONONLY)
            IDirectDraw4_GetCaps(lpDD4, NULL, &ddcaps);
        else
            IDirectDraw4_GetCaps(lpDD4, &ddcaps, NULL);

        AddCapsToLV((CAPDEF *)lParam2, (LPVOID)&ddcaps);
    }
}


//================================================================
//================================================================
BOOL DDPrintCaps(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo)
{
    // lParam1 is the GUID for the driver we should open
    // lParam2 is the CAPDEF table we should use

    if (DDCreate((GUID*)lParam1))
    {
        DDCAPS ddcaps;

        ddcaps.dwSize = sizeof(ddcaps);

        if (lParam1 == DDCREATE_EMULATIONONLY)
            IDirectDraw4_GetCaps(lpDD4, NULL, &ddcaps);
        else
            IDirectDraw4_GetCaps(lpDD4, &ddcaps, NULL);

        // Print DirectDraw caps
        return PrintCapsToDC((CAPDEF *)lParam2, (LPVOID)&ddcaps, lpInfo);
    }

    // DDCreate failed but keep printing
    return TRUE;
}

//================================================================
//================================================================

int BitCount(DWORD dw)
{
    int     i = 0;

    // Scary looking code, but all it does is count the set bits
    while (dw)
    {
        dw &= ~(dw & -dw);
        i++;
    }

    return i;
}

//================================================================
//================================================================
HRESULT CALLBACK EnumTextureCallback(DDPIXELFORMAT *ddpf, LPVOID lParam)
{
    TCHAR   ach[80];
    int     j;

    ach[0] = 0;
    if (ddpf->dwFlags & DDPF_BUMPDUDV)
    {
        // Display bumpmap info (in UDV format)
        if (ddpf->dwFlags & DDPF_BUMPLUMINANCE)
            wsprintf(ach, TEXT("%d bit %d%d%d "),
                ddpf->dwBumpBitCount,
                BitCount(ddpf->dwBumpDuBitMask),
                BitCount(ddpf->dwBumpLuminanceBitMask),
                BitCount(ddpf->dwBumpDvBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d "),
                ddpf->dwBumpBitCount,
                BitCount(ddpf->dwBumpDuBitMask),
                BitCount(ddpf->dwBumpDvBitMask));
    }
    if (ddpf->dwFlags & DDPF_LUMINANCE)
    {
        // Display luminance info (in AL format)
        if (BitCount(ddpf->dwLuminanceAlphaBitMask) == 0)
            wsprintf(ach, TEXT("%d bit %d "),
                ddpf->dwLuminanceBitCount,
                BitCount(ddpf->dwLuminanceBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d "),
                ddpf->dwLuminanceBitCount,
                BitCount(ddpf->dwLuminanceAlphaBitMask),
                BitCount(ddpf->dwLuminanceBitMask));
    }
    if (ddpf->dwFlags & DDPF_RGB)
    {
        // Display RGB info (in ARGB format)
        if (ddpf->dwRBitMask == 0 && ddpf->dwGBitMask == 0 && ddpf->dwBBitMask == 0)
        {
            // If there is a 0-bit FourCC, don't display "0 bit"
            if (!(ddpf->dwRGBBitCount == 0 && ddpf->dwFlags & DDPF_FOURCC))
                wsprintf(ach, TEXT("%d bit "), ddpf->dwRGBBitCount);
        }
        else if (BitCount(ddpf->dwRGBAlphaBitMask) == 0)
            wsprintf(ach, TEXT("%d bit %d%d%d "),
                ddpf->dwRGBBitCount,
                BitCount(ddpf->dwRBitMask),
                BitCount(ddpf->dwGBitMask),
                BitCount(ddpf->dwBBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d%d%d "),
                ddpf->dwRGBBitCount,
                BitCount(ddpf->dwRGBAlphaBitMask),
                BitCount(ddpf->dwRBitMask),
                BitCount(ddpf->dwGBitMask),
                BitCount(ddpf->dwBBitMask));
    }
    if (ddpf->dwFlags & DDPF_YUV)
    {
        // Display YUV info (in AYUV format)
        if (ddpf->dwYBitMask == 0 && ddpf->dwUBitMask == 0 && ddpf->dwVBitMask == 0)
        {
            // If there is a 0-bit FourCC, don't display "0 bit"
            if (!(ddpf->dwYUVBitCount == 0 && ddpf->dwFlags & DDPF_FOURCC))
                wsprintf(ach, TEXT("%d bit "), ddpf->dwYUVBitCount);
        }
        else if (BitCount(ddpf->dwYUVAlphaBitMask) == 0)
            wsprintf(ach, TEXT("%d bit %d%d%d "),
                ddpf->dwYUVBitCount,
                BitCount(ddpf->dwYBitMask),
                BitCount(ddpf->dwUBitMask),
                BitCount(ddpf->dwVBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d%d%d "),
                ddpf->dwYUVBitCount,
                BitCount(ddpf->dwYUVAlphaBitMask),
                BitCount(ddpf->dwYBitMask),
                BitCount(ddpf->dwUBitMask),
                BitCount(ddpf->dwVBitMask));
    }

    // Display the Four Character Code
    if (ddpf->dwFlags & DDPF_FOURCC) {
        j = lstrlen(ach);
        *(DWORD *)(&ach[j]) = ddpf->dwFourCC;
        ach[j+4] = '\0';
    }

    LVAddText(g_hwndLV, 0, ach);

    ach[0] = 0;

    if (ddpf->dwFlags & DDPF_RGB)
        lstrcat(ach,TEXT("RGB "));
    if (ddpf->dwFlags & DDPF_YUV)
        lstrcat(ach,TEXT("YUV "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXEDTO8)
        lstrcat(ach,TEXT("PALTO8 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED8)
        lstrcat(ach,TEXT("PAL8 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED4)
        lstrcat(ach,TEXT("PAL4 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED2)
        lstrcat(ach,TEXT("PAL2 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED1)
        lstrcat(ach,TEXT("PAL1 "));
    if (ddpf->dwFlags & DDPF_LUMINANCE)
        lstrcat(ach,TEXT("LUMINANCE "));
    if (ddpf->dwFlags & DDPF_ALPHA)
        lstrcat(ach,TEXT("ALPHA "));
    if (ddpf->dwFlags & DDPF_ALPHAPIXELS)
        lstrcat(ach,TEXT("ALPHAPIXELS "));
    if (ddpf->dwFlags & DDPF_ALPHAPREMULT)
        lstrcat(ach,TEXT("ALPHAPREMULT "));

    if (ddpf->dwFlags & DDPF_COMPRESSED)
        lstrcat(ach,TEXT("COMPRESSED "));
    if (ddpf->dwFlags & DDPF_FOURCC)
        lstrcat(ach,TEXT("FOURCC "));
    if (ddpf->dwFlags & DDPF_RGBTOYUV)
        lstrcat(ach,TEXT("RGBTOYUV "));
    if (ddpf->dwFlags & DDPF_ZBUFFER)
        lstrcat(ach,TEXT("ZBUFFER "));
    if (ddpf->dwFlags & DDPF_ZPIXELS)
        lstrcat(ach,TEXT("ZPIXELS "));
    if (ddpf->dwFlags & DDPF_STENCILBUFFER)
        lstrcat(ach,TEXT("STENCILBUFFER "));
    if (ddpf->dwFlags & DDPF_BUMPLUMINANCE)
        lstrcat(ach,TEXT("BUMPLUMINANCE "));
    if (ddpf->dwFlags & DDPF_BUMPDUDV)
        lstrcat(ach,TEXT("BUMPDUDV "));

    LVAddText(g_hwndLV, 1, ach);

    return DDENUMRET_OK;
}


//================================================================
HRESULT CALLBACK EnumPrintTextureCallback(DDPIXELFORMAT *ddpf, LPVOID lpContext)
{
    int         xName, xVal, yLine, j;
    PRINTCBINFO * lpInfo = (PRINTCBINFO *) lpContext;
    DWORD       cchLen;
    TCHAR       ach[80];

    ach[0] = 0;
    if (ddpf->dwFlags & DDPF_BUMPDUDV)
    {
        // Display bumpmap info (in UDV format)
        if (ddpf->dwFlags & DDPF_BUMPLUMINANCE)
            wsprintf(ach, TEXT("%d bit %d%d%d "),
                ddpf->dwBumpBitCount,
                BitCount(ddpf->dwBumpDuBitMask),
                BitCount(ddpf->dwBumpLuminanceBitMask),
                BitCount(ddpf->dwBumpDvBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d "),
                ddpf->dwBumpBitCount,
                BitCount(ddpf->dwBumpDuBitMask),
                BitCount(ddpf->dwBumpDvBitMask));
    }
    if (ddpf->dwFlags & DDPF_LUMINANCE)
    {
        // Display luminance info (in AL format)
        if (BitCount(ddpf->dwLuminanceAlphaBitMask) == 0)
            wsprintf(ach, TEXT("%d bit %d "),
                ddpf->dwLuminanceBitCount,
                BitCount(ddpf->dwLuminanceBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d "),
                ddpf->dwLuminanceBitCount,
                BitCount(ddpf->dwLuminanceAlphaBitMask),
                BitCount(ddpf->dwLuminanceBitMask));
    }
    if (ddpf->dwFlags & DDPF_RGB)
    {
        // Display RGB info (in ARGB format)
        if (ddpf->dwRBitMask == 0 && ddpf->dwGBitMask == 0 && ddpf->dwBBitMask == 0)
        {
            // If there is a 0-bit FourCC, don't display "0 bit"
            if (!(ddpf->dwRGBBitCount == 0 && ddpf->dwFlags & DDPF_FOURCC))
                wsprintf(ach, TEXT("%d bit "), ddpf->dwRGBBitCount);
        }
        else if (BitCount(ddpf->dwRGBAlphaBitMask) == 0)
            wsprintf(ach, TEXT("%d bit %d%d%d "),
                ddpf->dwRGBBitCount,
                BitCount(ddpf->dwRBitMask),
                BitCount(ddpf->dwGBitMask),
                BitCount(ddpf->dwBBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d%d%d "),
                ddpf->dwRGBBitCount,
                BitCount(ddpf->dwRGBAlphaBitMask),
                BitCount(ddpf->dwRBitMask),
                BitCount(ddpf->dwGBitMask),
                BitCount(ddpf->dwBBitMask));
    }
    if (ddpf->dwFlags & DDPF_YUV)
    {
        // Display YUV info (in AYUV format)
        if (ddpf->dwYBitMask == 0 && ddpf->dwUBitMask == 0 && ddpf->dwVBitMask == 0)
        {
            // If there is a 0-bit FourCC, don't display "0 bit"
            if (!(ddpf->dwYUVBitCount == 0 && ddpf->dwFlags & DDPF_FOURCC))
                wsprintf(ach, TEXT("%d bit "), ddpf->dwYUVBitCount);
        }
        else if (BitCount(ddpf->dwYUVAlphaBitMask) == 0)
            wsprintf(ach, TEXT("%d bit %d%d%d "),
                ddpf->dwYUVBitCount,
                BitCount(ddpf->dwYBitMask),
                BitCount(ddpf->dwUBitMask),
                BitCount(ddpf->dwVBitMask));
        else
            wsprintf(ach, TEXT("%d bit %d%d%d%d "),
                ddpf->dwYUVBitCount,
                BitCount(ddpf->dwYUVAlphaBitMask),
                BitCount(ddpf->dwYBitMask),
                BitCount(ddpf->dwUBitMask),
                BitCount(ddpf->dwVBitMask));
    }

    // Display the Four Character Code
    if (ddpf->dwFlags & DDPF_FOURCC) {
        j = lstrlen(ach);
        *(DWORD *)(&ach[j]) = ddpf->dwFourCC;
        ach[j+4] = '\0';
    }

    xName   = (lpInfo->dwCurrIndent * DEF_TAB_SIZE * lpInfo->dwCharWidth);
    xVal    = xName + (32 * lpInfo->dwCharWidth);
    yLine   = (lpInfo->dwCurrLine * lpInfo->dwLineHeight);
    cchLen = lstrlen (ach);
    PrintLine (xName, yLine, ach, cchLen, lpInfo);

    ach[0] = 0;

    if (ddpf->dwFlags & DDPF_RGB)
        lstrcat(ach,TEXT("RGB "));
    if (ddpf->dwFlags & DDPF_YUV)
        lstrcat(ach,TEXT("YUV "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXEDTO8)
        lstrcat(ach,TEXT("PALTO8 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED8)
        lstrcat(ach,TEXT("PAL8 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED4)
        lstrcat(ach,TEXT("PAL4 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED2)
        lstrcat(ach,TEXT("PAL2 "));
    if (ddpf->dwFlags & DDPF_PALETTEINDEXED1)
        lstrcat(ach,TEXT("PAL1 "));
    if (ddpf->dwFlags & DDPF_LUMINANCE)
        lstrcat(ach,TEXT("LUMINANCE "));
    if (ddpf->dwFlags & DDPF_ALPHA)
        lstrcat(ach,TEXT("ALPHA "));
    if (ddpf->dwFlags & DDPF_ALPHAPIXELS)
        lstrcat(ach,TEXT("ALPHAPIXELS "));
    if (ddpf->dwFlags & DDPF_ALPHAPREMULT)
        lstrcat(ach,TEXT("ALPHAPREMULT "));

    if (ddpf->dwFlags & DDPF_COMPRESSED)
        lstrcat(ach,TEXT("COMPRESSED "));
    if (ddpf->dwFlags & DDPF_FOURCC)
        lstrcat(ach,TEXT("FOURCC "));
    if (ddpf->dwFlags & DDPF_RGBTOYUV)
        lstrcat(ach,TEXT("RGBTOYUV "));
    if (ddpf->dwFlags & DDPF_ZBUFFER)
        lstrcat(ach,TEXT("ZBUFFER "));
    if (ddpf->dwFlags & DDPF_ZPIXELS)
        lstrcat(ach,TEXT("ZPIXELS "));
    if (ddpf->dwFlags & DDPF_STENCILBUFFER)
        lstrcat(ach,TEXT("STENCILBUFFER "));
    if (ddpf->dwFlags & DDPF_BUMPLUMINANCE)
        lstrcat(ach,TEXT("BUMPLUMINANCE "));
    if (ddpf->dwFlags & DDPF_BUMPDUDV)
        lstrcat(ach,TEXT("BUMPDUDV "));

    cchLen = lstrlen (ach);
    PrintLine (xVal, yLine, ach, cchLen, lpInfo);
    PrintNextLine (lpInfo);

    return DDENUMRET_OK;
}

//================================================================
//================================================================
typedef struct LLMode
{
    DWORD x,y,bpp;
    BOOL IsModeX;
    struct LLMode *Next;
}LinkMode;

static LinkMode *pModesHead = NULL;


void DDFourCCFormat(LPARAM lParam1, LPARAM lParam2)
{
    HRESULT ddrval;
    int iNumCodes,iCount;
    DWORD *FourCC;
    TCHAR szText[5]={0,0,0,0,0};
    BYTE *pByte;

    if(lpDD != NULL)
    {
        ddrval = IDirectDraw_GetFourCCCodes(lpDD,&iNumCodes, NULL);
        LVAddColumn(g_hwndLV, 0, TEXT("Codes"), 24);
        LVAddColumn(g_hwndLV, 1, TEXT(""), 24);
        if( ddrval == DD_OK)
        {
            FourCC = (DWORD *)LocalAlloc(LPTR,(sizeof(DWORD)*iNumCodes));
            if(FourCC)
            {
                ddrval = IDirectDraw_GetFourCCCodes(lpDD, &iNumCodes, FourCC);

                // Assume all FourCC values are ascii strings
                for(iCount = 0; iCount < iNumCodes; iCount++)
                {
                    // Convert 4 one-byte characters into 4 UNICODE characters
                    pByte = (BYTE *) FourCC;
                    wsprintf(szText, TEXT("%c%c%c%c"),
                             pByte[0],pByte[1],pByte[2],pByte[3]);

                    LVAddText(g_hwndLV, 0, TEXT("%s"), szText);
                }
            }
        }
    }
}


//================================================================
//================================================================
BOOL DDPrintFourCCFormat(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo)
{
    HRESULT ddrval;
    int iNumOfCodes,iCount;
    DWORD *FourCC;
    TCHAR szText[5]={0,0,0,0,0};
    int  xCode, yLine;
    BYTE *pByte;

    if(lpDD != NULL)
    {
        ddrval = IDirectDraw_GetFourCCCodes(lpDD,&iNumOfCodes, NULL);
        if( ddrval == DD_OK)
        {
            FourCC = (DWORD *)LocalAlloc(LPTR,(sizeof(DWORD)*iNumOfCodes));
            if(FourCC)
            {
                xCode  = (lpInfo->dwCurrIndent * DEF_TAB_SIZE * lpInfo->dwCharWidth);
                ddrval = IDirectDraw_GetFourCCCodes(lpDD,&iNumOfCodes, FourCC);
                if (ddrval == DD_OK)
                {
                    // Assume all FourCC values are ascii strings
                    for(iCount = 0;iCount < iNumOfCodes; iCount++)
                    {
                        yLine  = (lpInfo->dwCurrLine * lpInfo->dwLineHeight);

                        // Convert 4 one-byte characters into 4 UNICODE characters
                        pByte = (BYTE *) FourCC;
                        wsprintf(szText, TEXT("%c%c%c%c"),
                                 pByte[0],pByte[1],pByte[2],pByte[3]);

                        // Print Code
                        if (! PrintLine (xCode, yLine, szText, 4, lpInfo))
                            return FALSE;

                        if (! PrintNextLine (lpInfo))
                            return FALSE;
                    }
                }
            }
        }
    }

    // Success
    return TRUE;
}


//================================================================
//================================================================

//================================================================
// DisplayEnumModes
//================================================================
HRESULT CALLBACK EnumDisplayModesCallback(LPDDSURFACEDESC pddsd, LPVOID Context)
{
    if(pddsd->ddsCaps.dwCaps & DDSCAPS_STANDARDVGAMODE)
    {
        LVAddText(g_hwndLV, 0, TEXT("%dx%dx%d (StandardVGA)"), pddsd->dwWidth, pddsd->dwHeight, pddsd->ddpfPixelFormat.dwRGBBitCount);
    }else if(pddsd->ddsCaps.dwCaps & DDSCAPS_MODEX)
    {
        LVAddText(g_hwndLV, 0, TEXT("%dx%dx%d (ModeX)"), pddsd->dwWidth, pddsd->dwHeight, pddsd->ddpfPixelFormat.dwRGBBitCount);
    }else
    {
        LVAddText(g_hwndLV, 0, TEXT("%dx%dx%d "), pddsd->dwWidth, pddsd->dwHeight, pddsd->ddpfPixelFormat.dwRGBBitCount);
    }
    return DDENUMRET_OK;
}


HRESULT CALLBACK EnumDisplayModesCallbackPrint(LPDDSURFACEDESC pddsd, LPVOID Context)
{
    TCHAR szBuff[80];
    DWORD cchLen;
    PRINTCBINFO * lpInfo = Context;
    int xMode, yLine;

    if (! lpInfo)
        return FALSE;

    xMode = (lpInfo->dwCurrIndent * DEF_TAB_SIZE * lpInfo->dwCharWidth);
    yLine = (lpInfo->dwCurrLine * lpInfo->dwLineHeight);

    if(pddsd->ddsCaps.dwCaps & DDSCAPS_STANDARDVGAMODE)
    {
        wsprintf(szBuff, TEXT("%dx%dx%d (StandardVGA)"), pddsd->dwWidth, pddsd->dwHeight, pddsd->ddpfPixelFormat.dwRGBBitCount);
    }else if(pddsd->ddsCaps.dwCaps & DDSCAPS_MODEX)
    {
        wsprintf(szBuff, TEXT("%dx%dx%d (ModeX)"), pddsd->dwWidth, pddsd->dwHeight, pddsd->ddpfPixelFormat.dwRGBBitCount);
    }else
    {
        wsprintf(szBuff, TEXT("%dx%dx%d "), pddsd->dwWidth, pddsd->dwHeight, pddsd->ddpfPixelFormat.dwRGBBitCount);
    }
    // Print Mode Info
    cchLen = _tcslen (szBuff);
    if (! PrintLine (xMode, yLine, szBuff, cchLen, lpInfo))
        return DDENUMRET_CANCEL;
    // Advance to next line
    if (! PrintNextLine (lpInfo))
        return DDENUMRET_CANCEL;

    return DDENUMRET_OK;
}


//================================================================
// Should we hourglass the cursor? this takes a while
//================================================================
void DDAddVideoModes(LPARAM lParam1, LPARAM lParam2)
{
    DWORD mode;
    DDSURFACEDESC ddsd;

    LVAddColumn(g_hwndLV, 0, TEXT("Mode"), 24);
    LVAddColumn(g_hwndLV, 1, TEXT(""), 24);

    // lParam1 is the GUID for the driver we should open
    // lParam2 is not used

    if (DDCreate((GUID*)lParam1))
    {
        // Get the current mode mode for this driver
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        IDirectDraw_GetDisplayMode(lpDD, &ddsd);

        mode = MAKEMODE(ddsd.dwWidth, ddsd.dwHeight, ddsd.ddpfPixelFormat.dwRGBBitCount);

        // Find all mode for this driver and add them to the listview
        // Erase previous list
        {
            LinkMode *tmp, *tmp1;

            tmp = pModesHead;
            while(tmp != NULL)
            {
                tmp1 = tmp;
                tmp = tmp->Next;
                LocalFree(tmp1);  //Note We will exit with a list in place
            }
            pModesHead = NULL;
        }

        //Get Modes
        IDirectDraw_SetCooperativeLevel(lpDD, g_hwndMain, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX | DDSCL_NOWINDOWCHANGES);
        IDirectDraw_EnumDisplayModes(lpDD, DDEDM_STANDARDVGAMODES, NULL, (LPVOID)mode, EnumDisplayModesCallback);
        IDirectDraw_SetCooperativeLevel(lpDD, g_hwndMain, DDSCL_NORMAL);
    }
}


//================================================================
// Should we hourglass the cursor? this takes a while
//================================================================
BOOL DDPrintVideoModes(LPARAM lParam1, LPARAM lParam2, PRINTCBINFO * lpInfo)
{
    BOOL  fResult = TRUE;   // Even if we can't enum modes, let's keep on printing other stuff
    DWORD mode;
    DDSURFACEDESC ddsd;

    LVAddColumn(g_hwndLV, 0, TEXT("Mode"), 24);
    LVAddColumn(g_hwndLV, 1, TEXT(""), 24);

    // lParam1 is the GUID for the driver we should open
    // lParam2 is not used

    if (DDCreate((GUID*)lParam1))
    {
        // Get the current mode mode for this driver
        ddsd.dwSize = sizeof(DDSURFACEDESC);
        IDirectDraw_GetDisplayMode(lpDD, &ddsd);

        mode = MAKEMODE(ddsd.dwWidth, ddsd.dwHeight, ddsd.ddpfPixelFormat.dwRGBBitCount);

        // Find all mode for this driver and add them to the listview
        //Erase previous list
        {
            LinkMode *tmp, *tmp1;

            tmp = pModesHead;
            while(tmp != NULL)
            {
                tmp1 = tmp;
                tmp = tmp->Next;
                LocalFree(tmp1);  //Note We will exit with a list in place
            }
            pModesHead = NULL;
        }

        //Get Mode with ModeX
        IDirectDraw_SetCooperativeLevel(lpDD, g_hwndMain, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX | DDSCL_NOWINDOWCHANGES);
        IDirectDraw_EnumDisplayModes(lpDD, DDEDM_STANDARDVGAMODES, NULL, (LPVOID)lpInfo, EnumDisplayModesCallbackPrint);
        IDirectDraw_SetCooperativeLevel(lpDD, g_hwndMain, DDSCL_NORMAL);
    }

    // Success
    return fResult;
}


//================================================================
//================================================================
HRESULT CALLBACK DDEnumCallBack(GUID *pid, LPTSTR lpDriverDesc, LPTSTR lpDriverName, LPVOID lpContext)
{
    HTREEITEM hParent = (HTREEITEM)lpContext;
    TCHAR szText[256];

    if (HIWORD(pid) != 0)
    {
        GUID temp = *pid;
        pid = (GUID *)LocalAlloc(LPTR, sizeof(GUID));
        *pid = temp;
    }

    // Add subnode to treeview
    if (lpDriverName && *lpDriverName)
        wsprintf(szText, TEXT("%s (%s)"), lpDriverDesc, lpDriverName);
    else
        lstrcpy(szText, lpDriverDesc);

    DDCapDefs[0].szName = szText;
    AddCapsToTV(hParent, DDCapDefs, (LPARAM)pid);

    return(DDENUMRET_OK);
}


HRESULT CALLBACK DDEnumCallBack2(GUID *pid, LPTSTR lpDriverDesc, LPTSTR lpDriverName, LPVOID lpContext,HMONITOR hMonitor)
{
    return DDEnumCallBack(pid, lpDriverDesc, lpDriverName, lpContext);
}


VOID VidMem(long lParam1, long foo2)
{
    HRESULT     ddrval;
    DWORD       dwTotalMem, dwFreeMem;
    DDSCAPS2    ddsCaps2;
    TCHAR       szBuff[64];

    LVAddColumn(g_hwndLV, 0, TEXT("Type"), 24);
    LVAddColumn(g_hwndLV, 1, TEXT("Total"), 10);
    LVAddColumn(g_hwndLV, 2, TEXT("Free"), 10);

    if (DDCreate((GUID*)lParam1))
    {
        memset(&ddsCaps2, 0, sizeof(ddsCaps2));
        ddsCaps2.dwCaps = DDSCAPS_VIDEOMEMORY;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        LVAddText(g_hwndLV, 0, TEXT("Video"));
        Int2Str(szBuff,dwTotalMem);
        LVAddText(g_hwndLV, 1, TEXT("%s"), szBuff);
        Int2Str(szBuff,dwFreeMem);
        LVAddText(g_hwndLV, 2, TEXT("%s"), szBuff);

        ddsCaps2.dwCaps = DDSCAPS_LOCALVIDMEM;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        LVAddText(g_hwndLV, 0, TEXT("Video (local)"));
        Int2Str(szBuff,dwTotalMem);
        LVAddText(g_hwndLV, 1, TEXT("%s"), szBuff);
        Int2Str(szBuff,dwFreeMem);
        LVAddText(g_hwndLV, 2, TEXT("%s"), szBuff);

        ddsCaps2.dwCaps = DDSCAPS_NONLOCALVIDMEM;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        LVAddText(g_hwndLV, 0, TEXT("Video (non-local)"));
        Int2Str(szBuff,dwTotalMem);
        LVAddText(g_hwndLV, 1, TEXT("%s"), szBuff);
        Int2Str(szBuff,dwFreeMem);
        LVAddText(g_hwndLV, 2, TEXT("%s"), szBuff);

        ddsCaps2.dwCaps = DDSCAPS_TEXTURE;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        LVAddText(g_hwndLV, 0, TEXT("Texture"));
        Int2Str(szBuff,dwTotalMem);
        LVAddText(g_hwndLV, 1, TEXT("%s"), szBuff);
        Int2Str(szBuff,dwFreeMem);
        LVAddText(g_hwndLV, 2, TEXT("%s"), szBuff);
    }
}


BOOL VidMemPrint(long lParam1, long lParam2, PRINTCBINFO *lpInfo)
{
    HRESULT     ddrval;
    DWORD       dwTotalMem, dwFreeMem;
    DDSCAPS2    ddsCaps2;

    if (DDCreate((GUID*)lParam1))
    {
        ZeroMemory(&ddsCaps2, sizeof(ddsCaps2));
        ddsCaps2.dwCaps = DDSCAPS_VIDEOMEMORY;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        PrintValueLine(TEXT("TotalMem"),dwTotalMem,lpInfo);
        PrintValueLine(TEXT("FreeMem"),dwFreeMem,lpInfo);

        ddsCaps2.dwCaps = DDSCAPS_LOCALVIDMEM;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        PrintValueLine(TEXT("TotalVideoMem"),dwTotalMem,lpInfo);
        PrintValueLine(TEXT("FreeVideoMem"),dwFreeMem,lpInfo);

        ddsCaps2.dwCaps = DDSCAPS_NONLOCALVIDMEM;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        PrintValueLine(TEXT("TotalAGPMem"),dwTotalMem,lpInfo);
        PrintValueLine(TEXT("FreeAGPMem"),dwFreeMem,lpInfo);

        ddsCaps2.dwCaps = DDSCAPS_TEXTURE;
        ddrval = IDirectDraw4_GetAvailableVidMem(lpDD4, &ddsCaps2, &dwTotalMem, &dwFreeMem );
        if(ddrval != DD_OK)
        {
            dwTotalMem = 0;
            dwFreeMem = 0;
        }
        PrintValueLine(TEXT("TotalTextureMem"),dwTotalMem,lpInfo);
        PrintValueLine(TEXT("FreeTextureMem"),dwFreeMem,lpInfo);
    }
    return TRUE;
}


