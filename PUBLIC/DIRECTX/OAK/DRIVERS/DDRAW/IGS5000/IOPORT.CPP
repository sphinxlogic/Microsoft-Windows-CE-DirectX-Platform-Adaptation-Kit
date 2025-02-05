// -----------------------------------------------------------------------------
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1997-1998  Microsoft Corporation
//
//  Module Name:
//
//      ioport.cpp  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#include "precomp.h"

//
//  Map -
//  inputs:
//  outputs:
//
void Port::Map( unsigned long port, PortRange *&pFirstRange )
{
    DEBUGMSG(GPE_ZONE_INIT,(TEXT("Mapping port @ 0x%08x to port 0x%04x\r\n"),this,port));

    PortRange **ppRange = &pFirstRange;
    PortRange *pRange;

    while( pRange = *ppRange ) {// deliberate assignment
        if( port >= pRange->m_nFirstPortNo
            && port < pRange->m_nFirstPortNo + PORT_RANGE_SIZE )
            break;
        ppRange = &(pRange->m_pNextRange);
    }

    if( !pRange ) {
        // Need to create a new range of mapped in ports
        DEBUGMSG(GPE_ZONE_INIT,(TEXT("Creating new port range\r\n")));

        pRange = *ppRange = new PortRange;
        if( !pRange ) {
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("New: failed to create new PortRange\r\n")));
            return;
        }
        pRange->m_nFirstPortNo = port - ( port % PORT_RANGE_SIZE );
        pRange->m_pNextRange = (PortRange *)NULL;
        PHYSICAL_ADDRESS ioPhysicalBase = { pRange->m_nFirstPortNo, 0 };
#ifdef USE_MEMORY_MAPPED_PORTS

#ifdef PCI_BUS
        ULONG inIoSpace = 0;
        if (HalTranslateBusAddress(PCIBus, 0, ioPhysicalBase, &inIoSpace,
                                   &ioPhysicalBase)) {
#else
        ULONG inIoSpace = 0;
        if (HalTranslateBusAddress(Internal, 0, ioPhysicalBase, &inIoSpace,
                                    &ioPhysicalBase)) {
#endif

#else  //USE_MEMORY_MAPPED_PORTS
        ULONG inIoSpace = 1;
        if (HalTranslateBusAddress(PCIBus, 0, ioPhysicalBase, &inIoSpace,
                                    &ioPhysicalBase)) {
#endif  //USE_MEMORY_MAPPED_PORTS
            if (!inIoSpace) {
                DEBUGMSG(GPE_ZONE_INIT,(TEXT("New port range is not in iospace\r\n")));
                if ((pRange->m_nPortBase = (PUCHAR)MmMapIoSpace(ioPhysicalBase,
                        PORT_RANGE_SIZE, FALSE)) == NULL) {
                    DEBUGMSG(GPE_ZONE_ERROR,(TEXT("failed to map new port range\r\n")));
                    return;
                }
            } else {
                DEBUGMSG(GPE_ZONE_INIT,(TEXT("New port range is in iospace\r\n")));
                pRange->m_nPortBase = (PUCHAR)ioPhysicalBase.LowPart;
            }

        } else {
            DEBUGMSG(GPE_ZONE_ERROR,(TEXT("failed to determine bus address for port\r\n")));
            return;
        }
    }
    m_pPort = pRange->m_nPortBase + ( port % PORT_RANGE_SIZE );
    DEBUGMSG(GPE_ZONE_INIT,(TEXT("m_pPort = 0x%08x\r\n"), m_pPort));
}

