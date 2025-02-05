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
//	cursor.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#include "precomp.h"


#define INITIAL_MASK  0x00008000

// disable cursor macro
#define DISABLE_CURSOR()   (reg_CURSOR1_CONTROL = reg_CURSOR1_CONTROL & 0xFE)
    // enable cursor
/*64x64 cursor*/
#define ENABLE_CURSOR()    {(reg_CURSOR1_CONTROL = reg_CURSOR1_CONTROL | (0x3));}



//
//  SetPointerShape -
//  inputs:
//  outputs: none
//
SCODE IGS5000::SetPointerShape(
    GPESurf *pMask,
    GPESurf *pColorSurf,
    int xHot,
    int yHot,
    int cx,
    int cy )
{
    DEBUGMSG( GPE_ZONE_CURSOR, (TEXT("IGS5000::SetPointerShape\r\n")));


    ULONG shapeRow[4];	// buffer to contain AND word, XOR word, AND word...
    int count, row, col;
    ULONG *pAndOrDst;
    ULONG bitMask;
    int   x;
    ULONG *pWord;

    if (m_inHostBlt) {
	return S_FALSE;
    }

    DISABLE_CURSOR();	// disable cursor

    SetCursorColors( 0x00ffffff, 0x00000000 );	// Set colors to black & white

	// set the cursor pattern/offset
    reg_CURSOR1_XPRESET = 0x00;
    reg_CURSOR1_YPRESET = 0x00;

    reg_CURSOR1_ADDRLOW = (unsigned char) (m_CurOfs >> 10);    //low byte in unit of 1024 bytes
    reg_CURSOR1_ADDRHI	= (unsigned char) (m_CurOfs >> 10+8);  //hi  byte in unit of 1024 bytes

    pWord = m_CurPtr;	//cursor data location

    // initialize entire 64 x 64 shape to transparent

    m_nXHot = xHot;
    m_nYHot = yHot;	// we rely on MovePointer being called after this

    for( row=0; row<64; row++ ) {
	DEBUGMSG( GPE_ZONE_CURSOR, (TEXT("Setting pointer shape row %d\r\n"),row));
	// Clear the row
	for( count=0; count < 4; count++ ) {
	    shapeRow[count] = 0xAAAAAAAA;
	}
	if( row < cy ) {   // active bits in the cursor shape passed in
	    // Write available bits into shapeRow
	    pAndOrDst = shapeRow;
	    bitMask = INITIAL_MASK;
	    for( col=0; col<cx; col++ ) {
		if( !((((unsigned char *)pMask->Buffer())+row*pMask->Stride())[col/8] & ( 1 << ((col%8)) )) ) {
		    *pAndOrDst ^= bitMask;
		}
		bitMask >>= 1;
		if( bitMask == INITIAL_MASK) {
		    pAndOrDst += 1;
		}
		if( !bitMask ) {
		    bitMask = 0x80000000;
		}
		if( ((((unsigned char *)pMask->Buffer())+(cy+row)*pMask->Stride())[col/8] & ( 1 << ((col%8)) )) ) {
		    *pAndOrDst ^= bitMask;
		}
		bitMask >>= 1;
		if( bitMask == INITIAL_MASK) {
		    pAndOrDst += 1;
		}
		if( !bitMask ) {
		    bitMask = 0x80000000;
		}
	    }
	}

	for( x=0; x<4; x++ ) {
	    *pWord++ = shapeRow[x];
	}
    }

#ifdef TVOUT
    if (reg_CR[0x45] & 0x01)  /*interpolation on or off*/
      reg_XR[0x57] = reg_XR[0x57] & ~0x30;
    else
      reg_XR[0x57] = reg_XR[0x57] |  0x30;
#endif

    DEBUGMSG( GPE_ZONE_CURSOR, (TEXT("Leaving IGS5000::SetPointerShape\r\n")));
    return S_OK;
}


//
//  MovePointer -
//  inputs:
//  outputs: none
//
SCODE IGS5000::MovePointer(
    int x,
    int y )
{
    DEBUGMSG(GPE_ZONE_CURSOR,
	     (TEXT("IGS5000::MovePointer Moving cursor to %d,%d\r\n"),
	      x, y ));

   BYTE itemp = reg_XR[0x56];

#ifdef TVOUT
  int iDropLine, iRemainder;

	if (wV_Underscan==1)
	{
	   iDropLine = y / iMaxLines;
	   iRemainder= y % iMaxLines;
	   if (iRemainder > 0)
	     iDropLine ++;

	   y = y - iDropLine;
	}
#endif

    int newx = x - m_nXHot;	// Hot spot is currently 32,32
    int newy = y - m_nYHot;

    int dx = 0;
    int dy = 0;

    if( newx < 0 ) {
	dx = -newx;
	newx = 0;
    }
    if( newy < 0 ) {
	dy = -newy;
	newy = 0;
    }

    if( x == -1 ) {	  // disable the cursor
	DISABLE_CURSOR();
    }

	// set the cursor origin
    reg_CURSOR1_XLOW = newx & 0x00FF;
    reg_CURSOR1_XHI  = newx >> 8;
    reg_CURSOR1_YLOW = newy & 0x00FF;
    reg_CURSOR1_YHI  = newy >> 8;

    if( x != -1 ) {
	ENABLE_CURSOR();
    }

    DEBUGMSG(GPE_ZONE_CURSOR, (TEXT("Leaving IGS5000::MovePointer\r\n")));
    return S_OK;
}


//
//  SetCursorColors -
//  inputs:
//  outputs: none
//
void IGS5000::SetCursorColors(
    unsigned long foreground,	// 0x00bbggrr format
    unsigned long background )
{
    DEBUGMSG (GPE_ZONE_CURSOR, (TEXT("IGS5000::SetCursorColors\r\n")));

    DISABLE_CURSOR();

    BYTE bTmp;
	// write cursor colors	in RAMDAC
    bTmp = reg_CURSOR1_CONTROL;

    reg_CURSOR1_CONTROL = bTmp | 0x04;

    reg_DAC_WR_AD = (unsigned char)(1);
    reg_DAC_DATA = (unsigned char)foreground;
    reg_DAC_DATA = (unsigned char)(foreground>>8);
    reg_DAC_DATA = (unsigned char)(foreground>>16);
    reg_DAC_WR_AD = (unsigned char)(0);
    reg_DAC_DATA = (unsigned char)background;
    reg_DAC_DATA = (unsigned char)(background>>8);
    reg_DAC_DATA = (unsigned char)(background>>16);

    reg_CURSOR1_CONTROL = bTmp;

    DEBUGMSG (GPE_ZONE_CURSOR, (TEXT("Leaving IGS5000::SetCursorColors\r\n")));
}

#ifdef TVOUT
/*--------------------------------------------------------------------
  DetectTVVUnderscan: Detect whether TV out vertical underscan is turned
		     on or not.
  in   none
  out: set a few variables to be used in MoveCursor().

  Note: When vertical underscan is on, the hardware cursor position
	will have to be adjusted to be able to point to correct position
	in the screen.
  --------------------------------------------------------------------*/
void IGS5000::DetectUnderscan(void)
{
  wV_Underscan = (reg_CR[0x45] & 0x10) >>4;
  if (wV_Underscan==1)
  {
    iMaxLines  = reg_CR[0x44];

    iWhichLine = iMaxLines >> 4;
    iWhichLine += 1;
    iMaxLines  = iMaxLines & 0x0F;
    iMaxLines  += 1;
  }
}
#endif
