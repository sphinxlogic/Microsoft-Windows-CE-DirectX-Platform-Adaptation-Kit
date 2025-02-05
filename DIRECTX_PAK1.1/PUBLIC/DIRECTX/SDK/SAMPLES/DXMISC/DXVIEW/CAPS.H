/****************************************************************************

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

    Copyright (C) 1995-2000 Microsoft Corporation. All Rights Reserved.

    PROGRAM: caps.h

    PURPOSE: DirectX Device Viewer generic capabilities defines

****************************************************************************/


#define DDCAPDEF(name,val,flag) {name, FIELD_OFFSET(DDCAPS,val), flag}
#define DDVALDEF(name,val)      {name, FIELD_OFFSET(DDCAPS,val), 0}
#define DDHEXDEF(name,val)      {name, FIELD_OFFSET(DDCAPS,val), 0xFFFFFFFF}
#define ROPDEF(name,dwRops,rop) DDCAPDEF(name,dwRops[((rop>>16)&0xFF)/32],(1<<((rop>>16)&0xFF)%32))

#define SURFCAPDEF(name,val,flag) {name, FIELD_OFFSET(DDSURFACEDESC,val), flag}
#define SURFVALDEF(name,val)      {name, FIELD_OFFSET(DDSURFACEDESC,val), 0}


/****************************************************************************
 ***************************************************************************/

#define GEN_CAPS(dwCaps)                                                     \
\
    DDCAPDEF(TEXT("3D"),                        dwCaps, DDCAPS_3D),                   \
    DDCAPDEF(TEXT("ALIGNBOUNDARYDEST"),         dwCaps, DDCAPS_ALIGNBOUNDARYDEST),    \
    DDCAPDEF(TEXT("ALIGNSIZEDEST"),             dwCaps, DDCAPS_ALIGNSIZEDEST),        \
    DDCAPDEF(TEXT("ALIGNBOUNDARYSRC"),          dwCaps, DDCAPS_ALIGNBOUNDARYSRC),     \
    DDCAPDEF(TEXT("ALIGNSIZESRC"),              dwCaps, DDCAPS_ALIGNSIZESRC),         \
    DDCAPDEF(TEXT("ALIGNSTRIDE"),               dwCaps, DDCAPS_ALIGNSTRIDE),          \
    DDCAPDEF(TEXT("GDI"),                       dwCaps, DDCAPS_GDI),                  \
    DDCAPDEF(TEXT("OVERLAY"),                   dwCaps, DDCAPS_OVERLAY),              \
    DDCAPDEF(TEXT("OVERLAYCANTCLIP"),           dwCaps, DDCAPS_OVERLAYCANTCLIP),      \
    DDCAPDEF(TEXT("OVERLAYFOURCC"),             dwCaps, DDCAPS_OVERLAYFOURCC),        \
    DDCAPDEF(TEXT("OVERLAYSTRETCH"),            dwCaps, DDCAPS_OVERLAYSTRETCH),       \
    DDCAPDEF(TEXT("PALETTE"),                   dwCaps, DDCAPS_PALETTE),              \
    DDCAPDEF(TEXT("PALETTEVSYNC"),              dwCaps, DDCAPS_PALETTEVSYNC),         \
    DDCAPDEF(TEXT("READSCANLINE"),              dwCaps, DDCAPS_READSCANLINE),         \
    DDCAPDEF(TEXT("STEREOVIEW"),                dwCaps, DDCAPS_STEREOVIEW),           \
    DDCAPDEF(TEXT("VBI"),                       dwCaps, DDCAPS_VBI),                  \
    DDCAPDEF(TEXT("ZOVERLAYS"),                 dwCaps, DDCAPS_ZOVERLAYS),            \
    DDCAPDEF(TEXT("NOHARDWARE"),                dwCaps, DDCAPS_NOHARDWARE),           \
    DDCAPDEF(TEXT("BANKSWITCHED"),              dwCaps, DDCAPS_BANKSWITCHED),


#define GEN_CAPS2(dwCaps2)                                                      \
    DDCAPDEF(TEXT("CERTIFIED"),                 dwCaps2,DDCAPS2_CERTIFIED),           \
    DDCAPDEF(TEXT("NO2DDURING3DSCENE"),         dwCaps2,DDCAPS2_NO2DDURING3DSCENE),   \
    DDCAPDEF(TEXT("VIDEOPORT"),                 dwCaps2,DDCAPS2_VIDEOPORT),           \
    DDCAPDEF(TEXT("AUTOFLIPOVERLAY"),           dwCaps2,DDCAPS2_AUTOFLIPOVERLAY),     \
    DDCAPDEF(TEXT("CANBOBINTERLEAVED"),         dwCaps2,DDCAPS2_CANBOBINTERLEAVED),   \
    DDCAPDEF(TEXT("CANBOBNONINTERLEAVED"),      dwCaps2,DDCAPS2_CANBOBNONINTERLEAVED),\
    DDCAPDEF(TEXT("COLORCONTROLOVERLAY"),       dwCaps2,DDCAPS2_COLORCONTROLOVERLAY), \
    DDCAPDEF(TEXT("COLORCONTROLPRIMARY"),       dwCaps2,DDCAPS2_COLORCONTROLPRIMARY), \
    DDCAPDEF(TEXT("NONLOCALVIDMEM"),            dwCaps2,DDCAPS2_NONLOCALVIDMEM),      \
    DDCAPDEF(TEXT("NONLOCALVIDMEMCAPS"),        dwCaps2,DDCAPS2_NONLOCALVIDMEMCAPS),  \
    DDCAPDEF(TEXT("WIDESURFACES"),              dwCaps2,DDCAPS2_WIDESURFACES),        \
    DDCAPDEF(TEXT("NOPAGELOCKREQUIRED"),        dwCaps2,DDCAPS2_NOPAGELOCKREQUIRED),  \
    DDCAPDEF(TEXT("CANRENDERWINDOWED"),         dwCaps2,DDCAPS2_CANRENDERWINDOWED),   \
    DDCAPDEF(TEXT("COPYFOURCC"),                dwCaps2,DDCAPS2_COPYFOURCC),          \
    DDCAPDEF(TEXT("PRIMARYGAMMA"),              dwCaps2,DDCAPS2_PRIMARYGAMMA),        \
    DDCAPDEF(TEXT("CANCALIBRATEGAMMA"),         dwCaps2,DDCAPS2_CANCALIBRATEGAMMA),


#define GEN_BLTCAPS(dwCaps)                                                     \
    DDCAPDEF(TEXT("BLT"),                       dwCaps, DDCAPS_BLT),                  \
    DDCAPDEF(TEXT("BLTCOLORFILL"),              dwCaps, DDCAPS_BLTCOLORFILL),         \
    DDCAPDEF(TEXT("BLTDEPTHFILL"),              dwCaps, DDCAPS_BLTDEPTHFILL),         \
    DDCAPDEF(TEXT("BLTFOURCC"),                 dwCaps, DDCAPS_BLTFOURCC),            \
    DDCAPDEF(TEXT("BLTSTRETCH"),                dwCaps, DDCAPS_BLTSTRETCH),           \
    DDCAPDEF(TEXT("BLTQUEUE"),                  dwCaps, DDCAPS_BLTQUEUE),             \
    DDCAPDEF(TEXT("COLORKEY"),                  dwCaps, DDCAPS_COLORKEY),             \
    DDCAPDEF(TEXT("ALPHA"),                     dwCaps, DDCAPS_ALPHA),                \
    DDCAPDEF(TEXT("CKEYHWASSIST"),              dwCaps, DDCAPS_COLORKEYHWASSIST),     \
    DDCAPDEF(TEXT("CANCLIP"),                   dwCaps, DDCAPS_CANCLIP),              \
    DDCAPDEF(TEXT("CANCLIPSTRETCHED"),          dwCaps, DDCAPS_CANCLIPSTRETCHED),     \
    DDCAPDEF(TEXT("CANBLTSYSMEM"),              dwCaps, DDCAPS_CANBLTSYSMEM),         \
    DDCAPDEF(TEXT("ZBLTS"),                     dwCaps, DDCAPS_ZBLTS),


#define GEN_BLTCAPS2(dwCaps2)                                                   \
    DDCAPDEF(TEXT("CANDROPZ16BIT"),             dwCaps2,DDCAPS2_CANDROPZ16BIT),       \
    DDCAPDEF(TEXT("NOPAGELOCKREQUIRED"),        dwCaps2,DDCAPS2_NOPAGELOCKREQUIRED),  \
    DDCAPDEF(TEXT("CANFLIPODDEVEN"),            dwCaps2,DDCAPS2_CANFLIPODDEVEN),      \
    DDCAPDEF(TEXT("CANBOBHARDWARE"),            dwCaps2,DDCAPS2_CANBOBHARDWARE),      \
    DDCAPDEF(TEXT("WIDESURFACES"),              dwCaps2,DDCAPS2_WIDESURFACES),

/****************************************************************************
 ***************************************************************************/
#define GEN_CKCAPS(dwCKeyCaps)                                                              \
    DDCAPDEF(TEXT("DESTBLT"),                 dwCKeyCaps, DDCKEYCAPS_DESTBLT),                  \
    DDCAPDEF(TEXT("DESTBLTCLRSPACE"),         dwCKeyCaps, DDCKEYCAPS_DESTBLTCLRSPACE),          \
    DDCAPDEF(TEXT("DESTBLTCLRSPACEYUV"),      dwCKeyCaps, DDCKEYCAPS_DESTBLTCLRSPACEYUV),       \
    DDCAPDEF(TEXT("DESTBLTYUV"),              dwCKeyCaps, DDCKEYCAPS_DESTBLTYUV),               \
    DDCAPDEF(TEXT("DESTOVERLAY"),             dwCKeyCaps, DDCKEYCAPS_DESTOVERLAY),              \
    DDCAPDEF(TEXT("DESTOVERLAYCLRSPACE"),     dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYCLRSPACE),      \
    DDCAPDEF(TEXT("DESTOVERLAYCLRSPACEYUV"),  dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYCLRSPACEYUV),   \
    DDCAPDEF(TEXT("DESTOVERLAYONEACTIVE"),    dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYONEACTIVE),     \
    DDCAPDEF(TEXT("DESTOVERLAYYUV"),          dwCKeyCaps, DDCKEYCAPS_DESTOVERLAYYUV),           \
    DDCAPDEF(TEXT("NOCOSTOVERLAY"),           dwCKeyCaps, DDCKEYCAPS_NOCOSTOVERLAY),            \
    DDCAPDEF(TEXT("SRCBLT"),                  dwCKeyCaps, DDCKEYCAPS_SRCBLT),                   \
    DDCAPDEF(TEXT("SRCBLTCLRSPACE"),          dwCKeyCaps, DDCKEYCAPS_SRCBLTCLRSPACE),           \
    DDCAPDEF(TEXT("SRCBLTCLRSPACEYUV"),       dwCKeyCaps, DDCKEYCAPS_SRCBLTCLRSPACEYUV),        \
    DDCAPDEF(TEXT("SRCBLTYUV"),               dwCKeyCaps, DDCKEYCAPS_SRCBLTYUV),                \
    DDCAPDEF(TEXT("SRCOVERLAY"),              dwCKeyCaps, DDCKEYCAPS_SRCOVERLAY),               \
    DDCAPDEF(TEXT("SRCOVERLAYCLRSPACE"),      dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYCLRSPACE),       \
    DDCAPDEF(TEXT("SRCOVERLAYCLRSPACEYUV"),   dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYCLRSPACEYUV),    \
    DDCAPDEF(TEXT("SRCOVERLAYONEACTIVE"),     dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYONEACTIVE),      \
    DDCAPDEF(TEXT("SRCOVERLAYYUV"),           dwCKeyCaps, DDCKEYCAPS_SRCOVERLAYYUV),

#define GEN_FXCAPS(dwFXCaps)                                                            \
    DDCAPDEF(TEXT("BLTALPHA"),                  dwFXCaps, DDFXCAPS_BLTALPHA),                 \
    DDCAPDEF(TEXT("BLTARITHSTRETCHY"),          dwFXCaps, DDFXCAPS_BLTARITHSTRETCHY),         \
    DDCAPDEF(TEXT("BLTARITHSTRETCHYN"),         dwFXCaps, DDFXCAPS_BLTARITHSTRETCHYN),        \
    DDCAPDEF(TEXT("BLTFILTER"),                 dwFXCaps, DDFXCAPS_BLTFILTER),                \
    DDCAPDEF(TEXT("BLTMIRRORLEFTRIGHT"),        dwFXCaps, DDFXCAPS_BLTMIRRORLEFTRIGHT),       \
    DDCAPDEF(TEXT("BLTMIRRORUPDOWN"),           dwFXCaps, DDFXCAPS_BLTMIRRORUPDOWN),          \
    DDCAPDEF(TEXT("BLTROTATION"),               dwFXCaps, DDFXCAPS_BLTROTATION),              \
    DDCAPDEF(TEXT("BLTROTATION90"),             dwFXCaps, DDFXCAPS_BLTROTATION90),            \
    DDCAPDEF(TEXT("BLTSHRINKX"),                dwFXCaps, DDFXCAPS_BLTSHRINKX),               \
    DDCAPDEF(TEXT("BLTSHRINKXN"),               dwFXCaps, DDFXCAPS_BLTSHRINKXN),              \
    DDCAPDEF(TEXT("BLTSHRINKY"),                dwFXCaps, DDFXCAPS_BLTSHRINKY),               \
    DDCAPDEF(TEXT("BLTSHRINKYN"),               dwFXCaps, DDFXCAPS_BLTSHRINKYN),              \
    DDCAPDEF(TEXT("BLTSTRETCHX"),               dwFXCaps, DDFXCAPS_BLTSTRETCHX),              \
    DDCAPDEF(TEXT("BLTSTRETCHXN"),              dwFXCaps, DDFXCAPS_BLTSTRETCHXN),             \
    DDCAPDEF(TEXT("BLTSTRETCHY"),               dwFXCaps, DDFXCAPS_BLTSTRETCHY),              \
    DDCAPDEF(TEXT("BLTSTRETCHYN"),              dwFXCaps, DDFXCAPS_BLTSTRETCHYN),             \
    DDCAPDEF(TEXT("BLTTRANSFORM"),              dwFXCaps, DDFXCAPS_BLTTRANSFORM),             \
    DDCAPDEF(TEXT("OVERLAYALPHA"),              dwFXCaps, DDFXCAPS_OVERLAYALPHA),             \
    DDCAPDEF(TEXT("OVERLAYARITHSTRETCHY"),      dwFXCaps, DDFXCAPS_OVERLAYARITHSTRETCHY),     \
    DDCAPDEF(TEXT("OVERLAYARITHSTRETCHYN"),     dwFXCaps, DDFXCAPS_OVERLAYARITHSTRETCHYN),    \
    DDCAPDEF(TEXT("OVERLAYFILTER"),             dwFXCaps, DDFXCAPS_OVERLAYFILTER),            \
    DDCAPDEF(TEXT("OVERLAYMIRRORLEFTRIGHT"),    dwFXCaps, DDFXCAPS_OVERLAYMIRRORLEFTRIGHT),   \
    DDCAPDEF(TEXT("OVERLAYMIRRORUPDOWN"),       dwFXCaps, DDFXCAPS_OVERLAYMIRRORUPDOWN),      \
    DDCAPDEF(TEXT("OVERLAYSHRINKX"),            dwFXCaps, DDFXCAPS_OVERLAYSHRINKX),           \
    DDCAPDEF(TEXT("OVERLAYSHRINKXN"),           dwFXCaps, DDFXCAPS_OVERLAYSHRINKXN),          \
    DDCAPDEF(TEXT("OVERLAYSHRINKY"),            dwFXCaps, DDFXCAPS_OVERLAYSHRINKY),           \
    DDCAPDEF(TEXT("OVERLAYSHRINKYN"),           dwFXCaps, DDFXCAPS_OVERLAYSHRINKYN),          \
    DDCAPDEF(TEXT("OVERLAYSTRETCHX"),           dwFXCaps, DDFXCAPS_OVERLAYSTRETCHX),          \
    DDCAPDEF(TEXT("OVERLAYSTRETCHXN"),          dwFXCaps, DDFXCAPS_OVERLAYSTRETCHXN),         \
    DDCAPDEF(TEXT("OVERLAYSTRETCHY"),           dwFXCaps, DDFXCAPS_OVERLAYSTRETCHY),          \
    DDCAPDEF(TEXT("OVERLAYSTRETCHYN"),          dwFXCaps, DDFXCAPS_OVERLAYSTRETCHYN),         \
    DDCAPDEF(TEXT("OVERLAYTRANSFORM"),          dwFXCaps, DDFXCAPS_OVERLAYTRANSFORM),

#define GEN_FXALPHACAPS(dwFXAlphaCaps)                                                          \
    DDCAPDEF(TEXT("BLTALPHAEDGEBLEND"),           dwFXAlphaCaps, DDFXALPHACAPS_BLTALPHAEDGEBLEND),    \
    DDCAPDEF(TEXT("BLTALPHAPIXELS"),              dwFXAlphaCaps, DDFXALPHACAPS_BLTALPHAPIXELS),       \
    DDCAPDEF(TEXT("BLTALPHAPIXELSNEG"),           dwFXAlphaCaps, DDFXALPHACAPS_BLTALPHAPIXELSNEG),    \
    DDCAPDEF(TEXT("BLTALPHASURFACES"),            dwFXAlphaCaps, DDFXALPHACAPS_BLTALPHASURFACES),     \
    DDCAPDEF(TEXT("BLTALPHASURFACESNEG"),         dwFXAlphaCaps, DDFXALPHACAPS_BLTALPHASURFACESNEG),  \
    DDCAPDEF(TEXT("OVERLAYALPHAEDGEBLEND"),       dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHAEDGEBLEND),\
    DDCAPDEF(TEXT("OVERLAYALPHAPIXELS"),          dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHAPIXELS),   \
    DDCAPDEF(TEXT("OVERLAYALPHAPIXELSNEG"),       dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHAPIXELSNEG),\
    DDCAPDEF(TEXT("OVERLAYALPHASURFACES"),        dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHASURFACES), \
    DDCAPDEF(TEXT("OVERLAYALPHASURFACESNEG"),     dwFXAlphaCaps, DDFXALPHACAPS_OVERLAYALPHASURFACESNEG),

#define GEN_ROPS(dwRops)                        \
    ROPDEF(TEXT("SRCCOPY"),    dwRops, SRCCOPY),      \
    ROPDEF(TEXT("SRCPAINT"),   dwRops, SRCPAINT),     \
    ROPDEF(TEXT("SRCAND"),     dwRops, SRCAND),       \
    ROPDEF(TEXT("SRCINVERT"),  dwRops, SRCINVERT),    \
    ROPDEF(TEXT("SRCERASE"),   dwRops, SRCERASE),     \
    ROPDEF(TEXT("NOTSRCCOPY"), dwRops, NOTSRCCOPY),   \
    ROPDEF(TEXT("NOTSRCERASE"),dwRops, NOTSRCERASE),  \
    ROPDEF(TEXT("MERGECOPY"),  dwRops, MERGECOPY),    \
    ROPDEF(TEXT("MERGEPAINT"), dwRops, MERGEPAINT),   \
    ROPDEF(TEXT("PATCOPY"),    dwRops, PATCOPY),      \
    ROPDEF(TEXT("PATPAINT"),   dwRops, PATPAINT),     \
    ROPDEF(TEXT("PATINVERT"),  dwRops, PATINVERT),    \
    ROPDEF(TEXT("DSTINVERT"),  dwRops, DSTINVERT),    \
    ROPDEF(TEXT("BLACKNESS"),  dwRops, BLACKNESS),    \
    ROPDEF(TEXT("WHITENESS"),  dwRops, WHITENESS),
