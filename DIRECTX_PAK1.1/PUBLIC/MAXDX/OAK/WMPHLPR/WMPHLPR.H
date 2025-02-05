//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.02.88 */
/* at Tue Mar 07 14:12:25 2000
 */
/* Compiler settings for .\wmphlpr.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __wmphlpr_h__
#define __wmphlpr_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IWMPHelper_FWD_DEFINED__
#define __IWMPHelper_FWD_DEFINED__
typedef interface IWMPHelper IWMPHelper;
#endif 	/* __IWMPHelper_FWD_DEFINED__ */


#ifndef __WMPHelper_FWD_DEFINED__
#define __WMPHelper_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMPHelper WMPHelper;
#else
typedef struct WMPHelper WMPHelper;
#endif /* __cplusplus */

#endif 	/* __WMPHelper_FWD_DEFINED__ */


/* header files for imported files */
#include "ocidl.h"
#include "urlmon.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IWMPHelper_INTERFACE_DEFINED__
#define __IWMPHelper_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IWMPHelper
 * at Tue Mar 07 14:12:25 2000
 * using MIDL 3.02.88
 ****************************************/
/* [unique][helpstring][uuid][object] */ 



EXTERN_C const IID IID_IWMPHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("3478A2DE-B06F-4e2a-9C30-7002A8B75C97")
    IWMPHelper : public IUnknown
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IWMPHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPHelper __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPHelper __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPHelper __RPC_FAR * This);
        
        END_INTERFACE
    } IWMPHelperVtbl;

    interface IWMPHelper
    {
        CONST_VTBL struct IWMPHelperVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IWMPHelper_INTERFACE_DEFINED__ */



#ifndef __WMPHLPRLib_LIBRARY_DEFINED__
#define __WMPHLPRLib_LIBRARY_DEFINED__

/****************************************
 * Generated header for library: WMPHLPRLib
 * at Tue Mar 07 14:12:25 2000
 * using MIDL 3.02.88
 ****************************************/
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_WMPHLPRLib;

EXTERN_C const CLSID CLSID_WMPHelper;

#ifdef __cplusplus

class DECLSPEC_UUID("D00DA9D6-97AF-47f3-AB30-97AAB6AC54F9")
WMPHelper;
#endif
#endif /* __WMPHLPRLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
