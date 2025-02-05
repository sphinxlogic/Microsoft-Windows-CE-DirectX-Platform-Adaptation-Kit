// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-2000 Microsoft Corporation
//	COPYRIGHT (C) 1999-2000 IGS TECHNOLOGIES, INC. (IGST)
//
//  Module Name:
//
//	gpevga.h  - IGS5000 display driver
//
// -----------------------------------------------------------------------------


#ifndef __GPEVGA_H__
#define __GPEVGA_H__

#include "ioport.h"     // definitions of the port classes used here

struct ModeInit;
struct TVRegisters;

class GPEVGA;

//
// Class GPEVGA
//

// List of I/O ports which are used to access standard VGA registers
enum EVGAPortIDs{
    PortId_3BA,
    PortId_3C0,
    PortId_3C1,
    PortId_3C2,
    PortId_3C3,
    PortId_3C4,
    PortId_3C5,
    PortId_3C6,
    PortId_3C7,
    PortId_3C8,
    PortId_3C9,
    PortId_3CA,
    PortId_3CC,
    PortId_3CE,
    PortId_3CF,
    PortId_3D0,
    PortId_3D1,
    PortId_3D2,
    PortId_3D3,
    PortId_3D4,
    PortId_3D5,
    PortId_3D6,
    PortId_3D7,
    PortId_3DA,
    PortId_46E8,
    numVGAPortIDs
};

class GPEVGA : public DDGPE
{
protected:
    PortRange	    *m_pPortRanges;	    // Required for ioport.h
    RWPort8	    reg_POS;
    RWSplitPort8    reg_MISC;		    // R:3CC,W:3C2 Miscellaneous output register
    RWSplitPort8    reg_FCR_WT; 	    // R:3CA,W:3DA Feature control register
    IndexedReg8     reg_SR;		    // Sequence registers
    IndexedReg8     reg_CR;		    // CRTC registers (and extensions)
    IndexedReg8     reg_GR;		    // Graphics controller registers
    FlipIndexedReg8 reg_AR;		    // Attribute controller registers
    IndexedReg8     reg_XR;		    // Extension registers
    IndexedReg8     reg_MR;		    // Multimedia Registers
    unsigned int    m_nTicksPerFrame;	    // E.g. 17 for 60 Hz frames, 1mS ticks
    int 	    m_nXHot;		    // Hot spot for cursor
    int 	    m_nYHot;
#ifdef TVOUT
    unsigned char   *m_pTvRegs; 	    // TV register base
    UCHAR	    m_Reg33;		    // save location for register
    UCHAR	    m_Reg3C;		    // save location for register
#endif //TVOUT
public:
		    GPEVGA();		    // Maps in standard VGA ports

    Port	    m_VGAIOPort[numVGAPortIDs];   // All IO ports used by VGA

    void	    IGSInitChip();
    void	    SetVGAMode( ModeInit *pMode );
    virtual void    WaitForVBlank();
    void	    DetermineScreenRefreshRate();
    virtual int     InVBlank();
    void	    UnlockVGA();
    void	    LockVGA();
    virtual SCODE   SetPalette(
    const PALETTEENTRY *src,
    unsigned short  firstEntry,
    unsigned short  numEntries );
    virtual void    VBlankReceived() = 0;   // Called when VBlank is entered
#ifdef TVOUT
	// NTSC TV out support routines
    VOID SetTvMode();
    VOID SetTVReg(TVRegisters *wPtr, int iCounter);
    USHORT ReadTVReg(WORD index);
    VOID WriteTVReg(WORD index, WORD data);
    VOID UnlockTVReg(VOID);
    VOID LockTVReg(VOID);
    VOID EnableTV(BOOLEAN iOnOff);
    VOID TVOn(BOOLEAN iOnOff);
    VOID SetTVColor();
    VOID BypassMode(BOOLEAN iOnOff);
    VOID SetInterpolation();
    USHORT GetBusWidth(VOID);
#endif //TVOUT
};

// register sizes (they differ on different chip sets)
    /* defines for struct StandardRegs */
#define REG_SR_SIZE	 0x5
#define REG_GR_SIZE	 0x9
#define REG_AR_SIZE	 0x15
#define REG_CR_SIZE	 0x19
#define REG_XR_SIZE	 0x8
    /* defines for struct ExtendedRegs */
#define INIT_REG_XR_SIZE 0x34
#define INIT_REG_SG_SIZE 0x10


typedef struct _REG_INDEX_DATA {
    unsigned char   Index;	// register index
    unsigned char   Data;	// register data
} REG_INDEX_DATA, *PREG_INDEX_DATA;


/*
  Standard VGA register settings for specific resoultion and refresh rate.
  All color depth (8/16/24 bpp) use same StandardRegs.
*/
typedef struct _StandardRegs {
    unsigned char   Init_SR[REG_SR_SIZE]; // Sequence Registers      (index port 3C4, data port 3C5)
    unsigned char   Misc;		  // Misc Output Reg	     (read port 3CC, write port 3CC)
    unsigned char   Init_CR[REG_CR_SIZE]; // CRT Control Registers   (index port 3CE, data port 3CF)
    unsigned char   Init_AR[REG_AR_SIZE]; // Attribute Registers     (index port 3C0)
    unsigned char   Init_GR[REG_GR_SIZE]; // Graphic Controller Regs (index port 3D4, data port 3D5)
} StandardRegs;

/*
  IGS extended register settings for specific resoultion and refresh rate.
  These are the only registers which vary for different color depth (8/16/24 bpp)
*/
typedef struct _ExtendedRegs {
    REG_INDEX_DATA  Init_CR[1]; 	  // Crtc Reg_13
    REG_INDEX_DATA  Init_XR[REG_XR_SIZE]; // Extension Registers
} ExtendedRegs;

struct ModeInit
{
    GPEMode		gpeMode;
    StandardRegs	*std;		  //VGA standard registers
    ExtendedRegs	*ext;		  //IGS extended registers
};


#define REG_TV_SIZE	123*2
/*
  TV Register settings for specific resoultion and TV type (NTSC or PAL)
  All color depth (8/16/24 bpp) use same TV Registers.
*/
struct TVRegisters {
      USHORT Init_TV[REG_TV_SIZE];
};


#ifdef FB16BPP
typedef enum {MODE_640x480x16,MODE_640x440x16,MODE_640x480x16_U,\
     MODE_640x480x16_PAL,MODE_720x540x16_PAL,MODE_800x600x16_PAL, TOTAL_MODES} MODENUMBER;
#else
typedef enum {MODE_640x440x8, MODE_640x480x8, MODE_640x480x8_U,\
     MODE_640x480x8_PAL,MODE_720x540x8_PAL,MODE_800x600x8_PAL, TOTAL_MODES} MODENUMBER;
#endif


// Direct access IO mapped registers

#define reg_AR_ADDR	    (PORT_RW_8 m_VGAIOPort[PortId_3C0 ]) //Select AR0..14.  On Writes, sets 3C0[write]->ATR_DATA
#define reg_AR_DATA	    (PORT_RW_8 m_VGAIOPort[PortId_3C1 ]) //Read/Write AR0..14 On Writes, sets 3C0[write]->ATR_AD
#define reg_AR_RESET	    (PORT_RO_8 m_VGAIOPort[PortId_3DA ]) //Reset flip flop to set3CO[write]->ATR_AD
#define reg_CR_ADDR	    (PORT_RW_8 m_VGAIOPort[PortId_3D4 ]) //Select CR0..CR24
#define reg_CR_DATA	    (PORT_RW_8 m_VGAIOPort[PortId_3D5 ]) //Read/Write CR0..CR24

#define reg_XR_ADDR	    (PORT_RW_8 m_VGAIOPort[PortId_3CE ]) //Select XR0..XRFF
#define reg_XR_DATA	    (PORT_RW_8 m_VGAIOPort[PortId_3CF ]) //Read/Write XR0..XRFF

#define reg_MR_WRITE	    (PORT_WO_8 m_VGAIOPort[PortId_3C2 ]) // MISC register
#define reg_MR_READ	    (PORT_RO_8 m_VGAIOPort[PortId_3CC ])

#define reg_DAC_AD_MK	    (PORT_RW_8 m_VGAIOPort[PortId_3C6 ]) //Pixel read mask (put FF here!)
#define reg_DAC_DATA	    (PORT_RW_8 m_VGAIOPort[PortId_3C9 ]) //Access palette entry (use 3 times)
#define reg_DAC_RD_AD	    (PORT_WO_8 m_VGAIOPort[PortId_3C7 ]) //Read index into palette (auto-incr)
#define reg_DAC_STS	    (PORT_RO_8 m_VGAIOPort[PortId_3C7 ]) //DAC status - checks palette state
#define reg_DAC_WR_AD	    (PORT_WO_8 m_VGAIOPort[PortId_3C8 ]) //Write index into palette (auto-incr)
#define reg_GR_ADDR	    (PORT_RW_8 m_VGAIOPort[PortId_3CE ]) //Select GR0..8
#define reg_GR_DATA	    (PORT_RW_8 m_VGAIOPort[PortId_3CF ]) //Access GR0..8
#define reg_SR_ADDR	    (PORT_RW_8 m_VGAIOPort[PortId_3C4 ]) //Select SR0..18
#define reg_SR_DATA	    (PORT_RW_8 m_VGAIOPort[PortId_3C5 ]) //Read/Write SR0..18
#define reg_STATUS_0	    (PORT_RO_8 m_VGAIOPort[PortId_3C2 ]) //CRT Irq Pending
#define reg_STATUS_1	    (PORT_RO_8 m_VGAIOPort[PortId_3DA ]) //VSync Status


// Indexed registers and their aliases

#define reg_RST_SYNC	    reg_SR[0x0]     //	R/W reset - UNUSED!
#define reg_CLK_MODE	    reg_SR[0x1]     //	R/W dot clk, chr clk, screen_off
#define reg_EN_WT_PL	    reg_SR[0x2]     //	R/W enable cpu mem writes
#define reg_CH_FONT_SL	    reg_SR[0x3]     //	R/W font select
#define reg_MEM_MODE	    reg_SR[0x4]     //	R/W Mem access control
#define reg_UNLK_EXSR	    reg_SR[0x8]     //	R/W Enable SR9..SR18

#define reg_H_TOTAL	    reg_CR[0x0]     //	R/W Total character clocks per line
#define reg_H_D_END	    reg_CR[0x1]     //	R/W Active char clocks per line
#define reg_S_H_BLANK	    reg_CR[0x2]     //	R/W Char no of start of blank period
#define reg_E_H_BLANK	    reg_CR[0x3]     //	R/W End hBlank and display skew
#define reg_S_H_SY_P	    reg_CR[0x4]     //	R/W Start horizontal sync position
#define reg_E_H_SY_P	    reg_CR[0x5]     //	R/W End horizontal sync position
#define reg_V_TOTAL	    reg_CR[0x6]     //	R/W Vertical total in scan lines
#define reg_OVFL_REG	    reg_CR[0x7]     //	R/W Overflow reg - bit 8 &9’s of CR6,12,10,15,18
#define reg_P_R_SCAN	    reg_CR[0x8]     //	R/W Preset row scan - for soft scrolling
#define reg_MAX_S_LN	    reg_CR[0x9]     //	R/W Scan lines / char -1, Bit 9 of CR15,18. Double-scan enable
#define reg_CSSL	    reg_CR[0xA]     //	R/W Cursor start row
#define reg_CESL	    reg_CR[0xB]     //	R/W Cursor end row
#define reg_STA_H	    reg_CR[0xC]     //	R/W Start addr high
#define reg_STA_L	    reg_CR[0xD]     //	R/W Start addr low
#define reg_CLA_H	    reg_CR[0xE]     //	R/W Cursor addr high
#define reg_CLA_L	    reg_CR[0xF]     //	R/W Cursor addr low
#define reg_VRS 	    reg_CR[0x10]    //	R/W Vertical retrace start
#define reg_VRE 	    reg_CR[0x11]    //	R/W Vertical retrace end
#define reg_VDE 	    reg_CR[0x12]    //	R/W Vertical display end
#define reg_SCREEN_OFFSET   reg_CR[0x13]    //	R/W Screen pitch / 2,4,or8  - see CR51
#define reg_ULL 	    reg_CR[0x14]    //	R/W Underline row & char count mode
#define reg_SVB 	    reg_CR[0x15]    //	R/W Start vertical blank
#define reg_EVB 	    reg_CR[0x16]    //	R/W End vertical blank
#define reg_CRT_MD	    reg_CR[0x17]    //	R/W CRTC mode, and sync disable
#define reg_LCM 	    reg_CR[0x18]    //	R/W Split screen address
#define reg_GCCL	    reg_CR[0x22]    //	R CPU Latch 0..3  [via GR4 1:0]
#define reg_ATC_F	    reg_CR[0x24]    //	R Attribute controller index

#define reg_SET_RST_DT	    reg_GR[0x0]     //	R/W Value written in write modes 0 & 3
#define reg_EN_S_R_DT	    reg_GR[0x1]     //	R/W Bit write enables for write mode 0
#define reg_COLOR_CMP4Bpp   reg_GR[0x2]     //	R/W Color compare for read mode 1
#define reg_WT_ROP	    reg_GR[0x3]     //	R/W Raster op & rotate for writes
#define reg_RD_PL_SL	    reg_GR[0x4]     //	R/W Read plane select (0..3)
#define reg_GRP_MODE	    reg_GR[0x5]     //	R/W Select write mode (0..3), enable read compare, odd/even, and 256-color support
#define reg_MISC_GM	    reg_GR[0x6]     //	R/W Text/graphics & Memory addr range
#define reg_CMP_DNTC	    reg_GR[0x7]     //	R/W Color don’t care bits for read mode 1
#define reg_BIT_MASK	    reg_GR[0x8]     //	R/W Write mask

#define reg_PLT_REG	    reg_AR	    //	R/W 16 6-bit palette registers
#define reg_ATR_MODE	    reg_AR[0x10]    //	R/W Attribute mode control
#define reg_BDR_CLR	    reg_AR[0x11]    //	R/W Border color 8-bit PA modes only
#define reg_DISP_PLN	    reg_AR[0x12]    //	R/W Color plane enable
#define reg_H_PX_PAN	    reg_AR[0x13]    //	R/W Horiz pixel panning
#define reg_PX_PAD	    reg_AR[0x14]    //	R/W Pixel padding (converts to 8Bpp)

#endif // __GPEVGA_H__
