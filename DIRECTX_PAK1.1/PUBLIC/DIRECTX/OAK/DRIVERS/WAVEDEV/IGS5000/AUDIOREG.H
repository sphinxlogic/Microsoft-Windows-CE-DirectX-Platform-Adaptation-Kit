//
//
//  IGS technologies, Inc.
//
//
//
//
#ifndef _AUDIOREG_H_
#define _AUDIOREG_H_

#define AUDIO_BASE                  0x220
#define AUDIO_DMA_CHANNEL           5
#define AUDIO_DMA_BUFFER_BASE_PA    0x001FC000

#define MIXER_ADDR                  0x4
#define MIXER_DATA                  0x5
#define DSP_RESET                   0x6
#define DSP_READ_PORT               0xA
#define DSP_WRITE_PORT              0xC
#define DSP_DATA_AVAIL              0xE

#define MASTER_VOLUME_LEFT          0x30
#define MASTER_VOLUME_RIGHT         0x31
#define VOICE_VOLUME_LEFT           0x32
#define VOICE_VOLUME_RIGHT          0x33
#define MIDI_VOLUME_LEFT            0x34
#define MIDI_VOLUME_RIGHT           0x35
#define CD_VOLUME_LEFT              0x36
#define CD_VOLUME_RIGHT             0x37
#define LINE_VOLUME_LEFT            0x38
#define LINE_VOLUME_RIGHT           0x39
#define MIC_VOLUME                  0x3A

#define DSP_BLOCK_SIZE              0x48
#define DSP_HALT_AUTOINIT_DMA       0xD9
#define DSP_VERSION                 0xE1

#define DSP_READY                   0xAA

#endif
