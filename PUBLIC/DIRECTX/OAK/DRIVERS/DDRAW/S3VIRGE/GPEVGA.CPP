/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

Module Name:    gpevga.cpp

Abstract:       various VGA/SVGA setup routines
  
Functions:


Notes: 


--*/

#include "precomp.h"

// instantiate one global struct
INSTANTIATE_MODE_INIT
struct ModeInit *VGAMode;

const TCHAR chRegPath[] = TEXT("Drivers\\BuiltIn\\DDHAL");
const TCHAR chStartupDisplayMode[] = TEXT("Startup Display Mode");

static int
RegGetValue(
    const TCHAR *pPath,
    const TCHAR *pKey,
    int iDefault
    )
{
    HKEY hKey;
    LONG lRet;
    int  iRet = iDefault;

    lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pPath, 0, KEY_QUERY_VALUE, &hKey);

    if (lRet == ERROR_SUCCESS) 
    {
        DWORD   dwType, dwLen;
        dwLen = sizeof(iRet);
        if (ERROR_SUCCESS != RegQueryValueEx(hKey, pKey, 0L, &dwType,
                                             (LPBYTE)&iRet, &dwLen)) 
        {
            iRet = iDefault;
        }
        RegCloseKey(hKey);
    }

    return iRet;
}

static BOOL 
PciFindDevice(
    IN USHORT wVendorId,
    IN USHORT wDeviceId,
    OUT PULONG pdwBusNumber,
    OUT PPCI_SLOT_NUMBER ppPciSlotNumber,
    OUT PPCI_COMMON_CONFIG pPciCommonConfig
    )
{
    PCI_SLOT_NUMBER slotNumber;
    PCI_COMMON_CONFIG pciConfig;
    int bus, device, function;
    int length;

    for (bus = 0; bus < PCI_MAX_BUS; bus++) 
    {
        for (device = 0; device < PCI_MAX_DEVICES; device++) 
        {
            slotNumber.u.bits.DeviceNumber = device;

            for (function = 0; function < PCI_MAX_FUNCTION; function++) 
            {
                slotNumber.u.bits.FunctionNumber = function;

                length = HalGetBusData(
                    PCIConfiguration, bus, slotNumber.u.AsULONG,
                    &pciConfig,
                    sizeof(pciConfig) - sizeof(pciConfig.DeviceSpecific));

                if ((length == 0) || (pciConfig.VendorID == 0xFFFF)) 
                {
                    break;
                }

                if ((wVendorId == pciConfig.VendorID) &&
                    (wDeviceId == pciConfig.DeviceID))
                {
                    *pdwBusNumber = bus;
                    *ppPciSlotNumber = slotNumber;
                    *pPciCommonConfig = pciConfig;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

GPEVGA::GPEVGA()
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GPEVGA::GPEVGA\r\n")));

    DWORD dwBusNumber;
    PCI_SLOT_NUMBER sPciSlotNumber;
    PCI_COMMON_CONFIG sPciConfig;

    if (!PciFindDevice(S3_VENDOR_ID, CHIPID_VIRGE_GX_DX, 
                       &dwBusNumber, &sPciSlotNumber, &sPciConfig))
    {
        DEBUGMSG(1, (TEXT("S3 Virge display card not found!!!\r\n")));
        DebugBreak();
        return;
    }
#ifdef ENABLE_INTR
    m_nIntrLine = sPciConfig.u.type0.InterruptLine;
    if (m_nIntrLine == 0xFF)
    {
        DEBUGMSG(1, (TEXT("No interrupt line rountine info available!!!\r\n")
                      TEXT("For driver to utilize interrupt, ")
                      TEXT("please run BIOS PCI setup ")
                      TEXT("to allocate an IRQ for the S3 Virge Card.\r\n")));
        DebugBreak();
    }
#endif // ENABLE_INTR

    m_pPortRanges = (PortRange *) NULL;

    // Reserve / Map in - standard VGA ports

    m_VGAIOPort[PortId_3BA].Map(0x3BA, m_pPortRanges);
    m_VGAIOPort[PortId_3C0].Map(0x3C0, m_pPortRanges);
    m_VGAIOPort[PortId_3C1].Map(0x3C1, m_pPortRanges);
    m_VGAIOPort[PortId_3C2].Map(0x3C2, m_pPortRanges);
    m_VGAIOPort[PortId_3C3].Map(0x3C3, m_pPortRanges);
    m_VGAIOPort[PortId_3C4].Map(0x3C4, m_pPortRanges);
    m_VGAIOPort[PortId_3C5].Map(0x3C5, m_pPortRanges);
    m_VGAIOPort[PortId_3C6].Map(0x3C6, m_pPortRanges);
    m_VGAIOPort[PortId_3C7].Map(0x3C7, m_pPortRanges);
    m_VGAIOPort[PortId_3C8].Map(0x3C8, m_pPortRanges);
    m_VGAIOPort[PortId_3C9].Map(0x3C9, m_pPortRanges);
    m_VGAIOPort[PortId_3CA].Map(0x3CA, m_pPortRanges);
    m_VGAIOPort[PortId_3CC].Map(0x3CC, m_pPortRanges);
    m_VGAIOPort[PortId_3CE].Map(0x3CE, m_pPortRanges);
    m_VGAIOPort[PortId_3CF].Map(0x3CF, m_pPortRanges);
    m_VGAIOPort[PortId_3D4].Map(0x3D4, m_pPortRanges);
    m_VGAIOPort[PortId_3D5].Map(0x3D5, m_pPortRanges);
    m_VGAIOPort[PortId_3DA].Map(0x3DA, m_pPortRanges);

    // Initialize access to registers which have different read ports 
    // from write ports
    DEBUGMSG(VIRGE_INIT_MSGS, (TEXT("Initialize access to registers\r\n")));

    reg_MISC.Init(m_VGAIOPort[PortId_3CC], m_VGAIOPort[PortId_3C2]);
    reg_FCR_WT.Init(m_VGAIOPort[PortId_3CA], m_VGAIOPort[PortId_3DA]);

    // Initialize access to indexed ranges of registers
    DEBUGMSG(VIRGE_INIT_MSGS, 
        (TEXT("Initialize access to indexed ranges of registers\r\n")));

    DEBUGMSG(VIRGE_INIT_MSGS, (TEXT("Initialize access to SR[]\r\n")));
    reg_SR.Init(reg_SR_ADDR, reg_SR_DATA);
    DEBUGMSG(VIRGE_INIT_MSGS, (TEXT("Initialize access to CR[]\r\n")));
    reg_CR.Init(reg_CR_ADDR, reg_CR_DATA);
    DEBUGMSG(VIRGE_INIT_MSGS, (TEXT("Initialize access to GR[]\r\n")));
    reg_GR.Init(reg_GR_ADDR, reg_GR_DATA);
    DEBUGMSG(VIRGE_INIT_MSGS, (TEXT("Initialize access to AR[]\r\n")));
    reg_AR.Init(reg_AR_ADDR, reg_AR_DATA, reg_AR_RESET);

    // Find out which display mode to start with from the registry
    m_nModeIndex = RegGetValue(chRegPath, chStartupDisplayMode, 0);
    VGAMode = VGAModeP + m_nModeIndex;
}


void 
GPEVGA::SetVGAMode(ModeInit *pMode, int memSize)
{
    int i; 

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GPEVGA::SetVGAMode() enter\r\n")));

    reg_DAC_AD_MK = 0;    // port 3c6                 // Darken the screen
    reg_RST_SYNC = 1;     // reg_SR[0]                // Async reset

    volatile unsigned char junk = reg_STATUS_1; // wakes up chip

    // Pointer to LAW setup
    pMode->Init_CR50_CR6A[9] = m_nPos_high;
    pMode->Init_CR50_CR6A[0xA] = m_nPos_low;

    // Change some of the setting in the CR_SEQ50/30 setup
    if (memSize == 4 * 1024 * 1024)
    {
        // CR58 Linear Address Control Register 4MB
        pMode->Init_CR50_CR6A[8] = (pMode->Init_CR50_CR6A[8] & 0xFC) | 0x03;
        // CR36 Configuration 1 Register
        pMode->Init_CR31_CR3C[5] = (pMode->Init_CR31_CR3C[5] & 0x1F) | 0x00;
    }
    else if (memSize == 2 * 1024 * 1024)
    {
        // CR58 Linear Address Control Register 2MB
        pMode->Init_CR50_CR6A[8] = (pMode->Init_CR50_CR6A[8] & 0xFC) | 0x02;
        // CR36 Configuration 1 Register
        pMode->Init_CR31_CR3C[5] = (pMode->Init_CR31_CR3C[5] & 0x1F) | 0x80;
    }
    else
    {
        DebugBreak();
    }

    // 
    // Bulk load the majority of the VGA/SVGA registers for this mode.
    //
    for (i = 0; i <= SIZE_SR0_SR4; i++) // Set Sequencer registers SR[0..4]
        reg_SR[i] = pMode->Init_SR0_SR4[i];

    for (i = 0; i <= SIZE_GR0_GR8; i++) // Set graphics controller regs GR[0..8]
        reg_GR[i] = pMode->Init_GR0_GR8[i];

    for (i = 0; i <= SIZE_CR0_CR18; i++)// Set CRTC controller regs CR[0..x18]
        reg_CR[i] = pMode->Init_CR0_CR18[i];

    for (i = 0; i <= SIZE_AR0_AR14; i++)// Set attribute regs AR[0..x14]
        reg_AR[i] = pMode->Init_AR0_AR14[i];
    
    for(i = 0; i < SIZE_CR31_CR3C; i++) // Set CRTC controller regs CR[30..3C]
        reg_CR[i + 0x31] = pMode->Init_CR31_CR3C[i];

    for(i = 0; i < SIZE_CR40_CR49; i++) // Set CRTC controller regs CR[40..49]
        reg_CR[i + 0x40] = pMode->Init_CR40_CR49[i];

    for(i = 0; i < SIZE_CR50_CR6A; i++) // Set CRTC controller regs CR[50..6C]
        reg_CR[i + 0x50] = pMode->Init_CR50_CR6A[i];

    reg_AR.ResetToAddr();               // Point back at address reg to 
                                        // enable following

    reg_AR_ADDR = 0x20;                 // Re-enable the palette

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GPEVGA::SetVGAMode() leave\r\n")));
}


// Emperically determine the screen refresh rate by watching the VBlank
// a count of 17 ticks per frame is about 60 Hz.
void 
GPEVGA::DetermineScreenRefreshRate()
{
    int i;
    ULONG startTickCount;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("GPEVGA::DetermineScreenRefreshRate enter\r\n")));

    WaitForVBlank();
    startTickCount = GetTickCount();

    for (i = 0; i < 16; i++)
        WaitForVBlank();

    m_nTicksPerFrame = (GetTickCount() - startTickCount + 10) / 16;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("GPEVGA::DetermineScreenRefreshRate leave %d ticks ")
         TEXT("(17 == 60 Hz)\r\n"), m_nTicksPerFrame));
}

// Return TRUE if in Vertical Blank, otherwise FALSE
int 
GPEVGA::InVBlank()
{
    return reg_STATUS_1 & 0x08;
}

// Returns at the Start of the next Vertical Blank
void 
GPEVGA::WaitForVBlank()
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GPEVGA::WaitForVBlank()r\n")));

    // First wait to be out of vblank
    while (IN_VBLANK);

    // Wait to get into vblank
    while (!IN_VBLANK);
}


void 
GPEVGA::LockVGA()
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GPEVGA::LockVGA() enter\r\n")));

    // disable writes to CR0..7
    reg_VRE = reg_VRE | 0x80;   // reg_CR[0x11]
}


void 
GPEVGA::UnlockVGA()
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GPEVGA::UnlockVGA() enter\r\n")));

    // enable writes to CR0..7
    reg_VRE = reg_VRE & 0x7F;   // reg_CR[0x11]
}


SCODE 
GPEVGA::SetPalette(
    const PALETTEENTRY *src, 
    USHORT firstEntry, 
    USHORT numEntries
    )
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("GPEVGA::SetPallette() enter\r\n")));

    if (firstEntry < 0 || firstEntry + numEntries > 256)
        return E_INVALIDARG;

    for (; numEntries; numEntries--)
    {
        reg_DAC_WR_AD = (UCHAR) (firstEntry++);
        reg_DAC_DATA = src->peRed >> 2;
        reg_DAC_DATA = src->peGreen >> 2;
        reg_DAC_DATA = src->peBlue >> 2;
        src++;
    }

    return S_OK;
}
