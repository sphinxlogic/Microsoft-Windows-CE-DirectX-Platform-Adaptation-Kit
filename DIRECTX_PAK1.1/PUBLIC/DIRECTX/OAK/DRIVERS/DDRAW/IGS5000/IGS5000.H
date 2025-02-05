// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-2000 Microsoft Corporation
//	COPYRIGHT (C) 1999-2000 IGS TECHNOLOGIES, INC. (iGST)
//
//  Module Name:
//
//	igs5000.h  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#ifdef _X86_
#define COMMANDPARAM  //this will support command parameter passed in from loadcepc.exe
#endif

#ifndef __IGS5000_H__
#define __IGS5000_H__

#ifdef PCI_BUS
#else
#define PHYSICAL_LINEAR_FRAME_BUFFER 0x02000000    /*for VL bus linear frame buffer*/
#endif

#include "gpevga.h"

#define DBG_SLEEP   0

    // TV registers located in the memory space
#define TV_MEMORY_OFFSET  (0x000B0000 | 0x00800000)
    // size of the TV register map
#define TV_REG_SIZE	0xE52C

    // accelerator registers located in the memory space
#define ACCEL_MEMORY_OFFSET  (0x000BF000 | 0x00800000)
    // size of the accelerator register map
#define ACCEL_REG_SIZE	   0x0200

#define HOSTMEM_SIZE	0x10000
#define HOSTMEM_OFFSET	0x8A0000

class IGS5000Surf;

class IGS5000  : public GPEVGA {

private:
#if DDHAL
public:     // temporarily moved - will fix later
#endif
    // Ports and memory ranges	used to access registers
#ifdef ENABLE_ACCELERATION
    SubIndexedRegArray16 reg_BEE8;			// Indexed accelerator registers
#endif

    unsigned char   *m_pLAW;				// Linear access window
    int 			m_nVideoMemorySize;	// Size in bytes of video RAM total
    //Node2D			*m_p2DVideoMemory;	    // Base entry representing all video memory
	SurfaceHeap		*m_pLinearVideoMemory;	// Base entry representing all video memory
    int 			m_nScreenStride;	    // Stride of 2d memory in bytes
    unsigned int    m_nTicksAtFlip;			// TickCount at time flip was requested
    unsigned int    m_nTicksAtResync;	    // TickCount when we last waited for VBlank
    IGS5000Surf     *m_pOldVisibleSurface;  // Visible surface before flip
    IGS5000Surf     *m_pNewVisibleSurface;  // Visible surface after flip
    unsigned long   m_nLAWPhysical;

public:
    unsigned char   *m_pMMIO;				// acceleration Memory Mapped IO range

    // Contructor: CONFIG.CPP
	IGS5000();
    void			DumpVGARegs();
    virtual SCODE   SetMode( int modeNo, HPALETTE *pPalette );
    void			SetModeRegisters();
    void			SetClock();
    void			Setup2DEngine();

    virtual SCODE   GetModeInfo(
						GPEMode *pMode,
						int modeNo );
    virtual int     NumModes();

    int 			DetectVideoMemory(void);
    void			ResetSeq(void);
    int 			TestVMem(int iBankNum);
    void			BeginHBlt();
    void			EndHBlt();
    SCODE			AcceleratedTextBlt(GPEBltParms *pBltParms);
    SCODE			AcceleratedClr2ScrBlt(GPEBltParms *pBltParms);
    SCODE			AcceleratedMono2ScrBlt(GPEBltParms *pBltParms);
    SCODE			AcceleratedColorPatBlt(GPEBltParms *pBltParms);
    int 			m_inHostBlt;
    unsigned long   *m_CurPtr;				// point to h/w cursor data
    unsigned long   m_CurOfs;				// offset in videomem
    unsigned long   *m_PatPtr;				// point to pattern data
    unsigned long   m_PatOfs;				// offset in videomem
    unsigned long   *m_pHostMem;			// hostblt memory
	unsigned long	m_ScrOfs;				// offset to free memory that we can use.
    GPEMode			m_ModeInfo;			// For multiple resolution support

    // Cursor.cpp:
    virtual SCODE   SetPointerShape(
						GPESurf *pMask,
						GPESurf *pColorSurf,
						int xHot,
						int yHot,
						int cx,
						int cy );
    virtual SCODE   MovePointer(
						int x,
						int y );
    void			SetCursorColors(
						unsigned long foreground,   // 0x00bbggrr format
						unsigned long background );

#ifdef TVOUT
    void			DetectUnderscan(void);

    int 			wV_Underscan;
    int 			iMaxLines;	   //# of scan lines within which we drop one scan line
    int 			iWhichLine;	   //which scan line we drop
#endif

    // Misc.cpp:
    virtual void    WaitForNotBusy();
    virtual int     IsBusy();
    void			WaitForFIFO( int numParameters );
#ifdef ENABLE_ACCELERATION
    void			SelectSolidColor( unsigned long color );
#endif
    virtual void    GetPhysicalVideoMemory(
						unsigned long *pPhysicalMemoryBase,
						unsigned long *pVideoMemorySize );

    virtual ULONG DrvEscape(  SURFOBJ *pso,
			      ULONG    iEsc,
			      ULONG    cjIn,
			      PVOID    pvIn,
			      ULONG    cjOut,
			      PVOID    pvOut);


    // Surf.cpp:
    virtual SCODE   AllocSurface(
						GPESurf **ppSurf,
						int width,
						int height,
						EGPEFormat format,
						int surfaceFlags );

    virtual SCODE   AllocSurface(
						DDGPESurf **ppSurf,
						int width,
						int height,
					    EGPEFormat format,
						EDDGPEPixelFormat pixelFormat,
						int surfaceFlags );

    void			CheckVisibleSurface();
    virtual int     SurfaceBusyFlipping( IGS5000Surf *pSurf );
    virtual int     FlipInProgress();
    virtual void    SetVisibleSurface( GPESurf *pSurf );
    virtual void    SetVisibleSurface(GPESurf *pSurf, BOOL bWaitForVBlank);
    virtual void    VBlankReceived();	// Called when VBlank is entered

    // Line.cpp:
    virtual SCODE   Line(
						GPELineParms *pLineParms,
						EGPEPhase phase );

    SCODE			AcceleratedSolidLine( GPELineParms *pLineParms );

    // Blt.cpp:
    virtual SCODE   BltPrepare( GPEBltParms *pBltParms );
    virtual SCODE   BltComplete( GPEBltParms *pBltParms );
    SCODE			AcceleratedFillRect(GPEBltParms *pBltParms );
    SCODE			AcceleratedSrcCopyBlt(GPEBltParms *pBltParms );
#ifdef ENABLE_FILL_EMULATION
    SCODE			EmulatedFillRect(GPEBltParms *pBltParms );
#endif
#ifdef ENABLE_TEXT_EMULATION
    SCODE			EmulatedTextBlt(GPEBltParms *pBltParms );
#endif
#ifdef ENABLE_SRC_COPY_EMULATION
    SCODE			EmulatedSrcCopyBlt(GPEBltParms *pBltParms );
#endif

};

class IGS5000Surf : public DDGPESurf {
private:
	SurfaceHeap		*m_pSurfHeap;
	WORD			m_SurfWidth;
	ULONG			m_CopOfs;

public:
	/* constructor */
	IGS5000Surf(							// Create video-memory surface
		int width,
		int height,
		unsigned long offset,
		void *pBits,						// virtual address of allocated bits
		int stride,
		EGPEFormat format,
		SurfaceHeap *pHeap );

	IGS5000Surf(							// Create video-memory surface
		int width,
		int height,
		unsigned long offset,
		void *pBits,						// virtual address of allocated bits
		int stride,
		EGPEFormat format,
		EDDGPEPixelFormat pixelFormat,
		SurfaceHeap *pHeap );

	virtual 		~IGS5000Surf();
	int				SurfWidth() {
						return m_SurfWidth;
					}

	int				CopOfs() {
						return m_CopOfs;
					}

};


// Command types:

//#define DRAW_LINE			  0x02000
//#define RECTANGLE_FILL		  0x04000
//#define POLYGON_SOLID 		  0x06000
//#define FOUR_POINT_TRAPEZOID_SOLID	  0x08000
//#define BRESENHAM_TRAPEZOID_SOLID	  0x0A000
//#define CMD_BITBLT			  0x0C000
//#define PATTERN_FILL			  0x0E000
//#define CMD_POLYLINE			  0x02800
//#define POLYGON_PATTERN		  0x06800
//#define FOUR_POINT_TRAPEZOID_PATTERN	  0x08800
//#define BRESENHAM_TRAPEZOID_PATTERN	  0x0A800
//#define ROPBLT			  0x0C800


// Direct access non-standard VGA/IGS5000 IO mapped registers
#define reg_SETUP_MD	    (PORT_WO_8 m_VGAIOPort[PortId_46E8])    //Bit3:Addr decode enableBit4:Setup mode
#define reg_SETUP	    (PORT_RW_8 m_VGAIOPort[PortId_3C3]) //Sleep mode (was optionally 46E8 in old revs of chip)
#define reg_RAMDACMASK	    (PORT_RW_8 m_VGAIOPort[PortId_3C6])

// Indexed registers and their aliases
#define reg_MODE_CTL	    reg_CR[0x42]    //	R/W Interlace enable
#define reg_EXT_MODE	    reg_CR[0x43]    //	R/W Bit 8 of CR13, and Hor Ctr x2

#define reg_PIXEL_PIPE0     reg_XR[0x80]    //	R/W Pixel Pipeline Configuration 0

#define reg_CURSOR1_VERTEXT reg_XR[0xA1]    //	R/W Cursor 1 Veritcal Extension Register

#define reg_CURSOR1_XLOW    reg_XR[0x50]    //	R/W Cursor 1 X position low
#define reg_CURSOR1_XHI     reg_XR[0x51]    //	R/W Cursor 1 X position high
#define reg_CURSOR1_XPRESET reg_XR[0x52]    //	R/W Cursor 1 X horz. preset
#define reg_CURSOR1_YLOW    reg_XR[0x53]    //	R/W Cursor 1 Y position low
#define reg_CURSOR1_YHI     reg_XR[0x54]    //	R/W Cursor 1 Y position high
#define reg_CURSOR1_YPRESET reg_XR[0x55]    //	R/W Cursor 1 Y vert.. preset
#define reg_CURSOR1_CONTROL reg_XR[0x56]    //	R/W Cursor 1 Control Register, Sprite Control


#define reg_XR_CRTVertCount0	reg_XR[0x18]	//  R/W CRT vertical counter 0
#define reg_XR_CRTVertCount1	reg_XR[0x19]	//  R/W CRT vertical counter 1
#define reg_XR_BIUMisc		reg_XR[0x33]	//  R/W misc. BIU bits
#define reg_XR_ExtLinearAddress reg_XR[0x35]	//  R/W frame buffer high bits of address
#define reg_XR_FunctionControl	reg_XR[0x3C]	//  R/W port enable, endian
#define reg_XR_X2VideoDisplayVertEnd reg_XR[0x4E] //  R/W X2 video display vertical ending line high
#define reg_XR_SpriteControl	reg_XR[0x56]	//  R/W sprite control & RAMDAC select
#define reg_XR_Ramdac		reg_XR[0x5C]	//  R/W RAMDAC ?
#define reg_XR_MEMCTL2		reg_XR[0x72]	//  R/W memory controller 2
#define reg_CURSOR1_ADDRLOW	reg_XR[0x7E]	//  R/W Cursor 1 base address low
#define reg_CURSOR1_ADDRHI	reg_XR[0x7F]	//  R/W Cursor 1 base address high
#define reg_XR_ExtMiscClock	reg_XR[0xB9]	//  R/W extended misc clock
#define reg_XR_ExtMClockNum0	reg_XR[0xB2]	//  R/W extended M Clock numerator 0
#define reg_XR_ExtMClockNum1	reg_XR[0xB3]	//  R/W extended M Clock numerator 1
#define reg_XR_ExtMiscClock	reg_XR[0xB9]	//  R/W extended misc clock
#define reg_XR_IndirectIndexFor4x reg_XR[0xFA]	//  R/W indirect index for 4x


// memory mapped acceleration registers
#define reg_Page_Base		(MM_RW_32 m_pMMIO[0x0000])
#define reg_Virtual_Addr	(MM_RW_32 m_pMMIO[0x0004])
#define reg_Reserved08		(MM_RW_16 m_pMMIO[0x000E])
#define reg_State_A_Len 	(MM_RW_8  m_pMMIO[0x000C])
#define reg_State_B_Len 	(MM_RW_8  m_pMMIO[0x000D])
#define reg_Reserved0E		(MM_RW_16 m_pMMIO[0x000E])
#define reg_Reserved10		(MM_RW_16 m_pMMIO[0x0010])
#define reg_Cop_control 	(MM_RW_8  m_pMMIO[0x0011])
#define reg_Px_Map_Index	(MM_RW_8  m_pMMIO[0x0012])
#define reg_Reserved13		(MM_RW_16 m_pMMIO[0x0013])
#define reg_Px_Map_Base 	(MM_RW_32 m_pMMIO[0x0014])
#define reg_Src_Map_Width	(MM_RW_16 m_pMMIO[0x0018])
#define reg_Src2_Map_Width	(MM_RW_16 m_pMMIO[0x0118])
#define reg_Dst_Map_Width	(MM_RW_16 m_pMMIO[0x0218])
#define reg_Px_Map_Height	(MM_RW_16 m_pMMIO[0x001A])
#define reg_Px_Map_Format	(MM_RW_8  m_pMMIO[0x001C])
#define reg_Reserved1D		(MM_RW_16 m_pMMIO[0x001D])
#define reg_Reserved1E		(MM_RW_16 m_pMMIO[0x001E])
#define reg_Reserved1F		(MM_RW_16 m_pMMIO[0x001F])
#define reg_Bres_Err_Term	(MM_RW_16 m_pMMIO[0x0020])
#define reg_Reserved22		(MM_RW_16 m_pMMIO[0x0022])
#define reg_Bres_K1		(MM_RW_16 m_pMMIO[0x0024])
#define reg_Reserved26		(MM_RW_16 m_pMMIO[0x0026])
#define reg_Bres_K2		(MM_RW_16 m_pMMIO[0x0028])
#define reg_Reserved2A		(MM_RW_16 m_pMMIO[0x002A])

#define reg_Direction_Step	(MM_RW_32 m_pMMIO[0x002C])
#define reg_Direction_StepL	(MM_RW_16 m_pMMIO[0x002C])
#define reg_Direction_StepH	(MM_RW_16 m_pMMIO[0x002C+0x2]);

#define reg_Reserved30		(MM_RW_16 m_pMMIO[0x0030])
#define reg_Reserved34		(MM_RW_16 m_pMMIO[0x0034])
#define reg_Reserved38		(MM_RW_16 m_pMMIO[0x0038])
#define reg_Reserved3C		(MM_RW_16 m_pMMIO[0x003C])
#define reg_Reserved40		(MM_RW_16 m_pMMIO[0x0040])
#define reg_Reserved44		(MM_RW_16 m_pMMIO[0x0044])
#define reg_Fore_Mix		(MM_RW_8  m_pMMIO[0x0048])
#define reg_Back_Mix		(MM_RW_8  m_pMMIO[0x0049])
#define reg_Dest_Color_Comp	(MM_RW_8  m_pMMIO[0x004A])
#define reg_Reserved4B		(MM_RW_8  m_pMMIO[0x004B])
#define reg_Color_Comp_Val	(MM_RW_32 m_pMMIO[0x004C);
#define reg_Px_Bit_Mask 	(MM_RW_32 m_pMMIO[0x0050])
#define reg_Carry_Chain_Mask	(MM_RW_32 m_pMMIO[0x0054])
#define reg_Fore_Color		(MM_RW_32 m_pMMIO[0x0058])
#define reg_Back_Color		(MM_RW_32 m_pMMIO[0x005C])
#define reg_Op_Dimension_1	(MM_RW_16 m_pMMIO[0x0060])
#define reg_Op_Dimension_2	(MM_RW_16 m_pMMIO[0x0062])
#define reg_Reserved64		(MM_RW_16 m_pMMIO[0x0064])
#define reg_Reserved68		(MM_RW_16 m_pMMIO[0x0068])
#define reg_Mask_Map_X		(MM_RW_16 m_pMMIO[0x006C])
#define reg_Mask_Map_Y		(MM_RW_16 m_pMMIO[0x006E])
#define reg_Src_Base_Ptr	(MM_RW_32 m_pMMIO[0x0070])
#define reg_Pattern_Map_X	(MM_RW_16 m_pMMIO[0x0074])
#define reg_Pattern_Map_Y	(MM_RW_16 m_pMMIO[0x0076])
#define reg_Dst_X_Phase 	(MM_RW_8  m_pMMIO[0x0078])
#define reg_Dst_Y_Phase 	(MM_RW_8  m_pMMIO[0x007A])
#define reg_Pat_Y_Phase 	(MM_RW_8  m_pMMIO[0x007A])
#define reg_Pat_X_Phase 	(MM_RW_8  m_pMMIO[0x0150])

#define reg_SLine_RoOv		(MM_RW_8  m_pMMIO[0x0134])
#define reg_SLine_Incr		(MM_RW_8  m_pMMIO[0x0135])
#define reg_SLine_Pttn		(MM_RW_8  m_pMMIO[0x0136])
#define reg_SLine_Accu		(MM_RW_8  m_pMMIO[0x0137])
#define reg_SLine_Indx		(MM_RW_8  m_pMMIO[0x0138])

#define reg_Src_Start_Ptr	(MM_RW_32 m_pMMIO[0x0170])
#define reg_Dst_Start_Ptr	(MM_RW_32 m_pMMIO[0x0178])

#define reg_Px_Op		(MM_RW_32 m_pMMIO[0x007C])
#define reg_Px_OpL		(MM_RW_16 m_pMMIO[0x007C])
#define reg_Px_OpH		(MM_RW_16 m_pMMIO[0x007C+0x2])


#define  Mix_S		    0x03
#define  Right_Down	    0x00000000
#define  Left_Down	    0x00000004
#define  Right_Up	    0x00000002
#define  Left_Up	    0x00000007	  //for PxBlt direction
#define  Fore_Src_color     0x00000000
#define  Back_Src_color     0x00000000
#define  Fore_Src_PxMap     0x20000000
#define  Back_Src_PxMap     0x80000000
#define  PxBlt		    0x08000000
#define  PxBlt_Invert	    0x09000000
#define  PxBlt_Area_Fill    0x0A000000
#define  Pattern_PxMap_A    0x00001000
#define  Pattern_PxMap_B    0x00002000
#define  Pattern_PxMap_C    0x00003000
#define  Pattern_Foreground 0x00008000
#define  Pattern_From_Src   0x00009000

/* rop2 definition */
#define  Mix_Blackness	    0x00
#define  Mix_Whiteness	    0x0F


/* cop registers */
#define reg_Src2_Start_Ptr	(MM_RW_32 m_pMMIO[0x0174])
#define reg_Cop_Flags		(MM_RW_8  m_pMMIO[0x0130])
	#define TURBO_MONO_EN	0x10
#define reg_Mono_Size		(MM_RW_16 m_pMMIO[0x013C])



/* more cop command flags */
	#define Pos_DZ			0x00000000
	#define Neg_DZ			0x00000001
	#define Pos_DY			0x00000000
	#define Neg_DY			0x00000002	//for LINEDRAW
	#define Pos_DX			0x00000000
	#define Neg_DX			0x00000004	//for LINEDRAW

	#define Trans_is_SRC1		0x00000000
	#define Trans_iS_SRC2		0x00000100
	#define Trans_is_DST		0x00000200
	#define Trans_Invert		0x00000400
	#define Trans_Enable		0x00000800

	#define Opaque_Textout		0x00001000
	#define Opaque_MonoPattern	0x00002000
	#define Opaque_Line		0x00003000
	#define Transparent_Textout	0x00005000
	#define Transparent_MonoPattern 0x00006000
	#define Transparent_Line	0x00007000

	#define HBlt_Read		0x00010000
	#define HBlt_Write		0x00020000
	#define HBlt_Write2		0x00030000
	#define LineDraw		0x05000000
	#define TerBlt			0x0B000000
	#define SRC2_is_OpaqueMono	0x00100000
	#define SRC2_is_TransMono	0x00500000


	//common cop command
	#define COP_USE_TRANSSRC1	Trans_is_SRC1 | Trans_Invert | Trans_Enable
	#define COP_USE_TRANSDST	Trans_is_DST  | Trans_Invert | Trans_Enable
	#define COP_DRAWLINE		LineDraw | Fore_Src_color | Pattern_Foreground
	#define COP_FILLRECT		PxBlt | Pattern_Foreground | Fore_Src_color
	#define COP_COLORPAT		Fore_Src_PxMap | PxBlt | Pattern_From_Src
	#define COP_CLR2SCR		Fore_Src_PxMap | PxBlt | Pattern_Foreground | HBlt_Write
	#define COP_SCR2SCR		Fore_Src_PxMap | PxBlt | Pattern_Foreground
	#define COP_OPAQUEMON2SCR	PxBlt | Transparent_Textout
	#define COP_TRANSMON2SCR	PxBlt | Opaque_Textout
	#define COP_OPAQUETEXTOUT	Fore_Src_PxMap | TerBlt | SRC2_is_OpaqueMono| HBlt_Write2 | Pattern_Foreground
	#define COP_TRANSTEXTOUT	Fore_Src_PxMap | TerBlt | SRC2_is_TransMono | HBlt_Write2 | Pattern_Foreground


/*
 * For CPUs which requires dw/w/b aligment when access dw/w/b (such as MIPS)
 * we better read in byte, then convert to correct.
 */
#ifndef _X86_
#define MAKE_DWORD(x)		((((ulong)*(x+3))<<24) | \
							 (((ulong)*(x+2))<<16) | \
							 (((ulong)*(x+1))<<8 ) | \
							 (((ulong)*(x  ))    ) )

#define MAKE_24BITS(x)		((((ulong)*(x+2))<<16) | \
							 (((ulong)*(x+1))<<8 ) | \
							 (((ulong)*(x  ))    ) )

#define MAKE_WORD(x)		((((unsigned short)*(x+1))<<8) | \
							 (((unsigned short)*(x	))   ))

#else
#define MAKE_DWORD(x)		*(ulong*)x
#define MAKE_24BITS(x)		*(ulong*)x
#define MAKE_WORD(x)		*(ushort*)x
#endif

#endif __IGS5000_H__


