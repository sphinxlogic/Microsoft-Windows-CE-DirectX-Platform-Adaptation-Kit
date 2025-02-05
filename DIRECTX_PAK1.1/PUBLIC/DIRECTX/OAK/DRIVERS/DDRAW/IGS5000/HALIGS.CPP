// -----------------------------------------------------------------------------
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  COPYRIGHT (C) 1999-2000, IGS TECHNOLOGIES, INC. (iGST)
//
// -----------------------------------------------------------------------------
#include "precomp.h"
extern WORD wDoubleBuffer;

//define IGS5KREVD if it is IGS 5000 RevD chip, which has improved double buffer function.
//#define IGS5KREVD 

#ifdef IGS5KREVD
BOOL bCapDoubleBuffer = FALSE;
BOOL bOverDoubleBuffer = FALSE;
#endif

/* this variable will be initialized by buildHalInfo */
IndexedReg8 VidIO[4];	/*
						 *	index 0: 3CE
						 *	index 1: 3C4
						 *  index 2: 3D4
						 */


/* graphics functions */

WORD GetScanline() {
    BYTE lo=0xFF;
	WORD hi=0;

    while (lo==0xFF) {
		lo = reg_CURRENT_VLINE_LO;
	}
	hi = reg_CURRENT_VLINE_HI;
    hi = (hi<<8)+lo;
    return (hi&0x7FF);
}


void SetScreenStart(DWORD lOfs) {

	reg_SCREEN_START_LO = (BYTE) (lOfs>>2);
	reg_SCREEN_START_MI = (BYTE) ((lOfs>>10)&0xFF);
	reg_SCREEN_START_HI = (BYTE) ((lOfs>>18)|0x10);
	return;
}

int IsCopBusy() {
	return (reg_COP_STAT & COP_BUSY);
}


/* overlay functions */
void InitVideo(int e) {
	reg_DDA_X_INIT_L(e)=0x0;
	reg_DDA_X_INIT_H(e)=0x8;
	reg_DDA_Y_INIT_L(e)=0x0;
	reg_DDA_Y_INIT_H(e)=0x8;

	/*fifo? */
//  reg_FIFO_TIMING_CTL_L(e) = 0x3C;
//  reg_FIFO_TIMING_CTL_H(e) = 0x08;
//	reg_FIFO_CTL_I(e)		 = 0x06;

    IGS_ChangeOverlayFIFO(e);
}


const BYTE bFIFOPolicyNum[5] = {0x10, 0x10, 0x1C, 0x1C, 0x06};

/********************************************************
IGS_ChangeOverlayFIFO() sets some FIFO related registers in
 order to get best memory bandwidth
in:  none
out:  none
note: bFIFOPolicyNum array contains the preset FIFO values,
 if you want to adjust these values, change the value there
*********************************************************/
void  IGS_ChangeOverlayFIFO(int wOverlayIndex)
{
 Out_Video_Reg(0x74, bFIFOPolicyNum[0]);
 Out_Video_Reg(0x75, bFIFOPolicyNum[1]);
 Out_Video_Reg(0xD9, bFIFOPolicyNum[2]);
 Out_Video_Reg(0xDA, bFIFOPolicyNum[3]);

  if(wOverlayIndex == 0) {
   Out_Video_Reg_M(0xA6, 0x08, ~0x08);
   Out_Video_Reg_M(0xF1, 0x40, ~0xC0);
  }
  else if(wOverlayIndex == 1) {
   Out_SEQ_Reg_M(0xDD, 0x12, ~0x12);
   Out_Video_Reg_M(0xF1, 0x40, ~0xC0);
  }
  Out_Video_Reg_M(0xDD, bFIFOPolicyNum[4] & 0x05, ~0x05);
  Out_Video_Reg_M(0xDD, 0x2, ~0x02);
}


void Out_Video_Reg( BYTE bIndex, BYTE bValue )
{
  VidIO[0][bIndex]= bValue;
}

void Out_Video_Reg_M( BYTE bIndex, BYTE bValue, BYTE bMask )
{
 BYTE bTemp;
 bTemp = VidIO[0][bIndex];
 bTemp &= bMask;
 bTemp |= bValue;
 VidIO[0][bIndex]= bTemp;
}

/*=========================================================
 This routine is used internally
  Out_SEQ_Reg() sets 0x3C4 Based register to a special value
  in:	  bIndex: video register index
      bValue=value to be set
  out:	   none
  note:   Register base is 0x3C4
 =========================================================*/
void Out_SEQ_Reg( BYTE bIndex, BYTE bValue )
{
  VidIO[1][bIndex]= bValue;
}

/*=========================================================
 This routine is used internally
  Out_SEQ_Reg_M() sets 0x3C4 based register with bit mask
  in:	  bIndex: video register index
     bValue=bit/bits to be set/clear, untouched bit/bits must be zero
	bMask=bit/bits to be masked, 1:mask, 0:to be set/clear
  e.g. set <4,3>=<1,0>, bValue=0x10 and bMask=E7
  note:   Register base is 0x3C4
 =========================================================*/
void Out_SEQ_Reg_M( BYTE bIndex, BYTE bValue, BYTE bMask )
{
 BYTE bTemp;
 bTemp = VidIO[1][bIndex];
 bTemp &= bMask;
 bTemp |= bValue;
 VidIO[1][bIndex]= bTemp;
}

void SetColorKey(int e, BYTE bR, BYTE bG, BYTE bB) {

    reg_COLOR_CMP_RED(e)  = bR;
    reg_COLOR_CMP_GREEN(e)= bG;
    reg_COLOR_CMP_BLUE(e) = bB;
}

void ColorkeyOn(int e) {

    reg_DISP_CTL_I(e) = reg_DISP_CTL_I(e)&0xFD;
}

void ColorkeyOff(int e) {

    reg_DISP_CTL_I(e) = reg_DISP_CTL_I(e) | 0x02;
}

void SetChromaKey(int e, BYTE bHiR,BYTE bHiG,BYTE bHiB,BYTE bLoR,BYTE bLoG,BYTE bLoB) {

    if (e==0) {
	reg_VIDEO_INDEX = 0;
    }
    else {
	reg_VIDEO_INDEX = 0x80;
    }
    reg_CHROMA_CMP_RED_HI   = bHiR;
    reg_CHROMA_CMP_RED_LO   = bLoR;
    reg_CHROMA_CMP_GREEN_HI = bHiG;
    reg_CHROMA_CMP_GREEN_LO = bLoG;
    reg_CHROMA_CMP_BLUE_HI  = bHiB;
    reg_CHROMA_CMP_BLUE_LO  = bLoB;
}

void ChromaOn(int e) {


    if (e==0) {
	reg_VIDEO_INDEX = 0;
    }
    else {
	reg_VIDEO_INDEX = 0x80;
    }
    reg_VFAC_CTL_MODE_III = reg_VFAC_CTL_MODE_III | 0x20;
    if (e==1) {
	reg_MISC_CTL_I(1) = reg_MISC_CTL_I(1) | 0x04;
    }
}

void ChromaOff(int e) {

    if (e==0) {
	reg_VIDEO_INDEX = 0;
    }
    else {
	reg_VIDEO_INDEX = 0x80;
    }
    reg_VFAC_CTL_MODE_III = reg_VFAC_CTL_MODE_III & ~0x20;
    if(e==1) {
	reg_MISC_CTL_I(1) = reg_MISC_CTL_I(1) & ~0x04;
    }
}

void VideoOn(int e) {

    //turn on video
	reg_DISP_CTL_I(e) =  (reg_DISP_CTL_I(e) | VIDEO_ON);
}


void VideoOff(int e) {

    //turn off video
    reg_DISP_CTL_I(e) = reg_DISP_CTL_I(e) & ~VIDEO_ON;
}

void VideoFormat(int e, WORD vfmt) {

    reg_VIDEO_FORMAT(e) = (BYTE) vfmt;
}

void VideoControl(int e, WORD vctrl) {

    reg_DISP_CTL_I(e) = (reg_DISP_CTL_I(e)& VIDEO_ON)|vctrl;
}

void SetVideoWindow(int e, WORD wLeft, WORD wTop, WORD wRight, WORD wBottom) {

    reg_DEST_RECT_LEFT_L(e)   = LOBYTE(wLeft);
    reg_DEST_RECT_LEFT_H(e)   = HIBYTE(wLeft);
    reg_DEST_RECT_RIGHT_L(e)  = LOBYTE(wRight);
    reg_DEST_RECT_RIGHT_H(e)  = HIBYTE(wRight);

    reg_DEST_RECT_TOP_L(e)    = LOBYTE(wTop);
    reg_DEST_RECT_TOP_H(e)    = HIBYTE(wTop);
    reg_DEST_RECT_BOTTOM_L(e) = LOBYTE(wBottom);
    reg_DEST_RECT_BOTTOM_H(e) = HIBYTE(wBottom);

}

void SetVideoScale(int e, WORD wSrcXExt, WORD wDstXExt, WORD wSrcYExt, WORD wDstYExt) {

    WORD      wScale;

    if (wSrcXExt==wDstXExt) {
	wScale=0x1000;
    }
    else {
	wScale = (WORD) ( ( 4096 * (DWORD) ( wSrcXExt - 2  ) ) / wDstXExt );
    }
    reg_DDA_X_INC_L(e) = LOBYTE(wScale);
    reg_DDA_X_INC_H(e) = HIBYTE(wScale);

    if (wSrcYExt==wDstYExt) {
	wScale=0x1000;
    }
    else {
	wScale = (WORD) ( ( 4096 * (DWORD) ( wSrcYExt - 1 ) ) / wDstYExt );
    };
    reg_DDA_Y_INC_L(e) = LOBYTE(wScale);
    reg_DDA_Y_INC_H(e) = HIBYTE(wScale);
}

void SetVideoSrc(int e, DWORD dwLinStart, WORD wPitch, WORD wFetch) {
    // Pitch sometimes differs from Fetch, this case happens if Width is less than
    // pitch (we can set fetch=pitch but it waste bandwidth

    DWORD   dwL;
    BYTE    bHigh;
	if(wFetch > 352 * 2) {
		if(e == 0) {
			reg_XVIDEO_ABCTL_II = (reg_XVIDEO_ABCTL_II & ~0x70) | 0x60;
		}
		else if(e == 1) {
			reg_XVIDEO_ABCTL_II = (reg_XVIDEO_ABCTL_II & ~0xB0) | 0xA0;
		}
	}
	else{
		if(e == 0) {
			reg_XVIDEO_ABCTL_II = (reg_XVIDEO_ABCTL_II & ~0x70) | 0x20;
		}
		else if(e == 1) {
			reg_XVIDEO_ABCTL_II = (reg_XVIDEO_ABCTL_II & ~0xB0) | 0x20;
		}
	}

    dwL    = dwLinStart >> 2;
    wPitch = wPitch >> 3;	    // 64-bit boundary
    wFetch = wFetch >> 3;	    // 64-bit boundary


    // set src start
    reg_MEMORY_START_L(e) = (BYTE) dwL;
    reg_MEMORY_START_M(e) = (BYTE) ( dwL >> 8 );
    reg_MEMORY_START_H(e) = (BYTE) ( ( dwL & 0xff0000L) >> 16 );

    // set 8-bit low of pitch & fetch
    reg_MEMORY_PITCH_L(e)      = LOBYTE(wPitch);
    reg_MEMORY_OFFSET_PHASE(e) = LOBYTE(wFetch);

    // set 2-bit high of pitch & fetch
    bHigh = HIBYTE(wPitch) ;
    bHigh = bHigh | ( ( HIBYTE( wFetch) ) << 4 );
    reg_MEMORY_PITCH_H(e) =  bHigh;
}

void SetVideoBackBufferSrc(DWORD dwLinStart)
{
// Pitch sometimes differs from Fetch, this case happens if Width is less than
// pitch (we can set fetch=pitch but it waste bandwidth
DWORD   dwL;
//BYTE    bHigh;
dwL    = dwLinStart >> 2;
	reg_VIDEO_INDEX = 0x01; //Y
    reg_MEMORY_START_L1(0) = (BYTE) dwL;
    reg_MEMORY_START_M1(0) = (BYTE) ( dwL >> 8 );
	reg_VIDEO_INDEX = 0x00; //Y
    reg_MEMORY_START_H1(0) = (BYTE) ( ( dwL & 0xff0000L) >> 16 );
}

/* for VPE code */
void ResetSeq() {

	int	i;

   for (i=0; i<0x100000; i++) {
	   if (!IsCopBusy()) break;
   }

	reg_MEM_CTL_I	= reg_MEM_CTL_I | SEQ_RESET;
	reg_MEM_CTL_I	= reg_MEM_CTL_I & ~SEQ_RESET;

	reg_SEQ0		= 0x01;

	for (i=0; i<0x100; i++);
	reg_SEQ0		= 0x03;
	return;
}


void VPE_Freeze() {
	int nCounter = 0;
	reg_VFAC_CTL_MODE_I = (reg_VFAC_CTL_MODE_I& ~0xC)|0x0C;
	while(!(reg_XVID1_ROM & 0x02) && nCounter < 3) {
		while (!IN_VBLANK);
		while (IN_VBLANK);
		nCounter++;
	}
	return;
}


void VPE_UnFreeze() {
	int nCounter = 0;
	for(;nCounter < 4; nCounter++) {
		while (!IN_VBLANK);
		while (IN_VBLANK);
	}
	reg_VFAC_CTL_MODE_I = reg_VFAC_CTL_MODE_I & ~0x0C;
	return;
}

void VPE_SetDst(DWORD dwLinStart,WORD wPitch) {
	DWORD	dwL;
	BYTE	bMix;

	wPitch = wPitch >> 3;		// 64-bit boundary

	//if monotor is interlaced, pitch must be doubled.

	VPE_Freeze();
	dwL    = dwLinStart >> 2;

#ifdef IGS5KREVD
	if (wDoubleBuffer)
		wPitch = wPitch >> 1;
#else
	if(wDoubleBuffer)
		dwL -= wPitch;

	if (wDoubleBuffer)
		wPitch = wPitch >> 1;
#endif

	reg_VIDEO_INDEX = 0x00;
	
	reg_VP_MEMSTART_LOW = (BYTE) (dwL & 0x0000ffL);
	reg_VP_MEMSTART_MID = (BYTE) ( (dwL& 0x00ff00L) >> 8 );

	// Hi start address and pitch share same reg!
	bMix = (BYTE) ( ( dwL & 0xff0000L) >> 16 );
	bMix |= HIBYTE(wPitch)<<6;
	reg_VP_MEMSTART_HI = bMix;
	reg_VP_PITCH_LO   = LOBYTE(wPitch);
	VPE_UnFreeze();
	return;
}

void VPE_SetVPBackBufferDst(DWORD dwLinStart)
{
DWORD dwL;
//BYTE  bMix;
	VPE_Freeze();
	dwL    = dwLinStart >> 2;
	reg_VIDEO_INDEX = 0x00;
	reg_VP_MEMSTART_LOW1 = (BYTE) (dwL & 0x0000ffL);
	reg_VP_MEMSTART_MID1 = (BYTE) ( (dwL& 0x00ff00L) >> 8 );
	//reg_VP_MEMSTART_HI = ((BYTE) ( ( dwL & 0x0f0000L) >> 16 ) & ~0x0F) | 0x0F;
	reg_VP_MEMSTART_HI1 = (reg_VP_MEMSTART_HI1 & ~0x0F) | (BYTE) ( ( dwL & 0x0f0000L) >> 16 );
	VPE_UnFreeze();
}

void VPE_EnableVPDoubleBuffer(BYTE bEnable)
{
	if(bEnable != 0) {
	reg_VP_MEMSTART_HI1 = (reg_VP_MEMSTART_HI1 & ~0x10) | 0x10;
#ifdef IGS5KREVD
		bCapDoubleBuffer = TRUE;
		VidIO[0][0xF7] = 0x00;
		VidIO[0][0xFA] = 0x80;
		VidIO[0][0xE7] = VidIO[0][0xE7] | 0x20;
		if(bOverDoubleBuffer){
			VidIO[0][0xE7] = VidIO[0][0xE7] | 0x40;//Sync Cap here
			VidIO[0][0xE9] = VidIO[0][0xE9] | 0x04;//Sync Playback here
		}
		VidIO[0][0xE0] = (VidIO[0][0xE0] & ~0x68) | 0x68;
		VidIO[0][0xFA] = 0x00;
#endif
	}
	else {
		reg_VP_MEMSTART_HI1 = reg_VP_MEMSTART_HI1 & ~0x10;
#ifdef IGS5KREVD
		bCapDoubleBuffer = FALSE;
		VidIO[0][0xF7] = 0x00;
		VidIO[0][0xFA] = 0x80;
		VidIO[0][0xE7] = VidIO[0][0xE7] & ~0x20;
		VidIO[0][0xE9] = VidIO[0][0xE9] & ~0x04;//Disable Sync Playback here
		VidIO[0][0xE7] = VidIO[0][0xE7] & ~0x40;//Disable Sync Cap here
		VidIO[0][0xE0] = (VidIO[0][0xE0] & ~0x68);
		VidIO[0][0xFA] = 0x00;
#endif
	}
}

void VPE_EnableOverDoubleBuffer(BYTE bEnable)
{
	if(bEnable != 0 && wDoubleBuffer != 0) { /*03/28/2000*/
#ifdef IGS5KREVD
		bOverDoubleBuffer = TRUE;
		VidIO[0][0xF7] = 0x00;
		VidIO[0][0xFA] = 0x80;
		VidIO[0][0xE9] = VidIO[0][0xE9] | 0x02;
		if(bCapDoubleBuffer) {
			VidIO[0][0xE7] = VidIO[0][0xE7] | 0x40;//Sync Cap here
			VidIO[0][0xE9] = VidIO[0][0xE9] | 0x04;//Sync Playback here
		}
		VidIO[0][0xE3] = (VidIO[0][0xE0] & ~0x08) | 0x08;
		VidIO[0][0xFA] = 0x00;
		VidIO[0][0xAC] = VidIO[0][0xAC] | 0x02;
#else
		reg_VP_DOUBLEBUFFERCTL = (reg_VP_DOUBLEBUFFERCTL & ~0x40) | 0x40;
		reg_VP_MEMSTART_HI1 = (reg_VP_MEMSTART_HI1 & ~0x20) | 0x20;
#endif
	}
	else {
#ifdef IGS5KREVD
		bOverDoubleBuffer = FALSE;
		VidIO[0][0xF7] = 0x00;
		VidIO[0][0xFA] = 0x80;
		VidIO[0][0xE9] = VidIO[0][0xE9] & ~0x02;
		VidIO[0][0xE9] = VidIO[0][0xE9] & ~0x04;//Disable Sync Playback here
		VidIO[0][0xE7] = VidIO[0][0xE7] & ~0x40;//Disable Sync Cap here
		VidIO[0][0xE3] = 0x00;
		VidIO[0][0xFA] = 0x00;
		VidIO[0][0xAC] = VidIO[0][0xAC] &~ 0x02;
#else
		reg_VP_DOUBLEBUFFERCTL = reg_VP_DOUBLEBUFFERCTL & ~0x40;
		reg_VP_MEMSTART_HI1 = reg_VP_MEMSTART_HI1 & ~0x20;
#endif
	}
}

void VPE_SetCrop(WORD left, WORD top, WORD right,WORD bottom) {

	if (!(left % 2) && left != 0)
		left -= 1;

	reg_VIDEO_INDEX = 0x00;

	reg_VP_CROPLEFT_LO	= (BYTE)left;
	reg_VP_CROPLEFT_HI	= (BYTE)(left>>8);

	reg_VP_CROPRIGHT_LO	= (BYTE)right;
	reg_VP_CROPRIGHT_HI	= (BYTE)(right>>8);

	reg_VP_CROPTOP_LO	= (BYTE)top;
	reg_VP_CROPTOP_HI	= (BYTE)(top>>8);

	reg_VP_CROPBOTTOM_LO= (BYTE)bottom;
	reg_VP_CROPBOTTOM_HI= (BYTE)(bottom>>8);
}

void VPE_DDA(WORD wVPXExt, WORD wMemXExt, WORD wVPYExt, WORD wMemYExt) {

	WORD wScale;

	reg_VIDEO_INDEX 			= 0x00;
	reg_VP_HDDA_INIT_LO			= 0x00;
	reg_VP_HDDA_INIT_HI			= 0x08;
	if (wVPXExt==wMemXExt)
		wScale=0x1000;
	else
		wScale = (WORD) ( ( (DWORD)0x1000 * (DWORD) wMemXExt ) / (DWORD)wVPXExt );
	reg_VP_HDDA_INC_LO		= LOBYTE(wScale);
	reg_VP_HDDA_INC_HI		= HIBYTE(wScale);

	reg_VP_VDDA_INIT_LO		= 0x00;
	reg_VP_VDDA_INIT_HI		= 0x08;
	if (wVPYExt==wMemYExt)
		wScale=0x1000;
	else
		wScale = (WORD) ( ( (DWORD)0x1000 * (DWORD) wMemYExt ) / (DWORD)wVPYExt );
	reg_VP_VDDA_INC_LO		= LOBYTE(wScale);
	reg_VP_VDDA_INC_HI		= HIBYTE(wScale);
}


void VPE_DDA_On() {

	reg_CAP_NEW_CTL_I = (reg_CAP_NEW_CTL_I & ~0x01) | 0x01;
	return;
}

void VPE_DDA_Off() {
	reg_CAP_NEW_CTL_I = reg_CAP_NEW_CTL_I & ~0x01;
	return;
}


void VPE_Off() {
	int nCounter = 0;
	//let's finished last field, otherwise there will be a corrupt image
	reg_VFAC_CTL_MODE_I = (reg_VFAC_CTL_MODE_I& ~0x8)|0x08;//Sync freeze
	while(!(reg_XVID1_ROM & 0x02) && nCounter < 3) {//wait untile it freezed
		while (!IN_VBLANK);
		while (IN_VBLANK);
		nCounter++;
	}
	//Disable VAFC
	reg_BX_MISC0 = reg_BX_MISC0 & ~0x01;

	//Disable vp
	reg_VIDEO_INDEX 	= 0;
	reg_VFAC_CTL_MODE_I = 0x00;//reg_VFAC_CTL_MODE_I & ~0x01;
	reg_VP_BM_OFS		= reg_VP_BM_OFS & ~0x30;
	reg_VP_ZRGB_CTL 	= reg_VP_ZRGB_CTL & ~0x40;
	return;
}

void VPE_On() {

	VPE_Freeze ();

	//enable VAFC
	reg_BX_MISC0	= (reg_BX_MISC0 & ~0x01)|0x01;
	//reset video engine
	///reg_MISC_CTL_I(0)	= 0xC0;
	reg_MISC_CTL_I(0)	= 0x01;
	//enable VAFC, bit5 let clock in
	reg_VFAC_CTL_MODE_I = (reg_VFAC_CTL_MODE_I & ~0x61) | 0x61;

	ResetSeq();
	VPE_UnFreeze ();
	return;
}


void VPE_Init() {

	//following codes are specially for new igs 5000 board to support two video port
	reg_XB_CTL			= 0x01;
	reg_VCLK2			= (reg_VCLK2 & ~0x80) | 0x80;
	reg_MCLK2			= (reg_MCLK2 & ~0x02) | 0x02;
	reg_XB_CTL			= 0x00;


	//Disable all I2C access first //02/12/99
	reg_XB_CTL			= 0x01;
	reg_VCLK0			= reg_VCLK0 & ~0x11;
	reg_XB_CTL			= 0x00;
	reg_BX_MISC1		= (reg_BX_MISC1 & ~0x50)|0x50;

	VPE_Freeze();
	reg_VIDEO_INDEX 	= 0x00;

	reg_SR_EF			= reg_SR_EF & ~0x02;		//Video clock comes from Video port A
	reg_CAP_RATE_CTL	= reg_CAP_RATE_CTL & ~0x80;	//Video data comes from Video Port A
	reg_SR_ED			= reg_SR_ED & ~0x10;		//Video data comes from Video Port A
	//src is interlaced, but keep Chromakey for 1st video, preserve interrupt settings
	reg_VFAC_CTL_MODE_III	= (reg_VFAC_CTL_MODE_III & 0x60)| 0x02;
	reg_VIDEO_INDEX 		= 0x00;
	//Philips 711X
	reg_VFAC_CTL_MODE_II	= 0x10;
	reg_CAPTURE_CTL_MISC	= (reg_CAPTURE_CTL_MISC & ~0x30)|0x30;
	// Video Misc Control
	reg_GRDE						= 0x00;
	reg_GRDE						= 0x01;
	reg_GR88						= 0x3F;
	reg_GR89						= 0xC0;

	//extra bonus
	reg_VP_CCIR656_CAPMODE_I		= 0x10;
	reg_VP_CCIR656_CAPMODE_II		= 0x50;

	// New odd/even signal
	reg_GRAE						= 0x80;

	reg_CAPA_CTL_MISC				= (reg_CAPA_CTL_MISC & ~0x34) | 0x34;

	// Enable Vfac bit5 set 1 to let clock in, this is for 5000
	reg_VFAC_CTL_MODE_I				= (reg_VFAC_CTL_MODE_I & ~0x61)| 0x60;

	//Capture
	reg_BX_MISC0					= (reg_BX_MISC0 & 0xFE) | 0x01;

	//8bit capture and CCIR
	reg_VP_CCIR656_CAPMODE_I		= 0x11;

	//Enable Y Scale down for 5000
	reg_VBI_CTL_II			= (reg_VBI_CTL_II & ~0xC0) | 0xC0;

	reg_VIDEO_INDEX 		= 0x05;
	reg_GR49				= 0xC0;
	reg_VIDEO_INDEX 		= 0x00;

	VPE_SetCrop(0xF,0xC,712*2,0xFE);
	VPE_DDA(720,720,480,480);
	VPE_DDA_On();
}


void VPE_ConnectDecoder(WORD wGUID) {

	if(wGUID == CCIR656 || wGUID == BROOKTREE || wGUID == PHILIPS) {

		//use to discard VRefDate, otherwise use second one
		reg_VP_CCIR656_CAPMODE_I		= 0x17; 		//Starting from the VSync(0xA4 bit 2 set to 1)

		//use to keep FieldP, otherwise use second one
		reg_VP_CCIR656_CAPMODE_II	= 0x13;

		//Turn off odd/even signal self-generator
		reg_MIX_CAP_CTL_II				= reg_MIX_CAP_CTL_II & ~0x11;
		reg_VFAC_CTL_MODE_II			= 0x10;
	}
	else{/*CCIR601 mode*/
		reg_VP_CCIR656_CAPMODE_I		= 0x11; 	//Starting from the VSync(0xA4 bit 2 set to 1)
		reg_VP_CCIR656_CAPMODE_II		= 0x10;

		//Turn on odd/even signal self-generator
		reg_MIX_CAP_CTL_II			= (reg_MIX_CAP_CTL_II & ~0x11)|0x11;

		if(wGUID == HREFH_VREFH) {
			reg_VFAC_CTL_MODE_II	= 0x18;
		}
		else if(wGUID == HREFH_VREFL) {
			reg_VFAC_CTL_MODE_II	= 0x10;
		}
		else if(wGUID == HREFL_VREFH) {
			reg_VFAC_CTL_MODE_II	= 0x08;
		}
		else if(wGUID == HREFL_VREFL) {
			reg_VFAC_CTL_MODE_II	= 0x00;
		}
/*		else if(wGUID == PHILIPS) {
			reg_VFAC_CTL_MODE_II	= 0x10;

		}*/
	}
}

void VPE_InterlacedSrc() {
	reg_VIDEO_INDEX 		= 0;
	reg_VFAC_CTL_MODE_III	= (reg_VFAC_CTL_MODE_III & ~0x02)|0x02;
	reg_CAPTURE_CTL_MISC	= (reg_CAPTURE_CTL_MISC&~0x34)|0x30;
	return;
}

void VPE_NonInterlacedSrc() {
	reg_VIDEO_INDEX 		= 0;
	reg_VFAC_CTL_MODE_III	= reg_VFAC_CTL_MODE_III & ~0x02;
	reg_CAPTURE_CTL_MISC	= (reg_CAPTURE_CTL_MISC&~0x34)|0x34;
	return;
}

void VPE_InvertPolarity_On() {
	reg_VFAC_CTL_MODE_II = (reg_VFAC_CTL_MODE_II & ~0x20)|0x20;
	return;
}

void VPE_InvertPolarity_Off() {
	reg_VFAC_CTL_MODE_II = reg_VFAC_CTL_MODE_II & ~0x20;
	return;
}


void VPE_XMirror_On() {
	reg_VP_CCIR656_CAPMODE_I = reg_VP_CCIR656_CAPMODE_I|X_MIRROR;
	return;
}

void VPE_XMirror_Off() {
	reg_VP_CCIR656_CAPMODE_I = reg_VP_CCIR656_CAPMODE_I&~X_MIRROR;
	return;
}

void VPE_YMirror_On() {
	reg_VP_CCIR656_CAPMODE_I = reg_VP_CCIR656_CAPMODE_I|Y_MIRROR;
	return;
}

void VPE_YMirror_Off() {
	reg_VP_CCIR656_CAPMODE_I = reg_VP_CCIR656_CAPMODE_I&~Y_MIRROR;
	return;
}

BOOL VPE_CurrField() {
	//return TRUE if even, FALSE if odd
	reg_VIDEO_INDEX = 0;
	return ((reg_XVID1_ROM & 0x04)? FALSE: TRUE);
}

int VPE_CurrCapLine() {

	DWORD	line;

	reg_VIDEO_INDEX = 3;
	line = reg_GR47;
	line = ((line&0xF)<<8)+reg_GR46;
	return line;
}

BOOL VPE_DetectVideoSignal(WORD wGUID) {
	//return TRUE if OK

	//how?, check videosignal during 2 monitor's sync
	DWORD	howlong = 10*flipRec.dwTicksPerFrame;
	DWORD	start;
	DWORD	end;
	BYTE	bTemp;

	//Capture on
	reg_BX_MISC0 = (reg_BX_MISC0 & 0xFE)|0x01;

	// Enable Vfac bit5 set 1 to let clock in, this is for 5000
	bTemp = reg_VFAC_CTL_MODE_I;
	if((bTemp & 0x01) == 0x00)
		reg_VFAC_CTL_MODE_I = (reg_VFAC_CTL_MODE_I & ~0x65)|0x65;

	VPE_ConnectDecoder(wGUID);

	reg_VIDEO_INDEX = 3;

	//wait until irq active lo
	start = GetTickCount();
	while ( (reg_GR43 & 0x80) != 0x00) {
		end = GetTickCount();
		if ((end - start) > howlong) {
			if((bTemp & 0x01) == 0x00)
			reg_VFAC_CTL_MODE_I = bTemp;
			return FALSE;
		}
	}

	//search until irq active high
	start = GetTickCount();
	while ( (reg_GR43 & 0x80 ) == 0x00){
		end = GetTickCount();
		if ((end - start) > howlong) {
			if((bTemp & 0x01) == 0x00)
			reg_VFAC_CTL_MODE_I = bTemp;
			return FALSE;
		}
	}
	if((bTemp & 0x01) == 0x00)
		reg_VFAC_CTL_MODE_I = bTemp;
	return TRUE;
}

void VPE_DiscardVRef() {
	reg_VP_CCIR656_CAPMODE_I = reg_VP_CCIR656_CAPMODE_I & ~DISCARD_VREF;
	return;
}

void VPE_UseVRef() {
	reg_VP_CCIR656_CAPMODE_I = reg_VP_CCIR656_CAPMODE_I | DISCARD_VREF;
	return;
}


#if ENABLEVBI
//For VBI capture, source part means digital VBI data input, destination
//means our video frame buffer.
void VPEVBI_SetVBIDst(DWORD dwLinStart, DWORD dwLinEnd, WORD wPitch)
{
// Pitch sometimes differs from Fetch, this case happens if Width is less than
// pitch (we can set fetch=pitch but it waste bandwidth
DWORD	dwL;
BYTE	bHigh;
	dwL    = dwLinStart >> 2;
	reg_VIDEO_INDEX = 0;
	VidIO[0][0x40] = (BYTE)dwL;
    VidIO[0][0x41] = (BYTE) ( dwL >> 8 );
	VidIO[0][0x44] = (VidIO[0][0x44] & ~0x0F) | (BYTE) ( ( dwL & 0xff0000L)  >> 16 );

	dwL    = dwLinEnd >> 2;
	reg_VIDEO_INDEX = 0;
	VidIO[0][0x42] = (BYTE)dwL;
	VidIO[0][0x43] = (BYTE) ( dwL >> 8 );
	VidIO[0][0x44] = (VidIO[0][0x44] & ~0xF0) | ((BYTE) ( ( dwL & 0xff0000L) >> 16 ) << 4);

	wPitch = wPitch >> 3;		// 64-bit boundary
	VidIO[1][0xA9] = LOBYTE(wPitch);
	bHigh = HIBYTE(wPitch) << 6;
	VidIO[1][0xA8] = (VidIO[1][0xA8] & ~0xC0) | bHigh;
}

void VPEVBI_SetVBISrc(WORD dwXStart, WORD dwXEnd,
					  WORD dwYStart, WORD dwYEnd)
{
	//dwXEnd = dwXEnd * 2;
	VidIO[1][0xAA] = (BYTE) dwXStart;
	VidIO[1][0xAB] = (BYTE) dwXEnd;
	VidIO[1][0xAC] = (VidIO[1][0xAC] & ~0x07) | (BYTE) ( dwXStart >> 8 );
	VidIO[1][0xAC] = (VidIO[1][0xAC] & ~0x70) | ((BYTE) ( dwXEnd >> 8 ) << 4);


	VidIO[1][0xAD] = (BYTE) dwYStart;
	VidIO[1][0xAE] = (BYTE) dwYEnd;
	VidIO[1][0xAF] = (VidIO[1][0xAF] & ~0x07) | (BYTE) ( dwYStart >> 8 );
	VidIO[1][0xAF] = (VidIO[1][0xAF] & ~0x70) | ((BYTE) ( dwYEnd >> 8 ) << 4);
}

void VPEVBI_Enable(BOOL bEnable)
{
	WORD wTemp;
	VidIO[0][0xFA] = 0;

	if(bEnable) {
		VidIO[1][0xA4] = (VidIO[1][0xA4] & ~0x40) | 0x40;
		wTemp = ((IGS5000*)g_pGPE)->ReadTVReg(0xC008);
		((IGS5000*)g_pGPE)->WriteTVReg(0xC008, wTemp | 0x0002);
	}
	else {
		VidIO[1][0xA4] = (VidIO[1][0xA4] & ~0x40) | 0x00;
		wTemp = ((IGS5000*)g_pGPE)->ReadTVReg(0xC008);
		((IGS5000*)g_pGPE)->WriteTVReg(0xC008, wTemp & 0xFFFD);

	}
}

void VBI_InterruptSet(BOOL bEnable)
{
	VidIO[0][0xFA] = 0;

	if(bEnable) {
//		VidIO[0][0xea] = (VidIO[0][0xea] & ~0x40) | 0x40;
		VidIO[1][0xA8] = (VidIO[1][0xA8] & ~0x20) | 0x20;
	}
	else {
//		VidIO[0][0xea] = (VidIO[0][0xea] & ~0x40) | 0x00;
		VidIO[1][0xA8] = (VidIO[1][0xA8] & ~0x20) | 0x00;
	}
}

void VBI_ResetIntr()
{
	BYTE tmp;

	VidIO[0][0xFA] = 0;

	tmp = VidIO[0][0xe5];
	VidIO[1][0xA8] = (VidIO[1][0xA8] & ~0x20) | 0x00;
	VidIO[1][0xA8] = (VidIO[1][0xA8] & ~0x20) | 0x20;
//	VidIO[0][0xea] = (VidIO[0][0xea] & ~0x40) | 0x00;
//	VidIO[0][0xea] = (VidIO[0][0xea] & ~0x40) | 0x40;
}
#endif // ENABLEVBI

