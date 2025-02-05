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

// dxmasf.h : IMPORTANT PLEASE READ BELOW

// This file defines all the static create and init function in all of the
// core DirectShow filters. This allows individual projects to build one
// DirectShow dll (quartz.dll) with only the filters that they require.
//
// To avoid having to include numerous private include files, this file
// defines the classes used by each filter BUT with only the static functions
// so that the compiler can generate the right symbols which enables the
// linker to find the correct functions to link to.

#ifndef __DXMASF_H__
#define __DXMASF_H__

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

// macro for Create functions without objects
#define DECLARECREATE(createfunc) \
extern CUnknown *createfunc##(LPUNKNOWN lpunk, HRESULT *phr);

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
// Class definitions of all WMT filters, a product is free to
// add their own filters if they want to.
//==========================================================================;

// {B9D1F321-C401-11d0-A520-000000000000}
DEFINE_GUID(CLSID_ASFACMHandler,
0xb9d1f321, 0xc401, 0x11d0, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

// {B9D1F322-C401-11d0-A520-000000000000}
DEFINE_GUID(CLSID_ASFICMHandler,
0xb9d1f322, 0xc401, 0x11d0, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

// {B9D1F323-C401-11d0-A520-000000000000}
DEFINE_GUID(CLSID_ASFURLHandler,
0xb9d1f323, 0xc401, 0x11d0, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

// {6B6D0800-9ADA-11d0-A520-00A0D10129C0}
DEFINE_GUID(CLSID_NetShowSource,
0x6b6d0800, 0x9ada, 0x11d0, 0xa5, 0x20, 0x0, 0xa0, 0xd1, 0x1, 0x29, 0xc0);

// {640999A2-A946-11d0-A520-000000000000}
DEFINE_GUID(CLSID_NSCFile,
0x640999a2, 0xa946, 0x11d0, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

// {D51BD5AE-7548-11cf-A520-0080C77EF58A}
DEFINE_GUID(CLSID_XMLPlaylist,
0xd51bd5ae, 0x7548, 0x11cf, 0xa5, 0x20, 0x0, 0x80, 0xc7, 0x7e, 0xf5, 0x8a);

// {4B428940-263C-11d1-A520-000000000000}
DEFINE_GUID(CLSID_URLGrabber,
0x4b428940, 0x263c, 0x11d1, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

// {B9D1F32e-C401-11d0-A520-000000000000}
DEFINE_GUID(CLSID_ASFEmbeddedFileHandler,
0xb9d1f32e, 0xc401, 0x11d0, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

DEFINE_GUID(CLSID_AsfXStreamTypeJpegJfifVideo,
    0xb61be100,0x5b4e,0x11cf,0xa8,0xfd,0x00,0x80,0x5f,0x5c,0x44,0x2b);

DEFINE_GUID(CLSID_AsfXStreamTypeDegradableJpeg,
    0x35907de0,0xe415,0x11cf,0xa9,0x17,0x00,0x80,0x5f,0x5c,0x44,0x2b);

// {B9D1F325-C401-11d0-A520-000000000000}
DEFINE_GUID(CLSID_ASFDJPEGHandler, 
0xb9d1f325, 0xc401, 0x11d0, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

// {B9D1F324-C401-11d0-A520-000000000000}
DEFINE_GUID(CLSID_ASFJPEGHandler, 
0xb9d1f324, 0xc401, 0x11d0, 0xa5, 0x20, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0);

//==========================================================================;
// simple macros to allow a product to easily build the dxmasf.dll
//==========================================================================;

// ASF ACM Handler
DECLAREOBJCREATE(CAcmObjHandler, CreateInstance)
// ASF ICM Handler
DECLAREOBJCREATE(CIcmObjHandler, CreateInstance)
// ASF URL Handler
DECLAREOBJCREATE(CUrlObjHandler, CreateInstance)
// ASF File Handler
DECLAREOBJCREATE(CFileObjHandler, CreateInstance)

// DJPEG Handler
DECLAREOBJCREATE(CDJpegObjHandler, CreateInstance)
// JPEG Handler
DECLAREOBJCREATE(CJpegObjHandler, CreateInstance)

// Splitter
DECLARE2OBJCREATEANDINIT(CNSSource, CreateInstance, CreateNSCInstance, InitClass)

// URL File Source
DECLARECREATE( CreateURLGrabberInstance )

// XML Playlist
DECLARECREATE( CreateXMLPlayInstance )

// all the core componets that are required
#define INIT_SPLITTER \
    { L"Windows Media splitter", &CLSID_NetShowSource, CNSSource::CreateInstance, CNSSource::InitClass }

// individual filters

#define INIT_JPEGHANDLER \
    { L"JPEG ICM Handler", &CLSID_AsfXStreamTypeJpegJfifVideo, CJpegObjHandler::CreateInstance, NULL }

#define INIT_DJPEGHANDLER \
    { L"DJPEG ICM Handler", &CLSID_AsfXStreamTypeDegradableJpeg, CDJpegObjHandler::CreateInstance, NULL }

#define INIT_ACMHANDLER \
    { L"ASF ACM Handler", &CLSID_ASFACMHandler, CAcmObjHandler::CreateInstance, NULL }

#define INIT_ICMHANDLER \
    { L"ASF ICM Handler", &CLSID_ASFICMHandler, CIcmObjHandler::CreateInstance, NULL }

#define INIT_URLHANDLER \
    { L"ASF URL Handler", &CLSID_ASFURLHandler, CUrlObjHandler::CreateInstance, NULL }

#define INIT_XMLASXFILEPARSER \
    { L"XML-based ASX Parser", &CLSID_XMLPlaylist, CreateXMLPlayInstance, NULL }

#define INIT_NSCFILEPARSER \
    { L"NSC File Parser", &CLSID_NSCFile, CNSSource::CreateNSCInstance, NULL }

#define INIT_URLFILESOURCE \
    { L"Windows Media URL File Source", &CLSID_URLGrabber, CreateURLGrabberInstance, NULL }

struct IMediaStreamMonitor;
struct IMediaStream;

typedef HRESULT (WINAPI* PFNAMSCREATE)( LPCWSTR pszStreamName, 
                                        IMediaStreamMonitor *pMonitor,
                                        IMediaStream **ppStream );

struct AMSREGISTRYENTRY
{
    PFNAMSCREATE pfnCreate;
};

// macro for objects that only have a Create function
#define DECLARESTRMOBJCREATE(classname, createfunc) \
class classname## \
{ \
public: \
    static HRESULT WINAPI createfunc## (LPCWSTR pszStreamName,          \
                                        IMediaStreamMonitor *pMonitor,  \
                                        IMediaStream **ppStream );      \
};

DECLARESTRMOBJCREATE(CHttpStream, CreateInstance)
DECLARESTRMOBJCREATE(CRtspStream, CreateInstance)
DECLARESTRMOBJCREATE(CMediaServerStream, CreateInstance)
DECLARESTRMOBJCREATE(CAdioFileStream, CreateInstance)
DECLARESTRMOBJCREATE(CFileStream, CreateInstance)
DECLARESTRMOBJCREATE(CTigerStream, CreateInstance)
DECLARESTRMOBJCREATE(CMsbStream, CreateInstance)
DECLARESTRMOBJCREATE(CLiveAudioStream, CreateInstance)

#endif // __DXMASF_H__

