// hw_mi.h
// (c) 2005,2010 Gekko Team

#ifndef _HW_MI_H_
#define _HW_MI_H_
////////////////////////////////////////////////////////////

#ifndef MEM_NATIVE_LE32
# define REGMI8(X)			(*((u8 *) &MIRegisters[REG_SIZE - (X & REG_MASK) - 1]))
# define REGMI16(X)			(*((u16 *) &MIRegisters[REG_SIZE - (X & REG_MASK) - 2]))
# define REGMI32(X)			(*((u32 *) &MIRegisters[REG_SIZE - (X & REG_MASK) - 4]))
#else
# define REGMI8(X)			(*((u8 *) &MIRegisters[X & REG_MASK]))
# define REGMI16(X)			(*((u16 *) &MIRegisters[X & REG_MASK]))
# define REGMI32(X)			(*((u32 *) &MIRegisters[X & REG_MASK]))
#endif

extern u8 MIRegisters[REG_SIZE];

////////////////////////////////////////////////////////////

#define MI_MEMREGION0_LO	0xCC004000
#define MI_MEMREGION0_HI	0xCC004002
#define MI_MEMREGION1_LO	0xCC004004
#define MI_MEMREGION1_HI	0xCC004006
#define MI_MEMREGION2_LO	0xCC004008
#define MI_MEMREGION2_HI	0xCC00400A
#define MI_MEMREGION3_LO	0xCC00400C
#define MI_MEMREGION3_HI	0xCC00400E
#define MI_CONTROL			0xCC004010

#define	MI_CHANNEL0			(11 < 0)
#define	MI_CHANNEL1			(11 < 2)
#define	MI_CHANNEL2			(11 < 4)
#define	MI_CHANNEL3			(11 < 6)

void MI_Open(void);
void MI_Close(void);

u8		EMU_FASTCALL	MI_Read8(u32 addr);
void	EMU_FASTCALL	MI_Write8(u32 addr, u32 data);
u16		EMU_FASTCALL	MI_Read16(u32 addr);
void	EMU_FASTCALL  MI_Write16(u32 addr, u32 data);
u32		EMU_FASTCALL	MI_Read32(u32 addr);
void	EMU_FASTCALL	MI_Write32(u32 addr, u32 data);

#endif
