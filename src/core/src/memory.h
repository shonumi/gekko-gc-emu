// memory.h
// (c) 2005,2006 Gekko Team

#ifndef _MEMORY_H_
#define _MEMORY_H_

////////////////////////////////////////////////////////////

#define RAM_24MB					(1024*1024*24)
#define RAM_SIZE					(1024*1024*32)
#define RAM2_SIZE					(1024*1024*64)
#define RAM_MASK					(RAM_SIZE - 1)
#define RAM2_MASK					(RAM2_SIZE - 1)

#define L2_SIZE						0x4000
#define L2_MASK						(L2_SIZE - 1)

#define REG_SIZE					0x100
#define REG_MASK					0xFF

#define MEM8(X)						*MEMPTR8(X)
#define MEM16(X)					*MEMPTR16(X)
#define MEM32(X)					*MEMPTR32(X)
#define MEM64(X)					*MEMPTR64(X)

#define MEMPTR8(X)					(u8*)&Mem_RAM[X & RAM_MASK]
#define MEMPTR16(X)					(u16*)&Mem_RAM[X & RAM_MASK]
#define MEMPTR32(X)					(u32*)&Mem_RAM[X & RAM_MASK]
#define MEMPTR64(X)					(u64*)&Mem_RAM[X & RAM_MASK]

#define RMEM16(X)					BSWAP16(*(u16*)&Mem_RAM[X & RAM_MASK])
#define RMEM32(X)					BSWAP32(*(u32*)&Mem_RAM[X & RAM_MASK])
#define RMEM64(X)					BSWAP64(*(u64*)&Mem_RAM[X & RAM_MASK])

#define MEM28(X)					*MEM2PTR8(X)
#define MEM216(X)					*MEM2PTR16(X)
#define MEM232(X)					*MEM2PTR32(X)
#define MEM264(X)					*MEM2PTR64(X)

#define MEM2PTR8(X)					(u8*)&Mem_RAM2[X & RAM2_MASK]
#define MEM2PTR16(X)				(u16*)&Mem_RAM2[X & RAM2_MASK]
#define MEM2PTR32(X)				(u32*)&Mem_RAM2[X & RAM2_MASK]
#define MEM2PTR64(X)				(u64*)&Mem_RAM2[X & RAM2_MASK]

#define RMEM216(X)					BSWAP16(*(u16*)&Mem_RAM2[X & RAM2_MASK])
#define RMEM232(X)					BSWAP32(*(u32*)&Mem_RAM2[X & RAM2_MASK])
#define RMEM264(X)					BSWAP64(*(u64*)&Mem_RAM2[X & RAM2_MASK])

#define DATA8(data)					(data)
#define DATA16(data)				BSWAP16(data)
#define DATA32(data)				BSWAP32(data)
#define DATA64(data)				BSWAP64(data)

extern u8 Mem_L2[L2_SIZE];
//extern u8 Mem_RAM[RAM_SIZE];
//extern u8 *Mem_RAM;
//extern u8 Mem_RAM2[RAM2_SIZE];
extern u8 Mem_RAM[RAM2_SIZE];
		
////////////////////////////////////////////////////////////

void Memory_Open(void);
void Memory_Close(void);

//

u8 EMU_FASTCALL Memory_Read8(u32 addr);
u16 EMU_FASTCALL Memory_Read16(u32 addr);
u32 EMU_FASTCALL Memory_Read32(u32 addr);
u64 EMU_FASTCALL Memory_Read64(u32 addr);

void EMU_FASTCALL Memory_Write8(u32 addr, u32 data);
void EMU_FASTCALL Memory_Write16(u32 addr, u32 data);
void EMU_FASTCALL Memory_Write32(u32 addr, u32 data);
void EMU_FASTCALL Memory_Write64(u32 addr, u64 data);

////////////////////////////////////////////////////////////

//

#endif
