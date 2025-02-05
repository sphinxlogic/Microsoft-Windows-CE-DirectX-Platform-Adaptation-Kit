// ----------------------------------------------------------------------------- 
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//  Copyright (c) 1997-1998  Microsoft Corporation
//
//  Module Name:
//
//  blt.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#include "precomp.h"

#ifdef ENABLE_ACCELERATION
unsigned char IGSRop[]={
	0x00, 0x01, 0x02, 0x03, 0x10, 0x08, 0x12, 0x13,
	0x20, 0x21, 0x04, 0x23, 0x30, 0x31, 0x32, 0x0C,
	0x04, 0x08, 0x06, 0x07, 0x14, 0x15, 0x16, 0x17,
	0x24, 0x25, 0x26, 0x27, 0x34, 0x35, 0x36, 0x37,
	0x08, 0x09, 0x04, 0x0B, 0x18, 0x19, 0x1A, 0x1B,
	0x28, 0x29, 0x2A, 0x2B, 0x38, 0x39, 0x3A, 0x3B,
	0x0C, 0x0D, 0x0E, 0x0C, 0x1C, 0x1D, 0x1E, 0x1F,
	0x2C, 0x2D, 0x2E, 0x2F, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x41, 0x42, 0x43, 0x02, 0x51, 0x52, 0x53,
	0x60, 0x61, 0x62, 0x63, 0x70, 0x71, 0x72, 0x73,
	0x02, 0x45, 0x46, 0x47, 0x54, 0x0A, 0x56, 0x57,
	0x64, 0x65, 0x06, 0x67, 0x74, 0x75, 0x76, 0x0E,
	0x48, 0x49, 0x4A, 0x4B, 0x58, 0x59, 0x06, 0x5B,
	0x68, 0x69, 0x6A, 0x6B, 0x78, 0x79, 0x7A, 0x7B,
	0x4C, 0x4D, 0x4E, 0x4F, 0x5C, 0x5D, 0x5E, 0x0E,
	0x6C, 0x6D, 0x6E, 0x6F, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x90, 0x91, 0x92, 0x93,
	0x01, 0xA1, 0xA2, 0xA3, 0xB0, 0xB1, 0xB2, 0xB3,
	0x84, 0x85, 0x86, 0x87, 0x94, 0x95, 0x96, 0x97,
	0xA4, 0x09, 0xA6, 0xA7, 0xB4, 0xB5, 0xB6, 0xB7,
	0x01, 0x89, 0x8A, 0x8B, 0x98, 0x09, 0x9A, 0x9B,
	0xA8, 0xA9, 0x05, 0xAB, 0xB8, 0xB9, 0xBA, 0x0D,
	0x8C, 0x8D, 0x8E, 0x8F, 0x9C, 0x9D, 0x9E, 0x9F,
	0xAC, 0xAD, 0xAE, 0x0D, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xD0, 0xD1, 0xD2, 0xD3,
	0xE0, 0xE1, 0xE2, 0xE3, 0x03, 0xF1, 0xF2, 0xF3,
	0xC4, 0xC5, 0xC6, 0xC7, 0xD4, 0xD5, 0xD6, 0xD7,
	0xE4, 0xE5, 0xE6, 0xE7, 0xF4, 0x0B, 0xF6, 0xF7,
	0xC8, 0xC9, 0xCA, 0xCB, 0xD8, 0xD9, 0xDA, 0xDB,
	0xE8, 0xE9, 0xEA, 0xEB, 0xF8, 0xF9, 0x07, 0xFB,
	0x03, 0xCD, 0xCE, 0xCF, 0xDC, 0x0B, 0xDE, 0xDF,
	0xEC, 0xED, 0x07, 0xEF, 0xFC, 0xFD, 0xFE, 0x0F
};

#endif


//
//  BltPrepare - pre Blt prepare
//  inputs:  pBltParms - Blt parameters
//  outputs: pBltParms - Blt parameters
//
SCODE IGS5000::BltPrepare(GPEBltParms *pBltParms) {

	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::BltPrepare rop4: 0x%04x\r\n"), pBltParms->rop4 ));
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("Dest Format: %d, Dest Rect: t:%d,l:%d,b:%d,r:%d Dest GPESurf 0x%08x\r\n"),
	(int)pBltParms->pDst->Format(),
	(pBltParms->prclDst != NULL) ? pBltParms->prclDst->top : 0,
	(pBltParms->prclDst != NULL) ? pBltParms->prclDst->left : 0,
	(pBltParms->prclDst != NULL) ? pBltParms->prclDst->bottom : 0,
	(pBltParms->prclDst != NULL) ? pBltParms->prclDst->right : 0,
	pBltParms->pDst ));
	if( pBltParms->prclSrc && pBltParms->pSrc ) {
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("Src format: %d, Src Rect: %d,%d,%d,%d Src GPESurf 0x%08x\r\n"),
		(int)pBltParms->pSrc->Format(),
		pBltParms->prclSrc->top,
		pBltParms->prclSrc->left,
		pBltParms->prclSrc->bottom,
		pBltParms->prclSrc->right,
		pBltParms->pSrc ));
	}

	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("Blt preparation phase\r\n")));
	DEBUGWAIT(GPE_ZONE_BLT_LO)

	pBltParms->pBlt = EmulatedBlt; // catch all

	// Look for operations that can be accelerated in hardware
#ifdef ENABLE_ACCELERATION

	// Can only accelerate in S3 if destination is video memory
	if( !pBltParms->pDst->InVideoMemory() ) {
#ifdef ENABLE_EMULATION
	// If software emulations are enabled, go try those
	goto LCheckEmulation;
#else
	return S_OK;
#endif
	}

	switch( pBltParms->rop4 ) {
	case 0x0000:    // BLACKNESS
		DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - BLACKNESS\r\n")));
		WaitForNotBusy();
		reg_Fore_Mix = Mix_Blackness;
		pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedFillRect;
		return S_OK;

	case 0xFFFF:    // WHITENESS
		DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - WHITENESS\r\n")));
		WaitForNotBusy();
		reg_Fore_Mix = Mix_Whiteness;
		pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedFillRect;
		return S_OK;

	case 0xF0F0:    // PATCOPY
		if( pBltParms->solidColor == -1 ) {
			DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - PATCOPY - patterned brush\r\n")));
			DEBUGWAIT(GPE_ZONE_BLT_LO);

			if ((pBltParms->pBrush->Width()!=8)  ||
				(pBltParms->pBrush->Height()!=8)) {
				break;
			}

			uchar *src_ptr = (uchar*)(pBltParms->pBrush->Buffer());
			uchar *pat_ptr = (uchar*) m_PatPtr;
			WaitForNotBusy();
#ifdef FB16BPP
			for (int i=0;i<128;i++) {
				*pat_ptr++ = *src_ptr++;
			}
#else
			for (int i=0;i<64;i++) {
				*pat_ptr++ = *src_ptr++;
			};
#endif
			reg_Fore_Mix = Mix_S;
			reg_Back_Mix = Mix_S;
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedColorPatBlt;
			return S_OK;

		} else {
			DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - PATCOPY - solid brush\r\n")));
			DEBUGWAIT(GPE_ZONE_BLT_LO)
			SelectSolidColor( pBltParms->solidColor );
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedFillRect;
			return S_OK;
		}
		break;

	case 0xAAF0:
	/* use textout only if no alpha textout */
		if (pBltParms->solidColor!=-1) {
			SelectSolidColor( pBltParms->solidColor );
			reg_Fore_Mix = 0xF0;
			reg_Src2_Start_Ptr = 0;
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedTextBlt;
		}
		return S_OK;

	case 0x5A5A:    /* PATINVERT */
		if( pBltParms->solidColor != -1 ) {
			DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - PATINVERT - solid brush\r\n")));
			WaitForNotBusy();
			reg_Fore_Mix = 06;
			reg_Fore_Color=pBltParms->solidColor;
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedFillRect;
			return S_OK;
		}
		break;

	case 0x5555:    /* DSTINVERT */
		WaitForNotBusy();
		reg_Fore_Mix = IGSRop[0x55];
		pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedFillRect;
		return S_OK;

	case 0xCCCC:
	case 0x6666:    /* SRCINVERT */
	case 0x8888:    /* SRCAND    */
	case 0xEEEE:    /* SRCPAINT  */
		if ( pBltParms->pConvert ||
			pBltParms->bltFlags&BLT_STRETCH) {
			break;
		}

		//this just for debug
		if( pBltParms->bltFlags & BLT_TRANSPARENT ) {
			// Transparent Blt
			DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("Transparent Blt!\r\n")));		
//	      break;
		}
		//just for debug

		//since mono surface is not in video memory, so check it here
		if (pBltParms->pSrc->Format()==gpe1Bpp) {
			reg_Fore_Mix = IGSRop[pBltParms->rop4&0xFF];
			reg_Back_Mix = IGSRop[pBltParms->rop4&0xFF];
			reg_Back_Color     = *(unsigned long*)pBltParms->pLookup;
			reg_Fore_Color     = *(unsigned long*)(pBltParms->pLookup+1);
			reg_Src2_Start_Ptr = 0;
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedMono2ScrBlt;
			return S_OK;
		}

		if (pBltParms->pLookup) {
			break;
		}
		reg_Fore_Mix = IGSRop[pBltParms->rop4&0xFF];

		if (pBltParms->pSrc->InVideoMemory()) {
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedSrcCopyBlt;
		}
		else if (pBltParms->pSrc->Format()==pBltParms->pDst->Format()) {
			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedClr2ScrBlt;
		}
		else {
			DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("Unsupported format!\r\n")));
		}
		return S_OK;

//    case 0xCCAA:
//		DebugBreak();
//		reg_Fore_Mix = 0x03;
//		if (pBltParms->pSrc->InVideoMemory()) {
//			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedTransBlt;
//		}
//		else {
//			pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))AcceleratedClr2ScrTransBlt;
//		}
//		return S_OK;

		default:        // some random ROP4
		;
	}
	return S_OK;
#endif // ENABLE_ACCELERATION

	// Look for specific operations to emulate in software
#ifdef ENABLE_EMULATION

#ifdef ENABLE_ACCELERATION
LCheckEmulation:;
#endif

	//
	// Bail on any parameter values that would make this
	// blt non-emulatable
	//

#ifdef FB16BPP
	if ( EGPEFormatToBpp[pBltParms->pDst->Format()] != 16   ||
	(pBltParms->bltFlags & (BLT_TRANSPARENT | BLT_STRETCH)) ||
	 pBltParms->pLookup                 ||
	 pBltParms->pConvert) {
#else  //FB16BPP
	if ( EGPEFormatToBpp[pBltParms->pDst->Format()] != 8    ||
	(pBltParms->bltFlags & (BLT_TRANSPARENT | BLT_STRETCH)) ||
	 pBltParms->pLookup                 ||
	 pBltParms->pConvert) {
#endif  //FB16BPP
	return S_OK;
	}

#define FUNCNAME(basename) \
	((SCODE (GPE::*)(struct GPEBltParms *))Emulator::Emulated##basename)

	switch( pBltParms->rop4 ) {
#ifndef FB16BPP
	case 0x0000:    // BLACKNESS
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - BLACKNESS\r\n")));
	pBltParms->solidColor = 0;
	pBltParms->pBlt = FUNCNAME(BltFill08);
	return S_OK;

	case 0xFFFF:    // WHITENESS
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - WHITENESS\r\n")));
	pBltParms->solidColor = 0x00ffffff;
	pBltParms->pBlt = FUNCNAME(BltFill08);
	return S_OK;

	case 0xF0F0:    // PATCOPY
	if( pBltParms->solidColor != -1 ) {
		DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - PATCOPY - solid brush\r\n")));
		pBltParms->pBlt = FUNCNAME(BltFill08);
		return S_OK;
	}
	break;

	case 0x5A5A:    // PATINVERT
	if( pBltParms->solidColor != -1 ) {
		DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - PATINVERT - solid brush\r\n")));
		pBltParms->pBlt = FUNCNAME(BltPatInvert08);
		return S_OK;
	}
	break;

	case 0x5555:    // DSTINVERT
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - DSTINVERT\r\n")));
	pBltParms->pBlt = FUNCNAME(BltDstInvert08);
	return S_OK;

	case 0xCCCC:    // SRCCOPY
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - SRCCOPY\r\n")));
	pBltParms->pBlt = FUNCNAME(BltSrcCopy0808);
	return S_OK;

	case 0x8888:    // SRCAND
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - SRCAND\r\n")));
	pBltParms->pBlt = FUNCNAME(BltSrcAnd0808);
	return S_OK;

	case 0xEEEE:    // SRCPAINT
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - SRCPAINT\r\n")));
	pBltParms->pBlt = FUNCNAME(BltSrcPaint0808);
	return S_OK;

	case 0x6666:    // SRCINVERT
	DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - SRCINVERT\r\n")));
	pBltParms->pBlt = FUNCNAME(BltSrcInvert0808);
	return S_OK;
#endif  //!FB16BPP

	case 0xAAF0:    // Special PATCOPY rop for text output -- fill where mask is 1.
#ifndef FB16BPP
	// Not a pattern brush?
	if( pBltParms->solidColor != -1 ) {
		DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - 0xAAF0\r\n")));
		pBltParms->pBlt = FUNCNAME(BltText08);
		return S_OK;
	}
#else //!FB16BPP
	// Not a pattern brush?
	if( pBltParms->solidColor != -1 ) {
		if (pBltParms->pMask->Format() == gpe1Bpp) {
		DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - 0xAAF0\r\n")));
		pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))Emulator::EmulatedBltText16;
		return S_OK;
		} else  {
		// Anti-aliased text
		DEBUGMSG(GPE_ZONE_BLT_LO,(TEXT("IGS5000::Blt - 0xAAF0 alpha\r\n")));
		pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))Emulator::EmulatedBltAlphaText16;
		return S_OK;
		}
	}
#endif  //!FB16BPP
	break;

	default:
		;
	}

#endif // ENABLE_EMULATION

	return S_OK;
}

//
//  BltComplete - post Blt
//  inputs:  pBltParms - Blt parameters
//  outputs: pBltParms - Blt parameters
//
SCODE IGS5000::BltComplete( GPEBltParms *pBltParms ) {
	return S_OK;
}


#ifdef ENABLE_ACCELERATION
//
//  AcceleratedFillRect - Fill rectangle Blt
//  inputs:  pBltParms - Blt parameters
//  outputs: none
//
SCODE IGS5000::AcceleratedFillRect( GPEBltParms *pBltParms ) {

	DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("AcceleratedFillRect\r\n")));

	int x1 = pBltParms->prclDst->left;
	int y1 = pBltParms->prclDst->top;
	int x2 = pBltParms->prclDst->right;
	int y2 = pBltParms->prclDst->bottom;
	int width;
	int height;
	ulong dstptr = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs();

	unsigned short direction;

	// set the direction
	if( (x2 > x1) && (y2 > y1)) {
		direction = Right_Down;
		width = x2 - x1;
		height = y2 - y1;
	} 
	else if (x2 > x1) {
		direction = Right_Up;
		width = x2 - x1;
		height = y1 - y2;
	} 
	else if (y2 > y1) {
		direction = Left_Down;
		width = x1 - x2;
		height = y2 - y1;
	} 
	else {
		direction = Left_Up;
		width = x2 - x1;
		height = y2 - y1;
	}


	// Draw rectangle
	dstptr += (y1* ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()+x1);
	WaitForNotBusy();
	reg_Dst_Map_Width  = ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()-1;
	reg_Cop_control = 0;
	reg_Op_Dimension_1 = width-1;
	reg_Op_Dimension_2 = height - 1;
	reg_Dst_Start_Ptr  = dstptr;

	ULONG op  = COP_FILLRECT | direction;
	reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
	reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/

	return S_OK;
}

//
//  AcceleratedSrcCopyBlt - source copy Blt
//  inputs:  pBltParms - Blt parameters
//  outputs: none
//
SCODE IGS5000::AcceleratedSrcCopyBlt( GPEBltParms *pBltParms ) {

	DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("AcceleratedSrcCopyBlt\r\n")));

	int srcX = pBltParms->prclSrc->left;				// left of source rect
	int srcY = pBltParms->prclSrc->top;					// top of source rect
	int dstX = pBltParms->prclDst->left;				// left of dest rect
	int dstY = pBltParms->prclDst->top;					// top of dest rect
	int width = pBltParms->prclDst->right  - pBltParms->prclDst->left;
	int height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;
	ulong dstptr = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs();
	ulong srcptr = ((IGS5000Surf *)(pBltParms->pSrc))->CopOfs();
	unsigned short direction = Right_Down;

	if( !pBltParms->xPositive ) {
		srcX += ( width - 1 );
		dstX += ( width - 1 );
		if(pBltParms->yPositive) {
			direction = Left_Down;
		} 
		else {
			direction = Left_Up;
		}
	}
	if( !pBltParms->yPositive ) {
		srcY += ( height - 1 );
		dstY += ( height - 1 );
		if(pBltParms->xPositive) {
			direction = Right_Up;
		} 
		else {
			direction = Left_Up;
		}
	}

	dstptr += ((dstY * ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()) + dstX);
	srcptr += ((srcY * ((IGS5000Surf *)(pBltParms->pSrc))->SurfWidth()) + srcX);

	WaitForNotBusy();
	reg_Cop_control = 0;
	reg_Dst_Map_Width  = ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()-1;
	reg_Src_Map_Width  = ((IGS5000Surf *)(pBltParms->pSrc))->SurfWidth()-1;
	reg_Dst_Start_Ptr  = dstptr;
	reg_Src_Start_Ptr  = srcptr;
	reg_Op_Dimension_1 = width-1;
	reg_Op_Dimension_2 = height - 1;

	ULONG op  = COP_SCR2SCR | direction;
	if (pBltParms->bltFlags & BLT_TRANSPARENT) {
	reg_Fore_Color     = pBltParms->solidColor;
	reg_Back_Color     = pBltParms->solidColor;
	op |= COP_USE_TRANSSRC1;
	}
	reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
	reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/

	return S_OK;
}

void IGS5000::BeginHBlt() {
#ifdef ENABLE_ACCELERATION
	unsigned long i;

	for (i=0; i<0x100000; i++) {
	if( !(reg_Cop_control & 0x47)) {
		break;
	}
	}
	return;
#else
	return;
#endif
}

void IGS5000::EndHBlt() {
#ifdef ENABLE_ACCELERATION
	unsigned long i;

	for (i=0; i<0x100000; i++) {
	if (reg_Cop_control & 0x40) {
		break;
	}
	}
	for (i=0; i<0x100000; i++) {
	if( !(reg_Cop_control & 0x07)) {
		break;
	}
	}
	return;
#else
	return;
#endif
}

SCODE IGS5000::AcceleratedTextBlt(GPEBltParms *pBltParms) {

	int dstX = pBltParms->prclDst->left;
	int dstY = pBltParms->prclDst->top ;
	int srcX = pBltParms->prclMask->left;
	int srcY = pBltParms->prclMask->top;
	int srcwidth =pBltParms->prclMask->right-srcX;
	int srcheight=pBltParms->prclMask->bottom-srcY;
	int srcstride = pBltParms->pMask->Stride();
	unsigned char *src_ptr = (unsigned char*)(pBltParms->pMask->Buffer()) + srcY*srcstride+srcX/8;

	int     Xadj      = srcX%8;
	int     total_byte=srcstride*srcheight;
	int     h,w,i;
	int     xfer;
	unsigned long datxfer;
	unsigned long c;
	ulong dstptr;

	DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("AccelaratedClrXHBlt (%d,%d,%d)-(%d,%d,%d), xpos=%d, ypos=%d\r\n"),
		 pBltParms->pMask->Width(), pBltParms->pMask->Height(),
		 pBltParms->pMask->Stride(),dstX,dstY,pBltParms->pDst->Stride(),
		 pBltParms->xPositive,pBltParms->yPositive));


	/* special case begin: draw whole char, and size below 4K */
	if ( (Xadj==0) &&
		( ((srcwidth+7)>>3)==srcstride) &&
		( total_byte<=4*0x400) ) {

		unsigned long *pd = m_pHostMem;
		dstptr = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs() + ( ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()* dstY + dstX);
		
		WaitForNotBusy();
		m_inHostBlt = 1;
		reg_Dst_Map_Width  = ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()-1;
		reg_Dst_Start_Ptr  = dstptr;
		reg_Op_Dimension_1 = srcwidth-1;
		reg_Op_Dimension_2 = srcheight-1;
		reg_Mono_Size = total_byte-1;

		ULONG op  = COP_TRANSTEXTOUT;
		reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
		reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/

		BeginHBlt();
		for (i=0;i<total_byte/4;i++) {
			/* *pd = *(unsigned long*) src_ptr;*/
			*pd = MAKE_DWORD(src_ptr);
			pd++;
			src_ptr+=4;
		};
		/* last dword can exceed char's limit which causes GPF */		
		datxfer=0;
		switch (total_byte&3) {
		case 0:
			break;

		case 3:
			datxfer = MAKE_24BITS(src_ptr);
			*pd = datxfer;
			break;

		case 2:
			datxfer = MAKE_WORD(src_ptr);
			*pd = datxfer;
			break;

		case 1:
			datxfer = *src_ptr;
			*pd = datxfer;
			break;
		}
		EndHBlt();
		m_inHostBlt = 0;
		return S_OK;
	};
	/* special case end */

	/* normal case: 1st byte misalign */
	WaitForNotBusy();
	m_inHostBlt = 1;

	/* do 1st byte misalignment here: don't care about height, maximum line
	 * is 4096.
	 */
	if (Xadj) {
		int widthadj=min(8-Xadj,srcwidth);
		unsigned long *pd =  m_pHostMem;
		unsigned char *ps = src_ptr;

		dstptr = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs() + ( ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()* dstY + dstX);

		reg_Dst_Map_Width  = ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()-1;
		reg_Dst_Start_Ptr  = dstptr;
		reg_Op_Dimension_1 = widthadj-1;
		reg_Op_Dimension_2 = srcheight-1;
		reg_Mono_Size      = srcheight-1;

		ULONG op  = COP_TRANSTEXTOUT;
		reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
		reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/

		BeginHBlt();
		xfer=0;
		datxfer=0;
		for (DWORD yIndex=0; (int)yIndex<srcheight;yIndex++) {
			c = (unsigned char) *ps << Xadj;
			datxfer=datxfer+(c<<xfer);
			xfer+=8;
			if (xfer==32) {
				(unsigned long) *pd = (unsigned long) datxfer;
				pd++;
				xfer=0;
				datxfer=0;
			}
			ps+=srcstride;
		};
		/* last dword */
		if (xfer) {
			(unsigned long) *pd = (unsigned long) datxfer;
		}
		srcwidth-= widthadj;
		dstX    += widthadj;
		srcX    += widthadj;
		src_ptr ++;
		EndHBlt();
	};

	/* do the rest (if available), care about size since only 4K supported */
	if (srcwidth) {

		int  widthbytes = (srcwidth+7)>>3;
		unsigned long *pd;
		unsigned char *ps;

		dstptr = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs() + ( ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()* dstY + dstX);

		WaitForNotBusy();
		reg_Dst_Map_Width  = ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()-1;
		reg_Op_Dimension_1 = srcwidth-1;
		reg_Op_Dimension_2 = 0;
		reg_Mono_Size      = widthbytes-1;
		ULONG op  = COP_TRANSTEXTOUT;

		for (h=0;h<srcheight;h++) {
			xfer=0;
			datxfer=0;
			ps = src_ptr;
			src_ptr+=srcstride;
			pd = m_pHostMem;

			EndHBlt();
			reg_Dst_Start_Ptr  = dstptr;
			reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
			reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/
			BeginHBlt();
			for (w=0; w<widthbytes;w++) {
				c=(unsigned char)*ps++;
				datxfer=datxfer+(c<<xfer);
				xfer+=8;
				if (xfer==32) {
					xfer=0;
					(unsigned long) *pd = datxfer;
					datxfer=0;
					pd++;
				}
			}
			/* last dword */
			if (xfer) {
				(unsigned long) *pd = datxfer;
				pd++;
			}
			dstptr+=m_nScreenWidth;
		};
		EndHBlt();
	};
	EndHBlt();
	m_inHostBlt = 0;
	return S_OK;
}

SCODE IGS5000::AcceleratedMono2ScrBlt(GPEBltParms *pBltParms) {

	int dstX = pBltParms->prclDst->left;
	int dstY = pBltParms->prclDst->top;
	int srcX = pBltParms->prclSrc->left;
	int srcY = pBltParms->prclSrc->top;
	int srcwidth =pBltParms->prclSrc->right-srcX;
	int srcheight=pBltParms->prclSrc->bottom-srcY;
	int srcstride = pBltParms->pSrc->Stride();
	unsigned char *src_ptr = (unsigned char*)(pBltParms->pSrc->Buffer()) + srcY*srcstride+srcX/8;
	unsigned long dst_ptr  = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs() + ( ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()* dstY + dstX);	
	unsigned long *pd =  m_pHostMem;
	unsigned char *ps;
	int     Xadj      = srcX%8;
	unsigned long widthadj=min(8-Xadj,srcwidth);
	unsigned short a;
	unsigned long flags=0;
	unsigned long precode,midcode,postcode;
	unsigned long midcnt;
	ULONG    op;

	DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("AccelaratedMono2ScrBlt (%d,%d,%d)-(%d,%d,%d), xpos=%d, ypos=%d\r\n"),
		 pBltParms->pSrc->Width(), pBltParms->pSrc->Height(),
		 pBltParms->pSrc->Stride(),dstX,dstY,pBltParms->pDst->Stride(),
		 pBltParms->xPositive,pBltParms->yPositive));

	// build flags and param for 3 phases
	if (Xadj) {
		flags|=1;
		precode = (unsigned long)(0x0400+(widthadj-1))<<16;
		srcwidth-= widthadj;
	}
	if (srcwidth>=16) {
		flags   |= 2;
		midcode  = 0x040F0000;
		midcnt   = srcwidth>>4;
		srcwidth = srcwidth&0xF;
	}
	if (srcwidth) {
		flags   |= 4;
		postcode = (unsigned long)(0x0400+(srcwidth-1))<<16;
	}
	m_inHostBlt = 1;
	WaitForNotBusy();
	reg_Dst_Map_Width	= ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()-1;
	reg_Cop_Flags		= reg_Cop_Flags & ~TURBO_MONO_EN;
	reg_Dst_Start_Ptr   = dst_ptr;

	if (pBltParms->bltFlags & BLT_TRANSPARENT) {
		op  = COP_OPAQUEMON2SCR;
	}
	else {
		op  = COP_TRANSMON2SCR;
	}
	reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
	reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/

	Xadj+=8;    //for high byte
	BeginHBlt();
	for (DWORD yIndex=0; (int)yIndex<srcheight;yIndex++) {
		ps = src_ptr;
		src_ptr+=srcstride;
		//phase 1: 1st byte is misalignment
		if (flags&1) {
			a     = (unsigned short)((unsigned char) *ps++) << Xadj;
			*pd++ = precode+a;
		};
		//phase 2: 16 bit data in middle
		if (flags&2) {
			for (unsigned long i=0;i<midcnt;i++) {
				a = (unsigned short)((unsigned char) *ps++) << 8;
				a+= (unsigned char)*ps++;
				*pd++ = midcode + a;
			}
		}
		//phase 3: last byte/word
		if (flags&4) {
			a = (unsigned short)((unsigned char) *ps++) << 8;
			if (srcwidth&8) {
				a+= (unsigned char)*ps++;
			}
			*pd++ = postcode + a;
		}
		//new line
		*pd++ = 0x08000000;
	};
	*pd = 0x0F000000;
	EndHBlt();
	WaitForNotBusy();
	reg_Cop_Flags = reg_Cop_Flags | TURBO_MONO_EN;
	m_inHostBlt = 0;
	return S_OK;
}


SCODE IGS5000::AcceleratedClr2ScrBlt(GPEBltParms *pBltParms) {
	int dstX = pBltParms->prclDst->left;
	int dstY = pBltParms->prclDst->top;
	int srcX = pBltParms->prclSrc->left;
	int srcY = pBltParms->prclSrc->top;
	int srcwidth =pBltParms->prclSrc->right- pBltParms->prclSrc->left;
	int srcheight=pBltParms->prclSrc->bottom-pBltParms->prclSrc->top;
	int     srcstride = pBltParms->pSrc->Stride();

#ifdef FB16BPP
	unsigned char *src_ptr = (unsigned char*)(pBltParms->pSrc->Buffer()) + srcY*srcstride+srcX*2;
	int     dwCnt=(srcwidth*2+3)>>2;
#else
	unsigned char *src_ptr = (unsigned char*)(pBltParms->pSrc->Buffer()) + srcY*srcstride+srcX;
	int     dwCnt=(srcwidth+3)>>2;
#endif
	unsigned char *ps=src_ptr;
	unsigned long *pd;
//	unsigned long a;
	int       h,i;
	
	DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("AccelaratedClr2ClrHBLT (%d,%d,%d)-(%d,%d,%d), xpos=%d, ypos=%d\r\n"),
		 pBltParms->pSrc->Width(), pBltParms->pSrc->Height(),
		 pBltParms->pSrc->Stride(),dstX,dstY,pBltParms->pDst->Stride(),
		 pBltParms->xPositive,pBltParms->yPositive));

	ulong dstptr = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs() + ( ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()* dstY + dstX);	

	m_inHostBlt = 1;
	WaitForNotBusy();
	reg_Dst_Map_Width	= ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()-1;
	reg_Src_Start_Ptr  = 0;
	reg_Dst_Start_Ptr  = dstptr;
	reg_Op_Dimension_1 = srcwidth-1;
	reg_Op_Dimension_2 = srcheight-1;
	ULONG op  = COP_CLR2SCR;
	if (pBltParms->bltFlags & BLT_TRANSPARENT) {
		reg_Fore_Color     = pBltParms->solidColor;
		reg_Back_Color     = pBltParms->solidColor;
		op |= COP_USE_TRANSSRC1;
	}

	reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
	reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/
#ifdef FB16BPP
	srcwidth = srcwidth*2;
#endif
	BeginHBlt();
	switch (srcwidth&3) {
	case 0:
		for (h=0; h<srcheight; h++) {
			ps = src_ptr;
			pd = m_pHostMem;
			src_ptr+=srcstride;
			for (i=1;i<=dwCnt;i++) {
				(ulong) *pd = MAKE_DWORD(ps);
				ps+=4;
				pd++;
			};
		};
		break;

	case 1:
		for (h=0; h < srcheight; h++) {
			ps = src_ptr;
			pd = m_pHostMem;
			src_ptr+=srcstride;
			for (i=1;i<dwCnt;i++) {
				(ulong) *pd = MAKE_DWORD(ps);
				ps+=4;
				pd++;
			};
			*pd++ = *ps;
		};
		break;

	case 2:
		for (h=0; h < srcheight; h++) {
			ps = src_ptr;
			pd = m_pHostMem;
			src_ptr+=srcstride;
			for (i=1;i<dwCnt;i++) {
				(ulong) *pd = MAKE_DWORD(ps);
				ps+=4;
				pd++;
			};
			*pd++ = MAKE_WORD(ps);
		};
		break;

	case 3:
		for (h=0; h < srcheight; h++) {
			ps = src_ptr;
			pd = m_pHostMem;
			src_ptr+=srcstride;
			for (i=1;i<dwCnt;i++) {
				(ulong) *pd = MAKE_DWORD(ps);
				ps+=4;
				pd++;
			};
			*pd++ = MAKE_24BITS(ps);
		};
		break;
	}
	EndHBlt();
	WaitForNotBusy();
	m_inHostBlt = 0;
	return S_OK;
}

SCODE IGS5000::AcceleratedColorPatBlt(GPEBltParms *pBltParms) {

	int ofsX = pBltParms->prclDst->left;
	int ofsY = pBltParms->prclDst->top;
	int dstX = ofsX;
	int dstY = ofsY;
	int dstwidth = pBltParms->prclDst->right- pBltParms->prclDst->left;
	int dstheight= pBltParms->prclDst->bottom-pBltParms->prclDst->top;
	ulong dst_ptr  = ((IGS5000Surf *)(pBltParms->pDst))->CopOfs() + ( ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth()* dstY + dstX);	

	ofsX = (ofsX + pBltParms->pptlBrush->x)&0x07;
	ofsY = (ofsY + pBltParms->pptlBrush->y)&0x07;

	WaitForNotBusy();
	reg_Dst_Map_Width	= ((IGS5000Surf *)(pBltParms->pDst))->SurfWidth() -1;
	reg_Src_Start_Ptr   = m_PatOfs + ofsY*8;
	reg_Dst_Start_Ptr   = dst_ptr;
	reg_Op_Dimension_1  = dstwidth-1;
	reg_Op_Dimension_2  = dstheight-1;
	reg_Dst_X_Phase		= 0;
	reg_Dst_Y_Phase		= ofsY;
	ULONG op			=  COP_COLORPAT;
	reg_Px_OpL			= (USHORT)(op & 0xFFFF);			/*this can be byte, 2 bytes or 4 bytes access,*/
	reg_Px_OpH			= (USHORT)((op >> 16) & 0xFFFF);	/*but, hi byte must be the last to access.*/
	return S_OK;
}

#endif // ENABLE_ACCELERATION



