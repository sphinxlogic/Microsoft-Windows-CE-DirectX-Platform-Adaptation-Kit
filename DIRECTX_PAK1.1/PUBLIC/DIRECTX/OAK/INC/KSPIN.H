//==========================================================================;
//
//  Copyright (c) 1997 - 2000  Microsoft Corporation.	All Rights Reserved.
//
//--------------------------------------------------------------------------;

#ifndef __IKSPIN__
#define __IKSPIN__

#ifdef __cplusplus
extern "C" {
#endif

interface IKsInterfaceHandler;
interface IKsDataTypeHandler;
interface IKsPin;

typedef enum {
    AM_PROPERTY_PIN_CINSTANCES,
    AM_PROPERTY_PIN_CTYPES,
    AM_PROPERTY_PIN_DATAFLOW,
    AM_PROPERTY_PIN_DATARANGES,
    AM_PROPERTY_PIN_DATAINTERSECTION,
    AM_PROPERTY_PIN_INTERFACES,
    AM_PROPERTY_PIN_MEDIUMS,
    AM_PROPERTY_PIN_COMMUNICATION,
    AM_PROPERTY_PIN_GLOBALCINSTANCES,
    AM_PROPERTY_PIN_NECESSARYINSTANCES,
    AM_PROPERTY_PIN_PHYSICALCONNECTION,
    AM_PROPERTY_PIN_CATEGORY,
    AM_PROPERTY_PIN_NAME
} AM_PROPERTY_PIN;


typedef enum {
    AM_INTERFACE_STANDARD_STREAMING,
    AM_INTERFACE_STANDARD_LOOPED_STREAMING,
    AM_INTERFACE_STANDARD_CONTROL
} AM_INTERFACE_STANDARD;


typedef union 
{
    struct 
    {
        GUID    Set;
        ULONG   Id;
        ULONG   Flags;
    };
#if defined(_NTDDK_)
    ULONGLONG  Alignment;
#else // !_NTDDK_
    DWORDLONG  Alignment;
#endif // !_NTDDK_
} AM_IDENTIFIER, *PAM_IDENTIFIER;

typedef AM_IDENTIFIER AM_PIN_MEDIUM, *PAM_PIN_MEDIUM;
typedef AM_IDENTIFIER AM_PIN_INTERFACE, *PAM_PIN_INTERFACE;

typedef struct 
{
    ULONG    Size;
    ULONG    Count;
} AM_MULTIPLE_ITEM, *PAM_MULTIPLE_ITEM;

typedef enum 
{
    AM_PeekOperation_PeekOnly,
    AM_PeekOperation_AddRef
} AM_PEEKOPERATION;

typedef enum 
{
    AM_PIN_COMMUNICATION_NONE,
    AM_PIN_COMMUNICATION_SINK,
    AM_PIN_COMMUNICATION_SOURCE,
    AM_PIN_COMMUNICATION_BOTH,
    AM_PIN_COMMUNICATION_BRIDGE
} AM_PIN_COMMUNICATION, *PAM_PIN_COMMUNICATION;

#define AM_PROPERTY_SUPPORT_GET 1
#define AM_PROPERTY_SUPPORT_SET 2

typedef enum 
{
    AM_IoOperation_Write,
    AM_IoOperation_Read
} AM_IOOPERATION;


// This structure definition is the common header required by the proxy to 
// dispatch the stream segment to the interface handler.  Interface handlers 
// will create extended structures to include other information such as 
// media samples, extended header size and so on.
typedef struct _AM_STREAM_SEGMENT 
{
    IKsInterfaceHandler     *KsInterfaceHandler;
    IKsDataTypeHandler      *KsDataTypeHandler;
    AM_IOOPERATION          IoOperation;
    HANDLE                  CompletionEvent;
} AM_STREAM_SEGMENT;
typedef struct _AM_STREAM_SEGMENT *PAM_STREAM_SEGMENT;


// definition of interface IKsDataTypeHandler
DECLARE_INTERFACE_(IKsDataTypeHandler, IUnknown)
{
    STDMETHOD (KsCompleteIoOperation)(THIS_
				      IMediaSample *Sample, 
				      PVOID StreamHeader, 
				      AM_IOOPERATION IoOperation, 
				      BOOL Cancelled
				     ) PURE;
    STDMETHOD (KsIsMediaTypeInRanges)(THIS_
				      PVOID DataRanges
				     ) PURE;
    STDMETHOD (KsPrepareIoOperation)(THIS_
				     IMediaSample *Sample, 
				     PVOID StreamHeader, 
				     AM_IOOPERATION IoOperation
				    ) PURE;
    STDMETHOD (KsQueryExtendedSize)(THIS_
				    ULONG* ExtendedSize
				   ) PURE;
    STDMETHOD (KsSetMediaType)(THIS_
			       const CMediaType *MediaType
			      ) PURE;
};


// definition of interface IKsInterfaceHandler
DECLARE_INTERFACE_(IKsInterfaceHandler, IUnknown)
{
    STDMETHOD (KsSetPin)(THIS_
			 IKsPin *KsPin 
			) PURE;
    STDMETHOD (KsProcessMediaSamples)(THIS_
				      IKsDataTypeHandler *KsDataTypeHandler,
				      IMediaSample** SampleList, 
				      PLONG SampleCount, 
				      AM_IOOPERATION IoOperation, 
				      PAM_STREAM_SEGMENT *StreamSegment
				     ) PURE;
    STDMETHOD (KsCompleteIo)(THIS_
			     PAM_STREAM_SEGMENT StreamSegment
			    ) PURE;
};


// definition of interface IKsPin
DECLARE_INTERFACE_(IKsPin, IUnknown)
{
    STDMETHOD (KsQueryMediums)(THIS_
			       OUT PAM_MULTIPLE_ITEM* ppMediumList
			      ) PURE;

    STDMETHOD (KsQueryInterfaces)(THIS_
				  OUT PAM_MULTIPLE_ITEM* ppInterfaceList
				 ) PURE;

    STDMETHOD (KsCreateSinkPinHandle)(THIS_
				      IN AM_PIN_INTERFACE& Interface,
				      IN AM_PIN_MEDIUM& Medium
				     ) PURE;

    STDMETHOD (KsGetCurrentCommunication)(THIS_
					  OUT AM_PIN_COMMUNICATION *pCommunication,
					  OUT AM_PIN_INTERFACE *pInterface,
					  OUT AM_PIN_MEDIUM *pMedium
					 ) PURE;

    STDMETHOD (KsPropagateAcquire)(THIS_
				  ) PURE;

    STDMETHOD (KsDeliver)(THIS_
			  IN IMediaSample* Sample, 
			  IN ULONG Flags
			 ) PURE;

    STDMETHOD (KsMediaSamplesCompleted)(THIS_
					PAM_STREAM_SEGMENT StreamSegment 
				       ) PURE;
    
    STDMETHOD_(IMemAllocator *, KsPeekAllocator)(THIS_
						 IN AM_PEEKOPERATION Operation
						) PURE;

    STDMETHOD (KsReceiveAllocator)(THIS_
				   IMemAllocator *MemAllocator
				  ) PURE;

    STDMETHOD (KsRenegotiateAllocator)(THIS_
				      ) PURE;

    STDMETHOD_(LONG, KsIncrementPendingIoCount)(THIS_
					       ) PURE;

    STDMETHOD_(LONG, KsDecrementPendingIoCount)(THIS_
					       ) PURE;

    STDMETHOD (KsQualityNotify)(THIS_
				IN ULONG Proportion, 
				IN REFERENCE_TIME TimeDelta
			       ) PURE;
};

#ifdef __cplusplus
}
#endif


#endif // #define __IKSPIN__
