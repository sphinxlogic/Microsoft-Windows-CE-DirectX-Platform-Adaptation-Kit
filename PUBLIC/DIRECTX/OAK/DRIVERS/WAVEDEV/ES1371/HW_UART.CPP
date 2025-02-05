//
// Copyright 1998-1999 (c) Creative Labs, Malvern.  All rights reserved.
// Portions copyright 1999 (c) Microsoft Corporation.  All rights reserved.
//
// $Workfile: hw_uart.cpp $   
//
// Author: David Sowa
//
//
// Comments: This is the UART control code for the CES1371 object.  
//
// $History: hw_uart.cpp $
// 
// *****************  Version 3  *****************
// User: Dsowa        Date: 12/29/98   Time: 10:01a
// Updated in $/ac97/wdm/1371
// Use the standard register cache to save and restore the page register.
// 
// *****************  Version 2  *****************
// User: Dsowa        Date: 9/10/98    Time: 1:21p
// Updated in $/AC97/wdm/1371
// Added MP protection for Indirect IO reads and writes.
// 
// So MIDI and Wave work together now.
// 
// *****************  Version 1  *****************
// User: Dsowa        Date: 8/27/98    Time: 4:41p
// Created in $/AC97/wdm/1371
// the UART routines for the CES1371 object
// 

#include "es1371.h"

#define STR_MODULENAME "CES1371: "


void CES1371::InitUART()
{
   UCHAR ucPortVal;
   ULONG page;
   ULONG portoff;

   // disable the UART
   ucPortVal = 0x00;
   HwRegRMW( ES1371_bDEVCTL_OFF, ES1371_DEVCTL_UART_EN, ucPortVal );

   // send the UART reset command
   WRITE_PORT_UCHAR( (PUCHAR)( m_pPciAddr + ES1371_bUARTCSTAT_OFF ), ES1371_UART_CTL );

   // clear the UART fifo RAM
   for ( page = ES1371_FIFO0_PAGE; page <= ES1371_FIFO1_PAGE; page++ )
   {
      for ( portoff = 0x30; portoff <= 0x3c; portoff += 4 )
      {
        HwPagedIOWrite( UCHAR(page), portoff, 0x00000000 ); 
      }
   }

   // Reset the soft Rx Position pointer
   m_ulUARTRxPointer = 0;

   // Reset the Rx Software Queue ponters
   m_ulUARTRxQHead = 0;
   m_ulUARTRxQTail = 0;

   // Reset the Tx Software Queue ponters
   m_ulUARTTxQHead = 0;
   m_ulUARTTxQTail = 0;

}

ULONG CES1371::UARTEnable( BOOLEAN fRx )
{
   UCHAR ucPortVal = 0;

   if ( fRx && (m_ulUARTState & UART_RX_ENABLE) )
       return m_ulUARTState;

   if ( !fRx && (m_ulUARTState & UART_TX_ENABLE) )
       return m_ulUARTState;

   if ( !(m_ulUARTState & UART_ENABLE) )
   {
       // We're not enabled yet so start with Init
       InitUART();

       // enable the UART
       ucPortVal = ES1371_DEVCTL_UART_EN;
       HwRegRMW( ES1371_bDEVCTL_OFF, ES1371_DEVCTL_UART_EN, ucPortVal );

       m_ulUARTState |= UART_ENABLE; 

       // start the Rx interrupt so the Recieve queue stays in
       // sync all the time
       // since the UART wasn't enabled we know that we wern't Txing
       ucPortVal = ES1371_UART_RXINTEN;

       WRITE_PORT_UCHAR( (PUCHAR)( m_pPciAddr + ES1371_bUARTCSTAT_OFF ), ucPortVal );
   } 

   if ( fRx )
   {
       // Now set the Rx Enabled flag so the bytes get sent up the chain
       m_ulUARTState |= UART_RX_ENABLE;
   }
   else
   {
       // Now set the Tx Enable flag
       m_ulUARTState |= UART_TX_ENABLE;
   }

   return m_ulUARTState;
}

ULONG CES1371::UARTDisable( BOOLEAN fRx )
{
   UCHAR ucPortVal;

   if ( fRx && (m_ulUARTState & UART_RX_ENABLE) )
   {
     m_ulUARTState  &= ~UART_RX_ENABLE;
   }

   if ( !fRx && (m_ulUARTState & UART_TX_ENABLE) )
   {
     m_ulUARTState  &= ~UART_TX_ENABLE;
   }

   // Is Either MIDI section in use?
   if ( m_ulUARTState & (UART_RX_ENABLE | UART_TX_ENABLE) )
     return m_ulUARTState;
   
   // Are we currently enabled?
   if ( !(m_ulUARTState & UART_ENABLE) )
     return m_ulUARTState;

   // Turn off the Rx Interrupt that is enabled all the time
   ucPortVal = 0x00;
   HwRegRMW( ES1371_bUARTCSTAT_OFF, 0xff, ucPortVal );

   // disable the UART
   ucPortVal = 0x00;
   HwRegRMW( ES1371_bDEVCTL_OFF, ES1371_DEVCTL_UART_EN, ucPortVal );

   // reset the state flag
   m_ulUARTState = 0x00;

   return m_ulUARTState;
}

UCHAR CES1371::UARTWaitForTxRdy()
{
    ULONG ulTimeout = 0x0100;
    UCHAR ucCSTAT;

    ucCSTAT = READ_PORT_UCHAR((PUCHAR)(m_pPciAddr + ES1371_bUARTCSTAT_OFF));

    while ( !(ucCSTAT & ES1371_UART_TXRDY ) && ulTimeout-- )
    {
       ucCSTAT = READ_PORT_UCHAR((PUCHAR)(m_pPciAddr + ES1371_bUARTCSTAT_OFF));
    }

    return ucCSTAT;
}

UCHAR CES1371::UARTGetStatus()
{
  return ( READ_PORT_UCHAR((PUCHAR)(m_pPciAddr + ES1371_bUARTCSTAT_OFF)) );
}


ULONG CES1371::UARTReadRxBytes()
{
  ULONG ulRxBytes = 0;
  UCHAR ucCurPage;
  UCHAR ucRxRegData;
  ULONG ulFIFOPos;
  USHORT usRxFifoData;

  // calculate the page of the current "FIFO" position
  if ( m_ulUARTRxPointer > 3 )
    ucCurPage = ES1371_FIFO1_PAGE;
  else
    ucCurPage = ES1371_FIFO0_PAGE;

  // set the page register
  // since this is in the ISR we will set the page register directly
  if ( UCHAR(m_ulDRegs[ES1371_bMEMPAGE_OFF/4]) != ucCurPage )
  {
      WRITE_PORT_UCHAR( m_pPciAddr + ES1371_bMEMPAGE_OFF, 
                        ucCurPage & 0x0f );
      m_ulDRegs[ES1371_bMEMPAGE_OFF/4] = ULONG( ucCurPage );
  }
  
  // read the Rx register to clear the current interrupt
  ucRxRegData = READ_PORT_UCHAR((PUCHAR)(m_pPciAddr + ES1371_bUARTDATA_OFF));

  do
  {
      // now get all the valid data from the "FIFO"
      // calculate the current read position
      ulFIFOPos = (m_ulUARTRxPointer & 0x03) << 2;
      ulFIFOPos += ES1371_MEMBASE_OFF;

      usRxFifoData = READ_PORT_USHORT((PUSHORT)(m_pPciAddr + ulFIFOPos));

#ifdef DBG
      {
         if ( usRxFifoData & 0x100 )
            _DbgPrintF( DEBUGLVL_VERBOSE, ("Rx: %01x", usRxFifoData));
      }
#endif

      if ( usRxFifoData & 0x100 )
      {
        // the data is valid so put it in the queue
        if ( m_ulUARTState & UART_RX_ENABLE )
        {
           // check if the queue is full (BAD)
           if ( !( ((m_ulUARTRxQHead+1) & UART_RX_QSIZE_MASK) == m_ulUARTRxQTail ) )
           {
              m_ulUARTRxQ[m_ulUARTRxQHead] = UCHAR(usRxFifoData);
              m_ulUARTRxQHead = (m_ulUARTRxQHead+1) & UART_RX_QSIZE_MASK;

              // increment the read count
              ulRxBytes++;
           }
        }
        // write Zeros back over the "FIFO" data just read
        WRITE_PORT_ULONG((PULONG)(m_pPciAddr + ulFIFOPos),0x00000000);

        // increment the "FIFO" pointer
        m_ulUARTRxPointer =  (m_ulUARTRxPointer+1) & 0x07;

        // calculate the page of the current "FIFO" position
        if ( m_ulUARTRxPointer > 3 )
           ucCurPage = ES1371_FIFO1_PAGE;
        else
           ucCurPage = ES1371_FIFO0_PAGE;

        // set the page register
        // since this is in the ISR we will set the page register directly
        if ( UCHAR(m_ulDRegs[ES1371_bMEMPAGE_OFF/4]) != ucCurPage )
        {
            WRITE_PORT_UCHAR( m_pPciAddr + ES1371_bMEMPAGE_OFF, 
                              ucCurPage & 0x0f );
            m_ulDRegs[ES1371_bMEMPAGE_OFF/4] = ULONG( ucCurPage );
        }
      }

  } while ( usRxFifoData & 0x100 );   

  // How many bytes did we put in the queue
  return ulRxBytes;
}

ULONG CES1371::UARTDeQRxByte( PUCHAR pMidiByte )
{

  if ( m_ulUARTRxQHead == m_ulUARTRxQTail )
    return 0;

  *pMidiByte = m_ulUARTRxQ[m_ulUARTRxQTail];
   m_ulUARTRxQTail = (m_ulUARTRxQTail+1) & UART_RX_QSIZE_MASK;

  return 1;
}

void CES1371::UARTTxQueuedByte( void )
{
    ULONG fDataToSend = 1;
    UCHAR ucMidiByte;
    UCHAR ucPortVal;

    // make sure there is data in the queue
    if ( m_ulUARTTxQHead == m_ulUARTTxQTail )
      fDataToSend = 0;

    if ( fDataToSend )
    {
        // get the data byte out of the queue
        ucMidiByte = m_ulUARTTxQ[m_ulUARTTxQTail];
        m_ulUARTTxQTail = (m_ulUARTTxQTail+1) & UART_TX_QSIZE_MASK;

#ifdef DBG
        {
          _DbgPrintF( DEBUGLVL_VERBOSE, ("Tx: %x", ucMidiByte));
        }
#endif
        // write the data byte to the UART
        WRITE_PORT_UCHAR( (PUCHAR)( m_pPciAddr + ES1371_bUARTDATA_OFF ), ucMidiByte );
    }

    // if the queue is empty turn off the Tx interrupt
    if ( m_ulUARTTxQHead == m_ulUARTTxQTail )
    {
       // the Rx interrupt stays on as long as the port is enabled
       ucPortVal = ES1371_UART_RXINTEN;
       WRITE_PORT_UCHAR( (PUCHAR)( m_pPciAddr + ES1371_bUARTCSTAT_OFF ), ucPortVal );

       // turn off the flag
       m_ulUARTState  &= ~UART_TX_ACTIVE;
    }
}

ULONG CES1371::UARTQueueTxByte( UCHAR ucMidiByte )
{
    // check if the queue is full (BAD)
    if ( ((m_ulUARTTxQHead+1) & UART_TX_QSIZE_MASK) == m_ulUARTTxQTail )
        return 0;

    // Add the MIDI byte to the queue
    m_ulUARTTxQ[m_ulUARTTxQHead] = ucMidiByte;
    m_ulUARTTxQHead = (m_ulUARTTxQHead+1) & UART_TX_QSIZE_MASK;
  
    // enable interrupt if needed
    if ( !(m_ulUARTState & UART_TX_ACTIVE) )
    {
        // turn on the interrupt (the Rx inturrupt is always on while Tx is on)
        UCHAR ucPortVal = ES1371_UART_RXINTEN | ES1371_UART_TXINTEN;
        WRITE_PORT_UCHAR( (PUCHAR)( m_pPciAddr + ES1371_bUARTCSTAT_OFF ), ucPortVal );

        // set the active flag
        m_ulUARTState |= UART_TX_ACTIVE;
    }

    return 1;
}
