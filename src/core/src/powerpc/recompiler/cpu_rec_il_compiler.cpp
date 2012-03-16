////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_il_compiler.cpp
// DESC:		Compile IL to x86
// CREATED:		Mar. 29, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "powerpc/cpu_core_regs.h"
#include "hw/hw.h"

GekkoCPURecompiler::JumpList	*GekkoCPURecompiler::JumpLabelEntries;
GekkoCPURecompiler::JumpList	*GekkoCPURecompiler::JumpEntries;

GekkoCPURecompiler::CompiledBlock	GekkoCPURecompiler::CompileBlockNeeded;

GekkoCPURecompiler::CompiledBlock	CompileErrorBlock;

GekkoF EMU_FASTCALL CompileErrorFunction()
{
	printf("Error during dynarec compile\n");
}

GekkoF EMU_FASTCALL GekkoCPURecompiler::CompileIL(u32 OldPC, u32 NewPC)
{
	u8				*X86Buffer;
	u32				BufferSize;
	u32				InstSize;
	RecInstruction	*CurInstructionsBegin;
	u32				LastInstIsBranch;
	CompiledBlock	*NewCompiledBlock;
	CompiledBlock	*CurBlock;
	JumpList		*CurJump;
	PPCRegInfo		X86Regs[X86_REG_COUNT+1];	//current PPC assigned to this register
												//last entry is a dummy for verifying counts
	PPCRegInfo		FPURegs[FPU_REG_COUNT+1];	//current PPC FPU assigned to this register
												//last entry is a dummy for verifying counts

	LastInstIsBranch = (LastInstruction->Flags & RecInstrFlag_Branch);

	//allocate 2k for a buffer
	X86Buffer = (u8 *)RecompileAlloc(1024*1024);
	if(!X86Buffer)
	{
		printf("ERROR! Unable to allocate Recompile Buffer!\n");
		CheckMemory();
		DumpMemoryLayout();
		CompiledTable[(OldPC >> 2) & 0x7FFFFF] = (u32)&CompileErrorBlock;
		CompileErrorBlock.CodeBlock = (GekkoFP)&CompileErrorFunction;
		cpu->pause = true;
		return;
	}

	BufferSize = 0;

	NewCompiledBlock = (CompiledBlock *)RecompileAlloc(sizeof(CompiledBlock));
	NewCompiledBlock->CodeStart = (OldPC & 0x01FFFFFF);
	NewCompiledBlock->InstCount = ((NewPC - OldPC) / 4) + 1;

	//add a push to the beginning of the tree
	CurInstructionsBegin = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
	memset(CurInstructionsBegin, 0, sizeof(RecInstruction));
	CurInstructionsBegin->Instruction = GekkoRecIL(PUSHA);
	CurInstructionsBegin->Next = InstructionBlock;
	InstructionBlock->Prev = CurInstructionsBegin;
	InstructionBlock = CurInstructionsBegin;

//	if(!IsInterpret)
//	{
		memset(X86Regs, 0xFF, sizeof(X86Regs));
		memset(FPURegs, 0xFF, sizeof(FPURegs));
		memset(PPCRegVals, 0, sizeof(PPCRegVals));
		memset(CRKnown, 0, sizeof(CRKnown));
		memset(XERKnown, 0, sizeof(XERKnown));
		memset(SetXERKnown, 0, sizeof(SetXERKnown));
		memset(SetCRKnown, 0, sizeof(SetCRKnown));

		JumpEntries = 0;
		JumpLabelEntries = 0;

		AssignRegsAndCompile(X86Regs, FPURegs, X86Buffer, &BufferSize);

		CreateRecInstruction(GekkoRecIL(SaveAllX86Regs), 0, 0, 0, 0);
		CreateRecInstruction(GekkoRecIL(SaveAllFPURegs), 0, 0, 0, 0);
/*
		//store all of the current registers
		for(x = 0; x < X86_REG_COUNT; x++)
		{
			if((X86Regs[x].PPCRegister != -1) && X86Regs[x].ValueChanged)
			{
				//put in a move to save the register
				CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InReg | RecInstrFlagX86OutMem, 0);
				LastInstruction->X86InReg = x;
				LastInstruction->X86OutMemory = (u32)&ireg.gpr[X86Regs[x].PPCRegister];
			}
		}
*/
//	}
/*	else
	{
//if in debug then do the instruction count for easier validation
//otherwise only do it if we are doing a cpu compare to save time
#ifndef _DEBUG
		if(PipeHandle)
#endif
		{
			//only update IC if we are comparing cores, otherwise we do not care
			CreateRecInstruction(GekkoRecIL(ADD), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
			LastInstruction->X86InVal = NewCompiledBlock->InstCount;
			LastInstruction->X86OutMemory = (u32)&ireg.IC;
		}

//		CreateRecInstruction(GekkoRecIL(ADD), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
//		LastInstruction->X86InVal = NewCompiledBlock->InstCount;
//		LastInstruction->X86OutMemory = (u32)&TotalInstCount;
		CreateRecInstruction(GekkoRecIL(POPA), 0, 0, 0, 0);
		CreateRecInstruction(GekkoRecIL(RET), 0, 0, 0, 0);
	}
*/
	//go thru the instructions, compiling each one
	LastInstruction = InstructionBlock;
	while(LastInstruction)
	{
		InstSize = 0;
		if(!ProcessRecPPCCache(LastInstruction, &X86Buffer[BufferSize], &BufferSize, X86Regs, FPURegs))
			LastInstruction->Instruction(LastInstruction, &X86Buffer[BufferSize], &InstSize, X86Regs, FPURegs);
		BufferSize += InstSize;

		//if we get close to our buffer size then error about it
		if(BufferSize >= 1023*1024)
		{
			printf("ERROR! Recompile Buffer FULL!!!!\n");
			cpu->pause = true;
			CompiledTable[(OldPC >> 2) & 0x7FFFFF] = (u32)&CompileErrorBlock;
			CompileErrorBlock.CodeBlock = (GekkoFP)&CompileErrorFunction;
			return;
		}

		//move to the next entry
		LastInstruction = LastInstruction->Next;
	};

	while(InstructionBlock)
	{
		LastInstruction = InstructionBlock;
		InstructionBlock = InstructionBlock->Next;
		RecompileFree(LastInstruction);
	}

	LastInstruction = 0;

	//now tack on the known values and left over info
//	if(!IsInterpret)
//	{
		CreateRecInstruction(GekkoRecIL(SaveAllKnownX86ValRegs), 0, 0, 0, 0);

		if(!branch) // & (Branch_SC | Branch_RFI)))
		{
//			CreateRecInstruction(GekkoRecIL(ADD), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
//			LastInstruction->X86InVal = NewCompiledBlock->InstCount * 4;
//			LastInstruction->X86OutMemory = (u32)&pPC;
			CreateRecInstruction(GekkoRecIL(ADD), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
			LastInstruction->X86InVal = NewCompiledBlock->InstCount * 4;
			LastInstruction->X86OutMemory = (u32)&ireg.PC;
		}

		//add the instruction count to a couple values
#ifndef _DEBUG
		if(PipeHandle)
#endif
		{
			CreateRecInstruction(GekkoRecIL(ADD), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
			LastInstruction->X86InVal = NewCompiledBlock->InstCount;
			LastInstruction->X86OutMemory = (u32)&ireg.IC;
		}
//		CreateRecInstruction(GekkoRecIL(ADD), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
//		LastInstruction->X86InVal = NewCompiledBlock->InstCount;
//		LastInstruction->X86OutMemory = (u32)&TotalInstCount;
		CreateRecInstruction(GekkoRecIL(POPA), 0, 0, 0, 0);

		if(branch)
		{
/*
			ireg.TBR.TBR += TotalInstCount;
			Ret = Flipper_Update;
			if(DEC < TotalInstCount)
			{
				is_dec = MSR_BIT_EE;
				edx = MSR_BIT_EE;
			}
			else
			{
				edx = is_dec;
			}
			DEC -= TotalInstCount;

			if(!Ret && (ireg.MSR & edx))
			{
				is_dec = 0;
				cpu->Exception(GEX_DEC);
			}
*/
			u32	JumpID = NextJumpID;
			u32 JumpID2 = NextJumpID;

			CreateRecInstruction(GekkoRecIL(ADD), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
			LastInstruction->X86OutMemory = (u32)&ireg.TBR.TBL;
			LastInstruction->X86InVal = TotalInstCount;
			CreateRecInstruction(GekkoRecIL(ADC), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
			LastInstruction->X86OutMemory = (u32)&ireg.TBR.TBU;
			LastInstruction->X86InVal = 0;

			if(!(branch & Branch_RFI))
				CreateRecInstruction(REC_CALL((u32)&Flipper_Update));

			CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InMem | RecInstrFlagX86OutReg, 0);
			LastInstruction->X86InMemory = (u32)&ireg.spr[I_DEC];
			LastInstruction->X86OutReg = REG_EBX;

			CreateRecInstruction(GekkoRecIL(CMP), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutReg, 0);
			LastInstruction->X86InVal = TotalInstCount;
			LastInstruction->X86OutReg = REG_EBX;
			CreateRecInstruction(REC_JAE(JumpID));
			CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
			LastInstruction->X86InVal = MSR_BIT_EE;
			LastInstruction->X86OutMemory = (u32)&is_dec;
			if(!(branch & Branch_RFI))
			{
				CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutReg, 0);
				LastInstruction->X86InVal = MSR_BIT_EE;
				LastInstruction->X86OutReg = REG_EDX;
				CreateRecInstruction(REC_JMP(JumpID2));
			}

			CreateRecInstruction(REC_JUMPLABEL(JumpID));
			if(!(branch & Branch_RFI))
			{
				CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InMem | RecInstrFlagX86OutReg, 0);
				LastInstruction->X86InMemory = (u32)&is_dec;
				LastInstruction->X86OutReg = REG_EDX;

				CreateRecInstruction(REC_JUMPLABEL(JumpID2));
			}

			CreateRecInstruction(GekkoRecIL(SUB), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutReg, 0);
			LastInstruction->X86InVal = TotalInstCount;
			LastInstruction->X86OutReg = REG_EBX;
			CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InReg | RecInstrFlagX86OutMem, 0);
			LastInstruction->X86OutMemory = (u32)&ireg.spr[I_DEC];
			LastInstruction->X86InReg = REG_EBX;

			if(!(branch & Branch_RFI))
			{
				JumpID = NextJumpID;

				CreateRecInstruction(GekkoRecIL(CMP), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutReg, 0);
				LastInstruction->X86OutReg = REG_EAX;
				LastInstruction->X86InVal = 0;
				CreateRecInstruction(REC_JNZ(JumpID));

				CreateRecInstruction(GekkoRecIL(AND), 0, 0, RecInstrFlagX86InMem | RecInstrFlagX86OutReg, 0);
				LastInstruction->X86OutReg = REG_EDX;
				LastInstruction->X86InMemory = (u32)&ireg.MSR;
				CreateRecInstruction(REC_JZ(JumpID));

				CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutMem, 0);
				LastInstruction->X86InVal = 0;
				LastInstruction->X86OutMemory = (u32)&is_dec;
				CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutReg, 0);
				LastInstruction->X86InVal = GekkoCPU::GEX_DEC;
				LastInstruction->X86OutReg = REG_ECX;
				CreateRecInstruction(REC_CALL((u32)&GekkoCPURecompiler::Exception_REC));

				CreateRecInstruction(REC_JUMPLABEL(JumpID));
			}

			if(PipeHandle)
				CreateRecInstruction(REC_CALL((u32)&GekkoCPURecompiler::DoBranchChecks));

			//reset our instruction count for the branch
			TotalInstCount = 0;
		}

		CreateRecInstruction(GekkoRecIL(RET), 0, 0, 0, 0);

		//go thru the instructions, compiling each one
		LastInstruction = InstructionBlock;
		while(LastInstruction)
		{
			InstSize = 0;
			LastInstruction->Instruction(LastInstruction, &X86Buffer[BufferSize], &InstSize, X86Regs, 0);
			BufferSize += InstSize;

			//if we get close to our buffer size then error about it
			if(BufferSize >= 1023*1024)
			{
				printf("ERROR! Recompile Buffer FULL!!!!\n");
				cpu->pause = true;
				return;
			}

			//move to the next entry
			LastInstruction = LastInstruction->Next;
		};

		while(InstructionBlock)
		{
			LastInstruction = InstructionBlock;
			InstructionBlock = InstructionBlock->Next;
			RecompileFree(LastInstruction);
		}

		LastInstruction = 0;

		//wipe out the jump entries
		while(JumpEntries)
		{
			//find the label we need
			CurJump = JumpLabelEntries;
			while(CurJump)
			{
				if((CurJump->ID & 0x7FFFFFFF) == (JumpEntries->ID & 0x7FFFFFFF))
					break;

				CurJump = CurJump->Next;
			}

			if(!CurJump)
			{
				printf("Matching jump label %d not found, address 0x%08X\n", (JumpEntries->ID & 0x7FFFFFFF), JumpEntries->Address);
				cpu->pause = true;
			}
			else
			{
				//these entries did not have a label to find, reprocess them
				ProcessJump(CurJump, JumpEntries);
			}

			//remove the jump entry
			CurJump = JumpEntries;
			JumpEntries = JumpEntries->Next;
			RecompileFree(CurJump);
		}

		//delete all the labels
		while(JumpLabelEntries)
		{
			CurJump = JumpLabelEntries;
			JumpLabelEntries = JumpLabelEntries->Next;
			RecompileFree(CurJump);
		}
//	}

	RecompilePartialFree(X86Buffer, BufferSize);
	NewCompiledBlock->CodeBlock = (GekkoFP)X86Buffer;
	NewCompiledBlock->CommandLen = BufferSize;

	CurBlock = (CompiledBlock *)CompiledTablePages[((OldPC & 0x1FFFFFF) / PageSize)];

	//if an entry exists then fill it in
	if(CurBlock)
	{
		//a block already exists
		NewCompiledBlock->NextBlock = CurBlock;
//		CurBlock->NextBlock = NewCompiledBlock;
	}
	else
	{
		//this is the parent entry, set it
		NewCompiledBlock->NextBlock = 0;
	}

	CompiledTablePages[((OldPC & 0x1FFFFFF) / PageSize)] = (u32)NewCompiledBlock;
	CompiledTable[(OldPC >> 2) & 0x7FFFFF] = (u32)NewCompiledBlock;

/*
	//if the new block is larger than 1 instruction then create a compile entry
	//that has info about this entry
	if(NewCompiledBlock->InstCount > 1)
	{
		//create a new block with the same info
		//but a pointer to the compile function
		CurBlock = (CompiledBlock *)RecompileAlloc(sizeof(CompiledBlock));
		CurBlock->CodeStart = NewCompiledBlock->CodeStart;
		CurBlock->InstCount = NewCompiledBlock->InstCount;
		CurBlock->CodeBlock = &GekkoCPURecompiler::CompileInstruction;

		//add the info together
		CurBlock->ParentBlock = NewCompiledBlock;
		CurBlock->NextBlock = NewCompiledBlock->NextBlock;
		NewCompiledBlock->NextBlock = CurBlock;

		//add it to the list
		CompiledTable[((OldPC >> 2) & 0x007FFFFF) + 1] = (u32)CurBlock;
	}
*/
	//now that the buffer is full, 
	LastInstruction = 0;
	NextJumpID_Val = 0;
	NextGetRegID_Val = 0x50;
}

GekkoCPURecompiler::RecX86FPUAssignRegsPtr EMU_FASTCALL GekkoCPURecompiler::RecX86FPUAssignRegs[] =
{
	//0x00
	X86AssignRegs(NONE),
	X86AssignRegs(InReg),
	X86AssignRegs(InVal),
	0,0,0,0,
	X86AssignRegs(KnownPPCValue),
	X86AssignRegs(OutReg),
	X86AssignRegs(InReg_OutReg),
	X86AssignRegs(InVal_OutReg),
	0,0,0,
	X86FPUAssignRegs(InFPUReg_OutReg),
	0,
	//0x10
	0,
	X86AssignRegs(InReg_OutMem),
	X86AssignRegs(InVal_OutMem),
	0,0,0,0,0,0,
	X86FPUAssignRegs(InReg_OutFPUReg),
	X86FPUAssignRegs(InVal_OutFPUReg),
	0,
	FPUAssignRegs(InMem_OutReg),
	0,
	FPUAssignRegs(InReg_OutReg),
	0,
	//0x20
	0
};

GekkoF EMU_FASTCALL GekkoCPURecompiler::AssignRegsAndCompile(PPCRegInfo *X86Regs, PPCRegInfo *FPURegs, u8 *X86Buffer, u32 *BufferSize)
{
	RecInstruction	*CurInstruction;
	RecInstruction	*PrevInstruction;
	RecInstruction	*NextInstruction;
	RecInstruction	TempInstruction;
	u32				InstSize;

	//go thru an instruction block and figure out what to assign to each register
	//eax, ecx, edx, ebx, esp, ebp, esi, edi
	memset(&TempInstruction, 0, sizeof(RecInstruction));
	TempInstruction.Next = InstructionBlock;
	InstructionBlock->Prev = &TempInstruction;
	CurInstruction = InstructionBlock;
	while(CurInstruction)
	{
		if(CurInstruction->Flags & RecInstrFlag_IntInstruction)
		{
			//save off any registers needed to be saved
			InstSize = 0;
			GekkoRecIL(SaveAllX86Regs)(CurInstruction, &X86Buffer[*BufferSize], &InstSize, X86Regs, 0);
			(*BufferSize) += InstSize;

			GekkoRecIL(SaveAllFPURegs)(CurInstruction, &X86Buffer[*BufferSize], &InstSize, 0, FPURegs);
			(*BufferSize) += InstSize;

			//save off any known values needed to be saved
			InstSize = 0;
			GekkoRecIL(SaveAllKnownX86ValRegs)(CurInstruction, &X86Buffer[*BufferSize], &InstSize, X86Regs, 0);
			(*BufferSize) += InstSize;

			//call the instruction
			InstSize = 0;
			CurInstruction->Instruction(CurInstruction, &X86Buffer[*BufferSize], &InstSize, X86Regs, FPURegs); 
			(*BufferSize) += InstSize;

			CurInstruction = CurInstruction->Next;
		}
		else
		{
			PrevInstruction = CurInstruction->Prev;
			NextInstruction = CurInstruction->Next;

			if(!CheckRecPPCCache(CurInstruction))
				RecX86FPUAssignRegs[CurInstruction->Flags & RecInstrFlagPPC_MASK](CurInstruction, X86Regs, FPURegs);

			if(PrevInstruction)
				CurInstruction = PrevInstruction->Next;
			else 
				CurInstruction = InstructionBlock->Next;
			while(CurInstruction != NextInstruction)
			{
				if(!ProcessRecPPCCache(CurInstruction, &X86Buffer[*BufferSize], BufferSize, X86Regs, FPURegs))
				{
					InstSize = 0;

					CurInstruction->Instruction(CurInstruction, &X86Buffer[*BufferSize], &InstSize, X86Regs, FPURegs); 
					(*BufferSize) += InstSize;
				}

				CurInstruction = CurInstruction->Next;
			}
		}
	}

	LastInstruction = InstructionBlock;
	while(InstructionBlock)
	{
		CurInstruction = InstructionBlock;
		InstructionBlock = InstructionBlock->Next;
		RecompileFree(CurInstruction);
	}

	LastInstruction = 0;
}