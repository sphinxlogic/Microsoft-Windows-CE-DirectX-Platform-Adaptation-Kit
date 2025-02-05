/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995, 1996, 1997, 1998  Microsoft Corporation

Module Name:    blt.cpp

Abstract:       accelerated bitblt/rectangle for S3 ViRGE
  
Functions:


Notes: 


--*/

#include "precomp.h"
#include "dispperf.h"

static ULONG color;

SCODE 
S3Virge::BltPrepare(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("S3Virge::BltPrepare rop4: 0x%04x\r\n"), pBltParms->rop4));
    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("Dest Format: %d, Dest Rect: t:%d,l:%d,b:%d,r:%d ")
         TEXT("Dest GPESurf 0x%08x\r\n"),
        (int) pBltParms->pDst->Format(), pBltParms->prclDst->top, 
        pBltParms->prclDst->left, pBltParms->prclDst->bottom,
        pBltParms->prclDst->right, pBltParms->pDst));

#if VIRGE_VERBOSE_MSGS
    if (pBltParms->prclSrc && pBltParms->pSrc)
    {
        DEBUGMSG(VIRGE_VERBOSE_MSGS,
            (TEXT("Src format: %d, Src Rect: %d,%d,%d,%d ")
             TEXT("Src GPESurf 0x%08x\r\n"), 
             (int) pBltParms->pSrc->Format(), pBltParms->prclSrc->top, 
             pBltParms->prclSrc->left, pBltParms->prclSrc->bottom, 
             pBltParms->prclSrc->right, pBltParms->pSrc));
    }
#endif

    DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("Blt preparation phase\r\n")));
    DEBUGWAIT(GPE_ZONE_BLT_LO)

    DispPerfStart(pBltParms->rop4);        
            
    // make sure that S3D engine is idle
    while (S3D_IS_BUSY);

    pBltParms->pBlt = EmulatedBlt; // catch all

#ifdef ENABLE_ACCELERATION
#define FUNCNAME(basename) \
    (SCODE (GPE::*)(struct GPEBltParms *))Emulator::Emulated##basename

    // Can only accelerate in S3 if destination is video memory
    if (pBltParms->pDst->InVideoMemory()) 
    {
        switch (pBltParms->rop4)
        {
        case 0xCCCC:    // SRCCOPY
        case 0x6666:    // SRCINVERT
        case 0x8888:    // SRCAND
        case 0xEEEE:    // SRCPAINT
            // can't accelerate color translations
            if (pBltParms->pLookup || pBltParms->pConvert)
                break;

            if (pBltParms->bltFlags & BLT_STRETCH)
            {
                // Stretch Blt
                DEBUGMSG(VIRGE_VERBOSE_MSGS,(TEXT("Stretch Blt!\r\n")));
                break;
            }
            if (pBltParms->pSrc->Format() != pBltParms->pDst->Format())
            {
                // different formats, no color space conversion supported
                DEBUGMSG(VIRGE_VERBOSE_MSGS,(TEXT("Different Formats!\r\n")));
                break;
            }
            if (pBltParms->pSrc->InVideoMemory())
            {
                pBltParms->pBlt = (SCODE (GPE::*) 
                    (struct GPEBltParms *)) AcceleratedSrcCopyBlt;
            }
            else
            {
                // if the stride of source image is not DWORD aligned
                if (pBltParms->pSrc->Stride() % 4 != 0)
                {
                    break;
                }

                // if the start of source image is not DWORD aligned
                PBYTE pByte = (PBYTE)(pBltParms->pSrc->Buffer());
                if ((long)pByte % 4 != 0)
                {
                    break;
                }

                // if the start of each line is not DWORD aligned
                int Bpp = EGPEFormatToBpp[pBltParms->pSrc->Format()];
                int srcX = pBltParms->prclSrc->left;
                if ((srcX * Bpp) % 32 != 0)
                {
                    break;
                }

                pBltParms->pBlt = (SCODE (GPE::*) 
                     (struct GPEBltParms *)) AcceleratedImageSrcCopyBlt;
            }
            break;

        case 0x0000:    // BLACKNESS
            DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                (TEXT("S3Virge::Blt - BLACKNESS\r\n")));

            pBltParms->solidColor = 0x0;

            pBltParms->pBlt = 
                (SCODE (GPE::*) (struct GPEBltParms *)) AcceleratedFillRect;
            break;

        case 0xFFFF:    // WHITENESS
            DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                (TEXT("S3Virge::Blt - WHITENESS\r\n")));

            pBltParms->solidColor = 0xffffff;

            pBltParms->pBlt = 
                (SCODE (GPE::*) (struct GPEBltParms *)) AcceleratedFillRect;
            break;

        case 0xF0F0:    // PATCOPY
        case 0x5A5A:    // PATINVERT
            // Emulate Color Convertion
            if (pBltParms->pLookup || pBltParms->pConvert)  
            {
                DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                    (TEXT("S3Virge::Blt - Emulate Color Conversion\r\n")));
                break;
            }
            if (pBltParms->solidColor == -1)
            {
                DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                    (TEXT("S3Virge::Blt - PATCOPY - patterned brush\r\n")));
                DEBUGWAIT(GPE_ZONE_BLT_LO)
                pBltParms->pBlt = 
                  (SCODE (GPE::*) (struct GPEBltParms *)) AcceleratedPatCopyBlt;
            }
            else
            {
                DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                    (TEXT("S3Virge::Blt - PATCOPY - solid brush\r\n")));
                DEBUGWAIT(GPE_ZONE_BLT_LO)
                pBltParms->pBlt = 
                    (SCODE (GPE::*) (struct GPEBltParms *)) AcceleratedFillRect;
            }
            break;

        case 0xAAF0:    // IMAGEPATCOPY
            DEBUGMSG(VIRGE_VERBOSE_MSGS, 
              (TEXT("S3Virge::Blt - IMAGEPATCOPY - image patterned mask\r\n")));
            DEBUGWAIT(GPE_ZONE_BLT_LO)
			if (pBltParms->pMask->Format() == gpe1Bpp)
            {
                pBltParms->pBlt = 
                 (SCODE (GPE::*) (struct GPEBltParms *)) AcceleratedImagePatCopyBlt;
            }
            break;

        case 0xAAAA:    // DSTCOPY
        case 0x5555:    // DSTINVERT
            DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                (TEXT("S3Virge::Blt - DSTINVERT\r\n")));
            DEBUGWAIT(GPE_ZONE_BLT_LO)
            pBltParms->pBlt = 
                (SCODE (GPE::*) (struct GPEBltParms *)) AcceleratedDstCopyBlt;
            break;

        default:        // some random ROP4
            DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                (TEXT("Unsupported Rop4 = 0x%4x\r\n"), pBltParms->rop4));
        }
    }

    // Performance Logging Type
    if (pBltParms->pBlt != EmulatedBlt) 
    {
        DispPerfType(DISPPERF_ACCEL_HARDWARE);
        return S_OK;
    }
#endif

    // If there wasn't any accelerated function available, let's see if there
    // is an emulated function that will be a little faster.

    if (pBltParms->pBlt == EmulatedBlt) 
    {
		if (pBltParms->rop4 == 0xAAF0)    // Special PATCOPY rop for text output -- fill where mask is set.
        {		// Not a pattern brush?
        		if( (pBltParms->solidColor != -1) &&
        			(pBltParms->pDst->Format() == gpe16Bpp) )
        		{
        			WaitForNotBusy();
        			if (pBltParms->pMask->Format() == gpe1Bpp)
        			{
						pBltParms->pBlt = FUNCNAME(BltText16);
        			}
        			else   // Anti-aliased text
        			{
        				pBltParms->pBlt = (SCODE (GPE::*)(struct GPEBltParms *))EmulatedBltAlphaText16_555;
        			}
        		}
		}
        //
        // Bail on any parameter values that would make this
        // blt non-emulatable
        //
        if (!(EGPEFormatToBpp[pBltParms->pDst->Format()] != 8        ||
             (pBltParms->bltFlags & (BLT_TRANSPARENT | BLT_STRETCH)) ||
             pBltParms->pLookup                                      ||  
             pBltParms->pConvert))
        {
            switch (pBltParms->rop4)
            {
                case 0x0000:    // BLACKNESS
                    DEBUGMSG(GPE_ZONE_BLT_LO,
                        (TEXT("S3Virge::Blt - BLACKNESS\r\n")));
                    pBltParms->solidColor = 0;
                    pBltParms->pBlt = FUNCNAME(BltFill08);
                    break;

                case 0xFFFF:    // WHITENESS
                    DEBUGMSG(GPE_ZONE_BLT_LO,
                        (TEXT("S3Virge::Blt - WHITENESS\r\n")));
                    pBltParms->solidColor = 0x00ffffff;
                    pBltParms->pBlt = FUNCNAME(BltFill08);
                    break;

                case 0xF0F0:    // PATCOPY
                    if( pBltParms->solidColor != -1 )
                    {
                        DEBUGMSG(GPE_ZONE_BLT_LO,
                            (TEXT("S3Virge::Blt - PATCOPY - solid brush\r\n")));
                        pBltParms->pBlt = FUNCNAME(BltFill08);
                    }
                    break;
                case 0xCCCC:    // SRCCOPY
                    if (pBltParms->pSrc->Format() == gpe8Bpp)
                    {
                        WaitForNotBusy();
                        pBltParms->pBlt = FUNCNAME(BltSrcCopy0808);
                    }
                    break;
                    
#if 0
                case 0xAAF0:    // Special PATCOPY rop for text output -- 
                                // fill where mask is set.
                    // Not a pattern brush?
                    if( pBltParms->solidColor != -1 )
                    {
                        WaitForNotBusy();
                        pBltParms->pBlt = FUNCNAME(BltText08);
                    }
					break;
#endif
                case 0x5A5A :   // PATINVERT
                    if( pBltParms->solidColor != -1 )
                    {
                        DEBUGMSG(GPE_ZONE_BLT_LO,
                          (TEXT("S3Virge::Blt - PATINVERT - solid brush\r\n")));
                        pBltParms->pBlt = FUNCNAME(BltPatInvert08);
                    }
                    break;
                case 0x5555:    // DSTINVERT
                    DEBUGMSG(GPE_ZONE_BLT_LO,
                        (TEXT("S3Virge::Blt - DSTINVERT\r\n")));
                    pBltParms->pBlt = FUNCNAME(BltDstInvert08);
                    break;
                case 0x8888:    // SRCAND
                    DEBUGMSG(GPE_ZONE_BLT_LO,
                        (TEXT("S3Virge::Blt - SRCAND\r\n")));
                    if (pBltParms->pSrc->Format() == gpe8Bpp)
                    {
                        pBltParms->pBlt = FUNCNAME(BltSrcAnd0808);
                    }
                    break;
                case 0xEEEE:    // SRCPAINT
                    DEBUGMSG(GPE_ZONE_BLT_LO,
                        (TEXT("S3Virge::Blt - SRCPAINT\r\n")));
                    if (pBltParms->pSrc->Format() == gpe8Bpp)
                    {
                        pBltParms->pBlt = FUNCNAME(BltSrcPaint0808);
                    }
                    break;
                case 0x6666:    // SRCINVERT
                    DEBUGMSG(GPE_ZONE_BLT_LO,
                        (TEXT("S3Virge::Blt - SRCINVERT\r\n")));
                    if (pBltParms->pSrc->Format() == gpe8Bpp)
                    {
                        pBltParms->pBlt = FUNCNAME(BltSrcInvert0808);
                    }
                    break;

                default:        // some random ROP4
                    ;
            }
        }

        if (pBltParms->pBlt != EmulatedBlt) 
        {
            DispPerfType(DISPPERF_ACCEL_EMUL);
        }

    }
    return S_OK;
}


// This function would be used to undo the setting of clip registers etc
SCODE 
S3Virge::BltComplete(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("BltComplete\r\n")));

    DispPerfEnd(0);
    
    return S_OK;
}


/***************************************************
 *
 *  Note: VIRGE_ROP_BUG
 *      The Rectangle Fill with a ROP of "P" does
 *  NOT fill the whole rectangle. A portion of the
 *  rectangle contains the last color pattern drawn.
 *  In order to get around this, The Mono Pattern
 *  Foreground Color register is set to the inverse
 *  of the passed color pattern, and the ROP code
 *  of Inverted Pattern "Pn" is used.
 *
 ***************************************************/

SCODE 
S3Virge::AcceleratedFillRect(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("AcceleratedFillRect\r\n")));

    int dstX = pBltParms->prclDst->left;    // left of dest rect
    int dstY = pBltParms->prclDst->top;     // top of dest rect

    int width = pBltParms->prclDst->right - pBltParms->prclDst->left;
    int height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;

    int dstBase = ((S3VirgeSurf *) (pBltParms->pDst))->OffsetInVideoMemory();
    int dstStride = ((S3VirgeSurf *) (pBltParms->pDst))->Stride();

    ULONG cmd;                              // command set register
    UINT direction = PLUS_X | PLUS_Y;       // direction flag: X positive 
                                            // (left to right), Y positive
    ULONG color;                            // fill color
    int clipX, clipY;                       // clipping

#ifdef VIRGE_ROP_BUG
    BYTE rop;
#endif

    clipX = dstX;
    clipY = dstY;

    if (!pBltParms->xPositive)
    {
        dstX += (width - 1);
        direction &= ~PLUS_X;
    }

    if (!pBltParms->yPositive)
    {
        dstY += (height - 1);
        direction &= ~PLUS_Y;
    }

#if VIRGE_VERBOSE_MSGS
    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
        (TEXT("X Dest = %d Y Dest = %d Direction = 0x%x color=%d\n\r"),
        dstX, dstY, direction, pBltParms->solidColor));
#endif

    color = pBltParms->solidColor;

    if (pBltParms->pLookup)
    {
        color = (pBltParms->pLookup)[color];
    }
    if (pBltParms->pConvert)
    {
        color = (pBltParms->pColorConverter->*(pBltParms->pConvert))(color);
    }

#ifdef VIRGE_ROP_BUG
    color = ~color;
#endif

    // Virge Rectangle Fill Bug, Use BitBlt with Mono Pattern of all 1's 
    // instead of Rectangle Fill
    cmd = CMD_BITBLT | direction | MONO_PATTERN | 
          DRAW_ENABLE | CLIPPING_ENABLED | m_nDestFormat;

    WaitForFIFO(2);
    reg_MMA4DC = (clipX << 16) |                    // left and
                 (clipX + width - 1);               // right clipping
    reg_MMA4E0 = (clipY << 16) |                    // top and
                 (clipY + height - 1);              // bottom clipping

    // Virge Rectangle Fill Bug, Use BitBlt with Mono Pattern of all 1's 
    // instead of Rectangle Fill
    WaitForFIFO(3);
    reg_MMA4E8 = 0xFFFFFFFF;                        // mono pattern 0
    reg_MMA4EC = 0xFFFFFFFF;                        // mono pattern 1
    reg_MMA4F4 = color;                             // mono pattern fg color 
                            
    WaitForFIFO(4);
    reg_MMA504 = ((width - 1) << 16) | height;      // width and height
    reg_MMA4E4 = (dstStride << 16) + dstStride;     // stride
    reg_MMA4D8 = dstBase;                           // dest base address 
    reg_MMA50C = ((dstX)<<16) | (dstY & 0x00007FF); // dest XY

#ifdef VIRGE_ROP_BUG
    if (pBltParms->rop4 == 0x5A5A)
    {
        rop = 0xA5;
    }
    else if (pBltParms->rop4 == 0xF0F0)
    {
        rop = 0x0F;
    }
    else
    {
        // ROP Bug, for BLACKNESS and WHITENESS use Pattern ROP with Color
        rop = 0x0F;
    }
                            
    cmd |= (rop << ROPS_SHIFT);

    WaitForFIFO(1);
    reg_MMA500 = cmd;
#else
    cmd |= ((pBltParms->rop4 & 0xFF) << ROPS_SHIFT);

    WaitForFIFO(1);
    reg_MMA500 = cmd;
#endif

    return S_OK;
}


SCODE 
S3Virge::AcceleratedSrcCopyBlt(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("AcceleratedSrcCopyBlt Rop = 0x%4x\r\n"), pBltParms->rop4));

    int srcX = pBltParms->prclSrc->left;    // left of source rect
    int srcY = pBltParms->prclSrc->top;     // top of source rect
    int dstX = pBltParms->prclDst->left;    // left of dest rect
    int dstY = pBltParms->prclDst->top;     // top of dest rect

    int width = pBltParms->prclDst->right - pBltParms->prclDst->left;
    int height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;

    int srcBase = ((S3VirgeSurf *) (pBltParms->pSrc))->OffsetInVideoMemory();
    int dstBase = ((S3VirgeSurf *) (pBltParms->pDst))->OffsetInVideoMemory();
    int srcStride = ((S3VirgeSurf *) (pBltParms->pSrc))->Stride();
    int dstStride = ((S3VirgeSurf *) (pBltParms->pDst))->Stride();

    ULONG direction = PLUS_X | PLUS_Y;      // direction flag: X positive 
                                            // (left to right), Y positive
    ULONG cmd;                              // command set register
    ULONG color;                            // transparent color
    int clipX, clipY;                       // clipping data store

/*******************************************************************
 *
 *  See "ViRGE Integrated Accelerator Manual"
 *  The "BitBLT" Section on Programming Parameters for Overlapping
 *  BitBLTs. There is a table that describes the Direction, SrcX,
 *  SrcY, DestX, and DestY parameter, depending on the overlap
 *  of the rectangles.
 *
 *******************************************************************/
    clipX = dstX;
    clipY = dstY;

    if (!pBltParms->xPositive)
    {
        srcX += (width - 1);
        dstX += (width - 1);
        direction &= ~PLUS_X;
    }

    if (!pBltParms->yPositive)
    {
        srcY += (height - 1);
        dstY += (height - 1);
        direction &= ~PLUS_Y;
    }

    cmd = CMD_BITBLT | direction | ((pBltParms->rop4 & 0xFF) << ROPS_SHIFT) | 
          DRAW_ENABLE | CLIPPING_ENABLED | m_nDestFormat;

    if (pBltParms->bltFlags & BLT_TRANSPARENT)
    {
        DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("Transparent SrcCopyBlt\r\n")));

        cmd |= TRANSPARENT_ENABLE;

        color = pBltParms->solidColor;
        if (pBltParms->pLookup)
        {
            color = (pBltParms->pLookup)[color];
        }
        if (pBltParms->pConvert)
        {
            color = (pBltParms->pColorConverter->*(pBltParms->pConvert))(color);
        }

        WaitForFIFO(1);
        reg_MMA4FC = color;                         // source foreground color
    }

    WaitForFIFO(4);
    reg_MMA4DC = (clipX << 16) |                    // left and
                 (clipX + width - 1);               // right clipping
    reg_MMA4E0 = (clipY << 16) |                    // top and 
                 (clipY + height - 1);              // bottom clipping
    reg_MMA504 = ((width - 1) << 16) | height;      // width and height
    reg_MMA4E4 = (dstStride << 16) + srcStride;     // stride

    WaitForFIFO(4);
    reg_MMA4D4 = srcBase;                           // source base address
    reg_MMA508 = ((srcX)<<16) | (srcY & 0x00007FF); // source XY
    reg_MMA4D8 = dstBase;                           // dest base address
    reg_MMA50C = ((dstX)<<16) | (dstY & 0x00007FF); // dest XY

    WaitForFIFO(1);
    reg_MMA500 = cmd;                               // command set register

    return S_OK;
}

SCODE 
S3Virge::AcceleratedImageSrcCopyBlt(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("AcceleratedImageSrcCopyBlt\r\n")));

    int srcX = pBltParms->prclSrc->left;    // left of source rect
    int srcY = pBltParms->prclSrc->top;     // top of source rect
    int dstX = pBltParms->prclDst->left;    // left of dest rect
    int dstY = pBltParms->prclDst->top;     // top of dest rect

    int width = pBltParms->prclDst->right - pBltParms->prclDst->left;
    int height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;

    int dstBase = ((S3VirgeSurf *) (pBltParms->pDst))->OffsetInVideoMemory();
    int srcStride = ((S3VirgeSurf *) (pBltParms->pSrc))->Stride();
    int dstStride = ((S3VirgeSurf *) (pBltParms->pDst))->Stride();

    ULONG direction = PLUS_X | PLUS_Y;      // direction flag: X positive 
                                            // (left to right), Y positive
    PULONG srcBuffer;                       // ptr to source image 
    PULONG srcPtr;                          // buffer pointer
    int line, dword;                        // index counter
    ULONG color;                            // transparent color
    int numDWords;                          // number of DWORDS for each line 
    ULONG cmd;                              // command set register
    int clipX, clipY;                       // clipping

    clipX = dstX;
    clipY = dstY;
    
    if (!pBltParms->xPositive)
    {
        srcX += (width - 1);
        dstX += (width - 1);
        direction &= ~PLUS_X;
    }

    if (!pBltParms->yPositive)
    {
        srcY += (height - 1);
        dstY += (height - 1);
        direction &= ~PLUS_Y;
    }

    cmd = CMD_BITBLT | direction | ((pBltParms->rop4 & 0xFF) << ROPS_SHIFT) | 
          SOURCE_FROM_SYSTEM | CLIPPING_ENABLED | DRAW_ENABLE | m_nDestFormat;  
    cmd |= DWORD_ALIGNED_TRANSFER;

    if (pBltParms->bltFlags & BLT_TRANSPARENT)
    {
        DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("Transparent SrcCopyBlt\r\n")));

        cmd |= TRANSPARENT_ENABLE;

        color = pBltParms->solidColor;
        if (pBltParms->pLookup)
        {
            color = (pBltParms->pLookup)[color];
        }
        if (pBltParms->pConvert)
        {
            color = (pBltParms->pColorConverter->*(pBltParms->pConvert))(color);
        }

        WaitForFIFO(1);
        reg_MMA4FC = color;                         // source foreground color
    }
        
    WaitForFIFO(2);
    reg_MMA4DC = (clipX << 16) | (clipX + width - 1);   // left and right clip
    reg_MMA4E0 = (clipY << 16) | (clipY + height - 1);  // top and bottom clip

    WaitForFIFO(4);
    reg_MMA504 = ((width - 1) << 16) | height;          // dest width and height
    reg_MMA4E4 = (dstStride << 16) + abs(srcStride);    // stride
    reg_MMA4D8 = dstBase;                               // dest base address
    reg_MMA50C = ((dstX) << 16) | ((dstY) & 0x00007FF);
                                                        // dest XY
    WaitForFIFO(1);
    reg_MMA500 = cmd;

    int Bpp = EGPEFormatToBpp[pBltParms->pSrc->Format()];
    numDWords = (width * Bpp + 31) / 32;

    // Adjust the Buffer Pointer to the first displayable data
    srcBuffer = (PULONG)((S3VirgeSurf *) (pBltParms->pSrc))->Buffer() + 
                (srcStride * srcY / 4) + (srcX * Bpp / 32);
    
#ifndef ENABLE_DMA
    srcPtr = (PULONG) (m_pMMIO + 0x0000);
    PULONG lineStart = srcBuffer;
    for (line = 0; line < height; line++)
    {
        for (dword = 0; dword < numDWords; dword++)
        {
            *srcPtr++ = *srcBuffer++;
            if ((((LONG)srcPtr) & 0x7FFF) == 0)
            {
                srcPtr = (PULONG) (m_pMMIO + 0x0000);
            }
        }
        srcBuffer = lineStart + (srcStride / 4);
        lineStart = srcBuffer;
    }
#else // ENABLE_DMA

#define DMA_SIZE_IN_DWORDS     (0x400 - 1)     // 4K - 4 bytes

    // Number fo DWords to transfer =
    //                        int(((width * height * bits/pixel) +31) / 32)
    numDWords = ((8 * abs(srcStride) * (height + srcY)) + 31) / 32;

    // Number of DWords to transfer is adjusted by the Data skipped
    // at the top of the Buffer
    numDWords -= ((abs(srcStride) * srcY) / 4);

    DWORD totalDMAs = (numDWords + DMA_SIZE_IN_DWORDS - 1) / DMA_SIZE_IN_DWORDS;
    DWORD indexDMA, indexDword;

    srcPtr = (PULONG) (m_pDMABuf + (reg_MM8594 & 0xFFFF));
    for (indexDMA = 0; indexDMA < totalDMAs; indexDMA++)
    {
        DWORD dwNumOfDwords;        // num of dwords for the current dma
        if (indexDMA == (totalDMAs - 1))
        {
            dwNumOfDwords = numDWords % DMA_SIZE_IN_DWORDS;
        }
        else
        {
            dwNumOfDwords = DMA_SIZE_IN_DWORDS;
        }

        // check to see if the available space in the DMA buffer
        // is large enough for the data transfer, if not wait
        do {
            DWORD dwCurrRead = reg_MM8598 / 4;
            DWORD dwRead;
            if (dwCurrRead < m_dwDMAPrevRead)
            {
                dwRead = dwCurrRead + 0x4000 - m_dwDMAPrevRead;
            }
            else
            {
                dwRead = dwCurrRead - m_dwDMAPrevRead;
            }
            m_dwDMABufAvail += dwRead;
            m_dwDMAPrevRead = dwCurrRead;
        } while (m_dwDMABufAvail < dwNumOfDwords + 1);
        
        // write the header
        *srcPtr++ = (dwNumOfDwords & 0xFFFF) | DMA_IMAGE_DATA;

        // check to see if srcPtr is on the 64K boundary
        // if yes, reset the pointer
        if (((srcPtr - (PULONG) m_pDMABuf) % 0x4000) == 0)
        {
            srcPtr = (PULONG) m_pDMABuf;
        }
        
        // write the data to the buffer
        for (indexDword = 0; indexDword < dwNumOfDwords; indexDword++)
        {
            int index = indexDMA * DMA_SIZE_IN_DWORDS + indexDword;

            if (srcStride < 0)
            {
                // Width less than or equal to 1 word
                if (srcStride > -4 && index != 0)
                {
                    srcBuffer -= 2;
                }
                else
                {
                    // Adjust the Buffer Address Pointer by 2x Source Width
                    // each Line except Line 0
                    if (index % (srcStride / 4) == 0 && index != 0)
                    {
                        srcBuffer += (2 * (srcStride / 4));
                    }
                }
            }
            *srcPtr++ = *srcBuffer++;

            // check to see if srcPtr is on the 64K boundary
            // if yes, reset the pointer
            if (((srcPtr - (PULONG) m_pDMABuf) % 0x4000) == 0)
            {
                srcPtr = (PULONG) m_pDMABuf;
            }
        }

        // start DMA
        reg_MM8594 = (4 * (srcPtr - (PULONG) m_pDMABuf)) | DMA_WRITE_PTR_UPD;

        // update the status
        m_dwDMABufAvail -= (dwNumOfDwords + 1);
    }

#endif // ENABLE_DMA
    return S_OK;
}
SCODE 
S3Virge::AcceleratedPatCopyBlt(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("AcceleratedPatCopyBlt\r\n")));

    int dstX = pBltParms->prclDst->left;    // left of dest rect
    int dstY = pBltParms->prclDst->top;     // top of dest rect

    int width = pBltParms->prclDst->right - pBltParms->prclDst->left;
    int height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;

    int patternWidth = ((S3VirgeSurf *) (pBltParms->pBrush))->Width();
    int patternHeight = ((S3VirgeSurf *) (pBltParms->pBrush))->Height();

    int dstBase = ((S3VirgeSurf *) (pBltParms->pDst))->OffsetInVideoMemory();
    int dstStride = ((S3VirgeSurf *) (pBltParms->pDst))->Stride();

    ULONG direction = PLUS_X | PLUS_Y;      // direction flag: X positive 
                                            // (left to right), Y positive
    PULONG patternBuffer = 
        (PULONG)((S3VirgeSurf *) (pBltParms->pBrush))->Buffer();    
    PULONG patternPtr;                      // buffer pointer
    int patternStride = ((S3VirgeSurf *) (pBltParms->pBrush))->Stride();
    int patternFormat = ((S3VirgeSurf *) (pBltParms->pBrush))->Format();
    int index;                              // pattern index counter
    int numDWords;                          // number of DWORDS in 
                                            // pattern buffer
    int numPatCols = 1;                     // number of columns of patterns 
                                            // per destination rect
    int numPatRows = 1;                     // number of rows of patterns per 
                                            // destination rect
    int outputDstX, outputDstY;             // pattern output left and 
                                            // top dest
    int outputWidth, outputHeight;          // pattern output width and height 
                                            // dest
    int dataWidth;                          // pattern data width
    int offsetDstX, offsetDstY;             // pattern ofset left and top dest 
    int clipDstX;                           // clipping left destination 
    int clipX, clipY;                       // clipping
    ULONG cmd;                              // command set register
    int rowIndex;                           // multiple pattern per rectangle 
                                            // row index counter
    int columnIndex;                        // multiple pattern per rectangle 
                                            // column index counter

    clipX = dstX;
    clipY = dstY;
    
    // Calculate the Offsets of the pattern in relation to the 
    // destination rectangle
    offsetDstX = ((pBltParms->prclDst->left + patternWidth - 
                   pBltParms->pptlBrush->x) % patternWidth);
    offsetDstY = ((pBltParms->prclDst->top + patternHeight - 
                   pBltParms->pptlBrush->y) % patternHeight);

    // Calculate number of Patterns per Destination Rectange.
    numPatCols = (width + offsetDstX + patternWidth - 1) / patternWidth;
    numPatRows = (height + offsetDstY + patternHeight - 1) / patternHeight;

    if (!pBltParms->xPositive)
    {
        dstX += (width - 1);
        direction &= ~PLUS_X;
    }

    if (!pBltParms->yPositive)
    {
        dstY += (height - 1);
        direction &= ~PLUS_Y;
    }

    cmd = CMD_BITBLT | direction | SOURCE_FROM_SYSTEM | CLIPPING_ENABLED | 
          DRAW_ENABLE | m_nDestFormat;

    // For Image Transfer Area, the pattern is loaded and becomes the Source
    // therefore ~P -> ~S, and P ^ D -> S ^ D
    if (pBltParms->rop4 == 0xF0F0)          // PATCOPY
    {
        cmd |= (0xCC << ROPS_SHIFT);        // Use Source
    } 
    else                                    // PATINVERT
    {
        cmd |= (0x66 << ROPS_SHIFT);        // Use Source XOR Dest
    }

    WaitForFIFO(1);
    // top and bottom clipping
    reg_MMA4E0 = (clipY << 16) | (clipY + height - 1);

    // Calculate data pattern width on DWORD boundry
    dataWidth = ((patternWidth * (1 << patternFormat)) + 31) / 32;
    // Calculate output pattern width on 64-bit boundry
    outputWidth = ((dataWidth + 1) >> 1) << 1;

    for (rowIndex = 0; rowIndex < numPatRows; rowIndex++)
    {
        // Calculate number of DWORDS in the Pattern
        numDWords = outputWidth * patternHeight;
        // Set Output Height to the Pattern Height
        outputHeight = patternHeight;
        // Calculate Output Y Destination
        outputDstY = dstY - offsetDstY + (rowIndex * patternHeight);

        if ( rowIndex == 0)              // ROW 0
        {
            // Set Output Destination Y to Destination Y
            outputDstY = dstY;
            // Subtract initial rows not displayed
            numDWords -= offsetDstY * outputWidth;
            // Adjust the Output Height for rows not displayed
            outputHeight -= offsetDstY;
        }

        // if last row of pattern copy, calculate number of DWORD to read 
        // from the Pattern Buffer
        if (rowIndex == (numPatRows - 1))
        {
            // Last line not equal to last line of pattern
            if (((height + offsetDstY) % patternHeight) != 0)
            {
                // Subtract trailing rows not displayed
                numDWords -= 
                    (patternHeight - ((height + offsetDstY) % patternHeight)) *
                     outputWidth;   
                // Adjust the Output Height for rows not displayed
                outputHeight = ((height + offsetDstY) % patternHeight);
            }
        }

        for (columnIndex = 0; columnIndex < numPatCols; columnIndex++)
        {
            // Pattern Buffer Data Store
            patternBuffer = 
                (PULONG)((S3VirgeSurf *) (pBltParms->pBrush))->Buffer();
            // Calculate Output X Destination 
            outputDstX = dstX - offsetDstX + (columnIndex * patternWidth);
            clipDstX = outputDstX;

            // COLUMN 0
            if (columnIndex == 0)
            {
                // Set clipping to Destination X
                clipDstX = clipX;
            }

            // ROW 0
            if (rowIndex == 0)
            {
                // Skip initial data not displayed in the Buffer
                for (index = 0; index < (dataWidth * offsetDstY); index++)
                {
                    if (patternStride > 0)
                    {
                        patternBuffer++;
                    }
                    else
                    {
                        patternBuffer--;
                    }
                }
            }

            WaitForFIFO(2);
            // rectangle width and height
            reg_MMA504 = 
                (((outputWidth * 32 / (1 << patternFormat)) - 1) << 16) | 
                outputHeight;

            if ((outputDstX + patternWidth) < (dstX + width))
            {
                // left and right clipping to pattern clip
                reg_MMA4DC = (clipDstX << 16) | (outputDstX + patternWidth - 1);
            }
            else
            {
                // left and right clipping by dest rect clip
                reg_MMA4DC = (clipDstX << 16) | (dstX + width - 1);
            }

            WaitForFIFO(3);
            reg_MMA4E4 = (dstStride << 16) + dstStride;     // stride
            reg_MMA4D8 = dstBase;                           // dest base addr
            reg_MMA50C = (outputDstX << 16) | (outputDstY & 0x00007FF);
                                                            // dest XY

            WaitForFIFO(1);
            reg_MMA500 = cmd;

            // Load the Mask Pattern into the  MM0000 - XXXX Pattern registers
            for (index = 0; index < numDWords; index++)
            {
                // Set the Pattern Pointer to the start address of 
                // the Image Transfer Area on 32K boundries
                if ((index % 0x2000) == 0)
                {
                    patternPtr = (PULONG) (m_pMMIO + 0x0000);
                }
                if ((dataWidth != outputWidth) && 
                    ((index % outputWidth) == dataWidth))
                {
                    *patternPtr++ = 0;
                }
                else
                {
                    if (patternStride < 0)
                    {
                        // Adjust the Buffer Address Pointer by 2x Pattern 
                        // Width each Line except Line 0
                        if (((index % outputWidth) == 0) && (index != 0))
                        {
                            patternBuffer += (2 * (patternStride / 4));
                        }
                    }
                    *patternPtr++ = *patternBuffer++;
                }
            }
        }
    }

    return S_OK;
}

SCODE 
S3Virge::AcceleratedImagePatCopyBlt(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("AcceleratedImagePatCopyBlt\r\n")));

    int dstX = pBltParms->prclDst->left;    // left of dest rect
    int dstY = pBltParms->prclDst->top;     // top of dest rect

    int width = pBltParms->prclDst->right - pBltParms->prclDst->left;
    int height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;

    int dstBase = ((S3VirgeSurf *) (pBltParms->pDst))->OffsetInVideoMemory();
    int dstStride = ((S3VirgeSurf *) (pBltParms->pDst))->Stride();

    ULONG direction = PLUS_X | PLUS_Y;      // direction flag: X positive 
                                            // (left to right), Y positive
    PULONG patternBuffer = 
               (PULONG)((S3VirgeSurf *) (pBltParms->pMask))->Buffer();      

    PULONG patternPtr;                      // buffer pointer
    int patternStride = ((S3VirgeSurf *) (pBltParms->pMask))->Stride();
    int patternFormat = ((S3VirgeSurf *) (pBltParms->pMask))->Format();
    int patternWidth;                       // pattern width
    int patternLeft = pBltParms->prclMask->left;
    int patternTop = pBltParms->prclMask->top;
    int patternBottom = pBltParms->prclMask->bottom;
    int index;                              // pattern index counter
    ULONG color;                            // transparent color
    int numDWords;                          // number of DWORDS in 
                                            // pattern buffer
    ULONG cmd;                              // command set register
    int clipX, clipY;                       // clipping

    clipX = dstX;
    clipY = dstY;
    
    // Actual pattern width is 
    // Pattern Stride (Number of Bytes per Line) * 8 (Bits per Byte)
    patternWidth = abs(patternStride) * 8;

    if (!pBltParms->xPositive)
    {
        dstX += (width - 1);
        direction &= ~PLUS_X;
    }

    if (!pBltParms->yPositive)
    {
        dstY += (height - 1);
        direction &= ~PLUS_Y;
    }

    cmd = CMD_BITBLT | direction | (0xCC << ROPS_SHIFT) | 
          TRANSPARENT_ENABLE | MONO_SOURCE | SOURCE_FROM_SYSTEM | 
          CLIPPING_ENABLED | DRAW_ENABLE | m_nDestFormat;   
        
    color = (pBltParms->solidColor);

    if (pBltParms->pLookup)
    {
        color = (pBltParms->pLookup)[color];
    }
    if (pBltParms->pConvert)
    {
        color = (pBltParms->pColorConverter->*(pBltParms->pConvert))(color);
    }

    WaitForFIFO(4);
    reg_MMA4DC = (clipX << 16) | (clipX + width - 1);   // left and right clip
    reg_MMA4E0 = (clipY << 16) | (clipY + height - 1);  // top and bottom clip

    // No top and bottom clipping, Virge will lockup if reading more than 
    // exact number of DWords Command must be set to the correct First DWord 
    // Offset, and the corresponding Buffer data must be skipped:
    // Set command to read first dword offset
    cmd |= (((patternWidth / 8) * patternTop) % 4) << 12;

    // Adjust the Buffer Pointer to the first displayable data
    patternBuffer += (patternStride * patternTop / 4);
    
    reg_MMA4F8 = 0;                         // Source Color Background
    reg_MMA4FC = color;                     // Source Color Foreground.

    WaitForFIFO(4);
    reg_MMA504 = ((patternWidth - 1) << 16) | height;   // dest width and height
    reg_MMA4E4 = (dstStride << 16) + dstStride;         // stride
    reg_MMA4D8 = dstBase;                               // dest base address
    reg_MMA50C = ((dstX - patternLeft)<<16) | ((dstY) & 0x00007FF);
                                                        // dest XY
    WaitForFIFO(1);
    reg_MMA500 = cmd;

    // Number fo DWords to transfer = 
    //                        int(((width * height * bits/pixel) +31) / 32)
    numDWords = ((patternWidth * (height + patternTop) * 
                  (1 << patternFormat)) + 31) / 32;

    // Number of DWords to transfer is adjusted by the Data skipped 
    // at the top of the Buffer
    numDWords -= ((abs(patternStride) * patternTop) / 4);

    // Load the Mask Pattern into the  MM0000 - XXXX Pattern registers
    for (index = 0; index < numDWords; index++)
    {
        // Set the Pattern Pointer to the start address of 
        // the Image Transfer Area on 32K boundries
        if ((index % 0x2000) == 0)
        {
            patternPtr = (PULONG) (m_pMMIO + 0x0000);
        }

        if (patternStride < 0)
        {
            // Width less than or equal to 1 word
            if (patternStride > -4 && index != 0)
            {
                patternBuffer -= 2;
            }
            else
            {
                // Adjust the Buffer Address Pointer by 2x Pattern Width 
                // each Line except Line 0
                if (index % (patternStride / 4) == 0 && index != 0)
                {
                    patternBuffer += (2 * (patternStride / 4));
                }
            }
        }
        *patternPtr++ = *patternBuffer++;

    }
    return S_OK;
}

SCODE 
S3Virge::AcceleratedDstCopyBlt(GPEBltParms *pBltParms)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("AcceleratedDstCopyBlt Rop = 0x%4x\r\n"), pBltParms->rop4));

    int dstX = pBltParms->prclDst->left;    // left of dest rect
    int dstY = pBltParms->prclDst->top;     // top of dest rect

    int width = pBltParms->prclDst->right  - pBltParms->prclDst->left;
    int height = pBltParms->prclDst->bottom - pBltParms->prclDst->top;

    int dstBase = ((S3VirgeSurf *) (pBltParms->pDst))->OffsetInVideoMemory();
    int dstStride = ((S3VirgeSurf *) (pBltParms->pDst))->Stride();

    ULONG direction = PLUS_X | PLUS_Y;      // direction flag: X positive 
                                            // (left to right), Y positive
    ULONG cmd;                              // command set register
    int clipX, clipY;                       // clipping

/*******************************************************************
 *
 *  See "ViRGE Integrated Accelerator Manual"
 *  The "BitBLT" Section on Programming Parameters for Overlapping
 *  BitBLTs. There is a table that describes the Direction, SrcX,
 *  SrcY, DestX, and DestY parameter, depending on the overlap
 *  of the rectangles.
 *
 *******************************************************************/

    clipX = dstX;
    clipY = dstY;

    if (!pBltParms->xPositive)
    {
        dstX += (width - 1);
        direction &= ~PLUS_X;
    }

    if (!pBltParms->yPositive)
    {
        dstY += (height - 1);
        direction &= ~PLUS_Y;
    }

    cmd = CMD_BITBLT | direction | ((pBltParms->rop4 & 0xFF) << ROPS_SHIFT) | 
          DRAW_ENABLE | CLIPPING_ENABLED | m_nDestFormat;

    WaitForFIFO(4);
    reg_MMA4DC = (clipX << 16) | (clipX + width - 1);   // left and right clip
    reg_MMA4E0 = (clipY << 16) | (clipY + height - 1);  // top and bottom clip
    reg_MMA504 = ((width - 1) << 16) | height;          // width and height
    reg_MMA4E4 = (dstStride << 16) + dstStride;         // stride

    WaitForFIFO(2);
    reg_MMA4D8 = dstBase;                               // dest base address
    reg_MMA50C = ((dstX)<<16) | (dstY & 0x00007FF);     // dest XY
                            
    WaitForFIFO(1);
    reg_MMA500 = cmd;

    return S_OK;
}

// 
//  This really belongs in the EMUL library...
// 

// Assume the destination surface is 5-5-5, ie, color masks of:
//   0x7C00,0x03E0,0x001F
//
// AND-ing with one of these masks will zero the upper one (two) bits of each of the 
// R, G, and B components of a pixel.
// Thus, a pixel color can be "divided" by two (four) by shifting right by one (two) bits 
// and AND-ing with one of these masks to zero the bits that shifted from one color component to 
// another.
//
#define SHIFT_ONE_MASK 0x3DEF
#define SHIFT_TWO_MASK 0x1CE7

SCODE S3Virge::EmulatedBltAlphaText16_555( GPEBltParms *pParms )
{
	DEBUGMSG(GPE_ZONE_BLT_HI,(TEXT("EmulatedBltAlphaText16\r\n")));

    UINT32      iDstScanStride  = pParms->pDst->Stride();
    BYTE       *pDibBitsDst     = (BYTE *)pParms->pDst->Buffer();
    UINT32      iSrcScanStride  = pParms->pMask->Stride();
    BYTE       *pDibBitsSrc     = (BYTE *)pParms->pMask->Buffer();
    PRECTL      prcSrc          = pParms->prclMask;
    PRECTL      prcDst          = pParms->prclDst;
    WORD        wOnColor[5];
    int         iNumDstRows;
    int         iNumDstCols;
    BYTE       *pbSrcScanLine;
    BYTE       *pbDstScanLine;
    BYTE       *pbSrc;
    WORD       *pwDstPixel;
    int         i;
    int         j;
    BOOL        bOdd;
    BYTE        bSrc;

	//DebugBreak();

    // Caller assures a well-ordered, non-empty rect
    // compute size of destination rect
    iNumDstCols = prcDst->right  - prcDst->left;
    iNumDstRows = prcDst->bottom - prcDst->top;

    // compute pointers to the starting rows in the src and dst bitmaps
    pbSrcScanLine = pDibBitsSrc + prcSrc->top * iSrcScanStride + (prcSrc->left >> 1);
    pbDstScanLine = pDibBitsDst + prcDst->top * iDstScanStride + prcDst->left * 2;
    
    // Create pixel values with 0/4, 1/4, 2/4, 3/4 and 4/4 of the solid brush color
    wOnColor[0] = 0;
    wOnColor[4] = (WORD)pParms->solidColor;
    wOnColor[2] = (wOnColor[4] >> 1 ) & SHIFT_ONE_MASK;
    wOnColor[1] = (wOnColor[4] >> 2 ) & SHIFT_TWO_MASK;
    wOnColor[3] = wOnColor[1] + wOnColor[2];


    for (i = 0; i < iNumDstRows; i++) 
    {
        // set up pointers to first bytes on src and dst scanlines
        pbSrc = pbSrcScanLine;
        pwDstPixel = (WORD *)pbDstScanLine;
        bOdd = prcSrc->left & 1;

        for (j = 0; j < iNumDstCols; j++ ) 
        {
            if (bOdd)
                bSrc = *pbSrc++ & 0xF;
            else
                bSrc = *pbSrc >> 4;
            
            bOdd = !bOdd;
            
            switch ( (bSrc + 1) >> 2 )  // src pixel in range 0...4
            {
            case 0:
                //      Leave destination untouched
                break;
            case 1:
                //      3/4 destination color and 1/4 brush color
                *pwDstPixel = ((*pwDstPixel >> 2) & SHIFT_TWO_MASK) 
                            + ((*pwDstPixel >> 1) & SHIFT_ONE_MASK) + wOnColor[1];
                break;
            case 2:
                //      1/2 destination color and 1/2 brush color
                *pwDstPixel = ((*pwDstPixel >> 1) & SHIFT_ONE_MASK) + wOnColor[2];
                break;
            case 3:
                //      1/4 destination color and 3/4 brush color
                *pwDstPixel = ((*pwDstPixel >> 2) & SHIFT_TWO_MASK) + wOnColor[3];
                break;
            case 4:
                //      Fill with solid brush color
                *pwDstPixel = wOnColor[4];
                break;
            default:
                DebugBreak();
            }
            
            pwDstPixel++;
        }

        // advance to next scanline
        pbSrcScanLine += iSrcScanStride;
        pbDstScanLine += iDstScanStride;
    }

	return S_OK;
}
