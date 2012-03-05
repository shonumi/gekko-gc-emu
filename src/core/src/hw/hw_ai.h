// hw_ai.h
// (c) 2005,2006 Gekko Team

#ifndef _HW_AI_H_
#define _HW_AI_H_

////////////////////////////////////////////////////////////

#ifndef MEM_NATIVE_LE32
# define REGAI16(X)			(*((u16 *) &AIRegisters[REG_SIZE - (X & REG_MASK) - 2]))
# define REGAI32(X)			(*((u32 *) &AIRegisters[REG_SIZE - (X & REG_MASK) - 4]))
#else
# define REGAI16(X)			(*((u16 *) &AIRegisters[X & REG_MASK]))
# define REGAI32(X)			(*((u32 *) &AIRegisters[X & REG_MASK]))
#endif

extern u8		AIRegisters[REG_SIZE];
extern s32		g_AISampleRate;

////////////////////////////////////////////////////////////

#define AI_CR              0xCC006C00	// Control Register
#define AI_VR              0xCC006C04	// Volume Register
#define AI_SCNT            0xCC006C08	// Sample Count Register
#define AI_IT              0xCC006C0C	// Interrupt Timer Register

#define AI_CR_DSR			 (1 << 6)	// DSP Sample Rate
#define AI_CR_SCRESET		 (1 << 5)	// Sample Counter Reset
#define AI_CR_AIINTVLD       (1 << 4)	// Interrupt Validation
#define AI_CR_AIINT          (1 << 3)	// Interrupt Status
#define AI_CR_AIINTMSK       (1 << 2)	// Interrupt Mask
#define AI_CR_AFREQ			 (1 << 1)	// Auxilary Frequenxy Rate
#define AI_CR_PSTAT          (1 << 0)	// Toggle Streaming Clock

////////////////////////////////////////////////////////////

void AI_Open(void);
void AI_Update(void);

////////////////////////////////////////////////////////////

u8		__fastcall	AI_Read8(u32 addr);
void	__fastcall	AI_Write8(u32 addr, u32 data);
u16		__fastcall	AI_Read16(u32 addr);
void	__fastcall	AI_Write16(u32 addr, u32 data);
u32		__fastcall	AI_Read32(u32 addr);
void	__fastcall	AI_Write32(u32 addr, u32 data);

//

#endif
