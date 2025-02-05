//*****************************************************************************
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//
//*****************************************************************************

//*******************************************************************
//
//  Register Defines for our fake AC97 compliant device
//
//*******************************************************************

#ifndef _H_REGFAKE
#define _H_REGFAKE

#define BA0_CFGI                                0x000004B0L		
#define BA0_GPIOR                               0x000004B8L		

//****************************************************************************
//
// The following define the offsets of the AC97 shadow registers, which appear
// as a virtual extension to the base address register zero memory range.
//
//****************************************************************************
#define BA0_AC97_RESET                          0x00001000L		
#define BA0_AC97_MASTER_VOLUME                  0x00001002L		
#define BA0_AC97_HEADPHONE_VOLUME               0x00001004L		
#define BA0_AC97_MIC_VOLUME                     0x0000100EL		
#define BA0_AC97_LINE_IN_VOLUME                 0x00001010L		
#define BA0_AC97_AUX_VOLUME                     0x00001016L		
#define BA0_AC97_PCM_OUT_VOLUME                 0x00001018L		
#define BA0_AC97_RECORD_GAIN                    0x0000101CL		

//****************************************************************************
//
// The following define the offsets of addresses relative to BA1
// for the mixer device. 
//****************************************************************************

// Playback registers.
#define GOF_PER_SEC                                200	
#define PROC_SRC_SCB                              0xa0	
#define BA1_MASTER_MIX_CHILD_REG                  0x264	
#define BA1_SAMPLE_RATE_CORRECTION_OFFSET		  0x008	
#define BA1_ZV_BASE_REGISTER		              (0xBC0UL * 4)		
#define BA1_PLAY_PHASE_INCREMENT_REG              0x2b4	

//
// controls for serial1 ( ZV port)
//
#define BA1_IO_DATA_PTR             (0x00000BF6UL * 4)	
#define BA1_IO_ADDR                 (0x00000BF0UL * 4)	
#define BA1_IO_DATA                 (0x00000BF1UL * 4)	
#define IO_DATA_PTR_MASK            0xFFFF0000
#define IO_ADDR_MASK                0x0000FFFF	
#define IO_ADDR_NO_MORE_DATA        0x40000000	
#define IO_DATA_PTR_SHIFT           16	
#define IO_ADDR_SHIFT               0	
#define IO_ASER1_CONTROL            0x8043		
#define ASER1_CONTROL_ME            0x80000000L		
#define BA1_ZV_VOLUME				0x2EF8		


#endif  // _H_REGFAKE
