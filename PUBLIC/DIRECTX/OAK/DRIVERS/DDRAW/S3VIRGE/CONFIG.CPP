/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

Module Name:    config.cpp

Abstract:       initial setup of the S3 ViRGE chip, 
                VGA/SVGA/acceleration registers
  
Functions:


Notes: 
 This driver ignores the passed in resolution from "loadcepc" and 
 uses defines from the sources file.
 
 The driver is derived from the S3 Trio64 driver.

 MMIO of the S3 ViRGE chip is used to access all accelerated registers.
--*/

#include "precomp.h"
#include "nkintr.h"
#include "palette.h"        // for 8Bpp we use the natural palette

#ifdef ENABLE_INTR

#if defined(CEPC)
_inline
DWORD
MapIrq2SysIntr(DWORD _Irq)
{
    if( _Irq<=15 )
        return ( SYSINTR_FIRMWARE + _Irq );
    else
        return (0xFFFFFFFF);
}
#else
_inline
DWORD
MapIrq2SysIntr(DWORD _Irq)
{
    DebugBreak();
    return (0xFFFFFFFF);
}
#endif

#endif // ENABLE_INTR

INSTANTIATE_PALETTE

/* Start with Errors, warnings, and temporary messages */
INSTANTIATE_GPE_ZONES(0x0003,"DDI Driver","unused1","unused2")   

// Workaround for problems exporting from .lib
BOOL APIENTRY 
GPEEnableDriver(
    ULONG iEngineVersion, 
    ULONG cj, 
    DRVENABLEDATA *pded, 
    PENGCALLBACKS pEngCallbacks
    );

BOOL APIENTRY 
DrvEnableDriver(
    ULONG iEngineVersion, 
    ULONG cj, 
    DRVENABLEDATA *pded, 
    PENGCALLBACKS pEngCallbacks
    )
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS,(TEXT("GPEEnableDriver()\r\n")));
    
    return GPEEnableDriver(iEngineVersion, cj, pded, pEngCallbacks);
}


/*
// This driver currently ignores the passed in resolution from "loadcepc". 
//
// 0    320x200x256     loadcepc default mode, should work with any video card 
//                      with 64K memory
// 1    480x240x256     non-standard VGA resolution, run in a 640x480 window.
// 2    640x480x256
// 3    800x600x256
// 4    1024x768x256

UCHAR loadcepcVMode;

void 
GetRequestedResolution(void)
{
    PBYTE   args;
    DWORD   oldMode;

    oldMode = SetKMode(TRUE);                   // Enter Kernel mode

    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("checking on requested Display Mode\n\r")));

    args = *(PBYTE *) 0x801FFFFC;

    // Get the display resolution arguments (if any) passed down from loadcepc
    if (args != 0)
    {
        args = (PBYTE)((ULONG) args | 0x80000000);
        loadcepcVMode = args[0];
    }
    else
    {
        loadcepcVMode = LOADCEPC_640x480x256;   // default to 640x480
    }

    SetKMode(oldMode);                          // exit Kernel mode

    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("requested display Mode = %d\n\r"), loadcepcVMode));
}
*/

// Main entry point for a GPE-compliant driver
GPE *
GetGPE()
{
    static GPE *pGPE = (GPE *)NULL;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GetGPE\n\r")));

    // GetRequestedResolution();

    if (!pGPE)
        pGPE = new S3Virge();

    return pGPE;
}


#ifdef ENABLE_INTR

void
StartIntrThread(S3Virge * pS3Virge)
{
    pS3Virge->IntrThreadProc();
}

void
S3Virge::IntrThreadProc()
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("IntrThreadProc\n\r")));

    while (TRUE)
    {
        // wait for kernel to signal the main intr event
        WaitForSingleObject(m_hIntrEvent, INFINITE);

        // keep track whether there are new interrupts
        BOOL bMoreIntr;

        do
        {
            bMoreIntr = FALSE;

            // get the interrupt status
            DWORD dwIntrStatus = reg_MM8504RO;
        
            // signal individual event(s)
            if (dwIntrStatus & INTR_STATUS_V_SYNC) 
            {
                if (m_hCtl1VSync != NULL) 
                {
                    SetEvent(m_hCtl1VSync);
                    // reset m_hCtl1VSync
                    m_hCtl1VSync = NULL;
                }
                bMoreIntr = TRUE;
                // clear this interrupt
                reg_MM8504WO = CLEAR_V_SYNC | m_dwIntrSetting;
            }
            if (dwIntrStatus & INTR_STATUS_S3D_DONE) 
            {
                SetEvent(m_hS3dDone);
                bMoreIntr = TRUE;
                // clear this interrupt
                reg_MM8504WO = CLEAR_S3D_DONE | m_dwIntrSetting;
            }
            if (dwIntrStatus & INTR_STATUS_CMD_FIFO_OF)
            {
                SetEvent(m_hCmdFifoOF);
                bMoreIntr = TRUE;
                // clear this interrupt
                reg_MM8504WO = CLEAR_CMD_FIFO_OF | m_dwIntrSetting;
            }
            if (dwIntrStatus & INTR_STATUS_CMD_FIFO_EMPTY)
            {
                SetEvent(m_hCmdFifoEmpty);
                bMoreIntr = TRUE;
                // clear this interrupt
                reg_MM8504WO = CLEAR_CMD_FIFO_EMPTY | m_dwIntrSetting;
            }
            if (dwIntrStatus & INTR_STATUS_HOST_DMA_DONE)
            {
                SetEvent(m_hHostDmaDone);
                bMoreIntr = TRUE;
                // clear this interrupt
                reg_MM8504WO = CLEAR_HOST_DMA_DONE | m_dwIntrSetting;
            }
            if (dwIntrStatus & INTR_STATUS_CMD_DMA_DONE)
            {
                SetEvent(m_hCmdDmaDone);
                bMoreIntr = TRUE;
                // clear this interrupt
                reg_MM8504WO = CLEAR_CMD_DMA_DONE | m_dwIntrSetting;
            }
            if (dwIntrStatus & INTR_STATUS_S3D_FIFO_EMPTY)
            {
                SetEvent(m_hS3dFifoEmpty);
                bMoreIntr = TRUE;
                // clear this interrupt
                reg_MM8504WO = CLEAR_S3D_FIFO_EMPTY | m_dwIntrSetting;
            }
        }
        while (bMoreIntr);

        // signal to the kernel that interrupt processing has been completed
        InterruptDone(m_dwIntrId);
    }
    
    // disable the interrupt associated with this ID
    InterruptDisable(m_dwIntrId);
}

#endif // ENABLE_INTR

S3Virge::S3Virge()
{
    ULONG offset;
    UCHAR tmp;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::S3Virge\r\n")));

    reg_SETUP = 0x01;   // Wake the chip           ( = 0x3C3 )
    reg_MISC = 0x03;    // Use color emulation and Enable CPU access 
                        // of video memory ( = 0x3C2 write, 0x3CC read )

    Unlock();           // Unlock Virge registers

    tmp = reg_CHIP_ID_HI;       // Unwind. Compiler optimization??
    m_nChipID = (tmp << 8);     // m_nChipID = (reg_CHIP_ID_HI << 8) | 
                                //              reg_CHIP_ID_LO; fails
    m_nChipID |= reg_CHIP_ID_LO;

    switch (m_nChipID)
    {
    case CHIPID_VIRGE:
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("Found VIRGE chipset\r\n")));
        break;
    case CHIPID_VIRGE_GX_DX:
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("Found VIRGE GX/DX chipset\r\n")));
        break;
    default:
        DEBUGMSG(1, (TEXT("WARNING: unknown CHIP ID=0x%x, ")
             TEXT("attempting to load S3 Virge driver\r\n"),
             m_nChipID));
        break;
    }

    UCHAR checkChip = reg_MISC;     // read from 0x3CC

    // Reserve 64k of virtual address space for MMIO
    m_pMMIO = (PUCHAR) VirtualAlloc(0, BLOCK_64K, MEM_RESERVE, PAGE_NOACCESS);
    if (m_pMMIO == NULL)
    {
        DEBUGMSG(1, (TEXT("Error: no virtual memory left!\r\n")));
        return;
    }

    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("Virtual address of MMIO is 0x%08x\r\n"), m_pMMIO));

    // Map the S3 ViRGE IO space
    ULONG inIoSpace = 0;
    PHYSICAL_ADDRESS ioPhysicalBase = {MMIO_ADDRESS, 0};
    if (!HalTranslateBusAddress(PCIBus, 0, ioPhysicalBase, 
                               &inIoSpace, &ioPhysicalBase))
    {
        DEBUGMSG(1, (TEXT("Error: can't translate bus address!\r\n")));
    }

    BOOL bSuc = VirtualCopy(m_pMMIO, (LPVOID) (ioPhysicalBase.LowPart >> 8), 
                            BLOCK_64K, 
                            PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL);
    if (!bSuc)
    {
        DEBUGMSG(1, (TEXT("Error: failed to map MMIO!\r\n")));
        return;
    }

#ifdef ENABLE_DMA

    // Reserve 64k of virtual address space for DMA circular buffer
    m_pDMABuf = (PUCHAR) VirtualAlloc(0, BLOCK_64K, MEM_RESERVE, PAGE_NOACCESS);
    if (m_pDMABuf == NULL)
    {
        DEBUGMSG(1, (TEXT("Error: no virtual memory left!\r\n")));
        return;
    }

    DEBUGMSG(VIRGE_VERBOSE_MSGS,
        (TEXT("Virtual address of DMA buffer is 0x%08x\r\n"), m_pDMABuf));

    // Map the S3 ViRGE IO space
    bSuc = VirtualCopy(m_pDMABuf, (LPVOID) DMA_BUFFER_ADDRESS,
                       BLOCK_64K, PAGE_READWRITE | PAGE_NOCACHE);
    if (!bSuc)
    {
        DEBUGMSG(1, (TEXT("Error: failed to map DMA circular buffer!\r\n")));
        return;
    }

    m_dwDMABufAvail = 0x10000 / 4;  // 64K bytes or 16K dwords
    m_dwDMAPrevRead = 0;            // read pointer at 0

#endif // ENABLE_DMA

    reg_EXT_MISC_1 = reg_EXT_MISC_1 | 0x01 ;    // CR66 Enable modes.

    //
    // Determine where the LAW will appear in physical memory, 
    // and how big it is
    //
    reg_VRE = reg_VRE & 0x7F;       // Enable writing to CRTC regs

    m_nPos_high = reg_LAW_POS_H;
    m_nPos_low = reg_LAW_POS_L;

    m_nLAWPhysical = (m_nPos_high << 24) | (m_nPos_low << 16);

    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("LAWPhysical is at: 0x%08x\r\n"), m_nLAWPhysical));

    // The S3Virge can have 2 or 4 MB attached.
    // We can't rely upon the memory size reported by the chip strappings 
    // so 4MB is mapped in then the actual size is determined:

    m_nVideoMemorySize = 4 * 1024 * 1024;

    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("S3Virge::S3Virge - VirtualAlloc of LAW\r\n")));
    m_pLAW = (PUCHAR)VirtualAlloc(0, m_nVideoMemorySize, 
                                  MEM_RESERVE, PAGE_NOACCESS);

    // Map in LAW
    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("VirtualAlloc of LAW returns 0x%08x\r\n"), m_pLAW));

    inIoSpace = 0;
    ioPhysicalBase.LowPart = m_nLAWPhysical;
    ioPhysicalBase.HighPart = 0;

    if (!HalTranslateBusAddress(PCIBus, 0, ioPhysicalBase,
                               &inIoSpace, &ioPhysicalBase))
    {
        DEBUGMSG(1, (TEXT("Error: can't translate bus address!\r\n")));
    }

    VirtualCopy(m_pLAW, (LPVOID) (ioPhysicalBase.LowPart >> 8), 
                m_nVideoMemorySize, 
                PAGE_READWRITE | PAGE_NOCACHE | PAGE_PHYSICAL);

    UnlockVGA();        // Unlock Virge VGA registers
    Unlock();           // Unlock Virge registers

    reg_SR[0xA] = 0xC0;     // pin 50 outputs ~RAS1 and 2 MCLK mem writes
                            // req'd for 4M configurations
    reg_SR[0xD] = 0x02;     // selects LPB feature connector configuration
                            // bit 1 should be always 1 until feature
                            // connector operation is enabled
    reg_SR[0x18] = 0x40;    // 1 DCLK LUT write cycle
    reg_CR[0x53] = 0x10;    // enable MMIO
    reg_CR[0x31] = 0x08;    // disable 64K window, enable ENH maps
    reg_CR[0x66] = 0x89;    // enable S3 graphics engine
    reg_CR[0x58] = 0x13;    // enable a 4MB LAW
    reg_SR[0x09] = 0x00;    // Allow both programmed I/O and memory mapped I/Os

    for(offset = m_nVideoMemorySize; offset != 0; offset -= 65536)
    {
        DEBUGMSG(VIRGE_VERBOSE_MSGS, 
            (TEXT("Writing 0x%08x to 0x%08x (offset %d)\r\n"), 
             offset^0xAA551133, ((ULONG) m_pLAW) + offset - 4, offset));
        *(PULONG) ((PUCHAR) m_pLAW + offset - 4) = offset ^ 0xAA551133;
    }

    for (offset = m_nVideoMemorySize; offset != 0; offset -= 65536)
    {
        DEBUGMSG(VIRGE_VERBOSE_MSGS,
            (TEXT("Read from 0x%08x returns 0x%08x\r\n"), 
             ((ULONG) m_pLAW) + offset - 4, 
             *(PULONG) (m_pLAW + offset - 4)));
        if (*(PULONG) (m_pLAW + offset - 4) == (offset ^ 0xAA551133))
            break;
    }

    m_nVideoMemorySize = offset;
    
    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("Actual memory detected: %d bytes\r\n"), m_nVideoMemorySize));

    // initialize the flip record
    memset(&m_sFlipRecord, 0, sizeof(FLIPRECORD));

#ifdef ENABLE_INTR

    // translate line# into interrupt ID
    m_dwIntrId = MapIrq2SysIntr(m_nIntrLine);
    if (m_dwIntrId == 0xFFFFFFFF)
    {
        DEBUGMSG(1, (TEXT("Failed to map the IRQ!!!\r\n")));
        return;
    }

    // create a main interrupt event
    m_hIntrEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hIntrEvent == NULL)
    {
        DEBUGMSG(1, (TEXT("Can't create main interrupt event!!!\r\n")));
        return;
    }

    // create individual interrupt events
    m_hCtl1VSync = NULL;        // the v-sync event should be passed in
    m_hS3dDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hCmdFifoOF = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hCmdFifoEmpty = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hHostDmaDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hCmdDmaDone = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hS3dFifoEmpty = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (m_hS3dDone == NULL || m_hCmdFifoOF == NULL || 
        m_hCmdFifoEmpty == NULL || m_hHostDmaDone == NULL || 
        m_hCmdDmaDone == NULL || m_hS3dFifoEmpty == NULL)
    {
        DEBUGMSG(1, 
            (TEXT("Can't create individual interrupt events!!!\r\n")));
        return;
    }

    m_hIntrThread = CreateThread(NULL, 0, 
                                 (LPTHREAD_START_ROUTINE)StartIntrThread, 
                                 this, 0, NULL);
    if (m_hIntrThread == NULL)
    {
        DEBUGMSG(1, (TEXT("Can't create interrupt thread!!!\r\n")));
        return;
    }

    // set the priority to time critical so that it gets scheduled
    // without any delay
    SetThreadPriority(m_hIntrThread, THREAD_PRIORITY_TIME_CRITICAL);

    // set hardware registers to enable and clear interrupts
    BYTE bOld = reg_BKWD_1;
    reg_BKWD_1 = bOld | ENABLE_ALL_INTERRUPTS;
    // only enable v-sync at the moment
    m_dwIntrSetting = ENABLE_V_SYNC;
    reg_MM8504WO = m_dwIntrSetting | CLEAR_V_SYNC;

    // intialize the hardware interrupt with the kernel
    bSuc = InterruptInitialize(m_dwIntrId, m_hIntrEvent, NULL, NULL);
    if (!bSuc)
    {
        DEBUGMSG(1, (TEXT("Failed to initialize the interrupt!!!\r\n")));
        return;
    }

#endif // ENABLE_INTR

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::S3Virge done!\r\n")));
}

void
S3Virge::SetMemoryMapRegisters(void)
{
    PULONG ptr;
    int i; 
    
    // Setup the S3 ViRGE registers
    reg_MM850C = 0x11;      // Enable Linear addressing and Enhanced functions

    // Set the color pattern registers to a known value
    ptr = (PULONG) (m_pMMIO + 0xA100);
    for (i = 0; i < 48; i++)
        *ptr++ = 0;

    reg_MMA4D4 = 0x00000000;        // MMA4D4 Source base address register

    reg_MMA4D8 = 0x00000000;        // MMA4D8 Dest base address register
    reg_MMA508 = 0x00000000;        // MMA508 Rectangle source XY register
    reg_MMA50C = 0x00000000;        // MMA50C Rectangle dest XY register

    reg_MMA4E8 = 0xFFFFFFFF;        // MMA4E8 Mono Pattern 0
    reg_MMA4EC = 0xFFFFFFFF;        // MMA4EC Mono Pattern 1
    reg_MMA4F0 = 0x00FFFFFF;        // MMA4F0 Mono Pattern background color
    reg_MMA4F4 = 0x00FFFFFF;        // MMA4F4 Mono Pattern foreground color
    reg_MMA4F8 = 0x00FFFFFF;        // MMA4F8 Source Background Color Register
    reg_MMA4FC = 0x00FFFFFF;        // MMA4FC Source Foreground Color Register

    reg_MMA4DC = m_nScreenWidth;    // MMA4DC Left/Right clipping
    reg_MMA4E0 = m_nScreenHeight;   // MMA4E0 Top/Bottom clipping
    reg_MMA4E4 = (m_nScreenStride << 16) + m_nScreenStride;
}

void 
S3Virge::SetClock(ModeInit *pMode)
{
    int i;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::SetClock enter\r\n")));

    reg_BKWD_3 = 0x10;              // CR34
    reg_MISC = 0xEF;                // External clock

    for (i = 0; i <= SIZE_SR10_SR13; i++)           // Set clock timing
        reg_SR[i + 0x10] = pMode->Init_SR10_SR13[i];

    WaitForVBlank();
    reg_SR[0x15] = 0x20;            // SR15 on then off

    WaitForVBlank();
    reg_SR[0x15] = 0x03;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::SetClock leave\r\n")));
}

#ifdef ENABLE_OVERLAY

void
S3Virge::TurnOnStreamsProcessor()
{
    // wait for S3d engine idle
    WaitForNotBusy();

    // unlock
    reg_LOCK_1 = 0x48;
    reg_LOCK_2 = 0xA5;

    // streams needs to turned on/off during VBLANK
    WaitForVBlank();

    // turn on the stream
    BYTE bTmp = reg_EXT_MISC_2;
    reg_EXT_MISC_2 = bTmp | ENABLE_STREAM_PROCESSOR;

    // set up the registers for the primary stream
    reg_MM81C8 = m_nScreenStride;   // Primary Stream Stride

    if (m_pModeEx->ePixelFormat == ddgpePixelFormat_8bpp)
    {
        reg_MM8180 = P_RGB8;            // Primary Stream Control
    }
    else if (m_pModeEx->ePixelFormat == ddgpePixelFormat_5550 ||
             m_pModeEx->ePixelFormat == ddgpePixelFormat_5551)
    {
        reg_MM8180 = P_RGB5551;         // Primary Stream Control
    }
    else if (m_pModeEx->ePixelFormat == ddgpePixelFormat_565)
    {
        reg_MM8180 = P_RGB565;          // Primary Stream Control
    }
    else if (m_pModeEx->ePixelFormat == ddgpePixelFormat_8880)
    {
        reg_MM8180 = P_RGB8880;         // Primary Stream Control
    }
    else if (m_pModeEx->ePixelFormat == ddgpePixelFormat_8888)
    {
        reg_MM8180 = P_RGB8888;         // Primary Stream Control
    }

    reg_MM81F0 = XY(0, 0);          // Primary Window Start
    reg_MM81F4 = WH(m_nScreenWidth, m_nScreenHeight);
                                    // Primary Window Size
    // hide the overlay
    reg_MM81A0 = POnS;              // blend control
    reg_MM81DC = 0;                 // opaque control
    
    // initialize overlay globals
    m_pVisibleOverlay = NULL;
    m_dwOverlayFlipOffset = 0;

    // leave CR39 unlocked, but don't leave it be 0xA5, or VGA compatibility
    // (in CPU latch and Attribute registers: CR22, CR24/CR26) will be broken.
    reg_LOCK_2 = 0xA0;
}

#endif // ENABLE_OVERLAY

#ifdef ENABLE_DMA

void
S3Virge::TurnOnDMA()
{
    reg_MM8594 = 0;                 // initialize write pointer to 0
    reg_MM8598 = 0;                 // initialize read pointer to 0
    reg_MM8590 = (DMA_BUFFER_ADDRESS & 0x7FFFFFFF) | DMA_BUFFER_SIZE_64K;
                                    // set DMA base address
    reg_MM859C = COMMAND_DMA_ENABLE;// enable cmd/parameter/image data DMA
}

#endif // ENABLE_DMA

SCODE 
S3Virge::SetMode(int modeId, HPALETTE *pPalette)
{
    SCODE sc;
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::SetMode\r\n")));
    int mode;

    if (m_pMode)    // The mode is already set, just return
        return S_OK;

    for (mode = 0; mode < NUMVGAMODES; mode++)
    {
        DEBUGMSG(VIRGE_VERBOSE_MSGS, 
            (TEXT("VGAMode[mode].gpeModeEx.modeInfo.modeID = %d\r\n"), 
             mode, VGAMode[mode].gpeModeEx.modeInfo.modeId));

        if (modeId == VGAMode[mode].gpeModeEx.modeInfo.modeId)
        {
            DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                (TEXT("S3Virge::SetMode found modeID=%d at VGAMode[%d]\r\n"), 
                 modeId, mode));
            break;
        }
    }

    if (mode == NUMVGAMODES)
    {
        DEBUGMSG(VIRGE_VERBOSE_MSGS, 
            (TEXT("S3Virge::SetMode Wanted mode %d, not found\r\n"), modeId));
        return E_INVALIDARG;
    }

    m_pMode = &VGAMode[mode].gpeModeEx.modeInfo;
    m_pModeEx = &VGAMode[mode].gpeModeEx;
    m_nScreenWidth = m_pMode->width;
    m_nScreenHeight = m_pMode->height;
    m_nScreenBpp    = m_pMode->Bpp;
    m_nScreenStride = (m_nScreenWidth * m_nScreenBpp) / 8;
    m_dwPhysicalModeID = mode;

    // set up m_nDestFormat based on the Bpp
    switch (m_nScreenBpp)
    {
        case 8:
            m_nDestFormat = DESTINATION_FORMAT_8BIT;
            break;
        case 16:
            m_nDestFormat = DESTINATION_FORMAT_16BIT;
            break;
        case 24:
            m_nDestFormat = DESTINATION_FORMAT_24BIT;
            break;
        default:
            DebugBreak();
    }

    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("mode=%d modeID=%d width=%d height=%d stride=%d\r\n"),
         mode, m_pMode->modeId, m_nScreenWidth, 
         m_nScreenHeight, m_nScreenStride));

    m_pVideoMemoryHeap = new SurfaceHeap(m_nVideoMemorySize, NULL);

    GPESurf *pDummy;
    AllocSurface(&pDummy, m_nScreenWidth, 2, 
                 m_pMode->format, GPE_REQUIRE_VIDEO_MEMORY);     // for cursor
    
    if (FAILED(sc = AllocSurface((DDGPESurf**)&m_pPrimarySurface, 
                                 m_nScreenWidth, m_nScreenHeight, 
                                 m_pMode->format, m_pModeEx->ePixelFormat,
                                 GPE_REQUIRE_VIDEO_MEMORY)))
        return sc;

    UnlockVGA();                    // Unlock VGA registers
    Unlock();                       // Enable access to S3 specific regs
    
    SetVGAMode(&VGAMode[mode], m_nVideoMemorySize);

    SetMemoryMapRegisters();        // Setup Virge Extended Memory Mapped Regs

    SetClock(&VGAMode[mode]);       // Set the clock source & dividers dividers

    reg_DAC_AD_MK = 0xFF;           // Re-enable display

    DetermineScreenRefreshRate();

#ifdef ENABLE_OVERLAY
    // Enable streams processor
    TurnOnStreamsProcessor();
#endif // ENABLE_OVERLAY
#ifdef ENABLE_DMA
        // Enable Command DMA
    TurnOnDMA();
#endif // ENABLE_DMA

    // Clear the screen
    memset((PUCHAR) m_pPrimarySurface->Buffer(), 0x0, 
           m_nScreenStride * m_nScreenHeight);

    SetVisibleSurface((S3VirgeSurf *) m_pPrimarySurface);

    // Here, we use EngCreatePalette to create a palette that 
    // MGDI will use as a stock palette

    if (pPalette)
    {
        if (VGAMode[mode].gpeModeEx.modeInfo.Bpp == 8) {
            *pPalette = EngCreatePalette(PAL_INDEXED, PALETTE_SIZE, 
                            (PULONG)_rgbIdentity, 0, 0, 0);
        } 
        else 
        {
            ulong BitMasks[4] =
            {
                m_pModeEx->dwRBitMask,
                m_pModeEx->dwGBitMask,
                m_pModeEx->dwBBitMask,
                m_pModeEx->dwAlphaBitMask
            };
            *pPalette = EngCreatePalette(PAL_BITFIELDS, 
                                         BitMasks[3] ? 4 : 3, 
                                         BitMasks, 0, 0, 0);
        }
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (
            TEXT("Created %d Bpp palette, handle = 0x%08x\r\n"),
            VGAMode[mode].gpeModeEx.modeInfo.Bpp, *pPalette));
    }

    WaitForVBlank();

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("Leaving S3Virge::SetMode\r\n")));
    return S_OK;
}
