#include <quartz.h>

#ifndef FILTER_LIB

#include <ceconfig.h>

// If FILTER_LIB is defined then the component filters
// are being built as filter libs to,link into this dll.
// hence we need all these gubbins!

CFactoryTemplate g_Templates[] = {
    // core filters see quartz.h for these macros
    INITSYSTEMOBJECTS,
    // individual filters
#ifdef QUARTZ_ASYNCRDR
    INITFILTER_FILESOURCEASYNC,
#endif
#ifdef QUARTZ_WAVEMSR
    INITFILTER_WAVEPARSER,
#endif
#ifdef QUARTZ_ACMWRAP
    INITFILTER_ACMWRAPPER,
#endif
#ifdef QUARTZ_WAVEOUT
    INITFILTER_AUDIORENDERER,
#endif
#ifdef QUARTZ_DSNDOUT
    INITFILTER_DSOUNDRENDERER,
#endif
#ifdef QUARTZ_AVIMSR
    INITFILTER_AVISPLITTER,
#endif
#ifdef QUARTZ_AVIDEC
    INITFILTER_AVIDECOMPRESSOR,
#endif
#ifdef QUARTZ_MPGSPLIT
    INITFILTER_MPEG1SPLITTER,
#endif
#ifdef QUARTZ_MPGADEC
    INITFILTER_MPEG1AUDIOCODEC,
#endif
#ifdef QUARTZ_MPGVDEC
    INITFILTER_MPEG1VIDEOCODEC,
#endif
#ifdef QUARTZ_IMAGE
    INITFILTER_VIDEORENDERER,
#endif
#ifdef QUARTZ_COLOUR
    INITFILTER_COLOURSPACECONVERTER,
#endif
#ifdef QUARTZ_QTRDR
    INITFILTER_QUICKTIMEPARSER,
#endif
#ifdef QUARTZ_MIDIRDR
    INITFILTER_MIDIPARSER,
#endif
#ifdef QUARTZ_MIDIOUT
    INITFILTER_MIDIRENDERER,
#endif
#ifdef QUARTZ_URLRDR
    INITFILTER_FILESOURCEURL,
#endif
#if defined(QUARTZ_OVMIXER) || defined(QUARTZ_OVMIXER2)
    INITFILTER_OVERLAYMIXER,
    INITFILTER_VIDEOPORT,
#endif
#ifdef QUARTZ_DVSP
    INITFILTER_DVSPLITTER,
#endif
#ifdef QUARTZ_DVDEC
    INITFILTER_DVDECODER,
#endif
#ifdef QUARTZ_MIDIDM
	INITFILTER_MIDIDM,
#endif
#ifdef QUARTZ_SCMDOUT
	INITFILTER_SCMDRENDERER,
#endif
};


int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

#endif
