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
/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 3.02.88 */
/* at Tue Mar 07 14:12:25 2000
 */
/* Compiler settings for .\wmphlpr.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: none
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

const IID IID_IWMPHelper = {0x3478A2DE,0xB06F,0x4e2a,{0x9C,0x30,0x70,0x02,0xA8,0xB7,0x5C,0x97}};


const IID LIBID_WMPHLPRLib = {0x0984B40D,0xFE16,0x484d,{0x98,0x10,0xCE,0x0B,0x64,0x66,0xA4,0xD5}};


const CLSID CLSID_WMPHelper = {0xD00DA9D6,0x97AF,0x47f3,{0xAB,0x30,0x97,0xAA,0xB6,0xAC,0x54,0xF9}};


#ifdef __cplusplus
}
#endif

