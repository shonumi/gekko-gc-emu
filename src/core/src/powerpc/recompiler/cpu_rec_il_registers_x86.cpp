#include "cpu_rec.h"
#include "../cpu_core_regs.h"

u32	GekkoCPURecompiler::FindReplacableX86Reg(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, 
										  u32 FindReg, u32 *FindRegMem, u32 *FindCount,
										  u32 FindNextLowest)
{
	RecInstruction *NextInstruction;
	u8		RegFlag = 0;
	u32		x;
	u32		InstructCount;
	u32		MinRegCount;
	u32		MinRegCount2;
	u32		RegCount[X86_REG_COUNT+1];
	u32		OutRegLost;

	//go thru the assigned registers and find a register to replace
	//also return if the register is not used often enough and should come from memory
	//FindReg = register that is to be checked
	//*FindRegMem = set to a non zero value if reading from memory is suggested
	//*FindCount = how many times the register was found in a small block of code
	memset(RegCount, 0, sizeof(RegCount));
	NextInstruction = Instruction->Next;

	//assign to our temp buffer to see if the FindReg should be read from memory
	AssignedRegs[X86_REG_COUNT].PPCRegister = FindReg;
	AssignedRegs[X86_REG_COUNT].ValueChanged = 0;
	OutRegLost = 0;
	*FindCount = 0;
	*FindRegMem = 0;
//	for(InstructCount = 0; (InstructCount < 20); InstructCount++)
//	{
	for(x=0; x <= X86_REG_COUNT; x++)
	{
		//skip ESP to allow calling to work
		if(x == 4) x++;

		if(AssignedRegs[x].PPCRegister == -1)
		{
			//if getting a temporary register, see if it matches a register we can
			//not allow
			if(Instruction->Instruction == GekkoRecIL(GETTEMPREG))
			{
				if((1 << x) & Instruction->InReg)
					continue;
			}

			if(FindNextLowest)
				FindNextLowest = 0;
			else if(*FindCount == 0)
			{
				*FindCount = 1;
				*FindRegMem = x;

				//skip to the end being we found a register. We just need to
				//see how often it is used
				x = X86_REG_COUNT;

				NextInstruction = Instruction->Next;
				OutRegLost = 0;
				for(InstructCount = 0; (InstructCount < 20) && NextInstruction; InstructCount++)
				{
					if(!(NextInstruction->Flags & RecInstrFlag_NoPPCRegCount))
					{
						if((NextInstruction->InReg == AssignedRegs[x].PPCRegister) && 
						(NextInstruction->Flags & RecInstrFlagPPCInReg))
						{
							RegCount[x]++;
							break;
						}

						//if the output is used and it is not a direct assignment then count it
						if((NextInstruction->OutReg == AssignedRegs[x].PPCRegister) &&
						(NextInstruction->Flags & RecInstrFlagPPCOutReg) && !OutRegLost)
						{
							if(NextInstruction->Instruction == GekkoRecIL(MOVE))
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
			//if the register matches the in or out register then don't allow it
			if(((AssignedRegs[x].PPCRegister == Instruction->InReg) && (Instruction->Flags & RecInstrFlagPPCInReg)) ||
			   ((AssignedRegs[x].PPCRegister == Instruction->OutReg) && (Instruction->Flags & RecInstrFlagPPCOutReg)))
				continue;		//if the register matches one we can not override then exit
		}

		NextInstruction = Instruction->Next;
		OutRegLost = 0;
		for(InstructCount = 0; (InstructCount < 20) && NextInstruction; InstructCount++)
		{
			if(!(NextInstruction->Flags & RecInstrFlag_NoPPCRegCount))
			{
				if((NextInstruction->InReg == AssignedRegs[x].PPCRegister) && 
				(NextInstruction->Flags & RecInstrFlagPPCInReg))
					RegCount[x]++;

				//if the output is used and it is not a direct assignment then count it
				if((NextInstruction->OutReg == AssignedRegs[x].PPCRegister) &&
				(NextInstruction->Flags & RecInstrFlagPPCOutReg) && !OutRegLost)
				{
					if(NextInstruction->Instruction == GekkoRecIL(MOVE))
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
		if(RegCount[X86_REG_COUNT] == 0)
		{
			if(FindReg <= PPC_REG_COUNT)
				*FindRegMem = (u32)&ireg.gpr[FindReg];
			*FindCount = RegCount[X86_REG_COUNT];
		}
		else
			*FindRegMem = 0;

		return x;
	}

	//if the register matches the in or out register then don't allow it
	for(x = 0; x < X86_REG_COUNT; x++)
	{
		if(((AssignedRegs[x].PPCRegister != Instruction->InReg) ||
			((Instruction->Flags & RecInstrFlagPPCInReg) != RecInstrFlagPPCInReg)) &&
			((AssignedRegs[x].PPCRegister != Instruction->OutReg) ||
			((Instruction->Flags & RecInstrFlagPPCOutReg) != RecInstrFlagPPCOutReg)))
			break;	//no match, use it
	}

	//now find the lowest value and save it, not counting our own register
	MinRegCount = x;

//	MinRegCount = 0;
	for(x=1; x < 4; x++)
	{
		//if a special register is assigned, don't overwrite it
		if(AssignedRegs[x].PPCRegister & REG_SPECIAL)
			continue;

		//if the register matches the in or out register then don't allow it
		if(((AssignedRegs[x].PPCRegister == Instruction->InReg) && (Instruction->Flags & RecInstrFlagPPCInReg)) ||
			((AssignedRegs[x].PPCRegister == Instruction->OutReg) && (Instruction->Flags & RecInstrFlagPPCOutReg)))
			continue;		//if the register matches one we can not override then exit

//		if(x == 4) x++;		//skip esp
		if(RegCount[MinRegCount] > RegCount[x])
			MinRegCount = x;
	}

	//skip esp, x==4
	for(x=5; x < X86_REG_COUNT; x++)
	{
		//if a special register is assigned, don't overwrite it
		if(AssignedRegs[x].PPCRegister & REG_SPECIAL)
			continue;

		//if the register matches the in or out register then don't allow it
		if(((AssignedRegs[x].PPCRegister == Instruction->InReg) && (Instruction->Flags & RecInstrFlagPPCInReg)) ||
			((AssignedRegs[x].PPCRegister == Instruction->OutReg) && (Instruction->Flags & RecInstrFlagPPCOutReg)))
			continue;		//if the register matches one we can not override then exit

//		if(x == 4) x++;		//skip esp
		if(RegCount[MinRegCount] > RegCount[x])
			MinRegCount = x;
	}

	//if we need to find the second lowest register then look it up
	if(FindNextLowest)
	{/*
		MinRegCount2 = MinRegCount;
		for(;;)
		{
			MinRegCount2 = (MinRegCount2 + 1) % X86_REG_COUNT;

			//skip esp
			if(MinRegCount2 == 4)
				continue;

			if(((AssignedRegs[MinRegCount2].PPCRegister != Instruction->InReg) &&
				((Instruction->Flags & RecInstrFlagPPCInReg) == RecInstrFlagPPCInReg)) &&
				((AssignedRegs[MinRegCount2].PPCRegister != Instruction->OutReg) &&
				((Instruction->Flags & RecInstrFlagPPCOutReg) == RecInstrFlagPPCOutReg)))
				break;	//no match, use it
		}
*/
		MinRegCount2 = (MinRegCount + 1) % X86_REG_COUNT;
		for(x=1; x < 4; x++)
		{
			//if a special register is assigned, don't overwrite it
			if(AssignedRegs[x].PPCRegister & REG_SPECIAL)
				continue;

			//if the register matches the in or out register then don't allow it
			if(((AssignedRegs[x].PPCRegister == Instruction->InReg) && (Instruction->Flags & RecInstrFlagPPCInReg)) ||
			   ((AssignedRegs[x].PPCRegister == Instruction->OutReg) && (Instruction->Flags & RecInstrFlagPPCOutReg)))
				continue;		//if the register matches one we can not override then exit

			//if(x == 4) x++;		//skip esp
			if((RegCount[MinRegCount2] > RegCount[x]) && (x != MinRegCount))
				MinRegCount2 = x;
		}

		//skip esp, x==4
		for(x=5; x < X86_REG_COUNT; x++)
		{
			//if a special register is assigned, don't overwrite it
			if(AssignedRegs[x].PPCRegister & REG_SPECIAL)
				continue;

			//if the register matches the in or out register then don't allow it
			if(((AssignedRegs[x].PPCRegister == Instruction->InReg) && (Instruction->Flags & RecInstrFlagPPCInReg)) ||
			   ((AssignedRegs[x].PPCRegister == Instruction->OutReg) && (Instruction->Flags & RecInstrFlagPPCOutReg)))
				continue;		//if the register matches one we can not override then exit

			//if(x == 4) x++;		//skip esp
			if((RegCount[MinRegCount2] > RegCount[x]) && (x != MinRegCount))
				MinRegCount2 = x;
		}
		MinRegCount = MinRegCount2;
	}

	//return how many times the register was found
	*FindCount = RegCount[X86_REG_COUNT];

	//If the count is smaller or equal to the least used register then
	//assign a memory pointer
	if(*FindCount <= RegCount[MinRegCount])
		*FindRegMem = (u32)&ireg.gpr[FindReg];
	else
		*FindRegMem = 0;

	//return the register to replace
	return MinRegCount;
}

X86AssignRegsOp(InVal_OutMem)
{
	X86AssignRegs(InVal)(Instruction, AssignedRegs, FPURegs);
	Instruction->X86OutVal = Instruction->OutMem;
	Instruction->Flags = (Instruction->Flags & ~RecInstrFlagX86_OUTMASK) | RecInstrFlagX86OutVal;
}

X86AssignRegsOp(InReg_OutMem)
{
	X86AssignRegs(InReg)(Instruction, AssignedRegs, FPURegs);
	Instruction->X86OutVal = Instruction->OutMem;
	Instruction->Flags = (Instruction->Flags & ~RecInstrFlagX86_OUTMASK) | RecInstrFlagX86OutVal;
}

X86AssignRegsOp(InReg_Special)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		Reg;
	u32		RegMemPtr = 0;
	u32		FindCount = 0;
	u8		RegFlag = 0;

	//find the in register
	for(x=0; x < X86_REG_COUNT; x++)
	{
		if(Instruction->InReg == AssignedRegs[x].PPCRegister)
		{
			Reg = x;
			RegFlag = 1;
			break;
		}
	}

	//figure out what registers we have
	switch(RegFlag)
	{
		case 0:		//register is in memory
			//find a replacable register for in
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg,
						&RegMemPtr, &FindCount, 0);

			//if we have a pointer, or the register is not used again
			//then indicate it stays in memory
			if(RegMemPtr && (FindCount == 0) &&
			   ((Instruction->InReg & REG_PPC_SPECIALMASK) != REG_PPC_PC))
				RegFlag = 1;

			//find out what combo we need to handle
			switch(RegFlag)
			{
				case 0:			//need to save in to a register
					//create a new instruction to read the ppc register into an x86 register
					NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					memset(NextInstruction, 0, sizeof(RecInstruction));
					NextInstruction->Instruction = GekkoRecIL(MOVE);

					switch(Instruction->InReg & REG_PPC_SPECIALMASK)
					{
						case REG_PPC_SPR:
							NextInstruction->X86InMemory = (u32)&ireg.spr[Instruction->InReg & ~REG_PPC_SPR];
							break;

						case REG_PPC_SR:
							NextInstruction->X86InMemory = (u32)&ireg.sr[Instruction->InReg & ~REG_PPC_SR];
							break;

						case REG_PPC_MSR:
							NextInstruction->X86InMemory = (u32)&ireg.MSR;
							break;

						case REG_PPC_TBL:
							NextInstruction->X86InMemory = (u32)&ireg.TBR.TBL;
							break;

						case REG_PPC_TBU:
							NextInstruction->X86InMemory = (u32)&ireg.TBR.TBU;
							break;

						case REG_PPC_CR:
							NextInstruction->X86InMemory = (u32)&CR[Instruction->InReg & ~REG_PPC_CR];
							break;

						case REG_PPC_PC:
							NextInstruction->X86InMemory = (u32)&ireg.PC;
							break;

						case REG_PPC_XER_WORDINDEX:
							NextInstruction->X86InMemory = (u32)&XER_WORDINDEX;
							break;

						case REG_PPC_XER_CARRY:
							NextInstruction->X86InMemory = (u32)&XER_CARRY;
							break;

						case REG_PPC_XER_OVERFLOW:
							NextInstruction->X86InMemory = (u32)&XER_OVERFLOW;
							break;

						case REG_PPC_XER_SUMMARYOVERFLOW:
							NextInstruction->X86InMemory = (u32)&XER_SUMMARYOVERFLOW;
							break;
					}
					NextInstruction->X86OutReg = MinReg;
					NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
											 RecInstrFlag_NoPPCRegCount;

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

					//assign the registers to the instruction
					AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
					AssignedRegs[MinReg].ValueChanged = 1;
					Instruction->X86InReg = MinReg;
					Instruction->X86OutReg = MinReg;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
					break;

				case 1:			//need to leave in in memory
					//assign the registers to the instruction
					switch(Instruction->InReg & REG_PPC_SPECIALMASK)
					{
						case REG_PPC_SPR:
							Instruction->X86InMemory = (u32)&ireg.spr[Instruction->InReg & ~REG_PPC_SPR];
							break;

						case REG_PPC_SR:
							Instruction->X86InMemory = (u32)&ireg.sr[Instruction->InReg & ~REG_PPC_SR];
							break;

						case REG_PPC_MSR:
							Instruction->X86InMemory = (u32)&ireg.MSR;
							break;

						case REG_PPC_TBL:
							Instruction->X86InMemory = (u32)&ireg.TBR.TBL;
							break;

						case REG_PPC_TBU:
							Instruction->X86InMemory = (u32)&ireg.TBR.TBU;
							break;

						case REG_PPC_CR:
							Instruction->X86InMemory = (u32)&CR[Instruction->InReg & ~REG_PPC_CR];
							break;

						case REG_PPC_PC:
							Instruction->X86InMemory = (u32)&ireg.PC;
							break;

						case REG_PPC_XER_WORDINDEX:
							Instruction->X86InMemory = (u32)&XER_WORDINDEX;
							break;

						case REG_PPC_XER_CARRY:
							Instruction->X86InMemory = (u32)&XER_CARRY;
							break;

						case REG_PPC_XER_OVERFLOW:
							Instruction->X86InMemory = (u32)&XER_OVERFLOW;
							break;

						case REG_PPC_XER_SUMMARYOVERFLOW:
							Instruction->X86InMemory = (u32)&XER_SUMMARYOVERFLOW;
							break;
					}
					Instruction->X86OutMemory = Instruction->X86InMemory;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InMem | RecInstrFlagX86OutMem;
					break;
			};

			break;

		case 1:		//have register in a register
			Instruction->X86InReg = Reg;
			Instruction->X86OutReg = Reg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			break;
	}
}

X86AssignRegsOp(InReg)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		Reg;
	u32		RegMemPtr = 0;
	u32		FindCount = 0;
	u8		RegFlag = 0;

	//if a direct register then just assign
	if(Instruction->InReg & REG_SPECIAL)
	{
		Reg = Instruction->InReg & 0x1F;
		RegFlag = 1;
	}
	else
	{
		//if a special register then handle it
		if((Instruction->InReg | Instruction->OutReg) & REG_PPC_SPECIAL)
		{
			X86AssignRegs(InReg_Special)(Instruction, AssignedRegs, 0);
			return;
		}

		//find the in register
		for(x=0; x < X86_REG_COUNT; x++)
		{
			if(Instruction->InReg == AssignedRegs[x].PPCRegister)
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
		case 0:		//register is in memory
			//find a replacable register for in
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg,
						&RegMemPtr, &FindCount, 0);

			//if we have a pointer, or the register is not used again
			//then indicate it stays in memory
			if(RegMemPtr && (FindCount == 0))
				RegFlag = 1;

			//find out what combo we need to handle
			switch(RegFlag)
			{
				case 0:			//need to save in to a register
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

					//assign the registers to the instruction
					AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
					AssignedRegs[MinReg].ValueChanged = 0;
					Instruction->X86InReg = MinReg;
					Instruction->X86OutReg = MinReg;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
					break;

				case 1:			//need to leave in in memory
					//assign the registers to the instruction
					Instruction->X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
					Instruction->X86OutMemory = (u32)&ireg.gpr[Instruction->InReg];
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InMem | RecInstrFlagX86OutMem;
					break;
			};

			break;

		case 1:		//have register in a register
			Instruction->X86InReg = Reg;
			Instruction->X86OutReg = Reg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			break;
	}
}

X86AssignRegsOp(OutReg)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		Reg;
	u32		RegMemPtr = 0;
	u32		FindCount = 0;
	u8		RegFlag = 0;

	//if a special register then just assign
	if(Instruction->OutReg & REG_SPECIAL)
	{
		Reg = Instruction->OutReg & 0x1F;
		RegFlag = 1;
	}
	else
	{
		//find the in register
		for(x=0; x < X86_REG_COUNT; x++)
		{
			if(Instruction->OutReg == AssignedRegs[x].PPCRegister)
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
		case 0:		//register is in memory
			//find a replacable register for in
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->OutReg,
						&RegMemPtr, &FindCount, 0);

			//if we have a pointer, or the register is not used again
			//then indicate it stays in memory
			if(RegMemPtr && (FindCount == 0) && (!(Instruction->Flags & RecInstrFlagX86InMem)))
				RegFlag = 1;

			//find out what combo we need to handle
			switch(RegFlag)
			{
				case 0:			//need to save out to a register
					//create an instruction to store the out register we no longer will use
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
						NextInstruction->Prev = Instruction->Prev;
						NextInstruction->Prev->Next = NextInstruction;
						NextInstruction->Next = Instruction;
						Instruction->Prev = NextInstruction;
					}

					if(!(Instruction->Flags & RecInstrFlag_KnownOutValue))
					{
						//create a new instruction to read the ppc register into an x86 register
						NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction, 0, sizeof(RecInstruction));
						NextInstruction->Instruction = GekkoRecIL(MOVE);
						NextInstruction->X86InMemory = (u32)&ireg.gpr[Instruction->OutReg];
						NextInstruction->X86OutReg = MinReg;
						NextInstruction->InReg = Instruction->OutReg;
						NextInstruction->OutReg = Instruction->OutReg;
						NextInstruction->Flags |= RecInstrFlagX86InMem | RecInstrFlagX86OutReg |
												RecInstrFlag_NoPPCRegCount  | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

						NextInstruction->Prev = Instruction->Prev;
						NextInstruction->Prev->Next = NextInstruction;
						NextInstruction->Next = Instruction;
						Instruction->Prev = NextInstruction;

					}

					//assign the registers to the instruction
					AssignedRegs[MinReg].PPCRegister = Instruction->OutReg;
					AssignedRegs[MinReg].ValueChanged = 0;
					Instruction->X86OutReg = MinReg;
					Instruction->Flags |= RecInstrFlagX86OutReg;
					break;

				case 1:			//need to leave in in memory
					//assign the registers to the instruction
					Instruction->X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
					Instruction->Flags |= RecInstrFlagX86OutMem;
					break;
			};

			break;

		case 1:		//have register in a register
			Instruction->X86OutReg = Reg;
			Instruction->Flags |= RecInstrFlagX86OutReg;
			break;
	}
}

X86AssignRegsOp(InReg_OutReg_Special)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		Reg;
	u32		RegMemPtr = 0;
	u32		FindCount = 0;
	u8		RegFlag = 0;

	//see which register is special
	if(Instruction->InReg & REG_PPC_SPECIAL)
	{
		//in is special, find the out
		for(x=0; x < X86_REG_COUNT; x++)
		{
			if(Instruction->OutReg == AssignedRegs[x].PPCRegister)
			{
				Reg = x;
				RegFlag |= 2;
				break;
			}
		}

	}
	else
	{
		for(x=0; x < X86_REG_COUNT; x++)
		{
			if(Instruction->InReg == AssignedRegs[x].PPCRegister)
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
		case 0:		//both registers are in memory
			//find a replacable register for the non special
			if(Instruction->InReg & REG_PPC_SPECIAL)
			{
				MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->OutReg,
							&RegMemPtr, &FindCount, 0);
				RegFlag = 0;
			}
			else
			{
				MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg,
							&RegMemPtr, &FindCount, 0);
				RegFlag = 1;
			}

			//find out what combo we need to handle
			switch(RegFlag)
			{
				case 0:			//need to save out to register, leave in in memory
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
						NextInstruction->Prev = NextInstruction;
					}

					//if this is not a move then the out register needs to already
					//have it's value as it is being modified
					if(Instruction->Instruction != GekkoRecIL(MOVE))
					{
						//read a value into the out register
						NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction, 0, sizeof(RecInstruction));
						NextInstruction->Instruction = GekkoRecIL(MOVE);
						NextInstruction->X86OutReg = MinReg;
						NextInstruction->X86InMemory = (u32)&ireg.gpr[Instruction->OutReg];
						NextInstruction->InReg = Instruction->OutReg;
						NextInstruction->OutReg = Instruction->OutReg;
						NextInstruction->Flags |= RecInstrFlagX86OutReg |
												 RecInstrFlagX86InMem |
												 RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

						//update our pointers
						NextInstruction->Next = Instruction;
						NextInstruction->Prev = Instruction->Prev;
						Instruction->Prev->Next = NextInstruction;
						NextInstruction->Prev = NextInstruction;
					}

					//assign the registers to the instruction
					AssignedRegs[MinReg].PPCRegister = Instruction->OutReg;
					AssignedRegs[MinReg].ValueChanged = 1;

					switch(Instruction->InReg & REG_PPC_SPECIALMASK)
					{
						case REG_PPC_SPR:
							Instruction->X86InMemory = (u32)&ireg.spr[Instruction->InReg & ~REG_PPC_SPR];
							break;

						case REG_PPC_SR:
							Instruction->X86InMemory = (u32)&ireg.sr[Instruction->InReg & ~REG_PPC_SR];
							break;

						case REG_PPC_MSR:
							Instruction->X86InMemory = (u32)&ireg.MSR;
							break;

						case REG_PPC_TBL:
							Instruction->X86InMemory = (u32)&ireg.TBR.TBL;
							break;

						case REG_PPC_TBU:
							Instruction->X86InMemory = (u32)&ireg.TBR.TBU;
							break;

						case REG_PPC_CR:
							Instruction->X86InMemory = (u32)&CR[Instruction->InReg & ~REG_PPC_CR];
							break;

						case REG_PPC_PC:
							Instruction->X86InMemory = (u32)&ireg.PC;
							break;

						case REG_PPC_XER_WORDINDEX:
							Instruction->X86InMemory = (u32)&XER_WORDINDEX;
							break;

						case REG_PPC_XER_CARRY:
							Instruction->X86InMemory = (u32)&XER_CARRY;
							break;

						case REG_PPC_XER_OVERFLOW:
							Instruction->X86InMemory = (u32)&XER_OVERFLOW;
							break;

						case REG_PPC_XER_SUMMARYOVERFLOW:
							Instruction->X86InMemory = (u32)&XER_SUMMARYOVERFLOW;
							break;
					}
					Instruction->X86OutReg = MinReg;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
					break;

				case 1:			//need to save in to register, leave out in memory
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

					switch(Instruction->OutReg & REG_PPC_SPECIALMASK)
					{
						case REG_PPC_SPR:
							Instruction->X86OutMemory = (u32)&ireg.spr[Instruction->OutReg & ~REG_PPC_SPR];
							break;

						case REG_PPC_SR:
							Instruction->X86OutMemory = (u32)&ireg.sr[Instruction->OutReg & ~REG_PPC_SR];
							break;

						case REG_PPC_MSR:
							Instruction->X86OutMemory = (u32)&ireg.MSR;
							break;

						case REG_PPC_TBL:
							Instruction->X86OutMemory = (u32)&ireg.TBR.TBL;
							break;

						case REG_PPC_TBU:
							Instruction->X86OutMemory = (u32)&ireg.TBR.TBU;
							break;

						case REG_PPC_CR:
							Instruction->X86OutMemory = (u32)&CR[Instruction->OutReg & ~REG_PPC_CR];
							break;

						case REG_PPC_PC:
							Instruction->X86OutMemory = (u32)&ireg.PC;
							break;

						case REG_PPC_BRANCH:
							Instruction->X86OutMemory = (u32)&branch;
							break;

						case REG_PPC_XER_WORDINDEX:
							Instruction->X86OutMemory = (u32)&XER_WORDINDEX;
							break;

						case REG_PPC_XER_CARRY:
							Instruction->X86OutMemory = (u32)&XER_CARRY;
							break;

						case REG_PPC_XER_OVERFLOW:
							Instruction->X86OutMemory = (u32)&XER_OVERFLOW;
							break;

						case REG_PPC_XER_SUMMARYOVERFLOW:
							Instruction->X86OutMemory = (u32)&XER_SUMMARYOVERFLOW;
							break;
					}

					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
					break;
			};

			break;

		case 1:		//have in register, out is in memory, update our entries
			Instruction->X86InReg = Reg;

			switch(Instruction->OutReg & REG_PPC_SPECIALMASK)
			{
				case REG_PPC_SPR:
					Instruction->X86OutMemory = (u32)&ireg.spr[Instruction->OutReg & ~REG_PPC_SPR];
					break;

				case REG_PPC_SR:
					Instruction->X86OutMemory = (u32)&ireg.sr[Instruction->OutReg & ~REG_PPC_SR];
					break;

				case REG_PPC_MSR:
					Instruction->X86OutMemory = (u32)&ireg.MSR;
					break;

				case REG_PPC_TBL:
					Instruction->X86OutMemory = (u32)&ireg.TBR.TBL;
					break;

				case REG_PPC_TBU:
					Instruction->X86OutMemory = (u32)&ireg.TBR.TBU;
					break;

				case REG_PPC_CR:
					Instruction->X86OutMemory = (u32)&CR[Instruction->OutReg & ~REG_PPC_CR];
					break;

				case REG_PPC_PC:
					Instruction->X86OutMemory = (u32)&ireg.PC;
					break;

				case REG_PPC_BRANCH:
					Instruction->X86OutMemory = (u32)&branch;
					break;

				case REG_PPC_XER_WORDINDEX:
					Instruction->X86OutMemory = (u32)&XER_WORDINDEX;
					break;

				case REG_PPC_XER_CARRY:
					Instruction->X86OutMemory = (u32)&XER_CARRY;
					break;

				case REG_PPC_XER_OVERFLOW:
					Instruction->X86OutMemory = (u32)&XER_OVERFLOW;
					break;

				case REG_PPC_XER_SUMMARYOVERFLOW:
					Instruction->X86OutMemory = (u32)&XER_SUMMARYOVERFLOW;
					break;
		}

			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
			break;

		case 2:		//have out register, in is in memory
			switch(Instruction->InReg & REG_PPC_SPECIALMASK)
			{
				case REG_PPC_SPR:
					Instruction->X86InMemory = (u32)&ireg.spr[Instruction->InReg & ~REG_PPC_SPR];
					break;

				case REG_PPC_SR:
					Instruction->X86InMemory = (u32)&ireg.sr[Instruction->InReg & ~REG_PPC_SR];
					break;

				case REG_PPC_MSR:
					Instruction->X86InMemory = (u32)&ireg.MSR;
					break;

				case REG_PPC_TBL:
					Instruction->X86InMemory = (u32)&ireg.TBR.TBL;
					break;

				case REG_PPC_TBU:
					Instruction->X86InMemory = (u32)&ireg.TBR.TBU;
					break;

				case REG_PPC_CR:
					Instruction->X86InMemory = (u32)&CR[Instruction->InReg & ~REG_PPC_CR];
					break;

				case REG_PPC_PC:
					Instruction->X86InMemory = (u32)&ireg.PC;
					break;

				case REG_PPC_XER_WORDINDEX:
					Instruction->X86InMemory = (u32)&XER_WORDINDEX;
					break;

				case REG_PPC_XER_CARRY:
					Instruction->X86InMemory = (u32)&XER_CARRY;
					break;

				case REG_PPC_XER_OVERFLOW:
					Instruction->X86InMemory = (u32)&XER_OVERFLOW;
					break;

				case REG_PPC_XER_SUMMARYOVERFLOW:
					Instruction->X86InMemory = (u32)&XER_SUMMARYOVERFLOW;
					break;
			}
			Instruction->X86OutReg = Reg;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
			AssignedRegs[Reg].ValueChanged = 1;
			break;
	}
}

X86AssignRegsOp(InReg_OutReg)
{
	RecInstruction *NextInstruction;
	RecInstruction *NextInstruction2;
	u32		x;
	u32		MinReg = 0;
	u32		MinReg2 = 0;
	u32		Reg[2];
	u32		RegMemPtr = 0;
	u32		RegMemPtr2 = 0;
	u32		FindCount = 0;
	u32		FindCount2 = 0;
	u8		RegFlag = 0;

	if(Instruction->InReg & REG_SPECIAL)
	{
		Reg[0] = Instruction->InReg & 0x1F;
		RegFlag = 1;
	}

	if(Instruction->OutReg & REG_SPECIAL)
	{
		Reg[1] = Instruction->OutReg & 0x1F;
		RegFlag |= 2;
	}

	//if a special register then handle it
	if((Instruction->InReg | Instruction->OutReg) & REG_PPC_SPECIAL)
	{
		X86AssignRegs(InReg_OutReg_Special)(Instruction, AssignedRegs, 0);
		return;
	}

	//if not both special registers then find the in and/or out registers
	if(RegFlag != 3)
	{
		for(x=0; x < X86_REG_COUNT; x++)
		{
			if(Instruction->InReg == AssignedRegs[x].PPCRegister)
			{
				Reg[0] = x;
				RegFlag |= 1;
				if(RegFlag == 3) break;
			}
			if(Instruction->OutReg == AssignedRegs[x].PPCRegister)
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
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg,
						&RegMemPtr, &FindCount, 0);

			//find a replacable register for out
			if((Instruction->InReg != Instruction->OutReg) && (Instruction->Instruction != GekkoRecIL(MOVE)))
			{
				MinReg2 = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->OutReg,
							&RegMemPtr2, &FindCount2, 1);

				//if we have 1 pointer and not the other, or the register is not used again
				//then indicate it stays in memory
				if(RegMemPtr && ((RegMemPtr2 == 0) || (FindCount == 0)))
					RegFlag = 1;
				if(RegMemPtr2 && ((RegMemPtr == 0) || (FindCount2 == 0)) && !(Instruction->Flags & RecInstrFlag_KnownOutValue))
					RegFlag |= 2;
			}
			else
				RegFlag = 4;

			//find out what combo we need to handle
			switch(RegFlag)
			{
				case 0:			//need to save both to registers
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

					if(!(Instruction->Flags & RecInstrFlag_KnownOutValue))
					{
						NextInstruction->Next = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction->Next, 0, sizeof(RecInstruction));
						NextInstruction->Next->Instruction = GekkoRecIL(MOVE);
						NextInstruction->Next->X86InMemory = (u32)&ireg.gpr[Instruction->OutReg];
						NextInstruction->Next->X86OutReg = MinReg2;
						NextInstruction->Next->InReg = Instruction->OutReg;
						NextInstruction->Next->OutReg = Instruction->OutReg;
						NextInstruction->Next->Flags |= RecInstrFlagX86InMem |
													RecInstrFlagX86OutReg |
													RecInstrFlag_NoPPCRegCount |
													RecInstrFlagPPCInReg |
													RecInstrFlagPPCOutReg;
						NextInstruction->Next->Prev = NextInstruction;
					}

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
					if((AssignedRegs[MinReg2].PPCRegister != -1) && AssignedRegs[MinReg2].ValueChanged)
					{
						NextInstruction2 = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction2, 0, sizeof(RecInstruction));
						NextInstruction2->Instruction = GekkoRecIL(MOVE);
						NextInstruction2->X86InReg = MinReg2;
						NextInstruction2->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg2].PPCRegister];
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
					AssignedRegs[MinReg2].PPCRegister = Instruction->OutReg;
					AssignedRegs[MinReg2].ValueChanged = 0;
					Instruction->X86InReg = MinReg;
					Instruction->X86OutReg = MinReg2;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
					break;

				case 1:			//need to save out to register, leave in in memory
					//create a new instruction to store the register we no longer will use
					if((AssignedRegs[MinReg2].PPCRegister != -1) && AssignedRegs[MinReg2].ValueChanged)
					{
						NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction, 0, sizeof(RecInstruction));
						NextInstruction->Instruction = GekkoRecIL(MOVE);
						NextInstruction->X86InReg = MinReg2;
						NextInstruction->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[MinReg2].PPCRegister];
						NextInstruction->Flags |= RecInstrFlagX86InReg |
												 RecInstrFlagX86OutMem |
												 RecInstrFlag_NoPPCRegCount;

						//update our pointers
						NextInstruction->Next = Instruction;
						NextInstruction->Prev = Instruction->Prev;
						Instruction->Prev->Next = NextInstruction;
						NextInstruction->Prev = NextInstruction;
					}

					//if this is not a move then the out register needs to already
					//have it's value as it is being modified
					if(Instruction->Instruction != GekkoRecIL(MOVE) && !(Instruction->Flags & RecInstrFlag_KnownOutValue))
					{
						//read a value into the out register
						NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction, 0, sizeof(RecInstruction));
						NextInstruction->Instruction = GekkoRecIL(MOVE);
						NextInstruction->X86OutReg = MinReg2;
						NextInstruction->X86InMemory = (u32)&ireg.gpr[Instruction->OutReg];
						NextInstruction->InReg = Instruction->OutReg;
						NextInstruction->OutReg = Instruction->OutReg;
						NextInstruction->Flags |= RecInstrFlagX86OutReg |
												 RecInstrFlagX86InMem |
												 RecInstrFlag_NoPPCRegCount |
												 RecInstrFlagPPCInReg |
												 RecInstrFlagPPCOutReg;

						//update our pointers
						NextInstruction->Next = Instruction;
						NextInstruction->Prev = Instruction->Prev;
						Instruction->Prev->Next = NextInstruction;
						NextInstruction->Prev = NextInstruction;
					}

					//assign the registers to the instruction
					AssignedRegs[MinReg2].PPCRegister = Instruction->OutReg;
					AssignedRegs[MinReg2].ValueChanged = 0;
					Instruction->X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
					Instruction->X86OutReg = MinReg2;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
					break;

				case 2:			//need to save in to register, leave out in memory
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
					Instruction->X86OutMemory = (u32)&ireg.gpr[Instruction->OutReg];
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
					break;

				case 3:			//need to leave both in memory so temp register
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
					if(AssignedRegs[MinReg].PPCRegister != -1)
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
					Instruction->X86InReg = MinReg;
					Instruction->X86OutMemory = (u32)&ireg.gpr[Instruction->OutReg];
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InReg | RecInstrFlagX86OutMem;

					//move the instruction back in now
					if(AssignedRegs[MinReg].PPCRegister != -1)
					{
						NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction, 0, sizeof(RecInstruction));
						NextInstruction->Instruction = GekkoRecIL(MOVE);
						NextInstruction->X86InMemory = (u32)&ireg.gpr[AssignedRegs[MinReg].PPCRegister];
						NextInstruction->X86OutReg = MinReg;
						NextInstruction->InReg = AssignedRegs[MinReg].PPCRegister;
						NextInstruction->OutReg = AssignedRegs[MinReg].PPCRegister;
						NextInstruction->Flags |= RecInstrFlagX86InMem |
												 RecInstrFlagX86OutReg |
												 RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

						NextInstruction->Prev = Instruction;
						NextInstruction->Next = Instruction->Next;
						if(Instruction->Next)
							Instruction->Next->Prev = NextInstruction;
						Instruction->Next = NextInstruction;
						LastInstruction = NextInstruction;
					}

					break;

				case 4:		//in and out are the same register, just read it in
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
						NextInstruction->Prev = NextInstruction;
					}

					//assign the registers to the instruction
					AssignedRegs[MinReg].PPCRegister = Instruction->OutReg;
					AssignedRegs[MinReg].ValueChanged = 0;
					Instruction->X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
					Instruction->X86OutReg = MinReg;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
					break;

			};

			break;

		case 1:		//have in register, out is in memory, move out to a register
			//find a replacable register
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->OutReg,
					  &RegMemPtr, &FindCount, 0);

			//if we have a memory pointer, use it
			if(RegMemPtr)
			{
				//out to a memory pointer
				Instruction->X86InReg = Reg[0];
				Instruction->X86OutMemory = RegMemPtr;
				Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
										RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
			}
			else
			{
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

				//create a new instruction to read in the out value
				//if this is not a move
				if((Instruction->Instruction != GekkoRecIL(MOVE)) && !(Instruction->Flags & RecInstrFlag_KnownOutValue))
				{
					NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					memset(NextInstruction, 0, sizeof(RecInstruction));
					NextInstruction->Instruction = GekkoRecIL(MOVE);
					NextInstruction->X86OutReg = MinReg;
					NextInstruction->X86InMemory = (u32)&ireg.gpr[Instruction->OutReg];
					NextInstruction->InReg = Instruction->OutReg;
					NextInstruction->OutReg = Instruction->OutReg;
					NextInstruction->Flags |= RecInstrFlagX86OutReg |
											 RecInstrFlagX86InMem |
											 RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

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
										RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			}
			break;

		case 2:		//have out register, in is in memory, move in to a register
			//find a replacable register
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg,
					  &RegMemPtr, &FindCount, 0);

			//if we have a memory pointer, use it
			if(RegMemPtr)
			{
				//out to a memory pointer
				Instruction->X86InMemory = RegMemPtr;
				Instruction->X86OutReg = Reg[1];
				Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
										RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
			}
			else
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
										RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			}
			break;

		case 3:		//have registers in registers
			Instruction->X86InReg = Reg[0];
			Instruction->X86OutReg = Reg[1];
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
			break;
	}
}

X86AssignRegsOp(InVal)
{
	//move the value from ppc to x86
	Instruction->X86InVal = Instruction->InVal;
	Instruction->Flags |= RecInstrFlagX86InVal;
}

X86AssignRegsOp(InVal_OutReg_Special)
{
	switch(Instruction->OutReg & REG_PPC_SPECIALMASK)
	{
		case REG_PPC_SPR:
			Instruction->X86OutMemory = (u32)&ireg.spr[Instruction->OutReg & ~REG_PPC_SPR];
			break;

		case REG_PPC_SR:
			Instruction->X86OutMemory = (u32)&ireg.sr[Instruction->OutReg & ~REG_PPC_SR];
			break;

		case REG_PPC_MSR:
			Instruction->X86OutMemory = (u32)&ireg.MSR;
			break;

		case REG_PPC_TBL:
			Instruction->X86OutMemory = (u32)&ireg.TBR.TBL;
			break;

		case REG_PPC_TBU:
			Instruction->X86OutMemory = (u32)&ireg.TBR.TBU;
			break;

		case REG_PPC_CR:
			Instruction->X86OutMemory = (u32)&CR[Instruction->OutReg & ~REG_PPC_CR];
			break;

		case REG_PPC_PC:
			Instruction->X86OutMemory = (u32)&ireg.PC;
			break;

		case REG_PPC_BRANCH:
			Instruction->X86OutMemory = (u32)&branch;
			break;

		case REG_PPC_XER_WORDINDEX:
			Instruction->X86OutMemory = (u32)&XER_WORDINDEX;
			break;

		case REG_PPC_XER_CARRY:
			Instruction->X86OutMemory = (u32)&XER_CARRY;
			break;

		case REG_PPC_XER_OVERFLOW:
			Instruction->X86OutMemory = (u32)&XER_OVERFLOW;
			break;

		case REG_PPC_XER_SUMMARYOVERFLOW:
			Instruction->X86OutMemory = (u32)&XER_SUMMARYOVERFLOW;
			break;
	}

	Instruction->X86InVal = Instruction->InVal;
	Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
							RecInstrFlagX86OutMem | RecInstrFlagX86InVal;
}

X86AssignRegsOp(InVal_OutReg)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		Reg;
	u32		RegMemPtr = 0;
	u32		FindCount = 0;
	u8		RegFlag = 0;

	if(Instruction->Flags & RecInstrFlag_OutSpecialCase)
	{
		//we have a special command that can't accept a value as an input, div for instance
		//find a register to use

		//find a replacable register for in
		MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg,
					&RegMemPtr, &FindCount, 0);

		//create an instruction to store the current register we no longer will use
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

			NextInstruction->Next = Instruction;
			NextInstruction->Prev = Instruction->Prev;
			NextInstruction->Prev->Next = NextInstruction;
			Instruction->Prev = NextInstruction;
		}

		//if this is not a move already then we need to move the register value
		//into the out register
		if(!(Instruction->Flags & RecInstrFlag_KnownOutValue))
		{
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVE);
			NextInstruction->X86OutReg = MinReg;
			NextInstruction->InReg = Instruction->InReg;
			NextInstruction->OutReg = Instruction->InReg;
			NextInstruction->Flags |= RecInstrFlagX86InVal |
									 RecInstrFlagX86OutReg |
									 RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_KnownInValue;

			NextInstruction->Next = Instruction;
			NextInstruction->Prev = Instruction->Prev;
			NextInstruction->Prev->Next = NextInstruction;
			Instruction->Prev = NextInstruction;
		}

		//assign the registers to the instruction
		AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
		AssignedRegs[MinReg].ValueChanged = 0;
		Instruction->X86InReg = MinReg;
		Instruction->Flags = (Instruction->Flags & ~(RecInstrFlagX86_MASK | RecInstrFlag_KnownInValue)) |
								RecInstrFlagX86InReg;

		return;
	}

	if(Instruction->OutReg & REG_PPC_SPECIAL)
	{
		X86AssignRegs(InVal_OutReg_Special)(Instruction, AssignedRegs, 0);
		return;
	}

	//if a special register then just assign
	if(Instruction->OutReg & REG_SPECIAL)
	{
		Reg = Instruction->OutReg & 0x1F;
		RegFlag = 1;
	}
	else
	{
		//find the out register
		for(x=0; x < X86_REG_COUNT; x++)
		{
			if(Instruction->OutReg == AssignedRegs[x].PPCRegister)
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
		case 0:		//register is in memory
			//find a replacable register for out
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->OutReg,
						&RegMemPtr, &FindCount, 0);

			//if we have a pointer, or the register is not used again
			//then indicate it stays in memory
			if(RegMemPtr && (FindCount == 0))
				RegFlag = 1;

			//find out what combo we need to handle
			switch(RegFlag)
			{
				case 0:			//need to save out to a register
					//create an instruction to store the current register we no longer will use
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

						NextInstruction->Next = Instruction;
						NextInstruction->Prev = Instruction->Prev;
						NextInstruction->Prev->Next = NextInstruction;
						Instruction->Prev = NextInstruction;
					}

					//we need to move the register value into the out register
					if(!(Instruction->Flags & RecInstrFlag_KnownOutValue))
					{
						NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
						memset(NextInstruction, 0, sizeof(RecInstruction));
						NextInstruction->Instruction = GekkoRecIL(MOVE);
						NextInstruction->X86InMemory = (u32)&ireg.gpr[Instruction->OutReg];
						NextInstruction->X86OutReg = MinReg;
						NextInstruction->InReg = Instruction->OutReg;
						NextInstruction->OutReg = Instruction->OutReg;
						NextInstruction->Flags |= RecInstrFlagX86InMem |
												 RecInstrFlagX86OutReg |
												 RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;

						NextInstruction->Next = Instruction;
						NextInstruction->Prev = Instruction->Prev;
						NextInstruction->Prev->Next = NextInstruction;
						Instruction->Prev = NextInstruction;
					}

					//assign the registers to the instruction
					AssignedRegs[MinReg].PPCRegister = Instruction->OutReg;
					AssignedRegs[MinReg].ValueChanged = 0;
					Instruction->X86OutReg = MinReg;
					Instruction->X86InVal = Instruction->InVal;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86OutReg | RecInstrFlagX86InVal;
					break;

				case 1:			//need to leave out in memory
					//assign the registers to the instruction
					Instruction->X86OutMemory = (u32)&ireg.gpr[Instruction->OutReg];
					Instruction->X86InVal = Instruction->InVal;
					Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
											RecInstrFlagX86OutMem | RecInstrFlagX86InVal;
					break;
			};

			break;

		case 1:		//have register
			Instruction->X86OutReg = Reg;
			Instruction->X86InVal = Instruction->InVal;
			Instruction->Flags |= (Instruction->Flags & ~RecInstrFlagX86_MASK) |
									RecInstrFlagX86OutReg | RecInstrFlagX86InVal;
			break;
	}
}

X86AssignRegsOp(NONE)
{
	RecInstruction *NextInstruction;
	u32		x;
	u32		MinReg = 0;
	u32		RegMemPtr = 0;
	u32		FindCount = 0;

	//see if we need to handle a temporary register
	if(Instruction->Instruction == GekkoRecIL(GETTEMPREG))
	{
		//find an available register
		MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, TEMP_REG,
					&RegMemPtr, &FindCount, 0);

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
			NextInstruction->Prev = NextInstruction;
		}

		//assign our temp reg
		//AssignedRegs[MinReg].PPCRegister = TEMP_REG;
		AssignedRegs[MinReg].PPCRegister = -1;
		AssignedRegs[MinReg].ValueChanged = 0;
	}
	else if(Instruction->Instruction == GekkoRecIL(FREETEMPREG))
	{
		//go thru the register list and mark the temp register as usable
		for(x=0; x < X86_REG_COUNT; x++)
		{
			if(AssignedRegs[x].PPCRegister == TEMP_REG)
			{
				AssignedRegs[x].PPCRegister = -1;
				break;
			}
		}
	}
	else if(Instruction->Instruction == GekkoRecIL(GETTEMPFPUREG))
	{
		//find an available register for fpu
		MinReg = FindReplacableFPUReg(Instruction, FPURegs, TEMP_FPU_REG | Instruction->InReg, &RegMemPtr, &FindCount, 0);

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

		//assign our temp reg
		//FPURegs[MinReg].PPCRegister = TEMP_FPU_REG | Instruction->InReg;
		FPURegs[MinReg].PPCRegister = -1;
		FPURegs[MinReg].ValueChanged = 0;
	}
	else if(Instruction->Instruction == GekkoRecIL(FREETEMPFPUREG))
	{
		//go thru the register list and mark the temp register as usable
		for(x=0; x < FPU_REG_COUNT; x++)
		{
			if(FPURegs[x].PPCRegister == (TEMP_FPU_REG | Instruction->InReg))
			{
				FPURegs[x].PPCRegister = -1;
				break;
			}
		}
	}
	else if(Instruction->Instruction == GekkoRecIL(GETREG))
	{
		//if this is a special setup then select the specified register
		if(Instruction->InReg & REG_SPECIAL)
			MinReg = Instruction->InReg & 0x1F;
		else
		{
			//find an available register
			MinReg = FindReplacableX86Reg(Instruction, AssignedRegs, Instruction->InReg,
						&RegMemPtr, &FindCount, 0);
		}

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
			NextInstruction->Prev = NextInstruction;
		}

		//assign our new reg id
		AssignedRegs[MinReg].PPCRegister = Instruction->InReg;
		AssignedRegs[MinReg].ValueChanged = 0;
	}
	else if(Instruction->Instruction == GekkoRecIL(FREEREG))
	{
		//if special then mark the exact x86 register as available
		if(Instruction->InReg & REG_SPECIAL)
			AssignedRegs[Instruction->InReg & 0x1F].PPCRegister = -1;
		else
		{
			//go thru the register list and mark the register as usable
			for(x=0; x < X86_REG_COUNT; x++)
			{
				if(AssignedRegs[x].PPCRegister == Instruction->InReg)
				{
					AssignedRegs[x].PPCRegister = -1;
					break;
				}
			}
		}
	}
	else if(Instruction->Instruction == GekkoRecIL(STOREREG))
	{
		//if the register is used, then store it
		//note, the in value is a REG_* entry
		if((AssignedRegs[Instruction->InReg].PPCRegister != -1) && AssignedRegs[Instruction->InReg].ValueChanged)
		{
			NextInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
			memset(NextInstruction, 0, sizeof(RecInstruction));
			NextInstruction->Instruction = GekkoRecIL(MOVE);
			NextInstruction->X86InReg = Instruction->InReg;
			NextInstruction->X86OutMemory = (u32)&ireg.gpr[AssignedRegs[Instruction->InReg].PPCRegister];
			NextInstruction->Flags |= RecInstrFlagX86InReg |
										RecInstrFlagX86OutMem |
										RecInstrFlag_NoPPCRegCount;

			//update our pointers
			NextInstruction->Next = Instruction;
			NextInstruction->Prev = Instruction->Prev;
			Instruction->Prev->Next = NextInstruction;
			NextInstruction->Prev = NextInstruction;
		}

		//mark as unused
		AssignedRegs[Instruction->InReg].PPCRegister = -1;
	}
	else if((Instruction->Instruction == GekkoRecIL(UPDATEXERSOOV))		||
			(Instruction->Instruction == GekkoRecIL(UPDATETEMPCARRY))	||
			(Instruction->Instruction == GekkoRecIL(UPDATECARRY))		||
			(Instruction->Instruction == GekkoRecIL(UPDATECARRYWTEMP))	||
			(Instruction->Instruction == GekkoRecIL(ADDCARRY))			||
			(Instruction->Instruction == GekkoRecIL(UPDATECR0)))
	{
		if(Instruction->Prev->Flags & RecInstrFlag_KnownValue)
			Instruction->Flags |= RecInstrFlag_KnownValue;
	}
}

X86AssignRegsOp(KnownPPCValue)
{
}