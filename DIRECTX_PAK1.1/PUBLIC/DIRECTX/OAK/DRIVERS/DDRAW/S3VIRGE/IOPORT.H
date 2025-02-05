/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.
Copyright (c) 1995-2000  Microsoft Corporation

Module Name:    I/O port defines

Abstract:  

Notes: 
--*/
#ifndef __IOPORT_H__
#define __IOPORT_H__

#include <ceddk.h>

// Type conversions from Port to derived types
#define PORT_WO_8 *(WOPort8 *)&
#define PORT_RO_8 *(ROPort8 *)&
#define PORT_RW_8 *(RWPort8 *)&
#define PORT_WO_16 *(WOPort16 *)&
#define PORT_RO_16 *(ROPort16 *)&
#define PORT_RW_16 *(RWPort16 *)&
#define MM_RO_16 *(ROReg16 *)&
#define MM_RW_16 *(RWReg16 *)&
#define MM_WO_16 *(WOReg16 *)&
#define MM_WO_32 *(WOReg32 *)&
#define MM_RO_32 *(ROReg32 *)&
#define MM_RW_32 *(RWReg32 *)&

// Sample usage:
// #define reg_TST_WO           (PORT_WO_8 m_Port[PortId_3DA])
// #define reg_TST_RO           (PORT_RO_8 m_Port[PortId_3DA])
// #define reg_TST_RW           (PORT_RW_8 m_Port[PortId_3DA])

#define PORT_RANGE_SIZE 4096
struct PortRange
{
    PortRange   *m_pNextRange;
    USHORT      m_nFirstPortNo;
    PUCHAR      m_nPortBase;
};

class Port
{
protected:
    PUCHAR m_pPort;

public:
    void Map(USHORT port, PortRange * &pFirstRange);
};

class WOPort8 : public Port
{
public:
    UCHAR operator = (UCHAR v)
    {
        WRITE_PORT_UCHAR((PUCHAR) m_pPort, v);
        return v;
    }
};


class ROPort8 : public Port
{
public:
    operator UCHAR()
    {
        return READ_PORT_UCHAR((PUCHAR) m_pPort);
    }
};


class RWPort8 : public Port
{
public:
    UCHAR operator = (UCHAR v)
    {
        WRITE_PORT_UCHAR((PUCHAR) m_pPort, v);
        return v;
    }

    operator UCHAR()
    {
        return READ_PORT_UCHAR((PUCHAR) m_pPort);
    }
};


class RWSplitPort8
{
private:
    Port    *m_pReadPort;
    Port *m_pWritePort;

public:
    void Init(Port &readPort, Port &writePort)
    {
        m_pReadPort = &readPort;
        m_pWritePort = &writePort;
    }

    UCHAR operator = (UCHAR v)
    {
        return (PORT_WO_8 (*m_pWritePort)) = v;
    }

    operator UCHAR()
    {
        return (PORT_RO_8 (*m_pReadPort));
    }
};


class WOPort16 : public Port
{
public:
    USHORT operator = (USHORT v)
    {
        WRITE_PORT_USHORT((PUSHORT)m_pPort, v);
        return v;
    }
};


class ROPort16 : public Port
{
public:
    operator USHORT()
    {
        return READ_PORT_USHORT((PUSHORT) m_pPort);
    }
};


class RWPort16 : public Port
{
public:
    USHORT operator = (USHORT v)
    {
        WRITE_PORT_USHORT((PUSHORT) m_pPort, v);
        return v;
    }

    operator USHORT()
    {
        return READ_PORT_USHORT((PUSHORT)m_pPort);
    }
};


class IndexedReg8
{
private:
    RWPort8 *m_pAddressPort;
    RWPort8 *m_pDataPort;

public:
    void Init(RWPort8 &m_regAddress, RWPort8 &m_regData)
    {
        m_pAddressPort = &m_regAddress;
        m_pDataPort = &m_regData;
    }

    RWPort8& operator [](int nSubscript)
    {
        *m_pAddressPort = (UCHAR) nSubscript;
        return *m_pDataPort;
    }
};


class FlipIndexedReg8
{
private:
    RWPort8 *m_pRWPort;             // Write addr or data here, read addr here
    ROPort8 *m_pReadDataPort;       // Read data here
    ROPort8 *m_pResetPort;          // Read here to set m_pRWPort[writes] 
                                    // to addr
    RWSplitPort8 m_regData;         // Points to RWPort for writes, 
                                    // and ReadDataPort for reads

public:
    void ResetToAddr()
    {
        // Set m_pRWPort[writes] to addr
        volatile UCHAR temp = *m_pResetPort;
    }

    void Init(Port &rwPort, Port &readDataPort, Port &resetPort)
    {
        m_pRWPort = &(PORT_RW_8 rwPort);
        m_pReadDataPort = &(PORT_RO_8 readDataPort);
        m_regData.Init(readDataPort, rwPort); 
        m_pResetPort = &(PORT_RO_8 resetPort);
    }

    RWSplitPort8& operator [](int nSubscript)
    {
        ResetToAddr();
        *m_pRWPort = (UCHAR) nSubscript;
        return m_regData;
    }
};


class SubIndexedReg16
{
private:
    volatile PUSHORT m_pMMReg;
    USHORT m_nRegId;
public:
    void Init(PUSHORT pMMReg, USHORT regNo)
    {
        m_pMMReg = pMMReg;
        m_nRegId = regNo<<12;
    }
    operator USHORT()
    {
        *m_pMMReg = 0xF000 | (m_nRegId>>12);
        return 0x0FFF & *m_pMMReg;
    }
    USHORT operator = (USHORT v)
    {
        *m_pMMReg = v | m_nRegId;
        return v;
    }
};


class SubIndexedRegArray16
{
private:
    PUSHORT m_pMMReg;               // Address used for reads & writes
    SubIndexedReg16 m_aSubReg[16];

public:
    void Init(PUSHORT pMMReg)
    {
        int i;
        for (i = 0; i < 16; i++)
            m_aSubReg[i].Init(pMMReg, i);
    }

    SubIndexedReg16& operator [](int nSubscript)
    {
        return m_aSubReg[nSubscript];
    }
};


class RWReg16
{
private:
    volatile USHORT m_nV;

public:
    USHORT operator = (USHORT v)
    {
        m_nV = v;
        return v;
    }
    operator USHORT()
    {
        return m_nV;
    }
};


class ROReg16
{
private:
    volatile USHORT m_nV;
public:
    operator USHORT()
    {
        return m_nV;
    }
};


class WOReg16
{
private:
    volatile USHORT m_nV;

public:
    USHORT operator = (USHORT v)
    {
        m_nV = v;
        return v;
    }
};


class WOReg32
{
private:
    volatile ULONG m_nV;
public:
    ULONG operator = (ULONG v)
    {
        m_nV = v;
        return v;
    }
};


class ROReg32
{
private:
    volatile ULONG m_nV;
public:
    operator ULONG()
    {
        return m_nV;
    }
};


class RWReg32
{
private:
    volatile ULONG m_nV;
public:
    ULONG operator = (ULONG v)
    {
        m_nV = v;
        return v;
    }
    operator ULONG()
    {
        return m_nV;
    }
};

#endif // __IOPORT_H__
