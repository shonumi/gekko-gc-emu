// hw_pe.h
// (c) 2005,2006 Gekko Team

#ifndef _HW_PE_H_
#define _HW_PE_H_

void PE_Open(void);
void PE_Update(void);

void PE_Token(u16 *token);
void PE_Finish();

extern u32 GX_PE_FINISH;
extern u32 GX_PE_TOKEN;
extern u16 GX_PE_TOKEN_VALUE;

u8		EMU_FASTCALL	PE_Read8(u32 addr);
void	EMU_FASTCALL	PE_Write8(u32 addr, u32 data);
u16		EMU_FASTCALL	PE_Read16(u32 addr);
void	EMU_FASTCALL  PE_Write16(u32 addr, u32 data);
u32		EMU_FASTCALL	PE_Read32(u32 addr);
void	EMU_FASTCALL	PE_Write32(u32 addr, u32 data);

#endif
