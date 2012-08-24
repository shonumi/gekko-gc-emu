////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_memory.h
// DESC:		Recompiler Memory Header
// CREATED:		Feb. 26, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#ifndef _GEKKO_REC_MEMORY_H
#define _GEKKO_REC_MEMORY_H

#define RECOMPILEMEM_SIZE (1024*1024*128)

typedef struct RecompileMemBlock
{
	u32					Size;			//if top bit is set then memory block is in use
	RecompileMemBlock	*PtrNext;		//linked list with memory blocks in order
	RecompileMemBlock	*PtrPrev;
	RecompileMemBlock	*SizeNext;		//linked list with memory blocks in size order
	RecompileMemBlock	*SizePrev;
	RecompileMemBlock	*SizeSameNext;	//linked list with memory blocks of the same size
	RecompileMemBlock	*SizeSamePrev;
} RecompileMemBlock;

static RecompileMemBlock		*FreeMemSize;
static RecompileMemBlock		*FreeMemPtr;
static RecompileMemBlock		*StartMem;

static void	*RecompileAlloc(u32 Size);
static GekkoF	RecompileFree(void *FreePtr);
static GekkoF	RecompilePartialFree(void *AllocPtr, u32 EndSize);
static GekkoF	InitRecompileMemory();
static GekkoF	ReleaseRecompileMemory();
static GekkoF	CheckMemory();
static GekkoF	DumpMemoryLayout();
static GekkoF DumpMemoryDLL();
//static GekkoF	MemSet4Byte(void *StartPtr, u32 NewVal, u32 Count);

#endif
