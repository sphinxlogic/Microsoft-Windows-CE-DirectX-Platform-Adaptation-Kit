//
//
//  IGS technologies, Inc.
//
//  Bryan  M. Lo
//
//  Using Microsoft built-in stream interface structure
//
//
//

#include <windows.h>
#include <types.h>   
#include <memory.h>
#include <excpt.h>
#include <wavepdd.h>
#include <waveddsi.h>
#include <wavedbg.h>
#include <nkintr.h>
#include <audioreg.h>
#include <ceddk.h>
#include "igs5050.h"
#include <IrqMap.h>

#ifdef DEBUG
DBGPARAM dpCurSettings = {
    TEXT("ES1371")
	, {
		 TEXT("Test")
		,TEXT("Params")
		,TEXT("Verbose")
		,TEXT("Interrupt")
		,TEXT("WODM")
		,TEXT("WIDM")
		,TEXT("PDD")
		,TEXT("MDD")
		,TEXT("8")
		,TEXT("Misc")
		,TEXT("10")
		,TEXT("IOcontrol")
		,TEXT("Alloc")
		,TEXT("Function")
		,TEXT("Warning")
		,TEXT("Error")
	}
	, 0xC000
}; 
#endif

// bus, slot
ULONG		    	ulPciBusNum;
PCI_COMMON_CONFIG   PciCommonConf;
PCI_SLOT_NUMBER     PciSlotNum;

DWORD  	gIntrAudio;
UCHAR  	g_Irq;
USHORT 	g_IoBase;

ULONG  	g_ulFrameBufferBase;
ULONG  	g_ulLinAddrBase;
ULONG  	g_ulAudioAddrBase;   				//  same as  the memory-mapped io port range 
UCHAR  	g_ucChannel=0;        				// set channel 0 as default
ULONG 	g_ulMusWavVolume= DEFAULT_VOLUME;	// default music volume

PVOID  	membase;
PVOID  	pvBufPage[2];
PVOID  	portbase3ce;

ULONG  	ulOldMiscInt;
ULONG  	ulOldAInt;

BOOL 	fInUse;

ULONG   v_nVolume;

static PWAVEFORMATEX g_pwfx[2];
static PCM_TYPE g_pcmtype[2];

//
// These variables may be accessed by different processes or the hardware and
// are likely to change in a time critical fashion. Optimization may cause
// errors if not declared volatile.
//
volatile        BOOL    v_fMoreData[2];
volatile static ULONG   v_nNextPage[2];

// Mixer Registers
MIXER_UNIT  Mixers[] =
{
    { MASTER_VOLUME,              0x0 },
    { PCM_OUT_VOLUME,             0x0 }, 
    { RECORD_SELECT,              0x0 }, 

};
#define NUM_OF_MIXER_TO_BE_INIT (sizeof(Mixers) / sizeof(Mixers[0]))

// Volume Linear to Log Lookup table
// aproximates to roughly 2 db's per increment

UCHAR volumeLUT[] = {
0x00,0x02,0x04,0x07,0x09,0x0C,0x0F,0x12,
0x15,0x18,0x1B,0x1F,0x22,0x26,0x2A,0x2E,
0x33,0x37,0x3D,0x42,0x48,0x4E,0x55,0x5D,
0x66,0x70,0x7B,0x89,0x99,0xAE,0xCC,0xFF };

// -----------------------------------------------------------------------------
// Indirectly read a byte the mapped io Port indexed by index	
// -----------------------------------------------------------------------------
UCHAR IgsRIoPortI_0x3ce(UCHAR ucIdx)
{ 
	PVOID pvTmp;
	UCHAR ucTmp;
	
	(ULONG) pvTmp = (ULONG)portbase3ce;          
	WRITE_PORT_UCHAR( (PUCHAR)pvTmp, ucIdx);
	
	(ULONG)pvTmp = (ULONG) pvTmp + 1;
	ucTmp= READ_PORT_UCHAR((PUCHAR)pvTmp); 
	
	return ucTmp;
}


// -----------------------------------------------------------------------------
// Indirectly write a byte into the mapped io Port indexed by index	
// -----------------------------------------------------------------------------
void IgsWIoPortI_0x3ce(UCHAR ucIdx, UCHAR ucValue)
{ 
	PVOID pvTmp;
	
	(ULONG) pvTmp = (ULONG)portbase3ce;          
	WRITE_PORT_UCHAR( (PUCHAR)pvTmp, ucIdx);
	
	(ULONG)pvTmp = (ULONG) pvTmp + 1;
	WRITE_PORT_UCHAR((PUCHAR)pvTmp, ucValue); 
}


// NOTE: tv regs locking/unlocking causes contention in indexed register access by video driver
//		and is commented out for now...

// -----------------------------------------------------------------------------
// read a ULONG value from the memory register
// -----------------------------------------------------------------------------
ULONG IgsRMemReg(WORD wAddr)
{ 
//	UCHAR ucReg33;
//	UCHAR ucReg3c;
	ULONG ulTmp;
	PVOID pvTmp; 
 
	// get reg 33, 3c init value
	//ucReg33= (UCHAR) IgsRIoPortI_0x3ce(0x33);
	//ucReg3c= (UCHAR) IgsRIoPortI_0x3ce(0x3c);
	
	// unlock  reg
	//IgsWIoPortI_0x3ce(0x33, (UCHAR)(ucReg33 | 0x08));
	//IgsWIoPortI_0x3ce(0x3c, (UCHAR)(ucReg3c | 0x80));
	
	(ULONG)pvTmp= (ULONG)membase + wAddr;
	ulTmp =  READ_REGISTER_ULONG((PULONG) (ULONG)pvTmp); 
	
	// lock  reg
	//IgsWIoPortI_0x3ce(0x33, (UCHAR)(ucReg33));
	//IgsWIoPortI_0x3ce(0x3c, (UCHAR)(ucReg3c));
	
	return ulTmp;
}


// -----------------------------------------------------------------------------
// write a USHORT value into the memory register	
// -----------------------------------------------------------------------------
VOID IgsWMemReg(WORD wAddr, USHORT usValue)
{ 
	PVOID pvTmp; 
//	UCHAR ucReg33;
//	UCHAR ucReg3c;
	
	// get reg 33, 3c init value
	//ucReg33= (UCHAR) IgsRIoPortI_0x3ce(0x33);
	//ucReg3c= (UCHAR) IgsRIoPortI_0x3ce(0x3c);
	
	// unlock tv reg
	//IgsWIoPortI_0x3ce(0x33, (UCHAR)(ucReg33 | 0x08));
	//IgsWIoPortI_0x3ce(0x3c, (UCHAR)(ucReg3c | 0x80));
	
	(ULONG)pvTmp= (ULONG)membase + wAddr;
	WRITE_REGISTER_USHORT( (PUSHORT)pvTmp, usValue);
	
	// lock tv reg
	//IgsWIoPortI_0x3ce(0x33, (UCHAR)(ucReg33));
	//IgsWIoPortI_0x3ce(0x3c, (UCHAR)(ucReg3c));
}


// -----------------------------------------------------------------------------
// write a ULONG value into the memory register	
// -----------------------------------------------------------------------------
VOID IgsWMemRegL(WORD wAddr, ULONG ulValue)
{ 
	PVOID pvTmp; 
//	UCHAR ucReg33;
//	UCHAR ucReg3c;
	
	// get reg 33, 3c init value
	//ucReg33= (UCHAR) IgsRIoPortI_0x3ce(0x33);
	//ucReg3c= (UCHAR) IgsRIoPortI_0x3ce(0x3c);
	
	// unlock tv reg
	//IgsWIoPortI_0x3ce(0x33, (UCHAR)(ucReg33 | 0x08));
	//IgsWIoPortI_0x3ce(0x3c, (UCHAR)(ucReg3c | 0x80));
	
	(ULONG)pvTmp= (ULONG)membase + wAddr;
	WRITE_REGISTER_ULONG( (PULONG)pvTmp, ulValue);
	
	// lock tv reg
	//IgsWIoPortI_0x3ce(0x33, (UCHAR)(ucReg33));
	//IgsWIoPortI_0x3ce(0x3c, (UCHAR)(ucReg3c));
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID WriteMixerReg(UCHAR ucIdx, USHORT usData)
{ 
	USHORT usTmp;
	ULONG tick;

	while( IgsRMemReg(0xd040) & BUSY_WRITING ); 

	// hardware bug:
	// Wait a max of 2ms while writing the mixer registers
	// the chip apears to require somekind of power up time
	// between writes, otherwise no sound is heard, one probably
	// only needs a couple hundered microseconds but since we 
	// don't have access to a microsecond clock on all platforms
	// and using GetTickCount() to time a single millisecond may
	// result in reading a time just before the counter kicks over
	// cause not enough delay time.
	tick = GetTickCount();
	while ( GetTickCount() - tick <= 2 );

	usTmp = (0x8000 | ucIdx);
	IgsWMemReg(0xd040, usTmp); 

	while( IgsRMemReg(0xd040) & BUSY_WRITING ); 
	IgsWMemReg(0xd042, (USHORT)usData); 
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
USHORT ReadMixerReg(UCHAR ucIdx)
{ 
	ULONG  ulTmp;
	ULONG  l;
	
	IgsWMemReg(0xd044,  (USHORT)(0x8000 | ucIdx));
	IgsWMemReg(0xd046,  0x0);  // clean
	
	for(l=0; l < 0x7ffffff ; l++)
	{ 
		if ( !(((USHORT)IgsRMemReg(0xd044)) & BUSY_READING) )
		{ 
			ulTmp = IgsRMemReg(0xd044); 
			ulTmp >>= 16;
			return (USHORT)ulTmp;
		}
	}

	return -1;
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID IgsAudioFillBuffer ( PWAVEHDR pwh, BYTE UNALIGNED *pBuffer )
{
	ULONG offset;
	ULONG bytescopied;
    PPCMWAVEFORMAT pFormat = (PPCMWAVEFORMAT) g_pwfx[WAPI_OUT]; 

    FUNC_VERBOSE("+PDD_AudioFillBuffer");

	// reset   BANK_A address engine interrupt status
	IgsWMemRegL(0xd000+WER_AINT, (ULONG)(0x00000001 << g_ucChannel) );

    //
	// Fill the DMA Buffer and clear the remainder if nessary 
    //
	bytescopied = 0;
	offset = 0; 
	while ( offset < NODE_SIZE ) 
	{
		if (pwh != NULL && pwh->dwBytesRecorded >= pwh->dwBufferLength) 
		{
			pwh = pwh->lpNext;
			continue;
		}

	    if (pwh != NULL) 
		{
			// Fill the DMA Buffer 
			bytescopied = pwh->dwBufferLength - pwh->dwBytesRecorded;
			if ( bytescopied > NODE_SIZE - offset )
				bytescopied = NODE_SIZE - offset;

			memcpy(pBuffer + offset, pwh->lpData + pwh->dwBytesRecorded, bytescopied);

			pwh->dwBytesRecorded += bytescopied;
        }
		else
		{
			// Nothing lefted to copy clear the remainder of the buffer
			if ( pFormat->wBitsPerSample == 8 )
			    memset(pBuffer + offset, 0x80, NODE_SIZE-offset);
			else
			    memset(pBuffer + offset, 0, NODE_SIZE-offset);

			bytescopied += NODE_SIZE-offset;
		}

	    offset += bytescopied;
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

BOOL WakeUpChip(void)
{ PHYSICAL_ADDRESS 	MappedAddress, PortAddress;
  PUCHAR portbase;
  int MemType;

	// locate io address for 0
	PortAddress.LowPart  = 0;
	PortAddress.HighPart = 0; 
	MemType=1; // io space    
	if (!HalTranslateBusAddress( PCIBus, 0, PortAddress, &MemType, &MappedAddress)) {
		return FALSE;
	}
	if ( MemType == 1 ) {
		portbase = (PUCHAR) MappedAddress.LowPart;
	}
	else {
		portbase = MmMapIoSpace(MappedAddress, 0x4800, FALSE);
	}

	// I stole this from the IGS5000 video driver
	// The code makes no comment about what exactly this does,
	// but it works...
	// kl 7/27/99
	WRITE_PORT_UCHAR(portbase + 0x46e8, 0x18);
	WRITE_PORT_UCHAR(portbase +  0x102, 0x01);
	WRITE_PORT_UCHAR(portbase + 0x46e8, 0x08);

	if ( MemType != 1) {
		MmUnmapIoSpace(portbase, 0x4800);
	}
	return TRUE;
}

// -----------------------------------------------------------------------------
//   IsIgsPciAudioExist
//    		====>check if  IGS audio circuit exists in PCI bus
//
// -----------------------------------------------------------------------------
BOOL 
IsIgsPciAudioExist(IN OUT  PULONG             pulPciBusNum,
				   IN OUT  PPCI_SLOT_NUMBER   pPciSlotNum,
				   IN OUT  PPCI_COMMON_CONFIG pPciCommonConf
				   )
{ 
	ULONG  		ulReturn;
	int    		iBusNum=0;
	int	 		iDevNum=0;
	int	 		iFunNum=0;
	ULONG            	MemType;
	PHYSICAL_ADDRESS 	PortAddress;
	PHYSICAL_ADDRESS 	MappedAddress;
	BOOLEAN          	success_flag=FALSE;
	UCHAR               ucTmp=0x0;
	BOOLEAN             bRet=FALSE;
	
	DEBUGMSG(1, (TEXT("Igs5050: Init\r\n")));
	//
	// Scan the PCI bus to locate igs5050 audiocards 
	//
	for (iBusNum=0;  iBusNum < PCI_MAX_BUS  ; iBusNum++)
	{ 
		for(iDevNum=0; iDevNum < PCI_MAX_DEVICES; iDevNum++)
		{
			pPciSlotNum->u.bits.DeviceNumber = iDevNum;
			
			for (iFunNum=0; iFunNum < PCI_MAX_FUNCTION ; iFunNum++) 
			{ 
				pPciSlotNum->u.bits.FunctionNumber = iFunNum;
				ulReturn = HalGetBusDataByOffset(PCIConfiguration, iBusNum, pPciSlotNum->u.AsULONG ,
					pPciCommonConf, 0, sizeof(PCI_COMMON_CONFIG));
				
				// Couldn't even get the card data, though it's supposed to exist.
				if ((ulReturn == 0)||(pPciCommonConf->VendorID == 0xffff) )  
					break;
				
				if ((pPciCommonConf->VendorID == IGS_VENDOR_ID) &&
					(pPciCommonConf->DeviceID == IGS_DEVICE_ID)
					)
				{ 
					// we actually have our guy out there, but before we can 
					// talk to him, we need to wake him up
					if (!WakeUpChip()) {
						return FALSE;
					}

					*pulPciBusNum     = iBusNum;

					// mappping  physical interrupt to get interrupt id
					g_Irq = pPciCommonConf->u.type0.InterruptLine;   // not work!

					// ++ begin Puzzle Boot ROM workaround BUGBUG
					// the IGS5000 gets assigned Interrupt line 0, which is utterly bogus
					// after other, more clever workarounds failed, I'm resorting to this:
					// Most systems reserve IRQ0 for something pretty special (e.g. real-time clock)
					// so it's a pretty safe bet the getting back a zero means its wrong
					// In the case of Puzzle, it really MUST be 11 (the only line that' wired to
					// the riser card PCI slot).
					// kl 7/29/99

					if (g_Irq == 0) {
						g_Irq = 11;
					}
					// -- end workaround

					DEBUGMSG(1, (TEXT("Igs5050: Audio IRQ = %2xh\r\n"), g_Irq));
					
					//
					// No Interupt in config, write one
					//
					
					if (!MapInterruptToSystem(g_Irq, 0, &gIntrAudio)) {
						DEBUGMSG(1, (TEXT("Igs5050: Failed to map interrupt. IRQ= %02xh\r\n"), g_Irq));
						return FALSE;
					}


					// locate io address for 0x3ce
					PortAddress.LowPart  = ((ULONG)0x3ce);
					PortAddress.HighPart = 0; 
					MemType=1; // io space    
					
					success_flag = HalTranslateBusAddress( PCIBus, 0, PortAddress,
						&MemType, &MappedAddress
						);
					
					if (!success_flag)  	
						return FALSE;
					
					if ( MemType == 1 )
						(ULONG)portbase3ce = MappedAddress.LowPart;
					else
						portbase3ce = MmMapIoSpace(MappedAddress, 0xFF, FALSE);

					// turn on linear frame buffer
					IgsWIoPortI_0x3ce(0x33, (UCHAR)(IgsRIoPortI_0x3ce(0x33)|0x1)); 
					
					(ULONG)g_ulFrameBufferBase   = ((IgsRIoPortI_0x3ce(0x35))<<24);
					(ULONG)g_ulLinAddrBase       = (ULONG)g_ulFrameBufferBase + BIT_23;
					(ULONG)g_ulAudioAddrBase     = (ULONG)g_ulLinAddrBase + B_SEGMENT;
					
					// mapped Audio Segment
					PortAddress.LowPart  = ((ULONG)g_ulAudioAddrBase); 
					PortAddress.HighPart = 0; 
					MemType=0; // memory  space    
					
					success_flag = HalTranslateBusAddress( PCIBus, 0, PortAddress,
						&MemType, &MappedAddress
						);
					
					if (!success_flag)  	
						return FALSE;
					
					membase = MmMapIoSpace(MappedAddress, MAPPED_AUDIO_SIZE, FALSE);
					
					// allocate audio buffer and initialize it - place at the end of frame buffer.
                                        // Video driver will make sure not to allocate that region.
					PortAddress.LowPart  = (ULONG)g_ulFrameBufferBase + (0x400000 - BUFFER_SIZE);
					PortAddress.HighPart = 0; 
					MemType=0; // memory  space    

					success_flag = HalTranslateBusAddress( PCIBus, 0, PortAddress,
						&MemType, &MappedAddress
						);
					
					if (!success_flag)  	
						return FALSE;
					
					pvBufPage[0] = MmMapIoSpace(MappedAddress, BUFFER_SIZE, FALSE);
					pvBufPage[1] = ((PUCHAR)pvBufPage[0] + NODE_SIZE);
					
					return TRUE; 
				}  // if       
			}  // inneest for
		} //  inner for
	} // outtest for
	
	return FALSE;
}	


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID IgsWaveOutEndOfData()
{
    FUNC_WPDD("+PDD_WaveOutEndOfData");
	
    //
    // There is no more data coming...
    //
    v_fMoreData[WAPI_OUT] = FALSE;
	
	IgsWMemRegL(0xd000 + WER_STOP, (ULONG)(0x00000001 << g_ucChannel) ); // Stop Channel 
	IgsWMemRegL(0xd000 + WER_AINTEN, 0x0); // diable  address engine interrupt for all channels
	IgsWIoPortI_0x3ce( 0x12, 0x00);		   // turn off interupt 
	
    FUNC_WPDD("-PDD_WaveOutEndOfData");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID IgsWaveOutStart ( PWAVEHDR pwh)
{ 
	USHORT usVal = 0x0;
	double dVal  = 0.0;
	int    i=0, k=0, j=(pwh->dwBufferLength / NODE_SIZE) + 1;
	UINT   uTmp=0;
	BOOL   fPage=TRUE;
	
	FUNC_WPDD("+PDD_WaveOutStart");
	
	v_fMoreData[WAPI_OUT] = TRUE;        // we expect to have more data coming...
	v_nNextPage[WAPI_OUT] = 0;
	
	// fill audio buffers 
	IgsAudioFillBuffer(pwh, pvBufPage[0]);
	IgsAudioFillBuffer(pwh, pvBufPage[1]);
	
	// re-enable interrupt: offset the effect from display driver
	IgsWIoPortI_0x3ce( 0x12, 0x08);
	
	// enable designated channels of  BANK_A address engine interrupt
	IgsWMemRegL(0xd000 + WER_AINTEN,  (ULONG)(0x00000001 << g_ucChannel) );
	
	// reset   BANK_A address engine interrupt status
	IgsWMemRegL(0xd000 + WER_AINT,  (ULONG)(0x00000001 << g_ucChannel) );
	
	// set "format" and "sample rate" into chipset
	if (g_pwfx[WAPI_OUT]->wBitsPerSample == 16)          usVal |= 0xa000;  // select 16 bit &  signed
	if (g_pwfx[WAPI_OUT]->nChannels == 2)                usVal |= 0x4000;  // select "stereo" 
	usVal |= 0x1400;                                             // enable loop
	IgsWMemRegL(0xd000+WER_F0, ((IgsRMemReg(0xd000+WER_F0) & 0x0)|(ULONG)usVal) );
	
	usVal = (USHORT)(g_pwfx[WAPI_OUT]->nSamplesPerSec * 4096.0 / 48000.0);
	IgsWMemRegL(0xd000+WER_ESO_DELTA,  ((IgsRMemReg(0xd000+WER_ESO_DELTA) & 0xffff0000) | usVal) );
	
	// set envelope still mode
	IgsWMemRegL(0xd000+WER_EBUF1 , ((IgsRMemReg(0xd000+WER_EBUF1)& 0xcfffffff) | 0x30000000) );
	IgsWMemRegL(0xd000+WER_EBUF2 , ((IgsRMemReg(0xd000+WER_EBUF2)& 0xcfffffff) | 0x30000000) );
	
	// set start command into chipset 
	IgsWMemRegL(0xd000+WER_START, (ULONG)(0x00000001 << g_ucChannel) );
	
	FUNC_WPDD("-PDD_WaveOutStart");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID IgsWaveOutContinue ( PWAVEHDR pwh )
{
	ULONG ulTmp;

    FUNC_VERBOSE("+PDD_WaveOutContinue");

	// Get current buffer being played 
	ulTmp = IgsRMemReg(0xd000 + WER_CSPF); // presumption: only channel 0 running
	IgsAudioFillBuffer(pwh, pvBufPage[!ulTmp]);

    v_nNextPage[WAPI_OUT] = (!v_nNextPage[WAPI_OUT] ? 1 : 0);

    FUNC_VERBOSE("-PDD_WaveOutContinue");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT IgsWaveOutRestart ( PWAVEHDR pwh )
{
    FUNC_VERBOSE("+PDD_WaveOutRestart");

    IgsWaveOutStart (pwh);

    FUNC_VERBOSE("-PDD_WaveOutRestart");
    return(MMSYSERR_NOERROR);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT IgsWaveOutPause ( VOID )
{
    FUNC_VERBOSE("+PDD_WaveOutPause");

    IgsWaveOutEndOfData();

    FUNC_VERBOSE("-PDD_WaveOutPause");
    return(MMSYSERR_NOERROR);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID IgsWaveOutStop()
{ 
	FUNC_WPDD("+IgsWaveOutStop");
	
	// set stop command into chipset 
	IgsWMemRegL(0xd000+WER_STOP, (ULONG)(0x00000001 << g_ucChannel) );
	
	fInUse = FALSE;
	
	FUNC_WPDD("-IgsWaveOutStop");
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID IgsWaveStandby( WAPI_INOUT apidir)
{
    //
    // We are going to be idle. so power off what we can.
    //
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT IgsWaveGetDevCaps( WAPI_INOUT apidir, PVOID pCaps, UINT  wSize)
{
    PWAVEINCAPS pwic = pCaps;
    PWAVEOUTCAPS pwoc = pCaps;
	
    MMRESULT mmRet = MMSYSERR_NOERROR;
	
    FUNC_WPDD("+PDD_WaveGetDevCaps");
	
    if (pCaps == NULL)  
	{
        //
        // If pCaps == NULL, we are requesting if the driver PDD is capable of
        // this mode at all. In other words, if APIDIR == WAPI_IN,  we return
        // no error if input is supported, and MMSYSERR_NOTSUPPORTED otherwise.
        // Since Odo has both, we return MMSYSERR_NOERROR regardless.
        //
        return(MMSYSERR_NOERROR);
    }
	
    //
    // Fill in the DevCaps here.
    //    
    pwoc->wMid = MM_MICROSOFT;
    pwoc->wPid = (apidir == WAPI_OUT ? 24 : 23);// generic in or out...
    pwoc->vDriverVersion = 0x0100;
    wsprintf (pwoc->szPname, TEXT("CE/PC \"IGS5050\" Audio (%hs)"), __DATE__);
    pwoc->wChannels = 2;
    pwoc->dwFormats =   WAVE_FORMAT_1M08 | WAVE_FORMAT_1M16 |
						WAVE_FORMAT_1S08 | WAVE_FORMAT_1S16 |
						WAVE_FORMAT_2M08 | WAVE_FORMAT_2M16 |
						WAVE_FORMAT_2S08 | WAVE_FORMAT_2S16 |
						WAVE_FORMAT_4M08 | WAVE_FORMAT_4M16 |
						WAVE_FORMAT_4S08 | WAVE_FORMAT_4S16;
	
    if (apidir == WAPI_OUT) 
	{
        pwoc->dwSupport = WAVECAPS_VOLUME;
    }
	
    FUNC_WPDD("-PDD_WaveGetDevCaps");
	
    return(mmRet);
}    


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT IgsWaveOpen( WAPI_INOUT apidir, LPWAVEFORMATEX lpFormat, BOOL fQueryFormatOnly)
{
	MMRESULT mmRet = MMSYSERR_NOERROR;
	USHORT usVal   = 0x0;
	double dVal    = 0.0;
	USHORT usTmp;
	
	FUNC_WPDD("+PDD_WaveOpen");
	
	//
	// Allow PCM, mono or stereo, 8 or 16 bit at 11k , 22k, or 44.1k Hz
	//
	if ( (lpFormat->wFormatTag     != WAVE_FORMAT_PCM)                            ||
		 (lpFormat->nChannels      != 1 && lpFormat->nChannels != 2)              ||
		 (lpFormat->nSamplesPerSec != 11025 && lpFormat->nSamplesPerSec !=  8000  &&
          lpFormat->nSamplesPerSec != 22050 && lpFormat->nSamplesPerSec != 44100) ||
		 (lpFormat->wBitsPerSample != 16 && lpFormat->wBitsPerSample  != 8)
		)
    { 
		mmRet = WAVERR_BADFORMAT; 
	}
	
	if (fQueryFormatOnly || mmRet != MMSYSERR_NOERROR)          
		goto EXIT; 
	
    //
    // NOTE : If the hardware can only support either input or output one 
    // at a time, the driver should return MMSYSERR_ALLOCATED here.
    //
    if (fInUse)
    { 
		mmRet = MMSYSERR_ALLOCATED;
		goto EXIT;
    }
    fInUse = TRUE;
	
	// init LBA, CSO, ESO, and Buffer
	IgsWMemRegL(0xd000 + WER_LBA, (ULONG)(0x400000 - BUFFER_SIZE));
	
	usTmp = BUFFER_SIZE;
	
	if (lpFormat->nChannels == 2)         
		usTmp /= 2;  
	if (lpFormat->wBitsPerSample == 16)   
		usTmp /= 2; 
	
	IgsWMemReg(0xd000 + WER_ESO_DELTA + 2 , (USHORT)(usTmp - 1));
	
	IgsWMemRegL(0xd000 + WER_CSO_ALPHA_FMS, (ULONG)0x0);
	
	//
	// Set Adress Engine Interrupt 
	//
	
	// enable designated channels of  BANK_A address engine interrupt
	IgsWMemRegL(0xd000 + WER_AINTEN,  (ULONG)(0x00000001 << g_ucChannel) );
	
	// reset   BANK_A address engine interrupt status
	IgsWMemRegL(0xd000 + WER_AINT, 0x0);
	
	//
	// prepare for play
	//
	
	// set to current channel
	IgsWMemRegL(0xd000+WER_GC_CIR, (ULONG)((IgsRMemReg(0xd000+WER_GC_CIR)&0xffffffc0) | g_ucChannel) );
	
	// enable MIDLP_IE, ENDLP_IE 
	IgsWMemRegL(0xd000+WER_GC_CIR, (ULONG)(IgsRMemReg(0xd000+ WER_GC_CIR) | 0x3000) );
	
	// set  GLOBAL Volume        
	IgsWMemRegL(0xd000+WER_MUS_WAV_VOL, g_ulMusWavVolume);
	
	// Open with the given format. Choose a playback rate from this.
    g_pwfx[apidir] = lpFormat;
	
    if (g_pwfx[apidir]->wBitsPerSample == 8)
		g_pcmtype[apidir] = (g_pwfx[apidir]->nChannels == 1) ?  PCM_TYPE_M8 : PCM_TYPE_S8;
    else  
		g_pcmtype[apidir] = (g_pwfx[apidir]->nChannels == 1) ?  PCM_TYPE_M16 : PCM_TYPE_S16;
    
EXIT:
    FUNC_WPDD("-PDD_WaveOpen");
    return(mmRet);
}    


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ULONG IgsGetVolume()
{
	ULONG regvol,winvol,rvol,lvol,rindex,lindex;
	UCHAR igsrvol,igslvol;

//>>>>>>>>>>>>>>>>>>>>> work this 
	// Note: this code dosen't match the spec which appears to have music 
	//       volume and wave volume words reversed 

	regvol = IgsRMemReg(0xd000+WER_MUS_WAV_VOL) & 0xFFFF0000;

	igsrvol = (UCHAR)((regvol & 0xFF000000) >> 24);
	igslvol = (UCHAR)((regvol & 0x00FF0000) >> 16);

#define NELEMS(a) (sizeof(a)/sizeof((a)[0]))

	for ( rindex = 0; igsrvol > volumeLUT[rindex] && rindex < NELEMS(volumeLUT); rindex++);
	for ( lindex = 0; igslvol > volumeLUT[lindex] && lindex < NELEMS(volumeLUT); lindex++);

	rvol = 0xFF000000 - (rindex << 27);
	lvol = 0x0000FF00 - (lindex << 11);

	winvol = rvol | lvol;

	return winvol;
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void IgsSetVolume(ULONG vol)
{
	ULONG rvol,lvol;
	UCHAR rindex,lindex;


//>>>>>>>>>>>>>>>>>>>>> work this 
	// Note: this code dosen't match the spec which appears to have music 
	//       volume and wave volume words reversed 

	rindex = (UCHAR)((0xFF000000 - (vol & 0xFF000000)) >> 27);
	lindex = (UCHAR)((0x0000FF00 - (vol & 0x0000FF00)) >> 11);

	rvol = volumeLUT[rindex] << 24;
	lvol = volumeLUT[lindex] << 16;

	g_ulMusWavVolume = (IgsRMemReg(0xd000+WER_MUS_WAV_VOL) & 0x0000FFFF) | rvol | lvol; 

    IgsWMemRegL(0xd000+WER_MUS_WAV_VOL, g_ulMusWavVolume);
}


// -----------------------------------------------------------------------------
//                       PDD_AudioGetInterruptType
// -----------------------------------------------------------------------------
AUDIO_STATE PDD_AudioGetInterruptType( VOID)
{ 
	ULONG ulStatus;
	
	ulStatus = IgsRMemReg(0xd000 + WER_MISCINT); 
	if ((ulStatus & 0x010000ff) == 0)    
		return  AUDIO_STATE_IGNORE;  // not for Igs Aduio interrupt
	
	return ( (v_fMoreData[WAPI_OUT] == TRUE) ?  AUDIO_STATE_OUT_PLAYING : AUDIO_STATE_OUT_STOPPED);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
BOOL PDD_AudioInitialize( DWORD dwIndex )
{ 
	int i;
	ULONG ulTmp = 0;

	FUNC_WPDD("PDD_AudioInitialize");
	
	// check if igs 505x pci card exist?
	if (!IsIgsPciAudioExist(&ulPciBusNum, &PciSlotNum, &PciCommonConf))
	{ 
		DEBUGMSG(1, (TEXT("PDD_AudioInitialize : No Igs505x audio chip Found\r\n")));
		return FALSE;
	}
	
	// save old WER_AINT  and  WER_MISCINT
	ulOldMiscInt = IgsRMemReg(0xd000 + WER_MISCINT); 
	ulOldAInt    = IgsRMemReg(0xd000 + WER_AINT);
	
	// enable audio pins
	IgsWIoPortI_0x3ce( 0xbf, 0x01);
	IgsWIoPortI_0x3ce( 0xba, 0x20);
	IgsWIoPortI_0x3ce( 0xbb, 0x08);
	IgsWIoPortI_0x3ce( 0xbf, 0x0);
	
	// enable frame buffer mode
	IgsWMemReg(0xc02a, 0x01);
	
	//
	// Initialize Mixer
	//
	for (i = 0; i < NUM_OF_MIXER_TO_BE_INIT ; i++)
	{
		WriteMixerReg(Mixers[i].ucIdx , Mixers[i].usValue);
	}
	
	return TRUE;
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID PDD_AudioPowerHandler( BOOL power_down )
{
    //
    // Handle any power up/down issues here. 
    //
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
DWORD PDD_AudioMessage( UINT uMsg, DWORD dwParam1, DWORD dwParam2 )
{
    //
    // Handle any custom WAVEIN or WAVEOUT messages here
    //
    return(MMSYSERR_NOTSUPPORTED);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
VOID PDD_AudioDeinitialize( VOID)
{
	//
	// The driver is being unloaded. Deinitialize. Free up memory, etc.
	//
	
	MmUnmapIoSpace(membase, MAPPED_AUDIO_SIZE);
}


// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
MMRESULT PDD_WaveProc(WAPI_INOUT apidir, DWORD dwCode, DWORD dwParam1, DWORD dwParam2)
{ 
	MMRESULT mmRet = MMSYSERR_NOERROR;
	
    if (apidir == WAPI_IN) {
        //
        // This sample driver doesn't support input. Several of the functions
        // are setup to work for both input and output. We pre-empt them all
        // at this point to reject all input requests. This driver could be
        // upgraded later to support input by using the parameter APIDIR to 
        // determine input versus output.
        //
        return(MMSYSERR_NOTSUPPORTED);
    }
	
    switch (dwCode)
	{  
	case WPDM_CLOSE:
		fInUse =FALSE;
		break;
		
	case WPDM_CONTINUE:
		IgsWaveOutContinue((PWAVEHDR) dwParam1);
		break;
		
	case WPDM_GETDEVCAPS:
		mmRet = IgsWaveGetDevCaps(apidir, (PVOID) dwParam1, (UINT) dwParam2);
		break;
		
	case WPDM_OPEN:
		mmRet = IgsWaveOpen(apidir, (LPWAVEFORMATEX) dwParam1, (BOOL) dwParam2);
		break;
		
	case WPDM_STANDBY:
		IgsWaveStandby (apidir);
		break;
		
	case WPDM_START:
		IgsWaveOutStart((PWAVEHDR) dwParam1);
		break;
		
	case WPDM_STOP:
		IgsWaveOutStop();
		break;
		
	case WPDM_PAUSE:
		IgsWaveOutPause();
		break;
		
	case WPDM_ENDOFDATA:
		IgsWaveOutEndOfData();
		break;
		
	case WPDM_RESTART:
		IgsWaveOutRestart((PWAVEHDR) dwParam1);
		break;
		
	case WPDM_GETVOLUME:
		*((PULONG) dwParam1) = IgsGetVolume();
		break;
		
	case WPDM_SETVOLUME:
		IgsSetVolume(dwParam1);
		break;
		
	default :
		mmRet = MMSYSERR_NOTSUPPORTED;
		break;
    }
	
    return(mmRet);
}


// ---------------------------------------------------------------------------
//
// Routine for powering on Audio hardware.  Is called from within power
// handler, so should not make any system calls.
//
// ---------------------------------------------------------------------------
VOID AudioPowerOn()
{
   v_nNextPage[WAPI_OUT] = 0;
   v_nNextPage[WAPI_IN]  = 2;
   v_fMoreData[WAPI_OUT] = FALSE;
   v_fMoreData[WAPI_IN]  = FALSE;
}





