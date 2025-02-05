DirectX components

DIRECTX_MODULES:

directdraw      - Selects all the DirectDraw components (not sample drivers)
ddi_igs2        - Sample ddi/ddhal for IGS 2010
ddi_igs5        - Sample ddi/ddhal for IGS 53xx
ddi_ds3v        - Sample ddi/ddhal for S3 Virge

mixerapi        - Audio mixer API client side DLL

directsound     - Selects all the DirectSound components (dsound, dsounds, idswave, wavebuff, dsplay)
dsound		- DSound.dll client side proxy dll
dsounds		- DSoundS.dll runs in device.exe, main DSound object, mixer, interfaces with drivers
dsplay		- Simple DSound sample app allows you to play .wav file with DirectSound
idswave		- (legacy compatibility) IDsDriver wrapper to make DSound work with wavedev drivers
wavebuff	- (legacy compatibility) exposes wavedev interfaces to waveapi.dll so waveOut works on top of DSound

es1371		- Ensoniq PCI 1371/1373 wavedev driver
igs5000		- IGS 5000/5050 wavedev driver

acmdwrap	- (legacy compatibility) allows DriverProc, desktop style ACM codecs to work with CE CoreOS ACM

msadpcm         - Microsoft ADPCM audio codec
imaadpcm        - IMA ADPCM audio codec
pcmconv         - PCM bit/sample rate ACM conversion codec
msgsm610        - GSM 6.10 audio codec
msg711          - G.711 audio codec
wmaudio         - Windows Media Audio codec v2.0
msacmce         - Audio Compression Manger used by DirectShow

msrle32         - RLE8 video codec
iccvid          - Cinepak video codec
mpeg4v3         - MPEG4v3 video codec
icm             - Video Compression Manager

quartz          - Core DirectShow components (see QUARTZx options below)
mmtimer		- Multimedia timer services required by quartz
amovie          - Active Movie Control
amstream        - DirectShow multimedia streaming
dshowsamples    - Various DirectShow sample programs

wmt40           - Windows Media Technologies 4.0

The QUARTZx components are the filters that can be slected into quartz.dll

QUARTZ1_COMPONENTS:

acmwrap         - ACM wrapper filter
asyncrdr        - Asynchronous file reader
wavemsr         - Wave/AIFF/au/snd files parser
waveout         - waveOut audio renderer
dsndout         - DirectSound audio renderer
midiout         - midiOut audio renderer
midirdr         - midi/rmi parser
mididm		- MIDI renderer via DirectMusic

QUARTZ2_COMPONENTS:

avimsr          - AVI parser/splitter
avidec          - AVI Video decompressor (used ICM)
mpgsplit        - MPEG1 parser/splitter
mpgadec         - MPEG1 audio decoder
mpgvdec         - MPEG1 video decoder
image           - Video renderer
qtrdr           - QuickTime parser/splitter
urlrdr          - URL aysnchronous reader
ovmixer         - Old Overlay Mixer
ovmixer2        - New Overlay Mixer (include only one of ovmixer or ovmixer2)

dvsp 		- DV splitter
dvdec		- DV decoder
dvenc		- DV encoder
dvcap		- DV capture
dvmux   	- DV multiplexer
dvavc 		- DV audio/video control
dvdump		- DV dump

Must be set if you use any quartz components
 
set QUARTZ_COMPONENTS=quartz1 quartz2
