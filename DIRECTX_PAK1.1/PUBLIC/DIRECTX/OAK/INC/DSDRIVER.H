/*==========================================================================;
 *
 *  Copyright (C) 1995-2000 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dsdriver.h
 *  Content:    DirectSound driver include file
 *
 ***************************************************************************/

#ifndef __DSDRIVER_INCLUDED__
#define __DSDRIVER_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//
// Structures
//

// Driver description
typedef struct _DSDRIVERDESC 
{
    DWORD       dwFlags;
    CHAR        szDesc[256];
    CHAR        szDrvname[256];
    DWORD       dnDevNode;
    WORD        wVxdId;
    WORD        wReserved;
    ULONG       ulDeviceNum;
    DWORD       dwHeapType;
    LPVOID      pvDirectDrawHeap;
    DWORD       dwMemStartAddress;
    DWORD       dwMemEndAddress;
    DWORD       dwMemAllocExtra;
    LPVOID      pvReserved1;
    LPVOID      pvReserved2;
} DSDRIVERDESC, *PDSDRIVERDESC;

#define DSDDESC_DOMMSYSTEMOPEN          0x00000001
#define DSDDESC_DOMMSYSTEMSETFORMAT     0x00000002
#define DSDDESC_USESYSTEMMEMORY         0x00000004
#define DSDDESC_DONTNEEDPRIMARYLOCK     0x00000008
#define DSDDESC_DONTNEEDSECONDARYLOCK   0x00000010

#define DSDHEAP_NOHEAP                  0x00000000
#define DSDHEAP_CREATEHEAP              0x00000001
#define DSDHEAP_USEDIRECTDRAWHEAP       0x00000002
#define DSDHEAP_PRIVATEHEAP             0x00000003

// Driver caps
typedef struct _DSDRIVERCAPS 
{
    DWORD       dwFlags;
    DWORD       dwMinSecondarySampleRate;
    DWORD       dwMaxSecondarySampleRate;
    DWORD       dwPrimaryBuffers;
    DWORD       dwMaxHwMixingAllBuffers;
    DWORD       dwMaxHwMixingStaticBuffers;
    DWORD       dwMaxHwMixingStreamingBuffers;
    DWORD       dwFreeHwMixingAllBuffers;
    DWORD       dwFreeHwMixingStaticBuffers;
    DWORD       dwFreeHwMixingStreamingBuffers;
    DWORD       dwMaxHw3DAllBuffers;
    DWORD       dwMaxHw3DStaticBuffers;
    DWORD       dwMaxHw3DStreamingBuffers;
    DWORD       dwFreeHw3DAllBuffers;
    DWORD       dwFreeHw3DStaticBuffers;
    DWORD       dwFreeHw3DStreamingBuffers;
    DWORD       dwTotalHwMemBytes;
    DWORD       dwFreeHwMemBytes;
    DWORD       dwMaxContigFreeHwMemBytes;
} DSDRIVERCAPS, *PDSDRIVERCAPS;

// Buffer volume and pan
typedef struct _DSVOLUMEPAN 
{
    DWORD       dwTotalLeftAmpFactor;
    DWORD       dwTotalRightAmpFactor;
    LONG        lVolume;
    DWORD       dwVolAmpFactor;
    LONG        lPan;
    DWORD       dwPanLeftAmpFactor;
    DWORD       dwPanRightAmpFactor;
} DSVOLUMEPAN, *PDSVOLUMEPAN;

// Property set identifier
typedef union _DSPROPERTY
{
    struct
    {
        GUID    Set;
        ULONG   Id;
        ULONG   Flags;
        ULONG   InstanceId;
    };
#if defined(_NTDDK_)
    ULONGLONG   Alignment;
#else // !_NTDDK_
    DWORDLONG   Alignment;
#endif // !_NTDDK_
} DSPROPERTY, *PDSPROPERTY;

// IDsCaptureDriver caps
// same as DSCCAPS for IDirectSoundCapture::GetCaps
typedef struct _DSCDRIVERCAPS
{
    DWORD   dwSize;
    DWORD   dwFlags;
    DWORD   dwFormats;
    DWORD   dwChannels;
} DSCDRIVERCAPS, * PDSCDRIVERCAPS;

//
// DirectSound driver interfaces
//

DEFINE_GUID(IID_IDsDriver, 0x8c4233c0l, 0xb4cc, 0x11ce, 0x92, 0x94, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);
DEFINE_GUID(IID_IDsDriverBuffer, 0x8c4233c1l, 0xb4cc, 0x11ce, 0x92, 0x94, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);
DEFINE_GUID(IID_IDsDriverPropertySet, 0xf6f2e8e0, 0xd842, 0x11d0, 0x8f, 0x75, 0x0, 0xc0, 0x4f, 0xc2, 0x8a, 0xca);

// New for CE:
// {0363EF44-3B57-11d3-AC79-00105A017FE1}
DEFINE_GUID(IID_IDsDriverNotify, 
0x363ef44, 0x3b57, 0x11d3, 0xac, 0x79, 0x0, 0x10, 0x5a, 0x1, 0x7f, 0xe1);

// {3DD10C47-74FB-11d3-9049-CBB4B32EAA08}
DEFINE_GUID(IID_IDsCaptureDriver, 
0x3dd10c47, 0x74fb, 0x11d3, 0x90, 0x49, 0xcb, 0xb4, 0xb3, 0x2e, 0xaa, 0x8);

// {3DD10C48-74FB-11d3-9049-CBB4B32EAA08}
DEFINE_GUID(IID_IDsCaptureDriverBuffer, 
0x3dd10c48, 0x74fb, 0x11d3, 0x90, 0x49, 0xcb, 0xb4, 0xb3, 0x2e, 0xaa, 0x8);


//  IDsDriverNotify
typedef struct IDsDriverNotifyVtbl IDSDRIVERNOTIFYVTBL, *PIDSDRIVERNOTIFYVTBL;
typedef struct IDsDriverNotify IDSDRIVERNOTIFY, *PIDSDRIVERNOTIFY;

#undef INTERFACE
#define INTERFACE IDsDriverNotify

DECLARE_INTERFACE_(IDsDriverNotify, IUnknown)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
    // IDsDriverBuffer methods
	STDMETHOD(SetNotificationPositions) (THIS_ DWORD, LPCDSBPOSITIONNOTIFY) PURE;
};









//  IDsDriverBuffer
typedef struct IDsDriverBufferVtbl IDSDRIVERBUFFERVTBL, *PIDSDRIVERBUFFERVTBL;
typedef struct IDsDriverBuffer IDSDRIVERBUFFER, *PIDSDRIVERBUFFER;

#undef INTERFACE
#define INTERFACE IDsDriverBuffer

DECLARE_INTERFACE_(IDsDriverBuffer, IUnknown)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
    // IDsDriverBuffer methods
    STDMETHOD(Lock)                 (THIS_ LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD, DWORD, DWORD ) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID, DWORD, LPVOID, DWORD ) PURE;
    STDMETHOD(SetFormat)            (THIS_ LPWAVEFORMATEX ) PURE;
    STDMETHOD(SetFrequency)         (THIS_ DWORD ) PURE;
    STDMETHOD(SetVolumePan)         (THIS_ PDSVOLUMEPAN ) PURE;
    STDMETHOD(SetPosition)          (THIS_ DWORD ) PURE;
    STDMETHOD(GetPosition)          (THIS_ LPDWORD, LPDWORD ) PURE;
    STDMETHOD(Play)                 (THIS_ DWORD, DWORD, DWORD ) PURE;
    STDMETHOD(Stop)                 (THIS ) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD) PURE;
};

//  IDsDriver
typedef struct IDsDriverVtbl IDSDRIVERVTBL, *PIDSDRIVERVTBL;
typedef struct IDsDriver IDSDRIVER, *PIDSDRIVER;

#undef INTERFACE
#define INTERFACE IDsDriver

DECLARE_INTERFACE_(IDsDriver, IUnknown)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
    // IDsDriver methods
    STDMETHOD(GetDriverDesc)        (THIS_ PDSDRIVERDESC ) PURE;
    STDMETHOD(Open)                 (THIS ) PURE;
    STDMETHOD(Close)                (THIS ) PURE;
    STDMETHOD(GetCaps)              (THIS_ PDSDRIVERCAPS ) PURE;
    STDMETHOD(CreateSoundBuffer)    (THIS_ LPWAVEFORMATEX, DWORD, DWORD, LPDWORD, LPBYTE *, LPVOID * ) PURE;
    STDMETHOD(DuplicateSoundBuffer) (THIS_ PIDSDRIVERBUFFER, LPVOID * ) PURE;
};


//  IDsCaptureDriverBuffer
typedef struct IDsCaptureDriverBufferVtbl IDSCAPTUREDRIVERBUFFERVTBL, *PIDSCAPTUREDRIVERBUFFERVTBL;
typedef struct IDsCaptureDriverBuffer IDSCAPTUREDRIVERBUFFER, *PIDSCAPTUREDRIVERBUFFER;

#undef INTERFACE
#define INTERFACE IDsCaptureDriverBuffer

DECLARE_INTERFACE_(IDsCaptureDriverBuffer, IUnknown)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
    // IDsCaptureDriverBuffer methods
    STDMETHOD(Lock)                 (THIS_ LPVOID *, LPDWORD, LPVOID *, LPDWORD, DWORD, DWORD, DWORD ) PURE;
    STDMETHOD(Unlock)               (THIS_ LPVOID, DWORD, LPVOID, DWORD ) PURE;
    STDMETHOD(SetFormat)            (THIS_ LPWAVEFORMATEX ) PURE;
    STDMETHOD(GetPosition)          (THIS_ LPDWORD, LPDWORD ) PURE;
    STDMETHOD(Start)                (THIS_ DWORD ) PURE;
    STDMETHOD(Stop)                 (THIS ) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD) PURE;
};


//  IDsCaptureDriver
typedef struct IDsCaptureDriverVtbl IDSCAPTUREDRIVERVTBL, *PIDSCAPTUREDRIVERVTBL;
typedef struct IDsCaptureDriver IDSCAPTUREDRIVER, *PIDSCAPTUREDRIVER;

#undef INTERFACE
#define INTERFACE IDsCaptureDriver

DECLARE_INTERFACE_(IDsCaptureDriver, IUnknown)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
    // IDsDriver methods
    STDMETHOD(GetDriverDesc)        (THIS_ PDSDRIVERDESC ) PURE;
    STDMETHOD(Open)                 (THIS ) PURE;
    STDMETHOD(Close)                (THIS ) PURE;
    STDMETHOD(GetCaps)              (THIS_ PDSCDRIVERCAPS ) PURE;
    STDMETHOD(CreateCaptureBuffer)  (THIS_ LPWAVEFORMATEX, DWORD, DWORD, LPDWORD, LPBYTE *, LPVOID * ) PURE;
};

//  IDsDriverPropertySet
typedef struct IDsDriverPropertySetVtbl IDSDRIVERPROPERTYSETVTBL, *PIDSDRIVERPROPERTYSETVTBL;
typedef struct IDsDriverPropertySet IDSDRIVERPROPERTYSET, *PIDSDRIVERPROPERTYSET;

#undef INTERFACE
#define INTERFACE IDsDriverPropertySet

DECLARE_INTERFACE_(IDsDriverPropertySet, IUnknown)
{
    // IUnknown methods
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;
    
    // IDsDriverPropertySet
    STDMETHOD(Get)                  (THIS_ PDSPROPERTY, LPVOID, ULONG, LPVOID, ULONG, PULONG ) PURE;
    STDMETHOD(Set)                  (THIS_ PDSPROPERTY, LPVOID, ULONG, LPVOID, ULONG ) PURE;
    STDMETHOD(QuerySupport)         (THIS_ REFGUID, ULONG, PULONG ) PURE;
};

//
// Property sets
//

// 3D Listener property set {6D047B40-7AF9-11d0-9294-444553540000}
DEFINE_GUID(DSPROPSETID_DirectSound3DListener, 0x6d047b40, 0x7af9, 0x11d0, 0x92, 0x94, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

typedef enum 
{
    DSPROPERTY_DIRECTSOUND3DLISTENER_ALL,
    DSPROPERTY_DIRECTSOUND3DLISTENER_POSITION,
    DSPROPERTY_DIRECTSOUND3DLISTENER_VELOCITY,
    DSPROPERTY_DIRECTSOUND3DLISTENER_ORIENTATION,
    DSPROPERTY_DIRECTSOUND3DLISTENER_DISTANCEFACTOR,
    DSPROPERTY_DIRECTSOUND3DLISTENER_ROLLOFFFACTOR,
    DSPROPERTY_DIRECTSOUND3DLISTENER_DOPPLERFACTOR,
    DSPROPERTY_DIRECTSOUND3DLISTENER_BATCH,
    DSPROPERTY_DIRECTSOUND3DLISTENER_ALLOCATION
} DSPROPERTY_DIRECTSOUND3DLISTENER;

// 3D Buffer property set {6D047B41-7AF9-11d0-9294-444553540000}
DEFINE_GUID(DSPROPSETID_DirectSound3DBuffer, 0x6d047b41, 0x7af9, 0x11d0, 0x92, 0x94, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

typedef enum 
{
    DSPROPERTY_DIRECTSOUND3DBUFFER_ALL,
    DSPROPERTY_DIRECTSOUND3DBUFFER_POSITION,
    DSPROPERTY_DIRECTSOUND3DBUFFER_VELOCITY,
    DSPROPERTY_DIRECTSOUND3DBUFFER_CONEANGLES,
    DSPROPERTY_DIRECTSOUND3DBUFFER_CONEORIENTATION,
    DSPROPERTY_DIRECTSOUND3DBUFFER_CONEOUTSIDEVOLUME,
    DSPROPERTY_DIRECTSOUND3DBUFFER_MINDISTANCE,
    DSPROPERTY_DIRECTSOUND3DBUFFER_MAXDISTANCE,
    DSPROPERTY_DIRECTSOUND3DBUFFER_MODE
} DSPROPERTY_DIRECTSOUND3DBUFFER;

// Speaker configuration property set {6D047B42-7AF9-11d0-9294-444553540000}
DEFINE_GUID(DSPROPSETID_DirectSoundSpeakerConfig, 0x6d047b42, 0x7af9, 0x11d0, 0x92, 0x94, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

typedef enum 
{
    DSPROPERTY_DIRECTSOUNDSPEAKERCONFIG_SPEAKERCONFIG
} DSPROPERTY_DIRECTSOUNDSPEAKERCONFIG;



//
//  Driver registration
//


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __DSDRIVER_INCLUDED__
