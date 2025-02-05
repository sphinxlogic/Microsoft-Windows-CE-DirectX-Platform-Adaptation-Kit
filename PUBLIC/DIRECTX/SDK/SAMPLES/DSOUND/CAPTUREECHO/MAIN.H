#pragma once
#ifndef __MAIN_H__
#define __MAIN_H__

#include <windows.h>
#include <stdlib.h>

// Dreamcast Settings
#if !defined(CFG_PLATFORM_NAME) && (defined(KATANA) || defined(DREAMCAST) || defined(DRAGON) || defined(QADRAGON))
#   define CFG_PLATFORM_DREAMCAST 200
#   define CFG_PLATFORM_NAME "Dreamcast"
#endif

// NT Settings
#if defined(UNDER_NT)
#   include <assert.h>
#   include <winbase.h>
#   include <malloc.h>
#   include <memory.h>
#   include <stdio.h>
#   include <tchar.h>
#else
    // We're under windows CE, which has the standard stuff in windows.h and stdlib.h
#endif

#if !defined(DEBUG) && defined(_DEBUG)
#define DEBUG
#endif

#ifndef ASSERT
#   ifdef DEBUG
#       define ASSERT(EXPR) assert(EXPR)
#   else
#       define ASSERT(EXPR)
#   endif
#endif

#include <tchar.h>
#include <dsound.h>

// If you want to check HRESULTs, set this
// Macro to non-zero.
#define CFG_CHECK_HRESULTS 1

#endif