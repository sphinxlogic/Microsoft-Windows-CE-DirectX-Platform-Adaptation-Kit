// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-2000  Microsoft Corporation
//
//  Module Name:
//
//	line.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#include "precomp.h"


//
//  Line -
//  inputs:
//  outputs:
//
SCODE IGS5000::Line(
    GPELineParms *pLineParms,
    EGPEPhase phase ) {

    DEBUGMSG(GPE_ZONE_LINE,(TEXT("IGS5000::Line\r\n")));

#ifdef ENABLE_ACCELERATION
    if( phase == gpeSingle || phase == gpePrepare ) {
		pLineParms->pLine = EmulatedLine;
		if ( pLineParms->pDst->InVideoMemory() && pLineParms->mix == 0x0d0d ) {
		    reg_Fore_Mix = 0x03;
			reg_Fore_Color = pLineParms->solidColor;
		    pLineParms->pLine = (SCODE (GPE::*)(struct GPELineParms *))AcceleratedSolidLine;
		}
		else {
			DEBUGMSG(GPE_ZONE_LINE,(TEXT("Other Line rop4, use Emulation(0x%04x)\r\n"), pLineParms->mix));
		}
    }
    else {
		DEBUGMSG(GPE_ZONE_LINE,(TEXT("  Unsupported phase = %d\r\n"), phase));
    }
#else
    pLineParms->pLine = EmulatedLine;
#endif
    return S_OK;
}

unsigned short gaiDirection[] = {
	Pos_DX | Pos_DY | Pos_DZ,
	Pos_DX | Pos_DY | Neg_DZ,
	Neg_DX | Pos_DY | Neg_DZ,
	Neg_DX | Pos_DY | Pos_DZ,
	Neg_DX | Neg_DY | Pos_DZ,
	Neg_DX | Neg_DY | Neg_DZ,
	Pos_DX | Neg_DY | Neg_DZ,
	Pos_DX | Neg_DY | Pos_DZ,
};


/*
 * Error Term = 2*Dy - Dx - FixUp
 * K1	      = 2*Dy
 * K2	      = 2*Dy - 2*Dx
 * LineParm->Dm = max(2*Dx,2*Dy), LineParm->Dn = min(2*Dx,2*Dy)
 */

SCODE IGS5000::AcceleratedSolidLine( GPELineParms *pLineParms ) {
	DEBUGMSG(GPE_ZONE_LINE,(TEXT("AcceleratedSolidLine(x=%d,y=%d,dM=%d,dN=%d,p=%d,dir=%d,g=%d,mix=0x%04x\r\n"),
		pLineParms->xStart,pLineParms->yStart,pLineParms->dM,pLineParms->dN,pLineParms->cPels,pLineParms->iDir,pLineParms->llGamma,(int)pLineParms->mix));

    int octant = gaiDirection[pLineParms->iDir];
    int x1=pLineParms->xStart;
    int y1=pLineParms->yStart;
    int dx=pLineParms->dM, dy=pLineParms->dN;
	ulong dstptr = ((IGS5000Surf *)(pLineParms->pDst))->CopOfs();

    if (pLineParms->dN==0) { /* special case: vertical/horz line */
		dstptr += (y1 * ((IGS5000Surf *)(pLineParms->pDst))->SurfWidth() + x1);
		WaitForNotBusy();
		reg_Dst_Map_Width  = ((IGS5000Surf *)(pLineParms->pDst))->SurfWidth()-1;
		reg_Fore_Mix = 0x03;
		reg_Dst_Start_Ptr  = dstptr;
		if (octant&Neg_DZ) {
		    /* vert line */
			reg_Op_Dimension_1 =  0;
		    reg_Op_Dimension_2 =  pLineParms->cPels-1;
		}
		else { /* horz line */
		    reg_Op_Dimension_1 =  pLineParms->cPels-1;
			reg_Op_Dimension_2 =  0;
		}
		ULONG op=Fore_Src_color | PxBlt | Pattern_Foreground | (octant&~Neg_DZ);
		reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
		reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/
		return S_OK;
	};


    int errTerm = (int)pLineParms->dN + (int)pLineParms->llGamma - ( ( octant & Neg_DX ) ? 1 : 0 );

    if (( pLineParms->dN > 8191  ) ||
		( (pLineParms->dM-pLineParms->dN) > 8191  ) || 	   // remember dM >= dN
		(errTerm > 8191  ) ||
		(errTerm < -8192 ) ) {

		DEBUGMSG(GPE_ZONE_LINE,(TEXT("  ErrTerm Too large!\r\n")));
		return EmulatedLine(pLineParms);      // The hardware DDA would overflow so use the emulation
    }

	dstptr += (y1 * ((IGS5000Surf *)(pLineParms->pDst))->SurfWidth() + x1);	
    WaitForNotBusy();
	reg_Dst_Map_Width  = ((IGS5000Surf *)(pLineParms->pDst))->SurfWidth()-1;
    reg_Bres_Err_Term = errTerm;
    reg_Bres_K1       = (unsigned short)(pLineParms->dN);
    reg_Bres_K2       = (unsigned short)(pLineParms->dN-pLineParms->dM);
    reg_Op_Dimension_1= (pLineParms->cPels-1);
    reg_Dst_Start_Ptr = dstptr;
    ULONG op = COP_DRAWLINE | octant;
    reg_Px_OpL = (USHORT)(op & 0xFFFF);   /*this can be byte, 2 bytes or 4 bytes access,*/
    reg_Px_OpH = (USHORT)((op >> 16) & 0xFFFF);   /*but, hi byte must be the last to access.*/
    return S_OK;
}
