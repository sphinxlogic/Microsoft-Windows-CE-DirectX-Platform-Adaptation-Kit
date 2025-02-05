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
//  @doc    WDEV_EXT
//
//  @module common.c | Implements the WODM_XXX and WIDM_XXX messages that are 
//          passed to the wave audio driver via the <f WAV_IOControl> function.
//          This module contains code that is common or very similar between
//          input and output functions.
//
//  @xref   <t Wave Input Driver Messages> (WIDM_XXX) <nl>
//          <t Wave Output Driver Messages> (WODM_XXX)
//
// -----------------------------------------------------------------------------
#include <wavemdd.h>


// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_ADDBUFFER |
//          The WIDM_ADDBUFFER requests a waveform input driver to 
//          add an empty input buffer to its input buffer queue.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_ADDBUFFER
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
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInAddBuffer> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_ADDBUFFER message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          If the WHDR_PREPARED flag is not set in the dwFlags member of the 
//          WAVEHDR structure, the driver should return WAVERR_UNPREPARED. If 
//          the flag is set, the driver should:<nl>
//          <tab>• Clear the WHDR_DONE flag <nl>
//          <tab>• Set the WHDR_INQUEUE flag <nl>
//          <tab>• Place the empty buffer in its input queue <nl>
//          <tab>• Return with a value of MMSYSERR_NOERROR.
// 
//          The driver starts recording when it receives a <m WIDM_START> message.
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_ADDBUFFER(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    PWAVEHDR phdr = (PWAVEHDR) dwParam1;
    PWAVEHDR p;

    FUNC_WMDD("+wdev_COMMON_ADDBUFFER");

    //  set some default value
    //  assume it is only one node
    phdr->lpNext = NULL;
    phdr->dwFlags &= ~WHDR_DONE;
    phdr->dwBytesRecorded=0;
    phdr->reserved = (DWORD) FALSE;  // mark as not a looped buffer by default.

    LOCK_GSI(apidir);
    //  add it to the end
    if(!gsi[apidir].pwhReal) {
        gsi[apidir].pwhReal = phdr;
        gsi[apidir].pwh = phdr;
    } else {
        //  add the buffer to the tail
        p = gsi[apidir].pwhReal;
        while(p->lpNext)
            p = p->lpNext;
        p->lpNext = phdr;
    }

    //
    // Check the looping status.
    //
    if (IS_BUFFER_BEGINLOOP(phdr))  {
        //
        // BEGINLOOP is set.
        //        
        if (IS_BUFFER_ENDLOOP(phdr))  {
            //
            // Only this buffer is looped or finishing up a loop... same result.
            //
            gsi[apidir].bInLoop = FALSE;
        } else {
            //
            // Beginning of a loop. or invalid BEGINLOOP... same result.
            //
            gsi[apidir].bInLoop = TRUE;
        }
        MARK_BUFFER_INLOOP(phdr);

    } else  {
        //
        // BEGINLOOP is not set.
        //        
        if (gsi[apidir].bInLoop) {
            //
            // If we were looking for an ENDLOOP, close things off, else ignore.
            //
            if (IS_BUFFER_ENDLOOP(phdr))  {
                gsi[apidir].bInLoop = FALSE;
            }
            MARK_BUFFER_INLOOP(phdr);
        }
    }

    MARK_BUFFER_QUEUED(phdr);
    UNLOCK_GSI(apidir);

    FUNC_WMDD("-wdev_COMMON_ADDBUFFER");
    return(MMSYSERR_NOERROR);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_CLOSE |
//          The WIDM_CLOSE message requests a waveform input driver to close 
//          a specified device instance that was previously opened with a 
//          <m WIDM_OPEN> message.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_CLOSE
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
//          codes defined in mmsystem.h. See <f waveInClose> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_CLOSE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          If the driver has not filled and returned all of the buffers 
//          received with <f WIDM_ADDBUFFER> messages, it should not close the 
//          instance and should instead return WAVERR_STILLPLAYING.
//
//          After the driver closes the device instance it should send a 
//          <m WIM_CLOSE> callback message to the Wave API Manager.
//
// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_CLOSE |
//          The WODM_CLOSE message requests a waveform output driver to
//          close a specified device instance that was previously opened 
//          with a <m WODM_OPEN> message.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_CLOSE
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
//          codes defined in mmsystem.h. See <f waveOutClose> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_CLOSE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          If the client has passed data buffers to the driver by 
//          means of <m WODM_WRITE> messages, and if the driver hasn't 
//          finished sending the data to the PDD, the  
//          driver should return WAVERR_STILLPLAYING in response to WODM_CLOSE.
//
//          After the driver closes the device instance it should send a 
//          <m WOM_CLOSE> callback message to the Wave API Manager.
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_CLOSE(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;
    UINT msg = ((apidir == WAPI_OUT) ? MM_WOM_CLOSE : MM_WIM_CLOSE);
    HANDLE hWaveCallback = gsi[apidir].hWave;

    FUNC_WMDD("+wdev_COMMON_CLOSE");

    LOCK_GSI(apidir);

    if (gsi[apidir].pwh != NULL)  {
        //
        // We still seem to have a queue of buffers that haven't been returned
        // to their owner as DONE yet. 
        //
        mmRet = WAVERR_STILLPLAYING;
        goto EXIT;    
    }

    mmRet = PDD_WaveProc (apidir, WPDM_CLOSE, 0, 0);

    InitGSI(apidir);

EXIT:
    UNLOCK_GSI(apidir);

    FUNC_WMDD("-wdev_COMMON_CLOSE");
    return(mmRet);
}   


// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_GETDEVCAPS |
//          The WIDM_GETDEVCAPS message requests a waveform input driver to 
//          return the capabilities of a specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_GETDEVCAPS
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a WAVEINCAPS structure, which is described in the Win32 SDK.
//
//  @parm   DWORD | dwParam2 |
//          Size of the WAVEINCAPS structure in bytes. 
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInGetDevCaps> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_GETDEVCAPS message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_GETDEVCAPS |
//          The WODM_GETDEVCAPS message requests a waveform output driver to
//          return the capabilities of a specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_GETDEVCAPS
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a WAVEOUTCAPS structure, which is described in the Win32 SDK.
//
//  @parm   DWORD | dwParam2 |
//          Size of the WAVEOUTCAPS structure in bytes. 
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveOutGetDevCaps> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_GETDEVCAPS message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_GETDEVCAPS(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WMDD("+wdev_COMMON_GETDEVCAPS");

    mmRet = PDD_WaveProc (apidir, WPDM_GETDEVCAPS, dwParam1, dwParam2);

    FUNC_WMDD("-wdev_COMMON_GETDEVCAPS");
    return(mmRet);
}    


// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_GETNUMDEVS |
//          The WIDM_GETNUMDEVS message requests a waveform input driver to 
//          return the number of device instances that it supports.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_GETNUMDEVS
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
//  @rdesc  The driver returns the number of waveform input device instances 
//          it supports. For WindowsCE, the default MDD returns 1.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_GETNUMDEVS message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_GETNUMDEVS |
//          The WODM_GETNUMDEVS message requests a waveform output driver to
//          return the number of device instances that it supports.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_GETNUMDEVS
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
//  @rdesc  The driver returns the number of waveform output device instances 
//          it supports. For WindowsCE, the default MDD returns 1.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_GETNUMDEVS message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_GETNUMDEVS(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;

    FUNC_WMDD("+wdev_WODM_GETNUMDEVS");

    mmRet = PDD_WaveProc(apidir, WPDM_GETDEVCAPS, (DWORD) NULL, 0);

    FUNC_WMDD("-wdev_WODM_GETNUMDEVS");

    if (mmRet == MMSYSERR_NOERROR)
        return(1);
    else
        return(0);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_GETPOS |
//          The WIDM_GETPOS message requests a waveform input driver to 
//          return the current input position within a waveform. The input 
//          position is relative to the first recorded sample of the waveform.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_GETPOS
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to an MMTIME structure, which is described in the Win32 SDK.
//
//  @parm   DWORD | dwParam2 |
//          Size of the MMTIME structure in bytes.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInGetPosition> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_GETPOS message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          The wType member of the MMTIME structure indicates the time 
//          format requested by the client. If the driver cannot support the 
//          requested format, it should return the position in a format that 
//          it does support, and change the wType member accordingly.
//
//          The position should be reset to zero when the driver receives 
//          a <m WODM_OPEN> or <m WODM_RESET> message.
//
// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_GETPOS |
//          The WODM_GETPOS message requests a waveform output driver to
//          return the current position within a waveform. The position 
//          is relative to the beginning of the waveform.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_GETPOS
//
//  @parm   DWORD | dwUser |
//          Device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to an MMTIME structure, which is described in the Win32 SDK.
//
//  @parm   DWORD | dwParam2 |
//          Size of the MMTIME structure in bytes.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveOutGetPosition> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_GETPOS message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          The wType member of the MMTIME structure indicates the time 
//          format requested by the client. If the driver cannot support the 
//          requested format, it should return the position in a format that 
//          it does support, and change the wType member accordingly.
//
//          The position should be reset to zero when the driver receives 
//          a <m WODM_OPEN> or <m WODM_RESET> message.
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_GETPOS(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    DWORD    dwBytePosition;
    MMRESULT mmRet = MMSYSERR_NOERROR;
    PMMTIME  pmmt = (PMMTIME) dwParam1;

    FUNC_WMDD("+wdev_COMMON_GETPOS");

    if (IsBadWritePtr((LPVOID)pmmt, sizeof(MMTIME))) {
        PRINTMSG(ZONE_WARN, (TEXT("WARNING : wdev_COMMON_GETPOS was passed a bad pointer (0x%X)\r\n"), pmmt));
        mmRet = MMSYSERR_INVALPARAM;
    } else {
        
        dwBytePosition = GetBytePosition(apidir);

        switch (pmmt->wType) {

            case TIME_BYTES:
                pmmt->u.cb = dwBytePosition;
                break;

            case TIME_SAMPLES:
                pmmt->u.sample = (dwBytePosition * 8) / 
                    (gsi[apidir].pwfx->nChannels * gsi[apidir].pwfx->wBitsPerSample);
                break;

            case TIME_MS:
                if (gsi[apidir].pwfx->nAvgBytesPerSec != 0)
                    pmmt->u.ms = (dwBytePosition * 1000) / gsi[apidir].pwfx->nAvgBytesPerSec;
                else  {
                    pmmt->wType = TIME_BYTES;
                    pmmt->u.cb = dwBytePosition;
                }
                break;

            case TIME_MIDI:
            case TIME_TICKS:
            case TIME_SMPTE:
                //
                // We don't support these, so return TIME_BYTES instead.
                //
                pmmt->wType = TIME_BYTES;
                pmmt->u.cb = dwBytePosition;
                break;
        }
    }

    FUNC_WMDD("-wdev_COMMON_GETPOS");
    return(mmRet);
}    



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_OPEN |
//          The WIDM_OPEN message requests a waveform input driver to open 
//          an instance of a specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_OPEN
//
//  @parm   DWORD | dwUser |
//          Pointer to location to receive the device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a <t WAVEOPENDESC> structure, containing the
//          client's device handle, notification target, and instance ID.
//
//  @parm   DWORD | dwParam2 |
//          Contains flags. THe following flags are defined:
//
//  @flag   WAVE_FORMAT_DIRECT | Data compression/decompression should take
//          place in hardware. See Comments section below.
//
//  @flag   WAVE_FORMAT_QUERY | The driver should indicate whether or not 
//          it supports the specified format. See Comments.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds. 
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInOpen> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_OPEN message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          The driver assigns a device instance identifier and returns it in 
//          the location pointed to by dwUser. The driver can expect to 
//          receive this value as the dwUser input argument to all other 
//          <t Wave Input Driver Messages>.
//
//          The driver determines the number of clients it allows to use a 
//          particular device. If a device is opened by the maximum number of 
//          clients, it returns MMSYSERR_ALLOCATED for subsequent open 
//          requests.
//
//          The <p WAVE_FORMAT_DIRECT> flag is meant for use with a wave mapper. 
//          If the flag is set in dwParam2, the driver should not call the 
//          Audio Compression Manager to handle compression/decompression 
//          operations; the caller wants the hardware to perform these 
//          operations directly. If the hardware is not capable of performing 
//          compression/decompression operations, the driver should return 
//          MMSYSERR_NOTSUPPORTED when WAVE_FORMAT_DIRECT is set.
//
//          If the <p WAVE_FORMAT_QUERY> flag is set in dwParam2, the driver 
//          should not open the device, but should instead determine whether 
//          it supports the format specified by the <t WAVEOPENDESC> structure's 
//          lpFormat member. If the driver supports the requested format, it 
//          should return MMSYSERR_NOERROR. Otherwise it should return 
//          WAVERR_BADFORMAT.
//
//          If the open operation succeeds, the driver should send the 
//          Wave API Manager a <m WIM_OPEN> message by calling the 
//          PostThreadMessage function on the callback thread handle.
//
// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_OPEN |
//          The WODM_OPEN message requests a waveform output driver to open 
//          an instance of a specified device.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_OPEN
//
//  @parm   DWORD | dwUser |
//          Pointer to location to receive the device instance identifier.
//
//  @parm   DWORD | dwParam1 |
//          Pointer to a <t WAVEOPENDESC> structure, containing the
//          client's device handle, notification target, and instance ID.
//
//  @parm   DWORD | dwParam2 |
//          Contains flags. THe following flags are defined:
//
//  @flag   WAVE_FORMAT_DIRECT | Data compression/decompression should take
//          place in hardware. See Comments section below.
//
//  @flag   WAVE_FORMAT_QUERY | The driver should indicate whether or not 
//          it supports the specified format. See Comments.
//
//  @rdesc  The driver should return MMSYSERR_NOERROR if the operation succeeds.
//          Otherwise it should return one of the MMSYSERR or WAVEERR error
//          codes defined in mmsystem.h. See <f waveOutOpen>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_OPEN message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          The driver assigns a device instance identifier and returns it in 
//          the location pointed to by dwUser. The driver can expect to 
//          receive this value as the dwUser input argument to all other 
//          <t Wave Output Driver Messages>.
//
//          The driver determines the number of clients it allows to use a 
//          particular device. If a device is opened by the maximum number of 
//          clients, it returns MMSYSERR_ALLOCATED for subsequent open 
//          requests.
//
//          The <p WAVE_FORMAT_DIRECT> flag is meant for use with a wave mapper. 
//          If the flag is set in dwParam2, the driver should not call the 
//          Audio Compression Manager to handle compression/decompression 
//          operations; the caller wants the hardware to perform these 
//          operations directly. If the hardware is not capable of performing 
//          compression/decompression operations, the driver should return 
//          MMSYSERR_NOTSUPPORTED when <p WAVE_FORMAT_DIRECT> is set.
//
//          If the <p WAVE_FORMAT_QUERY> flag is set in dwParam2, the driver 
//          should not open the device, but should instead determine whether 
//          it supports the format specified by the <t WAVEOPENDESC> structure's 
//          lpFormat member. If the driver supports the requested format, it 
//          should return MMSYSERR_NOERROR. Otherwise it should return 
//          WAVERR_BADFORMAT.
//
//          If the open operation succeeds, the driver should send the 
//          Wave API Manager a <m WOM_OPEN> message by calling the 
//          PostThreadMessage function on the callback thread handle.
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_OPEN(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    MMRESULT mmRet = MMSYSERR_NOERROR;
    LPWAVEOPENDESC lpWOD = (LPWAVEOPENDESC) dwParam1;

    FUNC_WMDD("+wdev_COMMON_OPEN");

    if (WAVE_FORMAT_QUERY & dwParam2)  {
        //
        // If we just wanted to check formats, return NOERROR here.
        //
        mmRet = PDD_WaveProc (apidir, WPDM_OPEN, (DWORD) lpWOD->lpFormat, (DWORD) TRUE);

        goto EXIT;
    }

    //
    // Tell the PDD that this OPEN request is for real. The PDD can choose
    // to fail (ex. if it can't do simultaneous play/record). Would return
    // MMSYSERR_ALLOCATED.
    //
    mmRet = PDD_WaveProc (apidir, WPDM_OPEN, (DWORD) lpWOD->lpFormat, (DWORD) FALSE);

    if (mmRet != MMSYSERR_NOERROR) {
        goto EXIT;
    }

    //
    // All systems are GO! Store some globals.
    //
    LOCK_GSI(apidir);
    gsi[apidir].pwfx = (LPWAVEFORMATEX) lpWOD->lpFormat;
    gsi[apidir].pfnCallback = (HANDLE) lpWOD->dwCallback;
    gsi[apidir].hWave = lpWOD->hWave;
    gsi[apidir].dwOpenFlags = dwParam2;
    gsi[apidir].pwh = NULL;
    gsi[apidir].pwhReal = NULL;
    gsi[apidir].bInLoop = FALSE;
    gsi[apidir].bStarted = FALSE;
    gsi[apidir].dwBytePosition = 0;
    UNLOCK_GSI(apidir);

EXIT:
    FUNC_WMDD("-wdev_COMMON_OPEN");
    return(mmRet);
}   



// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_PREPARE |
//          The WIDM_PREPARE message requests a waveform input driver to
//          prepare a system-exclusive data buffer for input. 
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_PREPARE
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
//          codes defined in mmsystem.h. See <f waveInPrepareHeader>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_PREPARE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for this message by drivers is optional. If the 
//          driver supports WIDM_PREPARE, it must also support <m WIDM_UNPREPARE>.
//
//          If the driver returns MMSYSERR_NOTSUPPORTED, waveapi.dll prepares 
//          the buffer for use. For most drivers, this behavior is 
//          sufficient. If the driver does perform buffer preparation, it 
//          must set WHDR_PREPARED in the dwFlags member of WAVEHDR and 
//          return MMSYSERR_NOERROR.
//
// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_PREPARE |
//          The WODM_PREPARE message requests a waveform output driver to
//          prepare a system-exclusive data buffer for output. 
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_PREPARE
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
//          codes defined in mmsystem.h. See <f waveOutPrepareHeader>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_PREPARE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for this message by drivers is optional. If the 
//          driver supports WODM_PREPARE, it must also support <m WODM_UNPREPARE>.
//
//          If the driver returns MMSYSERR_NOTSUPPORTED, waveapi.dll prepares 
//          the buffer for use. For most drivers, this behavior is 
//          sufficient. If the driver does perform buffer preparation, it 
//          must set WHDR_PREPARED in the dwFlags member of WAVEHDR and 
//          return MMSYSERR_NOERROR.
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_PREPARE(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_COMMON_PREPARE");
    //
    // All the prep/unprep is handled in the Wave API Manager. Drivers that
    // replace this MDD library may choose to execute some code here.
    //
    FUNC_WMDD("-wdev_COMMON_PREPARE");
    return(MMSYSERR_NOTSUPPORTED);
}





// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WIDM_UNPREPARE |
//          The WIDM_UNPREPARE message requests a waveform input driver to 
//          remove the buffer preparation that was performed in response to a 
//          <m WIDM_PREPARE> message.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WIDM_UNPREPARE
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
//          Otherwise it should return one of the MMSYSERR or WAVERR error 
//          codes defined in mmsystem.h. See <f waveInUnprepareHeader> return values 
//          in the Win32 SDK.
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WIDM_UNPREPARE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for this message by user-mode drivers is optional. If the 
//          driver supports <m WIDM_PREPARE>, it must also support WIDM_UNPREPARE.
//
//          If the driver returns MMSYSERR_NOTSUPPORTED, waveapi.dll removes 
//          the buffer preparation. For most drivers, this behavior is 
//          sufficient. If the driver does support WODM_UNPREPARE, it must 
//          clear WHDR_PREPARED in the dwFlags member of WAVEHDR and return 
//          MMSYSERR_NOERROR.
//
// -----------------------------------------------------------------------------
//
//  @doc    WDEV_EXT
//
//  @msg    WODM_UNPREPARE |
//          The WODM_PREPARE message requests a waveform output driver to
//          remove the buffer preparation performed in response to a 
//          <m WODM_PREPARE> message.
//
//  @parm   UINT | uDeviceId |
//          Device identifier (0, 1, 2, and so on) for the target device.
//
//  @parm   UINT | uMsg |
//          WODM_UNPREPARE
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
//          codes defined in mmsystem.h. See <f waveOutUnprepareHeader>().
//
//  @comm   The Wave API Manager (WAVEAPI.DLL) sends the WODM_UNPREPARE message
//          by calling the audio driver's (WAVEDEV.DLL) <f WAV_IOControl>() entry
//          point via DeviceIoControl().
//
//          Support for this message by user-mode drivers is optional. If the 
//          driver supports <m WODM_PREPARE>, it must also support WODM_UNPREPARE.
//
//          If the driver returns MMSYSERR_NOTSUPPORTED, waveapi.dll removes 
//          the buffer preparation. For most drivers, this behavior is 
//          sufficient. If the driver does support WODM_UNPREPARE, it must 
//          clear WHDR_PREPARED in the dwFlags member of WAVEHDR and return 
//          MMSYSERR_NOERROR.
//
// -----------------------------------------------------------------------------
DWORD
wdev_COMMON_UNPREPARE(
    WAPI_INOUT apidir,
    UINT  uDeviceId,
    DWORD dwUser,
    DWORD dwParam1,
    DWORD dwParam2
    )
{
    FUNC_WMDD("+wdev_COMMON_UNPREPARE");
    //
    // All the prep/unprep is handled in the Wave API Manager. Drivers that
    // replace this MDD library may choose to execute some code here.
    //
    FUNC_WMDD("-wdev_COMMON_UNPREPARE");
    return(MMSYSERR_NOTSUPPORTED);
}    
