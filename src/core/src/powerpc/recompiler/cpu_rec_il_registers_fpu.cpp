////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_il_registers_fpu.cpp
// DESC:		Assembler of the IL opcodes for the recompiler
// CREATED:		May 22, 2009
////////////////////////////////////////////////////////////
// Copyright (c) 2009 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "../cpu_core_regs.h"

u32	GekkoCPURecompiler::FindReplacableFPUReg(RecInstruction *Instruction, PPCRegInfo *FPURegs, 
										  u32 FindReg, u32 *FindRegMem, u32 *FindCount,
										  u32 FindNextLowest)
{
	RecInstruction *NextInstruction;
	u8		RegFlag = 0;
	u32		x;
	u32		InstructCount;
	u32		MinRegCount;
	u32		MinRegCount2;
	u32		RegCount[FPU_REG_COUNT+1];
	u32		OutRegLost;

	//go thru the assigned registers and find a register to replace
	//also return if the register is not used often enough and should come from memory
	//FindReg = register that is to be checked
	//*FindRegMem = set to a non zero value if reading from memory is suggested
	//*FindCount = how many times the register was found in a small block of code
	memset(RegCount, 0, sizeof(RegCount));
	NextInstruction = Instruction->Next;

	//assign to our temp buffer to see if the FindReg should be read from memory
	FPURegs[FPU_REG_COUNT].PPCRegister = FindReg;
	FPURegs[FPU_REG_COUNT].ValueChanged = 0;
	OutRegLost = 0;
	*FindCount = 0;
	*FindRegMem = 0;
//	for(InstructCount = 0; (InstructCount < 20); InstructCount++)
//	{
	for(x=0; x <= FPU_REG_COUNT; x++)
	{
		if(FPURegs[x].PPCRegister == -1)
		{
			if(FindNextLowest)
				FindNextLowest = 0;
			else if(*FindCount == 0)
			{	
				*FindCount = 1;
				*FindRegMem = x;

				//skip to the end being we found a register. We just need to
				//see how often it is used
				x = FPU_REG_COUNT;

				NextInstruction = Instruction->Next;
				OutRegLost = 0;
				for(InstructCount = 0; (InstructCount < 20) && NextInstruction; InstructCount++)
				{
					if(!(NextInstruction->Flags & RecInstrFlag_NoPPCRegCount))
					{
						if((NextInstruction->InReg == FPURegs[x].PPCRegister) && 
							(NextInstruction->Flags & RecInstrFlagPPCInFloatReg))
						{
							RegCount[x]++;
							break;
						}

						//if the output is used and it is not a direct assignment then count it
						if((NextInstruction->OutReg == FPURegs[x].PPCRegister) && !OutRegLost &&
							(NextInstruction->Flags & RecInstrFlagPPCOutFloatReg))
						{
							if(NextInstruction->Instruction == GekkoRecIL(MOVAPD))
								OutRegLost = 1;
							else
							{
								RegCount[x]++;
								break;
							}
						}
					}

					NextInstruction = NextInstruction->Next;
				}

				break;
			}
		}
		else
		{
			//if a special register is assigned, don't overwrite it
			if(FPURegs[x].PPCRegister & REG_SPECIAL)
				continue;

			//if the register matches the in or out register then don't allow it
			if(((FPURegs[x].PPCRegister == Instruction->InReg) &&
				((Instruction->Flags & RecInstrFlagPPCInFloatReg) == RecInstrFlagPPCInFloatReg)) ||
			   ((FPURegs[x].PPCRegister == Instruction->OutReg) &&
			    ((Instruction->Flags & RecInstrFlagPPCOutFloatReg) == RecInstrFlagPPCOutFloatReg)))
				continue;		//if the register matches one we can not override then exit
		}

		NextInstruction = Instruction->Next;
		OutRegLost = 0;
		for(InstructCount = 0; (InstructCount < 20) && NextInstruction; InstructCount++)
		{
			if(!(NextInstruction->Flags & RecInstrFlag_NoPPCRegCount))
			{
				if((NextInstruction->InReg == FPURegs[x].PPCRegister) && 
				(NextInstruction->Flags & RecInstrFlagPPCInFloatReg))
					RegCount[x]++;

				//if the output is used and it is not a direct assignment then count it
				if((NextInstruction->OutReg == FPURegs[x].PPCRegister) &&
				(NextInstruction->Flags & RecInstrFlagPPCOutFloatReg) && !OutRegLost)
				{
					if(NextInstruction->Instruction == GekkoRecIL(MOVAPD))
						OutRegLost = 1;
					else
						RegCount[x]++;
				}
			}

			NextInstruction = NextInstruction->Next;
		}
	}

	//if the count is set then we have an empty register to use
	if(*FindCount == 1)
	{
		x = *FindRegMem;

		//if we did not find any use of the actual register then
		//indicate that it should stay in memory
		if(RegCount[FPU_REG_COUNT] == 0)
		{
			if(FindReg <= FPU_REG_COUNT)
				*FindRegMem = (u32)&ireg.fpr[FindReg];
			*FindCount = RegCount[FPU_REG_COUNT];
		}
		else
			*FindRegMem = 0;

		return x;
	}

	//if the register matches the in or out register then don't allow it
	for(x = 0; x < FPU_REG_COUNT; x++)
	{
		//if a special register is assigned, don't overwrite it
		if(FPURegs[x].PPCRegister & REG_SPECIAL)
			continue;

		if(((FPURegs[x].PPCRegister != Instruction->InReg) ||
			((Instruction->Flags & RecInstrFlagPPCInFloatReg) != RecInstrFlagPPCInFloatReg)) &&
			((FPURegs[x].PPCRegister != Instruction->OutReg) ||
			((Instruction->Flags & RecInstrFlagPPCOutFloatReg) != RecInstrFlagPPCOutFloatReg)))
			break;	//no match, use it
	}

	//now find the lowest value and save it, not counting our own register
	MinRegCount = x;
	for(x=1; x < FPU_REG_COUNT; x++)
	{
		//if a special register is assigned, don't overwrite it
		if(FPURegs[x].PPCRegister & REG_SPECIAL)
			continue;

		//if the register matches the in or out register then don't allow it
		if(((FPURegs[x].PPCRegister == Instruction->InReg) &&
			((Instruction->Flags & RecInstrFlagPPCInFloatReg) == RecInstrFlagPPCInFloatReg)) ||
			((FPURegs[x].PPCRegister == Instruction->OutReg) &&
			((Instruction->Flags & RecInstrFlagPPCOutFloatReg) == RecInstrFlagPPCOutFloatReg)))
			continue;		//if the register matches one we can not override then exit

		if(RegCount[MinRegCount] > RegCount[x])
			MinRegCount = x;
	}

	//if we need to find the second lowest register then look it up
	if(FindNextLowest)
	{
		MinRegCount2 = MinRegCount;
		for(;;)
		{
			MinRegCount2 = (MinRegCount2 + 1) % FPU_REG_COUNT;

			//if a special register is assigned, don't overwrite it
			if(FPURegs[MinRegCount2].PPCRegister & REG_SPECIAL)
				continue;

			if(((FPURegs[MinRegCount2].PPCRegister != Instruction->InReg) &&
				((Instruction->Flags & RecInstrFlagPPCInFloatReg) == RecInstrFlagPPCInFloatReg)) &&
				((FPURegs[MinRegCount2].PPCRegister != Instruction->OutReg) &&
				((Instruction->Flags & RecInstrFlagPPCOutFloatReg) == RecInstrFlagPPCOutFloatReg)))
				break;	//no match, use it
		}

		for(x=1; x < FPU_REG_COUNT; x++)
		{
			//if a special register is assigned, don't overwrite it
			if(FPURegs[x].PPCRegister & REG_SPECIAL)
				continue;

			//if the register matches the in or out register then don't allow it
			if(((FPURegs[x].PPCRegister == Instruction->InReg) &&
				((Instruction->Flags & RecInstrFlagPPCInFloatReg) == RecInstrFlagPPCInFloatReg)) ||
				((FPURegs[x].PPCRegister == Instruction->OutReg) &&
				((Instruction->Flags & RecInstrFlagPPCOutFloatReg) == RecInstrFlagPPCOutFloatReg)))
				continue;		//if the register matches one we can not override then exit

			if((RegCount[MinRegCount2] > RegCount[x]) && (x != MinRegCount))
				MinRegCount2 = x;
		}
		MinRegCount = MinRegCount2;
	}

	//return how many times the register was found
	*FindCount = RegCount[FPU_REG_COUNT];

	//If the count is smaller or equal to the least used register then
	//assign a memory pointer
	if(*FindCount <= RegCount[MinRegCount])
		*FindRegMem = (u32)&ireg.fpr[FindReg];
	else
		*FindRegMem = 0;

	//return the register to replace
	return MinRegCount;
}

FPUAssignRegsOp(InReg_OutMem)
{
	FPUAssignRegs(InReg)(Instruction, 0, FPURegs);
	Instruction->X86OutVal = Instruction->OutMem;
	Instruction->Flags = (Instruction->Flags & ~RecInstrFlagX86_OUTMASK) | RecInstrFlagX86OutVal;
}

FPUAssignRegsOp(InReg)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		Reg;
	u8		RegFlag = 0;
	u32		RegMem;
	u32		RegCount;

	//if a direct register then just assign
	if(Instruction->InReg & 0x80000000)
	{
		Reg = Instruction->InReg & 0xFF;
		RegFlag = 1;
	}
	else
	{
		//find the in register
		for(x=0; x < FPU_REG_COUNT; x++)
		{
			if(Instruction->InReg == FPURegs[x].PPCRegister)
			{
				Reg = x;
				RegFlag = 1;
				break;
			}
		}
	}

	//figure out what registers we have
	if(!RegFlag)
	{
		//find a replacable register for in
		MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->InReg, &RegMem, &RegCount, 0);

		//create a new instruction to read the ppc register into an xmm register
		NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
		memset(NextInstruction, 0, sizeof(RecInstruction));
		NextInstruction->Instruction = GekkoRecIL(MOVAPD);
		NextInstruction->X86InMemory = (u32)&ireg.fpr[Instruction->InReg];
		NextInstruction->X86OutReg = MinReg;
		NextInstruction->InReg = Instruction->InReg;
		NextInstruction->OutReg = Instruction->InReg;
		NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
									RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg;

		//create a second instruction to store the in register we no longer will use
		if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
		{
			NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
			NextInstruction->Prev->Instruction = GekkoRecIL(MOVAPD);
			NextInstruction->Prev->X86InReg = MinReg;
			NextInstruction->Prev->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
			NextInstruction->Prev->Flags |= RecInstrFlagX86InFloatReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

			NextInstruction->Prev->Prev = Instruction->Prev;
			NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
		}
		else
		{
			NextInstruction->Prev = Instruction->Prev;
		}

		//update our pointers
		NextInstruction->Prev->Next = NextInstruction;
		NextInstruction->Next = Instruction;
		Instruction->Prev = NextInstruction;

		//assign the registers to the instruction
		FPURegs[MinReg].PPCRegister = Instruction->InReg;
		FPURegs[MinReg].ValueChanged = 0;
		Instruction->X86InReg = MinReg;
		Instruction->X86OutReg = MinReg;
		Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
								RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
	}
	else
	{
		//have register in a register
		Instruction->X86InReg = Reg;
		Instruction->X86OutReg = Reg;
		Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
								RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
	}
}

FPUAssignRegsOp(InReg_OutReg)
{
	RecInstruction *NextInstruction;
	RecInstruction *NextInstruction2;
	u32		x;
	u32		MinReg = 0;
	u32		MinReg2 = 0;
	u32		Reg[2];
	u8		RegFlag = 0;
	u32		RegMem;
	u32		RegCount;

	if(Instruction->InReg & 0x80000000)
	{
		Reg[0] = Instruction->InReg & 0xFF;
		RegFlag = 1;
	}

	if(Instruction->OutReg & 0x80000000)
	{
		Reg[1] = Instruction->OutReg & 0xFF;
		RegFlag |= 2;
	}

	//if not both special registers then find the in and/or out registers
	if(RegFlag != 3)
	{
		for(x=0; x < FPU_REG_COUNT; x++)
		{
			if(Instruction->InReg == FPURegs[x].PPCRegister)
			{
				Reg[0] = x;
				RegFlag |= 1;
				if(RegFlag == 3) break;
			}
			if(Instruction->OutReg == FPURegs[x].PPCRegister)
			{
				Reg[1] = x;
				RegFlag |= 2;
				if(RegFlag == 3) break;
			}
		}
	}

	//figure out what registers we have
	switch(RegFlag)
	{
		case 0:		//both registers are in memory
			//find a replacable register for in
			MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->InReg, &RegMem, &RegCount, 0);

			//if the instruction does not allow memory or the register is used more than once
			//then read it in
			if(!(Instruction->Flags & RecInstrFlag_FPUMemoryAllowed) || (RegCount > 1))
			{
				//create a new instruction to read the ppc register into an x86 register
				NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction, 0, sizeof(RecInstruction));
				NextInstruction->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction->X86InMemory = (u32)&ireg.fpr[Instruction->InReg];
				NextInstruction->X86OutReg = MinReg;
				NextInstruction->InReg = Instruction->InReg;
				NextInstruction->OutReg = Instruction->InReg;
				NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
											RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg;

				if((Instruction->InReg != Instruction->OutReg) && !(Instruction->Flags & RecInstrFlag_KnownOutValue))
				{
					MinReg2 = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMem, &RegCount, 1);

					NextInstruction->Next = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					memset(NextInstruction->Next, 0, sizeof(RecInstruction));
					NextInstruction->Next->Instruction = GekkoRecIL(MOVAPD);
					NextInstruction->Next->X86InMemory = (u32)&ireg.fpr[Instruction->OutReg];
					NextInstruction->Next->X86OutReg = MinReg2;
					NextInstruction->Next->InReg = Instruction->OutReg;
					NextInstruction->Next->OutReg = Instruction->OutReg;
					NextInstruction->Next->Flags |= RecInstrFlagX86InMem |
												RecInstrFlagX86OutReg |
												RecInstrFlag_NoPPCRegCount |
												RecInstrFlagPPCInFloatReg |
												RecInstrFlagPPCOutFloatReg;
					NextInstruction->Next->Prev = NextInstruction;
				}

				//create a second instruction to store the in register we no longer will use
				if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
				{
					NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
					NextInstruction->Prev->Instruction = GekkoRecIL(MOVAPD);
					NextInstruction->Prev->X86InReg = MinReg;
					NextInstruction->Prev->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
					NextInstruction->Prev->Flags |= RecInstrFlagX86InReg |
													RecInstrFlagX86OutMem |
													RecInstrFlag_NoPPCRegCount;

					NextInstruction->Prev->Prev = Instruction->Prev;
					NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
				}
				else
				{
					NextInstruction->Prev = Instruction->Prev;
				}

				//update our pointers
				NextInstruction->Prev->Next = NextInstruction;
				if(!(Instruction->Flags & RecInstrFlag_KnownOutValue))
				{
					NextInstruction->Next->Next = Instruction;
					Instruction->Prev = NextInstruction->Next;
				}
				else
				{
					NextInstruction->Next = Instruction;
					Instruction->Prev = NextInstruction;
				}

				//create a third instruction to store the out register we no longer will use
				if((Instruction->InReg != Instruction->OutReg) && (FPURegs[MinReg2].PPCRegister != -1) && FPURegs[MinReg2].ValueChanged)
				{
					NextInstruction2 = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					memset(NextInstruction2, 0, sizeof(RecInstruction));
					NextInstruction2->Instruction = GekkoRecIL(MOVAPD);
					NextInstruction2->X86InReg = MinReg2;
					NextInstruction2->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg2].PPCRegister];
					NextInstruction2->Flags |= RecInstrFlagX86InReg |
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

					NextInstruction2->Prev = NextInstruction->Prev;
					NextInstruction2->Next = NextInstruction;
					NextInstruction->Prev->Next = NextInstruction2;
					NextInstruction->Prev = NextInstruction2;
				}

				//assign the registers to the instruction
				FPURegs[MinReg].PPCRegister = Instruction->InReg;
				FPURegs[MinReg].ValueChanged = 0;

				if(Instruction->InReg != Instruction->OutReg)
				{
					FPURegs[MinReg2].PPCRegister = Instruction->OutReg;
					FPURegs[MinReg2].ValueChanged = 0;
				}
				else
					MinReg2 = MinReg;

				Instruction->X86InReg = MinReg;
				Instruction->X86OutReg = MinReg2;
				Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
										RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			}
			else
			{
				//find a replacable register for out
				MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMem, &RegCount, 0);

				//create a new instruction to store the register we no longer will use
				if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
				{
					NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					memset(NextInstruction, 0, sizeof(RecInstruction));
					NextInstruction->Instruction = GekkoRecIL(MOVAPD);
					NextInstruction->X86InReg = MinReg;
					NextInstruction->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
					NextInstruction->Flags |= RecInstrFlagX86InReg |
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

					//update our pointers
					NextInstruction->Next = Instruction;
					NextInstruction->Prev = Instruction->Prev;
					Instruction->Prev->Next = NextInstruction;
					NextInstruction->Prev = NextInstruction;
				}

				//assign the registers to the instruction
				FPURegs[MinReg].PPCRegister = Instruction->OutReg;
				FPURegs[MinReg].ValueChanged = 0;
				Instruction->X86InMemory = (u32)&ireg.fpr[Instruction->InReg];
				Instruction->X86OutReg = MinReg;
				Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
										RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
			}

			break;

		case 1:		//have in register, out is in memory, move out to a register
			//find a replacable register
			MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMem, &RegCount, 0);

			//create a new instruction to store the register we no longer will use
			if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
			{
				NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction, 0, sizeof(RecInstruction));
				NextInstruction->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction->X86InReg = MinReg;
				NextInstruction->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
				NextInstruction->Flags |= RecInstrFlagX86InReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

				//update our pointers
				NextInstruction->Next = Instruction;
				NextInstruction->Prev = Instruction->Prev;
				Instruction->Prev->Next = NextInstruction;
				Instruction->Prev = NextInstruction;
			}

			//create a new instruction to read in the out value
			//if this is not a move
			if((Instruction->Instruction != GekkoRecIL(MOVAPD)) && !(Instruction->Flags & RecInstrFlag_KnownOutValue))
			{
				NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction, 0, sizeof(RecInstruction));
				NextInstruction->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction->X86OutReg = MinReg;
				NextInstruction->X86InMemory = (u32)&ireg.fpr[Instruction->OutReg];
				NextInstruction->InReg = Instruction->OutReg;
				NextInstruction->OutReg = Instruction->OutReg;
				NextInstruction->Flags |= RecInstrFlagX86OutReg |
											RecInstrFlagX86InMem |
											RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg;

				//update our pointers
				NextInstruction->Next = Instruction;
				NextInstruction->Prev = Instruction->Prev;
				Instruction->Prev->Next = NextInstruction;
				Instruction->Prev = NextInstruction;
			}

			//assign the registers to the instruction
			FPURegs[MinReg].PPCRegister = Instruction->OutReg;
			FPURegs[MinReg].ValueChanged = 0;
			Instruction->X86InReg = Reg[0];
			Instruction->X86OutReg = MinReg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			break;

		case 2:		//have out register, in is in memory, move in to a register
			//find a replacable register
			MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->InReg, &RegMem, &RegCount, 0);

			//create a new instruction to read the ppc register into an x86 register
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVAPD);
			NextInstruction->X86InMemory = (u32)&ireg.fpr[Instruction->InReg];
			NextInstruction->X86OutReg = MinReg;
			NextInstruction->InReg = Instruction->InReg;
			NextInstruction->OutReg = Instruction->InReg;
			NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
										RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg;

			//create a second instruction to store the register we no longer will use
			if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
			{
				NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
				NextInstruction->Prev->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction->Prev->X86InReg = MinReg;
				NextInstruction->Prev->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
				NextInstruction->Prev->Flags |= RecInstrFlagX86InReg | 
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

				NextInstruction->Prev->Prev = Instruction->Prev;
				NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
			}
			else
			{
				NextInstruction->Prev = Instruction->Prev;
			}

			//update our pointers
			NextInstruction->Prev->Next = NextInstruction;
			NextInstruction->Next = Instruction;
			Instruction->Prev = NextInstruction;

			//assign the registers to the instruction
			FPURegs[MinReg].PPCRegister = Instruction->InReg;
			FPURegs[MinReg].ValueChanged = 0;
			Instruction->X86InReg = MinReg;
			Instruction->X86OutReg = Reg[1];
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			break;

		case 3:		//have registers in registers
			Instruction->X86InReg = Reg[0];
			Instruction->X86OutReg = Reg[1];
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			break;
	}
}

FPUAssignRegsOp(InMem_OutReg)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		Reg;
	u8		RegFlag = 0;
	u32		RegMem;
	u32		RegCount;

	if(Instruction->OutReg & 0x80000000)
	{
		Reg = Instruction->OutReg & 0xFF;
		RegFlag = 1;
	}
	else
	{
		//if not both special registers then find the in and/or out registers
		for(x=0; x < FPU_REG_COUNT; x++)
		{
			if(Instruction->OutReg == FPURegs[x].PPCRegister)
			{
				Reg = x;
				RegFlag = 1;
				break;
			}
		}
	}

	//figure out what registers we have
	switch(RegFlag)
	{
		case 0:		//out is in memory, move out to a register
			//find a replacable register
			MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMem, &RegCount, 0);

			//create a new instruction to store the register we no longer will use
			if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
			{
				NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction, 0, sizeof(RecInstruction));
				NextInstruction->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction->X86InReg = MinReg;
				NextInstruction->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
				NextInstruction->Flags |= RecInstrFlagX86InReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

				//update our pointers
				NextInstruction->Next = Instruction;
				NextInstruction->Prev = Instruction->Prev;
				Instruction->Prev->Next = NextInstruction;
				Instruction->Prev = NextInstruction;
			}

			//assign the registers to the instruction
			FPURegs[MinReg].PPCRegister = Instruction->OutReg;
			FPURegs[MinReg].ValueChanged = 0;
			Instruction->X86InMemory = Instruction->InMem;
			Instruction->X86OutReg = MinReg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
			break;


		case 1:		//have register
			Instruction->X86InMemory = Instruction->InMem;
			Instruction->X86OutReg = Reg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
			break;
	}
}

X86FPUAssignRegsOp(InReg_OutFPUReg)
{
	RecInstruction *NextInstruction;
	RecInstruction *NextInstruction2;
	u32		x;
	u32		MinReg = 0;
	u32		MinReg2 = 0;
	u32		Reg[2];
	u8		RegFlag = 0;
	u32		RegMemPtr = 0;
	u32		RegMemPtr2 = 0;
	u32		FindCount = 0;
	u32		FindCount2 = 0;

	if(Instruction->InReg & 0x80000000)
	{
		Reg[0] = Instruction->InReg & 0xFF;
		RegFlag = 1;
	}

	if(Instruction->OutReg & 0x80000000)
	{
		Reg[1] = Instruction->OutReg & 0xFF;
		RegFlag |= 2;
	}

	//if not both special registers then find the in and/or out registers
	if(RegFlag != 3)
	{
		if(!(RegFlag & 1))
		{
			for(x=0; x < X86_REG_COUNT; x++)
			{
				if(Instruction->InReg == AssignedRegs[x].PPCRegister)
				{
					Reg[0] = x;
					RegFlag |= 1;
					break;
				}
			}
		}

		if(!(RegFlag & 2))
		{
			for(x=0; x < FPU_REG_COUNT; x++)
			{
				if(Instruction->OutReg == FPURegs[x].PPCRegister)
				{
					Reg[1] = x;
					RegFlag |= 2;
					break;
				}
			}
		}
	}

	//figure out what registers we have
	switch(RegFlag)
	{
		case 0:		//both registers are in memory
			//find a replacable register for in
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg, &RegMemPtr, &FindCount, 0);

			//find a replacable register for out
			MinReg2 = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMemPtr2, &FindCount2, 0);

			//create a new instruction to read the ppc register into an x86 register
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVE);
			NextInstruction->X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
			NextInstruction->X86OutReg = MinReg;
			NextInstruction->InReg = Instruction->InReg;
			NextInstruction->OutReg = Instruction->InReg;
			NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
										RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

			//create a second instruction to store the in register we no longer will use
			if((AssignedRegs[MinReg].PPCRegister != -1) && AssignedRegs[MinReg].ValueChanged)
			{
				NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
				NextInstruction->Prev->Instruction = GekkoRecIL(MOVE);
				NextInstruction->Prev->X86InReg = MinReg;
				NextInstruction->Prev->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg].PPCRegister];
				NextInstruction->Prev->Flags |= RecInstrFlagX86InReg |
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

				NextInstruction->Prev->Prev = Instruction->Prev;
				NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
			}
			else
			{
				NextInstruction->Prev = Instruction->Prev;
			}

			//update our pointers
			NextInstruction->Prev->Next = NextInstruction;
			NextInstruction->Next = Instruction;
			Instruction->Prev = NextInstruction;

			//create a third instruction to store the out register we no longer will use
			if((FPURegs[MinReg2].PPCRegister != -1) && FPURegs[MinReg2].ValueChanged)
			{
				NextInstruction2 = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction2, 0, sizeof(RecInstruction));
				NextInstruction2->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction2->X86InReg = MinReg2;
				NextInstruction2->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg2].PPCRegister];
				NextInstruction2->Flags |= RecInstrFlagX86InReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

				NextInstruction2->Prev = NextInstruction->Prev;
				NextInstruction2->Next = NextInstruction;
				NextInstruction->Prev->Next = NextInstruction2;
				NextInstruction->Prev = NextInstruction2;
			}

			//assign the registers to the instruction
			AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
			AssignedRegs[MinReg].ValueChanged = 0;
			FPURegs[MinReg2].PPCRegister = Instruction->OutReg;
			FPURegs[MinReg2].ValueChanged = 0;
			Instruction->X86InReg = MinReg;
			Instruction->X86OutReg = MinReg2;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg;
			break;

		case 1:		//have in register, out is in memory, move out to a register
			//find a replacable register
			MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMemPtr, &FindCount, 0);

			//create a new instruction to store the register we no longer will use
			if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
			{
				NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction, 0, sizeof(RecInstruction));
				NextInstruction->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction->X86InReg = MinReg;
				NextInstruction->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
				NextInstruction->Flags |= RecInstrFlagX86InReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

				//update our pointers
				NextInstruction->Next = Instruction;
				NextInstruction->Prev = Instruction->Prev;
				Instruction->Prev->Next = NextInstruction;
				Instruction->Prev = NextInstruction;
			}

			//assign the registers to the instruction
			FPURegs[MinReg].PPCRegister = Instruction->OutReg;
			FPURegs[MinReg].ValueChanged = 0;
			Instruction->X86InReg = Reg[0];
			Instruction->X86OutReg = MinReg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg;
			break;

		case 2:		//have out register, in is in memory, move in to a register if need be
			//find a replacable register
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg, &RegMemPtr, &FindCount, 0);

			if(FindCount != 0)
			{
				//create a new instruction to read the ppc register into an x86 register
				NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction, 0, sizeof(RecInstruction));
				NextInstruction->Instruction = GekkoRecIL(MOVE);
				NextInstruction->X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
				NextInstruction->X86OutReg = MinReg;
				NextInstruction->InReg = Instruction->InReg;
				NextInstruction->OutReg = Instruction->InReg;
				NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
											RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

				//create a second instruction to store the register we no longer will use
				if((AssignedRegs[MinReg].PPCRegister != -1) && AssignedRegs[MinReg].ValueChanged)
				{
					NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
					NextInstruction->Prev->Instruction = GekkoRecIL(MOVE);
					NextInstruction->Prev->X86InReg = MinReg;
					NextInstruction->Prev->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg].PPCRegister];
					NextInstruction->Prev->Flags |= RecInstrFlagX86InReg | 
													RecInstrFlagX86OutMem |
													RecInstrFlag_NoPPCRegCount;

					NextInstruction->Prev->Prev = Instruction->Prev;
					NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
				}
				else
				{
					NextInstruction->Prev = Instruction->Prev;
				}

				//update our pointers
				NextInstruction->Prev->Next = NextInstruction;
				NextInstruction->Next = Instruction;
				Instruction->Prev = NextInstruction;

				//assign the registers to the instruction
				AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
				AssignedRegs[MinReg].ValueChanged = 0;
				Instruction->X86InReg = MinReg;
				Instruction->X86OutReg = Reg[1];
				Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
										RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg;
			}
			else
			{
				//leave IN in memory
				//assign the registers to the instruction
				Instruction->X86InMemory = (u32)&ireg.gpr[MinReg];
				Instruction->X86OutReg = Reg[1];
				Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
										RecInstrFlagX86InMem | RecInstrFlagX86OutFloatReg;
			}

			break;

		case 3:		//have registers in registers
			Instruction->X86InReg = Reg[0];
			Instruction->X86OutReg = Reg[1];
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg;
			break;
	}
}

X86FPUAssignRegsOp(InVal_OutFPUReg)
{
	RecInstruction *NextInstruction;
	RecInstruction *NextInstruction2;
	u32		x;
	u32		MinReg = 0;
	u32		MinReg2 = 0;
	u32		Reg;
	u8		RegFlag = 0;
	u32		RegMemPtr = 0;
	u32		RegMemPtr2 = 0;
	u32		FindCount = 0;
	u32		FindCount2 = 0;

	if(Instruction->OutReg & 0x80000000)
	{
		Reg = Instruction->OutReg & 0xFF;
		RegFlag = 1;
	}

	//if not both special registers then find the in and/or out registers
	if(RegFlag != 1)
	{
		for(x=0; x < FPU_REG_COUNT; x++)
		{
			if(Instruction->OutReg == FPURegs[x].PPCRegister)
			{
				Reg = x;
				RegFlag |= 1;
				break;
			}
		}
	}

	//figure out what registers we have
	switch(RegFlag)
	{
		case 0:		//in is a value and out is in memory
			//find a replacable register for in
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg, &RegMemPtr, &FindCount, 0);

			//find a replacable register for out
			MinReg2 = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMemPtr2, &FindCount2, 0);

			//create a new instruction to put the known ppc register value into
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVE);
			NextInstruction->X86InVal = MinReg;
			NextInstruction->X86OutReg = MinReg;
			NextInstruction->InReg = Instruction->InReg;
			NextInstruction->OutReg = Instruction->InReg;
			NextInstruction->Flags |= RecInstrFlagX86InVal | RecInstrFlagX86OutReg |
										RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_KnownInValue;

			//create a second instruction to store the in register we no longer will use
			if((AssignedRegs[MinReg].PPCRegister != -1) && AssignedRegs[MinReg].ValueChanged)
			{
				NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
				NextInstruction->Prev->Instruction = GekkoRecIL(MOVE);
				NextInstruction->Prev->X86InReg = MinReg;
				NextInstruction->Prev->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg].PPCRegister];
				NextInstruction->Prev->Flags |= RecInstrFlagX86InReg |
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

				NextInstruction->Prev->Prev = Instruction->Prev;
				NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
			}
			else
			{
				NextInstruction->Prev = Instruction->Prev;
			}

			//update our pointers
			NextInstruction->Prev->Next = NextInstruction;
			NextInstruction->Next = Instruction;
			Instruction->Prev = NextInstruction;

			//create a third instruction to store the out register we no longer will use
			if((FPURegs[MinReg2].PPCRegister != -1) && FPURegs[MinReg2].ValueChanged)
			{
				NextInstruction2 = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction2, 0, sizeof(RecInstruction));
				NextInstruction2->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction2->X86InReg = MinReg2;
				NextInstruction2->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg2].PPCRegister];
				NextInstruction2->Flags |= RecInstrFlagX86InReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

				NextInstruction2->Prev = NextInstruction->Prev;
				NextInstruction2->Next = NextInstruction;
				NextInstruction->Prev->Next = NextInstruction2;
				NextInstruction->Prev = NextInstruction2;
			}

			//assign the registers to the instruction
			AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
			AssignedRegs[MinReg].ValueChanged = 0;
			FPURegs[MinReg2].PPCRegister = Instruction->OutReg;
			FPURegs[MinReg2].ValueChanged = 0;
			Instruction->X86InReg = MinReg;
			Instruction->X86OutReg = MinReg2;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg;
			Instruction->Flags &= ~RecInstrFlag_KnownInValue;

			break;

		case 1:		//have out register, in is a value, move in to a register
			//find a replacable register
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg, &RegMemPtr, &FindCount, 0);

			//create a new instruction to read the ppc register into an x86 register
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVE);
			NextInstruction->X86OutReg = MinReg;
			NextInstruction->InReg = Instruction->InReg;
			NextInstruction->OutReg = Instruction->InReg;
			NextInstruction->Flags |= RecInstrFlagX86InVal | RecInstrFlagX86OutReg |
										RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_KnownInValue;

			//create a second instruction to store the register we no longer will use
			if((AssignedRegs[MinReg].PPCRegister != -1) && AssignedRegs[MinReg].ValueChanged)
			{
				NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
				NextInstruction->Prev->Instruction = GekkoRecIL(MOVE);
				NextInstruction->Prev->X86InReg = MinReg;
				NextInstruction->Prev->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg].PPCRegister];
				NextInstruction->Prev->Flags |= RecInstrFlagX86InReg | 
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

				NextInstruction->Prev->Prev = Instruction->Prev;
				NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
			}
			else
			{
				NextInstruction->Prev = Instruction->Prev;
			}

			//update our pointers
			NextInstruction->Prev->Next = NextInstruction;
			NextInstruction->Next = Instruction;
			Instruction->Prev = NextInstruction;

			//assign the registers to the instruction
			AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
			AssignedRegs[MinReg].ValueChanged = 0;
			Instruction->X86InReg = MinReg;
			Instruction->X86OutReg = Reg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg;
			Instruction->Flags &= ~RecInstrFlag_KnownInValue;

			break;
	}
}

X86FPUAssignRegsOp(InFPUReg_OutReg)
{
	RecInstruction *NextInstruction;
	RecInstruction *NextInstruction2;
	u32		x;
	u32		MinReg = 0;
	u32		MinReg2 = 0;
	u32		Reg[2];
	u8		RegFlag = 0;
	u32		RegMemPtr = 0;
	u32		FindCount = 0;

	if(Instruction->InReg & 0x80000000)
	{
		Reg[0] = Instruction->InReg & 0xFF;
		RegFlag = 1;
	}

	if(Instruction->OutReg & 0x80000000)
	{
		Reg[1] = Instruction->OutReg & 0xFF;
		RegFlag |= 2;
	}

	//if not both special registers then find the in and/or out registers
	if(RegFlag != 3)
	{
		if(!(RegFlag & 1))
		{
			for(x=0; x < FPU_REG_COUNT; x++)
			{
				if(Instruction->InReg == FPURegs[x].PPCRegister)
				{
					Reg[0] = x;
					RegFlag |= 1;
					break;
				}
			}
		}

		if(!(RegFlag & 2))
		{
			for(x=0; x < X86_REG_COUNT; x++)
			{
				if(Instruction->OutReg == AssignedRegs[x].PPCRegister)
				{
					Reg[1] = x;
					RegFlag |= 2;
					break;
				}
			}
		}
	}

	//figure out what registers we have
	switch(RegFlag)
	{
		case 0:		//both registers are in memory
			//find a replacable register for in
			MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->OutReg, &RegMemPtr, &FindCount, 0);

			//find a replacable register for out
			MinReg2 = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg, &RegMemPtr, &FindCount, 0);

			//create a new instruction to read the ppc register into an x86 register
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVAPD);
			NextInstruction->X86InMemory = (u32)&ireg.fpr[Instruction->InReg];
			NextInstruction->X86OutReg = MinReg;
			NextInstruction->InReg = Instruction->InReg;
			NextInstruction->OutReg = Instruction->InReg;
			NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutFloatReg |
										RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg;

			//create a second instruction to store the out register we no longer will use
			if((AssignedRegs[MinReg2].PPCRegister != -1) && AssignedRegs[MinReg2].ValueChanged)
			{
				NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
				NextInstruction->Prev->Instruction = GekkoRecIL(MOVE);
				NextInstruction->Prev->X86InReg = MinReg2;
				NextInstruction->Prev->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg2].PPCRegister];
				NextInstruction->Prev->Flags |= RecInstrFlagX86InReg |
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

				NextInstruction->Prev->Prev = Instruction->Prev;
				NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
			}
			else
			{
				NextInstruction->Prev = Instruction->Prev;
			}

			//update our pointers
			NextInstruction->Prev->Next = NextInstruction;
			if(!(Instruction->Flags & RecInstrFlag_KnownOutValue))
			{
				NextInstruction->Next->Next = Instruction;
				Instruction->Prev = NextInstruction->Next;
			}
			else
			{
				NextInstruction->Next = Instruction;
				Instruction->Prev = NextInstruction;
			}

			//create a third instruction to store the in register we no longer will use
			if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
			{
				NextInstruction2 = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction2, 0, sizeof(RecInstruction));
				NextInstruction2->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction2->X86InReg = MinReg;
				NextInstruction2->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
				NextInstruction2->Flags |= RecInstrFlagX86InReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

				NextInstruction2->Prev = NextInstruction->Prev;
				NextInstruction2->Next = NextInstruction;
				NextInstruction->Prev->Next = NextInstruction2;
				NextInstruction->Prev = NextInstruction2;
			}

			//assign the registers to the instruction
			AssignedRegs[MinReg2].PPCRegister = Instruction->InReg;
			AssignedRegs[MinReg2].ValueChanged = 0;
			FPURegs[MinReg].PPCRegister = Instruction->OutReg;
			FPURegs[MinReg].ValueChanged = 0;
			Instruction->X86InReg = MinReg2;
			Instruction->X86OutReg = MinReg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InFloatReg | RecInstrFlagX86OutReg;
			break;

		case 1:		//have in register, out is in memory, move out to a register
			//find a replacable register
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->OutReg, &RegMemPtr, &FindCount, 0);

			//create a new instruction to store the register we no longer will use
			if((AssignedRegs[MinReg].PPCRegister != -1) && AssignedRegs[MinReg].ValueChanged)
			{
				NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction, 0, sizeof(RecInstruction));
				NextInstruction->Instruction = GekkoRecIL(MOVE);
				NextInstruction->X86InReg = MinReg;
				NextInstruction->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg].PPCRegister];
				NextInstruction->Flags |= RecInstrFlagX86InReg |
											RecInstrFlagX86OutMem |
											RecInstrFlag_NoPPCRegCount;

				//update our pointers
				NextInstruction->Next = Instruction;
				NextInstruction->Prev = Instruction->Prev;
				Instruction->Prev->Next = NextInstruction;
				Instruction->Prev = NextInstruction;
			}

			//assign the registers to the instruction
			AssignedRegs[MinReg].PPCRegister = Instruction->OutReg;
			AssignedRegs[MinReg].ValueChanged = 0;
			Instruction->X86InReg = Reg[0];
			Instruction->X86OutReg = MinReg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InFloatReg | RecInstrFlagX86OutReg;
			break;

		case 2:		//have out register, in is in memory, move in to a register
			//find a replacable register
			MinReg = FindReplacableFPUReg(Instruction, FPURegs, Instruction->InReg, &RegMemPtr, &FindCount, 0);

			//create a new instruction to read the ppc register into a fpu register
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVAPD);
			NextInstruction->X86InMemory = (u32)&ireg.fpr[Instruction->InReg];
			NextInstruction->X86OutReg = MinReg;
			NextInstruction->InReg = Instruction->InReg;
			NextInstruction->OutReg = Instruction->InReg;
			NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
										RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

			//create a second instruction to store the register we no longer will use
			if((FPURegs[MinReg].PPCRegister != -1) && FPURegs[MinReg].ValueChanged)
			{
				NextInstruction->Prev = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				memset(NextInstruction->Prev, 0, sizeof(RecInstruction));
				NextInstruction->Prev->Instruction = GekkoRecIL(MOVAPD);
				NextInstruction->Prev->X86InReg = MinReg;
				NextInstruction->Prev->X86OutMemory = (u32)&ireg.fpr[FPURegs[MinReg].PPCRegister];
				NextInstruction->Prev->Flags |= RecInstrFlagX86InReg | 
												RecInstrFlagX86OutMem |
												RecInstrFlag_NoPPCRegCount;

				NextInstruction->Prev->Prev = Instruction->Prev;
				NextInstruction->Prev->Prev->Next = NextInstruction->Prev;
			}
			else
			{
				NextInstruction->Prev = Instruction->Prev;
			}

			//update our pointers
			NextInstruction->Prev->Next = NextInstruction;
			NextInstruction->Next = Instruction;
			Instruction->Prev = NextInstruction;

			//assign the registers to the instruction
			FPURegs[MinReg].PPCRegister = Instruction->InReg;
			FPURegs[MinReg].ValueChanged = 0;
			Instruction->X86InReg = MinReg;
			Instruction->X86OutReg = Reg[1];
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InFloatReg | RecInstrFlagX86OutReg;
			break;

		case 3:		//have registers in registers
			Instruction->X86InReg = Reg[0];
			Instruction->X86OutReg = Reg[1];
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InFloatReg | RecInstrFlagX86OutReg;
			break;
	}
}
