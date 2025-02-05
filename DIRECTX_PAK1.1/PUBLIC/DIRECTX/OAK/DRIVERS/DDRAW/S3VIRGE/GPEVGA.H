/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-2000  Microsoft Corporation

Module Name:    gpevga.h

Abstract:  

Notes: 
--*/

#ifndef __GPEVGA_H__
#define __GPEVGA_H__

#include "ioport.h"     // definitions of the port classes used here

struct ModeInit;
class GPEVGA;

//
// Class GPEVGA
//
// The GPEVGA class is intended to ease the development of DDGPE device 
// drivers for graphics cards based on VGA. Classes for SVGA devices can 
// be derived from the GPEVGA class which takes care of VGA configuration 
// and provides a number of useful register mappings to make the remainder 
// of the class definition relatively easy.
//
// Note that this is a base class and contains pure virtual functions 
// which must be implemented for derived concrete classes
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
    PortId_3D4,
    PortId_3D5,
    PortId_3DA,
    numVGAPortIDs
};

class GPEVGA : public DDGPE
{
protected:
    PortRange       *m_pPortRanges;     // Required for ioport.h
    RWSplitPort8    reg_MISC;           // R:3CC,W:3C2 Misc output register
    RWSplitPort8    reg_FCR_WT;         // R:3CA,W:3DA Feature control register
    IndexedReg8     reg_SR;             // Sequence registers
    IndexedReg8     reg_CR;             // CRTC registers (and extensions)
    IndexedReg8     reg_GR;             // Graphics controller registers
    FlipIndexedReg8 reg_AR;             // Attribute controller registers
    unsigned int    m_nTicksPerFrame;   // E.g. 17 for 60 Hz frames, 1mS ticks
    int             m_nXHot;            // Hot spot for cursor
    int             m_nYHot;
    unsigned char   m_nPos_high;        // LAW read from Chip
    unsigned char   m_nPos_low;         // LAW read from Chip

#ifdef ENABLE_INTR
    UINT            m_nIntrLine;        // interrupt line rountine info
#endif // ENABLE_INTR

public:
    int             m_nModeIndex;       // index of the startup display mode
                                        // in VGAModeP[]
    Port            m_VGAIOPort[numVGAPortIDs];   // All IO ports used by VGA

                    GPEVGA();           // Maps in standard VGA ports
    void            SetVGAMode(ModeInit *pMode, int memSize);
    virtual void    WaitForVBlank();
    void            DetermineScreenRefreshRate();
    virtual int     InVBlank();
    void            UnlockVGA();
    void            LockVGA();
    virtual SCODE   SetPalette(
                        const PALETTEENTRY *src,
                        unsigned short firstEntry,
                        unsigned short numEntries 
                    );
};

#define SIZE_SR0_SR4    5
#define SIZE_SR10_SR13  4
#define SIZE_GR0_GR8    9
#define SIZE_AR0_AR14   0x15
#define SIZE_CR0_CR18   0x19
#define SIZE_CR31_CR3C  0x0C
#define SIZE_CR40_CR49  0x0A
#define SIZE_CR50_CR6A  0x1B

struct ModeInit
{
    GPEModeEx gpeModeEx;
    unsigned char Init_SR0_SR4[SIZE_SR0_SR4];
    unsigned char Init_SR10_SR13[SIZE_SR10_SR13];
    unsigned char Init_GR0_GR8[SIZE_GR0_GR8];
    unsigned char Init_AR0_AR14[SIZE_AR0_AR14];
    unsigned char Init_CR0_CR18[SIZE_CR0_CR18];
    unsigned char Init_CR31_CR3C[SIZE_CR31_CR3C];
    unsigned char Init_CR40_CR49[SIZE_CR40_CR49];
    unsigned char Init_CR50_CR6A[SIZE_CR50_CR6A];
};

#define NUMVGAMODES 2
extern struct ModeInit *VGAMode;

#define INSTANTIATE_MODE_INIT                                           \
struct ModeInit VGAModeP[2] =                                         \
{                                                                       \
    {                                                                   \
        /*  mode#0: 640 x 480  16Bpp  60Hz VESA mode 0x111 */           \
        { 0, 640, 480, 16, 60, gpe16Bpp,                                \
          sizeof(GPEModeEx), 0, GPEMODEEX_CURRENTVERSION, 0, 0, 0, 0,   \
          ddgpePixelFormat_565, 0, 0, 640 * 16 / 8, 0,                  \
          0xF800, 0x07E0, 0x001F, 0x0000 },                             \
        {   /*  Init_SR0_SR4 */                                         \
            0x03,   /* SR0 Reset */                                     \
            0x01,   /* SR1 Clocking Mode */                             \
            0x0F,   /* SR2 Enable Write Plane */                        \
            0x00,   /* SR3 Character Font Select */                     \
            0x0E    /* SR4 Memory Mode Control */                       \
        },                                                              \
        {   /*  Init_SR10_SR13 */                                       \
            0x41,   /* SR10 MCLK Value Low  */                          \
            0x2C,   /* SR11 MCLK Value High */                          \
            0x61,   /* SR12 DCLK Value Low  */                          \
            0x28    /* SR13 DCLK Value High */                          \
        },                                                              \
        {   /*  Init_GR0_GR8 */                                         \
            0x00,   /* GR0 Set/Reset */                                 \
            0x00,   /* GR1 Enable Set/Reset */                          \
            0x00,   /* GR2 Color Compare */                             \
            0x00,   /* GR3 Raster Operation/Rotate Counter */           \
            0x00,   /* GR4 Read Plane Select */                         \
            0x40,   /* GR5 Graphics Controll Mode */                    \
            0x05,   /* GR6 Memory Map Mode Control */                   \
            0x0F,   /* GR7 Color Don't Care */                          \
            0xFF    /* GR8 Bit Mask */                                  \
        },                                                              \
        {   /*  Init_AR0_AR14 */                                        \
            0x00,   /* AR0 Palette Register */                          \
            0x01,   /* AR1 Palette Register */                          \
            0x02,   /* AR2 Palette Register */                          \
            0x03,   /* AR3 Palette Register */                          \
            0x04,   /* AR4 Palette Register */                          \
            0x05,   /* AR5 Palette Register */                          \
            0x06,   /* AR6 Palette Register */                          \
            0x07,   /* AR7 Palette Register */                          \
            0x08,   /* AR8 Palette Register */                          \
            0x09,   /* AR9 Palette Register */                          \
            0x0A,   /* ARA Palette Register */                          \
            0x0B,   /* ARB Palette Register */                          \
            0x0C,   /* ARC Palette Register */                          \
            0x0D,   /* ARD Palette Register */                          \
            0x0E,   /* ARE Palette Register */                          \
            0x0F,   /* ARF Palette Register */                          \
            0x41,   /* AR10 Attribute Mode Control */                   \
            0x00,   /* AR11 Border Color */                             \
            0x0F,   /* AR12 Color Plane Enable */                       \
            0x00,   /* AR13 Horizontal Pixel Panning */                 \
            0x00,   /* AR14 Pixel Padding */                            \
        },                                                              \
        {   /*  Init_CR0_CR18 */                                        \
            0xC3,   /* CR0 Horizontal Total */                          \
            0x9F,   /* CR1 Horizontal Display End */                    \
            0xA0,   /* CR2 Start Horizontal Blank */                    \
            0x85,   /* CR3 End Horizontal Blank */                      \
            0xA4,   /* CR4 Start Horizontal Sync Position */            \
            0x1C,   /* CR5 End Horizontal Sync Position */              \
            0x0B,   /* CR6 Vertical Total */                            \
            0x3E,   /* CR7 CRTC Overflow */                             \
            0x00,   /* CR8 Preset Row Scan */                           \
            0x40,   /* CR9 Maximum Scan Line */                         \
            0x00,   /* CRA Cursor Start Scan Line */                    \
            0x00,   /* CRB Cursor End Scan Line */                      \
            0x00,   /* CRC Start Address High */                        \
            0x00,   /* CRD Start Address Low */                         \
            0xFF,   /* CRE Cursor Location High */                      \
            0x00,   /* CRF Cursor Location Low */                       \
            0xE9,   /* CR10 Vertical Retrace Start */                   \
            0x8B,   /* CR11 Vertical Retrace End */                     \
            0xDF,   /* CR12 Vertical Display End */                     \
            0xA0,   /* CR13 Offset */                                   \
            0x60,   /* CR14 Underline location */                       \
            0xE7,   /* CR15 Start Vertical Blank */                     \
            0x04,   /* CR16 End Vertical Blank */                       \
            0xAB,   /* CR17 CRTC Mode Control */                        \
            0xFF,   /* CR18 Line Compare */                             \
        },                                                              \
        {   /* Init_CR31_CR3C */                                        \
            0x08,   /* CR31 Memory Configuration Register */            \
            0x00,   /* CR32 Backward Compatibility 1 Register */        \
            0x00,   /* CR33 Backward Compatibility 2 Register */        \
            0x10,   /* CR34 Backward Compatibility 3 Register */        \
            0x00,   /* CR35 CRT Register Lock Register */               \
            0x12,   /* CR36 Configuration 1 Register */                 \
            0xFB,   /* CR37 Configuration 2 Register */                 \
            0x7B,   /* CR38 Register Lock 1 Register */                 \
            0xBD,   /* CR39 REgister Lock 2 Register */                 \
            0x15,   /* CR3A Misc. 1 Register */                         \
            0xB4,   /* CR3B Start Display FIFO Register */              \
            0x64,   /* CR3C Interlace Retrace Start Register */         \
        },                                                              \
        {   /* Init_CR40_CR49 */                                        \
            0x01,   /* CR40 System Configuration Register */            \
            0x04,   /* CR41 BIOS Flag Register */                       \
            0x04,   /* CR42 Mode Control Register */                    \
            0x00,   /* CR43 Extended Mode Register */                   \
            0x00,   /* CR44 n/a */                                      \
            0x00,   /* CR45 Hardware Graphics Cursor Mode Register */   \
            0x00,   /* CR46 Hardware Graphics Cursor Origin-X Reg */    \
            0xFF,   /* CR47 Hardware Graphics Cursor Origin-X Reg */    \
            0xF8,   /* CR48 Hardware Graphics Cursor Origin-Y Reg */    \
            0xFF    /* CR49 Hardware Graphics Cursor Origin-Y Reg */    \
        },                                                              \
        {   /* Init_CR50_CR6A */                                        \
            0x00,   /* CR50 n/a */                                      \
            0x00,   /* CR51 Extended System Cont 2 */                   \
            0x90,   /* CR52 Extended BIOS Flag 1   */                   \
            0x10,   /* CR53 Extended Memroy Cont 1 */                   \
            0x30,   /* CR54 Extended Memroy Cont 2 */                   \
            0x00,   /* CR55 Extended DAC Control   */                   \
            0x00,   /* CR56 External Sync Cont 1   */                   \
            0x00,   /* CR57 n/a */                                      \
            0x13,   /* CR58 (LAW) Linear Address Window Control */      \
            0xD8,   /* CR59 LAW Position */                             \
            0x00,   /* CR5A LAW Position */                             \
            0x00,   /* CR5B n/a */                                      \
            0x00,   /* CR5C General Out Port */                         \
            0x00,   /* CR5D Extended Horizontal Overflow */             \
            0x00,   /* CR5E Extended Vertical Overflow */               \
            0x00,   /* CR5F n/a */                                      \
            0x0F,   /* CR60 n/a */                                      \
            0x0F,   /* CR61 Extended Memory Control 4 */                \
            0x0F,   /* CR62 n/a */                                      \
            0x00,   /* CR63 n/a */                                      \
            0x00,   /* CR64 n/a */                                      \
            0x04,   /* CR65 Extended Misc. Control   */                 \
            0x89,   /* CR66 Extended Misc. Control 1 */                 \
            0x50,   /* CR67 Extended Misc. Control 2 */                 \
            0xFE,   /* CR68 Configuration 3 */                          \
            0xE0,   /* CR69 Extended System Control 3 */                \
            0xC0,   /* CR6A Extended System Control 4 */                \
        },                                                              \
    },                                                                  \
    {                                                                   \
        /*  mode#0: 800 x 600  555  60Hz VESA mode 0x113 */             \
        { 0, 800, 600, 16, 60, gpe16Bpp,                                \
          sizeof(GPEModeEx), 0, GPEMODEEX_CURRENTVERSION, 0, 0, 0, 0,   \
          ddgpePixelFormat_5550, 0, 0, 800 * 16 / 8, 0,                 \
          0x7C00, 0x03E0, 0x001F, 0x0000 },                             \
        {   /*  Init_SR0_SR4 */                                         \
            0x03,   /* SR0 Reset */                                     \
            0x01,   /* SR1 Clocking Mode */                             \
            0x0F,   /* SR2 Enable Write Plane */                        \
            0x00,   /* SR3 Character Font Select */                     \
            0x0E    /* SR4 Memory Mode Control */                       \
        },                                                              \
        {   /*  Init_SR10_SR13 */                                       \
            0x41,   /* SR10 MCLK Value Low  */                          \
            0x2C,   /* SR11 MCLK Value High */                          \
            0x61,   /* SR12 DCLK Value Low  */                          \
            0x41    /* SR13 DCLK Value High */                          \
        },                                                              \
        {   /*  Init_GR0_GR8 */                                         \
            0x00,   /* GR0 Set/Reset */                                 \
            0x00,   /* GR1 Enable Set/Reset */                          \
            0x00,   /* GR2 Color Compare */                             \
            0x00,   /* GR3 Raster Operation/Rotate Counter */           \
            0x00,   /* GR4 Read Plane Select */                         \
            0x00,   /* GR5 Graphics Controll Mode */                    \
            0x05,   /* GR6 Memory Map Mode Control */                   \
            0x0F,   /* GR7 Color Don't Care */                          \
            0xFF    /* GR8 Bit Mask */                                  \
        },                                                              \
        {   /*  Init_AR0_AR14 */                                        \
            0x00,   /* AR0 Palette Register */                          \
            0x01,   /* AR1 Palette Register */                          \
            0x02,   /* AR2 Palette Register */                          \
            0x03,   /* AR3 Palette Register */                          \
            0x04,   /* AR4 Palette Register */                          \
            0x05,   /* AR5 Palette Register */                          \
            0x06,   /* AR6 Palette Register */                          \
            0x07,   /* AR7 Palette Register */                          \
            0x08,   /* AR8 Palette Register */                          \
            0x09,   /* AR9 Palette Register */                          \
            0x0A,   /* ARA Palette Register */                          \
            0x0B,   /* ARB Palette Register */                          \
            0x0C,   /* ARC Palette Register */                          \
            0x0D,   /* ARD Palette Register */                          \
            0x0E,   /* ARE Palette Register */                          \
            0x0F,   /* ARF Palette Register */                          \
            0x41,   /* AR10 Attribute Mode Control */                   \
            0x00,   /* AR11 Border Color */                             \
            0x0F,   /* AR12 Color Plane Enable */                       \
            0x00,   /* AR13 Horizontal Pixel Panning */                 \
            0x00,   /* AR14 Pixel Padding */                            \
        },                                                              \
        {   /*  Init_CR0_CR18 */                                        \
            0x03,   /* CR0 Horizontal Total */                          \
            0xC7,   /* CR1 Horizontal Display End */                    \
            0xC8,   /* CR2 Start Horizontal Blank */                    \
            0x84,   /* CR3 End Horizontal Blank */                      \
            0xD2,   /* CR4 Start Horizontal Sync Position */            \
            0x12,   /* CR5 End Horizontal Sync Position */              \
            0x72,   /* CR6 Vertical Total */                            \
            0xF0,   /* CR7 CRTC Overflow */                             \
            0x00,   /* CR8 Preset Row Scan */                           \
            0x60,   /* CR9 Maximum Scan Line */                         \
            0x00,   /* CRA Cursor Start Scan Line */                    \
            0x00,   /* CRB Cursor End Scan Line */                      \
            0x00,   /* CRC Start Address High */                        \
            0x00,   /* CRD Start Address Low */                         \
            0xFF,   /* CRE Cursor Location High */                      \
            0x00,   /* CRF Cursor Location Low */                       \
            0x58,   /* CR10 Vertical Retrace Start */                   \
            0x8C,   /* CR11 Vertical Retrace End */                     \
            0x57,   /* CR12 Vertical Display End */                     \
            0xC8,   /* CR13 Offset */                                   \
            0x00,   /* CR14 Underline location */                       \
            0x57,   /* CR15 Start Vertical Blank */                     \
            0x00,   /* CR16 End Vertical Blank */                       \
            0xA3,   /* CR17 CRTC Mode Control */                        \
            0xFF,   /* CR18 Line Compare */                             \
        },                                                              \
        {   /* Init_CR31_CR3C */                                        \
            0x08,   /* CR31 Memory Configuration Register */            \
            0x00,   /* CR32 Backward Compatibility 1 Register */        \
            0x00,   /* CR33 Backward Compatibility 2 Register */        \
            0x10,   /* CR34 Backward Compatibility 3 Register */        \
            0x00,   /* CR35 CRT Register Lock Register */               \
            0x12,   /* CR36 Configuration 1 Register */                 \
            0xFB,   /* CR37 Configuration 2 Register */                 \
            0x7B,   /* CR38 Register Lock 1 Register */                 \
            0xBD,   /* CR39 REgister Lock 2 Register */                 \
            0x15,   /* CR3A Misc. 1 Register */                         \
            0xEB,   /* CR3B Start Display FIFO Register */              \
            0x84,   /* CR3C Interlace Retrace Start Register */         \
        },                                                              \
        {   /* Init_CR40_CR49 */                                        \
            0x01,   /* CR40 System Configuration Register */            \
            0x04,   /* CR41 BIOS Flag Register */                       \
            0x04,   /* CR42 Mode Control Register */                    \
            0x00,   /* CR43 Extended Mode Register */                   \
            0x00,   /* CR44 n/a */                                      \
            0x00,   /* CR45 Hardware Graphics Cursor Mode Register */   \
            0x06,   /* CR46 Hardware Graphics Cursor Origin-X Reg */    \
            0xBF,   /* CR47 Hardware Graphics Cursor Origin-X Reg */    \
            0xB8,   /* CR48 Hardware Graphics Cursor Origin-Y Reg */    \
            0xBE    /* CR49 Hardware Graphics Cursor Origin-Y Reg */    \
        },                                                              \
        {   /* Init_CR50_CR6A */                                        \
            0x00,   /* CR50 n/a */                                      \
            0x00,   /* CR51 Extended System Cont 2 */                   \
            0x90,   /* CR52 Extended BIOS Flag 1   */                   \
            0x10,   /* CR53 Extended Memroy Cont 1 */                   \
            0x30,   /* CR54 Extended Memroy Cont 2 */                   \
            0x00,   /* CR55 Extended DAC Control   */                   \
            0x00,   /* CR56 External Sync Cont 1   */                   \
            0x00,   /* CR57 n/a */                                      \
            0x13,   /* CR58 (LAW) Linear Address Window Control */      \
            0xD8,   /* CR59 LAW Position */                             \
            0x00,   /* CR5A LAW Position */                             \
            0x00,   /* CR5B n/a */                                      \
            0x00,   /* CR5C General Out Port */                         \
            0x01,   /* CR5D Extended Horizontal Overflow */             \
            0x00,   /* CR5E Extended Vertical Overflow */               \
            0x00,   /* CR5F n/a */                                      \
            0x0F,   /* CR60 n/a */                                      \
            0x0F,   /* CR61 Extended Memory Control 4 */                \
            0x0F,   /* CR62 n/a */                                      \
            0x00,   /* CR63 n/a */                                      \
            0x00,   /* CR64 n/a */                                      \
            0x04,   /* CR65 Extended Misc. Control   */                 \
            0x89,   /* CR66 Extended Misc. Control 1 */                 \
            0x30,   /* CR67 Extended Misc. Control 2 */                 \
            0xFE,   /* CR68 Configuration 3 */                          \
            0xE0,   /* CR69 Extended System Control 3 */                \
            0xC0,   /* CR6A Extended System Control 4 */                \
        },                                                              \
    },                                                                  \
};

extern ModeInit VGAModeP[];


// Direct access IO mapped registers

                            //Select AR0..14 On writes,sets 3C0[write]->ATR_DATA
#define reg_AR_ADDR         (PORT_RW_8 m_VGAIOPort[PortId_3C0 ]) 
                            //R/W AR0..14. On writes, sets 3C0[write]->ATR_AD
#define reg_AR_DATA         (PORT_RW_8 m_VGAIOPort[PortId_3C1 ]) 
                            //Reset flip flop to set3CO[write]->ATR_AD
#define reg_AR_RESET        (PORT_RO_8 m_VGAIOPort[PortId_3BA ]) 
                            //Select CR0..CR24
#define reg_CR_ADDR         (PORT_RW_8 m_VGAIOPort[PortId_3D4 ]) 
                            //Read/Write CR0..CR24
#define reg_CR_DATA         (PORT_RW_8 m_VGAIOPort[PortId_3D5 ]) 
                            //Pixel read mask (put FF here!)
#define reg_DAC_AD_MK       (PORT_RW_8 m_VGAIOPort[PortId_3C6 ]) 
                            //Access palette entry (use 3 times)
#define reg_DAC_DATA        (PORT_RW_8 m_VGAIOPort[PortId_3C9 ]) 
                            //Read index into palette (auto-incr)
#define reg_DAC_RD_AD       (PORT_WO_8 m_VGAIOPort[PortId_3C7 ]) 
                            //DAC status - checks palette state
#define reg_DAC_STS         (PORT_RO_8 m_VGAIOPort[PortId_3C7 ]) 
                            //Write index into palette (auto-incr)
#define reg_DAC_WR_AD       (PORT_WO_8 m_VGAIOPort[PortId_3C8 ]) 
                            //Select GR0..8
#define reg_GR_ADDR         (PORT_RW_8 m_VGAIOPort[PortId_3CE ]) 
                            //Access GR0..8
#define reg_GR_DATA         (PORT_RW_8 m_VGAIOPort[PortId_3CF ]) 
                            //Select SR0..18
#define reg_SR_ADDR         (PORT_RW_8 m_VGAIOPort[PortId_3C4 ]) 
                            //Read/Write SR0..18
#define reg_SR_DATA         (PORT_RW_8 m_VGAIOPort[PortId_3C5 ]) 
                            //CRT Irq Pending
#define reg_STATUS_0        (PORT_RO_8 m_VGAIOPort[PortId_3C2 ]) 
                            //VSync Status
#define reg_STATUS_1        (PORT_RO_8 m_VGAIOPort[PortId_3DA ]) 


// Indexed registers and their aliases

#define reg_RST_SYNC        reg_SR[0x0]     //  R/W reset
#define reg_CLK_MODE        reg_SR[0x1]     //  R/W dot clk, chr clk, screen_off
#define reg_EN_WT_PL        reg_SR[0x2]     //  R/W enable cpu mem writes
#define reg_CH_FONT_SL      reg_SR[0x3]     //  R/W font select
#define reg_MEM_MODE        reg_SR[0x4]     //  R/W Mem access control
#define reg_UNLK_EXSR       reg_SR[0x8]     //  R/W Enable SR9..SR18

#define reg_H_TOTAL         reg_CR[0x0]     //  R/W Total char clocks per line
#define reg_H_D_END         reg_CR[0x1]     //  R/W Active char clocks per line
#define reg_S_H_BLANK       reg_CR[0x2]     //  R/W Char no of start of 
                                            //  blank period
#define reg_E_H_BLANK       reg_CR[0x3]     //  R/W End hBlank and display skew
#define reg_S_H_SY_P        reg_CR[0x4]     //  R/W Start horizontal 
                                            //  sync position
#define reg_E_H_SY_P        reg_CR[0x5]     //  R/W End horizontal sync position
#define reg_V_TOTAL         reg_CR[0x6]     //  R/W Vertical total in scan lines
#define reg_OVFL_REG        reg_CR[0x7]     //  R/W Overflow reg - bit 8 &9’s 
                                            //  of CR6,12,10,15,18
#define reg_P_R_SCAN        reg_CR[0x8]     //  R/W Preset row scan - 
                                            //  for soft scrolling
#define reg_MAX_S_LN        reg_CR[0x9]     //  R/W Scan lines / char -1, Bit 9
                                            //  of CR15,18. Double-scan enable
#define reg_CSSL            reg_CR[0xA]     //  R/W Cursor start row
#define reg_CESL            reg_CR[0xB]     //  R/W Cursor end row
#define reg_STA_H           reg_CR[0xC]     //  R/W Start addr high
#define reg_STA_L           reg_CR[0xD]     //  R/W Start addr low
#define reg_CLA_H           reg_CR[0xE]     //  R/W Cursor addr high
#define reg_CLA_L           reg_CR[0xF]     //  R/W Cursor addr low
#define reg_VRS             reg_CR[0x10]    //  R/W Vertical retrace start
#define reg_VRE             reg_CR[0x11]    //  R/W Vertical retrace end
#define reg_VDE             reg_CR[0x12]    //  R/W Vertical display end
#define reg_SCREEN_OFFSET   reg_CR[0x13]    //  R/W Screen pitch / 2,4,or8  - 
                                            //  see CR51
#define reg_ULL             reg_CR[0x14]    //  R/W Underline row & char 
                                            //  count mode
#define reg_SVB             reg_CR[0x15]    //  R/W Start vertical blank
#define reg_EVB             reg_CR[0x16]    //  R/W End vertical blank
#define reg_CRT_MD          reg_CR[0x17]    //  R/W CRTC mode, and sync disable
#define reg_LCM             reg_CR[0x18]    //  R/W Split screen address
#define reg_GCCL            reg_CR[0x22]    //  R   CPU Latch 0..3  
                                            //  [via GR4 1:0]
#define reg_ATC_F           reg_CR[0x24]    //  R   Attribute controller index

#define reg_SET_RST_DT      reg_GR[0x0]     //  R/W Value written in 
                                            //  write modes 0 & 3
#define reg_EN_S_R_DT       reg_GR[0x1]     //  R/W Bit write enables for 
                                            //  write mode 0
#define reg_COLOR_CMP4Bpp   reg_GR[0x2]     //  R/W Color compare for 
                                            //  read mode 1 
#define reg_WT_ROP          reg_GR[0x3]     //  R/W ROP & rotate for writes
#define reg_RD_PL_SL        reg_GR[0x4]     //  R/W Read plane select (0..3)
#define reg_GRP_MODE        reg_GR[0x5]     //  R/W Select write mode (0..3), 
                                            //  enable read compare, odd/even, 
                                            //  and 256-color support
#define reg_MISC_GM         reg_GR[0x6]     //  R/W Text/graphics & 
                                            //  Memory addr range
#define reg_CMP_DNTC        reg_GR[0x7]     //  R/W Color don’t care bits 
                                            //  for read mode 1
#define reg_BIT_MASK        reg_GR[0x8]     //  R/W Write mask

#define reg_PLT_REG         reg_AR          //  R/W 16 6-bit palette registers
#define reg_ATR_MODE        reg_AR[0x10]    //  R/W Attribute mode control
#define reg_BDR_CLR         reg_AR[0x11]    //  R/W Border color 8-bit 
                                            //  PA modes only
#define reg_DISP_PLN        reg_AR[0x12]    //  R/W Color plane enable
#define reg_H_PX_PAN        reg_AR[0x13]    //  R/W Horiz pixel panning
#define reg_PX_PAD          reg_AR[0x14]    //  R/W Pixel padding 
                                            //  (converts to 8Bpp)


#endif // __GPEVGA_H__
