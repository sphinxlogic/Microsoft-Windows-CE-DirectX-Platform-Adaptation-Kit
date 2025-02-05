//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
#ifndef __WINX__
#define __WINX__

// this file tells us which version of the wince os we are using
// version 3.0 and up have native support for semaphores
#include <bldver.h>
#if (CE_MAJOR_VER >= 0x0003)
#define OS_SUPPORTED_SEMAPHORES
#endif

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#ifdef UNDER_CE

#define _strinc(_pc)    ((_pc)+1)

#define lstrcpyA    strcpy
LPSTR lstrcpynA(LPSTR lpString1, LPCSTR lpString2, int iMaxLength);
BOOL FileTimeToDosDateTime(CONST FILETIME *lpFileTime, LPWORD lpFatDate, LPWORD lpFatTime);

#define CoInitialize(x) CoInitializeEx(x, COINIT_MULTITHREADED)

long RegCreateKey(HKEY hkey, LPCWSTR lpszSubKey, PHKEY phkResult);
long RegSetValue(HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCTSTR lpData, DWORD cbData);
long RegOpenKey(HKEY hKey, LPCWSTR lpszSubKey, PHKEY phkResult);
long RegQueryValue(  HKEY hKey,  LPTSTR lpSubKey,  LPTSTR lpValue,  PLONG lpcbValue  );

#if (CE_MAJOR_VER < 0x0003)

int __cdecl isdigit(int c);
int __cdecl isxdigit(int c);
int __cdecl isalpha(int c);
int __cdecl isspace(int c);
int __cdecl iscntrl(int c);

size_t __cdecl strspn (const char * string, const char * control);
char * __cdecl _strlwr (char * string);
long __cdecl strtol (const char *nptr, char **endptr, int ibase );
unsigned long __cdecl strtoul (const char *nptr, char **endptr, int ibase);
char * __cdecl strrchr (const char * string, int ch);
int __cdecl _strcmpi(const char * dst, const char * src);
#define _stricmp(a, b)  _strcmpi(a, b)
int __cdecl _strnicmp (const char * first, const char * last, size_t count);

// from basetsd.h
typedef int INT_PTR, *PINT_PTR;
typedef unsigned int UINT_PTR, *PUINT_PTR;

typedef long LONG_PTR, *PLONG_PTR;
typedef unsigned long ULONG_PTR, *PULONG_PTR;

typedef unsigned short UHALF_PTR, *PUHALF_PTR;
typedef short HALF_PTR, *PHALF_PTR;

typedef HANDLE	HMONITOR;
typedef LONG 	OAHWND;
typedef DWORD   FOURCC;

#define HandleToUlong( h ) ((ULONG) (h) )
#define HandleToLong( h ) ((LONG) (h) )
#define PtrToUlong( p ) ((ULONG) (p) )
#define PtrToLong( p ) ((LONG) (p) )
#define PtrToUshort( p ) ((unsigned short) (p) )
#define PtrToShort( p ) ((short) (p) )

#define MAXUINT_PTR  ((UINT_PTR)~0)
#define MAXINT_PTR   ((INT_PTR)(MAXUINT_PTR >> 1))
#define MININT_PTR   (~MAXINT_PTR)

#define MAXULONG_PTR ((ULONG_PTR)~0)
#define MAXLONG_PTR  ((LONG_PTR)(MAXULONG_PTR >> 1))
#define MINLONG_PTR  (~MAXLONG_PTR)

#define MAXUHALF_PTR ((UHALF_PTR)~0)
#define MAXHALF_PTR  ((HALF_PTR)(UHALF_PTR >> 1))
#define MINHALF_PTR  (~MAXHALF_PTR)

// SIZE_T used for counts or ranges which need to span the range of
// of a pointer.  SSIZE_T is the signed variation.
typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;

// Add Windows flavor DWORD_PTR types
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

// The following types are guaranteed to be signed and 64 bits wide.
typedef __int64 LONG64, *PLONG64;
typedef __int64 INT64,  *PINT64;

// The following types are guaranteed to be unsigned and 64 bits wide.
typedef unsigned __int64 ULONG64, *PULONG64;
typedef unsigned __int64 DWORD64, *PDWORD64;
typedef unsigned __int64 UINT64,  *PUINT64;

// from winnt.h
#define MAXLONGLONG                      (0x7fffffffffffffff)

#ifndef MoveMemory
#define MoveMemory(Destination,Source,Length) \
        memmove((Destination),(Source),(Length))
#endif
#ifndef CopyMemory
#define CopyMemory(Destination,Source,Length) \
        memcpy((Destination),(Source),(Length))
#endif
#ifndef FillMemory
#define FillMemory(Destination,Length,Fill) \
        memset((Destination),(Fill),(Length))
#endif
#ifndef ZeroMemory
#define ZeroMemory(Destination,Length) \
        memset((Destination),0,(Length))
#endif

#else // (CE_MAJOR_VER < 0x0003)

#endif // (CE_MAJOR_VER < 0x0003)

#endif // UNDER_CE

#ifndef OS_SUPPORTED_SEMAPHORES
// semaphore routines
typedef struct _Semaphore {
    LONG lValue;           // current value
    LONG lMaximum;         // max value
    HANDLE hEvent;         // use event to implement semaphore
    CRITICAL_SECTION crit; // critical section
} SEMAPHORE, *PSEMAPHORE;

HANDLE
CreateSemaphore(
    LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    LONG lInitialCount,
    LONG lMaximumCount,
    LPCTSTR lpName
    );
#endif // OS_SUPPORTED_SEMAPHORES

#ifndef OS_SUPPORTED_SEMAPHORES
// mirror CloseHandle(hSem)
BOOL
DestroySemaphore(
    HANDLE hSem
    );
#else
#define DestroySemaphore(hSem)  CloseHandle(hSem)
#endif // OS_SUPPORTED_SEMAPHORES

#ifndef OS_SUPPORTED_SEMAPHORES
BOOL
ReleaseSemaphore(
    HANDLE hSem,
    LONG lReleaseCount,
    LPLONG lpPreviousCount
    );
#endif // OS_SUPPORTED_SEMAPHORES

#ifndef OS_SUPPORTED_SEMAPHORES
// mirror WaitForSingleObject(hSem, dwMilliseconds)
DWORD
WaitForSemaphore(
    HANDLE hSem,
    DWORD dwMilliseconds
    );
#else
#define WaitForSemaphore(hSem, dwMilliseconds)  WaitForSingleObject(hSem, dwMilliseconds)
#endif // OS_SUPPORTED_SEMAPHORES

// a fairly specific function use in msreader\reader.cpp
// once CE supports Semaphores we can remove this function
DWORD
WaitForSemaphoreOrEvent(
    HANDLE hSem,
    HANDLE hEvent,
    DWORD dwMilliseconds
    );

#if (CE_MAJOR_VER < 0x0003)
#define Int32x32To64(a, b) ((LONGLONG)((LONG)(a)) * (LONGLONG)((LONG)(b)))
#define UInt32x32To64(a, b) ((DWORDLONG)((DWORD)(a)) * (DWORDLONG)((DWORD)(b)))
#endif

// should be in wingdi.h
#define BI_RLE8       1L

// time slice: 25 ms
#define TIME_SLICE    25

#define MulDiv(x, y, z) (DWORD)(((LONGLONG)((LONG)(x)) * (LONGLONG)((LONG)(y)))\
                        / (LONGLONG)((LONG)(z)))

#ifndef USE_PERF_COUNTER

#define timeGetTime() GetTickCount()

#else

class PerfCounter {
public:
    PerfCounter();
    DWORD GetTime();
private:
    DWORD *m_pTimer;
};

extern PerfCounter * g_pTimer;

#define timeGetTime() (g_pTimer->GetTime())

#endif // USE_PERF_COUNTER

// supported in Cedar release of WinCE
#if CE_MAJOR_VER < 0x0003

/* Global Memory Flags */
#define GMEM_SHARE      0
#define GMEM_FIXED      0
#define GMEM_MOVEABLE   0   // defining this as LMEM_MOVEABLE will cause the allocation to FAIL!
#define GMEM_DDESHARE   0
#define GMEM_NODISCARD  LMEM_NODISCARD
#define GMEM_ZEROINIT   LMEM_ZEROINIT

#define GHND            LHND
#define GPTR            (LMEM_FIXED | LMEM_ZEROINIT)

#define GlobalAlloc     LocalAlloc
#define GlobalReAlloc   LocalReAlloc
#define GlobalFree      LocalFree
#define GlobalLock      LocalLock
#define GlobalUnlock    LocalUnlock
#define GlobalHandle    LocalHandle
#define GlobalSize      LocalSize
#define GlobalFlags     LocalFlags

#endif // #if CE_MAJOR_VER < 0x0003

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif // __WINX__
