//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;

// quartz.h : IMPORTANT PLEASE READ BELOW

// This file defines all the static create and init function in all of the
// core DirectShow filters. This allows individual projects to build one
// DirectShow dll (quartz.dll) with only the filters that they require.
//
// To avoid having to include numerous private include files, this file
// defines the classes used by each filter BUT with only the static functions
// so that the compiler can generate the right symbols which enables the
// linker to find the correct functions to link to.

#ifndef _QUARTZ_H__
#define _QUARTZ_H__

#ifdef	_MSC_VER
#if _MSC_VER>=1100
#define AM_NOVTABLE __declspec(novtable)
#else
#define AM_NOVTABLE
#endif
#endif	// MSC_VER

#include <windows.h>
#include <windowsx.h>
#ifdef UNDER_CE
#include <winx.h>
#endif // UNDER_CE
#include <strmif.h>
#include <wxdebug.h>
#include <combase.h>
#include <uuids.h>

// macro for objects that only have a Create function
#define DECLAREOBJCREATE(classname, createfunc) \
class classname## \
{ \
public: \
    static CUnknown *classname##::createfunc##(LPUNKNOWN lpunk, HRESULT *phr); \
};

// macro for objects that have a Create and Init function
#define DECLAREOBJCREATEANDINIT(classname, createfunc, initfunc) \
class classname## \
{ \
public: \
    static CUnknown *classname##::createfunc##(LPUNKNOWN lpunk, HRESULT *phr); \
    static void classname##::initfunc##(BOOL bLoading, const CLSID *rclsid); \
};

#define DECLARE2OBJCREATEANDINIT(classname, createfunc1, createfunc2, initfunc) \
class classname## \
{ \
public: \
    static CUnknown *classname##::createfunc1##(LPUNKNOWN lpunk, HRESULT *phr); \
    static CUnknown *classname##::createfunc2##(LPUNKNOWN lpunk, HRESULT *phr); \
    static void classname##::initfunc##(BOOL bLoading, const CLSID *rclsid); \
};

//==========================================================================;
// Class definitions of all core DirectShow filters, a product is free to
// add their own filters if they want to.
//==========================================================================;

//==========================================================================;
// Required core components, you MUST have these
//==========================================================================;
// System Clock
DECLAREOBJCREATE(CSystemClock, CreateInstance)
// Seeeking
DECLAREOBJCREATE(CSeekingPassThru, CreateInstance)
// Memory Allocator
DECLAREOBJCREATE(CMemAllocator, CreateInstance)
#ifndef NO_FILTERMAPPER2
// Filter Mapper 2
DECLAREOBJCREATEANDINIT(CFilterMapper2, CreateInstance, MapperInit)
#else
// Filter Mapper
DECLAREOBJCREATEANDINIT(CFilterMapper, CreateInstance, MapperInit)
#endif
// Filter Graph
//DECLAREOBJCREATE(CFilterGraph, CreateThreadedInstance)
// Filter Graph no thread and Filter Graph
DECLARE2OBJCREATEANDINIT(CFilterGraph, CreateThreadedInstance, CreateInstance, InitClass)

//==========================================================================;
// Optional Media Components
//==========================================================================;
// File Reader (Async)
DECLAREOBJCREATE(CAsyncReader, CreateInstance)
// Wave Audio Parser (WAVE, AIFF, AU, SND)
DECLAREOBJCREATE(CWAVEParse, CreateInstance)
// Audio Compression Manager Wrapper
DECLAREOBJCREATE(CACMWrapper, CreateInstance)
// AVI Parser
DECLAREOBJCREATE(CAviMSRFilter, CreateInstance)
// AVI Decompressor (also used for QuickTime video)
DECLAREOBJCREATE(CAVIDec, CreateInstance)
// Video Renderer
DECLAREOBJCREATE(CRenderer, CreateInstance)
// Colour Space Converter
DECLAREOBJCREATE(CColour, CreateInstance)
// Quick Time Parser
DECLAREOBJCREATE(CQT, CreateInstance)
// MIDI Renderer
DECLAREOBJCREATE(CMidiOutDevice, CreateInstance)
// MIDI Parser (MID and RMI)
DECLAREOBJCREATE(CMIDIParse, CreateInstance)
// DirectMusic MIDI Renderer
DECLAREOBJCREATE(CDMFilter, CreateInstance)
// MPEG-1 Splitter
DECLAREOBJCREATE(CMpeg1Splitter, CreateInstance)
// DirectSound Audio Renderer
DECLAREOBJCREATEANDINIT(CDSoundDevice, CreateInstance, InitClass)
// Audio Renderer
DECLAREOBJCREATE(CWaveOutDevice, CreateInstance)
// MPEG-1 Audio decoder
DECLAREOBJCREATEANDINIT(CMpegAudioCodec, CreateInstance, InitClass)
// MPEG-1 Video decoder
DECLAREOBJCREATEANDINIT(CMpegVideoCodec, CreateInstance, InitClass)
// File Reader (URL) WARNING: This filter is part of the galahad tree
DECLAREOBJCREATE(CURLReader, CreateInstance)
// Overlay Mixer
DECLAREOBJCREATE(COMFilter, CreateInstance)
// Video Port
DECLAREOBJCREATE(CAMVideoPort, CreateInstance)
// DV Splitter
DECLAREOBJCREATE(CDVSp, CreateInstance)
// DV Decoder
DECLAREOBJCREATEANDINIT(CDVVideoCodec, CreateInstance, InitClass)
// Text Renderer
DECLAREOBJCREATE(CTextThing, CreateInstance)

//==========================================================================;
// simple macros to allow a product to easily build the quartz.dll
//==========================================================================;

// all the core componets that are required
#define INITSYSTEMOBJECTS \
    {L"System Clock"                            , &CLSID_SystemClock           	\
        , CSystemClock::CreateInstance          , NULL},                      	\
    {L"Seeking"                                 , &CLSID_SeekingPassThru       	\
        , CSeekingPassThru::CreateInstance      , NULL},                      	\
    {L"Memory Allocator"                        , &CLSID_MemoryAllocator       	\
        , CMemAllocator::CreateInstance         , NULL },                      	\
    {L"Filter Mapper"                           , &CLSID_FilterMapper         	\
        , CFilterMapper::CreateInstance         , CFilterMapper::MapperInit },	\
    {L"Filter Graph"                            , &CLSID_FilterGraph           	\
        , CFilterGraph::CreateThreadedInstance  , NULL },                      	\
    {L"Filter Graph no thread"                  , &CLSID_FilterGraphNoThread   	\
        , CFilterGraph::CreateInstance          , CFilterGraph::InitClass }

// individual filters
#define INITFILTER_FILESOURCEASYNC \
    {L"File Source (Async.)"                    , &CLSID_AsyncReader \
        , CAsyncReader::CreateInstance          , NULL }

#define INITFILTER_WAVEPARSER \
    {L"Wave Parser"                             , &CLSID_WAVEParser \
        , CWAVEParse::CreateInstance            , NULL }

#define INITFILTER_ACMWRAPPER \
    {L"ACM Wrapper"								, &CLSID_ACMWrapper \
        , CACMWrapper::CreateInstance           , NULL }

#define INITFILTER_DSOUNDRENDERER \
    {L"DirectSound Audio Renderer"              , &CLSID_DSoundRender \
        , CDSoundDevice::CreateInstance         , CDSoundDevice::InitClass }

#define INITFILTER_AUDIORENDERER \
    {L"Audio Renderer"                          , &CLSID_AudioRender \
        , CWaveOutDevice::CreateInstance        , NULL }

#define INITFILTER_AVISPLITTER \
    {L"AVI Splitter"                            , &CLSID_AviSplitter \
        , CAviMSRFilter::CreateInstance         , NULL }

#define INITFILTER_AVIDECOMPRESSOR \
    {L"AVI Decompressor"                        , &CLSID_AVIDec \
        , CAVIDec::CreateInstance               , NULL }

#define INITFILTER_MPEG1AUDIOCODEC \
    {L"MPEG Audio Codec"                        , &CLSID_CMpegAudioCodec \
        , CMpegAudioCodec::CreateInstance       , CMpegAudioCodec::InitClass }

#define INITFILTER_MPEG1VIDEOCODEC \
    {L"MPEG Video Codec"                        , &CLSID_CMpegVideoCodec \
        , CMpegVideoCodec::CreateInstance       , CMpegVideoCodec::InitClass }

#define INITFILTER_MPEG1SPLITTER \
    {L"MPEG-I Stream Splitter"                  , &CLSID_MPEG1Splitter \
        , CMpeg1Splitter::CreateInstance        , NULL }

#define INITFILTER_VIDEORENDERER \
    {L"Video Renderer"                          , &CLSID_VideoRenderer \
        , CRenderer::CreateInstance             , NULL }

#define INITFILTER_COLOURSPACECONVERTER \
    {L"Color Space Converter"                   , &CLSID_Colour \
        , CColour::CreateInstance               , NULL }

#define INITFILTER_QUICKTIMEPARSER \
    {L"QuickTime Movie Parser"                  , &CLSID_QuickTimeParser \
        , CQT::CreateInstance                   , NULL }

#define INITFILTER_MIDIRENDERER \
    {L"MIDI Renderer"                           , &CLSID_AVIMIDIRender \
        , CMidiOutDevice::CreateInstance        , NULL }

#define INITFILTER_MIDIPARSER \
    {L"MIDI Parser"                             , &CLSID_MIDIParser \
        , CMIDIParse::CreateInstance            , NULL }

#define INITFILTER_FILESOURCEURL \
    {L"File Source (URL)"                       , &CLSID_URLReader \
        , CURLReader::CreateInstance            , NULL }

#define INITFILTER_OVERLAYMIXER \
	{L"Overlay Mixer"                           , &CLSID_OverlayMixer \
        , COMFilter::CreateInstance             , NULL }

#define INITFILTER_VIDEOPORT \
	{L"VideoPort Object"                        , &CLSID_VPObject \
        , CAMVideoPort::CreateInstance          , NULL }

#define INITFILTER_DVSPLITTER \
	{L"DV Splitter"                             , &CLSID_DVSplitter \
        , CDVSp::CreateInstance                 , NULL }

#define INITFILTER_DVDECODER \
    {L"DV Video Decoder"                        , &CLSID_DVVideoCodec \
        , CDVVideoCodec::CreateInstance         , CDVVideoCodec::InitClass }

#define INITFILTER_MIDIDM \
    {L"DirectMusic MIDI Renderer"				,&CLSID_DMFilter, \
		CDMFilter::CreateInstance,				NULL}

#define INITFILTER_SCMDRENDERER \
    {L"Internal Script Command Renderer"			,&CLSID_ScriptCommandRenderer, \
		CTextThing::CreateInstance,				NULL}

#endif // _QUARTZ_H__

