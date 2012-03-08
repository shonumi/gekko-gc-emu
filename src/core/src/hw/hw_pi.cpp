// hw_pi.cpp
// (c) 2005,2006 Gekko Team

#include "common.h"
#include "hw.h"
#include "hw_pi.h"
#include "powerpc/cpu_core.h"
#include "powerpc/cpu_core_regs.h"

//

u8		PIRegisters[REG_SIZE];
u32		PIInterrupt;

////////////////////////////////////////////////////////////////////////////////
// PI - Processor Interface
//
//			1/15/06		-	File Created. (ShizZy)
//			6/30/08		-	Added CPU Fifo Write Calls, Updated reads/writes,
//							emulated type register. (ShizZy)
//
//			Missing:
//
////////////////////////////////////////////////////////////////////////////////

void __fastcall PI_Fifo_Write8(u32 addr, u32 data)
{
	Memory_Write8(PI_FIFO_WRITE_POINTER, data);
    PI_FIFO_WRITE_POINTER++;
}

void __fastcall PI_Fifo_Write16(u32 addr, u32 data)
{
	Memory_Write16(PI_FIFO_WRITE_POINTER, data);
    PI_FIFO_WRITE_POINTER+=2;
}

void __fastcall PI_Fifo_Write32(u32 addr, u32 data)
{
	Memory_Write32(PI_FIFO_WRITE_POINTER, data);
    PI_FIFO_WRITE_POINTER+=4;
}

u32 PI_CheckForInterrupts(void)
{
	// If the status register has the right mask register bit set,
	// and external interrupts are enabled in MSR,
	// then do an external interrupt exception.
	//if((REGPI32(PI_INTSR) & REGPI32(PI_INTMR)) && (ireg_MSR() & MSR_BIT_EE))
	if(!PIInterrupt || !(ireg.MSR & MSR_BIT_EE))
		return 0;

	cpu->Exception(GekkoCPU::GEX_EXT);
	return 1;
}

void PI_RequestInterrupt(u32 mask)
{
	// Turn on an IRQ bit in the status register.
	REGPI32(PI_INTSR) |= mask;
	PIInterrupt = REGPI32(PI_INTSR) & REGPI32(PI_INTMR);
}

void PI_ClearInterrupt(unsigned int mask)
{
	// Turn off an IRQ bit in the status register
	REGPI32(PI_INTSR) &= ~mask;
	PIInterrupt = REGPI32(PI_INTSR) & REGPI32(PI_INTMR);
}

////////////////////////////////////////////////////////////////////////////////

// Desc: Read/Write from/to PI Hardware
//

u8 __fastcall PI_Read8(u32 addr)
{
	LOG_ERROR(TPI, "Undefined PI_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall PI_Write8(u32 addr, u32 data)
{
	LOG_ERROR(TPI, "Undefined PI_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall PI_Read16(u32 addr)
{
	switch(addr)
	{
	case PI_INTSR:						// Hi Interrupt Status Register
	case (PI_INTSR + 2):				// Lo
	case PI_INTMR:						// Hi Interrupt Mask Register
	case (PI_INTMR + 2):				// Lo
	case PI_FIFO_BASESTART:
	case (PI_FIFO_BASESTART + 2):
	case PI_FIFO_BASEEND:
	case (PI_FIFO_BASEEND + 2):
    case PI_FIFO_WRITEPOINTER:
    case (PI_FIFO_WRITEPOINTER + 2):
    case PI_CONSOLE_TYPE:
	case (PI_CONSOLE_TYPE + 2):
		return REGPI16(addr);
	default:
		LOG_ERROR(TPI, "Undefined PI_Read16: %08X!\n", addr);
		return 0;
	}
}

void __fastcall PI_Write16(u32 addr, u32 data)
{
	switch(addr)
	{
	case PI_INTSR:						// Hi Interrupt Status Register is Read-Only, the hardware must set it.
	case (PI_INTSR + 2):				// Lo
		return;
	case PI_INTMR:						// Hi Interrupt Mask Register
	case (PI_INTMR + 2):				// Lo
		REGPI16(addr) = data;
		PIInterrupt = REGPI32(PI_INTSR) & REGPI32(PI_INTMR);
		return;
	case PI_FIFO_BASESTART:
	case (PI_FIFO_BASESTART + 2):
	case PI_FIFO_BASEEND:
	case (PI_FIFO_BASEEND + 2):
    case PI_FIFO_WRITEPOINTER:
    case (PI_FIFO_WRITEPOINTER + 2):
    case PI_CONSOLE_TYPE:
	case (PI_CONSOLE_TYPE + 2):
		REGPI16(addr) = data;
		return;
	default:
		LOG_ERROR(TPI, "Undefined PI_Write16: %08X := %08X !\n", addr, data);
		return;
	}
}

u32 __fastcall PI_Read32(u32 addr)
{
	switch(addr)
	{
	case PI_INTSR: // Interrupt Status Register
	case PI_INTMR: // Interrupt Mask Register
	case PI_FIFO_BASESTART:
	case PI_FIFO_BASEEND:
    case PI_FIFO_WRITEPOINTER:
    case PI_CONSOLE_TYPE:
		return REGPI32(addr);
	default:
		LOG_ERROR(TPI, "Undefined PI_Read32: %08X!\n", addr);
		return 0;
	}
}

void __fastcall PI_Write32(u32 addr, u32 data)
{
	switch(addr)
	{
	case PI_INTSR: // Interrupt Status Register is Read-Only, the hardware must set it.
		return;
	case PI_INTMR: // Interrupt Mask Register.
		REGPI32(addr) = data;
		PIInterrupt = REGPI32(PI_INTSR) & REGPI32(PI_INTMR);
		return;
	case PI_FIFO_BASESTART:
	case PI_FIFO_BASEEND:
    case PI_FIFO_WRITEPOINTER:
    case PI_CONSOLE_TYPE:
		REGPI32(addr) = data;
		return;
	default:
		LOG_ERROR(TPI, "Undefined PI_Write32: %08X := %08X !\n", addr, data);
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////

// Desc: Update PI Hardware
//

void PI_Update(void)
{
}

// Desc: Initialize PI Hardware
//

void PI_Open(void)
{
	LOG_NOTICE(TPI, "initialized ok");

	memset(PIRegisters, 0, sizeof(PIRegisters));
	
	REGPI32(PI_CONSOLE_TYPE) = 0x20000000; // Make console a HW2 Retail
	PIInterrupt = 0;
}

////////////////////////////////////////////////////////////////////////////////
