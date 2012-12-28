// hw_vi.cpp
// (c) 2005,2008 Gekko Team / Wiimu Project

#include "common.h"
#include "hw.h"
#include "hw_vi.h"
#include "hw_pi.h"
#include "hw_si.h"
#include "powerpc/cpu_core.h"
#include "powerpc/cpu_core_regs.h"

//

sVI		vi;
u8		VIRegisters[REG_SIZE];

////////////////////////////////////////////////////////////////////////////////
// VI - Video Interface
//
//			1/15/06		-	File Created. (ShizZy)
//			1/20/06		-	Basic VI implementation - no
//							VSync, faked timing. (ShizZy)
//			2/25/06		-	Fixed top frame buffer address
//							retrieval, many new demos work. (ShizZy)
//			4/10/06		-	Fixed another TBFL adddress
//							retrieval bug - fixes csn_test.dol. (ShizZy)
//			4/17/06		-	Added fixed point YUV2 to RGB (thakis)
//			6/1/06		-	Recode, added per frame VSync (ShizZy)
//			7/9/06		-	Added per scanline VSync (Lightning)
//			7/10/06		-	Added more complete VI emulation, fixed
//							some typos, added support for the remaining
//							3 display interrupts, cleaned a bit. (ShizZy)
//			7/12/06		-	Added SIPoll. (ShizZy)	
//          6/27/08     -   With video integrated into core, XFB has been 
//                          reimplemented. (ShizZy)
//
//			Missing:
//						-	Non interlaced mode (unneccesary for an emulator).
//						-	Vertical timing registers (...).
//						-	True EFB -> XFB Copy (...).
//						-	True per pixel interrupt requests (...).
////////////////////////////////////////////////////////////////////////////////

// Desc: Read/Write from/to VI Hardware
//

u8 EMU_FASTCALL VI_Read8(u32 addr)
{
	printf("~Flipper: Undefined VI_Read8: %08X!\n", addr);
	return 0;
}

void EMU_FASTCALL VI_Write8(u32 addr, u32 data)
{
	printf("~Flipper: Undefined VI_Write8: %08X := %08X !\n", addr, data);
}

u16 EMU_FASTCALL VI_Read16(u32 addr)
{
	switch(addr)
	{
	case VI_VTR:				// Vertical Timing Register
	case VI_DCR:				// Control Register
	case VI_TFBL:				// Top Frame Buffer Address
	case (VI_TFBL + 2):			// ...
	case VI_BFBL:				// Bottom Frame Buffer Address (Unused)
	case (VI_BFBL + 2):			// ...
	case VI_DI0:				// Display Interrupt Registers
	case (VI_DI0 + 2):			// ...
	case VI_DI1:				// ...
	case (VI_DI1 + 2):			// ...
	case VI_DI2:				// ...
	case (VI_DI2 + 2):			// ...
	case VI_DI3:				// ...
	case (VI_DI3 + 2):			// ...
	case VI_VICLK:			
		return REGVI16(addr);
/*
	case VI_DPV:				// Vertical Position of Rasterbeam (Speed hack)
		if (REGVI16(addr) == 0)
		{
			REGVI16(addr) = 1;
			return 0;
		}
		else if (REGVI16(addr) == 1)
			return (REGVI16(addr) = 200);
		else if (REGVI16(addr) == 200)
			return (REGVI16(addr) = 522);
		else if (REGVI16(addr) == 522)
			return (REGVI16(addr) = 574);
		else
		{
			if(ireg.IC > 0xFFF)
			{	
				ireg.IC = 0;
				VI_YCbCr2RGB();
#ifndef USE_DOLPHIN_PLUGIN
				RenderGL();
#endif
			}

			return (REGVI16(addr) = 0);
		}
	case VI_DPH:				// Horizontal Position of Rasterbeam (Speed Hack)
		return 512;
*/
	case VI_DPV:
		return REGVI16(VI_DPV);

	case VI_DPH:
		return (REGVI16(VI_DPH) ^= 639);

	default:
		printf("~Flipper: Undefined VI_Read16: %08X!\n", addr);
		return REGVI16(addr);
	}
}

void EMU_FASTCALL VI_Write16(u32 addr, u32 data)
{
	switch(addr)
	{
	case VI_TFBL:				// Hi - Top Frame Buffer Address
	case (VI_TFBL + 2):			// Lo - Top Frame Buffer Address
		REGVI16(addr) = data;
		
		// Calculate Address of External Framebuffer in main RAM.
		vi.xfb_addr = (REGVI32(VI_TFBL) & 0xFFFFFF) >> ((data & 0x10000000) ? 5 : 0);
		
		// Set a pointer to the framebuffer.
		vi.xfbbuf = &Mem_RAM[vi.xfb_addr];

		return;

	case VI_VTR:				// Vertical Timing Register
	case VI_DCR:				// Control Register
	case VI_BFBL:				// Bottom Frame Buffer Address (Unused)
	case (VI_BFBL + 2):	
	case VI_DPV:				// Display Position (vertical)
	case VI_DPH:				// Display Position (horizontal)
	case VI_VICLK:
		REGVI16(addr) = data;
		return;

	case VI_DI0:				// Display Interrupt Registers Hi
	case VI_DI1:				// ...
	case VI_DI2:				// ...
	case VI_DI3:				// ...
	case (VI_DI0 + 2):			// ...
	case (VI_DI1 + 2):			// ...
	case (VI_DI2 + 2):			// ...
	case (VI_DI3 + 2):			// ...
		REGVI16(addr) = data;
		vi.vct[(addr - VI_DI0) / 4] = VI_DI_VER(REGVI32(addr & ~3));

		if((REGVI32(addr & ~3) & VI_DI_INT) == 0)
		{
			PI_ClearInterrupt(PI_MASK_VI);
		}
		return;

	default:
		//printf("~Flipper: Undefined VI_Write16: %08X := %08X !\n", addr, data);
		REGVI16(addr) = data;
		return;
	}
}

u32 EMU_FASTCALL VI_Read32(u32 addr)
{
	switch(addr)
	{
	case VI_DCR:				// Control Register
		return (u32)REGVI16(addr);

	case VI_TFBL:				// Top Frame Buffer Address
	case VI_BFBL:				// Bottom Frame Buffer Address
	case VI_DI0:				// Display Interrupt Registers
	case VI_DI1:				// ...
	case VI_DI2:				// ...
	case VI_DI3:				// ...
		return REGVI32(addr);

	case VI_DPV:				// Rasterbeam Vert/Hoz Position
		return ((REGVI16(VI_DPV) << 16) | (REGVI16(VI_DPV) ^= 639));
	
	default:
		printf("~Flipper: Undefined VI_Read32: %08X!\n", addr);
		return REGVI32(addr);
	}
}

void EMU_FASTCALL VI_Write32(u32 addr, u32 data)
{
	switch(addr)
	{
	case VI_TFBL:				// Top Frame Buffer Address
		REGVI32(addr) = data;
		vi.xfb_addr = (REGVI32(addr) & 0xFFFFFF) >> ((data & 0x10000000) ? 5 : 0);
		vi.xfbbuf = &Mem_RAM[vi.xfb_addr];
		return;

	case VI_BFBL: 				// Bottom Frame Buffer Address
	case VI_DPV:				// Rasterbeam Vert/Hoz Position (Two 16bit Regs)
		REGVI32(addr) = data;
		return;

	case VI_DI0:				// Display Interrupt Registers
	case VI_DI1:				// ...
	case VI_DI2:				// ...
	case VI_DI3:				// ...
		REGVI32(addr) = data;
		vi.vct[(addr - VI_DI0) / 4] = VI_DI_VER(data);
		if((REGVI32(addr) & VI_DI_INT) == 0)
		{
			// Reset VSync Interrupt.
			PI_ClearInterrupt(PI_MASK_VI);
		}
		return;

	default:
		printf("~Flipper: Undefined VI_Write32: %08X := %08X !\n", addr, data);
		REGVI32(addr) = data;
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////

// Desc: Set TV Mode - Reads Control Register and sets VI modes.
//

void VI_SetMode(void)
{
	vi.format = ((REGVI16(VI_DCR) >> 8) & 3);				// Read DCR format bits.
	vi.is_interlaced = REGVI16(VI_DCR) & VI_CR_INTERLACED;	// Read DCR interlaced bit.
															//	Note - Only Interlaced mode needed for emu.
	switch(vi.format)										// Set the correct mode.
	{
	case VI_NTSC:
	case VI_DEBUG:
	default:
		vi.framerate = 30;
		vi.vretrace = VI_NTSC_INTER;
		vi.tickcount = (((cpu->GetTicksPerSecond() / vi.framerate) / vi.vretrace));
		break;
	case VI_PAL:
	case VI_MPAL:
		vi.framerate = 25;
		vi.vretrace = VI_PAL_INTER;
		vi.tickcount = (((cpu->GetTicksPerSecond() / vi.framerate) / vi.vretrace));
		break;
	}
}

// Desc: XFB YCbCr to RGB - This reads the YUV2 (YCbCr) color data from the 
// 640 by 480 pixel external framebuffer in main RAM, converts it to RGB,
// and then copies it into our OpenGL buffer.
//

void VI_YCbCr2RGB(void)
{
	u8	Ya,	U,	Yb,	V;

	for(int i = 0, j = 0; i<((FB_WIDTH*FB_HEIGHT*FB_YUYV)/2); i+=4,j+=8)
	{
		// Extract YUV2 data for 2 pixels
/*
		Ya	= vi.xfbbuf[i];
		U	= vi.xfbbuf[i+1];
		Yb	= vi.xfbbuf[i+2];
		V	= vi.xfbbuf[i+3];
*/
		Ya	= vi.xfbbuf[i+3];
		U	= vi.xfbbuf[i+2];
		Yb	= vi.xfbbuf[i+1];
		V	= vi.xfbbuf[i+0];

		// Fixed Point YUV2 2 RGB Conversions
		s32 C = Ya - 16;
		s32 D = U - 128;
		s32 E = V - 128;

		s32 r = ((298*C         + 409*E + 128) >> 8);
		s32 g = ((298*C - 100*D - 208*E + 128) >> 8);
		s32 b = ((298*C + 516*D         + 128) >> 8);

		vi.fb_data[j]	= (u8)BCLAMP(r);
		vi.fb_data[j+1]= (u8)BCLAMP(g);
		vi.fb_data[j+2]= (u8)BCLAMP(b);

		C = Yb - 16;
		r = ((298*C         + 409*E + 128) >> 8);
		g = ((298*C - 100*D - 208*E + 128) >> 8);
		b = ((298*C + 516*D         + 128) >> 8);

		vi.fb_data[j+4]= (u8)BCLAMP(r);
		vi.fb_data[j+5]= (u8)BCLAMP(g);
		vi.fb_data[j+6]= (u8)BCLAMP(b);
	}
}

// Desc: Update VI hardware (Per Scanline)
//

void VI_Update(void)
{
	if(ireg.TBR.TBR >= vi.timer)
	{
		// Reset Timer
		vi.timer = ireg.TBR.TBR + vi.tickcount;

		VI_SCANLINE++;

		if( VI_SCANLINE == vi.vct[0] ||
			VI_SCANLINE == vi.vct[1] ||
			VI_SCANLINE == vi.vct[2] ||
			VI_SCANLINE == vi.vct[3]
			)
		{
			// Check VSync Interrupts

			if(REGVI32(VI_DI0) & VI_DI_ENB)
				REGVI32(VI_DI0) |= VI_DI_INT;
			if(REGVI32(VI_DI1) & VI_DI_ENB)
				REGVI32(VI_DI1) |= VI_DI_INT;
			if(REGVI32(VI_DI2) & VI_DI_ENB)
				REGVI32(VI_DI2) |= VI_DI_INT;
			if(REGVI32(VI_DI3) & VI_DI_ENB)
				REGVI32(VI_DI3) |= VI_DI_INT;

			if((REGVI32(VI_DI0) | REGVI32(VI_DI1) | REGVI32(VI_DI2) | REGVI32(VI_DI3))
				& VI_DI_ENB)
			{
				PI_RequestInterrupt(PI_MASK_VI);
			}
		}

		if(VI_SCANLINE > vi.vretrace)
		{
			VI_SCANLINE = 1;

			// Poll Joypads

			SI_Poll();

			// Set Television Mode

			VI_SetMode();

			// Update Framebuffer (if enabled)
#pragma todo(Reimplement enable framebuffer feature)
//			if(cfg.enb_framebuffer)
//			{
//   			VI_YCbCr2RGB();
//              OPENGL_DrawFramebuffer();
//              OPENGL_Render();
//            }
		}
	}
}

// Desc: Initialize VI Hardware
//

void VI_Open(void)
{
	LOG_NOTICE(TVI, "initialized ok");

	// Clear Registers
	memset(&VIRegisters, 0, sizeof(VIRegisters));

	// Assume NTSC until program changes it.
	vi.is_xfb = false;
	vi.is_autosync = true;
	vi.framerate = 30;
	vi.vretrace = VI_NTSC_NON_INTER;
	vi.tickcount = (((cpu->GetTicksPerSecond() / vi.framerate) / vi.vretrace));
	vi.timer = 0;

	// Point FB in RAM
	vi.xfbbuf = &Mem_RAM[0];
}

////////////////////////////////////////////////////////////////////////////////
