// hw_mi.cpp
// (c) 2010 Gekko Team

#include "../../emu.h"
#include "hw_mi.h"
//#include "windows.h"

//

////////////////////////////////////////////////////////////
// MI - Memory Interface
////////////////////////////////////////////////////////////

u8 MIRegisters[REG_SIZE];

////////////////////////////////////////////////////////////

// Desc: Read/Write from/to MI Hardware
//

u8 __fastcall MI_Read8(u32 addr)
{
	printf(".Flipper: Undefined MI_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall MI_Write8(u32 addr, u32 data)
{
	printf(".Flipper: Undefined MI_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall MI_Read16(u32 addr)
{
	switch(addr)
	{
	case MI_MEMREGION0_LO :
	case MI_MEMREGION0_HI :
	case MI_MEMREGION1_LO :
	case MI_MEMREGION1_HI :
	case MI_MEMREGION2_LO :
	case MI_MEMREGION2_HI :
	case MI_MEMREGION3_LO :
	case MI_MEMREGION3_HI :
	case MI_CONTROL :
		return REGMI16(addr);

	default:
		printf(".Flipper: Undefined MI_Read16: %08X!\n", addr);
		return 0;
	}
}

void __fastcall MI_Write16(u32 addr, u32 data)
{
	switch(addr)
	{
	case MI_MEMREGION0_LO :
	case MI_MEMREGION0_HI :
	case MI_MEMREGION1_LO :
	case MI_MEMREGION1_HI :
	case MI_MEMREGION2_LO :
	case MI_MEMREGION2_HI :
	case MI_MEMREGION3_LO :
	case MI_MEMREGION3_HI :
		REGMI16(addr) = data;
		break;

	/* 
	// Useless... VirtualProtect is windows secific. (mprotect for linux)
	// 
	case MI_CONTROL :
		if (REGMI16(MI_CONTROL) & MI_CHANNEL0 != data & MI_CHANNEL0)
			VirtualProtect((LPVOID)((REGMI16(MI_MEMREGION0_LO) << 16) & REGMI16(MI_MEMREGION0_HI)), 1024, data & MI_CHANNEL0, (PDWORD)(REGMI16(MI_CONTROL) & MI_CHANNEL0)); 
		if (REGMI16(MI_CONTROL) & MI_CHANNEL1 != data & MI_CHANNEL1)
			VirtualProtect((LPVOID)((REGMI16(MI_MEMREGION1_LO) << 16) & REGMI16(MI_MEMREGION1_HI)), 1024, data & MI_CHANNEL1, (PDWORD)(REGMI16(MI_CONTROL) & MI_CHANNEL1)); 
		if (REGMI16(MI_CONTROL) & MI_CHANNEL2 != data & MI_CHANNEL2)
			VirtualProtect((LPVOID)((REGMI16(MI_MEMREGION2_LO) << 16) & REGMI16(MI_MEMREGION2_HI)), 1024, data & MI_CHANNEL2, (PDWORD)(REGMI16(MI_CONTROL) & MI_CHANNEL2)); 
		if (REGMI16(MI_CONTROL) & MI_CHANNEL3 != data & MI_CHANNEL3)
			VirtualProtect((LPVOID)((REGMI16(MI_MEMREGION3_LO) << 16) & REGMI16(MI_MEMREGION3_HI)), 1024, data & MI_CHANNEL3, (PDWORD)(REGMI16(MI_CONTROL) & MI_CHANNEL3)); 
	
		REGMI16(addr) = data;
		break;
	*/

	default:
		printf(".Flipper: Undefined MI_Write16: %08X := %08X !\n", addr, data);
		return;
	}
}

u32 __fastcall MI_Read32(u32 addr)
{
	printf(".Flipper: Undefined MI_Read32: %08X!\n", addr);
	return 0;
}

void __fastcall MI_Write32(u32 addr, u32 data)
{
	printf(".Flipper: Undefined MI_Write32: %08X := %08X !\n", addr, data);
}

////////////////////////////////////////////////////////////

// Desc: Initialize MI Hardware
//

void MI_Open(void)
{
	printf("MI_Open()\n");

	memset(MIRegisters, 0, sizeof(MIRegisters));
}

void MI_Close(void)
{
	//VirtualFree(addr, size, type);
}
