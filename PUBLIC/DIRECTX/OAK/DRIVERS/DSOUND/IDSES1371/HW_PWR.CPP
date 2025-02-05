// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//  
// -----------------------------------------------------------------------------

// hw_pwr.cpp
//
// functions for controlling power management features of ES137x
//

#include "es1371.h"

#define STR_MODULENAME "CES1371: "

ULONG
CES1371::SetPowerState( ULONG ulPowerState )
{
  UCHAR ucReg;
  ULONG ulWrite;
  BOOLEAN fPowerCycled;

  // are we changing state
  if ( m_ulPowerState == ulPowerState )
    return ulPowerState;

  if ( 0 == ulPowerState )  // waking up
  {
    // save the current power state
    // do this first as a lot of routines wont really write
    // to the hardware if this shows we are powered down.
    m_ulPowerState = ulPowerState;        

    // check if the chip lost its state while in power down mode
    ucReg = READ_PORT_UCHAR( m_pPciAddr + ES1371_bDEVCTL_OFF );
    if ( fPowerCycled = (0x03 != ucReg) )
    {
      // the chip lost its state during the power down mode
      // restore all register values
      RetoreES1371Context();
    }
    
    // Reenable ES1371 Device blocks
    ucReg = 0x00;
    // turn the clocks back on
    HwRegRMW( ES1371_bDEVCTL_OFF, 0xff, ucReg );
    // then restore the saved register state
    HwRegRMW( ES1371_bDEVCTL_OFF, 0xff, m_ucDeviceControl );

    // Reenable Codec
    if ( !fPowerCycled )
      Codec_SetPowerState( AC97_PWR_D0 );
    else
    {
      InitCodec();
      m_ulCodecPowerState = AC97_PWR_D0;
    }

    // rewrite all the Codec registers
    Codec_RestoreRegisterState( m_usCRegsPMContext );

    // restore the SRC registers
    SRCRestoreRegisterState();

    // enable the SRC
    ulWrite = 0x00000000;
    HwRegRMW( ES1371_dSRCIO_OFF, 0xffffffff, ulWrite ); 

    // start the DMA running if needed
    HwRegRMW( ES1371_bSERCTL_OFF, 0xff, m_ucSerialControl );
  }
  else // shutting down
  {
    ucReg = 0x00;
    // save the state of the serial control register
    m_ucSerialControl = HwRegRMW( ES1371_bSERCTL_OFF, 0x00, ucReg );

    // pause the DMA so things are quiet
    ucReg = ES1371_SERCTL_DAC0PAUSE|ES1371_SERCTL_DAC1PAUSE;
    HwRegRMW( ES1371_bSERCTL_OFF,
              ES1371_SERCTL_DAC0IE|ES1371_SERCTL_DAC1IE|ES1371_SERCTL_ADCIE|
              ES1371_SERCTL_DAC0PAUSE|ES1371_SERCTL_DAC1PAUSE,
              ucReg );

    // disable the SRC
    ulWrite = SRC_DISABLE;
    HwRegRMW( ES1371_dSRCIO_OFF, 0xffffffff, ulWrite ); 

    // save the SRC registers
    SRCSaveRegisterState( );

    // save the Codec registers
    Codec_SaveRegisterState( m_usCRegsPMContext );

    // power down the Codec
    Codec_SetPowerState( AC97_PWR_D3 );

    // save the state of the enable bits
    m_ucDeviceControl = HwRegRMW( ES1371_bDEVCTL_OFF, 0x00, ucReg );

    // save all the ES1371 registers
    SaveES1371Context();

    // power down es1371
    ucReg = ES1371_DEVCTL_PCICLK_DS | ES1371_DEVCTL_XTALCLK_DS ;
    HwRegRMW( ES1371_bDEVCTL_OFF, 0xff, ucReg );

    // save the current power state
    m_ulPowerState = ulPowerState;
  }

  return m_ulPowerState;
}


void 
CES1371::SaveES1371Context( void )
{
  ULONG i, page;

  // save the direct registers first
  for ( i = ES1371_bDEVCTL_OFF; i < ES1371_MEMBASE_OFF; i+=4 )
  {
      m_ulDRegsPMContext[i/4] = READ_PORT_ULONG ( (PULONG)(m_pPciAddr + i) );
  }

  // save the indirect registers
  for ( page = ES1371_DAC0RAM_PAGE;  page <= ES1371_FIFO1_PAGE; page++ )
  {
      // set the page register
      WRITE_PORT_UCHAR(m_pPciAddr + ES1371_bMEMPAGE_OFF, UCHAR(page) );

      // read this pages data
      for ( i = 0; i < 4; i++ )
      {
          m_ulIRegsPMContext[ page * 4 + i] = 
                   READ_PORT_ULONG ( (PULONG)(m_pPciAddr + (i*4) + 0x30) );
      }
  }

  // put the page register back where it was when we got here
  WRITE_PORT_UCHAR( m_pPciAddr + ES1371_bMEMPAGE_OFF, 
                    UCHAR(m_ulDRegs[ES1371_bMEMPAGE_OFF/4]) & 0x0f );

}

void 
CES1371::RetoreES1371Context( void )
{
  ULONG i, page;


  // turn on the clocks 
    
  // restore the indirect registers
  for ( page = ES1371_DAC0RAM_PAGE;  page <= ES1371_FIFO1_PAGE; page++ )
  {
      // set the page register
      WRITE_PORT_UCHAR(m_pPciAddr + ES1371_bMEMPAGE_OFF, UCHAR(page) );

      // set this pages data
      for ( i = 0; i < 4; i++ )
      {
          WRITE_PORT_ULONG( PULONG(m_pPciAddr + (i*4) + 0x30), 
                            m_ulIRegsPMContext[ page * 4 + i] );
      }
  }

  // restore the direct registers 
  for ( i = ES1371_bDEVCTL_OFF; i < ES1371_MEMBASE_OFF; i+=4 )
  {
      if ( ES1371_dCODECCTL_OFF == i )
        continue;

      WRITE_PORT_ULONG( PULONG(m_pPciAddr + i), m_ulDRegsPMContext[i/4] );
  }

  // make the first write to the page register take
  m_ulDRegs[ES1371_bMEMPAGE_OFF/4] = 0xffffff00;
}

void
CES1371::SRCSaveRegisterState( void )
{
    USHORT i;

    // save off the active SRC registers
    for ( i = 0; i < 20; i++ )
          m_usSRCRegsPMContext[i] = SRCRegRead( i + SRC_ADC_VOL_L );
}

void
CES1371::SRCRestoreRegisterState( void )
{
    USHORT i;

//    // clear the SRC RAM
//    for( i = 0; i < 0x80; ++i )
//          SRCRegWrite(i, 0 );

    InitSRC( FALSE );

    // write the active registers back in
    for ( i = 0; i < 20; i++ )
          SRCRegWrite( i + SRC_ADC_VOL_L, m_usSRCRegsPMContext[i]);
}
