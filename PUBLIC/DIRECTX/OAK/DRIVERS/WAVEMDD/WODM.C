// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1998  Microsoft Corporation
//  
// -----------------------------------------------------------------------------
//
// @doc     WDEV_EXT
//
// @module  wodm.c | Implements the WODM_XXX messages that are passed to the
//          wave audio driver via the <f WAV_IOControl> function.
//
// @xref    <t Wave Output Driver Messages>
//
// -----------------------------------------------------------------------------
#include <wavemdd.h>


// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @topic  Wave Output Driver Messages |
//          The wave output driver receives messages via the <f WAV_IOControl>
//          function.
//
//  @xref   <nl>
//          <m WODM_BREAKLOOP>            <nl>
//          <m WODM_CLOSE>                <nl>
//          <m WODM_GETDEVCAPS>           <nl>
//          <m WODM_GETNUMDEVS>           <nl>
//          <m WODM_GETPITCH>             <nl>
//          <m WODM_GETPLAYBACKRATE>      <nl>
//          <m WODM_GETPOS>               <nl>
//          <m WODM_GETVOLUME>            <nl>
//          <m WODM_OPEN>                 <nl>
//          <m WODM_PAUSE>                <nl>
//          <m WODM_PREPARE>              <nl>
//          <m WODM_RESET>                <nl>
//          <m WODM_RESTART>              <nl>
//          <m WODM_SETPITCH>             <nl>
//          <m WODM_SETPLAYBACKRATE>      <nl>
//          <m WODM_SETVOLUME>            <nl>
//          <m WODM_UNPREPARE>            <nl>
//          <m WODM_WRITE>                <nl>
//
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_BREAKLOOP |
//          The WODM_BREAKLOOP message requests a waveform output driver to
//          break an output loop that was created with a <m WODM_WRITE> message.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_BREAKLOOP
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Not used.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveOutBreakLoop> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_GETPITCH message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          The driver should stop output of the loop buffers at the end of 
//          the next loop iteration.
//
//          If the driver receives this message and a loop is not in progress, 
//          it should return MMSYSERR_NOERROR.
//  
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_BREAKLOOP(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WODM_BREAKLOOP");

    LOCK_GSI(WAPI_OUT);
    MarkAllAsNotInLoop(WAPI_OUT);
    UNLOCK_GSI(WAPI_OUT);

    FUNC_WMDD("-wdev_WODM_BREAKLOOP");
    return(MMSYSERR_NOERROR);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_GETPITCH |
//          The WODM_GETPITCH message requests a waveform output driver to
//          return the specified device's current pitch multiplier value.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_GETPITCH
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a DWORD location used to return the current pitch 
//          multiplier value. This is specified as a fixed-point value, where 
//          the high-order word of the DWORD contains the signed integer part 
//          of the number, and the low-order word contains the fractional 
//          part. The fraction consists of a WORD value, for which 0x8000 
//          represents one half, and 0x4000 represents one quarter. For 
//          example, the value 0x00010000 specifies a multiplier of 1.0 
//          (no pitch change), and a value of 0x000F8000 specifies a multiplier 
//          of 15.5.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveOutGetPitch> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_GETPITCH message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for the WODM_GETPITCH message by drivers is optional. 
//          If a driver supports the <m WODM_SETPITCH> message, it must 
//          also support <m WODM_GETPITCH>.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_GETPITCH(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WODM_GETPITCH");
    //
    // MDD Library does not support this function.
    //
    FUNC_WMDD("-wdev_WODM_GETPITCH");
    return(MMSYSERR_NOTSUPPORTED);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_GETPLAYBACKRATE |
//          The WODM_GETPLAYBACKRATE message requests a waveform output driver
//          to return the current playback rate multiplier value for the 
//          specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_GETPLAYBACKRATE
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a DWORD location used to return the current playback 
//          rate multiplier value. This is specified as a fixed-point value, 
//          where the high-order word of the DWORD contains the signed integer part 
//          of the number, and the low-order word contains the fractional 
//          part. The fraction consists of a WORD value, for which 0x8000 
//          represents one half, and 0x4000 represents one quarter. For 
//          example, the value 0x00010000 specifies a multiplier of 1.0 
//          (no pitch change), and a value of 0x000F8000 specifies a multiplier 
//          of 15.5.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveOutGetPlaybackRate> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_GETPLAYBACKRATE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for the WODM_GETPLAYBACKRATE message by drivers 
//          is optional. If a driver supports the <m WODM_SETPLAYBACKRATE> message, 
//          it must also support <m WODM_GETPLAYBACKRATE>.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_GETPLAYBACKRATE(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WODM_GETPLAYBACKRATE");
    //
    // MDD Library does not support this function.
    //
    FUNC_WMDD("-wdev_WODM_GETPLAYBACKRATE");
    return(MMSYSERR_NOTSUPPORTED);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_GETVOLUME |
//          The WODM_GETVOLUME message requests a waveform output driver to
//          return the current volume level setting for the specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_GETVOLUME
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a DWORD location to receive the volume setting.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveOutGetVolume> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_GETVOLUME message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for this message by drivers is optional. If 
//          the driver supports <m WODM_SETVOLUME>, it must support 
//          <m WODM_GETVOLUME>.
//
//          A value of zero is silence, and a value of 0xFFFF is full volume.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_GETVOLUME(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WMDD("+wdev_WODM_GETVOLUME");

    mmRet = PDD_WaveProc(WAPI_OUT, WPDM_GETVOLUME, dwParam1, 0);

    FUNC_WMDD("-wdev_WODM_GETVOLUME");

    return(mmRet);
}    





// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_PAUSE |
//          The WODM_PAUSE message requests a waveform output driver to
//          pause playback of a waveform.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_PAUSE
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Not used.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutPause>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_PAUSE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          The driver should stop playing the waveform and should save the 
//          current position. Playback should continue from this position 
//          when a <m WODM_RESTART> message is received. Output buffers received 
//          with the <m WODM_WRITE> message while playback is paused should be 
//          placed in the output queue.
//
//          If the driver receives this message while output is already 
//          paused, it should return MMSYSERR_NOERROR.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_PAUSE(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WMDD("+wdev_WODM_PAUSE");

    LOCK_GSI(WAPI_OUT);
    gsi[WAPI_OUT].bPaused = TRUE;
    if(gsi[WAPI_OUT].bStarted){
        mmRet = PDD_WaveProc(WAPI_OUT, WPDM_PAUSE, 0, 0);
        gsi[WAPI_OUT].bStarted = FALSE;
    }
    UNLOCK_GSI(WAPI_OUT);

    FUNC_WMDD("-wdev_WODM_PAUSE");
    return(mmRet);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_RESET |
//          The WODM_RESET message requests a waveform output driver to
//          stop sending output data and return all output buffers to the client.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_RESET
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Not used.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutReset>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_RESET message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          If the driver's output queue contains any output buffers (see 
//          <m WODM_WRITE>) whose contents have not been played,
//          the driver should set the WHDR_DONE flag and clear the 
//          WDR_INQUEUE flag in each buffer's WAVEHDR structure. The driver 
//          should then send the Wave API Manager a <m WOM_DONE> callback 
//          message for each buffer.
//
//          The driver should reset its position count to zero. If playback 
//          is paused, the driver should also take itself out of the paused 
//          state.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_RESET(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WODM_RESET");

    LOCK_GSI(WAPI_OUT);
    
    try {
        //
        // All queued blocks should be marked as DONE.
        //
        MarkAllAsDone(WAPI_OUT);
        MarkAllAsNotInLoop(WAPI_OUT);
        //
        // Stop playback immediately.
        //
        PDD_WaveProc(WAPI_OUT, WPDM_STOP, 0, 0);
        gsi[WAPI_OUT].bStarted=FALSE;
        gsi[WAPI_OUT].bInMiddle=FALSE;
        //
        // Do all the callbacks to clean up.
        //
        RemoveCompleteBlocks(WAPI_OUT);

        gsi[WAPI_OUT].dwBytePosition = 0;
    
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ERRMSG("Error in WAVEDEV Reset: initializing...");
        InitGSI(WAPI_OUT);
    }
    
    UNLOCK_GSI(WAPI_OUT);

    FUNC_WMDD("-wdev_WODM_RESET");
    return(MMSYSERR_NOERROR);
}    




// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_RESTART |
//          The WODM_RESTART message requests a waveform output driver to
//          continue playback of a waveform after playback has been paused 
//          with the <m WODM_PAUSE> message.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_RESTART
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Not used.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutRestart>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_RESTART message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Data output should resume from the position that was saved when 
//          the <m WODM_PAUSE> message was received.
//
//          If the driver receives a <m WODM_RESTART> message and output is not 
//          in a paused state, it should to nothing except return 
//          MMSYSERR_NOERROR.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_RESTART(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WODM_RESTART");

    LOCK_GSI(WAPI_OUT);
    
    if (gsi[WAPI_OUT].bPaused && (NULL != gsi[WAPI_OUT].pwh)) {
        
        if (gsi[WAPI_OUT].bInMiddle) {
            PDD_WaveProc(WAPI_OUT, WPDM_RESTART, (DWORD) gsi[WAPI_OUT].pwh, 0);
        } else {
            PDD_WaveProc(WAPI_OUT, WPDM_START,   (DWORD) gsi[WAPI_OUT].pwh, 0);
        }
        
        gsi[WAPI_OUT].bStarted = TRUE;
        gsi[WAPI_OUT].bInMiddle = TRUE;
    }
    gsi[WAPI_OUT].bPaused = FALSE;

    UNLOCK_GSI(WAPI_OUT);

    FUNC_WMDD("-wdev_WODM_RESTART");
    return(MMSYSERR_NOERROR);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_SETPITCH |
//          The WODM_SETPITCH message requests a waveform output driver to
//          set the specified device's pitch multiplier value.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_SETPITCH
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          A DWORD containing the pitch multiplier value. This is specified 
//          as a fixed-point value, where the high-order word of the DWORD 
//          contains the signed integer part of the number, and the low-order 
//          word contains the fractional part. The fraction consists of a 
//          WORD value, for which 0x8000 represents one half, and 0x4000 
//          represents one quarter. For example, the value 0x00010000 
//          specifies a multiplier of 1.0 (no pitch change), and a value of 
//          0x000F8000 specifies a multiplier of 15.5.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutSetPitch>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_SETPITCH message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for the WODM_SETPITCH message by drivers is 
//          optional. If a driver supports the WODM_SETPITCH message, it must 
//          also support <m WODM_GETPITCH>. Additionally, in response to a 
//          <m WODM_GETDEVCAPS> message, it must set WAVECAPS_PITCH in the 
//          dwSupport member of the WAVEOUTCAPS structure.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_SETPITCH(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WODM_SETPITCH");
    //
    // MDD Library does not support this function.
    //
    FUNC_WMDD("-wdev_WODM_SETPITCH");
    return(MMSYSERR_NOTSUPPORTED);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_SETPLAYBACKRATE |
//          The WODM_SETPLAYBACKRATE message requests a waveform output driver to
//          set the playback rate multiplier value for the specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_SETPLAYBACKRATE
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          A DWORD containing the playback rate multiplier value. This is 
//          specified as a fixed-point value, where the high-order word of 
//          the DWORD contains the signed integer part of the number, and the 
//          low-order word contains the fractional part. The fraction 
//          consists of a WORD value, for which 0x8000 represents one half, 
//          and 0x4000 represents one quarter. For example, the value 
//          0x00010000 specifies a multiplier of 1.0 (no rate change), and a 
//          value of 0x000F8000 specifies a multiplier of 15.5.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutSetPlaybackRate>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_SETPLAYBACKRATE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for the WODM_SETPLAYBACKRATE message by drivers 
//          is optional. If a driver supports the WODM_SETPLAYBACKRATE 
//          message, it must also support <m WODM_GETPLAYBACKRATE>.  
//          Additionally, in response to a <m WODM_GETDEVCAPS> message, it must 
//          set WAVECAPS_PLAYBACKRATE in the dwSupport member of the 
//          WAVEOUTCAPS structure.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_SETPLAYBACKRATE(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_WODM_SETPLAYBACKRATE");
    //
    // MDD Library does not support this function.
    //
    FUNC_WMDD("-wdev_WODM_SETPLAYBACKRATE");
    return(MMSYSERR_NOTSUPPORTED);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_SETVOLUME |
//          The WODM_SETVOLUME message requests a waveform output driver to
//          set the volume level for the specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_SETVOLUME
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          DWORD containing the volume setting.
//
//  @parm   DWORD | dwParam2 |
//          Not used.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutSetVolume>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_SETVOLUME message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for this message by drivers is optional. If the 
//          driver supports WODM_SETVOLUME, it must support <m WODM_GETVOLUME>.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_SETVOLUME(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WMDD("+wdev_WODM_SETVOLUME");

    mmRet = PDD_WaveProc(WAPI_OUT, WPDM_SETVOLUME, dwParam1, 0);

    FUNC_WMDD("-wdev_WODM_SETVOLUME");

    return(mmRet);
}    




// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_WRITE |
//          The WODM_WRITE message requests a waveform output driver to
//          write a waveform data block to the specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_WRITE
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a WAVEHDR structure identifying the buffer.
//
//  @parm   DWORD | dwParam2 |
//          Size of the WAVEHDR structure.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutWrite>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_WRITE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          If the WHDR_PREPARED flag in the dwFlags member of WAVEHDR is not 
//          set, the driver should return WAVERR_UNPREPARED.
//
//          Unless the device has been paused with a WODM_PAUSE message, the 
//          driver should begin playback the first time it receives a 
//          WODM_WRITE message.
//
//          Waveform output drivers should handle output asynchronously, 
//          by creating a separate thread to handle communication with the 
//          hardware. Typically, the original thread queues the output buffer, 
//          sets its WHDR_INQUEUE flag and clears its WHDR_DONE flag in 
//          the WAVEHDR structure, and returns control to the client.
//
//          The driver should continue sending buffers to the audio circuit
//          until the client sends <m WODM_PAUSE> or <m WIDM_RESET>.
//
//          The driver is usually responsible for implementing 
//          waveform looping. The driver should check each buffer's WAVEHDR 
//          structure for WHDR_BEGINLOOP and WHDR_ENDLOOP flags, along with 
//          an iteration count in the structure's dwLoops member.
//
// -----------------------------------------------------------------------------
DWORD
wdev_WODM_WRITE(
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    PWAVEHDR phdr = (PWAVEHDR) dwParam1;

    FUNC_WMDD("+wdev_WODM_WRITE");

    wdev_COMMON_ADDBUFFER(WAPI_OUT, uDeviceId, dwUser, dwParam1, dwParam2);

    //  if it has not start playing, start it
    LOCK_GSI(WAPI_OUT);
    if(!gsi[WAPI_OUT].bStarted && !gsi[WAPI_OUT].bPaused){
        PDD_WaveProc(WAPI_OUT, WPDM_START, (DWORD) gsi[WAPI_OUT].pwh, 0);
        gsi[WAPI_OUT].bStarted = TRUE;
        gsi[WAPI_OUT].bInMiddle = TRUE;
    }
    UNLOCK_GSI(WAPI_OUT);

    FUNC_WMDD("-wdev_WODM_WRITE");
    return MMSYSERR_NOERROR;
}



