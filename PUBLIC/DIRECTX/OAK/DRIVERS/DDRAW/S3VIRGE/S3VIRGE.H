/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

Module Name:    s3virge.h

Abstract:       defines for S3 ViRGE chipset

Notes: 
--*/

#ifndef __S3VIRGE_H__
#define __S3VIRGE_H__

#define NEWHAL
#define ENABLE_OVERLAY

// @@@ not enabled yet, need to change config.bib
// to reserve DMA buffer space
// #define ENABLE_DMA

// @@@ not enabled yet
// #define ENABLE_INTR

#include "gpevga.h"

// Physical addresses, defined in config.bib
#define MMIO_ADDRESS                0x000A0000  // base address of MMIO
#ifdef ENABLE_DMA
#define DMA_BUFFER_ADDRESS          0x80200000  // address of DMA circular buf
#endif // ENABLE_DMA

// Timeouts
#define WAIT_FOR_NOT_BUSY_TIMEOUT   500
#define WAIT_FOR_FIFO_TIMEOUT       100


typedef struct _FLIPRECORD
{
    ULONG           ulFlipFrom;             // video memory offset of the
                                            // the surface we last flipped from
    ULONG           ulFlipTime;             // Time at which last flip occured
    BOOL            bHaveEverCrossedVBlank; // True if we noticed that we
                                            //   switched from inactive to
                                            //   vblank
    BOOL            bWasEverInDisplay;      // True if we ever noticed that
                                            //   we were inactive
    BOOL            bFlipFlag;              // True if we think a flip is
                                            //   still pending
} FLIPRECORD;


class S3VirgeSurf;

class S3Virge  : public GPEVGA
{
private:
    SubIndexedRegArray16 reg_BEE8;          // Indexed accelerator registers

    PUCHAR          m_pLAW;                 // Linear access window (virtual)
    int             m_nVideoMemorySize;     // Size in bytes of video RAM total
    SurfaceHeap     *m_pVideoMemoryHeap;    // Base entry representing 
                                            // all video memory
    int             m_nScreenStride;        // Stride of 2d memory in bytes
    int             m_nScreenBpp;           // Bits/pixel
    ULONG           m_nLAWPhysical;         // Physical address of LAW
    UINT            m_nChipID;              // Which S3 Virge chipset
    ULONG           m_nDestFormat;          // Dest format for cmd set register
    FLIPRECORD      m_sFlipRecord;          // Used to track flipping status

#ifdef ENABLE_OVERLAY
    DDGPESurf      *m_pVisibleOverlay;      // Visible overlay surface
    DWORD           m_dwOverlayFlipOffset;  // Overlay flip offset
#endif // ENABLE_OVERLAY

#ifdef ENABLE_INTR
    DWORD           m_dwIntrId;             // interrupt ID
    HANDLE          m_hIntrEvent;           // main interrupt event
    DWORD           m_dwIntrSetting;        // interrupt setting

    // individual interrupt events
    HANDLE          m_hCtl1VSync;           // Controller 1 vertical sync
    HANDLE          m_hS3dDone;             // S3d engine done
    HANDLE          m_hCmdFifoOF;           // Command FIFO overflow
    HANDLE          m_hCmdFifoEmpty;        // Command FIFO empty
    HANDLE          m_hHostDmaDone;         // Host DMA Done
    HANDLE          m_hCmdDmaDone;          // Command DMA Done
    HANDLE          m_hS3dFifoEmpty;        // S3d FIFO empty

    HANDLE          m_hIntrThread;          // IST handle
#endif // ENABLE_INTR

public:
    PUCHAR          m_pMMIO;                // Memory Mapped IO range
#ifdef ENABLE_DMA
    PUCHAR          m_pDMABuf;              // DMA circular buffer
    DWORD           m_dwDMABufAvail;        // Total available buffer in dwords
    DWORD           m_dwDMAPrevRead;        // Previous read ptr offset in
#endif // ENABLE_DMA

#ifdef ENABLE_INTR
    void            IntrThreadProc();       // IST
#endif // ENABLE_INTR

    // config.cpp
                    S3Virge(void);
    virtual SCODE   SetMode(int modeNo, HPALETTE *pPalette);
    void            SetClock(ModeInit *pMode);
    void            SetMemoryMapRegisters(void);
#ifdef ENABLE_OVERLAY
    void            TurnOnStreamsProcessor(void);
#endif // ENABLE_OVERLAY
#ifdef ENABLE_DMA
    void            TurnOnDMA(void);
#endif // ENABLE_DMA

    // cursor.cpp
    virtual SCODE   SetPointerShape(GPESurf *pMask, GPESurf *pColorSurf, 
                                    int xHot, int yHot, int cx, int cy);
    virtual SCODE   MovePointer(int x, int y);
    void            SetCursorColors(ULONG foreground, ULONG background);

    // misc.cpp
    virtual SCODE   GetModeInfo(GPEMode *pMode, int modeNo);
    virtual SCODE   GetModeInfoEx(GPEModeEx *pModeEx, int modeNo);
    virtual int     NumModes(void);
    void            Unlock(void);
    void            Lock(void);
    virtual void    WaitForNotBusy(void);
    virtual int     IsBusy(void);
    void            WaitForFIFO(UINT numParameters);
    void            SelectSolidColor(ULONG color);
    virtual void    GetPhysicalVideoMemory(PULONG pPhysicalMemoryBase, 
                                           PULONG pVideoMemorySize);
    virtual ULONG   DrvEscape(
                        SURFOBJ *pso,
                        ULONG    iEsc,
                        ULONG    cjIn,
                        PVOID    pvIn,
                        ULONG    cjOut,
                        PVOID    pvOut);
    virtual DWORD   GetFreeVideoMemory() 
                    { 
                        return (m_pVideoMemoryHeap?
                                (m_pVideoMemoryHeap->Available()):0); 
                    }

    DWORD           WaitForVerticalBlank(LPDDHAL_WAITFORVERTICALBLANKDATA pd);
    
    // surf.cpp
    virtual SCODE   AllocSurface(GPESurf **ppSurf, int width, int height, 
                                 EGPEFormat format, int surfaceFlags);
    virtual SCODE   AllocSurface(DDGPESurf **ppSurf, int width, int height, 
                                 EGPEFormat format, EDDGPEPixelFormat pxlFormat,
                                 int surfaceFlags);
    virtual int     SurfaceBusyFlipping(GPESurf *pSurf);
    virtual int     FlipInProgress(void);
    virtual void    SetVisibleSurface(GPESurf *pSurf);
    virtual void    SetVisibleSurface(GPESurf *pSurf, BOOL bWaitForVBlank);

    HRESULT         UpdateFlipStatus(ULONG ulVidMem);

    DWORD           Lock(LPDDHAL_LOCKDATA pd);
    DWORD           Flip(LPDDHAL_FLIPDATA pd);
    DWORD           Blt(LPDDHAL_BLTDATA pd);
    DWORD           GetBltStatus(LPDDHAL_GETBLTSTATUSDATA pd);
    DWORD           GetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA pd);
#ifdef ENABLE_OVERLAY
    DWORD           UpdateOverlay(LPDDHAL_UPDATEOVERLAYDATA pd);
    DWORD           SetOverlayPosition(LPDDHAL_SETOVERLAYPOSITIONDATA pd);
    DWORD           ColorControl(LPDDHAL_COLORCONTROLDATA pd);
#endif // ENABLE_OVERLAY

    // Line.cpp:
    virtual SCODE   Line(GPELineParms *pLineParms, EGPEPhase phase);
    SCODE           AcceleratedSolidLine( GPELineParms *pLineParms);
    void            LineSegment(int xStart, int yStart, 
                                int xEnd, int yEnd, UCHAR color);

    // Blt.cpp:
    virtual SCODE   BltPrepare(GPEBltParms *pBltParms);
    virtual SCODE   BltComplete(GPEBltParms *pBltParms);
    SCODE           AcceleratedFillRect(GPEBltParms *pBltParms);
    SCODE           AcceleratedSrcCopyBlt(GPEBltParms *pBltParms);
    SCODE           AcceleratedPatCopyBlt(GPEBltParms *pBltParms);
    SCODE           AcceleratedImagePatCopyBlt(GPEBltParms *pBltParms);
    SCODE           AcceleratedDstCopyBlt(GPEBltParms *pBltParms);
    SCODE           AcceleratedImageSrcCopyBlt(GPEBltParms *pBltParms);

	//andreip - from MartSh emulated AA text
	virtual ULONG   GetGraphicsCaps()  { return GCAPS_GRAY16; }
	SCODE           EmulatedBltAlphaText16_555( GPEBltParms *pParms );
};


class S3VirgeSurf : public DDGPESurf
{
private:
    SurfaceHeap     *m_pHeap;
public:
                    // video memory surface
                    S3VirgeSurf(int width, int height, ULONG offset, 
                                PVOID pBits, int stride, EGPEFormat format, 
                                SurfaceHeap *pHeap);
                    // video memory surface
                    S3VirgeSurf(int width, int height, ULONG offset,
                                PVOID pBits, int stride, EGPEFormat format,
                                EDDGPEPixelFormat pixelFormat, 
                                SurfaceHeap *pHeap);
                    // system memory surface
                    S3VirgeSurf(int width, int height, int stride,
                                EGPEFormat format, 
                                EDDGPEPixelFormat pixelFormat);

    virtual         ~S3VirgeSurf(void);
};

#define BLOCK_64K                       0x10000     // Block of 64 K

#define S3_VENDOR_ID            0x5333  // S3 Inc
#define CHIPID_VIRGE            0x5631  // Virge ChipID        CR2D + CR2E
#define CHIPID_VIRGE_GX_DX      0x8A01  // Virge GX/DX ChipID  CR2D + CR2E

// Useful defines and macros

#define P_Format_Shift      24
#define P_RGB8              (0L << P_Format_Shift)
#define P_RGB5551           (3L << P_Format_Shift)
#define P_RGB565            (5L << P_Format_Shift)
#define P_RGB8880           (6L << P_Format_Shift)
#define P_RGB8888           (7L << P_Format_Shift)

#define S_Format_Shift      24
#define S_YCrCb422          (1L << S_Format_Shift)
#define S_YUV422            (2L << S_Format_Shift)
#define S_RGB5551           (3L << S_Format_Shift)
#define S_YUV211            (4L << S_Format_Shift)
#define S_RGB565            (5L << S_Format_Shift)
#define S_RGB8880           (6L << S_Format_Shift)
#define S_RGB8888           (7L << S_Format_Shift)
#define S_UYVY              (9L << S_Format_Shift)

#define S_Filter_Shift      28
#define S_1x                (0L << S_Filter_Shift)
#define S_Upto2x            (1L << S_Filter_Shift)
#define S_2xTo4x            (2L << S_Filter_Shift)
#define S_Beyond4x          (3L << S_Filter_Shift)

#define HDDA_Shift   0
#define HDDA_Mask    (((1L << 13)-1) << HDDA_Shift)
#define VDDA_Shift   0
#define VDDA_Mask    (((1L << 13) - 1) << VDDA_Shift)
#define HK1_Shift    0
#define HK1_Mask     (((1L << 11) - 1) << HK1_Shift)
#define HK2_Shift    16
#define HK2_Mask     (((1L << 12) - 1) << HK2_Shift)
#define VK1_Shift    0
#define VK1_Mask     (((1L << 12) - 1) << VK1_Shift)
#define VK2_Shift    0
#define VK2_Mask     (((1L << 13) - 1) << VK2_Shift)
#define H_Shift      0
#define H_Mask       (0x07ffL << H_Shift)
#define W_Shift      16
#define W_Mask       (0x07ffL << W_Shift)
#define Y_Shift      0
#define Y_Mask       (0x07ffL << Y_Shift)
#define X_Shift      16
#define X_Mask       (0x07ffL << X_Shift)

#define HDDA(w0,w1)  (((2*(w0-1)-(w1-1)) << HDDA_Shift) & HDDA_Mask )
#define VDDA(h1)     (((1-h1)            << VDDA_Shift) & VDDA_Mask )
#define HK1(w0)      (((w0 - 1)          << HK1_Shift)  & HK1_Mask  )
#define HK2(w0,w1)   (((w0 - w1)         << HK2_Shift)  & HK2_Mask  )
#define HK1K2(w0,w1) (HK1(w0) | HK2(w0, w1))
#define VK1(h0)      (((h0 - 1)          << VK1_Shift)  & VK1_Mask  )
#define VK2(h0,h1)   (((h0 - h1)         << VK2_Shift)  & VK2_Mask  )
#define XY(x,y)      ((((x+1)<<X_Shift)&X_Mask) | (((y+1)<<Y_Shift)&Y_Mask))
#define WH(w,h)      ((((w-1)<<W_Shift)&W_Mask) | (((h)<<H_Shift)&H_Mask))

#define RGB15to32(c) \
   (((c & 0x7c00) << 9) | \
    ((c & 0x03e0) << 6) | \
    ((c & 0x001f) << 3))

#define RGB16to32(c) \
   (((c & 0xf800) << 8) | \
    ((c & 0x07e0) << 5) | \
    ((c & 0x001f) << 3))

#define Compare_Shift       24
#define CompareBits7        (0L  << Compare_Shift)
#define CompareBits6t7      (1L  << Compare_Shift)
#define CompareBits5t7      (2L  << Compare_Shift)
#define CompareBits4t7      (3L  << Compare_Shift)
#define CompareBits3t7      (4L  << Compare_Shift)
#define CompareBits2t7      (5L  << Compare_Shift)
#define CompareBits1t7      (6L  << Compare_Shift)
#define CompareBits0t7      (7L  << Compare_Shift)
#define KeyFrom_Shift       28
#define KeyFromStream       (0L << KeyFrom_Shift)
#define KeyFromCompare      (1L << KeyFrom_Shift)

#define Ks_Shift            2
#define Ks_Mask             (7L << Ks_Shift)
#define Kp_Shift            10
#define Kp_Mask             (7L << Kp_Shift)
#define Compose_Shift       24
#define SOnP                (0L << Compose_Shift)
#define POnS                (1L << Compose_Shift)
#define Dissolve            (2L << Compose_Shift)
#define Fade                (3L << Compose_Shift)
#define KeyOnP              (5L << Compose_Shift)
#define KeyOnS              (6L << Compose_Shift)

// Direct access non-standard VGA/S3Virge IO mapped registers

                            //Sleep mode 
#define reg_SETUP           (PORT_RW_8 m_VGAIOPort[PortId_3C3]) 

// Indexed registers and their aliases
#define reg_MCLK_LO         reg_SR[0x10]    //  R/W MCLK Low PLL divide
#define reg_MCLK_HI         reg_SR[0x11]    //  R/W MCLK High PLL divide
#define reg_DCLK_LO         reg_SR[0x12]    //  R/W DCLK Low PLL divide
#define reg_DCLK_HI         reg_SR[0x13]    //  R/W DCLK High PLL divide
#define reg_CHIP_ID_HI      reg_CR[0x2D]    //  R   Chip ID high 
                                            //  (=upper byte PCI devid)
#define reg_CHIP_ID_LO      reg_CR[0x2E]    //  R   Chip ID low
#define reg_CHIP_REVISION   reg_CR[0x2F]    //  R   Chip Revision Level

#define reg_BKWD_1          reg_CR[0x32]    //  Interrupt Enable
#define ENABLE_ALL_INTERRUPTS           0x10

#define reg_BKWD_3          reg_CR[0x34]    //  R/W Snoop control & CR3B enable
#define reg_LOCK_1          reg_CR[0x38]    //  R/W Write 0x48 here to unlock 
                                            //  S3 Reg set
#define reg_LOCK_2          reg_CR[0x39]    //  R/W Write 0xA0 here to enable 
                                            //  CR68 & CR36[7:2] 
#define reg_DT_EX_POS       reg_CR[0x3B]    //  R/W Start display FIFO fetch
                                            //  (= -5+CR0)
#define reg_VERT_CT         reg_CR[0x3E]    //  vertical counter
#define reg_SYS_CNFG        reg_CR[0x40]    //  R/W Enable enhanced 
                                            //  reg access ( x2E8 )
#define reg_HGC_MODE        reg_CR[0x45]    //  R/W Graphics cursor enable
#define reg_HWGC_ORGX_H     reg_CR[0x46]    //  R/W Graphics cursor origin 
                                            //  X high
#define reg_HWGC_ORGX_L     reg_CR[0x47]    //  R/W Graphics cursor origin X low
#define reg_HWGC_ORGY_H     reg_CR[0x48]    //  R/W Graphics cursor origin 
                                            //  Y high
#define reg_HWGC_ORGY_L     reg_CR[0x49]    //  R/W Graphics cursor origin Y low
#define reg_HWGC_FGSTK      reg_CR[0x4A]    //  R/W Foreground color 
                                            //  3-entry stack
#define reg_HWGC_BGSTK      reg_CR[0x4B]    //  R/W Background color 
                                            //  3-entry stack
#define reg_HWGC_STA_H      reg_CR[0x4C]    //  R/W Cursor storage 
                                            //  start addr high
#define reg_HWGC_STA_L      reg_CR[0x4D]    //  R/W Cursor storage 
                                            //  start addr low
#define reg_HWGC_DX         reg_CR[0x4E]    //  R/W Offset (for cursors off 
                                            //  left of screen)
#define reg_HWGC_DY         reg_CR[0x4F]    //  R/W Offset (for cursors off 
                                            //  top of screen)
#define reg_EXT_MCTL_1      reg_CR[0x53]    //  R/W Enable write-perbit, enable
                                            //  MMIO access, swap nibbles
#define reg_EXT_MCTL_2      reg_CR[0x54]    //  R/W Number of mem access 
                                            //  cyc avail
#define reg_LAW_POS_H       reg_CR[0x59]    //  R/W LAW address Bits 31:24
#define reg_LAW_POS_L       reg_CR[0x5A]    //  R/W LAW address Bits 23:16
#define reg_EXT_MCTL_3      reg_CR[0x60]    //  R/W # 4 or 8 byte units / 
                                            //  access grant
#define reg_EXT_MISC_1      reg_CR[0x66]    //  R/W interrupt PCI burst or 
                                            //  tri-state pix add

#define reg_EXT_MISC_2      reg_CR[0x67]    //  R/W Enable stream processor and 
                                            //  set DAC color mode
#define ENABLE_STREAM_PROCESSOR         0x0C

#define reg_EXT_SCTL_3      reg_CR[0x69]    //  R/W Bits [19:16] of 
                                            //  display start address
#define reg_PCI_VENDOR      reg_CR[0x81]    //  PCI Vendor ID
#define reg_PCI_DEVICE      reg_CR[0x82]    //  PCI Device ID


// S3 ViRGE Accelerated Registers in MMIO space.

/***
  PCI Registers
  ***/
#define reg_MM8000  (MM_RO_16 m_pMMIO[0x8000])  // PCI Configuration Space 
                                                // Registers
#define reg_MM8002  (MM_RO_16 m_pMMIO[0x8002])  // PCI Configuration Space 
                                                // Registers
/***
  Misc Registers
  ***/
#define reg_MM8504RO (MM_RO_32 m_pMMIO[0x8504]) // Subsystem Status Register
// Interrupt status (MM8504)
#define INTR_STATUS_V_SYNC              0x00000001
#define INTR_STATUS_S3D_DONE            0x00000002
#define INTR_STATUS_CMD_FIFO_OF         0x00000004
#define INTR_STATUS_CMD_FIFO_EMPTY      0x00000008
#define INTR_STATUS_HOST_DMA_DONE       0x00000010
#define INTR_STATUS_CMD_DMA_DONE        0x00000020
#define INTR_STATUS_S3D_FIFO_EMPTY      0x00000040

#define reg_MM8504WO (MM_WO_32 m_pMMIO[0x8504]) // Subsystem Control Register
// Enable interrupts
#define ENABLE_V_SYNC                   0x00000100
#define ENABLE_S3D_DONE                 0x00000200
#define ENABLE_CMD_FIFO_OF              0x00000400
#define ENABLE_CMD_FIFO_EMPTY           0x00000800
#define ENABLE_HOST_DMA_DONE            0x00000080
#define ENABLE_CMD_DMA_DONE             0x00001000
#define ENABLE_S3D_FIFO_EMPTY           0x00002000

// Clear interrupt status (MM8504)
#define CLEAR_V_SYNC                    0x00000001
#define CLEAR_S3D_DONE                  0x00000002
#define CLEAR_CMD_FIFO_OF               0x00000004
#define CLEAR_CMD_FIFO_EMPTY            0x00000008
#define CLEAR_HOST_DMA_DONE             0x00000010
#define CLEAR_CMD_DMA_DONE              0x00000020
#define CLEAR_S3D_FIFO_EMPTY            0x00000040

// Enable and clear all 7 interrupts above
#define ENABLE_AND_CLEAR_ALL_7          0x0000FFFF

// Enable all 7 interrupts above, keep their status unchanged
#define ENABLE_ALL_7                    0x0000FF80

#define reg_MM850C  (MM_RW_32 m_pMMIO[0x850C])  // Subsystem Control Register

/***
  S3D Registers
  ***/
#define reg_MMA100  (MM_RW_32 m_pMMIO[0xA100])  // First Pattern
#define reg_MMA4D4  (MM_RW_32 m_pMMIO[0xA4D4])  // BLT Source Base Address
#define reg_MMA4D8  (MM_RW_32 m_pMMIO[0xA4D8])  // BLT Destination Base Address
#define reg_MMA4DC  (MM_RW_32 m_pMMIO[0xA4DC])  // BLT Left/Right Clipping
#define reg_MMA4E0  (MM_RW_32 m_pMMIO[0xA4E0])  // BLT Top/Bottom Clipping
#define reg_MMA4E4  (MM_RW_32 m_pMMIO[0xA4E4])  // BLT Dest/Source Stride
#define reg_MMA4E8  (MM_RW_32 m_pMMIO[0xA4E8])  // BLT Mono Pattern 0
#define reg_MMA4EC  (MM_RW_32 m_pMMIO[0xA4EC])  // BLT Mono Pattern 1
#define reg_MMA4F0  (MM_RW_32 m_pMMIO[0xA4F0])  // BLT Mono Pattern Bkgrd Color
#define reg_MMA4F4  (MM_RW_32 m_pMMIO[0xA4F4])  // BLT Mono Pattern Fgrd Color 
#define reg_MMA4F8  (MM_RW_32 m_pMMIO[0xA4F8])  // Source Bkgrd Color
#define reg_MMA4FC  (MM_RW_32 m_pMMIO[0xA4FC])  // Source Fgrd Color

#define reg_MMA500  (MM_RW_32 m_pMMIO[0xA500])  // BLT Command Set 
#define reg_MMA900  (MM_RW_32 m_pMMIO[0xA900] ) // 2D Line Command Set
// Bit 31
#define MASK2D                          0x00000000
#define MASK3D                          0x80000000
// Bits 30-27
#define CMD_BITBLT                      0x00000000
#define CMD_RECTANGLE_FILL              0x10000000
#define CMD_LINE_DRAW                   0x18000000
#define CMD_POLYGON_FILL                0x28000000
#define CMD_NOP                         0x78000000
// Bit 26
#define MASK_Y_BLT                      0x04000000
#define PLUS_Y                          0x04000000
#define Y_BLT_SHIFT                     26
// Bit 25
#define MASK_X_BLT                      0x02000000
#define PLUS_X                          0x02000000
#define X_BLT_SHIFT                     25
// Bits 24-17   256 Raster Operations (ROPs)
#define MASK_ROPS                       0x01FE0000
#define ROPS_SHIFT                      17
// Bits 16-14 Reserved
// Bits 13-12
#define START_AT_FIRST_BYTE             0x00000000
#define START_AT_SECOND_BYTE            0x00001000
#define START_AT_THIRD_BYTE             0x00002000
#define START_AT_FOURTH_BYTE            0x00003000
// Bits 11-10
#define BYTE_ALIGNED_TRANSFER           0x00000000
#define WORD_ALIGNED_TRANSFER           0x00000400
#define DWORD_ALIGNED_TRANSFER          0x00000800
// Bit 9
#define TRANSPARENT_ENABLE              0x00000200
// Bit 8
#define MONO_PATTERN                    0x00000100
// Bit 7
#define SOURCE_FROM_SYSTEM              0x00000080
// Bit 6
#define MONO_SOURCE                     0x00000040
// Bit 5
#define DRAW_ENABLE                     0x00000020
// Bits 4-2
#define DESTINATION_FORMAT_8BIT         0x00000000
#define DESTINATION_FORMAT_16BIT        0x00000004
#define DESTINATION_FORMAT_24BIT        0x00000008
// Bit 1
#define CLIPPING_ENABLED                0x00000002
// Bit 0
#define AUTOEXECUTE_ENABLED             0x00000001

#define reg_MMA504  (MM_RW_32 m_pMMIO[0xA504])  // Rectangle Width/Height
#define reg_MMA508  (MM_RW_32 m_pMMIO[0xA508])  // Rectangle Source XY
#define reg_MMA50C  (MM_RW_32 m_pMMIO[0xA50C])  // Rectangle Dest XY

#define reg_MMA8D8  (MM_RW_32 m_pMMIO[0xA8D8])  // 2D Line Dest Base Address
#define reg_MMA8DC  (MM_RW_32 m_pMMIO[0xA8DC])  // 2D Line Clipping Left/Right
#define reg_MMA8E0  (MM_RW_32 m_pMMIO[0xA8E0])  // 2D Line Clipping Top/Bottom
#define reg_MMA8F4  (MM_RW_32 m_pMMIO[0xA8F4])  // 2D Line Mono Pattern Fg Color

#define reg_MMA96C  (MM_RW_32 m_pMMIO[0xA96C])  // Line Draw Endpoints Register
#define reg_MMA970  (MM_RW_32 m_pMMIO[0xA970])  // Line Draw X Delta Register
#define reg_MMA974  (MM_RW_32 m_pMMIO[0xA974])  // Line Draw X Start Register
#define reg_MMA978  (MM_RW_32 m_pMMIO[0xA978])  // Line Draw Y Start Register
#define reg_MMA97C  (MM_RW_32 m_pMMIO[0xA97C])  // Line Draw Y Count Register

/***
  LPB Registers
  ***/
#define reg_MMFF00  (MM_RW_32 m_pMMIO[0xFF00])  // LPB Mode
#define reg_MMFF04  (MM_RW_32 m_pMMIO[0xFF04])  // LPB FIFO Status
#define reg_MMFF08  (MM_RW_32 m_pMMIO[0xFF08])  // LPB Interrupt Flags
#define reg_MMFF0C  (MM_RW_32 m_pMMIO[0xFF0C])  // LPB Frame Buffer Address 0
#define reg_MMFF10  (MM_RW_32 m_pMMIO[0xFF10])  // LPB Frame Buffer Address 1
#define reg_MMFF14  (MM_RW_32 m_pMMIO[0xFF14])  // LPB Direct Read/Write Address
#define reg_MMFF18  (MM_RW_32 m_pMMIO[0xFF18])  // LPB Direct Read/Write Data
#define reg_MMFF1C  (MM_RW_32 m_pMMIO[0xFF1C])  // LPB General Purpose I/O Port
#define reg_MMFF20  (MM_RW_32 m_pMMIO[0xFF20])  // Serial Port
#define reg_MMFF24  (MM_RW_32 m_pMMIO[0xFF24])  // LPB Video Input Window Size
#define reg_MMFF28  (MM_RW_32 m_pMMIO[0xFF28])  // LPB Video Data Offsets
#define reg_MMFF2C  (MM_RW_32 m_pMMIO[0xFF2C])  // LPB Hori. Decimation Control
#define reg_MMFF30  (MM_RW_32 m_pMMIO[0xFF30])  // LPB Vert. Decimation Control
#define reg_MMFF34  (MM_RW_32 m_pMMIO[0xFF34])  // LPB Line Stride
#define reg_MMFF40  (MM_RW_32 m_pMMIO[0xFF40])  // LPB Output FIFO

/***
  Streams Processor Registers
  ***/
#define reg_MM8180  (MM_RW_32 m_pMMIO[0x8180])  // Primary Stream Control
#define reg_MM8184  (MM_RW_32 m_pMMIO[0x8184])  // Color/Chroma Key Control
#define reg_MM8190  (MM_RW_32 m_pMMIO[0x8190])  // Secondary Stream Control
#define reg_MM8194  (MM_RW_32 m_pMMIO[0x8194])  // Chroma Key Upper Bound
#define reg_MM8198  (MM_RW_32 m_pMMIO[0x8198])  // Secondary Stream Stretch
                                                // and Filter Constants
#define reg_MM819C  (MM_RW_32 m_pMMIO[0x819C])  // Color Adjustment Register
#define reg_MM81A0  (MM_RW_32 m_pMMIO[0x81A0])  // Blend Control
#define reg_MM81C0  (MM_RW_32 m_pMMIO[0x81C0])  // Primary Stream FB Address 0
#define reg_MM81C4  (MM_RW_32 m_pMMIO[0x81C4])  // Primary Stream FB Address 1
#define reg_MM81C8  (MM_RW_32 m_pMMIO[0x81C8])  // Primary Stream Stride
#define reg_MM81CC  (MM_RW_32 m_pMMIO[0x81CC])  // Double Buffer/LPB Support
#define reg_MM81D0  (MM_RW_32 m_pMMIO[0x81D0])  // Secondary Stream FB Address 0
#define reg_MM81D4  (MM_RW_32 m_pMMIO[0x81D4])  // Secondary Stream FB Address 1
#define reg_MM81D8  (MM_RW_32 m_pMMIO[0x81D8])  // Secondary Stream Stride
#define reg_MM81DC  (MM_RW_32 m_pMMIO[0x81DC])  // Opaque Overlay Control
#define reg_MM81E0  (MM_RW_32 m_pMMIO[0x81E0])  // K1 Vertical Scale Factor
#define reg_MM81E4  (MM_RW_32 m_pMMIO[0x81E4])  // K2 Vertical Scale Factor
#define reg_MM81E8  (MM_RW_32 m_pMMIO[0x81E8])  // DDA Vert. Acc. Initial Value
#define reg_MM81F0  (MM_RW_32 m_pMMIO[0x81F0])  // Pri. Window Start Coordinates
#define reg_MM81F4  (MM_RW_32 m_pMMIO[0x81F4])  // Primary Window Size
#define reg_MM81F8  (MM_RW_32 m_pMMIO[0x81F8])  // Sec. Window Start Coordinates
#define reg_MM81FC  (MM_RW_32 m_pMMIO[0x81FC])  // Secondary Window Size

#define reg_MM8200  (MM_RW_32 m_pMMIO[0x8200])  // Stream FIFO and RAS Controls
#define reg_MM8204  (MM_RW_32 m_pMMIO[0x8204])  // MIU Controls
#define reg_MM8208  (MM_RW_32 m_pMMIO[0x8208])  // Streams Timeout
#define reg_MM820C  (MM_RW_32 m_pMMIO[0x820C])  // Misc Timeout

/***
  DMA Registers
  ***/
#define reg_MM8220  (MM_RW_32 m_pMMIO[0x8220])  // DMA Read Base Address
#define reg_MM8224  (MM_RW_32 m_pMMIO[0x8224])  // DMA Read Stride/Width
#define reg_MM8580  (MM_RW_32 m_pMMIO[0x8580])  // Vid DMA Starting Sys Mem Add
#define reg_MM8584  (MM_RW_32 m_pMMIO[0x8584])  // Video DMA Transfer Length
#define reg_MM8588  (MM_RW_32 m_pMMIO[0x8588])  // Video DMA Transfer Enable
#define reg_MM8590  (MM_RW_32 m_pMMIO[0x8590])  // Cmd DMA Base Address
#define DMA_BUFFER_SIZE_64K     0x00000002
#define reg_MM8594  (MM_RW_32 m_pMMIO[0x8594])  // Cmd DMA Write Pointer
#define DMA_WRITE_PTR_UPD       0x00010000
#define reg_MM8598  (MM_RW_32 m_pMMIO[0x8598])  // Cmd DMA Read Pointer
#define reg_MM859C  (MM_RW_32 m_pMMIO[0x859C])  // Cmd DMA Enable
#define COMMAND_DMA_ENABLE      0x00000001

#define DMA_IMAGE_DATA          0x80000000

// color control defines
#define PI 3.1415926

#define DD_MIN_BRIGHT     0L
#define DD_MAX_BRIGHT 10000L
#define DD_DEF_BRIGHT   750L
#define HW_MIN_BRIGHT  -128L  /* SBBBBBBB */
#define HW_MAX_BRIGHT   127L
#define HW_DEF_BRIGHT     0L

#define DD_MIN_CONT        0L
#define DD_MAX_CONT    20000L
#define DD_DEF_CONT    10000L
#define HW_MIN_CONT        0L  /* Multiplied C.CCCC range 0..1.9375 by 16 */
#define HW_MAX_CONT       31L  /* in order to shift decimal right */
#define HW_DEF_CONT       16L  /* by 4 binary places */

#define DD_MIN_HUE      -180L  /* Hue is in degrees with cos ranging -1..1 */
#define DD_MAX_HUE       180L
#define DD_DEF_HUE         0L
#define HW_MIN_HUE      -180L
#define HW_MAX_HUE       180L
#define HW_DEF_HUE         0L

#define DD_MIN_SAT         0L
#define DD_MAX_SAT     20000L
#define DD_DEF_SAT     10000L
#define HW_MIN_SAT       -16L  /* Multiplied SF.FFF range -2..1.875 by 8 */
#define HW_MAX_SAT        15L  /* in order to shift decimal right */
#define HW_DEF_SAT         8L  /* by 3 binary places */

#endif __S3Virge_H__

