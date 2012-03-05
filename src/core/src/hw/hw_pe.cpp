// hw_pe.cpp
// (c) 2005,2006 Gekko Team

#include "common.h"
#include "hw.h"
#include "hw_pe.h"
#include "hw_pi.h"

//

////////////////////////////////////////////////////////////
// PE - Pixel Interface
// The pixel interface is part of the Gamecube's video hardeware,
// the GP Fifo.  It mainly is used for handling the GXToken and
// GXDrawDone interrupts.  There are additionally some registers
// for GX drawing control.
////////////////////////////////////////////////////////////

#define PE_SR				0xCC00100A
#define PE_TOKEN			0xCC00100E

#define PE_SR_MSK_TOKEN		0x8
#define PE_SR_MSK_FINISH	0x4
#define PE_SR_INT_TOKEN		0x2
#define PE_SR_INT_FINISH	0x1

#ifndef MEM_NATIVE_LE32
# define REGPE16(X)			(*((u16 *) &PERegisters[REG_SIZE - (X & REG_MASK) - 2]))
# define REGPE32(X)			(*((u32 *) &PERegisters[REG_SIZE - (X & REG_MASK) - 4]))
#else
# define REGPE16(X)			(*((u16 *) &PERegisters[X & REG_MASK]))
# define REGPE32(X)			(*((u32 *) &PERegisters[X & REG_MASK]))
#endif

long	GX_PE_FINISH;
long	GX_PE_TOKEN;
u16		GX_PE_TOKEN_VALUE;

// PE Registers store Alpha configuration, Z configuration,
// Dest Alpha, Alpha Mode, and the PE TOKEN
u8 PERegisters[REG_SIZE];

////////////////////////////////////////////////////////////

// Desc: Read/Write from/to PE Hardware
//

u8 __fastcall PE_Read8(u32 addr)
{
	LOG_ERROR(TPE, "Undefined PE_Read8: %08X!\n", addr);
	return 0;
}

void __fastcall PE_Write8(u32 addr, u32 data)
{
	LOG_ERROR(TPE, "Undefined PE_Write8: %08X := %08X !\n", addr, data);
}

u16 __fastcall PE_Read16(u32 addr)
{
	switch(addr)
	{
	case PE_SR:
		return REGPE16(PE_SR);
	case PE_TOKEN:
		return REGPE16(PE_TOKEN);
	default:
		LOG_ERROR(TPE, "Undefined PE_Read16: %08X!\n", addr);
		return 0;
	}
}

void __fastcall PE_Write16(u32 addr, u32 data)
{
	switch(addr)
	{
	case PE_SR:
		if(data & PE_SR_INT_FINISH)
			PI_ClearInterrupt(PI_MASK_PEFINISH);
		if(data & PE_SR_INT_TOKEN)
			PI_ClearInterrupt(PI_MASK_PETOKEN);
		REGPE16(PE_SR) = (data & ~(PE_SR_INT_FINISH | PE_SR_INT_TOKEN));
		break;
	case PE_TOKEN:
		REGPE16(PE_TOKEN) = data;
		break;
	default:
		LOG_ERROR(TPE, "Undefined PE_Write16: %08X := %08X !\n", addr, data);
		return;
	}
}

u32 __fastcall PE_Read32(u32 addr)
{
	LOG_ERROR(TPE, "Undefined PE_Read32: %08X!\n", addr);
	return 0;
}

void __fastcall PE_Write32(u32 addr, u32 data)
{
	LOG_ERROR(TPE, "Undefined PE_Write32: %08X := %08X !\n", addr, data);
}

////////////////////////////////////////////////////////////

// PE Token
void PE_Token(u16 *token)
{
	//LOG_ERROR(TPE, "PE_Token()\n");
	REGPE16(PE_TOKEN) = *token;
	REGPE16(PE_SR) |= PE_SR_INT_TOKEN;

	*token = 0;

	if(REGPE16(PE_SR) & PE_SR_MSK_TOKEN)
	{
		PI_RequestInterrupt(PI_MASK_PETOKEN);
	}
}

// PE Finish
// -> PE_DRAW_DONE, finish drawing screen.
void PE_Finish()
{
	REGPE16(PE_SR) |= PE_SR_INT_FINISH;

	if(REGPE16(PE_SR) & PE_SR_MSK_FINISH)
	{
		PI_RequestInterrupt(PI_MASK_PEFINISH);
	}

// TODO: FIXME. this doesn't belong here IMO
//	if(DisplayFPS)
//		WIN_Framerate();
}

////////////////////////////////////////////////////////////

// Desc: Update PE Hardware
//

void PE_Update(void)
{
	if(GX_PE_FINISH)
	{
		GX_PE_FINISH = 0;
		PE_Finish();
	}

	if(GX_PE_TOKEN)
	{
		GX_PE_TOKEN = 0;
		PE_Token(&GX_PE_TOKEN_VALUE);
	}
}

// Desc: Initialize PE Hardware
//

void PE_Open(void)
{
	LOG_NOTICE(TPE, "initialized ok");
	memset(PERegisters, 0, sizeof(PERegisters));
}
