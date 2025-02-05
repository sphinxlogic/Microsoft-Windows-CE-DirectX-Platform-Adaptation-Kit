//==========================================================================;
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright (c) 1992 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//-----------------------------------------------------------------------------
// File: DSPlay.cpp
//
// Desc: Simple waveaudio player using DirectSound API.  You can pass in a
//       filename (with or without .wav extension) and an optional loop count.
//

#ifndef DEBUG
#define DEBUG
#define _DEBUG
#endif

#include <windows.h>
#include <dsound.h>
#include <stdio.h>
#include <stdlib.h>
#include "ceshio.h"
#include "structs.h"

//
// Constants
//
#define ZONE_ERROR  1
#define ZONE_TEMP   1

//
// Function prototypes
//
int ParseCommandLine (void);
void DumpWaveStatistics (SWaveHeader *pWaveHeader, SDataHeader *pDataHeader);
int LoopWaveFile (int nNumLoops);
int ReadWaveFileData (void);
int ReadWaveHeaderData(void);
int SetupDirectSoundBuffer(void);
void CleanupDirectSound(void);


//
// Global data - for simplicity
//
SCODE sc;
LPDIRECTSOUND pDS=0;
LPDIRECTSOUNDBUFFER pDSB=0;
SWaveHeader UNALIGNED *pWaveHeader=0;
SMiscHeader UNALIGNED *pMiscHeader=0;
SDataHeader UNALIGNED *pDataHeader=0;
DSBUFFERDESC dsbdesc={0};
SRiffHeader  riffHeader={0};
unsigned char *bufferPtr=0, *fileBuffer=0;
unsigned char *dataPtr1=0, *dataPtr2=0;
unsigned long count1=0, count2=0;
unsigned long dsStatus=0;
BYTE * pBuffer=NULL;
LPWSTR lpCmdLine=0;
int nLoops=0;


int
WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmd,
    int nCmdShow )
{
    int rc;

    // Copy WinMain parameter into global variable for convenience
    lpCmdLine = lpCmd;

    DEBUGMSG(ZONE_TEMP, (TEXT("\n\rDirectSound Test: DSPLAY %s\r\n"), lpCmdLine));

    //
    // Search the command line for a loop value and proper filename
    //
    rc = ParseCommandLine();
    if (rc != 0)
        return (rc);

    //
    // Read audio data from file into a buffer
    //
    DEBUGMSG(ZONE_TEMP, (TEXT("Reading sound file from WinCE Release Dir...\r\n")));
    rc = ReadWaveFileData();
    if (rc != 0)
        return (rc);

    //
    // Parse the wave header
    //
    rc = ReadWaveHeaderData();
    if (rc != 0)
        return (rc);

    //
    // Display wave header statistics
    //
    DumpWaveStatistics (pWaveHeader, pDataHeader);

    //
    // Open DirectSound
    //

    if( DirectSoundCreate( 0, &pDS, NULL ) != 0 )
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error creating DirectSound object\r\n")));
        Sleep(1000);
        return -1;
    }

    //
    // Create and configure the direct sound buffer
    //
    rc = SetupDirectSoundBuffer();
    if (rc != 0)
    {
        CleanupDirectSound();
        return (rc);
    }

    //
    //  Play the wave file any number of times (specified by cmd line arg)
    //
    rc = LoopWaveFile(nLoops);
    if (rc != 0)
    {
        CleanupDirectSound();
        return (rc);
    }

    //
    // Delete Sound buffer, sound device, close file
    //
    CleanupDirectSound();

    DEBUGMSG(ZONE_TEMP,(TEXT("Finished playing %s.\n\r\n"), lpCmdLine));
    return 0;
}



int ParseCommandLine (void)
{
    TCHAR *lpSpace, szLoop[32]={0};

    if (!lpCmdLine)
        return (-1);

    // First find a space between [wavefile] and [loop count]
    lpSpace = wcsrchr(lpCmdLine, ' ');
    if (lpSpace == 0)
    {
       nLoops = 1;
    }

    // Space found.  Look for loop count.
    else
    {
       wcscpy (szLoop, lpSpace);
       nLoops = _wtoi (szLoop);

       // NULL out the loop counter to leave just the wavefile name
       *lpSpace = '\0';
    }

    // Add the .WAV extension if not present
    if (wcsstr(lpCmdLine, TEXT(".wav")) == 0)
    {
       wcscat (lpCmdLine, TEXT(".wav\0"));
    }

    // Success
    return (0);
}


void DumpWaveStatistics (SWaveHeader *pWaveHeader, SDataHeader *pDataHeader)
{
    DEBUGMSG(ZONE_TEMP,(TEXT("-------------------------------------------------------------------\r\n")));
    DEBUGMSG(ZONE_TEMP,(TEXT("         Wave Header Information: [%s]\r\n"), lpCmdLine));
    DEBUGMSG(ZONE_TEMP,(TEXT("SamplesPerSec:  %d\t"),  pWaveHeader->pcmwf.wf.nSamplesPerSec ));
    DEBUGMSG(ZONE_TEMP,(TEXT("nChannels: %d\t"),       pWaveHeader->pcmwf.wf.nChannels ));
    DEBUGMSG(ZONE_TEMP,(TEXT("BitsPerSample: %d\r\n"), pWaveHeader->pcmwf.wBitsPerSample ));
    DEBUGMSG(ZONE_TEMP,(TEXT("AvgBytesPerSec: %d\t"),  pWaveHeader->pcmwf.wf.nAvgBytesPerSec ));
    DEBUGMSG(ZONE_TEMP,(TEXT("Format: %4d\t"),         pWaveHeader->pcmwf.wf.wFormatTag ));
    DEBUGMSG(ZONE_TEMP,(TEXT("BlockAlign: %d"),        pWaveHeader->pcmwf.wf.nBlockAlign ));

    DEBUGMSG(ZONE_TEMP,(TEXT("\r\n-------------------------------------------------------------------\r\n")));
}


int ReadWaveFileData (void)
{
    // Open wave file
    int fh = pp_open(lpCmdLine, _O_RDONLY);

    if( fh == -1 )
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error opening %s. Error code = 0x%08x\r\n"), lpCmdLine,
                 GetLastError() ));
        Sleep(1000);
        return -1;
    }

    // Read file and determine sound format
    // Start with RIFF header:
    unsigned long bytesRead;

    if ((bytesRead = pp_read(fh, (BYTE *)&riffHeader, sizeof(riffHeader))) == 0)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error reading file header\r\n")));
        Sleep(1000);
        return -1;
    }

    // Read the rest of the file into a temporary buffer
    fileBuffer = (unsigned char *)LocalAlloc( LMEM_FIXED, riffHeader.remainingBytes );
    if( fileBuffer == (unsigned char *)NULL )
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error allocating %d bytes\r\n"), riffHeader.remainingBytes ));
        Sleep(1000);
        return -1;
    }

    if ((bytesRead = pp_read(fh, fileBuffer, riffHeader.remainingBytes)) == 0)
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error reading file\r\n")));
        Sleep(1000);
        return -1;
    }

    pp_close(fh);

    // Success
    return (0);
}


int ReadWaveHeaderData(void)
{
    // Now walk through the file decoding the chunks
    // First, we require a WAVE chunk
    pWaveHeader = (SWaveHeader *)fileBuffer;
    if( pWaveHeader->WaveCC != waveCC )
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Unable to read WAVE chunk\r\n")));
        Sleep(1000);
        return -1;
    }

    bufferPtr = fileBuffer + pWaveHeader->ckSize + 2 * sizeof(TFourCC) + 4;
    pDataHeader = (SDataHeader *)NULL;
    while( bufferPtr > fileBuffer && bufferPtr < fileBuffer + riffHeader.remainingBytes )
    {
        pMiscHeader = (SMiscHeader *)bufferPtr;
        if( pMiscHeader->MiscCC == dataCC )
            pDataHeader = (SDataHeader *)pMiscHeader;
        bufferPtr += sizeof(SMiscHeader) + pMiscHeader->ckSize;
    }

    if( !pDataHeader )
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Unable to locate data chunk!\r\n")));
        Sleep(1000);
        return -1;
    }

    // Success
    return (0);
}


int SetupDirectSoundBuffer(void)
{
    // Create DirectSoundBuffer
    memset( (void *)&dsbdesc, 0, sizeof(dsbdesc) );
    dsbdesc.dwSize = sizeof(dsbdesc);
    dsbdesc.dwFlags = DSBCAPS_CTRLDEFAULT;
    dsbdesc.dwBufferBytes = pDataHeader->dataSize;
    dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&(pWaveHeader->pcmwf);

    if( FAILED( sc=pDS->CreateSoundBuffer( &dsbdesc, &pDSB, NULL )))
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error 0x%x creating direct sound buffer\r\n"),sc));
        return -1;
    }

    // Populate Buffer
    if( FAILED( pDSB->Lock( 0, pDataHeader->dataSize, (void **)&dataPtr1, &count1,
            (void **)&dataPtr2, &count2, 0 )))
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error locking buffer for write\r\n")));
        return -1;
    }

    memcpy( dataPtr1, (&(pDataHeader->data)), count1 );

    if( FAILED( pDSB->Unlock( (void *)dataPtr1, count1, (void *)dataPtr2, count2 )))
    {
        DEBUGMSG(ZONE_ERROR,(TEXT("Error unlocking buffer after write\r\n")));
        return -1;
    }

    // Success
    return (0);
}


void CleanupDirectSound(void)
{
    if (pDSB)
    {
        pDSB->Release();
        pDSB = 0;
    }

    if (pDS)
    {
        pDS->Release();
        pDS = 0;
    }

    if (fileBuffer != 0)
    {
        LocalFree(fileBuffer);
        fileBuffer = 0;
    }
}


int LoopWaveFile (int nNumLoops)
{
    for (int nCount=0; nCount < nNumLoops; nCount++)
    {
        DEBUGMSG(ZONE_TEMP,(TEXT("Playing Loop #%02d of %d. [%s]  "),
                 nCount+1, nNumLoops, lpCmdLine));

        // Play
        if( FAILED( pDSB->Play( 0, 0, 0 )))
        {
            DEBUGMSG(ZONE_ERROR,(TEXT("Error playing buffer\r\n")));
            return -1;
        }

        DEBUGMSG(ZONE_TEMP,(TEXT("Waiting for completion...\r\n")));

        // Wait for termination
        do
        {
            //DEBUGMSG(ZONE_TEMP,(TEXT("Wait for termination...\n\r")));

            Sleep(50);

            if( FAILED( pDSB->GetStatus( &dsStatus ) ) )
            {
                DEBUGMSG(ZONE_ERROR,(TEXT("Error checking buffer status\r\n")));

                // Failure
                return -1;
            }

        } while( dsStatus & DSBSTATUS_PLAYING );

    }

    // Success
    return (0);
}

