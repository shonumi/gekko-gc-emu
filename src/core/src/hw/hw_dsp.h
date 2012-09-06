// hw_dsp.h

#ifndef _HW_DSP_H_
#define _HW_DSP_H_

////////////////////////////////////////////////////////////

#ifndef MEM_NATIVE_LE32
# define REGDSP8(X)			(*((u8 *) &DSPRegisters[REG_SIZE - (X & REG_MASK) - 1]))
# define REGDSP16(X)			(*((u16 *) &DSPRegisters[REG_SIZE - (X & REG_MASK) - 2]))
# define REGDSP32(X)			(*((u32 *) &DSPRegisters[REG_SIZE - (X & REG_MASK) - 4]))
#else
# define REGDSP8(X)			(*((u8 *) &DSPRegisters[X & REG_MASK]))
# define REGDSP16(X)			(*((u16 *) &DSPRegisters[X & REG_MASK]))
# define REGDSP32(X)			(*((u32 *) &DSPRegisters[X & REG_MASK]))
#endif

extern u8 DSPRegisters[REG_SIZE];

////////////////////////////////////////////////////////////

#define DSP_MAILBOX				0xCC005000
#define DSP_CPU_MAILBOX			0xCC005004
#define DSP_CSR					0xCC00500A

#define DSP_AR_INFO				0xCC005012
#define DSP_AR_MODE				0xCC005016
#define DSP_AR_REFRESH			0xCC00501A
#define DSP_AR_DMA_MMADDR		0xCC005020
#define DSP_AR_DMA_ARADDR		0xCC005024
#define DSP_AR_DMA_CNT			0xCC005028

#define DSP_DMA_ADDR			0xCC005030
#define DSP_DMA_LEN				0xCC005036
#define DSP_DMA_CNT				0xCC00503A

////////////////////////////////////////////////////////////

#define DSP_CSR_RES				(1 << 0)
#define DSP_CSR_PIINT			(1 << 1)
#define DSP_CSR_HALT			(1 << 2)
#define DSP_CSR_AIDINT			(1 << 3)
#define DSP_CSR_AIDINTMSK		(1 << 4)
#define DSP_CSR_ARINT			(1 << 5)
#define DSP_CSR_ARINTMSK		(1 << 6)
#define DSP_CSR_DSPINT			(1 << 7)
#define DSP_CSR_DSPINTMSK		(1 << 8)
#define DSP_CSR_DMAINT			(1 << 9)

#define DSP_DMALEN_ENB			(1 << 15)

////////////////////////////////////////////////////////////

#define ARAM_SIZE				(16 * 1024 * 1024)						// 16MB
#define ARAM_DMA_TYPE			(REGDSP32(DSP_AR_DMA_CNT) >> 31)
#define ARAM_DMA_SIZE			(REGDSP32(DSP_AR_DMA_CNT) & ~0x80000000)

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

typedef struct t_sDSP
{
	bool cntv[2];		// Count Register - validation
} sDSP;

extern sDSP dsp;
extern u32 dspCSRDSPInt;

////////////////////////////////////////////////////////////

void DSP_Open(void);
void DSP_Update(void);

u8		EMU_FASTCALL	DSP_Read8(u32 addr);
void	EMU_FASTCALL	DSP_Write8(u32 addr, u32 data);
u16		EMU_FASTCALL	DSP_Read16(u32 addr);
void	EMU_FASTCALL	DSP_Write16(u32 addr, u32 data);
u32		EMU_FASTCALL	DSP_Read32(u32 addr);
void	EMU_FASTCALL	DSP_Write32(u32 addr, u32 data);

//

#endif
