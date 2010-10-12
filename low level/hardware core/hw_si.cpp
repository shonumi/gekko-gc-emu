// hw_si.cpp
// (c) 2005,2008 Gekko Team

#include "../../emu.h"
#include "hw_si.h"
#include "hw_pi.h"

//

sSI		si;
u8		SIRegisters[REG_SIZE];

////////////////////////////////////////////////////////////
// SI - Serial Interface
// The Serial Interface is an interface for controlling data
// flow through Gamecube's 4 serial ports.  These ports are
// used for the Gamecube's game pads.  
//
//			1/15/06		-	File Created. (ShizZy)
//			2/1/06		-	Basic SI implementation - no
//							C-Stick or external interrupts.
//							Does not work with SDK. (ZeZu) 
//			7/12/06		-	Complete rewrite.  Should work
//							with anything built with SDK. (ShizZy)
//			4/25/08		-	Modified to use Dolwin's plugin specs
//							Added joypad support for channels 1-3 (Chrono)
//			6/25/08		-	Eliminated plugin system, new config.
//
//			Missing:
//						-	Rumble Support
//						-	L and R Trigger Pressure Sensitivity
////////////////////////////////////////////////////////////

inline static u32 SI_PadGetWord_Hi(int _pad)
{
	return (	(si.pad[_pad].buttons & 0x1FFF0000) | 
				((u32)si.pad[_pad].aX << 8) | 
				((u32)si.pad[_pad].aY)
			);
}

inline static u32 SI_PadGetWord_Lo(int _pad)
{
	return (	
				((u32)si.pad[_pad].cX << 24) | 
				((u32)si.pad[_pad].cY << 16) | 
				((u32)si.pad[_pad].aL << 8) | 
				((u32)si.pad[_pad].aR)
			);
}

// Desc: Read/Write from/to SI Hardware
//

u8 __fastcall SI_Read8(u32 addr)
{
	printf("~Flipper: Undefined SI_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall SI_Write8(u32 addr, u32 data)
{
	printf("~Flipper: Undefined SI_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall SI_Read16(u32 addr)
{
	printf("~Flipper: Undefined SI_Read16: %08X!\n", addr);
	return 0;
}

void __fastcall SI_Write16(u32 addr, u32 data)
{
	printf("~Flipper: Undefined SI_Write16: %08X := %08X !\n", addr, data);
}

u32 __fastcall SI_Read32(u32 addr)
{
	switch(addr)
	{
	case SI_C0OUTBUF:						// Channel 0 Output Buffer
	case SI_C1OUTBUF:						// Channel 0 Output Buffer
	case SI_C2OUTBUF:						// Channel 0 Output Buffer
	case SI_C3OUTBUF:						// Channel 0 Output Buffer
	case SI_CTRLSTATUS:						// Communication Control Status Register
	case SI_POLL:							// Poll Register
	case SI_EXILK:							// EXI Clock Register
	case SI_STATUS:							// Status Register
		return REGSI32(addr);	


	case SI_C0INBUFH:						// Input Buffer Hi
		REGSI32(SI_CTRLSTATUS) &= ~SI_CSTAT_RDSTINT;
		PI_ClearInterrupt(PI_MASK_SI);
		return SI_PadGetWord_Hi(0) | 0x00800000; // force this bit on to give analog pad meaning in many games (gcemu)
	case SI_C1INBUFH:						// Input Buffer Hi
		REGSI32(SI_CTRLSTATUS) &= ~SI_CSTAT_RDSTINT;
		PI_ClearInterrupt(PI_MASK_SI);
		return SI_PadGetWord_Hi(1) | 0x00800000; // force this bit on to give analog pad meaning in many games (gcemu)
	case SI_C2INBUFH:						// Input Buffer Hi
		REGSI32(SI_CTRLSTATUS) &= ~SI_CSTAT_RDSTINT;
		PI_ClearInterrupt(PI_MASK_SI);
		return SI_PadGetWord_Hi(2) | 0x00800000; // force this bit on to give analog pad meaning in many games (gcemu)
	case SI_C3INBUFH:						// Input Buffer Hi
		REGSI32(SI_CTRLSTATUS) &= ~SI_CSTAT_RDSTINT;
		PI_ClearInterrupt(PI_MASK_SI);
		return SI_PadGetWord_Hi(3) | 0x00800000; // force this bit on to give analog pad meaning in many games (gcemu)

	case SI_C0INBUFL:						// Input Buffer Lo
		return SI_PadGetWord_Lo(0);
	case SI_C1INBUFL:						// Input Buffer Lo
		return SI_PadGetWord_Lo(1);
	case SI_C2INBUFL:						// Input Buffer Lo
		return SI_PadGetWord_Lo(2);
	case SI_C3INBUFL:						// Input Buffer Lo
		return SI_PadGetWord_Lo(3);

	default:
		if((u32)(addr - SI_BUFFSTART) <= 0x80)
		{									// Access the SI Input/Output Buffer
			return REGSI32(addr);
		}

		printf("~Flipper: Undefined SI_Read32: %08X!\n", addr);
		return 0;
	}
}

void __fastcall SI_Write32(u32 addr, u32 data)
{
	switch(addr)
	{
	case SI_C0OUTBUF:						// Channel 0 Output Buffer						
		si.shadow[0] = data;
		REGSI32(SI_STATUS) |= SI_STAT_WRST0;		// Buffer Not Copied
		return;

	case SI_C1OUTBUF:						// Channel 1 Output Buffer						
		si.shadow[1] = data;
		REGSI32(SI_STATUS) |= SI_STAT_WRST1;		// Buffer Not Copied
		return;

	case SI_C2OUTBUF:						// Channel 2 Output Buffer						
		si.shadow[2] = data;
		REGSI32(SI_STATUS) |= SI_STAT_WRST2;		// Buffer Not Copied
		return;

	case SI_C3OUTBUF:						// Channel 3 Output Buffer						
		si.shadow[3] = data;
		REGSI32(SI_STATUS) |= SI_STAT_WRST3;		// Buffer Not Copied
		return;

	case SI_CTRLSTATUS:						// Communication Control Status Register
		if(data & SI_CSTAT_TCINT)					// Clear Transfer Complete Interrupt
		{
			REGSI32(SI_CTRLSTATUS) &= ~SI_CSTAT_TCINT;
			PI_ClearInterrupt(PI_MASK_SI);
		}
													// Copy RDST Interrupt Mask bit from Data
		if(data & SI_CSTAT_RDSTINTMSK) REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINTMSK;
		else REGSI32(SI_CTRLSTATUS) &= ~SI_CSTAT_RDSTINTMSK;

		if(data & SI_CSTAT_TSTART)
		{
			SI_ProcessCommand();

			REGSI32(SI_CTRLSTATUS) &= ~SI_CSTAT_TSTART;
			REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_TCINT;

			if(data & SI_CSTAT_TCINTMSK)
			{
				REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_TCINTMSK;
				PI_RequestInterrupt(PI_MASK_SI);
			}
		}

		return;

	case SI_STATUS:							// Status Register
		if(data & SI_STAT_WR)						// Copy Shadow Registers
		{
			REGSI32(SI_C0OUTBUF) = si.shadow[0];
			REGSI32(SI_STATUS) &= ~SI_STAT_WRST0;	// Set SR bit to copy done.

			REGSI32(SI_C1OUTBUF) = si.shadow[1];
			REGSI32(SI_STATUS) &= ~SI_STAT_WRST1;	// Set SR bit to copy done.

			REGSI32(SI_C2OUTBUF) = si.shadow[2];
			REGSI32(SI_STATUS) &= ~SI_STAT_WRST2;	// Set SR bit to copy done.

			REGSI32(SI_C3OUTBUF) = si.shadow[3];
			REGSI32(SI_STATUS) &= ~SI_STAT_WRST3;	// Set SR bit to copy done.
		}

		return;

	case SI_C0INBUFH:						// Input Buffer (Read-Only)
	case SI_C0INBUFL:						// ...
	case SI_C1INBUFH:						// Input Buffer (Read-Only)
	case SI_C1INBUFL:						// ...
	case SI_C2INBUFH:						// Input Buffer (Read-Only)
	case SI_C2INBUFL:						// ...
	case SI_C3INBUFH:						// Input Buffer (Read-Only)
	case SI_C3INBUFL:						// ...
		return;

	case SI_POLL:							// Poll Register
	case SI_EXILK:							// EXI Clock Register
		REGSI32(addr) = data;
		return;

	default:
		if((u32)(addr - SI_BUFFSTART) <= 0x80)
		{									// Access the SI Input/Output Buffer
			REGSI32(addr) = data;
			return;
		}

		printf("~Flipper: Undefined SI_Write32: %08X := %08X !\n", addr, data);
	}
}

////////////////////////////////////////////////////////////

// Desc: Process a SI command - Special thanks to or9 for reversing :-)
//

void SI_ProcessCommand(void)
{
	switch(REGSI8(SI_BUFFSTART))
	{
	case 0x0:
		REGSI8(SI_BUFFSTART) = 9;
		REGSI8(SI_BUFFSTART + 1) = 0;
		REGSI8(SI_BUFFSTART + 2) = 0;

		break;

	case 0x41:
		REGSI8(SI_BUFFSTART + 1) = 0;

		REGSI8(SI_BUFFSTART + 2) = 
		REGSI8(SI_BUFFSTART + 3) = 
		REGSI8(SI_BUFFSTART + 4) =
		REGSI8(SI_BUFFSTART + 5) = 0x80;

		REGSI8(SI_BUFFSTART + 6) = 
		REGSI8(SI_BUFFSTART + 7) = 0x1f;
		            
		break;
	}
}

// Desc: Read Joypad keys - (Could go in a plugin)
//

void SI_ReadKeys(int _channel)
{
	if(emu.keys[jp_cfg.pads[_channel].analog[0]])					// Analog Stick Y Axis Up
	{
		si.pad[_channel].aY = A_HIGH;
		goto sec_1;
	}else{
		si.pad[_channel].aY = A_NEUTRAL;
	}

	if(emu.keys[jp_cfg.pads[_channel].analog[1]])					// Analog Stick Y Axis Down
	{
		si.pad[_channel].aY = A_LOW;
	}else{
		si.pad[_channel].aY = A_NEUTRAL;
	}

sec_1:
	if(emu.keys[jp_cfg.pads[_channel].analog[2]])					// Analog Stick X Axis Left				
	{
		si.pad[_channel].aX = A_LOW;
		goto sec_2;
	}else{
		si.pad[_channel].aX = A_NEUTRAL;
	}

	if(emu.keys[jp_cfg.pads[_channel].analog[3]])					// Analog Stick X Axis Right
	{
		si.pad[_channel].aX = A_HIGH;
	}else{
		si.pad[_channel].aX = A_NEUTRAL;
	}

sec_2:
	if(emu.keys[jp_cfg.pads[_channel].cstick[0]])					// C Stick Y Axis Up
	{
		si.pad[_channel].cY = A_HIGH;
		goto sec_3;
	}else{
		si.pad[_channel].cY = A_NEUTRAL;
	}

	if(emu.keys[jp_cfg.pads[_channel].cstick[1]])					// C Stick Y Axis Down
	{
		si.pad[_channel].cY = A_LOW;
	}else{
		si.pad[_channel].cY = A_NEUTRAL;
	}

sec_3:
	if(emu.keys[jp_cfg.pads[_channel].cstick[2]])					// C Stick X Axis Left
	{
		si.pad[_channel].cX = A_LOW;
		goto sec_4;
	}else{
		si.pad[_channel].cX = A_NEUTRAL;
	}

	if(emu.keys[jp_cfg.pads[_channel].cstick[3]])					// C Stick X Axis Right
	{
		si.pad[_channel].cX = A_HIGH;
	}else{
		si.pad[_channel].cX = A_NEUTRAL;
	}

sec_4:
	if(emu.keys[jp_cfg.pads[_channel].start])				// Start Button
	{
		si.pad[_channel].buttons |= B_START;
	}else{
		si.pad[_channel].buttons &= ~B_START;
	}

	if(emu.keys[jp_cfg.pads[_channel].b])				// B Button
	{
		si.pad[_channel].buttons |= B_B;
	}else{
		si.pad[_channel].buttons &= ~B_B;
	}

	if(emu.keys[jp_cfg.pads[_channel].a])				// A Button
	{
		si.pad[_channel].buttons |= B_A;
	}else{
		si.pad[_channel].buttons &= ~B_A;
	}

	if(emu.keys[jp_cfg.pads[_channel].x])					// X Button
	{
		si.pad[_channel].buttons |= B_X;
	}else{
		si.pad[_channel].buttons &= ~B_X;
	}

	if(emu.keys[jp_cfg.pads[_channel].y])					// Y Button
	{
		si.pad[_channel].buttons |= B_Y;
	}else{
		si.pad[_channel].buttons &= ~B_Y;
	}

	if(emu.keys[jp_cfg.pads[_channel].z])					// Z Button
	{
		si.pad[_channel].buttons |= B_Z;
	}else{
		si.pad[_channel].buttons &= ~B_Z;
	}

	if(emu.keys[jp_cfg.pads[_channel].l])					// L Trigger
	{
		si.pad[_channel].buttons |= B_L;
		si.pad[_channel].aL = T_PRESSED;
	}else{
		si.pad[_channel].buttons &= ~B_L;
		si.pad[_channel].aL = T_RELEASED;
	}

	if(emu.keys[jp_cfg.pads[_channel].r])					// R Triger
	{
		si.pad[_channel].buttons |= B_R;
		si.pad[_channel].aR = T_PRESSED;
	}else{
		si.pad[_channel].buttons &= ~B_R;
		si.pad[_channel].aR = T_RELEASED;
	}

	if(emu.keys[jp_cfg.pads[_channel].dpad[0]])					// Directional Pad Up
	{
		si.pad[_channel].buttons |= B_UP;
	}else{
		si.pad[_channel].buttons &= ~B_UP;
	}

	if(emu.keys[jp_cfg.pads[_channel].dpad[1]])					// Directional Pad Down
	{
		si.pad[_channel].buttons |= B_DN;
	}else{
		si.pad[_channel].buttons &= ~B_DN;
	}

	if(emu.keys[jp_cfg.pads[_channel].dpad[2]])					// Directional Pad Left
	{
		si.pad[_channel].buttons |= B_LT;
	}else{
		si.pad[_channel].buttons &= ~B_LT;
	}

	if(emu.keys[jp_cfg.pads[_channel].dpad[3]])					// Directional Pad Right
	{
		si.pad[_channel].buttons |= B_RT;
	}else{
		si.pad[_channel].buttons &= ~B_RT;
	}
}

// Desc: Poll for button presses
//

void SI_Poll(void)
{
	// Channel 0:

	if(REGSI32(SI_POLL) & SI_POLL_ENB0)
	{
		if(si.pad_connected[0])
		{
			SI_ReadKeys(0);

			REGSI32(SI_STATUS) |= SI_STAT_RDST0;
			REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINT;
		}
	}

	// Channel 1:

	if(REGSI32(SI_POLL) & SI_POLL_ENB1)
	{
		if(si.pad_connected[1])
		{
			SI_ReadKeys(1);

			REGSI32(SI_STATUS) |= SI_STAT_RDST1;
			REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINT;
		}
	}

	// Channel 2:

	if(REGSI32(SI_POLL) & SI_POLL_ENB2)
	{
		if(si.pad_connected[2])
		{
			SI_ReadKeys(2);

			REGSI32(SI_STATUS) |= SI_STAT_RDST2;
			REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINT;
		}
	}

	// Channel 3:
	
	if(REGSI32(SI_POLL) & SI_POLL_ENB3)
	{
		if(si.pad_connected[3])
		{
			SI_ReadKeys(3);

			REGSI32(SI_STATUS) |= SI_STAT_RDST3;
			REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINT;
		}
	}

	// Request Interrupt:

	if( (REGSI32(SI_CTRLSTATUS) & SI_CSTAT_RDSTINT) &&
		(REGSI32(SI_CTRLSTATUS) & SI_CSTAT_RDSTINTMSK) )
	{
		PI_RequestInterrupt(PI_MASK_SI);
	}
}

// Desc: Initialize SI Hardware
//

void SI_Open(void)
{
	printf("SI_Open()\n");

	// Reset Device

	memset(&si, 0, sizeof(sSI));

	// Runtime Initialization (IPL)
	REGSI32(SI_C0OUTBUF) = 
	REGSI32(SI_C1OUTBUF) = 
	REGSI32(SI_C2OUTBUF) = 
	REGSI32(SI_C3OUTBUF) = 0x00400300;

	REGSI32(SI_POLL) |= 
	  (	SI_POLL_ENB0 |
		SI_POLL_ENB1 |
		SI_POLL_ENB2 |
		SI_POLL_ENB3 );

    // enabled/disable pads
    for(int i = 0; i < 4; i++)
	{
        si.pad_connected[i] = jp_cfg.pads[i].is_on;
	}
}

////////////////////////////////////////////////////////////
