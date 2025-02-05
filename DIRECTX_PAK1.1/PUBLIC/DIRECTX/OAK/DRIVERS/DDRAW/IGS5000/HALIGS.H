
//	COPYRIGHT (C) 1999-2000, IGS TECHNOLOGIES, INC. (iGST)

#define ENABLEVBI 1

#if ENABLEVBI
#define VBI_STARTLINE  0
#define VBI_ENDLINE  0x0E
#define VBI_WIDTH 1728
#define MAXVBI_BUFFER_SIZE 0x10000
void VPEVBI_SetVBIDst(DWORD, DWORD, WORD);
void VPEVBI_SetVBISrc(WORD, WORD, WORD, WORD);
void VPEVBI_Enable(BOOL);
void VBI_InterruptSet(BOOL);
void VBI_ResetIntr();

#endif

//General decleration
#define GR_INDEX		0x3CE
#define GR_DATA 		0x3CF
#define CR_INDEX		0x3D4
#define CR_DATA 		0x3D5
#define SR_INDEX		0x3C4
#define SR_DATA 		0x3C5
#define STATUS_1		0x3DA

//STATUS_1 decleration
#define VSYN_ACTIVE		0x8


//SR regs
#define reg_SEQ0						VidIO[1][0x00]
#define reg_SEQ1						VidIO[1][0x01]
#define reg_SEQ2						VidIO[1][0x02]
#define reg_SEQ3						VidIO[1][0x03]

#define reg_CAP_RATE_CTL				VidIO[1][0xA0]
#define reg_CAP_ANC_DATACNT				VidIO[1][0xA1]
#define reg_VBI_CTL_I					VidIO[1][0xA2]
#define reg_VBI_CTL_II					VidIO[1][0xA3]
#define reg_MIX_CAP_CTL_II				VidIO[1][0xA4]

#define reg_MIX_CAP_CTL_III				VidIO[1][0xA5]
#define reg_XVIDEO_ABCTL_II		VidIO[1][0xA6]
#define reg_XVIDEO_BCTL_II		VidIO[1][0xA7]
#define reg_VBI_SRCWIDTH_HI				VidIO[1][0xA8]
#define reg_VBI_SRCWIDTH_LO				VidIO[1][0xA9]


#define reg_SR_ED						VidIO[1][0xED]
#define reg_SR_EF						VidIO[1][0xEF]


//GR Regs
#define reg_SCREEN_START_HI				VidIO[0][0x10]
#define reg_EXT_VOVERFLOW				VidIO[0][0x11]
#define reg_CRT_IRQ						VidIO[0][0x12]
#define reg_MEM_FETCH_LO				VidIO[0][0x14]
#define reg_MEM_FETCH_HI				VidIO[0][0x15]
#define reg_CRT_OFFSET_HI				VidIO[0][0x15]
#define reg_CURRENT_VLINE_LO			VidIO[0][0x18]
#define reg_CURRENT_VLINE_HI			VidIO[0][0x19]

#define reg_BIUMisc						VidIO[0][0x33]
#define LA_LO							0x34
#define LA_HI							0x35
#define V_SRC_START_LO					0x40
#define V_SRC_START_MI					0x41
#define V_SRC_START_HI					0x42
#define V_SRC_STRIDE_LO 				0x43
#define reg_GR43						VidIO[0][0x43]
#define V_SRC_STRIDE_HI 				0x44
#define V_SRC_FETCH_LO					0x4D
#define V_SRC_FETCH_HI					0x44
#define V_DST_X1_LO						0x45
#define V_DST_X1_HI						0x46
#define reg_GR46						VidIO[0][0x46]
#define reg_GR47						VidIO[0][0x47]
#define V_DST_X2_LO						0x47
#define V_DST_X2_HI						0x48
#define V_DST_Y1_LO						0x49
#define reg_GR49						VidIO[0][0x49]
#define V_DST_Y1_HI						0x4A
#define V_DST_Y2_LO						0x4B
#define V_DST_Y2_HI						0x4C

#define A_STARTX_LO						0x40
#define A_STARTX_HI						0x41
#define A_ENDX_HI						0x41
#define A_ENDX_LO						0x42
#define A_STARTY_LO						0x43
#define A_STARTY_HI						0x44
#define A_ENDY_HI						0x44
#define A_ENDY_LO						0x45
#define A_RED							0x46
#define A_GREEN 						0x47
#define A_BLUE							0x48
#define ALPHA_CTRL_I					0x49
    #define ASRCA_IS_PRIMARY		0x00
    #define ASRCA_IS_VIDEO1		0x01
    #define ASRCA_IS_VIDEO2		0x02
    #define ASRCA_IS_LUT		0x03
    #define ASRCA_INVERT		0x04

    #define LUT_FROM_ATF		0x00
    #define LUT_FROM_VIDEO2		0x08
    #define LUT_FROM_VSYNC		0x10
    #define LUT_FROM_CPUWRITE		0x18

    #define ALPHA_FROM_PIXEL		0x00
    #define ALPHA_FROM_VIDEO2		0x20
    #define ALPHA_FROM_LUT		0x40
    #define ALPHA_FROM_REGISTER 	0x60
    #define ALPHA_INVERT		0x80

#define ALPHA_CTRL_II					0x4A
    #define AOUT_INVERT 		0x04
    #define ALPHA_1BPP			0x00
    #define ALPHA_2BPP			0x040
    #define ALPHA_4BPP			0x080
    #define ALPHA_8BPP			0x0C0

#define ALPHA_CTRL_III		0x4B

    #define ALPHA_FULLSCREEN		0x40
    #define ALPHA_ENABLE		0x80

#define ALPHA_CTRL_IV		0x4C
    #define GENLOCK_ENABLE		0x20

#define ALPHA_CTRL_V		0x4D
    #define ASRCB_IS_VIDEO1		0x00
    #define ASRCB_IS_VIDEO2		0x10
    #define ASRCB_IS_PRIMARY		0x20
    #define ASRCB_IS_RAMTABLE		0x30
    #define ASRCB_INVERT		0x40

#define ALPHA_CTRL_VI		0x4E

#define ALPHA_WINCTRL		0x4E
    #define AWIN0_DISABLE		0x01
    #define AWIN1_DISABLE		0x02
    #define AWIN2_DISABLE		0x04
    #define AWIN3_DISABLE		0x08
    #define ALPHA_WINKEY		0x20


#define reg_COP_STAT					VidIO[0][0x5B]
	#define COP_BUSY					0x80

#define reg_MEM_CTL_I					VidIO[0][0x70]
	#define SEQ_FIFOFLUSH				0x40
	#define SEQ_RESET					0x80

#define reg_MEM_CTL_II					VidIO[0][0x71]
#define reg_MEM_CTL_III 				VidIO[0][0x72]

#define reg_CRT_FIFO_LO 				0x74
#define reg_CRT_FIFO_HI 				0x75


#define reg_CAP_NEW_CTL_I				VidIO[0][0x88]
#define reg_GR88						VidIO[0][0x88]
#define reg_GR89						VidIO[0][0x89]
#define reg_CHROMA_CMP_RED_HI		VidIO[0][0x8A]
#define reg_CHROMA_CMP_RED_LO		VidIO[0][0x8B]
#define reg_CHROMA_CMP_GREEN_HI 	VidIO[0][0x8C]
#define reg_CHROMA_CMP_GREEN_LO 	VidIO[0][0x8D]
#define reg_CHROMA_CMP_BLUE_HI		VidIO[0][0x8E]
#define reg_CHROMA_CMP_BLUE_LO		VidIO[0][0x8F]

#define CHIPID_LO						VidIO[0][0x91]
#define CHIPID_MI						VidIO[0][0x92]
#define CHIPID_HI						VidIO[0][0x93]

#define reg_GRAE						VidIO[0][0xAE]

#define reg_VCLK0						VidIO[0][0xB0]
#define reg_VCLK1						VidIO[0][0xB1]
#define reg_MCLK0						VidIO[0][0xB2]
#define reg_MCLK1						VidIO[0][0xB3]
#define reg_PLL_TEST					VidIO[0][0xB4]
#define reg_BX_MISC0					VidIO[0][0xB5]
#define reg_BX_MISC1					VidIO[0][0xB6]
#define reg_FEATCON					VidIO[0][0xB7]
#define reg_BX_MISC2					VidIO[0][0xB8]
#define reg_MISC_CLK					VidIO[0][0xB9]
#define reg_VCLK2						VidIO[0][0xBA]
#define reg_MCLK2						VidIO[0][0xBB]
#define reg_VCLK_CTL					VidIO[0][0xBC]
#define reg_MCLK_CTL					VidIO[0][0xBD]
#define reg_XT_CTL						VidIO[0][0xBE]
#define reg_XB_CTL						VidIO[0][0xBF]


#define reg_MEMORY_START_L(e)		VidIO[e][0xC0]
#define reg_MEMORY_START_M(e)		VidIO[e][0xC1]
#define reg_MEMORY_START_H(e)		VidIO[e][0xC2]
#define reg_MEMORY_START_L1(e)		VidIO[e][0x41]
#define reg_MEMORY_START_M1(e)		VidIO[e][0x42]
#define reg_MEMORY_START_H1(e)		VidIO[e][0xAD]
#define reg_MEMORY_PITCH_L(e)		VidIO[e][0xC3]
#define reg_MEMORY_PITCH_H(e)		VidIO[e][0xC4]
#define reg_DEST_RECT_LEFT_L(e) 	VidIO[e][0xC5]
#define reg_DEST_RECT_LEFT_H(e) 	VidIO[e][0xC6]
#define reg_DEST_RECT_RIGHT_L(e)	VidIO[e][0xC7]
#define reg_DEST_RECT_RIGHT_H(e)	VidIO[e][0xC8]
#define reg_DEST_RECT_TOP_L(e)		VidIO[e][0xC9]
#define reg_DEST_RECT_TOP_H(e)		VidIO[e][0xCA]
#define reg_DEST_RECT_BOTTOM_L(e)	VidIO[e][0xCB]
#define reg_DEST_RECT_BOTTOM_H(e)	VidIO[e][0xCC]
#define reg_MEMORY_OFFSET_PHASE(e)	VidIO[e][0xCD]
#define reg_COLOR_CMP_RED(e)		VidIO[e][0xCE]
#define reg_COLOR_CMP_GREEN(e)		VidIO[e][0xCF]
#define reg_COLOR_CMP_BLUE(e)		VidIO[e][0xD0]
#define reg_DDA_X_INIT_L(e)		VidIO[e][0xD1]
#define reg_DDA_X_INIT_H(e)		VidIO[e][0xD2]
#define reg_DDA_X_INC_L(e)		VidIO[e][0xD3]
#define reg_DDA_X_INC_H(e)		VidIO[e][0xD4]
#define reg_DDA_Y_INIT_L(e)		VidIO[e][0xD5]
#define reg_DDA_Y_INIT_H(e)		VidIO[e][0xD6]
#define reg_DDA_Y_INC_L(e)		VidIO[e][0xD7]
#define reg_DDA_Y_INC_H(e)		VidIO[e][0xD8]
#define reg_FIFO_TIMING_CTL_L(e)	VidIO[e][0xD9]
#define reg_FIFO_TIMING_CTL_H(e)	VidIO[e][0xDA]
#define reg_VIDEO_FORMAT(e)		VidIO[e][0xDB]
	#define VFMT_YUV422		0
	#define VFMT_RGB555		1
	#define VFMT_RGB565		2
	#define VFMT_RGB888		3
	#define VFMT_RGB8888		4
	#define VFMT_RGB8IDX		5
	#define VFMT_RGB444		6
	#define VFMT_RGB8		7


#define reg_DISP_CTL_I(e)		VidIO[e][0xDC]
#define reg_FIFO_CTL_I(e)		VidIO[e][0xDD]
#define reg_MISC_CTL_I(e)		VidIO[e][0xDE]

#define reg_XVID1_ROM					VidIO[0][0xE5]
#define reg_VFAC_CTL_MODE_I				VidIO[0][0xE8]
#define reg_VFAC_CTL_MODE_II			VidIO[0][0xE9]
#define reg_VFAC_CTL_MODE_III		VidIO[0][0xEA]
#define reg_CAPTURE_ADDR_L				VidIO[0][0xEB]
#define reg_CAPTURE_ADDR_M				VidIO[0][0xEC]
#define reg_CAPTURE_ADDR_H				VidIO[0][0xED]
#define reg_CAPTURE_PITCH_L				VidIO[0][0xEE]
#define reg_CAPTURE_CTL_MISC			VidIO[0][0xEF]

#define VIDEO_PLANNAR_MODE	0xF9
	#define EN_YUV420	1
	#define EN_YUV411	2
	#define EN_DBUF 	4

#define reg_VIDEO_INDEX 		VidIO[0][0xFA]
#define YUV_OFS_LO		0xFC
#define YUV_OFS_HI		0xFD

#define reg_FC							VidIO[0][0xFC]


#define MAX_IGS_REG		48


// for VPE
/*
 *  Below code for video port
 */
#define reg_VP_CROPLEFT_LO				VidIO[0][0x60]
#define reg_VP_CROPLEFT_HI				VidIO[0][0x61]
#define reg_VP_CROPRIGHT_LO				VidIO[0][0x62]
#define reg_VP_CROPRIGHT_HI				VidIO[0][0x63]
#define reg_VP_CROPTOP_LO				VidIO[0][0x64]
#define reg_VP_CROPTOP_HI				VidIO[0][0x65]
#define reg_VP_CROPBOTTOM_LO			VidIO[0][0x66]
#define reg_VP_CROPBOTTOM_HI			VidIO[0][0x67]
#define reg_VP_HDDA_INIT_LO				VidIO[0][0x68]
#define reg_VP_HDDA_INIT_HI				VidIO[0][0x69]
#define reg_VP_HDDA_INC_LO				VidIO[0][0x6A]
#define reg_VP_HDDA_INC_HI				VidIO[0][0x6B]
#define reg_VP_VDDA_INIT_LO				VidIO[0][0x6C]
#define reg_VP_VDDA_INIT_HI				VidIO[0][0x6D]
#define reg_VP_VDDA_INC_LO				VidIO[0][0x6E]
#define reg_VP_VDDA_INC_HI				VidIO[0][0x6F]


#define reg_VP_CCIR656_CAPMODE_I		VidIO[0][0xA4]
	#define Y_MIRROR				0x40
	#define X_MIRROR				0x80
	#define DISCARD_VREF			0x04

#define reg_VP_CCIR656_CAPMODE_II		VidIO[0][0xA5]
#define reg_VP_BM_I						VidIO[0][0xA6]
#define reg_VP_MEMSTART_LOW1			VidIO[0][0xA7]
#define reg_VP_MEMSTART_MID1			VidIO[0][0xA8]
#define reg_VP_MEMSTART_HI1				VidIO[0][0xA9]
#define reg_VP_DOUBLEBUFFERCTL			VidIO[0][0xAB]
#define reg_VP_BM_OFS					VidIO[0][0xAA]
#define reg_VP_ZRGB_CTL 				VidIO[0][0xAB]


#define reg_VP_MEMSTART_LOW				VidIO[0][0xEB]
#define reg_VP_MEMSTART_MID				VidIO[0][0xEC]
#define reg_VP_MEMSTART_HI				VidIO[0][0xED]
#define reg_VP_PITCH_LO 				VidIO[0][0xEE]
#define reg_CAPA_CTL_MISC				VidIO[0][0xEF]

#define reg_GRDE						VidIO[0][0xDE]

// TV's MemMap Registers
#define   TV_BASE		0xE000
#define   TV_DACOUTPUT		0x434		+ TV_BASE


// Video Registers
#define   MAX_SRC_PITCH 	352*2	//    = 704 byte


// Display Control I
#define GOTHRU_DAC		1
#define WINKEY			2
#define NO_VERT_INTERPOLATION	0x20
#define VIDEO_ON		0x80


// VAFC
#define CHROMAKEY		0x20
#define WINKEY_NOCRT		0x10



// CR Register Decleration
#define reg_SCREEN_START_MI				VidIO[2][0x0C]
#define reg_SCREEN_START_LO				VidIO[2][0x0D]


//define which decoder we can support here
#define CCIR656     0
#define BROOKTREE   1
#define PHILIPS     2
#define HREFH_VREFL 3
#define HREFH_VREFH 4
#define HREFL_VREFL 5
#define HREFL_VREFH 6

typedef struct {
	DWORD				dwInFlip;
	DWORD				dwTicksPerFrame;
	DWORD				dwCurrLine;
	DWORD				dwTicksAtFlip;		/* for each surface type */

} FLIPRECORD;


typedef struct {
    LPDDRAWI_DDRAWSURFACE_LCL pSrcSurf;
    DWORD		dwID   ;	//0 if available
    DWORD		dwDstX1;
    DWORD		dwDstY1;
    DWORD		dwDstW ;	//w and h in pixel
    DWORD		dwDstH ;
    DWORD		dwDstCKHi;
    DWORD		dwDstCKLo;
    DWORD		dwSrcW ;	//w and h in pixel
    DWORD		dwSrcH ;
    DWORD		dwSrcCKHi;
    DWORD		dwSrcCKLo;
    DWORD		pSrcStart;
    DWORD		dwSrcPitch;
    DWORD		dwSurfBpp;
    DWORD		dwFlags;
    DWORD		dwFlipOfs;
    //Hardware-Used stuffs
    DWORD		dwHWFlags;	// 1 if frontend conversion
    long		dwVidEngNo;	// -1,0/1/2 or
    WORD		wVideoFmt;
    WORD		wVideoCtrl;
    WORD		wVFIFO;
    WORD		wFetch;

} OVERLAYSURF,FAR *LPOVERLAYSURF;


extern IndexedReg8 VidIO[];
extern int iOvlCnt;
extern int iOvlMax;
extern OVERLAYSURF ovl[];
extern FLIPRECORD  flipRec;


/* graphic functions */
WORD GetScanline();

/* video functions */
void	InitVideo(int e);
void	SetScreenStart(DWORD lOffset);
void	SetColorKey(int, BYTE, BYTE, BYTE);
void	ColorkeyOn(int e);
void	ColorkeyOff(int e);
void	SetChromaKey(int, BYTE,BYTE,BYTE,BYTE,BYTE,BYTE);
void	ChromaOn(int);
void	ChromaOff(int);
void	VideoOn(int);
void	VideoOff(int);
void	VideoFormat(int e, WORD vfmt);
void	VideoControl(int e, WORD vctrl);
void	SetVideoWindow(int,  WORD, WORD, WORD, WORD );
void	SetVideoScale(int, WORD, WORD, WORD, WORD );
void	SetVideoSrc(int, DWORD, WORD,WORD);
void SetVideoBackBufferSrc(DWORD);

/*  VPE functions */
void	VPE_Init();
void	VPE_SetDst(DWORD, WORD);
void	VPE_SetVPBackBufferDst(DWORD);
void	VPE_EnableVPDoubleBuffer(BYTE);
void	VPE_EnableOverDoubleBuffer(BYTE);
void	VPE_SetCrop(WORD, WORD, WORD, WORD);

void	VPE_DDA(WORD, WORD, WORD, WORD);
void	VPE_DDA_On();
void	VPE_DDA_Off();

void	VPE_On();
void	VPE_Off();

void	VPE_Freeze();
void	VPE_UnFreeze();

void	VPE_ConnectDecoder(WORD wGUID);
void	VPE_NonInterlacedSrc();
void	VPE_InterlacedSrc();
void	VPE_InvertPolarity_On();
void	VPE_InvertPolarity_Off();
void	VPE_XMirror_On();
void	VPE_XMirror_Off();
void	VPE_YMirror_On();
void	VPE_YMirror_Off();

BOOL	VPE_CurrField();
int		VPE_CurrCapLine();
BOOL	VPE_DetectVideoSignal(WORD wGUID);
void	VPE_DiscardVRef();
void	VPE_UseVRef();

/*FIFO functions*/

void  IGS_ChangeOverlayFIFO(int wOverlayIndex);
void Out_Video_Reg( BYTE bIndex, BYTE bValue );
void Out_Video_Reg_M( BYTE bIndex, BYTE bValue, BYTE bMask );
void Out_SEQ_Reg( BYTE bIndex, BYTE bValue );
void Out_SEQ_Reg_M( BYTE bIndex, BYTE bValue, BYTE bMask );
