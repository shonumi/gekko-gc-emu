////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_memory.cpp
// DESC:		Recompiler Memory Handler
// CREATED:		Feb. 26, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "../cpu_core_regs.h"

GekkoCPURecompiler::RecompileMemBlock *GekkoCPURecompiler::FreeMemSize;
GekkoCPURecompiler::RecompileMemBlock *GekkoCPURecompiler::FreeMemPtr;
GekkoCPURecompiler::RecompileMemBlock *GekkoCPURecompiler::StartMem;

void *GekkoCPURecompiler::RecompileAlloc(u32 Size)
{
	//find the smallest block to alloc
	RecompileMemBlock	*CurBlock;
	RecompileMemBlock	*PrevBlock;
	RecompileMemBlock	*NewBlock;
	void				*RetPtr;

	RetPtr = 0;
	CurBlock = FreeMemSize;
	PrevBlock = 0;

	//store the memory block at the beginning of the allocation
	Size += sizeof(RecompileMemBlock);

	//multiples of 4 for size
	if(Size & 3)
		Size = (Size & ~3) + 4;

	while(CurBlock)
	{
		if(CurBlock->Size < Size)
			break;

		//keep going until we get to the block that is too small or hit the end
		PrevBlock = CurBlock;
		CurBlock = CurBlock->SizeNext;
	};

	if(PrevBlock)
	{
		//if we have a block of the same size, use it instead to avoid update to the tree
		if(PrevBlock->SizeSameNext)
		{
			CurBlock = PrevBlock->SizeSameNext;
			PrevBlock->SizeSameNext = CurBlock->SizeSameNext;
			if(CurBlock->SizeSameNext)
				CurBlock->SizeSameNext->SizeSamePrev = PrevBlock;
			PrevBlock = CurBlock;
		}
		else
		{
			//remove it from the tree
			if(PrevBlock->SizePrev)
				PrevBlock->SizePrev->SizeNext = CurBlock;
			else
				FreeMemSize = CurBlock;

			if(CurBlock)
				CurBlock->SizePrev = PrevBlock->SizePrev;
		}

		//if we have a Previous Block then it must be the right size so use it
		//offset our pointer so size is not lost
		RetPtr = (void *)((u32)(PrevBlock) + sizeof(RecompileMemBlock));

		//if the previous block is large enough for another memory block then split it
		//and add it
		if(PrevBlock->Size > (Size + sizeof(RecompileMemBlock)))
		{
			NewBlock = (RecompileMemBlock *)((u32)PrevBlock + Size);
			NewBlock->Size = PrevBlock->Size - Size;
			NewBlock->PtrNext = PrevBlock->PtrNext;
			NewBlock->PtrPrev = PrevBlock;
			NewBlock->SizeSameNext = 0;
			NewBlock->SizeSamePrev = 0;
			NewBlock->SizePrev = 0;
			NewBlock->SizeNext = 0;

			PrevBlock->Size = Size | 0x80000000;
			PrevBlock->PtrNext = NewBlock;
			if(NewBlock->PtrNext)
				NewBlock->PtrNext->PtrPrev = NewBlock;

			//now add in the new block we found
			CurBlock = FreeMemSize;
			PrevBlock = 0;
			while(CurBlock)
			{
				if(CurBlock->Size <= NewBlock->Size)
					break;

				PrevBlock = CurBlock;
				CurBlock = CurBlock->SizeNext;
			};

			//if we have curblock then newblock is larger than current or equal
			if(CurBlock)
			{
				//if size matches then add to the list
				if(CurBlock->Size == NewBlock->Size)
				{
					NewBlock->SizePrev = 0;
					NewBlock->SizeNext = 0;
					NewBlock->SizeSameNext = CurBlock->SizeSameNext;
					if(CurBlock->SizeSameNext)
						CurBlock->SizeSameNext->SizeSamePrev = NewBlock;
					NewBlock->SizeSamePrev = CurBlock;
					CurBlock->SizeSameNext = NewBlock;
				}
				else
				{
					//insert between the blocks
					NewBlock->SizePrev = PrevBlock;
					NewBlock->SizeNext = CurBlock;
					if(PrevBlock)
						PrevBlock->SizeNext = NewBlock;
					else
						FreeMemSize = NewBlock;
					CurBlock->SizePrev = NewBlock;
				}
			}
			else
			{
				//must be smaller, add to the end
				NewBlock->SizeNext = 0;
				NewBlock->SizeSameNext = 0;
				NewBlock->SizeSamePrev = 0;
				NewBlock->SizePrev = PrevBlock;
				if(PrevBlock)
					PrevBlock->SizeNext = NewBlock;
				else
					FreeMemSize = NewBlock;
			}
		}
		else
		{
			//can not adjust the size of the previous block, it is removed from
			//the tree, only need to flag the size as being in use
			PrevBlock->Size |= 0x80000000;
		}
	}

	//clear the memory
//#pragma todo(asm version of memset for speed needed)
//	if(RetPtr)
//		memset(RetPtr, 0, Size - sizeof(RecompileMemBlock));

//		CheckMemory();

	//return the pointer for anything found
	return RetPtr;
}

void GekkoCPURecompiler::RecompileFree(void *FreePtr)
{
	RecompileMemBlock	*CurBlock;
	RecompileMemBlock	*PrevBlock;
	RecompileMemBlock	*NewBlock;
	int					MergeFlag = 0;

	//take a pointer being free'd and return it back to memory

	//set the block back to allow for size info
	NewBlock = (RecompileMemBlock *)((u32)FreePtr - sizeof(RecompileMemBlock));
	NewBlock->SizeNext = 0;
	NewBlock->SizePrev = 0;
	NewBlock->SizeSameNext = 0;
	NewBlock->SizeSamePrev = 0;
	NewBlock->Size &= ~0x80000000;

	//if we have a previous block, see if we can combine
	if(NewBlock->PtrPrev && !(NewBlock->PtrPrev->Size & 0x80000000))
	{
		//combine with the previous
		NewBlock->PtrPrev->Size += NewBlock->Size;
		NewBlock->PtrPrev->PtrNext = NewBlock->PtrNext;
		
		if(NewBlock->PtrNext)
			NewBlock->PtrNext->PtrPrev = NewBlock->PtrPrev;

		NewBlock = NewBlock->PtrPrev;

		//update the size tree

		//if a previous, update it
		if(NewBlock->SizePrev)
		{
			//if we have a block of the same size then point to it
			if(NewBlock->SizeSameNext)
			{
				NewBlock->SizePrev->SizeNext = NewBlock->SizeSameNext;
				NewBlock->SizeSameNext->SizePrev = NewBlock->SizePrev;
				NewBlock->SizeSameNext->SizeSamePrev = 0;
				NewBlock->SizeSameNext->SizeNext = NewBlock->SizeNext;
				if(NewBlock->SizeNext)
					NewBlock->SizeNext->SizePrev = NewBlock->SizeSameNext;
			}
			else
			{
				NewBlock->SizePrev->SizeNext = NewBlock->SizeNext;
				if(NewBlock->SizeNext)
					NewBlock->SizeNext->SizePrev = NewBlock->SizePrev;
			}
		}
		else
		{
			//if we have a previous then we are part of a sub chain
			if(NewBlock->SizeSamePrev)
			{
				NewBlock->SizeSamePrev->SizeSameNext = NewBlock->SizeSameNext;
				if(NewBlock->SizeSameNext)
					NewBlock->SizeSameNext->SizeSamePrev = NewBlock->SizeSamePrev;
			}
			else if(NewBlock->SizeSameNext)
			{
				//no previous but we do have a block of the same size
				//this indicates we are at the top of the main size tree
				//put the same size in our place
				FreeMemSize = NewBlock->SizeSameNext;
				FreeMemSize->SizeSamePrev = 0;
				FreeMemSize->SizePrev = 0;
				FreeMemSize->SizeNext = NewBlock->SizeNext;
				if(FreeMemSize->SizeNext)
					FreeMemSize->SizeNext->SizePrev = FreeMemSize;
			}
			else
			{
				//no other blocks to fall on, fall to the next block in the tree
				FreeMemSize = NewBlock->SizeNext;
				if(FreeMemSize)
					FreeMemSize->SizePrev = 0;
			}
		}
	}

	//see if we can combine with the block after us
	if(NewBlock->PtrNext && !(NewBlock->PtrNext->Size & 0x80000000))
	{
		CurBlock = NewBlock->PtrNext;
		NewBlock->Size += CurBlock->Size;

		NewBlock->PtrNext = CurBlock->PtrNext;
		if(CurBlock->PtrNext)
			CurBlock->PtrNext->PtrPrev = NewBlock;

		//update the size tree

		//if a previous, update it
		if(CurBlock->SizePrev)
		{
			//if we have a block of the same size then point to it
			if(CurBlock->SizeSameNext)
			{
				CurBlock->SizePrev->SizeNext = CurBlock->SizeSameNext;
				CurBlock->SizeSameNext->SizePrev = CurBlock->SizePrev;
				CurBlock->SizeSameNext->SizeSamePrev = 0;
				CurBlock->SizeSameNext->SizeNext = CurBlock->SizeNext;
				if(CurBlock->SizeNext)
					CurBlock->SizeNext->SizePrev = CurBlock->SizeSameNext;
			}
			else
			{
				CurBlock->SizePrev->SizeNext = CurBlock->SizeNext;
				if(CurBlock->SizeNext)
					CurBlock->SizeNext->SizePrev = CurBlock->SizePrev;
			}
		}
		else
		{
			//if we have a previous then we are part of a sub chain
			if(CurBlock->SizeSamePrev)
			{
				CurBlock->SizeSamePrev->SizeSameNext = CurBlock->SizeSameNext;
				if(CurBlock->SizeSameNext)
					CurBlock->SizeSameNext->SizeSamePrev = CurBlock->SizeSamePrev;
			}
			else if(CurBlock->SizeSameNext)
			{
				//no previous but we do have a block of the same size
				//this indicates we are at the top of the main size tree
				//put the same size in our place
				FreeMemSize = CurBlock->SizeSameNext;
				FreeMemSize->SizeSamePrev = 0;
				FreeMemSize->SizePrev = 0;
				FreeMemSize->SizeNext = CurBlock->SizeNext;
				if(FreeMemSize->SizeNext)
					FreeMemSize->SizeNext->SizePrev = FreeMemSize;
			}
			else
			{
				//no other blocks to fall on, fall to the next block in the tree
				FreeMemSize = CurBlock->SizeNext;
				if(FreeMemSize)
					FreeMemSize->SizePrev = 0;
			}
		}
	}

	//now add in the new block we found
	CurBlock = FreeMemSize;
	PrevBlock = 0;
	while(CurBlock)
	{
		if(CurBlock->Size <= NewBlock->Size)
			break;

		PrevBlock = CurBlock;
		CurBlock = CurBlock->SizeNext;
	};

	//if we have curblock then newblock is larger than current or equal
	if(CurBlock)
	{
		//if size matches then add to the list
		if(CurBlock->Size == NewBlock->Size)
		{
			NewBlock->SizePrev = 0;
			NewBlock->SizeNext = 0;
			NewBlock->SizeSameNext = CurBlock->SizeSameNext;
			if(CurBlock->SizeSameNext)
				CurBlock->SizeSameNext->SizeSamePrev = NewBlock;
			NewBlock->SizeSamePrev = CurBlock;
			CurBlock->SizeSameNext = NewBlock;
		}
		else
		{
			//insert between the blocks
			NewBlock->SizePrev = PrevBlock;
			NewBlock->SizeNext = CurBlock;
			NewBlock->SizeSameNext = 0;
			NewBlock->SizeSamePrev = 0;
			if(PrevBlock)
				PrevBlock->SizeNext = NewBlock;
			else
				FreeMemSize = NewBlock;
			CurBlock->SizePrev = NewBlock;
		}
	}
	else
	{
		//must be smaller, add to the end
		NewBlock->SizeNext = 0;
		NewBlock->SizeSameNext = 0;
		NewBlock->SizeSamePrev = 0;
		NewBlock->SizePrev = PrevBlock;
		if(PrevBlock)
			PrevBlock->SizeNext = NewBlock;
		else
			FreeMemSize = NewBlock;
	}

//		CheckMemory();
}

void GekkoCPURecompiler::RecompilePartialFree(void *AllocPtr, u32 EndSize)
{
	RecompileMemBlock	*CurBlock;
	RecompileMemBlock	*NewBlock;

	//take a pointer being free'd and return part of it back to memory

	if(!EndSize)
	{
		RecompileFree(AllocPtr);
		return;
	}

	if(EndSize & 3)
		EndSize = (EndSize & ~3) + 4;

	//set the block back to allow for size info
	CurBlock = (RecompileMemBlock *)((u32)AllocPtr - sizeof(RecompileMemBlock));
	if((s32)((CurBlock->Size & 0x7FFFFFFF) - EndSize - sizeof(RecompileMemBlock)) < (s32)sizeof(RecompileMemBlock))
		return;

	//setup the new block in memory
	NewBlock = (RecompileMemBlock *)((u32)AllocPtr + EndSize);
	NewBlock->SizeNext = 0;
	NewBlock->SizePrev = 0;
	NewBlock->SizeSameNext = 0;
	NewBlock->SizeSamePrev = 0;

	//adjust the pointers
	NewBlock->PtrNext = CurBlock->PtrNext;
	if(NewBlock->PtrNext)
		NewBlock->PtrNext->PtrPrev = NewBlock;
	NewBlock->PtrPrev = CurBlock;
	CurBlock->PtrNext = NewBlock;

	//adjust our sizes
	EndSize += sizeof(RecompileMemBlock);
	NewBlock->Size = (CurBlock->Size & ~0x80000000) - EndSize;
	CurBlock->Size = EndSize | 0x80000000;

	//free the new block
	RecompileFree((void *)((u32)NewBlock + sizeof(RecompileMemBlock)));
}

GekkoF GekkoCPURecompiler::InitRecompileMemory()
{
	//allocate 128mb of memory
	FreeMemSize = (RecompileMemBlock *)VirtualAlloc(NULL, RECOMPILEMEM_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	FreeMemSize->Size = 1024*1024*128;
	FreeMemSize->SizeNext = 0;
	FreeMemSize->PtrNext = 0;
	FreeMemSize->SizePrev = 0;
	FreeMemSize->PtrPrev = 0;
	FreeMemSize->SizeSameNext = 0;
	FreeMemSize->SizeSamePrev = 0;
	FreeMemPtr = FreeMemSize;
	StartMem = FreeMemSize;

	LOG_NOTICE(TPOWERPC, "DynaRec Memory Location: 0x%08X\n", StartMem);

#ifdef USE_INLINE_ASM_X86
	_asm
	{
		jmp InitEnd
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
			nop
		InitEnd:
	};
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoF GekkoCPURecompiler::ReleaseRecompileMemory()
{
	//HANDLE	MemDump;
	DWORD	BytesOut;
	char	DumpFile[100];

//	DumpMemoryDLL();
/*
	sprintf(DumpFile, "memdump-%08X.bin", (u32)StartMem);
	printf("Dumping recompile memory to %s...", DumpFile);
	MemDump = CreateFile(DumpFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(MemDump != INVALID_HANDLE_VALUE)
	{
		WriteFile(MemDump, StartMem, 1024*1024*128, &BytesOut, 0);
		CloseHandle(MemDump);
	}
	else
		printf("Error!");
	printf("\n");
*/
	VirtualFree(StartMem, 0, MEM_RELEASE);
}

GekkoF GekkoCPURecompiler::CheckMemory()
{
	RecompileMemBlock	*CurBlock;
	RecompileMemBlock	*CurBlock2;
	u32					FreeSize;
	u32					FreeSizePtr;
	u32					TotalSize;

	CurBlock = FreeMemPtr;
	FreeSizePtr = 0;
	while(CurBlock)
	{
		if(!(CurBlock->Size & 0x80000000))
			FreeSizePtr += CurBlock->Size;

		if(CurBlock->PtrNext && (CurBlock->PtrNext < CurBlock))
			printf("Flaw\n");

		if(CurBlock->PtrPrev)
			if((u32)CurBlock != ((u32)CurBlock->PtrPrev + ((u32)CurBlock->PtrPrev->Size & ~0x80000000)))
				printf("Flaw\n");

		if(CurBlock->PtrNext)
			if((u32)CurBlock->PtrNext != ((u32)CurBlock + ((u32)CurBlock->Size & ~0x80000000)))
				printf("Flaw\n");

		CurBlock = CurBlock->PtrNext;
	}

	CurBlock = FreeMemSize;
	FreeSize = 0;
	while(CurBlock)
	{
		CurBlock2 = CurBlock->SizeSameNext;
		while(CurBlock2)
		{
			if(CurBlock2->Size & 0x80000000)
				printf("Flaw\n");
			FreeSize += CurBlock2->Size;
			CurBlock2 = CurBlock2->SizeSameNext;
		}

		if(CurBlock->Size & 0x80000000)
			printf("Flaw\n");

		FreeSize += CurBlock->Size;
		CurBlock = CurBlock->SizeNext;
	}

	if(FreeSize != FreeSizePtr)
		printf("Mismatch in list\n");
}

GekkoF GekkoCPURecompiler::DumpMemoryLayout()
{
	RecompileMemBlock	*CurBlock;
	//HANDLE				FileHandle;
	char				DumpData[1024];
	DWORD				BytesWritten;

    // TODO(ShizZy): Make cross platform 2012-03-07
	/*FileHandle = CreateFile("memory layout.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		printf("Unable to create memory layout file!\n");
		return;
	}*/return; // removeme when fixed

	CurBlock = StartMem;
	while(CurBlock)
	{
		sprintf(DumpData, "Block 0x%08X\tSize 0x%08X\tBlock End 0x%08X\t",(u32)CurBlock, CurBlock->Size & 0x7FFFFFFF, (u32)CurBlock + (CurBlock->Size & 0x7FFFFFFF));
		if(CurBlock->Size & 0x80000000)
			strcat(DumpData, "Allocated\n");
		else
			strcat(DumpData, "FREE\n");

        // TODO(ShizZy): Make cross platform 2012-03-07
		//WriteFile(FileHandle, DumpData, strlen(DumpData), &BytesWritten, 0);

		CurBlock = CurBlock->PtrNext;
	}

    // TODO(ShizZy): Make cross platform 2012-03-07
	//CloseHandle(FileHandle);
	printf("Dumped memory layout\n");
}

GekkoF GekkoCPURecompiler::DumpMemoryDLL()
{
	//HANDLE					FileHandle;
	DWORD					BytesWritten;
	IMAGE_NT_HEADERS32		PEHeader;
	IMAGE_DOS_HEADER		DOSHeader;
	IMAGE_SECTION_HEADER	SectionHeader;

	//This function will take the memory block and dump it into a dll format
	//The purpose of this is for loading with something like Intel VTune to allow disassembly
	/*FileHandle = CreateFile("gekko memory dump.dll", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		printf("Unable to create memory layout file!\n");
		return;
	}*/ return; //removeme when fixed

	memset(&DOSHeader, 0, sizeof(DOSHeader));
	memset(&PEHeader, 0, sizeof(PEHeader));
	memset(&SectionHeader, 0, sizeof(SectionHeader));

	DOSHeader.e_lfanew = sizeof(DOSHeader);
	DOSHeader.e_magic = 0x5A4D;

	//PE\0\0
	PEHeader.Signature = 0x00004550;
	PEHeader.FileHeader.SizeOfOptionalHeader = sizeof(PEHeader.OptionalHeader);
	PEHeader.FileHeader.Machine = 0x14C;
	PEHeader.FileHeader.NumberOfSections = 1;
	PEHeader.FileHeader.Characteristics = 0x2102;
	PEHeader.OptionalHeader.Magic = 0x10B;
	PEHeader.OptionalHeader.SizeOfCode = RECOMPILEMEM_SIZE;
	PEHeader.OptionalHeader.BaseOfCode = 0x1000;
	PEHeader.OptionalHeader.ImageBase = (DWORD)StartMem;
	PEHeader.OptionalHeader.FileAlignment = 4096;
	PEHeader.OptionalHeader.SectionAlignment = 4096;
	PEHeader.OptionalHeader.SizeOfHeaders = sizeof(DOSHeader) + sizeof(PEHeader) + sizeof(SectionHeader);
	PEHeader.OptionalHeader.SizeOfImage = PEHeader.OptionalHeader.SizeOfHeaders + RECOMPILEMEM_SIZE;
	PEHeader.OptionalHeader.AddressOfEntryPoint = 0;
	PEHeader.OptionalHeader.MajorOperatingSystemVersion = 4;
	PEHeader.OptionalHeader.MajorLinkerVersion = 8;
	PEHeader.OptionalHeader.MajorSubsystemVersion = 4;
	PEHeader.OptionalHeader.SizeOfHeapCommit = 0x1000;
	PEHeader.OptionalHeader.SizeOfStackCommit = 0x1000;
	PEHeader.OptionalHeader.SizeOfHeapReserve = 0x10000;
	PEHeader.OptionalHeader.SizeOfStackReserve = 0x10000;
	PEHeader.OptionalHeader.Subsystem = 2;
	PEHeader.OptionalHeader.NumberOfRvaAndSizes = 16;

	strcpy((char *)&SectionHeader.Name[0], ".text");
	SectionHeader.SizeOfRawData = RECOMPILEMEM_SIZE;
	SectionHeader.PointerToRawData = 0x1000;
	SectionHeader.VirtualAddress = 0x1000;
	SectionHeader.Misc.VirtualSize = RECOMPILEMEM_SIZE;
	SectionHeader.Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_EXECUTE;

    // TODO(ShizZy): Make cross platform 2012-03-07
	/*WriteFile(FileHandle, &DOSHeader, sizeof(DOSHeader), &BytesWritten, 0);
	WriteFile(FileHandle, &PEHeader.Signature, 4, &BytesWritten, 0);
	WriteFile(FileHandle, &PEHeader.FileHeader, IMAGE_SIZEOF_FILE_HEADER, &BytesWritten, 0);
	WriteFile(FileHandle, &PEHeader.OptionalHeader, sizeof(PEHeader.OptionalHeader), &BytesWritten, 0);
	WriteFile(FileHandle, &SectionHeader, sizeof(SectionHeader), &BytesWritten, 0);
	SetFilePointer(FileHandle, 0x1000, 0, 0);
	WriteFile(FileHandle, StartMem, RECOMPILEMEM_SIZE, &BytesWritten, 0);
	CloseHandle(FileHandle);*/
	printf("Dumped dynarec memory to dll binary\n");
}