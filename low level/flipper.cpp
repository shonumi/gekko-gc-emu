// flipper.cpp
// (c) 2005,2006 Gekko Team

#include "../emu.h"
#include "hardware core/hw_pe.h"
#include "hardware core/hw_vi.h"
#include "hardware core/hw_si.h"
#include "hardware core/hw_pi.h"
#include "hardware core/hw_exi.h"
#include "hardware core/hw_mi.h"
#include "hardware core/hw_dsp.h"
#include "hardware core/hw_gx.h"
#include "hardware core/hw_ai.h"
#include "hardware core/hw_di.h"
#include "hardware core/hw_cp.h"
#include "../cpu core/cpu_core_regs.h"

////////////////////////////////////////////////////////////

// Desc: Update Flipper Hardware
//

u32 Flipper_Update(void)
{
	static u32 FlipCount = 0;

	FlipCount++;

	if(!(FlipCount & 0x7F))
	{
		FlipCount = 0;
		{
			DSP_Update();
			EXI_Update();
			VI_Update();
			AI_Update();
		}
	}

	return PI_CheckForInterrupts();
}

// Desc: Initialize Flipper Hardware
//

void Flipper_Open(void)
{
	CP_Open();
	PE_Open();
	PI_Open();
	VI_Open();
	SI_Open();
	MI_Open();
	DSP_Open();
	EXI_Open();
	AI_Open();
	DI_Open();
	GX_Open();
}

// Desc: Shutdown Flipper Hardware
//

void Flipper_Close(void)
{
	EXI_Close();
	DI_Close();
}


////////////////////////////////////////////////////////////

// Desc: Write 8bit Data to Flipper Hardware
//

void __fastcall Flipper_Write8(u32 addr, u32 data)
{
	if(HARDWARE_ADDR == GX_Fifo)
		CP_WPAR_Write8(addr, data);
	else
	{
		switch(HARDWARE_ADDR)
		{
		case CP_Regs:	CP_Write8(addr, data);	break;
		case PE_Regs:	PE_Write8(addr, data);	break;
		case VI_Regs:	VI_Write8(addr, data);	break;
		case PI_Regs:	PI_Write8(addr, data);	break;
		case MI_Regs:	MI_Write8(addr, data);	break;
		case DSP_Regs:	DSP_Write8(addr, data);	break;
		case DI_Regs:	DI_Write8(addr, data);	break;
		case SI_Regs:	SI_Write8(addr, data);	break;
		case EXI_Regs:	EXI_Write8(addr, data);	break;
		case AI_Regs:	AI_Write8(addr, data); break;

		default:
/*				if((addr & CP_WPAR_MASK) == commandprocessor.wpar_address)
   					CP_WPAR_Write8(addr, (u32)data);
				else
*/					printf(".Flipper: ERROR: Non-Existent Flipper_Write8(%08X) !\n", addr);
		}
	}
}

// Desc: Write 16bit Data to Flipper Hardware
//

void __fastcall Flipper_Write16(u32 addr, u32 data)
{
	if(HARDWARE_ADDR == GX_Fifo)
		CP_WPAR_Write16(addr, data);
	else
	{
		switch(HARDWARE_ADDR)
		{
		case CP_Regs:	CP_Write16(addr, data);	break;
		case PE_Regs:	PE_Write16(addr, data);	break;
		case VI_Regs:	VI_Write16(addr, data);	break;
		case PI_Regs:	PI_Write16(addr, data);	break;
		case MI_Regs:	MI_Write16(addr, data);	break;
		case DSP_Regs:	DSP_Write16(addr, data);	break;
		case DI_Regs:	DI_Write16(addr, data); break;
		case SI_Regs:	SI_Write16(addr, data);	break;
		case EXI_Regs:	EXI_Write16(addr, data); break;
		case AI_Regs:	AI_Write16(addr, data); break;

		default:
/*				if((addr & CP_WPAR_MASK) == commandprocessor.wpar_address)
   					CP_WPAR_Write16(addr, (u32)data);
				else
*/					printf(".Flipper: ERROR: Non-Existent Flipper_Write16(%08X) !\n", addr);
		}
	}
}

// Desc: Write 32bit Data to Flipper Hardware
//

void __fastcall Flipper_Write32(u32 addr, u32 data)
{
	if(HARDWARE_ADDR == GX_Fifo)
		CP_WPAR_Write32(addr, data);
	else
	{
		switch(HARDWARE_ADDR)
		{
		case CP_Regs:	CP_Write32(addr, data);	break;
		case PE_Regs:	PE_Write32(addr, data);	break;
		case VI_Regs:	VI_Write32(addr, data);	break;
		case PI_Regs:	PI_Write32(addr, data);	break;
		case MI_Regs:	MI_Write32(addr, data);	break;
		case DSP_Regs:	DSP_Write32(addr, data);	break;
		case DI_Regs:	DI_Write32(addr, data); break;
		case SI_Regs:	SI_Write32(addr, data);	break;
		case EXI_Regs:	EXI_Write32(addr, data);	break;
		case AI_Regs:	AI_Write32(addr, data);	break;

		default:
/*				if((addr & CP_WPAR_MASK) == commandprocessor.wpar_address)
   					CP_WPAR_Write32(addr, data);
				else
*/					printf(".Flipper: ERROR: Non-Existent Flipper_Write32(%08X) !\n", addr);
		}
	}
}

////////////////////////////////////////////////////////////

// Desc: Read 8bit Data from Flipper Hardware
//

u8 __fastcall Flipper_Read8(u32 addr)
{
	switch(HARDWARE_ADDR)
	{
	case CP_Regs:	return CP_Read8(addr);
	case PE_Regs:	return PE_Read8(addr);
	case VI_Regs:	return VI_Read8(addr);
	case PI_Regs:	return PI_Read8(addr);
	case MI_Regs:	return MI_Read8(addr);
	case DSP_Regs:	return DSP_Read8(addr);
	case DI_Regs:	return DI_Read8(addr);
	case SI_Regs:	return SI_Read8(addr);
	case EXI_Regs:	return EXI_Read8(addr);
	case AI_Regs:	return AI_Read8(addr);

	default:
		printf(".Flipper: ERROR: Non-Existent Fipper_Read8(%08X) !\n", addr);
	}

	return 0x0;
}

// Desc: Read 16bit Data from Flipper Hardware
//

u16 __fastcall Flipper_Read16(u32 addr)
{
	switch(HARDWARE_ADDR)
	{
	case CP_Regs:	return CP_Read16(addr);
	case PE_Regs:	return PE_Read16(addr);
	case VI_Regs:	return VI_Read16(addr);
	case PI_Regs:	return PI_Read16(addr);
	case MI_Regs:	return MI_Read16(addr);
	case DSP_Regs:	return DSP_Read16(addr);
	case DI_Regs:	return DI_Read16(addr);
	case SI_Regs:	return SI_Read16(addr);
	case EXI_Regs:	return EXI_Read16(addr);
	case AI_Regs:	return AI_Read16(addr);

	default:
		printf(".Flipper: ERROR: Non-Existent Fipper_Read16(%08X) !\n", addr);
	}

	return 0x0;
}

// Desc: Read 32bit Data from Flipper Hardware
//

u32 __fastcall Flipper_Read32(u32 addr)
{
	switch(HARDWARE_ADDR)
	{
	case CP_Regs:	return CP_Read32(addr);
	case PE_Regs:	return PE_Read32(addr);
	case VI_Regs:	return VI_Read32(addr);
	case PI_Regs:	return PI_Read32(addr);
	case MI_Regs:	return MI_Read32(addr);
	case DSP_Regs:	return DSP_Read32(addr);
	case DI_Regs:	return DI_Read32(addr);
	case SI_Regs:	return SI_Read32(addr);
	case EXI_Regs:	return EXI_Read32(addr);
	case AI_Regs:	return AI_Read32(addr);	

	default:
		printf(".Flipper: ERROR: Non-Existent Fipper_Read16(%08X) !\n", addr);
	}

	return 0x0;
}

////////////////////////////////////////////////////////////
