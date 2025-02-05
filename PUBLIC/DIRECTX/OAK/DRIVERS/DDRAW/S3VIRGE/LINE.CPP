/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-1998  Microsoft Corporation

Module Name:    line.cpp

Abstract:       draws accelerated lines
  
Functions:


Notes:          not all lines are accelerated.
                clipping used

--*/

#include "precomp.h"
#include "dispperf.h"

SCODE 
S3Virge::Line(GPELineParms *pLineParms, EGPEPhase phase)
{
#if VIRGE_VERBOSE_MSGS
    DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("S3Virge::Line ")));
    switch (phase)
    {
    case  gpeSingle:
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("single\n\r")));
        break;
    case gpePrepare:
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("prepare\n\r")));
        break;
    case gpeContinue:
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("continue\n\r")));
        break;
    case gpeComplete:
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("complete\n\r")));
        break;
    }
#endif

    UnlockVGA();
    Unlock();

#if 0
#ifdef ENABLE_ACCELERATION
    if (phase == gpeSingle || phase == gpePrepare)
    {
        DispPerfStart (ROP_LINE);
        pLineParms->pLine = EmulatedLine;
        if (pLineParms->pDst->InVideoMemory() && pLineParms->mix == 0x0d0d)
        {
#if VIRGE_VERBOSE_MSGS
            switch (phase)
            {
            case gpeSingle:
                DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("in single\n\r")));
                break;
            case gpePrepare:
                DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("in prepare\n\r")));
                break;
            }
#endif
            SelectSolidColor(pLineParms->solidColor);
            pLineParms->pLine = 
                (SCODE (GPE::*)(struct GPELineParms *)) AcceleratedSolidLine;
        }
    } 
    else if (phase == gpeComplete) 
    {
        DispPerfEnd (0);
    }
#else
    pLineParms->pLine = EmulatedLine;
#endif
#else
    // turn off line acceration at the moment,
    // there is a bug that needs to be worked out
    // before turning it back on.
    pLineParms->pLine = EmulatedLine;
#endif
    return S_OK;
}


SCODE 
S3Virge::AcceleratedSolidLine(GPELineParms *pLineParms)
{
    int dstBase = ((S3VirgeSurf *) (pLineParms->pDst))->OffsetInVideoMemory();
    int dstStride = ((S3VirgeSurf *) (pLineParms->pDst))->Stride();
    int adjustTop = dstBase / dstStride;
    int adjustLeft = dstBase % dstStride;
    int clipTop, clipBottom, clipRight, clipLeft;
    int x1, x2, y1, y2;

#if VIRGE_VERBOSE_MSGS
    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
              (TEXT(" x=%d y=%d cPels=%d dM=%d dN=%d gamma=%d Dir=%d\r\n"),
               pLineParms->xStart, pLineParms->yStart, 
               pLineParms->cPels, pLineParms->dM, pLineParms->dN,
               pLineParms->llGamma, pLineParms->iDir));
#endif

    // If the number of pixels to light up isn't the same as 
    // the major/minor axis this isn't the expected case, so emulate the line.

    if (((unsigned long) pLineParms->cPels != (pLineParms->dM >> 4)) && 
        ((unsigned long) pLineParms->cPels != (pLineParms->dN >> 4)))
    {
        return EmulatedLine(pLineParms);    
    }
    
    DispPerfType(DISPPERF_ACCEL_HARDWARE);

    //
    // Determine the offsets from the start position. This depends upon 
    // the octant.
    // 
    // LineSegment() could be merged into this code. The seperation helps 
    // keep the line drawing destinct from the CEPC line setup.
    //
    x1 = pLineParms->xStart + adjustLeft;
    y1 = pLineParms->yStart + adjustTop;
        
    //
    // Clip one pixel less in the major axis.
    //
    switch (pLineParms->iDir)
    {
    // X Major 
    case 0:
        // Setup the line
        x2 = x1 + (pLineParms->dM >> 4); 
        y2 = y1 + (pLineParms->dN >> 4);

        // Setup clipping registers for the line
        clipLeft    = x1;
        clipRight   = x2 - 1;
        clipTop     = y1;
        clipBottom  = y2;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom + 1);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, pLineParms->prclClip->left + adjustLeft);
        break;
    case 3:
        // Setup the line
        x2 = x1 - (pLineParms->dM >> 4);
        y2 = y1 + (pLineParms->dN >> 4);

        // Setup clipping registers for the line
        clipLeft    = x2 + 1;
        clipRight   = x1;
        clipTop     = y1;
        clipBottom  = y2;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom + 1);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, 
                          pLineParms->prclClip->left + adjustLeft + 1);
        break;
    case 4:
        // Setup the line
        x2 = x1 - (pLineParms->dM >> 4);
        y2 = y1 - (pLineParms->dN >> 4);

        // Setup clipping registers for the line
        clipLeft    = x2 + 1;
        clipRight   = x1;
        clipTop     = y2;
        clipBottom  = y1;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop + 1);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, 
                          pLineParms->prclClip->left + adjustLeft + 1);
        break;
    case 7:
        // Setup the line
        x2 = x1 + (pLineParms->dM >> 4);
        y2 = y1 - (pLineParms->dN >> 4);

        // Setup clipping registers for the line
        clipLeft    = x1;
        clipRight   = x2 - 1;
        clipTop     = y2;
        clipBottom  = y1;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, pLineParms->prclClip->left + adjustLeft);
        break;

    // Y Major
    case 1:
        // Setup the line
        x2 = x1 + (pLineParms->dN >> 4);
        y2 = y1 + (pLineParms->dM >> 4);

        // Setup clipping registers for the line
        clipLeft    = x1;
        clipRight   = x2;
        clipTop     = y1;
        clipBottom  = y2 - 1;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom + 1);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, pLineParms->prclClip->left + adjustLeft);
        break;
    case 2:
        // Setup the line
        x2 = x1 - (pLineParms->dN >> 4), 
        y2 = y1 + (pLineParms->dM >> 4),

        // Setup clipping registers for the line
        clipLeft    = x2;
        clipRight   = x1;
        clipTop     = y1;
        clipBottom  = y2 - 1;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom + 1);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, pLineParms->prclClip->left + adjustLeft);
        break;
    case 5:
        // Setup the line
        x2 = x1 - (pLineParms->dN >> 4);
        y2 = y1 - (pLineParms->dM >> 4);
        
        // Setup clipping registers for the line
        clipLeft    = x2;
        clipRight   = x1;
        clipTop     = y2 + 1;
        clipBottom  = y1;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop + 1);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, pLineParms->prclClip->left + adjustLeft);
        break;
    case 6:
        // Setup the line
        x2 = x1 + (pLineParms->dN >> 4);
        y2 = y1 - (pLineParms->dM >> 4);
        
        // Setup clipping registers for the line
        clipLeft    = x1;
        clipRight   = x2;
        clipTop     = y2 + 1;
        clipBottom  = y1;

        // Now clip either according to the line itself or the clipping rect
        clipTop     = max(clipTop, pLineParms->prclClip->top  + adjustTop);
        clipBottom  = min(clipBottom, pLineParms->prclClip->bottom + 1);
        clipRight   = min(clipRight, pLineParms->prclClip->right - 1);
        clipLeft    = max(clipLeft, pLineParms->prclClip->left + adjustLeft);
        break;
    }

    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
              (TEXT("Line::adjusted clip: top=%d bottom=%d ")
               TEXT("left=%d right=%d DIR=%d\n\r"),
               clipTop, clipBottom, clipLeft, clipRight, pLineParms->iDir));

    WaitForFIFO(2);
    // Setup clipping registers.
    reg_MMA8DC = (clipLeft << 16) + clipRight;
    reg_MMA8E0 = (clipTop << 16) + clipBottom;

    // Draw the line
    LineSegment(x1, y1, x2, y2, (UCHAR) (pLineParms->solidColor));

    return S_OK;
}


//
// S3 ViRGE Line drawing has 8 distinct cases. It's worth reading up on 
// the Bresenham Line drawing algorithm to follow this code easily. 
//
// With the Bresenham algorithm there are 8 drawing octants. 
// This information is derived from the following:
//
// Line Drawing:
// x1,y1 = start position
// x2,y2 = finish position
// 
// Calculations:
// deltaX = x2 - x1
// deltaY = y2 - y1
// X Error term, deltaX/deltaY
//
// The dirrection of each X and Y
// X_DIR  = -1, 0, +1
// Y_DIR  = -1, 0, +1
//
// If the change in X is larger than the change in Y (ie: deltaX > deltaY) 
// then the line is an X Major line, otherwise it's a Y Major line, 
// (for deltaX == deltaY use Y Major since it is slightly simpler to compute). 
//
// If x1 < x2 then the X_DIR is positive (+1), if x1 > x2 then the X_DIR is 
// negative, otherwise X_DIR is 0. (Similarly for y).
//
//
// As per screen drawing on Windows the upper left hand
// corner is 0,0 with X increasing to the right and Y increasing
// down the screen. If in the following diagram a line is drawn
// from the origin it will be in one of the following 8 octants:
//
//                                                  
//             - Y-axis   
//               
//          \   5  |  6   /
//            \    |    /
//              \  |  /
//           4    \|/    7
// -X-axis  -------+-------- + X-axis
//           3    /|\    0
//              /  |  \
//            /    |    \
//          /   2  |  1   \ 
//       
//             + Y-axis   
//       
//
//  Octant    Axis    X-DIR  Y-DIR  X Error
//     0      X-Major    +1     +1      -
//     1      Y-Major    +1     +1      -
//     2      Y-Major    -1     +1      +
//     3      X-Major    -1     +1      +      
//     4      X-Major    -1     -1      -
//     5      Y-Major    -1     -1      -
//     6      Y-Major    +1     -1      +
//     7      X-Major    +1     -1      +      
//                                                  
//                                                  
// NOTES:
// There is a slight complication in that the S3 ViRGE engine always starts
// drawing from the largest Y = max(y1, y2).
//

void S3Virge::LineSegment(int x1, int y1, int x2, int y2, UCHAR color)
{   
    INT  yStart, deltaX, deltaY;
    INT  xError, xStartPlusErrorTerm, xStart, xEnd, scanLines;
    UINT direction = 0;             // Default drawing is from right to left.
    UINT command;

    DEBUGMSG(VIRGE_VERBOSE_MSGS, 
              (TEXT("Line:: x1,y1=(%d, %d) x2,y2=(%d, %d)\n\r\n\r"), 
               x1, y1, x2, y2));

    // determine the delta in each direction.
    deltaX = x2 - x1;
    deltaY = y2 - y1;

    if (deltaY > 0)
    {
        // Swap the start & end points to switch to the ViRGE drawing direction
        // Octants 0, 1,2,3

        scanLines = 1 + deltaY;     // Total scan lines 
        yStart = y2;

        xStart = x2;
        xEnd = x1;
    }
    else
    {
        scanLines = 1 - deltaY;     // Total scan lines
        yStart = y1;

        xStart = x1;
        xEnd = x2;
    }

    // Determine the slope of the x/y with subpixel precision
    // NOTE: special case for infinity for xError
    if (deltaY == 0)
    {
        xError = 0;                     // Horizontal Line.
        
        if (deltaX > 0)
        {
            direction = 0x80000000;     // Draw lines from left to right.
        }
    }
    else 
    {
        // X Error term, deltaX shift by 20 bits to 
        // get 20 bit subpixel precision
        xError = -(deltaX << 20) / deltaY;

        if (xError > 0)
        {
            direction = 0x80000000;     // Draw lines from left to right.
        }
    }

    // Determine the slope
    if (abs(deltaX) >= abs(deltaY))
    {
        // X Major Line  Octants 0, 3, 4, 7
        if (xError >= 0)
        {
            // Octants 0, 7
            // Starting X + 1/2 X error term
            xStartPlusErrorTerm = (xStart << 20) + (xError/2);  
        }
        else 
        {
            // Octants 3, 4
            xStartPlusErrorTerm = (xStart << 20) + (xError/2) + ((1 << 20) - 1);
        }
    }
    else 
    {
        // Y Major Line  Octants 1, 2, 5, 6
        xStartPlusErrorTerm = xStart << 20;
    }

    WaitForFIFO(7);
#ifdef VIRGE_ROP_BUG
    reg_MMA8F4 = ~color;
#else
    reg_MMA8F4 = color;
#endif
    reg_MMA96C = (xStart << 16) + xEnd;         // Last pixel off for End1
    reg_MMA970 = xError;                        // X direction gradient
    reg_MMA974 = xStartPlusErrorTerm;           // start X coordinate
    reg_MMA978 = yStart;                        // start Y coordinate
    reg_MMA97C = scanLines + direction;         // draw direction and # of 
                                                // scanlines

#ifdef VIRGE_ROP_BUG
    command = 0x18000122 + (0x0F << 17);
#else
    command = 0x18000122 + (0xF0 << 17);
#endif

    reg_MMA900 = command | m_nDestFormat;       // Command Set: clipping, 
                                                // autoexecute off, 8 bpp
}
