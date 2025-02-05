//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-1999 Microsoft Corporation

Module Name:

	Debug.h

Abstract:

	Header file for debug registration.

Revision History:


*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// BEGIN INCLUDES
//
// standard includes:
//
//#include <windows.h>
//
// where we can find declaration for main class in this file:
//
//
//
// where we can find declarations needed in this file:
//

//
// END INCLUDES
//////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG_H_
#define _DEBUG_H_

//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
Setup up debug zones
*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG


// Uncomment this to turn on debugging for latency.
//#define DEBUG_LATENCY


// Uncomment this to debug play position.
//#define DEBUG_PLAY_POSITION




void __cdecl
ZoneDbgPrintf(PTCHAR pszFormat, ...);

#define MODNAME TEXT("IDsES1371")

#define DBG_BIT(bitno) (1<<(bitno))

#define ZONE_ERROR		DEBUGZONE(0)
#define ZONE_WARNING	DEBUGZONE(1)
#define ZONE_ENTER		DEBUGZONE(2)
#define ZONE_INIT		DEBUGZONE(3)
#define ZONE_INFO		DEBUGZONE(4)
#define ZONE_IOCTL		DEBUGZONE(5)
#define ZONE_REFCOUNT	DEBUGZONE(6)
#define ZONE_RESOURCES	DEBUGZONE(7)
#define ZONE_CRITSEC	DEBUGZONE(8)
#define ZONE_MIXER		DEBUGZONE(9)
#define ZONE_PERF		DEBUGZONE(14)
#define ZONE_TEMP		DEBUGZONE(15)

#ifdef INSTANTIATE_DEBUG
DBGPARAM dpCurSettings =
{
	MODNAME,
	{
		TEXT("Errors"),			// 0
		TEXT("Warnings"),		// 1
		TEXT("Enter,Exit"),		// 2
		TEXT("Initialize"),		// 3
		TEXT("Info"),			// 4
		TEXT("IOCTL"),			// 5
		TEXT("RefCount"),		// 6
		TEXT("Resources"),		// 7
		TEXT("Critical Sections"),	// 8
		TEXT("Mixer"),	 		// 9
		TEXT("Undefined"),		// 10
		TEXT("Undefined"),		// 11
		TEXT("Undefined"), 		// 12
		TEXT("Undefined"),		// 13
		TEXT("Performance"),	// 14
		TEXT("Temporary tests")	// 15
	},
//	0x0000
//  0xffff						// Show everything
	0x0001 | 0x0002				// Errors and warnings
//	0x0001 | 0x0002 | 0x8000	// Errors, warnings, and temporary messages
//	0x0001 | 0x0002 | 0x8000 | 0x0200	// Errors, warnings, temporary and mixer messages
//	0x0001 | 0x0002 | 0x0040	// Errors, warnings, and refcount
//	0x0001 | 0x0002 | 0x8000 | 0x4000	// Errors, warnings, temporary messages, and perf
};
#endif // INSTANTIATE_DEBUG

#else // NOT DEBUG



#endif //DEBUG


#endif // _DEBUG_H_
