/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995, 1996, 1997, 1998  Microsoft Corporation

Module Name:    cursor.cpp

Abstract:       S3 ViRGE hardware accelerated cursor
  
Functions:      

Notes:  

--*/

#include "precomp.h"

SCODE 
S3Virge::SetPointerShape(
    GPESurf *pMask, 
    GPESurf *pColorSurf, 
    int xHot, 
    int yHot, 
    int cx, 
    int cy
    )
{
    USHORT shapeRow[8]; // buffer to contain AND word, XOR word, AND word...
    USHORT bitMask;
    PUSHORT pShape, pAndDst, pXorDst, pWord;
    PUCHAR pBuf;
    int count, row, col, y, x, stride;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("S3Virge::SetPointerShape()\r\n")));

    reg_HGC_MODE = reg_HGC_MODE & 0xFE;         // disable cursor

    SetCursorColors(0x00ffffff, 0x00000000);    // Set colors to black & white

    reg_HWGC_STA_H = 0 >> 8;
    reg_HWGC_STA_L = 0 & 0x00ff;

    pWord = (PUSHORT) (&m_pLAW[0 * 1024]);

    if (!pMask)
    {
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("turning off cursor\r\n")));
        
        for (y = 0; y < 64; y++)    // Turn off cursor by setting mask to clear
        {
            for (x = 0; x < 4; x++)
            {
                *pWord++ = 0xFFFF;  //ANDWord
                *pWord++ = 0X0000;  //XORWord
            }
        }
    }
    else
    {
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("Setting pointer shape\r\n")));

        // A cursor shape has been entered, draw it.
        
        // Setup freq addressed information into local variables.
        stride = pMask->Stride();
        pBuf = (PUCHAR) pMask->Buffer();
        
        // initialize entire 64 x 64 shape to be 
        // ANDs=0xFFFF and XORs = 0x0000 (i.e. transparent)
        m_nXHot = xHot;
        m_nYHot = yHot;     // we rely on MovePointer being called after this

        for (row = 0; row < 64; row++)
        {
            // setup the shapeRow array
            for (pShape = shapeRow, count= 0; count < 8; count += 2)
            {
                *pShape++ = 0xFFFF;
                *pShape++ = 0x0000;
            }

            if (row < cy)
            {
                // Write available bits into shapeRow
                pAndDst = shapeRow;
                pXorDst = pAndDst+1;
                bitMask = 0x0080;       // Starting here achieves byte swapping

                for (col = 0; col < cx; col++)
                {
                    if (!((pBuf + row * stride)[col / 8] & 
                          (1 << (7 - (col % 8)))))
                    {
                        *pAndDst ^= bitMask;
                    }

                    if (((pBuf + (cy + row) * stride)[col / 8] & 
                         (1 << (7 - (col % 8)))))
                    {
                        *pXorDst ^= bitMask;
                    }

                    bitMask >>= 1;
                    if (bitMask == 0x0080)
                    {
                        pAndDst += 2;
                        pXorDst += 2;
                    }

                    if (!bitMask)
                    {
                        bitMask = 0x8000;
                    }
                }
            }

            for (pShape = shapeRow, x = 0; x < 8; x++)
                *pWord++ = *pShape++;   // AND Word, then XOR word,...
        }
    }

    DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("Cursor shape set done\r\n")));

    return S_OK;
}

//
// Reposition the cursor
//
SCODE 
S3Virge::MovePointer(int x, int y)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("S3Virge::MovePointer(%d, %d)\r\n"), x, y));

    int newX = x - m_nXHot; // Hot spot is currently 32,32
    int newY = y - m_nYHot;
    int dx = 0;
    int dy = 0;

    if (newX < 0)
    {
        dx = -newX;
        newX = 0;
    }
    if (newY < 0)
    {
        dy = -newY;
        newY = 0;
    }

    if (x == -1)
    {
        reg_HGC_MODE = reg_HGC_MODE & 0xFE;     // disable cursor
    }

    reg_HWGC_DX = dx;
    reg_HWGC_DY = dy;
    reg_HWGC_ORGX_L = newX & 0x00FF;
    reg_HWGC_ORGX_H = newX >> 8;
    reg_HWGC_ORGY_L = newY & 0x00FF;
    reg_HWGC_ORGY_H = newY >> 8;                // this must be done last

    if (x != -1)
        reg_HGC_MODE = reg_HGC_MODE | 1;        // enable cursor

    return S_OK;
}



void S3Virge::SetCursorColors(ULONG foreground, ULONG background)
{
    volatile UCHAR temp;

    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("S3Virge::SetCursorColors(0x%x, 0x%x)\r\n"), 
        foreground, background));

    reg_HGC_MODE = reg_HGC_MODE & 0xFE;             // Turn off cursor & reset 
                                                    // cursor stack
    temp = reg_HGC_MODE;
    reg_HWGC_FGSTK =    (UCHAR) foreground;         // First byte of foreground
                                                    // color of cursor
    reg_CR_DATA =       (UCHAR) (foreground >> 8);  // Second byte
    reg_CR_DATA =       (UCHAR) (foreground >> 16); // Third byte
    temp = reg_HGC_MODE;
    reg_HWGC_BGSTK =    (UCHAR) background;         // First byte of background
                                                    // color of cursor
    reg_CR_DATA =       (UCHAR) (background >> 8);  // Second byte
    reg_CR_DATA =       (UCHAR) (background >> 16); // Third byte
}
