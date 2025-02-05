/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-2000  Microsoft Corporation

Module Name:    I/O port mapping and control function

Abstract:  
  
Functions:


Notes: 


--*/

#include "precomp.h"

void 
Port::Map(USHORT port, PortRange *&pFirstRange)
{
    DEBUGMSG(VIRGE_FUNCTION_MSGS, 
        (TEXT("Mapping port @ 0x%08x to port 0x%04x\r\n"), this, port));

    PortRange **ppRange = &pFirstRange;
    PortRange *pRange;

    while (pRange = *ppRange)   // deliberate assignment
    {
        if (port >= pRange->m_nFirstPortNo && 
            port < pRange->m_nFirstPortNo + PORT_RANGE_SIZE)
            break;
        ppRange = &(pRange->m_pNextRange);
    }

    if (!pRange)
    {
        // Need to create a new range of mapped in ports
        DEBUGMSG(VIRGE_VERBOSE_MSGS, (TEXT("Creating new port range\r\n")));

        pRange = *ppRange = new PortRange;
        if (!pRange)
        {
            DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                (TEXT("New: failed to create new PortRange\r\n")));
            return;
        }

        pRange->m_nFirstPortNo = port - (port % PORT_RANGE_SIZE);
        pRange->m_pNextRange = (PortRange *)NULL;
        PHYSICAL_ADDRESS ioPhysicalBase = { pRange->m_nFirstPortNo, 0 };
        ULONG inIoSpace = 1;

        if (HalTranslateBusAddress(PCIBus, 0, ioPhysicalBase, 
                                   &inIoSpace, &ioPhysicalBase))
        {
            if (!inIoSpace)
            {
                DEBUGMSG(VIRGE_VERBOSE_MSGS,   
                    (TEXT("New port range is not in iospace\r\n")));
                if ((pRange->m_nPortBase = 
                         (PUCHAR) MmMapIoSpace(ioPhysicalBase, 
                                               PORT_RANGE_SIZE, 
                                               FALSE)) == NULL)
                {
                    DEBUGMSG(1, 
                        (TEXT("ERROR: failed to map new port range\r\n")));
                    return;
                }
            }
            else
            {
                DEBUGMSG(VIRGE_VERBOSE_MSGS, 
                    (TEXT("New port range is in iospace\r\n")));
                pRange->m_nPortBase = (PUCHAR) ioPhysicalBase.LowPart;
            }
            
        }
        else
        {
            DEBUGMSG(1, 
                (TEXT("ERROR: failed to determine bus address for port\r\n")));
            return;
        }
    }
    m_pPort = pRange->m_nPortBase + (port % PORT_RANGE_SIZE);
    DEBUGMSG(VIRGE_FUNCTION_MSGS, (TEXT("m_pPort = 0x%08x\r\n"), m_pPort));
}
