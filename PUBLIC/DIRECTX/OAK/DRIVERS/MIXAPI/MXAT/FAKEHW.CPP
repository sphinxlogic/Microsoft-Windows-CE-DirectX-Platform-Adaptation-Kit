//*****************************************************************************
//
//      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//      ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//      THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//      PARTICULAR PURPOSE.
//      Copyright (c) 1995-1999  Microsoft Corporation
//
//*****************************************************************************

#include "wavemdd.h"
#include "regfake.h"
#include "mixdrv.h"
#include "globals.h"
#include "csmix.h"
#include "fakehw.h"

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	PeekBA0

Description:

	reads a word from the specified location in the Mixer Device's address
	space (based on the part's base address zero register).

Arguments:

	PDRIVER_CONTEXT pDriverContext: the structure describing the driver state
	ULONG ulByteOffset : offset in the BA0 area from where the data is read 
		note that the value is a BYTE offset, but the function retrieves
		a whole DWORD
	DWORD *pulValue : value to be retrieved

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD PeekBA0( PDRIVER_CONTEXT pDriverContext, ULONG ulByteOffset, DWORD *pulValue)
{
	ASSERT(pDriverContext);
	ASSERT(pulValue);
	
    if (ulByteOffset >= BA0_AC97_RESET)
	{
        return ReadAC97 ( pDriverContext, ulByteOffset, pulValue);
	}
	
	// Read the value of BA0 memory
	pDriverContext->Aperture0->Peek(ulByteOffset, pulValue);
	return NO_ERROR;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	PeekBA1

Description:

	reads a word from the specified location in the Mixer Device's address 
	space (based on the part's base address one register).

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	ULONG ulByteOffset :
	DWORD *pulValue :

Return Value: DWORD 
-------------------------------------------------------------------*/
DWORD PeekBA1(
	PDRIVER_CONTEXT pDriverContext,
	ULONG ulByteOffset,
	DWORD *pulValue
	)
{
	ASSERT(pDriverContext);
	ASSERT(pulValue);
	
	// Read the value of BA1 memory
	pDriverContext->Aperture1->Peek(ulByteOffset, pulValue);
	return NO_ERROR;
}



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	PokeBA0

Description:

	

Arguments:

	 PDRIVER_CONTEXT pDriverContext :
	ULONG ulByteOffset :
	ULONG ulValue :

Return Value: DWORD
-------------------------------------------------------------------*/
DWORD PokeBA0( 
	PDRIVER_CONTEXT pDriverContext, 
	ULONG ulByteOffset, 
	ULONG ulValue
	)
{
	//    ENTRY(4, "Hrdw::PokeBA0 ( pDriverContext, )");
	//    ARG("ulOffset","0x%08lx", ulByteOffset);
	//    ARG("ulValue", "0x%08lx",ulValue);
	
    if (ulByteOffset >= BA0_AC97_RESET)
    {
        return WriteAC97 ( pDriverContext, ulByteOffset, ulValue);
    }
	
    // Write the specified double word to the Mixer Device's memory.
	pDriverContext->Aperture0->Poke(ulByteOffset, ulValue);
	return NO_ERROR;
}




/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	PokeBA1

Description:

	

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	ULONG ulByteOffset :
	ULONG ulValue :

Return Value: void
-------------------------------------------------------------------*/
void PokeBA1(PDRIVER_CONTEXT pDriverContext, ULONG ulByteOffset, ULONG ulValue)
{
	ASSERT(pDriverContext);
    // Write the specified double word to the Mixer Device's memory.
	pDriverContext->Aperture1->Poke(ulByteOffset, ulValue);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	SetPlaySampleRate

Description:

	Calculate & store the sample rate parameters corresponding to 
	specified number of samples per second.	

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	DWORD PlayRate :

Return Value: void 
-------------------------------------------------------------------*/
void SetPlaySampleRate(
	PDRIVER_CONTEXT pDriverContext,
	DWORD dwBaseRegister,
	DWORD dwPlayRate
	)
{
    DWORD temp1, temp2;
    DWORD PhsIncr, CorrectionPerGOF, CorrectionPerSec;
    DWORD OutRate=48000;
	
	DEBUGMSG(ZONE_FUNCTION|ZONE_PDD|ZONE_VERBOSE, (TEXT("+SetPlaySampleRate\r\n")));
	
	// Calculate and store the values for the
	// correction & phase increment regs.
	// Algorithm from DOSHWInterface::SetPlaySampleRate():
	//    PhsIncr = floor( (PlayRate * 2^26)/ OutRate )
	//    CorrectionPerGOF = floor((PlayRate*2^26 - OutRate * PhsIncr) /
	//                              GOF_PER_SEC)
	//    CorrectionPerSec = PlayRate*2^26 - OutRate*PhsIncr -
	//                          GOF_PER_SEC * CorrectionPerGOF
	
    temp1 = dwPlayRate << 16;
    PhsIncr = temp1 / OutRate;
    temp1   -= PhsIncr * OutRate;
    temp1 <<= 10;
    PhsIncr <<= 10;
    temp2 = temp1/OutRate;
    PhsIncr += temp2;
    temp1 -= temp2*OutRate;
    CorrectionPerGOF = temp1/GOF_PER_SEC;
    temp1 -= CorrectionPerGOF * GOF_PER_SEC;
    CorrectionPerSec = temp1;
	
	// Set BA1_PLAY_SAMPLE_RATE_CORRECTION_REG(288)(0 @ default 48K)
	// and BA1_PlAY_PHASE_INCREMENT_REG(2b4)(0x08000000 @ default 48k)
    PokeBA1 ( pDriverContext,  dwBaseRegister + BA1_SAMPLE_RATE_CORRECTION_OFFSET,
		((CorrectionPerSec << 16) & 0xFFFF0000) | (CorrectionPerGOF & 0x0000FFFF));
	
    PokeBA1 ( pDriverContext,  BA1_PLAY_PHASE_INCREMENT_REG, PhsIncr);
	
	// Make SRC the child of the master mixer.
    PokeBA1 ( pDriverContext,  BA1_MASTER_MIX_CHILD_REG, (PROC_SRC_SCB <<16) & 0xFFFF0000);
	
	return;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	ReadAC97

Description:

	Reads a word from the specified location in the Hrdw's address space
	(based on the BA0 register).

	1. Write ACCAD = Command Address Register = 46Ch for AC97 register address
	2. Write ACCDA = Command Data Register = 470h for data to write to AC97 register,
                                            0h for reads.
	3. Write ACCTL = Control Register = 460h for initiating the write
	4. Read ACCTL = 460h, DCV should be reset by now and 460h = 17h
	5. if DCV not cleared, break and return error
	6. Read ACSTS = Status Register = 464h, check VSTS bit

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	DWORD ulOffset :
	DWORD *pulValue :

Return Value: DWORD 
-------------------------------------------------------------------*/
DWORD ReadAC97(
	PDRIVER_CONTEXT pDriverContext, 
	DWORD ulOffset, 
	DWORD *pulValue
	)
{
	pDriverContext->AC97Regs->Peek(ulOffset, pulValue);

	return NO_ERROR;
}


DWORD WriteAC97( 
	PDRIVER_CONTEXT pDriverContext, 
	DWORD ulOffset, 
	DWORD ulValue
	)
{
	ASSERT(pDriverContext);

	pDriverContext->AC97Regs->Poke(ulOffset, ulValue);
	
    return NO_ERROR;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:

	PokeIO

Description:

	

Arguments:

	PDRIVER_CONTEXT pDriverContext :
	ULONG ulAddress :
	ULONG ulValue :

Return Value: void
-------------------------------------------------------------------*/
void PokeIO(PDRIVER_CONTEXT pDriverContext, ULONG ulAddress, ULONG ulValue)
{
    //
    // First check to see that the processor doesn't have any pending writes.
    //

	DEBUGMSG(ZONE_TEST|ZONE_FUNCTION|ZONE_PDD|ZONE_VERBOSE, 
		(TEXT("+PokeIO : addr 0x%8.8x, value: 0x%8.8x\r\n"), ulAddress, ulValue));


    //
    // Now, specify address and data.
    //
    ulAddress <<= IO_ADDR_SHIFT;
    ulAddress &= IO_ADDR_MASK;
    ulAddress |= IO_ADDR_NO_MORE_DATA;
    PokeBA1(pDriverContext, BA1_IO_ADDR, ulAddress);
    PokeBA1(pDriverContext, BA1_IO_DATA, ulValue);

    //
    // Finally, get the processor to do the write.
    //
    DWORD ulDataAddr = (BA1_IO_ADDR/4) << IO_DATA_PTR_SHIFT;
    PokeBA1(pDriverContext, BA1_IO_DATA_PTR, ulDataAddr);

    // Success.
    return;
}


//#########################################################################################
HwRegAperture::HwRegAperture(int count)
{
	m_reg_count = 0;
	m_reg_table_size = count;
	m_reg_table = new HWREG[count];
	ASSERT(m_reg_table != NULL);
}

HwRegAperture::~HwRegAperture()
{
	delete [] m_reg_table;
}

void HwRegAperture::Peek(ULONG ulByteOffset, DWORD *pulValue)
{ int i;

	for (i = 0; i < m_reg_count; i++) { 
		if (m_reg_table[i].offset == ulByteOffset) {
			*pulValue = m_reg_table[i].value;
			return;
		}
	}
	// fell off the end of the known regs. return all FF's
	DEBUGMSG(ZONE_ERROR, (TEXT("HwRegAperture::Peek - read before write at offset %x\r\n"), ulByteOffset));
	*pulValue = 0xffffffff;
}

void HwRegAperture::Poke(ULONG ulByteOffset, DWORD ulValue)
{ int i;

	// first, see if the register has already been written.
	for (i = 0; i < m_reg_count; i++) { 
		if (m_reg_table[i].offset == ulByteOffset) {
			m_reg_table[i].value = ulValue;
			return;
		}
	}
	// fell off the end of the known regs.
	// record new value, if there's still room
	if (i >= m_reg_table_size) {
		DEBUGMSG(ZONE_ERROR, (TEXT("HwRegAperture::Poke - out of room for new writes. size = %d\r\n"), i));
		return;
	}
	m_reg_count++;
	m_reg_table[i].value = ulValue;
}
