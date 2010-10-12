// flipper.h
// (c) 2005,2006 Gekko Team

#ifndef _FLIPPER_H_
#define _FLIPPER_H_

////////////////////////////////////////////////////////////

#define CP_Regs						0x00
#define PE_Regs						0x10
#define VI_Regs						0x20
#define PI_Regs						0x30
#define MI_Regs						0x40
#define DSP_Regs					0x50
#define DI_Regs						0x60
#define SI_Regs						0x64
#define EXI_Regs					0x68
#define AI_Regs						0x6C
#define GX_Fifo						0x80

#define REG_SIZE					0x100
#define REG_MASK					0xFF

#define	HARDWARE_ADDR				((addr & 0xFF00)>> 8)
#define	REGISTER_ADDR				(addr & 0xFF)

#define BUS_CLOCK_SPEED				0x09A7EC80 // 162 MHz
#define GEKKO_CLOCK_SPEED			0x1Cf7C580 // 486 MHz
#define CONSOLE_TYPE_RETAIL1		0x00000001 // Console - Retail Version
#define CONSOLE_TYPE_HW2_PB			0x00000002 // Console - HW2 Production Board
#define CONSOLE_TYPE_LATEST_PB		0x00000003 // Console - Latest Production Board
#define ARENA_LO					0x80403100 // Actually can vary by game...
#define ARENA_HI					0x81600000 // ...
#define SIZE_OF_RAM					0x01800000 // 24Mb
#define MAGIC_NORMAL				0x0D15EA5E
#define MAGIC_JTAG					0xE5207C22

#define DOLPHIN_MAGIC				0x80000020
#define DOLPHIN_VERSION				0x80000024
#define DOLPHIN_RAM_SIZE			0x80000028
#define DOLPHIN_CONSOLE_TYPE		0x8000002C
#define DOLPHIN_ARENALO				0x80000030
#define DOLPHIN_ARENAHI				0x80000034
#define DOLPHIN_FST					0x80000038
#define DOLPHIN_FST_LENGTH			0x8000003C
#define DOLPHIN_DEBUG				0x80000040
#define DOLPHIN_SIM_RAM_SIZE		0x800000F0
#define DOLPHIN_GEKKOBUS			0x800000F8
#define DOLPHIN_GEKKOCLOCK			0x800000FC
#define DOLPHIN_CONTEXT_PHYSICAL	0x800000C0
#define DOLPHIN_CONTEXT_LOGICAL		0x800000D4

////////////////////////////////////////////////////////////

static u32 GEX_Reset_Handler[] =
{
	0x38600000,		// li  GPR(3), 0
	0x3d20cc00,		// lis GPR(9), 0xcc00
	0x90693024,		// stw GPR(3), 0x3024 (GPR(9)) -> will cause reset
};

static u32 GEX_Decrementer_Handler[] =
{
	0x3bff0001,		// addi r31, r31, 1
	0x4c000064,		// rfi
};

static u32 GEX_Empty_Handler[] =
{
	0x3d20cc00,		// lis GPR(9), 0xcc00
	0x61293000,		// ori GPR(9), GPR(9), 0x3000
	0x80090000,		// lwz GPR(0), 0 (GPR(9))
	0x4c000064,		// rfi
};

////////////////////////////////////////////////////////////

void				Flipper_Open(void);
void				Flipper_Close(void);
u32					Flipper_Update(void);

u32		__fastcall	Flipper_Read32(u32 addr);
u16		__fastcall	Flipper_Read16(u32 addr);
u8		__fastcall	Flipper_Read8(u32 addr);

void	__fastcall	Flipper_Write32(u32 addr, u32 data);
void	__fastcall	Flipper_Write16(u32 addr, u32 data);
void	__fastcall	Flipper_Write8(u32 addr, u32 data);

////////////////////////////////////////////////////////////

#endif