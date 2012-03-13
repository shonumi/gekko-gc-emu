// hw_cp.cpp
// (c) 2005,2008 Gekko Team

#include "common.h"
#include "hw.h"
#include "hw_cp.h"
#include "hw_pi.h"
#include "hw_gx.h"

//

sCP     commandprocessor;
u8		CPRegisters[REG_SIZE];

wwpar   CP_WPAR_Write8;
wwpar   CP_WPAR_Write16;
wwpar   CP_WPAR_Write32;

////////////////////////////////////////////////////////////////////////////////
// CP - Command Processor
//
//			1/15/06		-	File Created. (ShizZy)
//			6/30/08		-	Added CPU/GP Fifo attaching and WPAR support (ShizZy)
//
//			Missing:
//
////////////////////////////////////////////////////////////////////////////////

// Desc: Read/Write from/to CP Hardware
//

u8 __fastcall CP_Read8(u32 addr)
{
	LOG_ERROR(TCP, "Undefined CP_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall CP_Write8(u32 addr, u32 data)
{
	LOG_ERROR(TCP, "Undefined CP_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall CP_Read16(u32 addr)
{
	switch(addr)
	{
		case CP_SR:
			return REGCP16(addr) | CP_SR_GPIDLECMD | CP_SR_GPIDLEREAD;

		case CP_FIFOREADWRITEDISTLO:
        case CP_FIFOREADWRITEDISTHI:
			CP_FIFO_DISTANCE = CP_FIFO_WRITE_POINTER - CP_FIFO_READ_POINTER;
             
		case CP_CR:
		case CP_CLEAR:
		case CP_TOKEN:
		case CP_BOUNDLEFT:
		case CP_BOUNDRIGHT:
		case CP_BOUNDTOP:
		case CP_BOUNDBOTTOM:
		case CP_FIFOBASELO:
		case CP_FIFOBASEHI:
		case CP_FIFOENDLO:
		case CP_FIFOENDHI:
		case CP_FIFOHIGHWATERLO:
		case CP_FIFOHIGHWATERHI:
		case CP_FIFOLOWWATERLO:
		case CP_FIFOLOWWATERHI:
		case CP_FIFOWRITELO:
		case CP_FIFOWRITEHI:
		case CP_FIFOREADLO:
		case CP_FIFOREADHI:
		case CP_FIFOBPLO:
		case CP_FIFOBPHI:
			return REGCP16(addr);

	default:
		LOG_ERROR(TCP, "Undefined CP_Read16: %08X!\n", addr);
		return 0;
	}
}

void __fastcall CP_Write16(u32 addr, u32 data)
{
	switch(addr)
	{
		case CP_SR:
			return;

		case CP_CR:

			REGCP16(addr) = data;
			if(data & CP_CR_BPCLEAR)
			{
				REGCP16(CP_SR) &= ~CP_SR_BPINT;
				PI_ClearInterrupt(PI_MASK_CP);
			}
			return;

		case CP_CLEAR:
			if(data & CP_CLEAR_OVERFLOW)
					REGCP16(CP_SR) &= ~CP_SR_GXOVERFLOW;

			if(data & CP_CLEAR_UNDERFLOW)
					REGCP16(CP_SR) &= ~CP_SR_GXUNDERFLOW;

			if(data & (CP_CLEAR_OVERFLOW | CP_CLEAR_UNDERFLOW))
				PI_ClearInterrupt(PI_MASK_CP);

			return;
			
		case CP_TOKEN:
		case CP_BOUNDLEFT:
		case CP_BOUNDRIGHT:
		case CP_BOUNDTOP:
		case CP_BOUNDBOTTOM:
		case CP_FIFOBASELO:
		case CP_FIFOBASEHI:
		case CP_FIFOENDLO:
		case CP_FIFOENDHI:
		case CP_FIFOHIGHWATERLO:
		case CP_FIFOHIGHWATERHI:
		case CP_FIFOLOWWATERLO:
		case CP_FIFOLOWWATERHI:
		case CP_FIFOREADWRITEDISTLO:
		case CP_FIFOREADWRITEDISTHI:
		case CP_FIFOWRITELO:
		case CP_FIFOWRITEHI:
		case CP_FIFOREADLO:
		case CP_FIFOREADHI:
		case CP_FIFOBPLO:
		case CP_FIFOBPHI:
			REGCP16(addr) = data;
		return;

	default:
		LOG_ERROR(TCP, "Undefined CP_Write16: %08X := %08X !\n", addr, data);
		return;
	}
}

u32 __fastcall CP_Read32(u32 addr)
{
		LOG_ERROR(TCP, " Undefined CP_Read16: %08X!\n", addr);
		return 0;
}

void __fastcall CP_Write32(u32 addr, u32 data)
{
	LOG_ERROR(TCP, "Undefined CP_Write16: %08X := %08X !\n", addr, data);
	return;
}

////////////////////////////////////////////////////////////////////////////////

// Desc: Update CP Hardware
//

void CP_Update(u32 _addr)
{
	if(commandprocessor.wpar_address != _addr)
	{           
		commandprocessor.gp_link_enable = (REGCP32(CP_SR) & CP_CR_GPLINKENABLE) >> 4;  
		
		if(commandprocessor.gp_link_enable) // attach gp fifo
		{
			commandprocessor.wpar_address = _addr & CP_WPAR_MASK;   
			CP_WPAR_Write8 = GX_Fifo_Write8;
			CP_WPAR_Write16 = GX_Fifo_Write16;
			CP_WPAR_Write32 = GX_Fifo_Write32;
        }else{ // attach cpu fifo
			commandprocessor.wpar_address = _addr;   
  			CP_WPAR_Write8 = PI_Fifo_Write8;
			CP_WPAR_Write16 = PI_Fifo_Write16;
			CP_WPAR_Write32 = PI_Fifo_Write32;
        }
	}
}

// Desc: Initialize AI Hardware
//

void CP_Open(void)
{
    LOG_NOTICE(TCP, "initialized ok");

	memset(&CPRegisters, 0, sizeof(CPRegisters));
	
	REGCP32(CP_BOUNDRIGHT) = 640;
	REGCP32(CP_BOUNDBOTTOM) = 480;

	// attach cpu fifo
	CP_WPAR_Write8 = PI_Fifo_Write8;
	CP_WPAR_Write16 = PI_Fifo_Write16;
	CP_WPAR_Write32 = PI_Fifo_Write32;
}

////////////////////////////////////////////////////////////////////////////////

