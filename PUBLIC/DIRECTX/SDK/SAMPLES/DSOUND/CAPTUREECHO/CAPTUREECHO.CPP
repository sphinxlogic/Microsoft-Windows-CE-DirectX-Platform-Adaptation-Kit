#include "main.h"
#include "WinApp.h"
#include "ComUty.h"
#include "DebugUty.h"
#include "DSoundUty.h"

namespace  
{
    // Class CEvent
    // ============
    // This class wraps an event handle to ameliorate event lifetime
    // management.  Since we want the class size to be _exactly_ the same 
    // as the HANDLE we'll set the struct alignment to 1 byte.
#   include <pshpack1.h>
    class CEvent
    {

    public:

        CEvent() 
            : m_hEvent(NULL) 
        {
        }

        ~CEvent() 
        { 
            Close();
        }

        bool Create()
        {
            m_hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
            return m_hEvent!=NULL;
        }

        void Close()
        {
            if (m_hEvent!=NULL) 
                CloseHandle(m_hEvent); 
            m_hEvent=NULL;
        }

        operator HANDLE() // by returning an r-value, we still protect the data member
        { 
            return m_hEvent; 
        }

    protected:
        HANDLE m_hEvent;

    };
#   include <poppack.h>
}

// class CCaptureEchoApplication 
// =============================
// This class represents the application and is self-explanatory.  
// See WinApp.h for info on CWindowsApplication
class CCaptureEchoApplication : public CWindowsApplication
{
public:
    CCaptureEchoApplication();
    virtual bool Initialize();
    virtual void Run();  
    virtual void Shutdown();
    virtual bool ProcessCommandLine();
private:
    CComPointer<IDirectSound> m_piDS;
    CComPointer<IDirectSoundCapture> m_piDSC;
    CComPointer<IDirectSoundBuffer> m_piDSBPrimary;
    DWORD m_dwTimeout;
    DWORD m_dwDelay; 

    void DisplayUsage();
};

int
WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmd,
    int nCmdShow )
{
	// Declare a single instance of the application.
	CCaptureEchoApplication Application;

    return CWindowsApplication::WinMain(hInstance, 0, lpCmd, nCmdShow);
}

CCaptureEchoApplication::CCaptureEchoApplication()
: m_dwTimeout(30000), // 30 second default timeout
  m_dwDelay(2000) // 2 second delay
{
}

bool CCaptureEchoApplication::Initialize()
{
    HRESULT hr;
    const DSBUFFERDESC dsbdPrimary = { sizeof(DSBUFFERDESC), DSBCAPS_PRIMARYBUFFER, 0, 0, NULL };

    if (!CWindowsApplication::Initialize())
        return false;

    if (!ProcessCommandLine())
        return false;

    // Get the Direct Sound interface for the default Direct Sound object
    hr = DirectSoundCreate(NULL, m_piDS.AsOutParam(), NULL);
    CheckHRESULT(hr, "DirectSoundCreate", return false);

    // Set the cooperative level
    hr = m_piDS->SetCooperativeLevel(m_hWnd, DSSCL_NORMAL);
    CheckHRESULT(hr, "IDirectSound::SetCooperativeLevel(DSSCL_NORMAL)", return false);

    // Get the Direct Sound Capture interface for the default Direct Sound Capture object
    hr = DirectSoundCaptureCreate(NULL, m_piDSC.AsOutParam(), NULL);
    CheckHRESULT(hr, "DirectSoundCaptureCreate", return false);

    // Create the primary sound buffer
    hr = m_piDS->CreateSoundBuffer(&dsbdPrimary, m_piDSBPrimary.AsOutParam(), NULL);
    CheckHRESULT(hr, "IDirectSound::CreateSoundBuffer for the primary sound buffer", return false);

    // Play the primary sound buffer
    hr = m_piDSBPrimary->Play(0,0,DSBPLAY_LOOPING);
    CheckHRESULT(hr,"IDirectSoundBuffer::Play(Looping) on Primary Sound buffer", return false);

    return true;
}

void CCaptureEchoApplication::Shutdown()
{
    // Note: shutdown will be called even when Initialize fails

    // Release application global objects
    m_piDSBPrimary.ReleaseInterface();
    m_piDSC.ReleaseInterface();
    m_piDS.ReleaseInterface();

    CWindowsApplication::Shutdown();
}


void CCaptureEchoApplication::Run()
{
    const int nNumStreamingBlocks = 4;
    HRESULT hr;
    CEvent rhEvents[nNumStreamingBlocks];
    DSBPOSITIONNOTIFY rdsbpnNotifications[nNumStreamingBlocks];
    CComPointer<IDirectSoundBuffer> piDSB, piDSBPrimary;
    CComPointer<IDirectSoundCaptureBuffer> piDSCB;
    CComPointer<IDirectSoundNotify> piDSNCapture;
    WAVEFORMATEX wfx;
    DSBUFFERDESC dsbd;
    DSCBUFFERDESC dscbd;
    DWORD dwStartTime, dwTimeElapsed, dwTmp, dwWaitResult = 0;
    DWORD dwLastReadPosition, dwReadPosition, dwBlockSize, dwWritePosition, dwBufferSize;
    LPBYTE pbAudioCapturePtr0, pbAudioCapturePtr1, pbAudioPlaybackPtr0, pbAudioPlaybackPtr1;
    DWORD dwAudioCaptureBytes0, dwAudioCaptureBytes1, dwAudioPlaybackBytes0, dwAudioPlaybackBytes1;
    bool fPlaying;
    int i;

    dbgout(TEXT("Capturing %g seconds and playing back with a %lums delay.\n"), 
           static_cast<double>(m_dwTimeout)/1000.0,
           m_dwDelay);

    // Because this application is using DirectSound as well as DirectSoundCapture, 
    // capture buffer creation can fail when the format of the capture buffer is not 
    // the same as that of the primary buffer. The reason is that some cards have only 
    // a single clock and cannot support capture and playback at two different frequencies.
    // To simplify buffer creation, we'll borrow the wave format of the primary buffer
    // to create our capure and playback buffers.
    hr = m_piDSBPrimary->GetFormat(&wfx, sizeof(wfx), NULL);
    CheckHRESULT(hr,"IDirectSoundBuffer::GetFormat on the primary sound buffer", return);

    // Because some dsound drivers have hardware specific formats for the primary buffer
    // we may sometimes need to use an alternate default setting.  We'll query the 
    // DSCCAPS structure for this information
    if (wfx.wFormatTag!=WAVE_FORMAT_PCM)
    {
        hr = GetDefaultCaptureFormat(m_piDSC.AsInParam(), wfx);
        CheckHRESULT(hr, "GetDefaultCaptureFormat", return);
    }

    // The buffer size should be aligned along wfx.nBlockAlign.  
    // We're going to use four blocks, each of which is 2 seconds long.
    // Change this to fit your specific memory requirements
    // and your platform's specific alignment requirements
    dwBlockSize = (m_dwDelay>>1)*wfx.nAvgBytesPerSec/1000;
    dwBlockSize -= dwBlockSize%wfx.nBlockAlign;
    dwBufferSize = dwBlockSize*nNumStreamingBlocks;

    // Create the Capture Buffer
    dscbd.dwSize = sizeof(DSCBUFFERDESC);
    dscbd.dwFlags = 0;
    dscbd.dwBufferBytes = dwBufferSize;
    dscbd.dwReserved = 0; 
    dscbd.lpwfxFormat = &wfx;

    hr = m_piDSC->CreateCaptureBuffer(&dscbd, piDSCB.AsOutParam(), NULL);
    CheckHRESULT(hr, "IDirectSoundCapture::CreateCaptureBuffer", return);

    // Create a streaming playback buffer
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_CTRLALL | DSBCAPS_GLOBALFOCUS; 
    dsbd.dwBufferBytes = dwBufferSize;
    dsbd.dwReserved = 0; 
    dsbd.lpwfxFormat = &wfx;

    hr = m_piDS->CreateSoundBuffer(&dsbd, piDSB.AsOutParam(), NULL);
    CheckHRESULT(hr, "IDirectSound::CreateSoundBuffer", return);

    // clear the playback buffer
    hr = ClearSoundBuffer(piDSB.AsInParam());
    CheckHRESULT(hr, "ClearSoundBuffer", return);

    // Set up the notifications
    for (i=0; i<nNumStreamingBlocks; i++)
    {
        if (!rhEvents[i].Create())
        {
            dbgout(TEXT("Could not create the notification event %d\n"), i);
            return;
        }
        rdsbpnNotifications[i].dwOffset = dwBlockSize*(i+1)-wfx.nBlockAlign;
        rdsbpnNotifications[i].hEventNotify = rhEvents[i];
    }

    hr = piDSCB->QueryInterface(IID_IDirectSoundNotify, reinterpret_cast<void**>(piDSNCapture.AsOutParam()));
    CheckHRESULT(hr, "IDirectSoundCaptureBuffer::QueryInterface", return);

    hr = piDSNCapture->SetNotificationPositions(nNumStreamingBlocks, rdsbpnNotifications);
    CheckHRESULT(hr, "IDirectSoundNotify::SetNotificationPositions", return);

    hr = piDSCB->Start(DSCBSTART_LOOPING);
    CheckHRESULT(hr, "IDirectSoundCaptureBuffer::Start(DSCBSTART_LOOPING)", return);

    dwStartTime = GetTickCount();
    dwTimeElapsed = 0;
    dwLastReadPosition = 0;
    dwWritePosition = 0;
    fPlaying = false;

    while (m_dwTimeout>dwTimeElapsed && dwWaitResult != WAIT_TIMEOUT)
    {
        dwWaitResult = WaitForMultipleObjects(nNumStreamingBlocks, (const HANDLE*) rhEvents, FALSE, 2 * m_dwDelay);

		//dbgout(TEXT("wait returns %d\n"), dwWaitResult);

        do
        {
            // Wait until portion of the capture buffer we want to lock is available.
            // If notifications are sent correctly this loop will execute only once.
            hr = piDSCB->GetCurrentPosition(&dwTmp, &dwReadPosition);
            CheckHRESULT(hr, "IDirectSoundCapture::GetCurrentPosition", return);

            // adjust the read position if we're wrapped around the end of the buffer
            if (dwReadPosition<dwLastReadPosition)
                dwReadPosition+=dwBufferSize;
        } while (dwLastReadPosition+dwBlockSize > dwReadPosition);

        // Note: Some types of audio systems use different clocks for capture and playback, 
        // which can cause the streaming rate from the capture buffer to be slightly 
        // different than the streaming rate into the playback buffer.  If that is the case,
        // there will eventually be a locking conflict when we try to lock the playback buffer.
        // This test will stop the audio capture if this condition occurs.
        // If this condition happens on your HW, it will happen sooner with small block sizes.
        // Using the -t and -d options, you can determine how large a block is needed to 
        // achieve a reasonable mean time before failure. 
#       ifdef DEBUG
        if (fPlaying)
        {
            // Get the play position
            DWORD dwCurrentPlayPosition, dwCurrentWritePosition;
            hr = piDSB->GetCurrentPosition(&dwCurrentPlayPosition, &dwCurrentWritePosition);
            CheckHRESULT(hr, "IDirectSoundBuffer::GetCurrentPosition", return);

            if (dwCurrentWritePosition<dwCurrentPlayPosition)
                dwCurrentWritePosition += dwBufferSize;

            if ((dwCurrentPlayPosition>=dwWritePosition && dwCurrentPlayPosition<=dwWritePosition+dwBlockSize)
                || (dwCurrentWritePosition>=dwWritePosition && dwCurrentWritePosition<=dwWritePosition+dwBlockSize))
            {
                dbgout(TEXT("IDirectSoundBuffer::Lock collided with the play position of the playback buffer at time %lums.\n"), GetTickCount()-dwStartTime);
                break;
            }
        }
#       endif

        // Lock the first half of the capture buffer and the next block in the playback buffer
        hr = piDSCB->Lock(dwLastReadPosition, dwBlockSize, 
                          reinterpret_cast<LPVOID*>(&pbAudioCapturePtr0), &dwAudioCaptureBytes0,
                          reinterpret_cast<LPVOID*>(&pbAudioCapturePtr1), &dwAudioCaptureBytes1,
                          0);
        CheckHRESULT(hr, "IDirectSoundCaptureBuffer::Lock", return);

        // Lock the next block of the playback buffer
        hr = piDSB->Lock(dwWritePosition, dwBlockSize, 
                         (LPVOID*)&pbAudioPlaybackPtr0, &dwAudioPlaybackBytes0,
                         (LPVOID*)&pbAudioPlaybackPtr1, &dwAudioPlaybackBytes1,
                         0);
        CheckHRESULT(hr, "IDirectSoundBuffer::Lock", return);

        // Sanity check
        ASSERT(dwAudioCaptureBytes0+dwAudioCaptureBytes1==dwBlockSize);
        ASSERT(dwAudioPlaybackBytes0+dwAudioPlaybackBytes1==dwBlockSize);

        // Copy the capture data to the locked portion of the playback buffer
        if (dwAudioPlaybackBytes0 <= dwAudioCaptureBytes0)
        {
            dwTmp = dwAudioCaptureBytes0-dwAudioPlaybackBytes0;
            memcpy(pbAudioPlaybackPtr0, pbAudioCapturePtr0, dwAudioPlaybackBytes0);
            if (dwTmp>0)
            {
                memcpy(pbAudioPlaybackPtr1, pbAudioCapturePtr0 + dwAudioPlaybackBytes0, dwTmp);
                if (dwAudioCaptureBytes1)
                    memcpy(pbAudioPlaybackPtr1+dwTmp, pbAudioCapturePtr1, dwAudioCaptureBytes1);
            }
        }
        else
        {
            memcpy(pbAudioPlaybackPtr0, pbAudioCapturePtr0, dwAudioCaptureBytes0);
            dwTmp = dwAudioPlaybackBytes0-dwAudioCaptureBytes0;
            if (dwTmp>0)
            {
                memcpy(pbAudioPlaybackPtr0+dwAudioCaptureBytes0, pbAudioCapturePtr1, dwTmp);
                if (dwAudioCaptureBytes1>dwTmp)
                    memcpy(pbAudioPlaybackPtr1, pbAudioCapturePtr1+dwTmp, dwAudioCaptureBytes1-dwTmp);
            }
        }

        // Unlock the buffers
        hr = piDSB->Unlock((LPVOID*)pbAudioPlaybackPtr0, dwAudioPlaybackBytes0, 
                           (LPVOID*)pbAudioPlaybackPtr1, dwAudioPlaybackBytes1);
        CheckHRESULT(hr, "IDirectSoundBuffer::Unlock", return);

        hr = piDSCB->Unlock((LPVOID*)pbAudioCapturePtr0, dwAudioCaptureBytes0,
                            (LPVOID*)pbAudioCapturePtr1, dwAudioCaptureBytes1);
        CheckHRESULT(hr, "IDirectSoundCaptureBuffer::Unlock", return);

        if (!fPlaying && dwWritePosition != 0)
        {
            hr = piDSB->Play(0,0,DSBPLAY_LOOPING);
            CheckHRESULT(hr, "IDirectSoundBuffer::Play", return);
            fPlaying=true;
        }

        dwLastReadPosition = (dwLastReadPosition+dwBlockSize)%dwBufferSize;
        dwWritePosition = (dwWritePosition+dwBlockSize)%dwBufferSize;
        dwTimeElapsed = GetTickCount()-dwStartTime;
    }


    // clear the next playback block to prevent playing noise when we reach the end
    // of the last block
    hr = piDSB->Lock(dwWritePosition, dwBlockSize, 
                     (LPVOID*)&pbAudioPlaybackPtr0, &dwAudioPlaybackBytes0,
                     (LPVOID*)&pbAudioPlaybackPtr1, &dwAudioPlaybackBytes1,
                     0);
    CheckHRESULT(hr, "IDirectSoundBuffer::Lock", return);

    memset(pbAudioPlaybackPtr0, 0, dwAudioPlaybackBytes0);
    if (pbAudioPlaybackPtr1)
        memset(pbAudioPlaybackPtr1, 0, dwAudioPlaybackBytes1);

    hr = piDSB->Unlock((LPVOID*)pbAudioPlaybackPtr0, dwAudioPlaybackBytes0, 
                       (LPVOID*)pbAudioPlaybackPtr1, dwAudioPlaybackBytes1);
    CheckHRESULT(hr, "IDirectSoundBuffer::Unlock", return);

    // Wait until the next notification is sent. We're no longer streaming, but we're 
    // going to continue to run the capture buffer to use its notification
    dwTmp = ((dwWritePosition/dwBlockSize)+nNumStreamingBlocks+1)%nNumStreamingBlocks;
    dwWaitResult = WaitForMultipleObjects(1, (const HANDLE*) &rhEvents[dwTmp], FALSE, m_dwDelay);

    hr = piDSB->Stop();
    CheckHRESULT(hr, "IDirectSoundBuffer::Stop", return);

    hr = piDSCB->Stop();
    CheckHRESULT(hr, "IDirectSoundCaptureBuffer::Stop", return);

    dbgout(TEXT("Capturing completed.\n"));

}

void CCaptureEchoApplication::DisplayUsage()
{
    dbgout(TEXT("usage:\n\tCaptureEcho [-t timeout] [-d delay]\n"));
    dbgout(TEXT("\t-d approximate delay in milliseconds, default = %lu\n"), m_dwDelay);
    dbgout(TEXT("\t   NOTE: Delay must be greater than 500ms\n"));
    dbgout(TEXT("\t-t timeout in seconds, default = %lu\n"), m_dwTimeout/1000);
    dbgout(TEXT("\t   NOTE: Timeout must be greater than 4 times the delay.\n"));
    dbgout(TEXT("\t         It will also be converted to ms and quantized to the delay.\n"));
}

bool CCaptureEchoApplication::ProcessCommandLine()
{
    LPCTSTR pszHelpOption = _tcsstr(m_pszCommandLine, TEXT("-?"));
    if (pszHelpOption==NULL) LPCTSTR pszHelpOption = _tcsstr(m_pszCommandLine, TEXT("-h"));
    LPCTSTR pszTimeOption = _tcsstr(m_pszCommandLine, TEXT("-t"));
    LPCTSTR pszDelayOption = _tcsstr(m_pszCommandLine, TEXT("-d"));

    if (pszHelpOption!=NULL)
    {
        DisplayUsage();
        return false;
    }

    if (pszDelayOption!=NULL)
    {
        pszDelayOption+=2;

        // Skip Whitespace
        while (_istspace(*pszDelayOption))
            pszDelayOption++;

        m_dwDelay = _ttoi(pszDelayOption);

        if (m_dwDelay<500)
        {
            DisplayUsage();
            return false;
        }
    }

    if (pszTimeOption!=NULL)
    {
        pszTimeOption+=2;

        // Skip Whitespace
        while (_istspace(*pszTimeOption))
            pszTimeOption++;

        m_dwTimeout = _ttoi(pszTimeOption) * 1000;

        if (m_dwTimeout==0)
        {
            DisplayUsage();
            return false;
        }

        if (m_dwTimeout<4*m_dwDelay)
        {
            dbgout(TEXT("ERROR: Timeout must be at least 4 times the delay.\n"));
            DisplayUsage();
            return false;
        }

        m_dwTimeout -= m_dwTimeout%m_dwDelay;
    }

    return true;
}

