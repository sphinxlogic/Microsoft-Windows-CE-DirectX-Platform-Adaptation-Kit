// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-1998  Microsoft Corporation
//
//  Module Name:
//
//	gpevga.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------


#include "precomp.h"
#include "igsmode.h"

#ifdef USE_MEMORY_MAPPED_PORTS
#define CP5000_VENDOR_ID     0x10EA
#define CP5000_DEVICE_ID     0x5000

static BOOL PciFindDevice
(
    IN USHORT wVendorId,
    IN USHORT wDeviceId,
    OUT PULONG pdwBusNumber,
    OUT PPCI_SLOT_NUMBER ppPciSlotNumber,
    OUT PPCI_COMMON_CONFIG pPciCommonConfig
);
#define MEMORY_IO  0x00800000 /*memory mapped I/O control bit: bit 23*/

#define MEMORY_MAPPED_PORT_OFFSET (LinearBaseAddress | MEMORY_IO)
#else  //USE_MEMORY_MAPPED_PORTS
#define MEMORY_MAPPED_PORT_OFFSET  0
#endif //USE_MEMORY_MAPPED_PORTS
	// LinearBaseAddress is used globally in CONFIG.CPP
	// because XR35 register does not seem to return the correct
	// value when using memory mapped registers
ULONG LinearBaseAddress = NULL; /*linear frame buffer pointer base address*/


//
//  GPEVGA - constructor
//  inputs:
//  outputs: none
//
GPEVGA::GPEVGA()
{
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("GPEVGA::GPEVGA\r\n")));

#ifdef USE_MEMORY_MAPPED_PORTS
    DWORD	dwBusNumber;
    DWORD	dwSlotNumber;
    PCI_SLOT_NUMBER	    sPciSlotNumber; // defined in wdm.h
    PCI_COMMON_CONFIG	    sPciConfig;
	// find the card and get its memory mapped base address

#ifdef PCI_BUS
    if (!PciFindDevice(CP5000_VENDOR_ID, CP5000_DEVICE_ID,
	&dwBusNumber, &sPciSlotNumber, &sPciConfig)) {
	    // we are hosed if we can't find the card, since we can't really
	    // return an error from here
	DEBUGMSG( GPE_ZONE_ERROR,(TEXT("IGS5000 Display Not Found - fatal error\r\n")));
	return;
    }
    dwSlotNumber = sPciSlotNumber.u.AsULONG;

    DEBUGMSG(GPE_ZONE_ERROR,(TEXT("IGS5000 Device %04x %04x found in bus %d, slot%d\r\n"),
			    CP5000_VENDOR_ID,
			    CP5000_DEVICE_ID,
			    dwBusNumber,
			    dwSlotNumber));

	// Extract the important info from the PCI header
    if(sPciConfig.HeaderType != 0) {
	DEBUGMSG(GPE_ZONE_ERROR,(TEXT("IGS5000 Error: header type is %d, should be 0\r\n"),
		sPciConfig.HeaderType ));
	//return;
    }
	// get the base address and save for global use
    LinearBaseAddress = (ULONG) (sPciConfig.u.type0.BaseAddresses[0] & ~0x0000000FL);

#else
	// get the base address and save for global use
    LinearBaseAddress = PHYSICAL_LINEAR_FRAME_BUFFER;	 //Set physical linear frame buffer address

#endif //PCI_BUS

#endif //USE_MEMORY_MAPPED_PORTS

    m_pPortRanges = (PortRange *)NULL;

    // Reserve / Map in - standard VGA ports
    reg_POS.Map(0x102 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges);
    m_VGAIOPort[PortId_3BA].Map(0x3BA | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C0].Map(0x3C0 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C1].Map(0x3C1 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C2].Map(0x3C2 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C3].Map(0x3C3 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C4].Map(0x3C4 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C5].Map(0x3C5 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C6].Map(0x3C6 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C7].Map(0x3C7 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C8].Map(0x3C8 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3C9].Map(0x3C9 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3CA].Map(0x3CA | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3CC].Map(0x3CC | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3CE].Map(0x3CE | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3CF].Map(0x3CF | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D0].Map(0x3D0 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D1].Map(0x3D1 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D2].Map(0x3D2 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D3].Map(0x3D3 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D4].Map(0x3D4 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D5].Map(0x3D5 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D6].Map(0x3D6 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3D7].Map(0x3D7 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_3DA].Map(0x3DA | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );
    m_VGAIOPort[PortId_46E8].Map(0x46E8 | MEMORY_MAPPED_PORT_OFFSET, m_pPortRanges );

    // Initialize access to registers which have different read ports from write ports
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to registers\r\n")));

    reg_MISC.Init( m_VGAIOPort[PortId_3CC], m_VGAIOPort[PortId_3C2] );
    reg_FCR_WT.Init( m_VGAIOPort[PortId_3CA], m_VGAIOPort[PortId_3DA] );

    // Initialize access to indexed ranges of registers
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to indexed ranges of registers\r\n")));

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to SR[]\r\n")));
    reg_SR.Init( reg_SR_ADDR, reg_SR_DATA );
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to CR[]\r\n")));
    reg_CR.Init( reg_CR_ADDR, reg_CR_DATA );
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to GR[]\r\n")));
    reg_GR.Init( reg_GR_ADDR, reg_GR_DATA );
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to AR[]\r\n")));
    reg_AR.Init( reg_AR_ADDR, reg_AR_DATA, reg_AR_RESET );
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to XR[]\r\n")));
    reg_XR.Init( reg_XR_ADDR, reg_XR_DATA );
}


//
//  IGSInitChip -  Initialize all the extended registers
//  inputs:
//  outputs: none
//
void GPEVGA::IGSInitChip()
{
    int i;

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("GPEVGA::IGSInitChip\r\n")));

    Sleep (DBG_SLEEP);
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Setting XR registers\r\n")));
    for (i = 0; i < sizeof(IGSExtRegs)	; i+=2) {
	reg_XR[IGSExtRegs[i]] = IGSExtRegs[i+1];
    }


    Sleep (DBG_SLEEP);
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Setting SGRAM registers\r\n")));
    for (i = 0; i < sizeof(IGSExtSGRegs)  ; i+=2) {
	reg_SR[IGSExtSGRegs[i]] = IGSExtSGRegs[i+1];
    }

    reg_XR_SpriteControl = 0x04;    // RAMDAC address select
    reg_RAMDACMASK = 0x04;	    // use VREF
    reg_XR_SpriteControl = 0x00;


    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Clean up some extended registers\r\n")));
    // The clear-up is only necessary when warm boot due to abnormal cases
      //1. clean up CRT lock register
	reg_CR[0x1F] = 0x03;

      //2. clean up interpolation register
	reg_CR[0x45] = 0x00;

      //3. Video capture and overlay registers
	reg_XR[0xDC] = 0x00;	//video 1 overlay clean up
	reg_SR[0xDC] = 0x00;	//video 2 overlay clean up
	reg_XR[0xE8] = 0x00;	//video 1 capture clean up

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Leaving GPEVGA::IGSInitChip\r\n")));
}

//
//  SetVGAMode -
//  inputs:
//  outputs: none
//
void GPEVGA::SetVGAMode( ModeInit *pMode )
{
    int i;
#ifdef DEBUG
    UCHAR temp;
#endif

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("GPEVGA::SetVGAMode\r\n")));

    UnlockVGA();

    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Setting SR registers\r\n")));
    for( i=0; i< REG_SR_SIZE; i++ )  { // Set Sequencer registers SR[0..4]
	reg_SR[i] = pMode->std->Init_SR[i];
    }
    reg_MISC = pMode->std->Misc;
    reg_SR[0] = 0x03;	// sequence normal
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("SR[0] 0x%02X\r\n"), temp = reg_RST_SYNC));

    Sleep (DBG_SLEEP);

    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Setting GR registers\r\n")));
    for( i=0; i< REG_GR_SIZE; i++ ) { // Set graphics controller regs GR[0..8]
	reg_GR[i] = pMode->std->Init_GR[i];
    }
    Sleep (DBG_SLEEP);
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Setting CR registers\r\n")));
    for( i=0; i< REG_CR_SIZE; i++ ) { // Set CRTC controller regs CR[0..x18]
	reg_CR[i] = pMode->std->Init_CR[i];
    }

    Sleep (DBG_SLEEP);
    /*reprogram 3D4_13 to match color depth*/
    reg_CR[pMode->ext->Init_CR[0].Index] = pMode->ext->Init_CR[0].Data;

    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Setting XR registers\r\n")));
    for (i=0; i < REG_XR_SIZE; i++) {	// Set Extension Registers
	reg_XR[pMode->ext->Init_XR[i].Index] = pMode->ext->Init_XR[i].Data;
    }

    Sleep (DBG_SLEEP);
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Setting AR registers\r\n")));
    for( i=0; i< REG_AR_SIZE; i++ ) { // Set attribute regs AR[0..x14]
	reg_AR[i] = pMode->std->Init_AR[i];
    }

    Sleep (DBG_SLEEP);

    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Re-enabling the palette\r\n")));
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("SR[0] 0x%02X\r\n"), temp = reg_RST_SYNC));

    reg_AR.ResetToAddr();	// Point back at address reg to enable following

    reg_AR_ADDR = 0x20; 	// Re-enable the palette

    Sleep (DBG_SLEEP);
    LockVGA();
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Leaving GPEVGA::SetVGAMode\r\n")));
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("SR[0] 0x%02X\r\n"), temp = reg_RST_SYNC));
}

//
//  DetermineScreenRefreshRate -
//  inputs:
//  outputs: none
//
void GPEVGA::DetermineScreenRefreshRate()
{
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("GPEVGA::DetermineScreenRefreshRate\r\n")));
    WaitForVBlank();
    int i;
    unsigned long startTickCount = GetTickCount();
    for( i=0; i<20; i++ ) {
	WaitForVBlank();
    }
    m_nTicksPerFrame = ( GetTickCount() - startTickCount + 10 ) / 20;
    DEBUGMSG( GPE_ZONE_INIT, ( TEXT("Frame takes %d ticks\r\n"), m_nTicksPerFrame ));
}

//
//  InVBlank -
//  inputs:
//  outputs: none
//
int GPEVGA::InVBlank()
{
    return reg_STATUS_1 & 0x08;
}

void GPEVGA::WaitForVBlank()
{
    unsigned long i;

    DEBUGMSG (GPE_ZONE_INIT, (TEXT("GPEVGA::WaitForVBlank()\r\n")));


    // First wait to be out of vblank

    for (i = 0; i < 0x1000000; i++) {
	if( !(reg_STATUS_1 & 0x08 ) ) {
	    break;
	}
    }

    // Wait to get into vblank
    for (; i < 0x1000000; i++) {
	if( reg_STATUS_1 & 0x08 ) {
	    break;
	}
    }

    VBlankReceived();	// Reset counters etc
    DEBUGMSG (GPE_ZONE_INIT, (TEXT("Leaving GPEVGA::WaitForVBlank()\r\n")));
}

//
//  LockVGA -
//  inputs:
//  outputs: none
//
void GPEVGA::LockVGA()
{
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("GPEVGA::LockVGA\r\n")));
    // disable writes to CR0..7
    reg_VRE = reg_VRE | 0x80;	// reg_CR[0x11]
}

//
//  UnlockVGA -
//  inputs:
//  outputs: none
//
void GPEVGA::UnlockVGA()
{
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("GPEVGA::UnlockVGA\r\n")));
    // enable writes to CR0..7
    reg_VRE = reg_VRE & 0x7F;	// reg_CR[0x11]
}

//
//  SetPalette -
//  inputs:
//  outputs: none
//
SCODE GPEVGA::SetPalette(
    const PALETTEENTRY *src,
    unsigned short firstEntry,
    unsigned short numEntries )
{
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("GPEVGA::SetPalette\r\n")));
    if( firstEntry < 0 || firstEntry + numEntries > 256 ) {
	return E_INVALIDARG;
    }

    for( ; numEntries; numEntries-- ) {
	reg_DAC_WR_AD = (unsigned char)(firstEntry++);
	reg_DAC_DATA = src->peRed >> 2;
	reg_DAC_DATA = src->peGreen >> 2;
	reg_DAC_DATA = src->peBlue >> 2;
	src++;
    }
    return S_OK;
}


#ifdef USE_MEMORY_MAPPED_PORTS
//
//  PciFindDevice -
//  inputs:
//  outputs: none
//
static BOOL PciFindDevice
(
    IN USHORT wVendorId,
    IN USHORT wDeviceId,
    OUT PULONG pdwBusNumber,
    OUT PPCI_SLOT_NUMBER ppPciSlotNumber,
    OUT PPCI_COMMON_CONFIG pPciCommonConfig
)
{
    PCI_SLOT_NUMBER	slotNumber;
    PCI_COMMON_CONFIG	pciConfig;
    int 		bus, device, function;
    int 		length;

    for (bus = 0; bus < PCI_MAX_BUS; bus++) {
	for (device = 0; device < PCI_MAX_DEVICES; device++) {
	    slotNumber.u.bits.DeviceNumber = device;

	    for (function = 0; function < PCI_MAX_FUNCTION; function++) {
		slotNumber.u.bits.FunctionNumber = function;

		length = HalGetBusData(
		    PCIConfiguration, bus, slotNumber.u.AsULONG,
		    &pciConfig,
		    sizeof(pciConfig) - sizeof(pciConfig.DeviceSpecific));

		if ((length == 0) || (pciConfig.VendorID == 0xFFFF)) {
		    break;
		}

		if ((wVendorId == pciConfig.VendorID) &&
		    (wDeviceId == pciConfig.DeviceID)) {
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
#endif //USE_MEMORY_MAPPED_PORTS


#ifdef TVOUT
//  TV Programming
//
//  PciFindDevice -
//  inputs:
//  outputs: none
//
VOID GPEVGA::SetTvMode()
{
     /*save ext_reg_33 and ext_reg_3C*/
    m_Reg33 = reg_XR_BIUMisc;
    m_Reg3C = reg_XR_FunctionControl;

    SetTVReg(TVRegs[WhichMode], REG_TV_SIZE);

    EnableTV(TRUE);

#ifdef FB16BPP
    if (WhichMode == MODE_640x440x16)
#else
    if (WhichMode == MODE_640x440x8)
#endif
    {
      /*640x440 screen needs a little centering adjustment*/
      WriteTVReg(0xE468, ReadTVReg(0xE468) + 2);
      WriteTVReg(0xE46C, ReadTVReg(0xE46C) + 2);
    }

    TVOn(TRUE);

    SetTVColor();

    // Interpolation is only for 640x480 Underscan mode
#ifdef FB16BPP
    if ( WhichMode == MODE_640x480x16_U )
#else
    if ( WhichMode == MODE_640x480x8_U	)
#endif
      SetInterpolation();
}


//
//  SetTVReg - Program TV registers.
//  inputs:  wPtr - a WORD pointer to an array of TV register settings
//	     iCounter - counter of the TV registers.
//  outputs: none
//
VOID GPEVGA::SetTVReg(TVRegisters *wPtr, int iCounter)
{
    WORD wTVIndex, wTVValue;

    for (USHORT ii=0; ii < iCounter; ii+=2) {
      wTVIndex = wPtr->Init_TV[ii];
      wTVValue = wPtr->Init_TV[ii+1];

      WriteTVReg(wTVIndex, wTVValue);
    }
}

//
//  ReadTVReg - Read a 16-bit value from a TV register
//  inputs:  Index - TV register index.
//  outputs: Return a 16-bit value of the TV register
//
USHORT GPEVGA::ReadTVReg(WORD index)
{
    WORD value;

    BYTE valueL;
    BYTE valueH;

    UnlockTVReg();
    valueL =  *(PUCHAR)((m_pTvRegs) + index);
    valueH =  *(PUCHAR)((m_pTvRegs) + index + 1);

    value = (USHORT) (valueL + (valueH << 8) );

    LockTVReg();

    return value;
}

//
//  WriteTVReg - Write a 16-bit value into a TV register
//  inputs:  Index - TV register index.
//	     data  - data to write
//  outputs: none
//
VOID GPEVGA::WriteTVReg(WORD index, WORD data)
{
    UnlockTVReg();

    *((USHORT *)(m_pTvRegs + index)) = data;

    LockTVReg();
}

//
//  UnlockTVReg - Allow the access to TV registers
//  inputs:  none
//  outputs: none
//
VOID GPEVGA::UnlockTVReg(VOID)
{
    reg_XR_BIUMisc = (m_Reg33 | 0x08);
    reg_XR_FunctionControl = (m_Reg3C | 0x80);
}

//
//  LockTVReg - Deny the access to TV registers
//  inputs:  none
//  outputs: none
//
VOID GPEVGA::LockTVReg(VOID)
{
    reg_XR_FunctionControl = m_Reg3C;
    reg_XR_BIUMisc = m_Reg33;
}


//
//  EnableTV - Enable or Disable TV out
//  inputs:  none
//  outputs: none
//
VOID GPEVGA::EnableTV(BOOLEAN iOnOff)
{
    BYTE iTmp;
    WORD iTVType;

    iTVType = ReadTVReg(0xE438) & 0x1000;

    reg_XR_IndirectIndexFor4x = 0x05;	/*Banking I/O control*/

    if (iOnOff) {
      iTmp = reg_XR_X2VideoDisplayVertEnd & ~0x05;  /*bit <2,0>*/
      if (iTVType)
	reg_XR_X2VideoDisplayVertEnd = iTmp | 0x04; //NTSC
      else
	reg_XR_X2VideoDisplayVertEnd = iTmp | 0x05; //PAL
    } else {
      iTmp = reg_XR_X2VideoDisplayVertEnd & ~0x04;
      reg_XR_X2VideoDisplayVertEnd = iTmp;
    }
}


//
//  TVOn - Turn on or turn off TV.
//  inputs:  iOnOff - TRUE on
//  outputs: none
//
VOID GPEVGA::TVOn(BOOLEAN iOnOff)
{
    UCHAR iTmp;

    if (iOnOff) {
	iTmp = reg_XR_Ramdac;
	reg_XR_Ramdac = iTmp | 0x20;
    } else {
	iTmp = reg_XR_Ramdac;
	reg_XR_Ramdac = iTmp & ~0x20;
    }
}

//
//  SetTVColor - It corrects TV color each time when a new mode is set
//  inputs:  none
//  outputs: none
//
VOID GPEVGA::SetTVColor()
{
     WORD wTmp0;
     WORD wTmpE440;
     WORD wTmpE448;
     WORD wTmpE430;

     wTmpE440 = ReadTVReg(0xE440);
     wTmpE448 = ReadTVReg(0xE448);
     wTmpE430 = ReadTVReg(0xE430);

   /* Reset FSC: 3CE_4E<1>=1 */
     reg_XR_IndirectIndexFor4x = 0x05;
     reg_XR_X2VideoDisplayVertEnd = reg_XR_X2VideoDisplayVertEnd | 0x02;

   /*make TV screen is not visible to avoid flash*/
     WriteTVReg(0xE430, wTmpE430 | 0x0003);

   /*Make the TV blank area nerrow*/
     WriteTVReg(0xE440, 0x20);
     WriteTVReg(0xE448, 0x20);

   /* set test logic to blue */
     WriteTVReg(0xE604, 0xFF40);
     WriteTVReg(0xE606, 0x0000);

   /* Wait for Vertical Display Enable and wait 2 frames */
     while ( ( reg_STATUS_1 & 0x08 ) == 0x00) { /*search until 1*/ ; }
     while ( ( reg_STATUS_1 & 0x08 ) != 0x00) { /*search until 0 active low*/ ; }
     while ( ( reg_STATUS_1 & 0x08 ) == 0x00) { /*search until 1*/ ; }
     while ( ( reg_STATUS_1 & 0x08 ) != 0x00) { /*search until 0 active low*/ ; }

   /*
     if (E454<8>=1)
	the color is O.K., do nothing.
     else
	the Color needs to be toggled through E46C<0>
   */

     do {
	  wTmp0 = ReadTVReg(0xE454);

     } while ( ((wTmp0 & 0x0100) != 0x0100) && ((wTmp0 & 0x0100) != 0x0000) ) ;

   /* set test logic back to normal */
     WriteTVReg(0xE604, 0x0000);
     WriteTVReg(0xE606, 0x0000);

     WriteTVReg(0xE440, wTmpE440);   /*restore them*/
     WriteTVReg(0xE448, wTmpE448);
     WriteTVReg(0xE430, wTmpE430);

   /* clear FSC: 3CE_4E<1>=0 and toggle TV_REG_E46C<0> accordingly */
     reg_XR_IndirectIndexFor4x = 0x05;
     reg_XR_X2VideoDisplayVertEnd = (reg_XR_X2VideoDisplayVertEnd & ~0x02);

     if ( (wTmp0 & 0x0100) == 0x0000 )
       WriteTVReg(0xE46C ,ReadTVReg(0xE46C) ^ 0x0001);
}

//
//  BypassMode -
//  inputs:  iOnOff - True on
//  outputs: none
//
VOID GPEVGA::BypassMode(BOOLEAN iOnOff)
{
    BYTE bTmp;

    bTmp = reg_XR_SpriteControl;
    reg_XR_SpriteControl = bTmp | 0x04;
    if (iOnOff) {
	reg_RAMDACMASK = reg_RAMDACMASK  | 0x10;
    } else {
    reg_RAMDACMASK = reg_RAMDACMASK  & ~0x10;
    }
    reg_XR_SpriteControl = bTmp & ~0x04;
}

//
//  Enable interpolation in underscan mode
//  inputs:  none
//  outputs: none
//
VOID GPEVGA::SetInterpolation()
{
    BYTE bTmp;

    for (ULONG ii = 0; ii < sizeof(I640x480x60_3CE); ii+=2) {
	reg_XR[I640x480x60_3CE[ii]] = I640x480x60_3CE[ii+1];
    }
    reg_XR_SpriteControl = reg_XR_SpriteControl  | 0x40;

#ifdef FB16BPP
    for (      ii = 0; ii < sizeof(I640x480x16x60_3D4); ii+=2) {
	 reg_CR[I640x480x16x60_3D4[ii]] = I640x480x16x60_3D4[ii+1];
    }
#else  //FB16BPP
    for (      ii = 0; ii < sizeof(I640x480x8x60_3D4);	ii+=2) {
	 reg_CR[I640x480x8x60_3D4[ii]] = I640x480x8x60_3D4[ii+1];
    }
#endif	//FB16BPP

    if (GetBusWidth() == 0) { /* only need to change if it is 32 bits */
	/* fetch register low*/
	bTmp = reg_EXT_MODE;

	if (bTmp & 0x80) {    /*overflow bit*/
	    reg_MODE_CTL = reg_MODE_CTL  | 0x10;
	}

	bTmp = bTmp << 1;
	reg_EXT_MODE = bTmp;
    }

    reg_XR[0xFA] = 0x10;      /*banking*/
    reg_SR[0x4F] = reg_SR[0x4F] | 0x01;
    reg_XR[0xFA] = 0x00;
}

//
//  obtain current video memory bus width
//  inputs:  none
//  outputs: Return 0: 32 bits; 1: 64 bits
//
USHORT GPEVGA::GetBusWidth(VOID)
{
    BYTE bData;
    bData = reg_XR_MEMCTL2 & 0x04;  /*0: 32 bits; 1: 64 bits.*/
    return (bData >> 2);
}
#endif
