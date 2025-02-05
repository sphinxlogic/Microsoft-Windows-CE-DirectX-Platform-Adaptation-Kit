// FILE: nsdll.cpp
//
// Copyright (c) Microsoft Corporation 2000. All Rights Reserved
//
// NOTES: This file controls which pieces of functionality are included in dxmasf.dll
// 

#include <dxmasf.h>
#include <ceconfig.h> // This gets us all the DXMASF_ defines that specify which pieces of functionality to include

#ifndef NO_AMOVIESETUP
#error Something is messed up!  NO_AMOVIESETUP needs to be defined!!!
#endif

CFactoryTemplate g_Templates[] = {
    INIT_SPLITTER,
#ifdef DXMASF_NSCFILEPARSER
    INIT_NSCFILEPARSER,
#endif
#ifdef DXMASF_ACMOBJ
    INIT_ACMHANDLER,
#endif
#ifdef DXMASF_ICMOBJ
    INIT_ICMHANDLER,
#endif
#ifdef DXMASF_URLOBJ
    INIT_URLHANDLER,
#endif
#ifdef DXMASF_JPEGOBJ
    INIT_JPEGHANDLER,
#endif
#ifdef DXMASF_ANIMOBJ
    INIT_ANIMHANDLER,
#endif
#ifdef DXMASF_DIBOBJ
    INIT_DIBHANDLER,
#endif
#ifdef DXMASF_XMLPLAYL
   INIT_XMLASXFILEPARSER,
#endif
#ifdef DXMASF_M3UPLAYL
   INIT_M3UFILEPARSER,
#endif
#ifdef DXMASF_NSMULTI
   INIT_MULTIPLEXER,
#endif
#ifdef DXMASF_URLGRAB
   INIT_URLGRAB
#endif
};

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

AMSREGISTRYENTRY g_MediaStreamRegistry [] =
{
#ifdef DXMASF_HTTPSTRM
    { CHttpStream::CreateInstance },
#endif
#ifdef DXMASF_RTSPSTRM    
    { CRtspStream::CreateInstance },
#endif
#ifdef DXMASF_MSSTRM
    { CMediaServerStream::CreateInstance },
#endif
#ifdef DXMASF_ADIOSTRM
    { CAdioFileStream::CreateInstance },
#endif
#ifdef DXMASF_FILESTRM   
    { CFileStream::CreateInstance },
#endif   
#ifdef DXMASF_TIGRSTRM
    { CTigerStream::CreateInstance },
#endif
#ifdef DXMASF_MSBSTRM
    { CMsbStream::CreateInstance },
#endif
#ifdef DXMASF_AUDSTRM
    { CLiveAudioStream::CreateInstance },
#endif
    { NULL },
};

int g_cMediaStreamRegistrySize = sizeof(g_MediaStreamRegistry)/sizeof(AMSREGISTRYENTRY);
