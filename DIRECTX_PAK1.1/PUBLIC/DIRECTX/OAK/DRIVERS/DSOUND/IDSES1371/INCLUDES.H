//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************

	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
	PARTICULAR PURPOSE.
	Copyright (c) 1995-2000 Microsoft Corporation

Module Name:

	includes.h

Abstract:

	One single place for include files

*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <types.h>
#include <memory.h>
#include <excpt.h>
#include <nkintr.h>

#include <DSound.h>
#include <DsDriver.h>

#include <tchar.h>
#include "Debug.h"
#include "..\\..\\inc\\irqmap.h"		// For MapInterruptToSystem

#include "globdef.h"
#include "es1371.h"
#include "dsdriverbuffer.h"
#include "dsdriverimp.h"
#include "dscapturedriverbuffer.h"
#include "dsdrivercaptureimp.h"

//don't change the order of the include files