#pragma once
#ifndef __DEBUGUTY_H__
#define __DEBUGUTY_H__

extern void dbgout(LPCTSTR pszFormat, ...);

#if CFG_CHECK_HRESULTS
#   define CheckHRESULT(__HR, __MSG, __ACTION) \
        if (FAILED(__HR)) \
        { \
            dbgout(TEXT("%s returned hr=%s\n"), TEXT(__MSG), g_ErrorMap[__HR]); \
            __ACTION; \
        }
#else // CFG_CHECK_HRESULTS
#   define CheckHRESULT(__HR, __MSG, __ACTION)
#endif // CFG_CHECK_HRESULTS

#define TRACE dbgout(TEXT("TRACE: %s(%d)\n"), TEXT(__FILE__), __LINE__)

#endif