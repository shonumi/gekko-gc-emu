// memory.cpp
// (c) 2005,2008 Gekko Team

#include "common.h"
#include "memory.h"
#include "hw/hw.h"

////////////////////////////////////////////////////////////////////////////////
// Memory
//
//			1/15/06		-	File Created. (ShizZy)
//			x/x/06		-	Various Updates (Lightning)
//			2/25/06		-	Implemented early Wii 64mb RAM2 support (ShizZy)
//
//			Missing:
//						
////////////////////////////////////////////////////////////////////////////////

//these should be virtual alloc'd but have a bug somewhere
//so reverted to original

#pragma push(align)
#pragma align 4096
u8 Mem_L2[L2_SIZE]; // L2 Cache
//u8 Mem_RAM[RAM_SIZE]; // Ram 24mb
//u8 *Mem_RAM = 0;
//u8 Mem_RAM2[RAM2_SIZE]; // Ram2 64mb (Wii)
u8 Mem_RAM[RAM2_SIZE]; // Ram2 64mb (Wii)
#pragma pop(align)

////////////////////////////////////////////////////////////////////////////////

#include "hw/hw_pe.h"
#include "hw/hw_vi.h"
#include "hw/hw_si.h"
#include "hw/hw_pi.h"
#include "hw/hw_exi.h"
#include "hw/hw_dsp.h"
#include "hw/hw_gx.h"
#include "hw/hw_ai.h"
#include "hw/hw_di.h"
#include "hw/hw_cp.h"

//dummy value incase a read is done to an invalid area to limit code in the dynarec
static u32 EMU_FASTCALL Read0Mem(u32 Addr)
{
	return 0;
}

typedef u32(EMU_FASTCALL *HWRead32Ptr)(u32 Addr);
static HWRead32Ptr	HWRead32[0x100] =
{
	//0x00
	CP_Read32, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x10
	PE_Read32, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x20
	VI_Read32, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x30
	PI_Read32, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x40
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x50
	DSP_Read32, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x60
	DI_Read32, Read0Mem, Read0Mem, Read0Mem, SI_Read32, Read0Mem, Read0Mem, Read0Mem,
	EXI_Read32, Read0Mem, Read0Mem, Read0Mem, AI_Read32, Read0Mem, Read0Mem, Read0Mem,
	//0x70
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x80
	Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0x90
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0xA0
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0xB0
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0xC0
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0xD0
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0xE0
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
	//0xF0
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem,
		Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem, Read0Mem
};

// Initialization
//

void Memory_Open(void)
{
//	if(!Mem_RAM)
//		Mem_RAM = (u8 *)VirtualAlloc(NULL, RAM_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	memset(Mem_RAM, 0, RAM_SIZE);
	memset(Mem_L2, 0, L2_SIZE);

	LOG_NOTICE(TMEM, "initialized ok");
}

void Memory_Close(void)
{
//	VirtualFree(Mem_RAM, 0, MEM_RELEASE);
}

////////////////////////////////////////////////////////////////////////////////

// Memory Reads
//

u8 EMU_FASTCALL Memory_Read8(u32 addr)
{
//	if (addr >= 0x90000000 && addr < 0x94000000) // MEM 2 (Wii)
//		return Mem_RAM2[(addr ^ 3) & RAM2_MASK];

//	if((addr & RAM_MASK) == (0x803C4BDC & RAM_MASK))
//		printf("Reading 0x803C4BDC: %02X\n", Mem_RAM[(0x803C4BDC ^ 3) & RAM_MASK]);

	if( addr < 0xC8000000 )				// Logical RAM
		return Mem_RAM[(addr ^ 3) & RAM_MASK];
	else if( addr >= 0xCC000000 && addr < 0xE0000000 )				// HW
	{
		return Flipper_Read8(addr);
	}
	else if( addr < 0xCC000000 )				// EFB
	{
		ASSERT_T(1, ".Memory: Invalid Memory_Read8 from EFB!");
		return 0;
	}
	else if( addr < 0xF0000000 )				// L2
	{
		//printf("Memory_Read8() L2 Accessed!\n");
		return Mem_L2[(addr ^ 3) & L2_MASK];
	}else{								// IPL
//		printf(".Memory: ERROR: IPL Memory_Read8(%08X) !\n", addr);
		return 0;
	}
}

//

u16 EMU_FASTCALL Memory_Read16(u32 addr)
{
/*	if (addr >= 0x90000000 && addr < 0x94000000) // MEM 2 (Wii)
	{
		return *(u16 *)(&Mem_RAM2[(addr ^ 2) & RAM2_MASK]);
	}
*/
	if( addr < 0xC8000000 )				// Logical RAM
	{
		if(!(addr & 1))
			return *(u16 *)(&Mem_RAM[(addr ^ 2) & RAM_MASK]);
		else
		{
			addr = addr & RAM_MASK;
			return (u16)(Mem_RAM[(addr + 0) ^ 3] << 8) |
				   (u16)(Mem_RAM[(addr + 1) ^ 3]);
		}
	}
	else if( addr >= 0xCC000000 && addr < 0xE0000000 )				// HW
	{
		return Flipper_Read16(addr);
	}
	else if( addr < 0xCC000000 )				// EFB
	{
		ASSERT_T(1, ".Memory: Invalid Memory_Read16 from EFB!");
		return 0;
	}
	else if( addr < 0xF0000000 )				// L2
	{
		if(!(addr & 1))
			return *(u16 *)(&Mem_L2[(addr ^ 2) & L2_MASK]);
		else
		{
			addr = addr & L2_MASK;
			return (u16)(Mem_L2[(addr + 0) ^ 3] << 8) |
				   (u16)(Mem_L2[(addr + 1) ^ 3]);
		}
	}
	else
	{								// IPL
//		printf(".Memory: ERROR: IPL Memory_Read16(%08X) !\n", addr);
		return 0;
	}
}

//

u32 EMU_FASTCALL Memory_Read32(u32 addr)
{
/*	if (addr >= 0x90000000 && addr < 0x94000000) // MEM 2 (Wii)
	{
		return *(u32 *)(&Mem_RAM2[addr & RAM2_MASK]);
	}
*/
	if( addr < 0xC8000000 )				// Logical RAM
	{
		addr &= RAM_MASK;
		if(!(addr & 3))
			return *(u32 *)(&Mem_RAM[addr]);
		else
		{
			return ((u32)Mem_RAM[(addr + 0) ^ 3] << 24) |
				   ((u32)Mem_RAM[(addr + 1) ^ 3] << 16) |
				   ((u32)Mem_RAM[(addr + 2) ^ 3] << 8) |
				   ((u32)Mem_RAM[(addr + 3) ^ 3]);
		}
	}
	else if( addr >= 0xCC000000 && addr < 0xE0000000 )				// HW
	{
		return Flipper_Read32(addr);
	}
	else if( addr < 0xCC000000 )				// EFB
	{
		ASSERT_T(1, "MEM: Invalid Memory_Read32 from EFB!");
		return 0;
	}
	else if( addr < 0xF0000000 )				// L2
	{
		if(!(addr & 3))
			return *(u32 *)(&Mem_L2[addr & L2_MASK]);
		else
		{
			addr = addr & L2_MASK;
			return ((u32)Mem_L2[(addr + 0) ^ 3] << 24) |
				   ((u32)Mem_L2[(addr + 1) ^ 3] << 16) |
				   ((u32)Mem_L2[(addr + 2) ^ 3] << 8) |
				   ((u32)Mem_L2[(addr + 3) ^ 3]);
		}
	}
	else
	{								// IPL
//		printf(".Memory: ERROR: IPL Memory_Read32(%08X) !\n", addr);
		return 0;
	}
}

//

u64 EMU_FASTCALL Memory_Read64(u32 addr)
{
	addr &= RAM_MASK;
	return ((u64)(*(u32 *)(&Mem_RAM[addr])) << 32) |
			(u64)(*(u32 *)(&Mem_RAM[addr + 4]));
}

////////////////////////////////////////////////////////////////////////////////

// Memory Writes
//

void EMU_FASTCALL Memory_Write8(u32 addr, u32 data)
{
/*	if(((addr ^ 3) & RAM_MASK) == (0x803C4BDC & RAM_MASK))
	{
		printf("Writing 0x803C4BDC: %02X, PC: %08X, IC: %08X\n", (u8)data, ireg_PC(), ireg_IC());
//		_asm {int 3};
	}
*/
/*	if (addr >= 0x90000000 && addr < 0x94000000) // MEM 2 (Wii)
	{
		Mem_RAM2[(addr ^ 3) & RAM2_MASK] = data;
		return;
	}
*/	if( addr < 0xC8000000 )				// Logical RAM
	{
		Mem_RAM[(addr ^ 3) & RAM_MASK] = data;
		return;
	}
	if( addr >= 0xCC000000 && addr < 0xE0000000 )				// HW
	{
		Flipper_Write8(addr, data);
		return;
	}
	if( addr < 0xCC000000 )				// EFB
	{
		ASSERT_T(1, ".Memory: Invalid Memory_Write8 to EFB!");
		return;
	}
	if( addr < 0xF0000000 )				// L2
	{
		//printf("Memory_Write8() L2 Accessed!\n");
		Mem_L2[(addr ^ 3) & L2_MASK] = data;
		return;
	}else{								// IPL
//		printf(".Memory: ERROR: IPL Memory_Write8(%08X) !\n", addr);
		return;
	}
}

//

void EMU_FASTCALL Memory_Write16(u32 addr, u32 data)
{
/*	if((((addr & RAM_MASK) <= (0x803C4BDC & RAM_MASK)) && (((addr & RAM_MASK)+1) >= (0x803C4BDC & RAM_MASK))))
	{
		printf("Writing 0x803C4BDC: %04X, PC: %08X\n", (u16)data, ireg_PC());
//		_asm {int 3};
	}
*/
/*	if (addr >= 0x90000000 && addr < 0x94000000) // MEM 2 (Wii)
	{
		*(u16 *)&Mem_RAM2[(addr ^ 2) & RAM2_MASK] = data;
		return;
	}
*/	if( addr < 0xC8000000 )				// Logical RAM
	{
		if(!(addr & 1))
			*(u16 *)(&Mem_RAM[(addr ^ 2) & RAM_MASK]) = data;
		else
		{
			addr = addr & RAM_MASK;
			Mem_RAM[(addr + 1) ^ 3] = (u8)data;
			Mem_RAM[(addr + 0) ^ 3] = (u8)(data >> 8);
		}
		return;
	}
	else if( addr >= 0xCC000000 && addr < 0xE0000000 )				// HW
	{
		Flipper_Write16(addr, data);
		return;
	}
	else if( addr < 0xCC000000 )				// EFB
	{
		ASSERT_T(1, ".Memory: Invalid Memory_Write16 to EFB!");
		return;
	}
	else if( addr < 0xF0000000 )				// L2
	{
		if(!(addr & 1))
			*(u16 *)&Mem_L2[(addr ^ 2) & L2_MASK] = data;
		else
		{
			addr = addr & L2_MASK;
			Mem_L2[(addr + 1) ^ 3] = (u8)data;
			Mem_L2[(addr + 0) ^ 3] = (u8)(data >> 8);
		}
		return;
	}
	else
	{								// IPL
//		printf(".Memory: ERROR: IPL Memory_Write16(%08X) !\n", addr);
		return;
	}
}

//

void EMU_FASTCALL Memory_Write32(u32 addr, u32 data)
{
/*	if((((addr & RAM_MASK) <= (0x803C4BDC & RAM_MASK)) && (((addr & RAM_MASK)+3) >= (0x803C4BDC & RAM_MASK))))
	{
		printf("Writing 0x803C4BDC: %08X, PC: %08X\n", data, ireg_PC());
//		_asm {int 3};
	}
*/
/*	if (addr >= 0x90000000 && addr < 0x94000000) // MEM 2 (Wii)
	{
		*(u32 *)&Mem_RAM2[addr & RAM2_MASK] = data;
		return;
	}
*/	if( addr < 0xC8000000 )				// Logical RAM
	{
		addr &= RAM_MASK;
		if(!(addr & 3))
			*(u32 *)(&Mem_RAM[addr]) = data;
		else
		{
			Mem_RAM[(addr + 3) ^ 3] = (u8)data;
			Mem_RAM[(addr + 2) ^ 3] = (u8)(data >> 8);
			Mem_RAM[(addr + 1) ^ 3] = (u8)(data >> 16);
			Mem_RAM[(addr + 0) ^ 3] = (u8)(data >> 24);
		}

		return;
	}
	else if( addr >= 0xCC000000 && addr < 0xE0000000 )				// HW
	{
		Flipper_Write32(addr, data);
		return;
	}
	else if( addr < 0xCC000000 )				// EFB
	{
		ASSERT_T(1, ".Memory: Invalid Memory_Write32 to EFB!");
		return;
	}
	else if( addr < 0xF0000000 )				// L2
	{
		if(!(addr & 3))
			*(u32 *)&Mem_L2[addr & L2_MASK] = data;
		else
		{
			addr = addr & L2_MASK;
			Mem_L2[(addr + 3) ^ 3] = (u8)data;
			Mem_L2[(addr + 2) ^ 3] = (u8)(data >> 8);
			Mem_L2[(addr + 1) ^ 3] = (u8)(data >> 16);
			Mem_L2[(addr + 0) ^ 3] = (u8)(data >> 24);
		}
		return;
	}
	else
	{								// IPL
//		printf(".Memory: ERROR: IPL Memory_Write32(%08X) !\n", addr);
		return;
	}
}

//

void EMU_FASTCALL Memory_Write64(u32 addr, u64 data)
{
	addr &= RAM_MASK;
	*(u32 *)(&Mem_RAM[addr]) = (u32)(data >> 32);
	*(u32 *)(&Mem_RAM[addr + 4]) = (u32)data;
	return;
}
