//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1998 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------;
// wmphlpie.h

// used for iesample communication
#define WMP_USER_MESSAGE_STR    L"wmp_user_message"
#define WMPHELPER_URL_STR       L"res://wmphlpr.dll/media.htm#"
#define WMPHELPER_URL_LEN       (sizeof(WMPHELPER_URL_STR) / sizeof(WCHAR))

typedef struct tagWMPHELPERMSG
{
    DWORD       dwThreadId;
    WCHAR       wszURL[1];
} WMPHELPERMSG;

