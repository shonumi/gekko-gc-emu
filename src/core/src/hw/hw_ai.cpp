// hw_ai.cpp
// (c) 2005,2006 Gekko Team

#include "common.h"
#include "hw.h"
#include "hw_ai.h"
#include "hw_pi.h"
#include "hw_dsp.h"

//

u8		AIRegisters[REG_SIZE];

s32		g_AISampleRate;
u32		AICRInterrupt = 0;

////////////////////////////////////////////////////////////
// AI - Audio Interface
////////////////////////////////////////////////////////////

void AI_SetSampleRate(long _rate)
{
	g_AISampleRate = _rate;
}

////////////////////////////////////////////////////////////

// Desc: Read/Write from/to AI Hardware
//

u8 __fastcall AI_Read8(u32 addr)
{
	LOG_ERROR(TAI, "Undefined AI_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall AI_Write8(u32 addr, u32 data)
{
	LOG_ERROR(TAI, "Undefined AI_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall AI_Read16(u32 addr)
{
	LOG_ERROR(TAI, "Undefined AI_Read16: %08X!\n", addr);
	return 0;
}

void __fastcall AI_Write16(u32 addr, u32 data)
{
	LOG_ERROR(TAI, "Undefined AI_Write16: %08X := %08X !\n", addr, data);
}

u32 __fastcall AI_Read32(u32 addr)
{
	switch(addr)
	{
	case AI_CR:
	case AI_SCNT:
	case AI_IT:
	case AI_VR:
		return REGAI32(addr);

	default:
		LOG_ERROR(TAI, "Undefined AI_Read32: %08X!\n", addr);
		return 0;
	}
}

void __fastcall AI_Write32(u32 addr, u32 data)
{
	switch(addr)
	{
	case AI_SCNT:
		return;

	case AI_CR:
		REGAI32(AI_CR) = data;

		if(REGAI32(AI_CR) & AI_CR_AIINT)							// Clear AI Interrupt
		{
			REGAI32(AI_CR) &= ~AI_CR_AIINT;
			PI_ClearInterrupt(PI_MASK_AI);
		}

		if(REGAI32(AI_CR) & AI_CR_SCRESET) REGAI32(AI_SCNT) = 0;	// Clear Sample Counter

		if(REGAI32(AI_CR) & AI_CR_DSR)
			AI_SetSampleRate(48000);		// Set Sample Rate
		else
			AI_SetSampleRate(32000);

		AICRInterrupt = REGAI32(AI_CR) & (AI_CR_PSTAT | AI_CR_AIINTVLD);
		return;

	case AI_IT:
	case AI_VR:
		REGAI32(addr) = data;
		return;

	default:
		LOG_ERROR(TAI, "Undefined AI_Write32: %08X := %08X !\n", addr, data);
		return;
	}
}

////////////////////////////////////////////////////////////

// Desc: Update AI Hardware
//

void AI_Update(void)
{
	// Sample counter (interrupt)

    if(!AICRInterrupt)
		return;

    if(REGAI32(AI_SCNT)++ >= REGAI32(AI_IT))
    {
		REGAI32(AI_CR) |= AI_CR_AIINT;
		if(REGAI32(AI_CR) & AI_CR_AIINTMSK)
		{
			PI_RequestInterrupt(PI_MASK_AI);
		}
    }
}

// Desc: Initialize AI Hardware
//

void AI_Open(void)
{
    LOG_NOTICE(TAI, "initialized ok");
	ZeroMemory(&AIRegisters, REG_SIZE);
}

////////////////////////////////////////////////////////////
