// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-1998  Microsoft Corporation
//	COPYRIGHT (C) 1999, IGS TECHNOLOGIES, INC. (iGST)
//
//  Module Name:
//
//	config.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#include "precomp.h"


extern ModeInit *VGAMode[];
extern MODENUMBER WhichMode;

// The dpCurSettings structure
//INSTANTIATE_GPE_ZONES(0x3,"DDI Driver","unused1","unused2")      /* Start with Errors, warnings, and temporary messages */
INSTANTIATE_GPE_ZONES(0x20,"DDI Driver","unused1","unused2")      /* Start with Errors, warnings, and temporary messages */

	// LinearBaseAddress is used globally from GPEVGA.CPP
	// because XR35 register does not seem to return the correct
	// value when using memory mapped registers
extern ULONG LinearBaseAddress; /*linear frame buffer pointer base address*/


// This gets around problems exporting from .lib
BOOL APIENTRY GPEEnableDriver(
    ULONG	   iEngineVersion,
    ULONG	   cj,
    DRVENABLEDATA *pded,
    PENGCALLBACKS  pEngCallbacks);



//
//  DrvEnableDriver - main entry point
//  inputs:
//  outputs: none
//
BOOL APIENTRY DrvEnableDriver(
    ULONG	   iEngineVersion,
    ULONG	   cj,
    DRVENABLEDATA *pded,
    PENGCALLBACKS  pEngCallbacks)
{

    return GPEEnableDriver( iEngineVersion, cj, pded, pEngCallbacks );
}


#ifdef FB16BPP
extern ulong BitMasks[];// 16 bpp
#else  // FB16BPP
#include "palette.h"    // for 8Bpp we use the natural palette
INSTANTIATE_PALETTE
#endif // FB16BPP

static GPE *pGPE = (GPE *)NULL;

//
//  GetGPE - Main entry point for a GPE-compliant driver
//  inputs:  none
//  outputs: none
//
GPE *GetGPE()
{
    if( !pGPE )
	pGPE = new IGS5000();
    return pGPE;
}

#ifdef DEBUG
//
//  DumpVGARegs - dump the chips registers to the debugger
//  inputs:  none
//  outputs: none
//
void IGS5000::DumpVGARegs()
{
    ULONG   i;
    BYTE bTemp;

    DEBUGMSG (GPE_ZONE_HW, (TEXT("Dumping VGA Registers\r\n")));



    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\nMSR   %02X;  FCR   %02X;\r\n"),
	      bTemp = reg_MISC, bTemp = reg_FCR_WT));

    // SR Registers
    DEBUGMSG (GPE_ZONE_HW, (TEXT("Dumping SR\r\n")));
    for (i=0; i < REG_SR_SIZE; i++) {
	DEBUGMSG (GPE_ZONE_HW, (TEXT("SR%02X  %02X;  "), i, bTemp = reg_SR[i]));
	if (i % 2) {
	    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));
	}
    }
    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));

    // CR registers
    DEBUGMSG (GPE_ZONE_HW, ((TEXT("Dumping CR\r\n"))));
    for (i=0; i < REG_CR_SIZE; i++) {
	DEBUGMSG (GPE_ZONE_HW, (TEXT("CR%02X  %02X;  "), i, bTemp = reg_CR[i]));
	if (i % 2) {
	    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));
	}
    }
    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));

    // GR registers
    DEBUGMSG (GPE_ZONE_HW, (TEXT("Dumping GR\r\n")));
    for (i=0; i < REG_GR_SIZE; i++) {
	DEBUGMSG (GPE_ZONE_HW, (TEXT("GR%02X  %02X;  "), i, bTemp = reg_GR[i]));
	if (i % 2) {
	    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));
	}
    }
    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));

    // AR registers
    DEBUGMSG (GPE_ZONE_HW, (TEXT("Dumping AR\r\n")));
    for (i=0; i < REG_AR_SIZE; i++) {
	reg_AR.ResetToAddr();
	DEBUGMSG (GPE_ZONE_HW, (TEXT("AR%02X  %02X;  "), i, bTemp = reg_AR[i]));
	if (i % 2) {
	    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));
	}
    }
    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));

    // XR registers
    DEBUGMSG (GPE_ZONE_HW, (TEXT("Dumping XR\r\n")));
    //for (i=0; i < REG_XR_SIZE; i++) {
    for (i=0; i < 0xFF; i++) {
	DEBUGMSG (GPE_ZONE_HW, (TEXT("XR%02X  %02X;  "), i, bTemp = reg_XR[i]));
	if (i % 2) {
	    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));
	}
    }
    DEBUGMSG (GPE_ZONE_HW, (TEXT("\r\n")));

    Sleep (DBG_SLEEP);
    DEBUGMSG (GPE_ZONE_HW, (TEXT("Leaving DumpVGARegs()\r\n")));
}
#endif //if DEBUG


//
//  IGS5000 Constructor
//  inputs:  none
//  outputs: none
//
IGS5000::IGS5000()
{
    BYTE    bTemp;

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("IGS5000::IGS5000\r\n")));


#ifdef COMMANDPARAM  //this will support command parameter passed in from loadcepc.exe
    PBYTE   pArgs;
    DWORD   dwOldMode;

    //
    // GDI calls this routine before anything is done
    // Hardware specific tasks can be carried out here, for example, setting
    //	display width or initialize hardware.
    // If everything is taken care of at boot time, nothing needs to be done
    //	here.
    //


    dwOldMode = SetKMode(TRUE);

    pArgs = *(PBYTE *)0x801FFFFC;

    if (pArgs != 0)
    {
//     (ULONG)pArgs |= 0x80000000;
       unsigned long itmp = (unsigned long ) pArgs | 0x80000000;
       pArgs = (PBYTE) itmp;

       WhichMode = (MODENUMBER) pArgs[0];
    }
    else
    {
       OutputDebugString(TEXT("GetBootArgs failed - using default mode\r\n"));

       WhichMode = (MODENUMBER) 0;
    }

    SetKMode(dwOldMode);
#else
    WhichMode = (MODENUMBER) 0;
#endif //COMMANDPARAM


/*=== code to initialize the chip to a valid mode ===*/

#ifdef USE_MEMORY_MAPPED_PORTS

#ifdef MIPS
	//	a HACK for normal IGS card to run in memory mode.
	//	Microsoft's IGS card has the pull up resistor that sets the default mode to Memory Mapped mode.
	//	On the sample IGS card from IGST, the pull up is not implemented and hence the video card default to IO mode.
	//	This driver assumes Memory Mapped mode.
	//	The fix below is from IGST, it basically goes into the standard IO port (for vga) and turn the chip to
	//	IO mode hence eliminating the need for pull up.

	BOOL	bKMode;
	PBYTE	pIO_46E8	= (PBYTE) 0xb80046e8;
	PBYTE	pIO_102 	= (PBYTE) 0xb8000102;
	PBYTE	pIO_3CE 	= (PBYTE) 0xb80003ce;
	PBYTE	pIO_3CF 	= (PBYTE) 0xb80003cf;

	bKMode = SetKMode (TRUE);

	*pIO_46E8 = 0x18;
	*pIO_102  = 0x01;
	*pIO_46E8 = 0x08;


	*pIO_3CE  = 0x33;
	*pIO_3CF  = 0x01;

	SetKMode (bKMode);
#else // MIPS
    /*HERE IS THE ONLY PLACE WHERE CONTAINS ASSEMBLY CODE*/
	// wake up the chip
    __asm{
	 mov edx, 0x46e8;     //only a few port I/O needed, the rest of the code
	 mov eax, 0x18;       //will use memory mapped I/O only
	 out dx, al;

	 mov edx, 0x102;
	 mov eax, 0x01; 	  //enable the chip
	 out dx, al;

	 mov edx, 0x46e8;
	 mov eax, 0x08;
	 out dx, al;

  #ifndef PCI_BUS
  /*
     The code is only necessary for the platform where video linear frame
     buffer is not assigned by a system software, such as system BIOS.
     E.g. For VL bus system, we will have to assign a valid linear address
	  for the video memory before we can enable it.
  */
	 mov edx, 0x3ce;
	 mov eax, (PHYSICAL_LINEAR_FRAME_BUFFER >>16) | 0x35;
	 out dx, ax;
  #endif

	 mov edx, 0x3ce;
	 mov eax, 0x0133;	  //enable linear frame buffer
	 out dx, ax;
    }
#endif // MIPS

#else  //use port I/O

	// wake up the chip
    reg_SETUP_MD = 0x18;
    reg_POS = 0x01;		  // Global Enable register ( = 0x102)
    reg_SETUP_MD = 0x08;

  #ifndef PCI_BUS
  /*
     The code is only necessary for the platform where video linear frame
     buffer is not assigned by a system software, such as system BIOS.
     E.g. For VL bus system, we will have to assign a valid linear address
	  for the video memory before we can enable it.
  */
    reg_XR[0x35] = (PHYSICAL_LINEAR_FRAME_BUFFER >>24);
  #endif

#endif //USE_MEMORY_MAPPED_PORTS

    reg_XR[0xBF] = 0x00;      /*banking register*/
    reg_XR[0xBA] = 0x80;      /*use external RC filter for PLL*/
    reg_XR[0xBB] = 0x80;      /*use external RC filter for PLL*/

   /*need maximum 1 millisecond for PLL to be stable*/
    int iTmp, i;
    for (i=0; i<0x4000;i++)   /*This should be longer enough*/
      iTmp = reg_XR[0x92];    /*dummy read a chip ID*/


    reg_MISC = 0xED;   // Use color address range  ( = 0x3C2 write, 0x3CC read )
    unsigned char checkChip = reg_MISC;     // read from 0x3CC
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("reg_MISC reads back as %02X\r\n"), checkChip ));

    reg_XR[0x70] = 0xCB;     /*reset sequencer*/

    IGSInitChip();	     /*Init the chip extended registers*/
    UnlockVGA();
    SetVGAMode(VGAMode[WhichMode]);  /*Set the chip to a valid mode, e.g 640x480x8*/
    SetClock(); 	     /*Activate the new clock settings*/

    reg_XR[0x70] = 0x0B;     /*set sequencer*/
    reg_SR[0x36] = 0x3D;     /*activate SGRAM initial cycle*/

/*=== the above code has initialized the chip to a valid mode ===*/


#ifdef USE_MEMORY_MAPPED_PORTS
    m_nLAWPhysical = LinearBaseAddress;
#else  //USE_MEMORY_MAPPED_PORTS
	// get the linear frame buffer address from the extension mapped
	// PCI base address register
    UCHAR lowAddress = 0;
    UCHAR highAddress = reg_XR_ExtLinearAddress;
    m_nLAWPhysical = (((ULONG)highAddress) << 24) | (((ULONG)lowAddress) << 20);
    LinearBaseAddress = m_nLAWPhysical;
#endif	//USE_MEMORY_MAPPED_PORTS

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("LAWPhysical is at: 0x%08x\r\n"), m_nLAWPhysical ));

    //to perform a video memory size auto-detection, need 4 MB video memory size
    m_nVideoMemorySize= 4 * 1024 * 1024;
    m_pLAW = (unsigned char *)VirtualAlloc(0, m_nVideoMemorySize, MEM_RESERVE, PAGE_NOACCESS );

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("VirtualAlloc of LAW returns 0x%08x\r\n"), m_pLAW));

    ULONG inIoSpace = 0;
    PHYSICAL_ADDRESS ioPhysicalBase = { m_nLAWPhysical, 0 };

    if (HalTranslateBusAddress(PCIBus, 0, ioPhysicalBase, &inIoSpace, &ioPhysicalBase))
    {
	m_nLAWPhysical = ioPhysicalBase.LowPart;

	// map the physical pages into the virtual region
	VirtualCopy(m_pLAW, (LPVOID)(m_nLAWPhysical>>8), m_nVideoMemorySize,
		    PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL);
    }
    else
    {
	// aaargh!!!!
    }

    if ( DetectVideoMemory() == -1) /* detect video memory size */
       DEBUGMSG( GPE_ZONE_INIT,(TEXT("Something is wrong with Video Memory!!!\r\n")));

    m_nVideoMemorySize = reg_XR_MEMCTL2 & 0x3;

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Mem Size\r\n"),m_nVideoMemorySize));

    switch(m_nVideoMemorySize) {
	case 0:
	    m_nVideoMemorySize = 1024 * 1024;	// 1MB
	    //DEBUGMSG( GPE_ZONE_INIT,(TEXT("IGS5000: 1MB FrameBuffer\r\n")));
	    DEBUGMSG( 1,(TEXT("IGS5000: 1MB FrameBuffer\r\n")));
	    break;
	case 1:
	    m_nVideoMemorySize= 2 * 1024 * 1024;  // 2MB
	    DEBUGMSG( 1,(TEXT("IGS5000: 2MB FrameBuffer\r\n")));
	    break;
	case 2:
	    m_nVideoMemorySize= 4 * 1024 * 1024; // 4MB
	    DEBUGMSG( 1,(TEXT("IGS5000: 4MB FrameBuffer\r\n")));
	    break;
	default:
	    m_nVideoMemorySize= 4 * 1024 * 1024; // ?MB
	    DEBUGMSG( 1,(TEXT("IGS5000: FrameBuffer size unknown\r\n")));
	    break;
    }

    // Make sure we deduct 4k off the end for the sound buffer allocation.

    m_nVideoMemorySize -= 4096;


#ifdef TVOUT
	// map the TV memory mapped registers in
    m_pTvRegs = (unsigned char *)VirtualAlloc(0, TV_REG_SIZE, MEM_RESERVE, PAGE_NOACCESS );

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("VirtualAlloc of m_pTvRegs returns 0x%08X, mapping: 0x%X\r\n"),
		m_pTvRegs, ((m_nLAWPhysical | TV_MEMORY_OFFSET) >>8)));

	// map the physical pages into the virtual region
    VirtualCopy(m_pTvRegs, (LPVOID)((m_nLAWPhysical | TV_MEMORY_OFFSET) >>8), TV_REG_SIZE,
		PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL);
#endif //TVOUT


#ifdef ENABLE_ACCELERATION
    // Map in memory mapped range of registers
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("VirtualAlloc accelerator registers\r\n")));

    m_pMMIO = (PBYTE)VirtualAlloc(0, ACCEL_REG_SIZE, MEM_RESERVE, PAGE_NOACCESS);

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("VirtualAlloc of MMIO returns 0x%08X, mapping: 0x%X\r\n"),
		m_pMMIO, ((m_nLAWPhysical | ACCEL_MEMORY_OFFSET) >>8)));

	// map the physical pages into the virtual region
    VirtualCopy(m_pMMIO, (LPVOID)((m_nLAWPhysical | ACCEL_MEMORY_OFFSET) >>8), ACCEL_REG_SIZE,
		PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL);

    // Map in memory mapped range of host memory
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("VirtualAlloc accelerator hostmem\r\n")));

    m_pHostMem = (PDWORD)VirtualAlloc(0, HOSTMEM_SIZE, MEM_RESERVE, PAGE_NOACCESS);

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("VirtualAlloc of HostMem returns 0x%08X, mapping: 0x%X\r\n"),
		m_pHostMem, ((m_nLAWPhysical | HOSTMEM_OFFSET) >>8)));

    // map the physical pages into the virtual region
    VirtualCopy(m_pHostMem, (LPVOID)((m_nLAWPhysical | HOSTMEM_OFFSET) >>8), HOSTMEM_SIZE,
		PAGE_READWRITE|PAGE_NOCACHE|PAGE_PHYSICAL);

#endif	// ENABLE_ACCELERATION

#ifdef DEBUG
    DumpVGARegs();
#endif

    // Try reading/writting to FrameBuffer
    *m_pLAW = 0x55;
    bTemp = *m_pLAW;
    if (bTemp != 0x55) {
	DEBUGMSG (GPE_ZONE_ERROR, (TEXT("Memory read=0x%X\r\n"), bTemp));
    } else {
	DEBUGMSG (GPE_ZONE_INIT, (TEXT("!!Success, can write/read memory\r\n")));
    }

    // clear the frame buffer
    PULONG pFrameBuffer;
    LONG ii = 0;
    pFrameBuffer = (PULONG)m_pLAW;
    for (ii = 0; ii < m_nVideoMemorySize; ii+=sizeof(ULONG)) {
	*pFrameBuffer++ = 0x00;
    }

    m_ModeInfo = VGAMode[WhichMode]->gpeMode;

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Actual memory detected: %d bytes\r\n"),
	    m_nVideoMemorySize ));

    DEBUGMSG( 1 || GPE_ZONE_INIT,(TEXT("IGS5000::IGS5000 done!\r\n")));
}

//
//  SetModeRegisters
//  inputs:  none
//  outputs: none
//
void IGS5000::SetModeRegisters()
{
    // Since we needed to do this earlier let's limit the number of
    // times we do it
    static BOOL fDone = FALSE;

    if (fDone) {
	return;
    }
    fDone = TRUE;

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("IGS5000::SetModeRegisters\r\n")));

    UnlockVGA();
    SetVGAMode(VGAMode[WhichMode]);  /*Set the chip to a valid mode, e.g 640x480x8*/
    DEBUGMSG (1, (TEXT("About to set clock\r\n")));
    SetClock(); 	    // Set the clock source & dividers dividers

    Sleep (DBG_SLEEP);

    DEBUGMSG (1, (TEXT("About to set reg_DAC_AD_MK (Currently 0x%X)\r\n"),
			       reg_DAC_AD_MK));

    reg_DAC_AD_MK = 0xFF;   // Re-enable display

#ifdef TVOUT
    // setup the NTSC TV out
    SetTvMode();

    DetectUnderscan();	//use only for h/w cursor
#endif //TVOUT

#ifdef FB16BPP
    BypassMode(TRUE);
#else //FB16BPP
    BypassMode(FALSE);
#endif	//FB16BPP


    Sleep (DBG_SLEEP);
    DEBUGMSG (1, (TEXT("About to determine screen refresh rate\r\n")));

    DetermineScreenRefreshRate();

    LockVGA();

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Leaving IGS5000::SetModeRegisters\r\n")));
}


//
//  SetClock
//  inputs:  none
//  outputs: none
//
void IGS5000::SetClock()
{
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("IGS5000::SetClock\r\n")));

    BYTE bTmp;

    reg_XR[0xBF] = 0x00;		/*banking register*/
    reg_XR[0xBA] = reg_XR[0xBA] | 0x80; /*use external RC filter for PLL*/

    bTmp = reg_XR_ExtMiscClock;
    *reg_XR.m_pDataPort =  bTmp & 0x7F;
    *reg_XR.m_pDataPort =  bTmp | 0x80;
    *reg_XR.m_pDataPort =  bTmp & 0x7F;

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Leaving IGS5000::SetClock\r\n")));
}

//
//  SetMode
//  inputs:  none
//  outputs: none
//
SCODE IGS5000::SetMode( int modeId, HPALETTE *pPalette )
{
    SCODE sc;
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("IGS5000::SetMode(%d,0x%X)\r\n"),
			     modeId, pPalette));

    if( modeId != VGAMode[WhichMode]->gpeMode.modeId ) {
	DEBUGMSG( GPE_ZONE_ERROR,(TEXT("IGS5000::SetMode Want mode %d, only have mde %d\r\n"),
		  modeId, VGAMode[WhichMode]->gpeMode.modeId ));
	return E_INVALIDARG;
    }
    if( m_pMode ) {
	// The mode is already set, just return
	return S_OK;
    }
    m_pMode = &VGAMode[WhichMode]->gpeMode;
    m_nScreenWidth = m_pMode->width;
    m_nScreenHeight = m_pMode->height;
    if (m_nScreenWidth <= 640) {
	m_nScreenStride = (m_nScreenWidth * m_pMode->Bpp) / 8;
    } else {
	m_nScreenStride = (m_nScreenWidth * m_pMode->Bpp) / 8;
    }

    /*
	 * Video memory is treated like one-dimension (linear mode), in this way, video memory
	 * is used efficiency.
     */
	m_pLinearVideoMemory = new SurfaceHeap(m_nVideoMemorySize, (ulong) m_pLAW,0,0);

    if( FAILED( sc = AllocSurface(
			&m_pPrimarySurface,
			m_nScreenWidth,
			m_nScreenHeight,
			m_pMode->format,
			GPE_REQUIRE_VIDEO_MEMORY ) ) ) {
	return sc;
    }

	/* better align offscreen at 1K boudary */
	ulong RoundedSize = (m_nVideoMemorySize - m_pLinearVideoMemory->Available() + 0x3FF) & ~0x3FF;
    RoundedSize -= (m_nVideoMemorySize - m_pLinearVideoMemory->Available());

	if (RoundedSize) {
		SurfaceHeap *dummyheap=m_pLinearVideoMemory->Alloc(RoundedSize);
	}
    /* allocate for h/w cursor: remind that cursor pos must be 1K boundary */
    GPESurf *pCursor;
    AllocSurface( &pCursor, 1024, 1, m_pMode->format, GPE_REQUIRE_VIDEO_MEMORY );	// for cursor
	m_CurOfs = pCursor->OffsetInVideoMemory();
    m_CurPtr = (unsigned long*)(m_CurOfs + m_pLAW);

    /* allocate for pattern: remind that pattern must be in 256 byte boundary */
    GPESurf *pPat;
    AllocSurface( &pPat, 1024, 1, m_pMode->format, GPE_REQUIRE_VIDEO_MEMORY );// for pattern
    m_PatOfs = pPat->OffsetInVideoMemory();
    m_PatPtr = (unsigned long*)(m_PatOfs + m_pLAW);

#ifdef FB16BPP
    m_PatOfs = m_PatOfs>>1;
#endif

    SetModeRegisters();

    // clear the screen
    int fbrow,fbcol;
#ifdef FB16BPP
    unsigned short *pBuf = (unsigned short *)m_pPrimarySurface->Buffer();
#else	// FB16BPP
    unsigned char *pBuf = (unsigned char *)m_pPrimarySurface->Buffer();
#endif	// FB16BPP
    for( fbrow=0;fbrow<m_nScreenHeight; fbrow++ ) {
	for(fbcol=0;fbcol<m_nScreenWidth;fbcol++) {
	    *pBuf++=0;
	}
    }
    DEBUGMSG (1, (TEXT("About to call SetVisibleSurface\r\n")));
    SetVisibleSurface( (IGS5000Surf *)m_pPrimarySurface );
    DEBUGMSG (1, (TEXT("After call to SetVisibleSurface\r\n")));

    // Here, we use EngCreatePalette to create a palette that that MGDI will use as a
    // stock palette
#ifdef FB16BPP
    if( pPalette ) {
	// 15,16,24, and 32bpp primaries are defined by red,green, and blue bitfields...
	*pPalette = EngCreatePalette
			    (
				PAL_BITFIELDS,
				0,
				NULL,
				BitMasks[0],
				BitMasks[1],
				BitMasks[2]
			    );

	DEBUGMSG(1,(TEXT("Created 16 Bpp bitfield-palette, handle = 0x%08x\r\n"),*pPalette));
    }
#else	// FB16BPP

    if( pPalette ) {
	DEBUGMSG (1, (TEXT("About to create palette\r\n")));

	*pPalette = EngCreatePalette
			    (
				PAL_INDEXED,
				PALETTE_SIZE,	// i.e. 256
				(ULONG *)_rgbIdentity,
				0,
				0,
				0
			    );
	SetPalette (_rgbIdentity, 0, 256);

	DEBUGMSG(1,(TEXT("Created 8 Bpp palette, handle = 0x%08x\r\n"),*pPalette));
    }
#endif	//FB16BPP

    WaitForVBlank();

#ifdef DEBUG
    if (GPE_ZONE_INIT) {
	int i, j;

	// This should draw some random lines on the screen.

#ifdef FB16BPP
	pBuf = (unsigned short *)m_pPrimarySurface->Buffer();
#else	// FB16BPP
	pBuf = (unsigned char *)m_pPrimarySurface->Buffer();
#endif	// FB16BPP

	for (i=0; i < m_nScreenHeight; i++) {
	    for (j=0; j < m_nScreenWidth; j++) {
#ifdef FB16BPP
		if (i == 240) {
		    *(pBuf + i*m_nScreenWidth + j) = 0xFFFF ;
		} else if (j == 320) {
		    *(pBuf + i*m_nScreenWidth + j) = 0x0;
		} else {
		    *(pBuf + i*m_nScreenWidth + j) = (i | (i<<4) | (i<<8)) % 0xFFFF ;
		}
#else	// FB16BPP
		*(pBuf + i*m_nScreenWidth + j) = (i / 5) % 256 ;
#endif	// FB16BPP
	    }
	}
    }
#endif // DEBUG

    Setup2DEngine();

    //nav: set dwPhys
    m_dwPhysicalModeID = modeId;

    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Leaving IGS5000::SetMode\r\n")));
    return S_OK;
}

void IGS5000::Setup2DEngine()
{
    //setup COP after setmode
    DEBUGMSG( GPE_ZONE_INIT,(TEXT("Initialize access to m_pMMIO[]\r\n")));
    reg_XR_BIUMisc = reg_XR_BIUMisc | 0x0C;

    reg_Src_Map_Width  = m_nScreenWidth-1;
    reg_Dst_Map_Width  = m_nScreenWidth-1;
    reg_Src2_Map_Width = m_nScreenWidth-1;

#ifdef FB16BPP
    reg_Px_Map_Format  = 1;
#else  //FB16BPP
    reg_Px_Map_Format  = 0;
#endif	//FB16BPP

    //enable HostBlt range
    reg_XR_FunctionControl = reg_XR_FunctionControl|0x40;

    reg_Cop_Flags      = reg_Cop_Flags | TURBO_MONO_EN;
    m_inHostBlt = 0;
}



/*-----------------------------------------------------------------------------
  DetectVideoMem: Auto-detect video memory bus width and size.
	  1. Detect Video Memory bus width (32 or 64 bits)
	  2. Detect Video Memory size (1, 2, or 4 MB)
  In:  None
  Out: return 0: success and set register 3CE_72 and 3CE_30 accordingly.
       non-zero: fail. It means something is wrong with video memory
		       (memory chip defact or memory interface).

  Note: 1) This routine can be ignored if you know video memory size and
	   bus width. All you need to do is to set correct values into
	   3CE_72/3CE_30 in table VGAMode registers 0x72 and 0x30		 \
	2) This routine can be used to find defect video memory chips.
-----------------------------------------------------------------------------*/
int IGS5000::DetectVideoMemory(void)
{
  int i, iFlag=0;
  BYTE bData;

    reg_SR[1] = reg_SR[1] | 0x20; /*screen off*/

  /* 1. Detect SGRAM chip: 512k or 256k */
    reg_XR[0x71] = 0xC0;

    bData = 0x00;
    reg_XR_MEMCTL2 = bData;	/*force to be 32Bits/1MB*/
    reg_XR[0x30] = bData & 0x03;

    ResetSeq();

    for (i=0; i<16; i++)	   /*first clean up SGRAM pipe line garbage*/
    {
      iFlag += TestVMem(i);
    }

    iFlag = 0;
    for (i=0; i<16; i++)
    {
      iFlag += TestVMem(i);
    }

    if (iFlag == 0)
    {
      bData = 0x05;		     /*Temporarily force to be 64Bits/2MB*/
      reg_XR[0x71] = 0xA0;	     /*set to be 256K SGRAM*/
    }
    else
    {
      bData = 0x06;		     /*Temporarily force to be 64Bits/4MB*/
    }

  /* 2. Detect Video Memory Bus width */
    reg_XR_MEMCTL2 = bData;	/*Temporarily force to be 64Bits/2MB*/
    reg_XR[0x30] = bData & 0x03;

    ResetSeq();

    iFlag = 0;
    for (i=0; i<32; i++)	/*test total 32*64K - 2MB*/
    {
      iFlag += TestVMem(i);
    }

    if (iFlag == 0)
    {
      bData = 0x06;
      reg_XR_MEMCTL2 = bData;	  /*it is 64 bit. Temporarily force to be 4MB*/
      reg_XR[0x30] = bData & 0x03;
    }
    else
    {
      bData = 0x02;
      reg_XR_MEMCTL2 = bData;	  /*it is 32 bit. Temporarily force to be 4MB*/
      reg_XR[0x30] = bData & 0x03;
    }

  /* 3. Detect Video Memory Size */

    ResetSeq();

    iFlag = 0;
    for (i=0; i<64; i++)	/*test total 64*64K - 4MB*/
    {
      iFlag += TestVMem(i);
    }

    if (iFlag != 0)
    {
      bData = (reg_XR_MEMCTL2 & 0xFC) | 0x01;
      reg_XR_MEMCTL2 = bData;	  /*force to be 2MB*/
      reg_XR[0x30] = bData & 0x03;

      iFlag = 0;
      for (i=0; i<32; i++)	  /*test total 32*64Kb - 2MB*/
      {
	iFlag += TestVMem(i);
      }

      if (iFlag != 0)
      {
	bData = 0x00;
	reg_XR_MEMCTL2 = bData;      /*force to be 1MB/32bits*/
	reg_XR[0x30] = bData & 0x03;

	iFlag = 0;
	for (i=0; i<16; i++)	    /*test total 16*64Kb - 1MB*/
	{
	  iFlag += TestVMem(i);
	}
      }
    }

    reg_SR[1] = reg_SR[1] & ~0x20; /*screen on*/

    return iFlag;
}


/*-----------------------------------------------------------------------------
  ResetSeq: Reset sequencer.
  In:  None
  Out: None
-----------------------------------------------------------------------------*/
void IGS5000::ResetSeq(void)
{
    reg_RST_SYNC = 0x01;
    reg_RST_SYNC = 0x03;
}


/*-----------------------------------------------------------------------------
  TestVMem: To test 32 bytes video memory.
  In:  iBankNum: 0 - 63  (one bank = 64 KB, maximum 64 banks = 4 MB)
  Out: Return 0 if pass or non-zoro if fail
-----------------------------------------------------------------------------*/
int  IGS5000::TestVMem(int iBankNum)
{
    int i,iFlag;
    BYTE bData0, bData1;
    unsigned char *fpTmp;

    iFlag = 0;
    fpTmp = m_pLAW;

    fpTmp += iBankNum*64*1024;

    bData0 = 0x55;
    bData1 = 0xAA;
    for (i=0; i<16; i++)
    {
     *fpTmp++ = bData0;
     *fpTmp++ = bData1;
    }

    fpTmp = m_pLAW;

    fpTmp += iBankNum*64*1024;

    for (i=0; i<16; i++)
    {
     if ( (*fpTmp++ != bData0) || (*fpTmp++ != bData1) )
       iFlag = 1;
    }

    return iFlag;
}

