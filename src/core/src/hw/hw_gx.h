// hw_gx.h
// (c) 2005,2008 Gekko Team / Wiimu Project

#ifndef HW_GX_H_
#define HW_GX_H_

void GX_Open(void);
void GX_Update(void);

void GX_BPLoad(u8 _addr, u32 _value);
void GX_CPLoad(u8 _addr, u32 _value);
void GX_XFLoad(u16 _length, u16 _addr, u32 _regs[64]);
void GX_XFLoadIndexed(u8 _n, u16 _index, u8 _length, u16 _addr);

u8 EMU_FASTCALL GX_Fifo_Read8(u32 addr);
u16 EMU_FASTCALL GX_Fifo_Read16(u32 addr);
u32 EMU_FASTCALL GX_Fifo_Read32(u32 addr);

void EMU_FASTCALL GX_Fifo_Write8(u32 addr, u32 data);
void EMU_FASTCALL GX_Fifo_Write16(u32 addr, u32 data);
void EMU_FASTCALL GX_Fifo_Write32(u32 addr, u32 data);

#endif
