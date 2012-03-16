// hw_si.h
// (c) 2005,2006 Gekko Team

#ifndef _HW_SI_H_
#define _HW_SI_H_

////////////////////////////////////////////////////////////

#ifndef MEM_NATIVE_LE32
# define REGSI8(X)			(*((u8 *) &SIRegisters[REG_SIZE - (X & REG_MASK) - 1]))
# define REGSI16(X)			(*((u16 *) &SIRegisters[REG_SIZE - (X & REG_MASK) - 2]))
# define REGSI32(X)			(*((u32 *) &SIRegisters[REG_SIZE - (X & REG_MASK) - 4]))
#else
# define REGSI8(X)			(*((u8 *) &SIRegisters[X & REG_MASK]))
# define REGSI16(X)			(*((u16 *) &SIRegisters[X & REG_MASK]))
# define REGSI32(X)			(*((u32 *) &SIRegisters[X & REG_MASK]))
#endif

extern u8 SIRegisters[REG_SIZE];

////////////////////////////////////////////////////////////

// Main Definitions:

#define SI_C0OUTBUF			0xCC006400
#define SI_C0INBUFH			0xCC006404
#define SI_C0INBUFL			0xCC006408
#define SI_C1OUTBUF			0xCC00640C
#define SI_C1INBUFH			0xCC006410
#define SI_C1INBUFL			0xCC006414
#define SI_C2OUTBUF			0xCC006418
#define SI_C2INBUFH			0xCC00641C
#define SI_C2INBUFL			0xCC006420
#define SI_C3OUTBUF			0xCC006424
#define SI_C3INBUFH			0xCC006428
#define SI_C3INBUFL			0xCC00642C
#define SI_POLL				0xCC006430
#define SI_CTRLSTATUS		0xCC006434
#define SI_STATUS			0xCC006438
#define SI_EXILK			0xCC00643C
#define SI_BUFFSTART		0xCC006480
#define SI_BUFFEND			0xCC006500

// Control Status Register Bits:

#define SI_STAT_WR					(1 << 31)
#define SI_STAT_WRST0				(1 << 28)
#define SI_STAT_WRST1				(1 << 20)
#define SI_STAT_WRST2				(1 << 12)
#define SI_STAT_WRST3				(1 << 4)
#define SI_STAT_RDST0				(1 << 29)
#define SI_STAT_RDST1				(1 << 21)
#define SI_STAT_RDST2				(1 << 13)
#define SI_STAT_RDST3				(1 << 5)

#define SI_CSTAT_TSTART				(1 << 0)
#define SI_CSTAT_RDSTINTMSK			(1 << 27)
#define SI_CSTAT_RDSTINT			(1 << 28)
#define SI_CSTAT_TCINTMSK			(1 << 30)
#define SI_CSTAT_TCINT				(1 << 31)

#define SI_POLL_ENB0				(1 << 7)
#define SI_POLL_ENB1				(1 << 6)
#define SI_POLL_ENB2				(1 << 5)
#define SI_POLL_ENB3				(1 << 4)

// Desc: Joypad Button Control:

#define B_START				0x10000000
							
#define B_Y					0x08000000
#define B_X					0x04000000
#define B_B					0x02000000
#define B_A					0x01000000
							
#define B_L					0x00400000
#define B_R					0x00200000
#define B_Z					0x00100000
							
#define B_UP				0x00080000
#define B_DN				0x00040000
#define B_LT				0x00020000
#define B_RT				0x00010000
							
#define A_LOW				0x20
#define A_NEUTRAL			0x80
#define A_HIGH				0xE0

#define T_PRESSED			0xFF
#define T_RELEASED			0x00

////////////////////////////////////////////////////////////

typedef struct t_sSI
{
	bool pad_connected[4];	// Joypad connected to system?

	u32	shadow[4];			// Store "shadow" output registers.

	struct sPad				// Unused, should use PADState with Dolwin specs
	{
		u32 buttons;		// Button states.

		u8 aX, aY;			// Analog stick states.
		u8 cX, cY;			// C-Stick states.
		u8 aL, aR;			// L & R button states.

	}pad[4];				// Supports 4 Joypads
}sSI;

extern sSI si;

////////////////////////////////////////////////////////////

inline static u32 PadGetWord1(u32 nPad)
{
	return (si.pad[nPad].buttons & 0x1FFF0000) | (si.pad[nPad].aX << 8) | (si.pad[nPad].aY);
}

////////////////////////////////////////////////////////////

void SI_Open(void);
void SI_Poll(void);
void SI_ProcessCommand(void);

u8		EMU_FASTCALL	SI_Read8(u32 addr);
void	EMU_FASTCALL	SI_Write8(u32 addr, u32 data);
u16		EMU_FASTCALL	SI_Read16(u32 addr);
void	EMU_FASTCALL	SI_Write16(u32 addr, u32 data);
u32		EMU_FASTCALL	SI_Read32(u32 addr);
void	EMU_FASTCALL	SI_Write32(u32 addr, u32 data);

//

#endif
