// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-2000  Microsoft Corporation
//  
// -----------------------------------------------------------------------------
//
//  Module Name:  
//  
//      wavemdd.c
//  
//  Abstract:  
//  
//  Functions:
//  
//  Notes:
//  
// -----------------------------------------------------------------------------
#include <wavemdd.h>

BOOL fPowerUp;
// Debug Zones.
//

// Global
ULONG WMDD_InterruptThread(VOID);

HANDLE hAudioInterrupt;          // Handle to Audio Interrupt event.
HANDLE hAudioInterruptThread;    // Handle to thread which waits on an audio 
                                 // interrupt event.
CRITICAL_SECTION v_GSICritSect[2]; // One for input, one for output


STREAM_INFO gsi[2];  // Global Stream Info : one for input and one for output

BOOL    g_bDriverOpened;
BOOL    g_bDriverInitialized;   //  whether it has been inited or not


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL __stdcall
dllentry (
    HANDLE  hinstDLL,
    DWORD   Op,
    LPVOID  lpvReserved
    )
{

//    HANDLE hCoreDll;

    switch (Op) {

        case DLL_PROCESS_ATTACH :

// This check is lame.  We may very well want to have wavedev drivers
// without the waveOut API's being supported, especially now that
// we have DirectSound working "in emulation mode" over the wavedev
// driver.
//
//            hCoreDll = LoadLibrary(TEXT("coredll.dll"));
//
//            if (hCoreDll == NULL)  {
//                //
//                // ERROR! This is VERY bad if we can't LoadLibrary on COREDLL.DLL!!!
//                //
//                return(FALSE);
//            }
//
//            if (!GetProcAddress (hCoreDll, TEXT("waveOutOpen"))) {
//                //
//                // The wave functions have not been exported by coredll, so
//                // we must not continue loading this driver.
//                //
//                FreeLibrary(hCoreDll);
//                return(FALSE);
//            }
//
//            FreeLibrary(hCoreDll);


            DEBUGREGISTER((HINSTANCE)hinstDLL);
            break;

        case DLL_PROCESS_DETACH :
            break;
            
        case DLL_THREAD_DETACH :
            break;
            
        case DLL_THREAD_ATTACH :
            break;
            
        default :
            break;
    }
    return TRUE;
}



// -----------------------------------------------------------------------------
//                      RemoveCompleteBlocks
// -----------------------------------------------------------------------------
VOID
RemoveCompleteBlocks(
    WAPI_INOUT apidir
    )
{
    UINT uMsg = ((apidir == WAPI_OUT) ? MM_WOM_DONE : MM_WIM_DATA);
    DRVCALLBACK* pfnCallback = gsi[apidir].pfnCallback;
    HANDLE hWave = gsi[apidir].hWave;
    PWAVEHDR pwh = gsi[apidir].pwh;

    while (gsi[apidir].pwh != NULL) {

        if (IS_BUFFER_DONE(gsi[apidir].pwh))  {

            pwh = gsi[apidir].pwh;

            if (IS_BUFFER_INLOOP(pwh)) {
                //
                // This buffer is in a loop.
                //
                if (IS_BUFFER_BEGINLOOP(pwh)) {
                    if (pwh->dwLoops == 1) {
                        PWAVEHDR pwh_temp = pwh;
                        while (pwh_temp != NULL) {
                            MARK_BUFFER_NOT_INLOOP(pwh_temp);
                            pwh_temp = pwh_temp->lpNext;
                        }   
                    } else {
                        if (pwh->dwLoops != INFINITE)
                            pwh->dwLoops--;
                    }

                }

            }


            if (!IS_BUFFER_INLOOP(pwh)) {
                //
                // Only remove a block if it's not in a loop.
                //
                INTMSG("Block Complete... Sending callback");

                gsi[apidir].dwBytePosition += gsi[apidir].pwh->dwBytesRecorded;
                gsi[apidir].pwh = gsi[apidir].pwh->lpNext;
                gsi[apidir].pwhReal = gsi[apidir].pwh;
                MARK_BUFFER_DEQUEUED(pwh);
                //
                // Make the callback function call to the Wave API Manager
                //
                pfnCallback(hWave, uMsg, 0, (DWORD) pwh, 0);

            } else {
                //
                // If this buffer is in a loop, then just advance to the next block.
                //
                if (IS_BUFFER_ENDLOOP(pwh) || pwh->lpNext == NULL) {
                    gsi[apidir].pwh = gsi[apidir].pwhReal;   // go the beginning.
                } else {
                    gsi[apidir].dwBytePosition += gsi[apidir].pwh->dwBytesRecorded;
                    gsi[apidir].pwh = gsi[apidir].pwh->lpNext;
                }
                //
                // Since we're not actually removing anything, we don't want to loop.
                //
                MARK_BUFFER_NOT_DONE(pwh);
                MARK_BUFFER_EMPTY(pwh);
                break;
            }


        } else {
            //
            // Only remove from the front of the list, but as many as are done.
            //
            break;  // If the first one's not DONE, then break the loop.
        }

    }   
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
DWORD
GetBytePosition(
    WAPI_INOUT apidir
    )
{
    DWORD dwBytePositionTotal;    

    LOCK_GSI(apidir);
    if (gsi[apidir].pwh != NULL)
        dwBytePositionTotal = gsi[apidir].dwBytePosition + 
                              gsi[apidir].pwh->dwBytesRecorded;
    else
        dwBytePositionTotal = gsi[apidir].dwBytePosition;
    UNLOCK_GSI(apidir);

    return(dwBytePositionTotal);
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
MarkAllAsDone(
    WAPI_INOUT apidir
    )
{
    PWAVEHDR pwh_temp = gsi[apidir].pwh;

    while (pwh_temp != NULL) {
        MARK_BUFFER_DONE(pwh_temp);
        pwh_temp = pwh_temp->lpNext;
    }   
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
MarkAllAsNotInLoop(
    WAPI_INOUT apidir
    )
{
    PWAVEHDR pwh_temp = gsi[apidir].pwh;

    while (pwh_temp != NULL) {
        MARK_BUFFER_NOT_INLOOP(pwh_temp);
        pwh_temp = pwh_temp->lpNext;
    }   
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
MarkAllAsFull(
    WAPI_INOUT apidir
    )
{
    PWAVEHDR pwh_temp = gsi[apidir].pwh;

    while (pwh_temp != NULL) {
        MARK_BUFFER_FULL(pwh_temp);
        pwh_temp = pwh_temp->lpNext;
    }   
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
MarkFullAsDone(
    WAPI_INOUT apidir
    )
{
    PWAVEHDR pwh_temp = gsi[apidir].pwh;

    while (pwh_temp != NULL) {
        if (IS_BUFFER_FULL(pwh_temp))  {
            MARK_BUFFER_DONE(pwh_temp);
        }
        pwh_temp = pwh_temp->lpNext;
    }   
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
InitGSI(
    WAPI_INOUT apidir
    )
{
    FUNC_WMDD("+InitGSI");

    gsi[apidir].pwh = NULL;
    gsi[apidir].pwhReal = NULL;
    gsi[apidir].bInLoop = FALSE;
    gsi[apidir].bStarted = FALSE;
    gsi[apidir].bPaused = FALSE;
    gsi[apidir].bInMiddle = FALSE;
    gsi[apidir].pfnCallback = NULL;
    gsi[apidir].dwOpenFlags = 0;
    gsi[apidir].pwfx = NULL;
    gsi[apidir].hWave = INVALID_HANDLE_VALUE;

    FUNC_WMDD("-InitGSI");
}



// -----------------------------------------------------------------------------
//                      WMDD_InterruptThread
// -----------------------------------------------------------------------------
//  This is the interrupt thread which waits on an audio interrupt event.
// -----------------------------------------------------------------------------
ULONG 
WMDD_InterruptThread(
   VOID
   )
{
    AUDIO_STATE state;
    DWORD dwAccessKey;

    FUNC_WMDD("+WMDD_InterruptThread");

    while (TRUE)  {

        // certain MIPS platforms are set up for edge-trigered interrupts
        // this can lead to problems when both playback and capture channels
        // are firing interrupts simultaneously. In particular, if still have interrupts
        // pending AFTER we've called InterruptDone, there's a chance that the kernel
        // missed the interrupt and we'll never wake up again.
        // Therefore, we skip the wait and just go re-handle the outstanding interrupt
#ifdef MIPS
#define NO_NESTED_INTERRUPTS
#endif


#ifdef NO_NESTED_INTERRUPTS
        state = PDD_AudioGetInterruptType();
        if (state == AUDIO_STATE_IGNORE) {
            // no pending interrupts, so it's safe to go wait for the next one
            WaitForSingleObject(hAudioInterrupt, INFINITE);
        }
#else
        WaitForSingleObject(hAudioInterrupt, INFINITE);
#endif



        if (fPowerUp) {
            INTMSG("Audio MDD is recovering from Power On");
            fPowerUp = FALSE;
            InterruptDone(gIntrAudio);

            LOCK_GSI(WAPI_OUT);
            LOCK_GSI(WAPI_IN);

            dwAccessKey=GetCurrentPermissions();
            SetProcPermissions((ULONG) -1);

            MarkAllAsFull(WAPI_IN);
            MarkAllAsFull(WAPI_OUT);
            MarkAllAsNotInLoop(WAPI_OUT);
            MarkFullAsDone(WAPI_IN);
            MarkFullAsDone(WAPI_OUT);
            RemoveCompleteBlocks(WAPI_OUT);
            RemoveCompleteBlocks(WAPI_IN);

            SetProcPermissions(dwAccessKey);

            gsi[WAPI_OUT].bStarted=FALSE;

            UNLOCK_GSI(WAPI_IN);
            UNLOCK_GSI(WAPI_OUT);

            //
            // Go back to waiting for real interrupts
            //
            continue;
        }

        INTMSG("INT!");
#ifdef NO_NESTED_INTERRUPTS
        if (state == AUDIO_STATE_IGNORE) {
            state = PDD_AudioGetInterruptType(); 
        }
#else
        state = PDD_AudioGetInterruptType(); 
#endif
        InterruptDone(gIntrAudio);

        //  before we try to access the gsi[apidir].pwh, need to set access key
        dwAccessKey=GetCurrentPermissions();
        SetProcPermissions((ULONG) -1); //  access everybody

        //
        // Check for output state...
        //
        switch (state & AUDIO_STATE_OUT_MASK) {

            case AUDIO_STATE_OUT_UNDERFLOW:
                INTMSG("Audio Underflowed!");
                LOCK_GSI(WAPI_OUT);
                MarkFullAsDone(WAPI_OUT);
                RemoveCompleteBlocks(WAPI_OUT);
                if (gsi[WAPI_OUT].pwh != NULL)
                    PDD_WaveProc(WAPI_OUT, WPDM_START, (DWORD) gsi[WAPI_OUT].pwh, 0);
                UNLOCK_GSI(WAPI_OUT);
                break;

            case AUDIO_STATE_OUT_PLAYING:
               
                INTMSG("Audio Playing");

                LOCK_GSI(WAPI_OUT);
                MarkFullAsDone(WAPI_OUT);
                RemoveCompleteBlocks(WAPI_OUT);
                //
                // Send more data if there is more or else stop the audio.
                //
                if (gsi[WAPI_OUT].pwh != NULL) {
                    PDD_WaveProc(WAPI_OUT, WPDM_CONTINUE, (DWORD) gsi[WAPI_OUT].pwh, 0);
                } else {
                    INTMSG("No more data. List is NULL. Stopping audio.");
                    gsi[WAPI_OUT].bStarted=FALSE;
                    PDD_WaveProc(WAPI_OUT, WPDM_ENDOFDATA, 0, 0);
                }

                UNLOCK_GSI(WAPI_OUT);
                break;

            case AUDIO_STATE_OUT_STOPPED:
                INTMSG("Audio Stopped!");
                LOCK_GSI(WAPI_OUT);
                PDD_WaveProc(WAPI_OUT, WPDM_STANDBY, 0, 0);
                gsi[WAPI_OUT].bStarted=FALSE;
                UNLOCK_GSI(WAPI_OUT);
                INTMSG("End of Play");
                break;

            case AUDIO_STATE_IGNORE:
                INTMSG("No output state change. Do nothing.");
                break;

            default:
                ERRMSG("Invalid output interrupt type!");
                break;
        }

        //
        // Check for input state...
        //
        switch (state & AUDIO_STATE_IN_MASK) {

            case AUDIO_STATE_IN_OVERFLOW:
                INTMSG("Audio Overflowed!");
                LOCK_GSI(WAPI_IN);
                MarkFullAsDone(WAPI_IN);
                RemoveCompleteBlocks(WAPI_IN);
                PDD_WaveProc(WAPI_IN, WPDM_START, (DWORD) gsi[WAPI_IN].pwh, 0);

                UNLOCK_GSI(WAPI_IN);
                break;

            case AUDIO_STATE_IN_RECORDING:
               
                INTMSG("Audio Recording");
                LOCK_GSI(WAPI_IN);
                MarkFullAsDone(WAPI_IN);
                RemoveCompleteBlocks(WAPI_IN);
                //
                // Send more data if there is more or else stop the audio.
                //
                PDD_WaveProc(WAPI_IN, WPDM_CONTINUE, (DWORD) gsi[WAPI_IN].pwh, 0);

                UNLOCK_GSI(WAPI_IN);
                break;

            case AUDIO_STATE_IN_STOPPED:
                INTMSG("Audio Stopped!");

                LOCK_GSI(WAPI_IN);
                MarkFullAsDone(WAPI_IN);
                RemoveCompleteBlocks(WAPI_IN);

                PDD_WaveProc(WAPI_IN, WPDM_STANDBY, 0, 0);
                gsi[WAPI_IN].bStarted=FALSE;
                UNLOCK_GSI(WAPI_IN);

                INTMSG("End of Play");
                break;


            case AUDIO_STATE_IGNORE:
                INTMSG("No input state change. Do nothing.");
                break;

            default:
                ERRMSG("Invalid input interrupt type!");
                break;

        }  // switch

        //  change permission back
        SetProcPermissions(dwAccessKey);

    }  // forever while...

   return TRUE;
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID
WMDD_PowerHandler(
   BOOL power_down
   )
{
    if (!power_down) {
        fPowerUp = TRUE;
        SetInterruptEvent(gIntrAudio);
    }

    PDD_AudioPowerHandler(power_down);
}



// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL
WMDD_Deinit(
    DWORD dwData
    )
{
    DEBUGMSG (ZONE_FUNCTION, (TEXT("WVO_Deinit(0x%X)\r\n"), dwData));

    PDD_AudioDeinitialize();
  
    DeleteCriticalSection(&(v_GSICritSect[WAPI_IN]));
    DeleteCriticalSection(&(v_GSICritSect[WAPI_OUT]));

    if (hAudioInterrupt) {
        CloseHandle(hAudioInterrupt);
        hAudioInterrupt = NULL;
    }

    g_bDriverInitialized = FALSE;

    return TRUE;
}




// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
DWORD
WMDD_Init(
    DWORD Index
    )
{
    BOOL b;

    if(g_bDriverInitialized)
        return  10L;    //  already inited, we let it go anyway

    FUNC_WMDD("+WVO_Init");
    DECPARAM(Index);

    if(!(hAudioInterrupt = CreateEvent( NULL, FALSE, FALSE,NULL))) {
        ERRMSG("CreateEvent() FAILED");
        goto InitFail;
    }

    //  init the hardware first
    if (PDD_AudioInitialize(Index) != TRUE) {
        ERRMSG("PDD_AudioInitialize() FAILED");
        goto InitFail;
    }

    InitializeCriticalSection(&(v_GSICritSect[WAPI_IN]));
    InitializeCriticalSection(&(v_GSICritSect[WAPI_OUT]));

    hAudioInterruptThread  = CreateThread((LPSECURITY_ATTRIBUTES)NULL,
                                          0,
                                          (LPTHREAD_START_ROUTINE)WMDD_InterruptThread,
                                          NULL,
                                          0,
                                          NULL);
    if (!hAudioInterruptThread) {
        ERRMSG("hAudioInterruptThread Create FAILED");
        goto InitFail;
    }

    // 
    // Bump up the priority since the interrupt must be serviced
    // immediately. Both threads do a WaitForSingleObject when not
    // processing so changing priority does not affect overall 
    // performance. It merely gives the audio prompt response.
    //
    SetThreadPriority (hAudioInterruptThread, THREAD_PRIORITY_HIGHEST);

    b=InterruptInitialize(gIntrAudio, hAudioInterrupt, NULL, 0);
    InterruptDone(gIntrAudio);

    FUNC_WMDD("-WVO_Init (SUCCESS)");

    g_bDriverInitialized = TRUE; //  set it
    g_bDriverOpened = FALSE;

    // Return Non-Zero for success, will be passed to Deinit and Open
    return 1;

InitFail:
    FUNC_WMDD("-WVO_Init (FAILURE)");

    WMDD_Deinit(0);
    return  0L;
}

