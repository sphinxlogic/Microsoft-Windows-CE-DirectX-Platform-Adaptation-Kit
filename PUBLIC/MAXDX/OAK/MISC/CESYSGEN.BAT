REM **USAGE**
REM MAXALL_TASKMAN=0 Include Handheld-PC style shell
REM MAXALL_NOPIE=0 Include Pocket Internet Explorer
REM MAXALL_NOJSCRIPT=0 Include JScript
REM MAXALL_NOHELP=0 Include Help engine (Peghelp)
REM MAXALL_NOCPL=0 Include Control Panels
REM MAXALL_NOPWORD=0 Include Pocket Word
REM MAXALL_NOPMAIL=0 Include Pocket Mail

REM // Base system components: explained in MINKERN
set CE_MODULES=coredll filesys nk toolhelp shell

set COREDLL_COMPONENTS=coremain coreloc lmem thunks fmtmsg accel_c
set FILESYS_COMPONENTS=fsysram fsreg fsheap fsmain fspass fsdbase

REM // Base GWE components explained in MININPUT
set CE_MODULES=%CE_MODULES% gwes
set GWE1_COMPONENTS=wmbase gweshare gwesmain immthunk msgque loadstr GSetWinLong
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% rectapi wmgr_c
set GWE1_COMPONENTS=%GWE1_COMPONENTS% foregnd uibase kbdui journal hotkey idle getpower nled msgbeep

REM // Base GDI components explained in MINGDI
set GWE2_COMPONENTS=mgbase mgbitmap mgblt mgblt2 mgdc mgdibsec mgdraw mgrgn mgwinmgr mgpalnat
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% mgdi_c
REM // Extra MGDI components + tt support + printing
set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgtt mgdrwtxt mgpal mgprint mgtci mgalias mgfe
set GWE2_COMPONENTS=%GWE2_COMPONENTS% tchui calibrui

REM Required by DirectDraw
set GWE2_COMPONENTS=%GWE2_COMPONENTS% mgdx

REM // Window manager
if "%MAXALL2_NOCURSOR%"=="1" set GWE3_COMPONENTS=icon cursor cursor8
if not "%MAXALL2_NOCURSOR%"=="1" set GWE3_COMPONENTS=iconcurs mcursor mcursor8
set GWE3_COMPONENTS=%GWE3_COMPONENTS% winmgr sbcmn nclient caret clipbd timer accel defwndproc gcache loadbmp loadimg syscolor
set GWE3_COMPONENTS=%GWE3_COMPONENTS% menu mNoTapUI cascade column menuscrl drawmbar
REM Choose mOverlap instead of mNoOver to get overlapping (instead of adjacent) cascading menus
set GWE3_COMPONENTS=%GWE3_COMPONENTS% mNoOver 
set GWE4_COMPONENTS=dlgmgr dlgmnem msgbox msgbox28 gwectrl btnctl stcctl
set GWE4_COMPONENTS=%GWE4_COMPONENTS% startup oomui oom startui cmbctl cdlctl edctl imgctl lbctl scbctl
if "%COUNTRY%" == "JAPAN" set GWE4_COMPONENTS=%GWE4_COMPONENTS% edimefe edimejpn
if "%COUNTRY%" == "KOREA" set GWE4_COMPONENTS=%GWE4_COMPONENTS% EdImeFEK EdImeKor

REM // IMM and SIP
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% coreimm coresip
set CE_MODULES=%CE_MODULES% softkb

REM // Communications components explained in MINCOMM
REM // TCP on PPP & NDIS, IRCOMM on IRDA
set CE_MODULES=%CE_MODULES% device cxport winsock afd netui
set CE_MODULES=%CE_MODULES% tcpstk ppp
set CE_MODULES=%CE_MODULES% ircomm irdastk
set CE_MODULES=%CE_MODULES% ndis arp ne2000 proxim xircce2 dhcp
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% ndis arp ne2000 proxim xircce2 dhcp
if not "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% ndis arp ne2000 dhcp

REM // USB
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% usbd usbmouse usbhid

REM // Serial port, Tapi, Unimodem
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% serdev tapilib
set CE_MODULES=%CE_MODULES% tapi unimodem

REM // Secure components: SSPI, schannel, ntlm; cryptapi; secure sockets
set CE_MODULES=%CE_MODULES% secur32 ntlmssp schannel rsabase
set WINSOCK_COMPONENTS=sslsock
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% cryptapi rsa32
REM // Redirector
set CE_MODULES=%CE_MODULES% redir netbios

REM // COM and desktop compatible OLE
set CE_MODULES=%CE_MODULES% uuid ole32 oleaut32
set OLE32_COMPONENTS=ole232 com docfile msf exp

REM // required by IE
set OLE32_COMPONENTS=%OLE32_COMPONENTS% stg olemain

REM // FatFS support: needs pcmcia
set CE_MODULES=%CE_MODULES% fatfs atadisk ramdisk ceramdrv sramdisk trueffs
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% fatfs atadisk ramdisk ceramdrv sramdisk trueffs
if not "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% fatfs

REM // Support the Socket Dual Serial card.
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% dualio

REM // Wave API and PCM manager support
set CE_MODULES=%CE_MODULES% waveapi cegsm msfilter
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% wavelib
set GWE1_COMPONENTS=%GWE1_COMPONENTS% audio

REM // Printing support
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES%  parallel pcl prnport prnerr

REM // Handwriting recognition
if not "%MAXDX_FULL%"=="1" goto end_handwriting
if "%COUNTRY%" == "JAPAN" set CE_MODULES=%CE_MODULES% hwxjpn
if not "%COUNTRY%" == "JAPAN" set CE_MODULES=%CE_MODULES% hwxusa splusa
:end_handwriting

REM // Notification APIS
set GWE1_COMPONENTS=%GWE1_COMPONENTS% notify notifpub

set GWES_COMPONENTS=gwe1 gwe2 gwe3 gwe4

REM // Console support
if not "%MAXDX_FULL%"=="1" goto end_console
set CE_MODULES=%CE_MODULES% cmd
if "%COUNTRY%" == "JAPAN" set CE_MODULES=%CE_MODULES% consolej
if not "%COUNTRY%" == "JAPAN" set CE_MODULES=%CE_MODULES% console
:end_console

set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% fmtres
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% corestrw corestra coresiow coresioa

REM Shell API components: SHMISC, SHEXEC, SHORTCUT, FILEOPEN
REM If any are chosen SHCORE must also be included.
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% shcore shortcut shexec shmisc fileopen fileinfo

REM This component adds thunks for Shell_NotifyIcon & SHAddToRecentDocs
REM NOTE: On anything other than HPC or PPC these stubs will silently do nothing
REM The only purpose of this component is to make it easier to write drivers etc
REM that run on the HPC or PPC as well as on an ETK-based custon OEM config
set COREDLL_COMPONENTS=%COREDLL_COMPONENTS% shellapis

REM commctrl, commdlg, taskman
set CE_MODULES=%CE_MODULES% commdlg commctrl

REM //Communications apps
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% pegterm termctrl rnaapp remnet

REM // Sample SDK apps: MSIM
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% msim

REM // Select all of the IME's
REM // At Makeimg time the IME is selected via the IME environment variable
REM //    IME=PIME     - Indicates to use pocket MS-IME
REM //    IME=TESTIME  - Indicates to use the testime sample in common\sdk\samples 
REM //    anything else- Indicate to use MS-IME98 for Windows CE
IF NOT "%COUNTRY%" == "JAPAN"   goto end_ime_japan
set CE_MODULES=%CE_MODULES% msime98 msime98k msime98t msime98d msime98c msime98m msime98u utl98 ime_files
set CE_MODULES=%CE_MODULES% imejpp imejppui pime_files
set CE_MODULES=%CE_MODULES% testime
:end_ime_japan

REM // Korean IME
if "%COUNTRY%" == "KOREA" set CE_MODULES=%CE_MODULES% mshime97

REM //=== These CONN components should be in their own project but for now they are in COMMON =====
if "%MAXDX_FULL%"=="1" set CE_MODULES=%CE_MODULES% repllog rapisrv

REM // ++++++ Resolve dependencies among conditionals +++++++
REM // Peghelp needs PIE
if not "%MAXALL_NOHELP%"=="1" set MAXALL_NOPIE=0
REM // PIE neeeds HPC shell
if not "%MAXALL_NOPIE%"=="1" set MAXALL_TASKMAN=0

REM //============ These components come from the IE project ====================
set IE_MODULES=

REM // Wininet (reqd by Pocket IE & HPC shell, below)
set IE_MODULES=%IE_MODULES% wininet
set WININET_COMPONENTS=inetftp

REM // MLANG (reqd by Pocket IE, below)
set IE_MODULES=%IE_MODULES% mlang

REM // urlmon--reqd by dshow
set IE_MODULES=%IE_MODULES% urlmon

REM // the WebBrowser Control
set IE_MODULES=%IE_MODULES% mshtml shdocvw

REM //============ These components come from the WCESHELL project ====================
set WCESHELL_MODULES=

REM // Sample minshell (can have this OR jupiter shell but NOT both)
if "%MAXALL_TASKMAN%"=="1" set WCESHELL_MODULES=%WCESHELL_MODULES% taskman

REM // Jupiter shell (can have this OR minshell (taskman) but NOT both)
if not "%MAXALL_TASKMAN%"=="1" set WCESHELL_MODULES=%WCESHELL_MODULES% asform ceshell explorer

REM // PIE (Pocket Internet Explorer -- requires Jupiter shell)
if not "%MAXALL_NOPIE%"=="1" set WCESHELL_MODULES=%WCESHELL_MODULES% imgdecmp webview iexplore

REM // Jscript is optional with PIE
if not "%MAXALL_NOJSCRIPT%"=="1" set CE_MODULES=%CE_MODULES% jscript jsen

REM // Help engine (requires PIE & jupiter shell)
if not "%MAXALL_NOHELP%"=="1" set WCESHELL_MODULES=%WCESHELL_MODULES% htmlview peghelp

if "%MAXALL_NOCPL%"=="1" goto nocpl
REM // Control panel modules
set WCESHELL_MODULES=%WCESHELL_MODULES% control ctlpnl cplmain

REM // choose L (landscape, i.e. HPC style) or P (portrait, i.e. PsPC-style) resources for CPLMAIN
if "%CPLMAIN_LP%"=="" set CPLMAIN_LP=L

REM // the component CPLs in cplmain. Remove what you dont need (Note: if you incl either mouse or stylus you must incl pointercmn)
set CPLMAIN_COMPONENTS=network comm datetime dialing keyboard password owner power system display pointercmn stylus mouse sounds sip
REM // Optional tabs of the display CPL. Remove those you don't need
set CPLMAIN_COMPONENTS=%CPLMAIN_COMPONENTS% backlight colorscheme

REM // Regional-settings CPL. Replace intll with intlp for PalmPC-sized dialogs
if not "%CPLMAIN_LP%"=="P" set WCESHELL_MODULES=%WCESHELL_MODULES% intll
if "%CPLMAIN_LP%"=="P" set WCESHELL_MODULES=%WCESHELL_MODULES% intlp
:nocpl

REM //====== If target locale is JAPAN use WCESHELLFE tree instead of WCESHELL =======
if "%COUNTRY%"=="JAPAN" set WCESHELLFE_MODULES=%WCESHELL_MODULES%&set WCESHELL_MODULES=

REM //============ These components come from the WCEAPPS project ====================
set WCEAPPS_MODULES=

REM // Riched20 is used by both pmail and pword
REM // pWord
if not "%MAXALL_NOPWORD%"=="1" set WCEAPPS_MODULES=%WCEAPPS_MODULES% office pword pwd_res pwwiff riched20

REM // pInbox 
if not "%MAXALL_NOPMAIL%"=="1" set WCEAPPS_MODULES=%WCEAPPS_MODULES% pmail imap4 smtp msgstore tnefutil mailutil labledit uicom pimprint riched20

REM //====== If target locale is JAPAN use WCEAPPSFE tree instead of WCEAPPS =======
if "%COUNTRY%"=="JAPAN" set WCEAPPSFE_MODULES=%WCEAPPS_MODULES%&set WCEAPPS_MODULES=
if "%COUNTRY%"=="JAPAN" if not "%MAXALL_NOPMAIL%"=="1" set WCEAPPSFE_MODULES=%WCEAPPSFE_MODULES% tnefu tnefex

REM //============ These components come from the MSMQ project ====================
REM // For SP2 the default is no MSMQ components, set MAXALL_MSMQ=1 to get the MSMQ components

set MSMQ_MODULES=
if "%MAXALL_MSMQ%"=="1" set MSMQ_MODULES=msmqd mqoa msmqrt msmqadm netregd

REM //============ These components are from DirectX ==============================
REM // clean out any old settings
set DIRECTX_MODULES=

set QUARTZ1_COMPONENTS=acmwrap wavemsr waveout dsndout mididm mpgadec 
set QUARTZ2_COMPONENTS=asyncrdr urlrdr qtrdr avimsr avidec mpgsplit mpgvdec %QUARTZ_EXTRA_COMPONENTS%
set QUARTZ2_COMPONENTS=%QUARTZ2_COMPONENTS% image ovmixer scmdout 
set QUARTZ_COMPONENTS=quartz1 quartz2

REM // DirectSound core components
set DIRECTX_MODULES=%DIRECTX_MODULES% directsound
REM // DirectSound wave compatibility components
set DIRECTX_MODULES=%DIRECTX_MODULES% idswave wavebuff
REM // Mixer API
set DIRECTX_MODULES=%DIRECTX_MODULES% mixerapi
REM // sample 'null' audio mixer driver
set DIRECTX_MODULES=%DIRECTX_MODULES% mxat
REM // DirectSound sample application
set DIRECTX_MODULES=%DIRECTX_MODULES% dsplay
REM // Sample DSHAL DirectSound driver
set DIRECTX_MODULES=%DIRECTX_MODULES% idses1371
REM // Sample wavedev drivers for DirectSound emulation
set DIRECTX_MODULES=%DIRECTX_MODULES% es1371 igs5000
REM // Sample capture application
set DIRECTX_MODULES=%DIRECTX_MODULES% CaptureEcho

REM // DirectDraw components
set DIRECTX_MODULES=%DIRECTX_MODULES% directdraw ddhel
REM // Sample DirectDraw drivers
set DIRECTX_MODULES=%DIRECTX_MODULES% ddi_igs5 ddi_ds3v

REM // DirectShow components
set DIRECTX_MODULES=%DIRECTX_MODULES% mmtimer quartz amstream
REM // Windows Media Player component
set DIRECTX_MODULES=%DIRECTX_MODULES% amovie
REM // DirectShow audio codecs
set DIRECTX_MODULES=%DIRECTX_MODULES% msacmce msadpcm imaadpcm pcmconv msgsm610 msg711
REM // DirectShow video codecs
set DIRECTX_MODULES=%DIRECTX_MODULES% icm iccvid msrle32
REM // DirectShow sample applications
set DIRECTX_MODULES=%DIRECTX_MODULES% dshowsamples

REM // core DirectMusic engine
set DIRECTX_MODULES=%DIRECTX_MODULES% dmusic
REM // DirectMusic software synth
set DIRECTX_MODULES=%DIRECTX_MODULES% dmsynth
REM // DirectMusic wave samples
set DIRECTX_MODULES=%DIRECTX_MODULES% msdls

REM // Windows Media Audio decompressor
set DIRECTX_MODULES=%DIRECTX_MODULES% wmaudio

if not exist %_PROJECTROOT%\oak\misc\internal.bat goto endif_1
call %_PROJECTROOT%\oak\misc\internal.bat
:endif_1

REM //=================================== the end =====================================

echo Running MAXALL with
echo MAXALL_TASKMAN=%MAXALL_TASKMAN%
echo MAXALL_NOPIE=%MAXALL_NOPIE%
echo MAXALL_NOJSCRIPT=%MAXALL_NOJSCRIPT%
echo MAXALL_NOHELP=%MAXALL_NOHELP%
echo MAXALL_NOCPL=%MAXALL_NOCPL%
echo CPLMAIN_LP=%CPLMAIN_LP%
echo MAXALL_NOPWORD=%MAXALL_NOPWORD%
echo MAXALL_NOPMAIL=%MAXALL_NOPMAIL%
echo MAXALL_MSMQ=%MAXALL_MSMQ%
