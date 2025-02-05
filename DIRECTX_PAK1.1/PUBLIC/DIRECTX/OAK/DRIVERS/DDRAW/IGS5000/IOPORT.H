// -----------------------------------------------------------------------------
//
//	THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//	ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//	PARTICULAR PURPOSE.
//	Copyright (c) 1997-2000  Microsoft Corporation
//
//  Module Name:
//
//	ioport.h  - IGS5000 display driver
//
// -----------------------------------------------------------------------------

#ifndef __IOPORT_H__
#define __IOPORT_H__

#include <ceddk.h>


    // the USE_MEMORY_MAPPED_PORTS define affects the way ports are mapped.
    // the IGS5000 can be accessed as Port mapped or Memory mapped
    // control registers. To determine a port mapped address you
    // add 0x00800000 to the port address, e.g. 0x3CF, plus the base
    // address of the card. A card mapped at 0xFE000000 will access
    // port 0x3CF at 0xFE8003CF. This works on X86 and RISC platforms.


// Type conversions from Port to derived types
#define PORT_WO_8 *(WOPort8 *)&
#define PORT_RO_8 *(ROPort8 *)&
#define PORT_RW_8 *(RWPort8 *)&
#define PORT_WO_16 *(WOPort16 *)&
#define PORT_RO_16 *(ROPort16 *)&
#define PORT_RW_16 *(RWPort16 *)&
#define MM_RO_8  *(ROReg8 *)&
#define MM_RW_8  *(RWReg8 *)&
#define MM_RO_16 *(ROReg16 *)&
#define MM_RW_16 *(RWReg16 *)&
#define MM_WO_16 *(WOReg16 *)&
#define MM_RW_32 *(RWReg32 *)&
#define MM_WO_32 *(WOReg32 *)&

// Sample usage:
// #define reg_TST_WO		(PORT_WO_8 m_Port[PortId_3DA])
// #define reg_TST_RO		(PORT_RO_8 m_Port[PortId_3DA])
// #define reg_TST_RW		(PORT_RW_8 m_Port[PortId_3DA])

#define PORT_RANGE_SIZE 4096
struct PortRange
{
    PortRange		   *m_pNextRange;
    unsigned long	   m_nFirstPortNo;
    unsigned char	   *m_nPortBase;
};

class Port
{
protected:
    unsigned char * m_pPort;
public:
    void Map( unsigned long port, PortRange * &pFirstRange );
};

class WOPort8 : public Port
{
public:
    unsigned char operator =(unsigned char v)
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	READ_REGISTER_UCHAR( (PUCHAR) ((long) m_pPort & 0xFFFFF000)+0x3CE);   /*dummy read, required only if it is PCI and uses memory mapped I/O */
	WRITE_REGISTER_UCHAR( (PUCHAR)m_pPort, v );
#else  //USE_MEMORY_MAPPED_PORTS
	WRITE_PORT_UCHAR( (PUCHAR)m_pPort, v );
#endif //USE_MEMORY_MAPPED_PORTS
	return v;
    }
};

class ROPort8 : public Port
{
public:
    operator unsigned char()
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	return READ_REGISTER_UCHAR( (PUCHAR)m_pPort );
#else  //USE_MEMORY_MAPPED_PORTS
	return READ_PORT_UCHAR( (PUCHAR)m_pPort );
#endif //USE_MEMORY_MAPPED_PORTS
    }
};

class RWPort8 : public Port
{
public:
    unsigned char operator =(unsigned char v)
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	READ_REGISTER_UCHAR( (PUCHAR)((long) m_pPort & 0xFFFFF000)+0x3CE);     /*dummy read, required only if it is PCI and uses memory mapped I/O */
	WRITE_REGISTER_UCHAR( (PUCHAR)m_pPort, v );
#else  //USE_MEMORY_MAPPED_PORTS
	WRITE_PORT_UCHAR( (PUCHAR)m_pPort, v );
#endif //USE_MEMORY_MAPPED_PORTS
	return v;
    }
    operator unsigned char()
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	return READ_REGISTER_UCHAR( (PUCHAR)m_pPort );
#else  //USE_MEMORY_MAPPED_PORTS
	return READ_PORT_UCHAR( (PUCHAR)m_pPort );
#endif //USE_MEMORY_MAPPED_PORTS
    }
};

class RWSplitPort8
{
private:
    Port    *m_pReadPort;
    Port *m_pWritePort;
public:
    void Init( Port &readPort, Port &writePort )
    {
	m_pReadPort = &readPort;
	m_pWritePort = &writePort;
    }
    unsigned char operator =(unsigned char v)
    {
	return (PORT_WO_8 (*m_pWritePort)) = v;
    }
    operator unsigned char()
    {
	return (PORT_RO_8 (*m_pReadPort));
    }
};

class WOPort16 : public Port
{
public:
    unsigned short operator =(unsigned short v)
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	READ_REGISTER_UCHAR( (PUCHAR)((long) m_pPort & 0xFFFFF000)+0x3CE);     /*dummy read, required only if it is PCI and uses memory mapped I/O */
	WRITE_REGISTER_USHORT( (unsigned short *)m_pPort, v );
#else  //USE_MEMORY_MAPPED_PORTS
	WRITE_PORT_USHORT( (unsigned short *)m_pPort, v );
#endif //USE_MEMORY_MAPPED_PORTS
	return v;
    }
};

class ROPort16 : public Port
{
public:
    operator unsigned short()
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	return READ_REGISTER_USHORT( (unsigned short *)m_pPort );
#else  //USE_MEMORY_MAPPED_PORTS
	return READ_PORT_USHORT( (unsigned short *)m_pPort );
#endif //USE_MEMORY_MAPPED_PORTS
    }
};

class RWPort16 : public Port
{
public:
    unsigned short operator =(unsigned short v)
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	READ_REGISTER_UCHAR( (PUCHAR)((long) m_pPort & 0xFFFFF000)+0x3CE);     /*dummy read, required only if it is PCI and uses memory mapped I/O */
	WRITE_REGISTER_USHORT( (unsigned short *)m_pPort, v );
#else  //USE_MEMORY_MAPPED_PORTS
	WRITE_PORT_USHORT( (unsigned short *)m_pPort, v );
#endif //USE_MEMORY_MAPPED_PORTS
	return v;
    }
    operator unsigned short()
    {
#ifdef USE_MEMORY_MAPPED_PORTS
	return READ_REGISTER_USHORT( (unsigned short *)m_pPort );
#else  //USE_MEMORY_MAPPED_PORTS
	return READ_PORT_USHORT( (unsigned short *)m_pPort );
#endif //USE_MEMORY_MAPPED_PORTS
    }
};

class IndexedReg8
{
public:
    RWPort8 *m_pAddressPort;
    RWPort8 *m_pDataPort;
public:
    void Init( RWPort8 &m_regAddress, RWPort8 &m_regData )
    {
	m_pAddressPort = &m_regAddress;
	m_pDataPort = &m_regData;
    }
    RWPort8& operator [](int nSubscript)
    {
	*m_pAddressPort = (unsigned char)nSubscript;
	return *m_pDataPort;
    }
};


class FlipIndexedReg8
{
private:
    RWPort8 *m_pRWPort; 		    // Write addr or data here, read addr here
    ROPort8 *m_pReadDataPort;	    // Read data here
    ROPort8 *m_pResetPort;	    // Read here to set m_pRWPort[writes] to addr
    RWSplitPort8 m_regData;	    // Points to RWPort for writes, and ReadDataPort for reads
public:
    void ResetToAddr()
    {
	// Set m_pRWPort[writes] to addr
	volatile unsigned char temp = *m_pResetPort;
    }
    void Init( Port &rwPort, Port &readDataPort, Port &resetPort )
    {
	m_pRWPort = &(PORT_RW_8 rwPort);
	m_pReadDataPort = &(PORT_RO_8 readDataPort);
	m_regData.Init( readDataPort, rwPort );
	m_pResetPort = &(PORT_RO_8 resetPort);
    }
    RWSplitPort8& operator [](int nSubscript)
    {
	ResetToAddr();
	*m_pRWPort = (unsigned char)nSubscript;
	return m_regData;
    }
};


class SubIndexedReg16
{
private:
    volatile unsigned short *m_pMMReg;
    unsigned short m_nRegId;
public:
    void Init( unsigned short *pMMReg, unsigned short regNo )
    {
	m_pMMReg = pMMReg;
	m_nRegId = regNo<<12;
    }
    operator unsigned short()
    {
	*m_pMMReg = 0xF000 | (m_nRegId>>12);
	return 0x0FFF & *m_pMMReg;
    }
    unsigned short operator =(unsigned short v)
    {
	*m_pMMReg = v | m_nRegId;
	return v;
    }
};


class SubIndexedRegArray16
{
private:
    unsigned short *m_pMMReg;		    // Address used for reads & writes
    SubIndexedReg16 m_aSubReg[16];
public:
    void Init( unsigned short *pMMReg )
    {
	int i;
	for( i=0; i<16; i++ )
	    m_aSubReg[i].Init( pMMReg, i);
    }

    SubIndexedReg16& operator [](int nSubscript)
    {
	return m_aSubReg[nSubscript];
    }
};

class RWReg8
{
private:
    volatile unsigned char m_nV;
public:
    unsigned char operator =(unsigned char v)
    {
	m_nV = v;
	return v;
    }
    operator unsigned char()
    {
	return m_nV;
    }
};

class ROReg8
{
private:
    volatile unsigned char m_nV;
public:
    operator unsigned char()
    {
	return m_nV;
    }
};

class RWReg16
{
private:
    volatile unsigned short m_nV;
public:
    unsigned short operator =(unsigned short v)
    {
	m_nV = v;
	return v;
    }
    operator unsigned short()
    {
	return m_nV;
    }
};

class ROReg16
{
private:
    volatile unsigned short m_nV;
public:
    operator unsigned short()
    {
	return m_nV;
    }
};

class WOReg16
{
private:
    volatile unsigned short m_nV;
public:
    unsigned short operator =(unsigned short v)
    {
	m_nV = v;
	return v;
    }
};

class RWReg32
{
private:
    volatile unsigned long m_nV;
public:
    unsigned long operator =(unsigned long v)
    {
	m_nV = v;
	return v;
    }
    operator unsigned long()
    {
	return m_nV;
    }
};

class WOReg32
{
private:
    volatile unsigned long m_nV;
public:
    unsigned long operator =(unsigned long v)
    {
	m_nV = v;
	return v;
    }
};

#endif // __IOPORT_H__
