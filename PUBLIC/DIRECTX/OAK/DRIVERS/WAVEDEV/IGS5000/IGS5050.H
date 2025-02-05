
// for IGS CYBERPRO 5050 CARD with audio
#define IGS_VENDOR_ID		0x10ea
#define IGS_DEVICE_ID		0x5050



#define AUDIO_SEGMENT		0xbd000
#define B_SEGMENT			0xb0000
#define BIT_23				0x800000
#define MAPPED_IO_SIZE		(4 * 1024 * 1024)  	// block size: 4MB  
#define MAPPED_AUDIO_SIZE	0x100000  			// block size: 1mB  



// node and buffer
#define NODE_SIZE 			1024 * 2     		// 2k
#define BUFFER_SIZE			(NODE_SIZE * 2)  	// 4k

#define DEFAULT_VOLUME		0x00000000       	// music





 // Mixer register behavior
#define BUSY_WRITING       0x8000  // for ac97 mixer write register
#define BUSY_READING       0x8000  // for ac97 mixer read register

typedef const struct _MIXER_UNIT
                     { UCHAR   ucIdx;
                       USHORT  usValue; 
		     } MIXER_UNIT;

// Mixer Register
#define	MASTER_VOLUME		0x02		
#define	PCM_OUT_VOLUME		0x18		
#define	RECORD_SELECT 		0x1a		




#define OPL3_TIMER_IRQ_ON	0x20000   // bit 17

#define SBR0				0x10      // a byte
#define SBR1				0x11      // a byte




//
// WER : Wave Engine Register (Bank A Only)
//

#define WER_CSPF  			0x90      //current Sample Position Flag
#define WER_START 			0x80      // 
#define WER_STOP   	        0x84      // 
#define WER_DLY   			0x88      // 
#define WER_CSPF  			0x90      // 
#define WER_AINT  			0x98      // address engine interrupt status
#define WER_AINTEN			0xa4      // address engine interrupt enable
#define WER_GC_CIR			0xa0      // global control and channel idx
#define WER_MUS_WAV_VOL 	0xa8      // global music and wave volume set
#define WER_MISCINT     	0xb0      // Miscellaneous Int & Status     
#define WER_CSO_ALPHA_FMS	0xe0      // CSO, ALPHA, and FMS            
#define WER_LBA         	0xe4      // LBA                            
#define WER_ESO_DELTA   	0xe8      // ESO and DELTA                  
#define WER_F0          	0xf0      // LFO_INIT                      
#define WER_EBUF1       	0xf4      // EBUF1 
#define WER_EBUF2       	0xf8      // EBUF2 





//
// Miscellaneous Interrupts Status  (Bank A Only)
//

#define PLAY_URUN_IRQ  		0x01		// PlayBack FIFO is underrun                                 
#define REC_ORUN_IRQ       	0x02   	 	// Recording overrun 
#define SB_IRQ   	        0x04   	 	// Sound Blaster 
#define MPU401_IRQ 			0x08    	// MPU401 

#define OPL3_TIMER_IRQ 		0x010		// OPL3 TIMER                                                
#define ADDR_ENGINE_IRQ    	0x020   	// Wave-table Address  Engine
#define ENVELOPE_ENGINE_IRQ 0x040   	// Wave-table Envelope Engine  
#define SAMPLE_TIMER_IRQ	0x080   	// Sample Timer
#define AC97_GPIO_IRQ	    0x01000000 	// AC97 GPIO   








