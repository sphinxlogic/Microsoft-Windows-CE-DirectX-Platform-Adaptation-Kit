/*
 *  Company: IGS Technologies. 1995-1998.
 *  Author : Ngo^ Anh Vu~
 *  Purpose: See TOOL98.C for more information
 *
 *  This module is used for Driver, HAL and TAL
 *
 */


#ifndef TOOLHAL_H

#define TOOLHAL_H

/* Error code return from dwRetVal */
#define TOOL98_OK                       0x0000
#define TOOL98ERR_CREATEDCFAIL          0x0001
#define TOOL98ERR_INVALIDDC             0x0002
#define TOOL98ERR_UNSUPPORTED           0x0003
#define TOOL98ERR_HKEYINVALID           0x0004
#define TOOL98ERR_REGISTRYFAILED        0x0005



/* we connect driver thru ExtEscape function with Escape code=IGST's vendor ID */
#define ESCAPE_IGST                             0x2000
#define IGST_SIGNATURE                          'IGST'
#define TOOL98_GETVERSION                       0x0000
#define TOOL98_GETCHIPINFO                      0x0001

#define TOOL98_GETDEVADDR                       0x0010

#define TOOL98_OPENDEVKEY                       0x0020

#define TOOL98_GETTVINTERFACE                   0x0030
#define TOOL98_GETCOPINTERFACE                  0x0031


#define TOOL98_DOPHYSIO                         0x0100
#define TOOL98_PHYS2LINR                        0x0101
#define TOOL98_LINR2PHYS                        0x0102  //not avail yet!

#define TOOL98_FLUSHDEVBMP                      0x0200
#define TOOL98_SUSPENDDEVBMP                    0x0201
#define TOOL98_RESUMEDEVBMP                     0x0202

typedef struct {
    DWORD   dwSignature;
    DWORD   dwCmd;
} TOOL98_CMD, *LPTOOL98_CMD;

typedef struct {
    DWORD   dwDrvVer;
    DWORD   dwItfVer;
    DWORD   dwRetVal;
} TOOL98_VERSION, *LPTOOL98_VERSION;

typedef struct {
    DWORD   dwChipID;
    DWORD   dwVideoMemory;
    DWORD   dwHWFlags;

    DWORD   dwRetVal;
} TOOL98_CHIPINFO, *LPTOOL98_CHIPINFO;

typedef struct {
    DWORD   dwDevPhysAddr;
    DWORD   dwScrAddr;
    DWORD   dwOffAddr;
    DWORD   dwIOAddr;
    DWORD   dwCOPAddr;
    DWORD   dwTVAddr;
    DWORD   dwRetVal;
} TOOL98_DEVADDR, *LPTOOL98_DEVADDR;

typedef struct {
    WORD    wTVOn;
    WORD    wFlicker;
    WORD    wScart;
    WORD    WhatTVMode;                      //0: DropLine, 1: Interp
    DWORD   dwTVFlags;
    DWORD   TVRes1;
    //color base
    WORD    wE4A0Base;
    WORD    wE4A0Ofs;
    WORD    wE4A4Base;
    WORD    wE4A4Ofs;
    DWORD   TVRes2;
    DWORD   TVRes3;

    //zoom in/out
    WORD    TV_LineCtrl;
    WORD    TV_VTotal;
    WORD    TV_VDispE;
    WORD    TVRes4;
    DWORD   TVRes5;
    DWORD   TVRes6;

    //centering
    WORD    wE468Base;
    WORD    wE468Ofs;
    WORD    wE46CBase;
    WORD    wE46COfs;
    DWORD   TVRes7;
    DWORD   TVRes8;

    WORD    wE490Base;
    WORD    wE490Ofs;
    WORD    wE494Base;
    WORD    wE494Ofs;
    WORD    wE498Base;
    WORD    wE498Ofs;
    WORD    wE49CBase;
    WORD    wE49COfs;

} TVINFO, *LPTVINFO;

#define TVIF_GETTVINFO                  0x00000
#define TVIF_TVON                       0x00001
#define TVIF_FLICKER                    0x00002
#define TVIF_SCART                      0x00004
#define TVIF_INTERPOLATION              0x00008
#define TVIF_UPDATECOLORBASE            0x00100
#define TVIF_LINECONTROL                0x10000
#define TVIF_CENTERING                  0x20000

typedef struct {
    LPTVINFO pTVI;
    DWORD   dwCmd;
    DWORD   dwRetVal;
} TOOL98_TVINTERFACE, *LPTOOL98_TVINTERFACE;


#define IOF_INDEX   0x80
#define IOF_READ    0x40
#define IOF_MASK    0x20
#define IOF_WRITE   0x10
#define IOF_BYTE    0x00
#define IOF_WORD    0x01
#define IOF_DWORD   0x02
#define IOF_RMWRITE (IOF_READ+IOF_MASK+IO_WRITE)

#define IO_Idx(x)   LOBYTE(LOWORD(x))
#define IO_Val(x)   HIBYTE(LOWORD(x))
#define IO_Mask(x)  LOBYTE(HIWORD(x))
#define IO_Op(x)    HIBYTE(HIWORD(x))
#define IO_Make(idx,val,mask,op)    (DWORD)(idx+(val<<8)+(mask<<16)+(op<<24))

typedef struct {
    DWORD   dwPort;
#ifdef _WIN32
    DWORD   dwValue;
#else
    BYTE    bIndex;
    BYTE    bValue;
    BYTE    bMask;
    BYTE    bOp;
#endif

} TOOL98_IODATA, *LPTOOL98_IODATA;


typedef struct {
    DWORD   dwIOSize;
    DWORD   dwRetVal;
    TOOL98_IODATA iodata[1];

} TOOL98_PHYSIO, *LPTOOL98_PHYSIO;


/*
 *  Pass this struct with dwAddr = PA, dwSize=PA's size.
 *  Return value in dwAddr will be LA.
 */
typedef struct {
    DWORD   dwAddr;
    DWORD   dwSize;
    DWORD   dwRetVal;
} TOOL98_ADDR,*LPTOOL98_ADDR;


typedef struct {
    HKEY    hkDev;
    DWORD   dwRetVal;
} TOOL98_DEVKEY, *LPTOOL98_DEVKEY;

typedef struct {
    char    *pSubKey;
    char    *pValueName;
    char    *pBuffer;
    DWORD   dwType;
    DWORD   dwSize;
    DWORD   dwRetVal;
} TOOL98_REGISTRY, *LPTOOL98_REGISTRY;


#endif
