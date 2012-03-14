// hw_si.cpp
// (c) 2005,2008 Gekko Team

#include "common.h"
#include "config.h"

#include "input_common.h"

#include "hw.h"
#include "hw_si.h"
#include "hw_pi.h"

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
    LOG_ERROR(TSI, "Undefined SI_Read8: %08X!\n", addr);
    return 0;
}

void __fastcall SI_Write8(u32 addr, u32 data)
{
    LOG_ERROR(TSI, "Undefined SI_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall SI_Read16(u32 addr)
{
    LOG_ERROR(TSI, "Undefined SI_Read16: %08X!\n", addr);
    return 0;
}

void __fastcall SI_Write16(u32 addr, u32 data)
{
    LOG_ERROR(TSI, "Undefined SI_Write16: %08X := %08X !\n", addr, data);
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

        LOG_ERROR(TSI, "Undefined SI_Read32: %08X!\n", addr);
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

        LOG_ERROR(TSI, "Undefined SI_Write32: %08X := %08X !\n", addr, data);
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
sec_0:
    // Analog Stick Y Axis Up
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->analog_stick_status(input_common::GCController::STICK_UP))) {
        si.pad[_channel].aY = A_HIGH;
        goto sec_1;
    }else{
        si.pad[_channel].aY = A_NEUTRAL;
    }

    // Analog Stick Y Axis Down
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->analog_stick_status(input_common::GCController::STICK_DOWN))) {
        si.pad[_channel].aY = A_LOW;
    }else{
        si.pad[_channel].aY = A_NEUTRAL;
    }

sec_1:
    // Analog Stick X Axis Left				
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->analog_stick_status(input_common::GCController::STICK_LEFT))) {
        si.pad[_channel].aX = A_LOW;
        goto sec_2;
    }else{
        si.pad[_channel].aX = A_NEUTRAL;
    }

    // Analog Stick X Axis Right
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->analog_stick_status(input_common::GCController::STICK_RIGHT))) {
        si.pad[_channel].aX = A_HIGH;
    }else{
        si.pad[_channel].aX = A_NEUTRAL;
    }

sec_2:
    // C Stick Y Axis Up
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->c_stick_status(input_common::GCController::STICK_UP))) {
        si.pad[_channel].cY = A_HIGH;
        goto sec_3;
    }else{
        si.pad[_channel].cY = A_NEUTRAL;
    }

    // C Stick Y Axis Down
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->c_stick_status(input_common::GCController::STICK_DOWN))) {
        si.pad[_channel].cY = A_LOW;
    }else{
        si.pad[_channel].cY = A_NEUTRAL;
    }

sec_3:
    // C Stick X Axis Left
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->c_stick_status(input_common::GCController::STICK_LEFT))) {
        si.pad[_channel].cX = A_LOW;
        goto sec_4;
    }else{
        si.pad[_channel].cX = A_NEUTRAL;
    }

    // C Stick X Axis Right
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->c_stick_status(input_common::GCController::STICK_RIGHT))) {
        si.pad[_channel].cX = A_HIGH;
    }else{
        si.pad[_channel].cX = A_NEUTRAL;
    }

sec_4:
    // Start button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->start_status())) {
        si.pad[_channel].buttons |= B_START;
    } else {
        si.pad[_channel].buttons &= ~B_START;
    }

    // B button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->b_status())) {
        si.pad[_channel].buttons |= B_B;
    } else {
        si.pad[_channel].buttons &= ~B_B;
    }

    // A button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->a_status())) {
        si.pad[_channel].buttons |= B_A;
    } else {
        si.pad[_channel].buttons &= ~B_A;
    }

    // X button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->x_status())) {
        si.pad[_channel].buttons |= B_X;
    } else {
        si.pad[_channel].buttons &= ~B_X;
    }

    // Y button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->y_status())){
        si.pad[_channel].buttons |= B_Y;
    } else {
        si.pad[_channel].buttons &= ~B_Y;
    }

    // Z button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->z_status())) {
        si.pad[_channel].buttons |= B_Z;
    } else {
        si.pad[_channel].buttons &= ~B_Z;
    }

    // L button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->l_status())) {
        si.pad[_channel].buttons |= B_L;
    } else {
        si.pad[_channel].buttons &= ~B_L;
    }

    // R button
    if (IS_GCBUTTON_PRESSED(input_common::g_controller_state[_channel]->r_status())) {
        si.pad[_channel].buttons |= B_R;
    } else {
        si.pad[_channel].buttons &= ~B_R;
    }
    /*
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
    }*/
}

// Desc: Poll for button presses
//

void SI_Poll(void)
{
    // Channel 0:

    if(REGSI32(SI_POLL) & SI_POLL_ENB0)
    {
        if(common::g_config->controller_ports(0).enable)
        {
            SI_ReadKeys(0);

            REGSI32(SI_STATUS) |= SI_STAT_RDST0;
            REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINT;
        }
    }

    // Channel 1:

    if(REGSI32(SI_POLL) & SI_POLL_ENB1)
    {
        if(common::g_config->controller_ports(1).enable)
        {
            SI_ReadKeys(1);

            REGSI32(SI_STATUS) |= SI_STAT_RDST1;
            REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINT;
        }
    }

    // Channel 2:

    if(REGSI32(SI_POLL) & SI_POLL_ENB2)
    {
        if(common::g_config->controller_ports(2).enable)
        {
            SI_ReadKeys(2);

            REGSI32(SI_STATUS) |= SI_STAT_RDST2;
            REGSI32(SI_CTRLSTATUS) |= SI_CSTAT_RDSTINT;
        }
    }

    // Channel 3:

    if(REGSI32(SI_POLL) & SI_POLL_ENB3)
    {
        if(common::g_config->controller_ports(3).enable)
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
    LOG_NOTICE(TSI, "initialized ok");

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
}
