// hw_exi.h
// (c) 2005,2006 Gekko Team

#ifndef _HW_EXI_H_
#define _HW_EXI_H_

////////////////////////////////////////////////////////////

#define EXI_CSR0		0xCC006800
#define EXI_MAR0		0xCC006804
#define EXI_LENGTH0		0xCC006808
#define EXI_CR0			0xCC00680C
#define EXI_DATA0		0xCC006810

#define EXI_CSR1		0xCC006814
#define EXI_MAR1		0xCC006818
#define EXI_LENGTH1		0xCC00681C
#define EXI_CR1			0xCC006820
#define EXI_DATA1		0xCC006824

#define EXI_CSR2		0xCC006828
#define EXI_MAR2		0xCC00682C
#define EXI_LENGTH2		0xCC006830
#define EXI_CR2			0xCC006834
#define EXI_DATA2		0xCC006838

////////////////////////////////////////////////////////////

#define EXI_CSR_EXT					(1 << 12)
#define EXI_CSR_EXTINT				(1 << 11)
#define EXI_CSR_EXTINTMASK			(1 << 10)
#define EXI_CSR_TCINT				(1 << 3)
#define EXI_CSR_TCINTMASK			(1 << 2)
#define EXI_CSR_EXIINT				(1 << 1)
#define EXI_CSR_EXIINTMASK			(1 << 0)

#define EXI_CR_TSTART				(1 << 0)
#define EXI_CR_DMA					(1 << 1)
#define EXI_CR_RW					(3 << 2)

typedef struct t_sEXI
{
	u32 csr[3];			// Channel parameter register.
	u32 padding1;		//padding to align csrINTMask on 64bit boundary
	u32 csrINTMask[3];	//mask for interrupts for csr
	u32 mar[3];			// Channel DMA start address.
	u32 len[3];			// Channel DMA transfer length.
	u32 cr[3];			// Channel control register.
	u32 data[3];		// Channel data
	u32	FinalINTMask;
}sEXI;

extern sEXI exi;

////////////////////////////////////////////////////////////

void EXI_Open(void);
void EXI_Update(void);
void EXI_Close(void);

extern u32 MemCardInterruptSet[2];
extern u32 MemCardBusy[3];

void MemCard_Open();
void MemCard_Close();
void MemCard_Update();
u32 MemCard_InterruptSet(u32 Channel);
void MemCard_Transfer(u32 addr);

u8		EMU_FASTCALL	EXI_Read8(u32 addr);
void	EMU_FASTCALL	EXI_Write8(u32 addr, u32 data);
u16		EMU_FASTCALL	EXI_Read16(u32 addr);
void	EMU_FASTCALL	EXI_Write16(u32 addr, u32 data);
u32		EMU_FASTCALL	EXI_Read32(u32 addr);
void	EMU_FASTCALL	EXI_Write32(u32 addr, u32 data);

//

#endif
