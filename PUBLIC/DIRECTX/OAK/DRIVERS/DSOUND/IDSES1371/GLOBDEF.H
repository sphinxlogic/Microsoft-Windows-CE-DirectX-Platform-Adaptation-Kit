// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1998  Microsoft Corporation
//  
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @enum    PCM_TYPE | Enumeration of standard PCM data types.
//
// @field   PBYTE | pData | Pointer to just the data buffer which contains 
//          the waveform data to be played.  
//
// @field   ULONG | nBufferSize |
//          Size, in number of bytes, of the buffer to play.
//
// @field   ULONG | nBufferPosition |
//          Current position, in number of bytes in the user buffer.
//
// @field   BOOL | fLooping |
//          If TRUE, the buffer should be played in an infinite loop.
//
// -----------------------------------------------------------------------------

typedef enum {
     PCM_TYPE_M8,
     PCM_TYPE_M16,
     PCM_TYPE_S8,
     PCM_TYPE_S16
} PCM_TYPE, *PPCM_TYPE;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_8_MONO | Single sample from 8-bit mono PCM data stream.
//
// @field   UINT8 | sample | Unsigned 8-bit sample
//
// -----------------------------------------------------------------------------
typedef struct  {
    UINT8 sample;              // Unsigned 8-bit sample
} SAMPLE_8_MONO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_16_MONO | Single sample from 16-bit mono PCM data stream.
//
// @field   INT16 | sample | Unsigned 16-bit sample
//
// -----------------------------------------------------------------------------
typedef struct  {
    INT16 sample;              // Signed 16-bit sample
} SAMPLE_16_MONO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_8_STEREO | Single sample from 8-bit stereo PCM data stream.
//
// @field   UINT8 | sample_left | Unsigned 8-bit sample from left channel.
//
// @field   UINT8 | sample_right | Unsigned 8-bit sample from right channel.
//
// -----------------------------------------------------------------------------
typedef struct  {
    UINT8 sample_left;         // Unsigned 8-bit sample
    UINT8 sample_right;        // Unsigned 8-bit sample
} SAMPLE_8_STEREO;


// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  SAMPLE_16_STEREO | Single sample from 16-bit stereo PCM data stream.
//
// @field   UINT16 | sample_left | Unsigned 16-bit sample from left channel.
//
// @field   UINT16 | sample_right | Unsigned 16-bit sample from right channel.
//
// -----------------------------------------------------------------------------
typedef struct  {
    INT16 sample_left;         // Signed 16-bit sample
    INT16 sample_right;        // Signed 16-bit sample
} SAMPLE_16_STEREO;



// -----------------------------------------------------------------------------
//
// @doc     EX_AUDIO_DDSI
//
// @struct  PCM_SAMPLE | Union that allows access to any of the Sample Types
//
// @field   SAMPLE_8_MONO | m8 | <t SAMPLE_8_MONO>
// @field   SAMPLE_16_MONO | m16 | <t SAMPLE_16_MONO>
// @field   SAMPLE_8_STEREO | s8 | <t SAMPLE_8_STEREO>
// @field   SAMPLE_16_STEREO | s16 | <t SAMPLE_16_STEREO>
//
// -----------------------------------------------------------------------------
typedef union {

     SAMPLE_8_MONO m8;
     SAMPLE_16_MONO m16;
     SAMPLE_8_STEREO s8;
     SAMPLE_16_STEREO s16;

} PCM_SAMPLE, *PPCM_SAMPLE;



//////////////////////////////////////////////////////////////////////////////
/*****************************************************************************
Helper class for entering and leaving critical sections.
*****************************************************************************/
//////////////////////////////////////////////////////////////////////////////
class CriticalSectionAcquisition
{
public:

	inline CriticalSectionAcquisition( CRITICAL_SECTION * pCS )
	{
		m_pCS = pCS;
		EnterCriticalSection( m_pCS );
	}

	inline ~CriticalSectionAcquisition()
	{
		LeaveCriticalSection( m_pCS );
	}

private:

	CRITICAL_SECTION * m_pCS;

};




// Use define to retirieve Irq and IO Base from registry instead of card
//#define ENSONIQ_USE_REGSISTRY 

// Use memory locations located in config.bib, for contigeuos memory larger than 4k
//#define ENSONIQ_USE_PREALLOCATEDMEMORY

//
// Defined and allocated in Config.bib
//
#ifdef ENSONIQ_USE_PREALLOCATEDMEMORY
#ifdef CEPC
#define AUDIO_DMA_BUFFER_BASE_PA    0x001FC000
#endif

#ifdef PUZZLE
#define AUDIO_DMA_BUFFER_BASE_PA    0X01FD0000
#endif

#ifdef ALTOONA
#define AUDIO_DMA_BUFFER_BASE_PA    0X03BB0000
#endif

//WARNING:  This define is place here only to prevent build breaks
//          If one is using a preallocated buffer a hardcode defined
//          physical memory address must be provided in config.bib
#ifndef AUDIO_DMA_BUFFER_BASE_PA
#define AUDIO_DMA_BUFFER_BASE_PA    0x00000000
#endif
#endif

BOOL ReservePhysicalMem(DWORD dwSize, ULONG *pPhysAddr, ULONG *pVirtAddr);


// ----- you can Fine Tune this value ---- \\

//size of phys mem allocated at driver load time for
#define	PRE_ALLOC_BUFFER_SIZE_DAC0	0x2000		//8kb	
#define	PRE_ALLOC_BUFFER_SIZE_DAC1	0x10000		//64kb	needed by dmusic
#define	PRE_ALLOC_BUFFER_SIZE_ADC	0x8000		//32kb	

// ------------------------------------------ //
