////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_assembler.cpp
// DESC:		Assembler of the IL opcodes for the recompiler
// CREATED:		Mar. 29, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "cpu_rec_assembler.h"
#include "powerpc/cpu_core_regs.h"

u32 GekkoCPURecompiler::XER_CARRY;
u32 GekkoCPURecompiler::XER_SUMMARYOVERFLOW;
u32 GekkoCPURecompiler::XER_OVERFLOW;
u32 GekkoCPURecompiler::XER_WORDINDEX;
u32 GekkoCPURecompiler::XER_TEMPCARRY;
u32 GekkoCPURecompiler::CR[32];

u32		GekkoCPURecompiler::NextGetRegID_Val;

static u32 Val1 = 1;

//take any IL instruction and output the required asm for
//the instruction

//the function layout is that the right side is applied to the
//left. Example, ADD_REG_MEM. Add what is in memory to the register

void EMU_FASTCALL GekkoCPURecompiler::Unknown_Mask(char *Instr, long Mask)
{
	printf("%s: Unknown Instruction Mask %0.8X\n", Instr, Mask);
	cpu->pause = true;
}

GekkoRecILOp(CDQ)
{
	((u8 *)OutInstruction)[0] = 0x99;	//cdq
	*OutSize = 1;
}

Instruction2Param(ADD,  0x01, 0x03, 0x81, 0)
Instruction2Param(ADC,  0x11, 0x13, 0x81, 2)
Instruction2Param(AND,  0x21, 0x23, 0x81, 4)
Instruction2Param(CMP,  0x39, 0x3B, 0x81, 7)
Instruction1Param(DIV,  0xF7, 6)
Instruction1Param(IDIV, 0xF7, 7)
Instruction1Param(NEG,  0xF7, 3)
Instruction1Param(NOT,  0xF7, 2)
Instruction2Param(OR,   0x09, 0x0B, 0x81, 1)
Instruction2Byte(SETE, 0x940F, 0x940F, 0, 0)
Instruction2Param(SUB,  0x29, 0x2B, 0x81, 5)
Instruction2Param(XOR,  0x31, 0x33, 0x81, 6)
Instruction2Param(XCHG, 0x87, 0x87, 0, 0) 
Instruction2Byte(SETC, 0x920F, 0x920F, 0, 0)

GekkoRecILOp(MOVE)
{
	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
			if(Instruction->X86InReg == Instruction->X86OutReg)
				*OutSize = 0;
			else
			{
				*(u16 *)OutInstruction = (ModRM(0x03, Instruction->X86InReg,
												Instruction->X86OutReg) << 8) | 0x89;
				*OutSize = 2;

				if((Instruction->Flags & RecInstrFlagPPC_MASK) ==
					(RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg))
				{
					if(Instruction->InReg != Instruction->OutReg)
						X86Regs[Instruction->X86OutReg].ValueChanged = 1;
				}
				else
				{
					//not sure what is being moved, mark it as changed
					if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutReg)
						X86Regs[Instruction->X86OutReg].ValueChanged = 1;
				}
			}
			break;

		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
			*(u16 *)OutInstruction = (ModRM(0x00, Instruction->X86InReg, 0x05) << 8) | 0x89;
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
			*OutSize = 6;
			break;

		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
			*(u16 *)OutInstruction = (ModRM(0x00, Instruction->X86OutReg, 0x05) << 8) | 0x8B;
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InMemory;
			if((Instruction->Flags & RecInstrFlagPPC_MASK) ==
				(RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg))
			{
				if(Instruction->InReg != Instruction->OutReg)
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				//not sure what is being moved, mark it as changed
				if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutReg)
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			*OutSize = 6;
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
			*(u16 *)OutInstruction = (ModRM(0x03, 0, Instruction->X86OutReg) << 8) | 0xC7;
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal;
			if((Instruction->Flags & RecInstrFlagPPC_MASK) ==
				(RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg))
			{
				if(Instruction->InReg != Instruction->OutReg)
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				//not sure what is being moved, mark it as changed
				if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutReg)
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			*OutSize = 6;
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			*(u16 *)OutInstruction = (ModRM(0x00, 0, 0x05) << 8) | 0xC7;
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
			*(u32 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal;
			*OutSize = 10;
			break;

		default:
			Unknown_Mask("MOVE", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

GekkoRecILOp(NOP)
{
	*OutSize = 0;
}

GekkoRecILOp(UPDATECRSO)
{
	RecInstruction	TempInst;
	u32				NewSize;

	//mov reg, XER_SUMMARYOVERFLOW
	TempInst.X86OutReg = Instruction->InReg;
	TempInst.X86InMemory = (u32)&XER_SUMMARYOVERFLOW;
	TempInst.Flags = RecInstrFlagX86InMem | RecInstrFlagX86OutReg;
	GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewSize, X86Regs, 0);
	*OutSize = NewSize;

	//mov CR[OVERFLOW], reg
	TempInst.X86InReg = Instruction->InReg;
	TempInst.X86OutMemory = (u32)&CR[(Instruction->TempData * 4) + 3];
	TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
	GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
	*OutSize += NewSize;
}

GekkoRecILOp(UPDATECRSIGNED)
{
	*(u32 *)(&((u8 *)OutInstruction)[0]) = 0x00059C0F;		//SETL
	*(u32 *)(&((u8 *)OutInstruction)[3]) = (u32)(&CR[(Instruction->TempData * 4)]);
	*(u32 *)(&((u8 *)OutInstruction)[7]) = 0x00059F0F;		//SETG
	*(u32 *)(&((u8 *)OutInstruction)[10]) = (u32)(&CR[(Instruction->TempData * 4) + 1]);
	*(u32 *)(&((u8 *)OutInstruction)[14]) = 0x0005940F;		//SETE
	*(u32 *)(&((u8 *)OutInstruction)[17]) = (u32)(&CR[(Instruction->TempData * 4) + 2]);
	*OutSize = 21;
}

GekkoRecILOp(UPDATECRUNSIGNED)
{
	*(u32 *)(&((u8 *)OutInstruction)[0]) = 0x0005920F;		//SETB
	*(u32 *)(&((u8 *)OutInstruction)[3]) = (u32)(&CR[(Instruction->TempData * 4)]);
	*(u32 *)(&((u8 *)OutInstruction)[7]) = 0x0005970F;		//SETA
	*(u32 *)(&((u8 *)OutInstruction)[10]) = (u32)(&CR[(Instruction->TempData * 4) + 1]);
	*(u32 *)(&((u8 *)OutInstruction)[14]) = 0x0005940F;		//SETZ
	*(u32 *)(&((u8 *)OutInstruction)[17]) = (u32)(&CR[(Instruction->TempData * 4) + 2]);
	*OutSize = 21;
}

GekkoRecILOp(UPDATECR0)
{
	long			Pos;
	RecInstruction	*Inst;
	RecInstruction	TempInst;
	u32				NewOutSize;
	u32				FindRegMem;
	u32				FindCount;
	u32				Reg1;
	u32				x;

	//Our code for updating CR0
	*OutSize = 0;
	if(*(u32 *)(SetCRKnown) & 0x80808080)
	{
		for(x = 0; x < 4; x++)
		{
			//mov CR[x], SetCRKnown[x]
			TempInst.X86OutMemory = (u32)&CR[x];
			TempInst.X86InVal = SetCRKnown[x];
			TempInst.Flags = RecInstrFlagX86OutMem | RecInstrFlagX86InVal;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
		}
		*(u32 *)(SetCRKnown) = 0;
	}
	else
	{
		//find a register we can use
		Reg1 = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);

		if((X86Regs[Reg1].PPCRegister != -1) && X86Regs[Reg1].ValueChanged)
		{
			//register returned is in use, move it out to memory
			TempInst.X86InReg = Reg1;
			TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[Reg1].PPCRegister];
			TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
		}
		X86Regs[Reg1].PPCRegister = -1;

		//figure out what compare to do
		Inst = Instruction;
		for(Pos = Inst->OutReg; Pos >= 0; Pos--)
			Inst = Inst->Prev;

		if((Inst->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
		{
			//cmp reg, 0
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 0x07, Inst->X86OutReg) << 8)
													| 0x83;
			Pos = *OutSize + 3;
		}
		else
		{
			//cmp dword [mem], 0
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 0]) = 0x3D83;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Inst->X86OutMemory;
			((u8 *)OutInstruction)[*OutSize + 6] = 0;
			Pos = *OutSize + 7;
		}

		//update CR
		//Instruction->X86InReg = Reg1;
		Instruction->TempData = 0;
		Instruction->InReg = Reg1;		//we no longer care what we were in the Gekko CPU
		GekkoRecIL(UPDATECRSIGNED)(Instruction, &((u8 *)OutInstruction)[Pos], &NewOutSize, X86Regs, 0);
		*OutSize = Pos + NewOutSize;

		//set which SO bit to set
		GekkoRecIL(UPDATECRSO)(Instruction, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
	}
}

GekkoRecILOp(UPDATEXERSOOV)
{
	RecInstruction	TempInst;
	u32				NewOutSize;
	u32				FindRegMem;
	u32				FindCount;
	u32				Reg;

	*OutSize = 0;
	if(*(u16 *)(&SetXERKnown[2]) & 0x8080)
	{
		*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x05C7;	//mov dword [XER_OVERFLOW], val
		*(u32 *)(&((u8 *)OutInstruction)[2]) = (u32)(&XER_OVERFLOW);
		*(u32 *)(&((u8 *)OutInstruction)[6]) = XERKnown[2];
		*(u16 *)(&((u8 *)OutInstruction)[10]) = 0x05C7;	//mov dword [XER_SUMMARYOVERFLOW], val
		*(u32 *)(&((u8 *)OutInstruction)[12]) = (u32)(&XER_SUMMARYOVERFLOW);
		*(u32 *)(&((u8 *)OutInstruction)[16]) = XERKnown[3];
		*(u16 *)(&SetXERKnown[2]) = 0;

		*OutSize = 20;
		return;
	}

	//Our code for updating XER SO/OV
	Reg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
	if((X86Regs[Reg].PPCRegister != -1) && X86Regs[Reg].ValueChanged)
	{
		//register returned is in use, move it out to memory
		TempInst.X86InReg = Reg;
		TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[Reg].PPCRegister];
		TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
		GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
	}
	X86Regs[Reg].PPCRegister = -1;

	//mov dword reg, XER_SUMMARYOVERFLOW
	*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 0]) = (ModRM(0, Reg, 5) << 8) | 0x8B;
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)(&XER_SUMMARYOVERFLOW);

#pragma todo("Could this be done with seto XER_OVERFLOW  seto reg[h]  or reg[h],reg[l]  mov dword XER_SUMMARYOVERFLOW, reg?")

	//seto XER_OVERFLOW
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = 0x0005900F;
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 9]) = (u32)(&XER_OVERFLOW);

	//cmovo reg, Val1
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 13]) = (ModRM(0, Reg, 5) << 16) | 0x400F;
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 16]) = (u32)&Val1;

	//mov dword XER_SUMMARYOVERFLOW, reg
	*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 20]) = (ModRM(0, Reg, 5) << 8) | 0x89;
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 22]) = (u32)(&XER_SUMMARYOVERFLOW);
	*OutSize += 26;
}
GekkoRecILOp(ADDCARRY)
{
	*OutSize = 0;
	if(SetXERKnown[1] & 0x80)
	{
		*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x05C6;		//mov byte [mem], val
		*(u32 *)(&((u8 *)OutInstruction)[2]) = (u32)(&XER_CARRY);
		*(u8 *)(&((u8 *)OutInstruction)[6]) = SetXERKnown[1] & 1;

		//add reg, 0/1
		*(u32 *)(&((u8 *)OutInstruction)[7]) = (ModRM(0x03, 2, Instruction->X86OutReg) << 8)
										| 0x83;
		*(u8 *)(&((u8 *)OutInstruction)[11]) = SetXERKnown[1] & 1;
		SetXERKnown[1] = 0;
		*OutSize = 12;
		return;
	}

	*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 0]) = 0x3D80;		//cmp byte [mem], 1
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)&XER_CARRY;
	*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = 0xF501;		//cmc

	//adc reg, 0
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 8]) = (ModRM(0x03, 2, Instruction->X86OutReg) << 8)
									| 0x83;
	*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 10]) = 0;
	*OutSize = 11;
	X86Regs[Instruction->X86OutReg].ValueChanged = 1;
}

GekkoRecILOp(SETCARRY)
{
	SetXERKnown[1] = 0;

	*(u8 *)(&((u8 *)OutInstruction)[0]) = 0xF9;	//stc
	*OutSize = 1;
}

GekkoRecILOp(CLEARCARRY)
{
	SetXERKnown[1] = 0;

	*(u8 *)(&((u8 *)OutInstruction)[0]) = 0xF8;	//clc
	*OutSize = 1;
}

GekkoRecILOp(COMPLIMENTCARRY)
{
	SetXERKnown[1] = 0;

	*(u8 *)(&((u8 *)OutInstruction)[0]) = 0xF5;	//cmc
	*OutSize = 1;
}

GekkoRecILOp(UPDATECARRY)
{
	SetXERKnown[1] = 0;

	*(u32 *)(&((u8 *)OutInstruction)[0]) = 0x0005920F;		//SETC
	*(u32 *)(&((u8 *)OutInstruction)[3]) = (u32)(&XER_CARRY);

	*OutSize = 7;
}

GekkoRecILOp(UPDATETEMPCARRY)
 {
	*OutSize = 0;
	if(SetXERKnown[0] & 0x80)
	{
		*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x05C6;		//mov byte [mem], val
		*(u32 *)(&((u8 *)OutInstruction)[2]) = (u32)(&XER_TEMPCARRY);
		*(u8 *)(&((u8 *)OutInstruction)[6]) = SetXERKnown[0] & 1;
		SetXERKnown[0] = 0;
		*OutSize = 7;
		return;
	}

	*(u32 *)(&((u8 *)OutInstruction)[0]) = 0x0005920F;		//SETC
	*(u32 *)(&((u8 *)OutInstruction)[3]) = (u32)&XER_TEMPCARRY;

	*OutSize = 7;
}

GekkoRecILOp(UPDATECARRYWTEMP)
{
	RecInstruction	TempInst;
	u32				NewOutSize;
	u32				FindRegMem;
	u32				FindCount;
	u32				Reg;

	*OutSize = 0;
	if(*(u16 *)(&SetXERKnown[0]) & 0x8080)
	{
		//see if carry should be set or not
		if((*(u16 *)(&SetXERKnown[0]) & 0x8080) == 0x0080)
		{
			//only the temp is known, if it is set then just set carry
			//otherwise temp is not set so ignore it
			if(*(u16 *)(&SetXERKnown[0]) & 0x01)
			{
				//move byte [mem], 1
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 0]) = (ModRM(0x00, 0, 0x05) << 8) | 0xC6;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)(&XER_CARRY);
				*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = 1;
				*(u16 *)(&SetXERKnown[0]) = 0;
				*OutSize = 7;
			}

			return;
		}
		else
		{
			//move byte [mem], ?
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 0]) = (ModRM(0x00, 0, 0x05) << 8) | 0xC6;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)(&XER_CARRY);

			if(*(u16 *)(&SetXERKnown[0]) & 0x0101)
			{
				//mov byte [mem], 1
				*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = 1;
			}
			else
			{
				//mov byte [mem], 0
				*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = 0;
			}
			*(u16 *)(&SetXERKnown[0]) = 0;
			*OutSize = 7;
			return;
		}
	}

	Reg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
	if((X86Regs[Reg].PPCRegister != -1) && X86Regs[Reg].ValueChanged)
	{
		//register returned is in use, move it out to memory
		TempInst.X86InReg = Reg;
		TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[Reg].PPCRegister];
		TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
		GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
	}
	X86Regs[Reg].PPCRegister = -1;

	//mov dword reg, XER_TEMPCARRY
	*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0, Reg, 5) << 8) | 0x8B;
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)(&XER_TEMPCARRY);

	//or XER_CARRY, reg
	*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (ModRM(0, Reg, 5) << 8) | 0x09;
	*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 8]) = (u32)(&XER_CARRY);

	*OutSize += 12;
}

GekkoRecILOp(PUSHA)
{
	//push all registers
#pragma todo("In 64bit mode, need to push each register due to being an invalid op otherwise")
	*(u8 *)OutInstruction = 0x60;
	*OutSize = 1;
}

GekkoRecILOp(POPA)
{
	//push all registers
#pragma todo("In 64bit mode, need to pop each register due to being an invalid op otherwise")
	*(u8 *)OutInstruction = 0x61;
	*OutSize = 1;
}

GekkoRecILOp(RET)
{
	*(u8 *)OutInstruction = 0xC3;
	*OutSize = 1;
}

GekkoRecILOp(GETTEMPREG)
{
	//reset if the temp value is known
	PPCRegVals[TEMP_REG].Flags = 0;
}

GekkoRecILOp(FREETEMPREG)
{
	//reset if the temp value is known
	PPCRegVals[TEMP_REG].Flags = 0;
}

GekkoRecILOp(GETREG)
{
}

GekkoRecILOp(FREEREG)
{
}

GekkoRecILOp(STOREREG)
{
}

GekkoRecILOp(TURNOFFCACHING)
{
	CachingEnabled = 0;
}

GekkoRecILOp(TURNONCACHING)
{
	CachingEnabled = 1;
}

GekkoRecILOp(INT_INSTRUCTION)
{
	//setup all the needed data to run an interpreted instruction

	//call xxxxxxxx
	*(u8 *)(&((u8 *)OutInstruction)[0]) = 0xE8;
	*(u32 *)(&((u8 *)OutInstruction)[1]) = Instruction->InVal - (u32)OutInstruction - 5;
	*OutSize = 5;
}

GekkoRecILOp(CMPUPDATECR)
{
	RecInstruction	TempInst;
	u32				NewOutSize;
	u32				FindRegMem;
	u32				FindCount;
	u32				Reg1;
	u32				x;

	//Our code for updating CR0
	*OutSize = 0;
	if(*(u32 *)(SetCRKnown) & 0x80808080)
	{
		//set some values for CR
		for(x = 0; x < 4; x++)
		{
			if(SetCRKnown[x] & 0x80)
			{
				TempInst.X86OutMemory = (u32)&CR[x];
				TempInst.X86InVal = SetCRKnown[x] & 1;
				TempInst.Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
		}
		*(u32 *)(SetCRKnown) = 0;
	}
	else
	{
		//find a register we can use
		Reg1 = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);

		if((X86Regs[Reg1].PPCRegister != -1) && X86Regs[Reg1].ValueChanged)
		{
			//register returned is in use, move it out to memory
			TempInst.X86InReg = Reg1;
			TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[Reg1].PPCRegister];
			TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
		}
		X86Regs[Reg1].PPCRegister = -1;

		//do the compare
		GekkoRecIL(CMP)(Instruction, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;

		//update CR
		Instruction->InReg = Reg1;		//we no longer care what we were in the Gekko CPU
		GekkoRecIL(UPDATECRUNSIGNED)(Instruction, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;

		//set which SO bit to set
		GekkoRecIL(UPDATECRSO)(Instruction, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
	}
}

GekkoRecILOp(CMPUPDATECRSIGNED)
{
	RecInstruction	TempInst;
	u32				NewOutSize;
	u32				FindRegMem;
	u32				FindCount;
	u32				Reg1;
	u32				x;

	//Our code for updating CR0
	*OutSize = 0;
	if(*(u32 *)(SetCRKnown) & 0x80808080)
	{
		//set some values for CR
		for(x = 0; x < 4; x++)
		{
			if(SetCRKnown[x] & 0x80)
			{
				TempInst.X86OutMemory = (u32)&CR[x];
				TempInst.X86InVal = SetCRKnown[x] & 1;
				TempInst.Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
		}
		*(u32 *)(SetCRKnown) = 0;
	}
	else
	{
		//find a register we can use
		Reg1 = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);

		if((X86Regs[Reg1].PPCRegister != -1) && X86Regs[Reg1].ValueChanged)
		{
			//register returned is in use, move it out to memory
			TempInst.X86InReg = Reg1;
			TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[Reg1].PPCRegister];
			TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
		}
		X86Regs[Reg1].PPCRegister = -1;

		//do the compare
		GekkoRecIL(CMP)(Instruction, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;

		//update CR
		Instruction->InReg = Reg1;		//we no longer care what we were in the Gekko CPU
		GekkoRecIL(UPDATECRSIGNED)(Instruction, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;

		//set which SO bit to set
		GekkoRecIL(UPDATECRSO)(Instruction, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
	}
}

GekkoRecILOp(SMUL)
{
	u32				Reg;
	RecInstruction	TempInst;
	u32				NewOutSize;
	u32				FindRegMem;
	u32				FindCount;

	//signed multiply

	*OutSize = 0;
	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
			//imul OutReg, InReg
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, Instruction->X86InReg) << 16) | 0xAF0F;
			*OutSize += 3;
			break;

		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
			//imul OutReg, [InMem]
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, Instruction->X86OutReg, 0x05) << 16) | 0xAF0F;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = Instruction->X86InMemory;
			*OutSize += 7;
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
			//imul OutReg, OutReg, InVal
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, Instruction->X86OutReg) << 8) | 0x69;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InVal;
			*OutSize += 6;
			break;

		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			Reg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
			if((X86Regs[Reg].PPCRegister != -1) && X86Regs[Reg].ValueChanged)
			{
				//register returned is in use, move it out to memory
				TempInst.X86InReg = Reg;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[Reg].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
			X86Regs[Reg].PPCRegister = -1;

			//move our memory into the register
			TempInst.X86OutReg = Reg;
			TempInst.X86InMemory = (u32)&Instruction->X86InMemory;
			TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;

			//update our entry incase a compare is done for the results
			Instruction->X86OutReg = Reg;
			Instruction->Flags = (Instruction->Flags & ~RecInstrFlagX86_OUTMASK) | RecInstrFlagX86OutReg;

			X86Regs[Reg].PPCRegister = Instruction->OutReg;

			//figure out which imul to do
			switch(Instruction->Flags & RecInstrFlagX86_INMASK)
			{
				case RecInstrFlagX86InReg:
					//imul OutReg, InReg
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Reg, Instruction->X86InReg) << 16) | 0xAF0F;
					*OutSize += 3;
					break;

				case RecInstrFlagX86InVal:
					//imul OutReg, OutReg, InVal
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Reg, Reg) << 8) | 0x69;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InVal;
					*OutSize += 6;
					break;
			}
			break;

		default:
			Unknown_Mask("IMUL", (Instruction->Flags & RecInstrFlagX86_MASK));
			return;
	}

	X86Regs[Instruction->X86OutReg].ValueChanged = 1;
}
/*
GekkoRecILOp(SMULH)
{
	u32				Reg;
	RecInstruction	TempInst;
	u32				NewOutSize;
	u32				FindRegMem;
	u32				FindCount;

	//signed multiply, return high word

	*OutSize = 0;
	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
			//see if out is in edx and in is in eax
			if(Instruction->X86OutReg != REG_EDX)
			{
				if(Instruction->X86InReg == REG_EDX)
				{
					//In is in edx, if we are in eax then xchg
					//otherwise we need to move in to eax then move to edx
					if(Instruction->X86OutReg == REG_EAX)
					{
					}
					else
					{
						//move in to eax
						//move out to edx

					}
				}
				else
				{
				}
			}

			//imul OutReg, InReg
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, Instruction->X86InReg) << 16) | 0xAF0F;
			*OutSize += 3;
			break;

		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
			//imul OutReg, [InMem]
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, Instruction->X86OutReg, 0x05) << 16) | 0xAF0F;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = Instruction->X86InMemory;
			*OutSize += 7;
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
			//imul OutReg, OutReg, InVal
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, Instruction->X86OutReg) << 8) | 0x69;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InVal;
			*OutSize += 6;
			break;

		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			Reg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
			if((X86Regs[Reg].PPCRegister != -1) && X86Regs[Reg].ValueChanged)
			{
				//register returned is in use, move it out to memory
				TempInst.X86InReg = Reg;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[Reg].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
			X86Regs[Reg].PPCRegister = -1;

			//move our memory into the register
			TempInst.X86OutReg = Reg;
			TempInst.X86InMemory = (u32)&Instruction->X86InMemory;
			TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;

			//update our entry incase a compare is done for the results
			Instruction->X86OutReg = Reg;
			Instruction->Flags = (Instruction->Flags & ~RecInstrFlagX86_OUTMASK) | RecInstrFlagX86OutReg;

			X86Regs[Reg].PPCRegister = Instruction->OutReg;

			//figure out which imul to do
			switch(Instruction->Flags & RecInstrFlagX86_INMASK)
			{
				case RecInstrFlagX86InReg:
					//imul OutReg, InReg
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Reg, Instruction->X86InReg) << 16) | 0xAF0F;
					*OutSize += 3;
					break;

				case RecInstrFlagX86InVal:
					//imul OutReg, OutReg, InVal
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Reg, Reg) << 8) | 0x69;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InVal;
					*OutSize += 6;
					break;
			}
			break;

		default:
			Unknown_Mask("IMUL", (Instruction->Flags & RecInstrFlagX86_MASK));
			return;
	}

	X86Regs[Instruction->X86OutReg].ValueChanged = 1;
}
*/
GekkoRecILOp(ROL)
{
	PPCRegInfo		TempReg;
	u32				TempX86Reg;
	RecInstruction	TempInst;
	u32				NewOutSize;

	//due to needing to handle a register as a rol amount
	//and requiring cl for that, we can't our our macros for ROL
	if(Instruction->Flags & RecInstrFlagX86InVal)
	{
		if((Instruction->X86InVal & 0x1F) != 0)
		{
			//going to rotate by a set amount
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = (ModRM(3, 0, Instruction->X86OutReg) << 8) | 0xC1;
				*(u8 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal & 0x1F;
				*OutSize = 3;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				//memory
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x05C1;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u8 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal & 0x1F;
				*OutSize = 7;
			}
		}
		return;
	}
	else
	{
		//going to rotate by a value in a register
		*OutSize = 0;

		//first, is our value already in a register or in memory
		if(Instruction->Flags & RecInstrFlagX86InReg)
		{
			//already have it in a register, if it is not ecx then
			//we need to exchange ecx
			if(Instruction->X86InReg != 1)
			{
				TempReg = X86Regs[1];
				X86Regs[1] = X86Regs[Instruction->X86InReg];
				X86Regs[Instruction->X86InReg] = TempReg;
				TempX86Reg = Instruction->X86InReg;
				Instruction->X86InReg = 1;
				if((Instruction->Flags & RecInstrFlagX86OutReg) && Instruction->X86OutReg == 1)
					Instruction->X86OutReg = TempX86Reg;

				TempInst.X86InReg = TempX86Reg;
				TempInst.X86OutReg = 1;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
				GekkoRecIL(XCHG)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
		}
		else
		{
			//see if ecx is in use, if so, write it out
			if((X86Regs[1].PPCRegister != -1) && X86Regs[1].ValueChanged)
			{
				//register returned is in use, move it out to memory
				TempInst.X86InReg = 1;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[1].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
			X86Regs[1].PPCRegister = -1;

			//read our value in
			TempInst.X86OutReg = 1;
			TempInst.X86InMemory = Instruction->X86InMemory;
			TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
		}
	}

	//now do the ROL
	if(Instruction->Flags & RecInstrFlagX86OutReg)
	{
		//rol reg, cl
		*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(3, 0, Instruction->X86OutReg) << 8) | 0xD3;
		*OutSize += 2;
		X86Regs[Instruction->X86OutReg].ValueChanged = 1;
	}
	else
	{
		//rol [mem], cl
		*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05D3;
		*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
		*OutSize += 6;
	}
}

GekkoRecILOp(ROR)
{
	PPCRegInfo		TempReg;
	u32				TempX86Reg;
	RecInstruction	TempInst;
	u32				NewOutSize;

	//due to needing to handle a register as a ror amount
	//and requiring cl for that, we can't our our macros for ROR
	if(Instruction->Flags & RecInstrFlagX86InVal)
	{
		if((Instruction->X86InVal & 0x1F) != 0)
		{
			//going to rotate by a set amount
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = (ModRM(3, 1, Instruction->X86OutReg) << 8) | 0xC1;
				*(u8 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal & 0x1F;
				*OutSize = 3;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				//memory
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x0DD3;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u8 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal & 0x1F;
				*OutSize = 7;
			}
		}
		return;
	}
	else
	{
		//going to rotate by a value in a register
		*OutSize = 0;

		//first, is our value already in a register or in memory
		if(Instruction->Flags & RecInstrFlagX86InReg)
		{
			//already have it in a register, if it is not ecx then
			//we need to exchange ecx
			if(Instruction->X86InReg != 1)
			{
				TempReg = X86Regs[1];
				X86Regs[1] = X86Regs[Instruction->X86InReg];
				X86Regs[Instruction->X86InReg] = TempReg;
				TempX86Reg = Instruction->X86InReg;
				Instruction->X86InReg = 1;
				if((Instruction->Flags & RecInstrFlagX86OutReg) && Instruction->X86OutReg == 1)
					Instruction->X86OutReg = TempX86Reg;

				TempInst.X86InReg = TempX86Reg;
				TempInst.X86OutReg = 1;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
				GekkoRecIL(XCHG)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
		}
		else
		{
			//see if ecx is in use, if so, write it out
			if((X86Regs[1].PPCRegister != -1) && X86Regs[1].ValueChanged)
			{
				//register returned is in use, move it out to memory
				TempInst.X86InReg = 1;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[1].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
			X86Regs[1].PPCRegister = -1;

			//read our value in
			TempInst.X86OutReg = 1;
			TempInst.X86InMemory = Instruction->X86InMemory;
			TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
		}
	}

	//now do the ROR
	if(Instruction->Flags & RecInstrFlagX86OutReg)
	{
		//ror reg, cl
		*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(3, 1, Instruction->X86OutReg) << 8) | 0xD3;
		*OutSize += 2;
		X86Regs[Instruction->X86OutReg].ValueChanged = 1;
	}
	else
	{
		//ror [mem], cl
		*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0DD3;
		*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
		*OutSize += 6;
	}
}

GekkoRecILOp(SAL)
{
	PPCRegInfo		TempReg;
	RecInstruction	TempInst;
	u32				NewSize;

	//due to needing to handle a register as a SAL amount
	//and requiring cl for that, we can't our our macros for SAL
	if(Instruction->Flags & RecInstrFlagX86InVal)
	{
		if(Instruction->X86InVal >= 32)
		{
			//move a 0 in place
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u8 *)(&((u8 *)OutInstruction)[0]) = 0xB8 + Instruction->X86OutReg;
				*(u32 *)(&((u8 *)OutInstruction)[1]) = 0;
				*OutSize = 5;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x05C7;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u32 *)(&((u8 *)OutInstruction)[6]) = 0;
				*OutSize = 10;
			}
		}
		else
		{
			//going to shift by a set amount
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = (ModRM(3, 4, Instruction->X86OutReg) << 8) | 0xC1;
				*(u8 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal & 0x1F;
				*OutSize = 3;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				//memory
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x25C1;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u8 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal & 0x1F;
				*OutSize = 7;
			}
		}
	}
	else
	{
		//mov ecx, shiftreg
		//sal reg, cl

		*OutSize = 0;

		//if our value is in memory move it to ecx if ecx is not already claimed
		//we don't assign it to ecx as it is probably in memory for a reason
		if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InMem)
		{
			if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
			{
				//ecx is in use and modified, write the value back out
				//mov ireg.gpr[ppcreg], ecx
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
				TempInst.X86InReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}

			//make sure our register is reset
			X86Regs[REG_ECX].PPCRegister = -1;

			//move our value to ecx
			TempInst.X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
			TempInst.X86OutReg = REG_ECX;
			TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
			*OutSize += NewSize;

			//define our shift reg as ecx
			Instruction->X86InReg = REG_ECX;
		}

		if(Instruction->X86InReg != REG_ECX)
		{
			if(((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg) &&
				(Instruction->X86OutReg == REG_ECX))
			{
				//xchg outreg, inreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = Instruction->X86OutReg;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
				GekkoRecIL(XCHG)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;

				//swap our registers
				TempReg = X86Regs[Instruction->X86InReg];
				X86Regs[Instruction->X86InReg] = X86Regs[Instruction->X86OutReg];
				X86Regs[Instruction->X86OutReg] = TempReg;

				//flip our registers for the instruction
				NewSize = Instruction->X86InReg;
				Instruction->X86InReg = Instruction->X86OutReg;
				Instruction->X86OutReg = NewSize;
			}
			else
			{
				if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
				{
					//ecx is in use and modified, write the value back out

					//mov ireg.gpr[ppcreg], ecx
					TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
					TempInst.X86InReg = REG_ECX;
					TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
					GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
					*OutSize += NewSize;
				}

				//make sure our register is reset
				X86Regs[REG_ECX].PPCRegister = -1;

				//mov ecx, shiftreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}
		}

		if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
		{
			//sal [mem], cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0, 4, 5) << 8) | 0xD3;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)Instruction->X86OutMemory;
			*OutSize += 6;
		}
		else
		{
			//sal reg, cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(3, 4, Instruction->X86OutReg) << 8) | 0xD3;
			*OutSize += 2;
		}
	}
}

GekkoRecILOp(SAR)
{
	PPCRegInfo		TempReg;
	RecInstruction	TempInst;
	u32				NewSize;

	//due to needing to handle a register as a SAR amount
	//and requiring cl for that, we can't our our macros for SAR
	if(Instruction->Flags & RecInstrFlagX86InVal)
	{
		if(Instruction->X86InVal >= 32)
		{
			//move a 0 in place
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u8 *)(&((u8 *)OutInstruction)[0]) = 0xB8 + Instruction->X86OutReg;
				*(u32 *)(&((u8 *)OutInstruction)[1]) = 0;
				*OutSize = 5;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x05C7;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u32 *)(&((u8 *)OutInstruction)[6]) = 0;
				*OutSize = 10;
			}
			return;
		}
		else
		{
			//going to shift by a set amount
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = (ModRM(3, 7, Instruction->X86OutReg) << 8) | 0xC1;
				*(u8 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal & 0x1F;
				*OutSize = 3;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				//memory
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x3DC1;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u8 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal & 0x1F;
				*OutSize = 7;
			}
		}
	}
	else
	{
		//mov ecx, shiftreg
		//sar reg, cl

		*OutSize = 0;

		//if our value is in memory move it to ecx if ecx is not already claimed
		//we don't assign it to ecx as it is probably in memory for a reason
		if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InMem)
		{
			if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
			{
				//ecx is in use and modified, write the value back out
				//mov ireg.gpr[ppcreg], ecx
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
				TempInst.X86InReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}

			//make sure our register is reset
			X86Regs[REG_ECX].PPCRegister = -1;

			//move our value to ecx
			TempInst.X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
			TempInst.X86OutReg = REG_ECX;
			TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
			*OutSize += NewSize;

			//define our shift reg as ecx
			Instruction->X86InReg = REG_ECX;
		}

		if(Instruction->X86InReg != REG_ECX)
		{
			if(((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg) &&
				(Instruction->X86OutReg == REG_ECX))
			{
				//xchg outreg, inreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = Instruction->X86OutReg;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
				GekkoRecIL(XCHG)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;

				//swap our registers
				TempReg = X86Regs[Instruction->X86InReg];
				X86Regs[Instruction->X86InReg] = X86Regs[Instruction->X86OutReg];
				X86Regs[Instruction->X86OutReg] = TempReg;

				//flip our registers for the instruction
				NewSize = Instruction->X86InReg;
				Instruction->X86InReg = Instruction->X86OutReg;
				Instruction->X86OutReg = NewSize;
			}
			else
			{
				if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
				{
					//ecx is in use and modified, write the value back out

					//mov ireg.gpr[ppcreg], ecx
					TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
					TempInst.X86InReg = REG_ECX;
					TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
					GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
					*OutSize += NewSize;
				}

				//make sure our register is reset
				X86Regs[REG_ECX].PPCRegister = -1;

				//mov ecx, shiftreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}
		}

		if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
		{
			//sar [mem], cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0, 7, 5) << 8) | 0xD3;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)Instruction->X86OutMemory;
			*OutSize += 6;
		}
		else
		{
			//sar reg, cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(3, 7, Instruction->X86OutReg) << 8) | 0xD3;
			*OutSize += 2;
		}
	}
}

GekkoRecILOp(SHL)
{
	PPCRegInfo		TempReg;
	RecInstruction	TempInst;
	u32				NewSize;

	//due to needing to handle a register as a SHL amount
	//and requiring cl for that, we can't use our macros for SHL
	if(Instruction->Flags & RecInstrFlagX86InVal)
	{
		//going to shift by a set amount
		if(Instruction->Flags & RecInstrFlagX86OutReg)
		{
			*(u16 *)(&((u8 *)OutInstruction)[0]) = (ModRM(3, 4, Instruction->X86OutReg) << 8) | 0xC1;
			*(u8 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal & 0x1F;
			*OutSize = 3;
			X86Regs[Instruction->X86OutReg].ValueChanged = 1;
		}
		else
		{
			//memory
			*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x25C1;
			*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
			*(u8 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal & 0x1F;
			*OutSize = 7;
		}
	}
	else
	{
		//mov ecx, shiftreg
		//shl reg, cl

		*OutSize = 0;

		//if our value is in memory move it to ecx if ecx is not already claimed
		//we don't assign it to ecx as it is probably in memory for a reason
		if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InMem)
		{
			if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
			{
				//ecx is in use and modified, write the value back out
				//mov ireg.gpr[ppcreg], ecx
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
				TempInst.X86InReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}

			//make sure our register is reset
			X86Regs[REG_ECX].PPCRegister = -1;

			//move our value to ecx
			TempInst.X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
			TempInst.X86OutReg = REG_ECX;
			TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
			*OutSize += NewSize;

			//define our shift reg as ecx
			Instruction->X86InReg = REG_ECX;
		}

		if(Instruction->X86InReg != REG_ECX)
		{
			if(((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg) &&
				(Instruction->X86OutReg == REG_ECX))
			{
				//xchg outreg, inreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = Instruction->X86OutReg;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
				GekkoRecIL(XCHG)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;

				//swap our registers
				TempReg = X86Regs[Instruction->X86InReg];
				X86Regs[Instruction->X86InReg] = X86Regs[Instruction->X86OutReg];
				X86Regs[Instruction->X86OutReg] = TempReg;

				//flip our registers for the instruction
				NewSize = Instruction->X86InReg;
				Instruction->X86InReg = Instruction->X86OutReg;
				Instruction->X86OutReg = NewSize;
			}
			else
			{
				if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
				{
					//ecx is in use and modified, write the value back out

					//mov ireg.gpr[ppcreg], ecx
					TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
					TempInst.X86InReg = REG_ECX;
					TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
					GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
					*OutSize += NewSize;
				}

				//make sure our register is reset
				X86Regs[REG_ECX].PPCRegister = -1;

				//mov ecx, shiftreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}
		}

		if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
		{
			//shl [mem], cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0, 4, 5) << 8) | 0xD3;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)Instruction->X86OutMemory;
			*OutSize += 6;
		}
		else
		{
			//shl reg, cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(3, 4, Instruction->X86OutReg) << 8) | 0xD3;
			*OutSize += 2;
		}
	}
}

GekkoRecILOp(SHR)
{
	PPCRegInfo		TempReg;
	RecInstruction	TempInst;
	u32				NewSize;

	//due to needing to handle a register as a SHR amount
	//and requiring cl for that, we can't our our macros for SHR
	if(Instruction->Flags & RecInstrFlagX86InVal)
	{
		if(Instruction->X86InVal >= 32)
		{
			//move a 0 in place
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u8 *)(&((u8 *)OutInstruction)[0]) = 0xB8 + Instruction->X86OutReg;
				*(u32 *)(&((u8 *)OutInstruction)[1]) = 0;
				*OutSize = 5;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x05C7;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u32 *)(&((u8 *)OutInstruction)[6]) = 0;
				*OutSize = 10;
			}
		}
		else
		{
			//going to shift by a set amount
			if(Instruction->Flags & RecInstrFlagX86OutReg)
			{
				*(u16 *)(&((u8 *)OutInstruction)[0]) = (ModRM(3, 5, Instruction->X86OutReg) << 8) | 0xC1;
				*(u8 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86InVal & 0x1F;
				*OutSize = 3;
				X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else
			{
				//memory
				*(u16 *)(&((u8 *)OutInstruction)[0]) = 0x2DC1;
				*(u32 *)(&((u8 *)OutInstruction)[2]) = Instruction->X86OutMemory;
				*(u8 *)(&((u8 *)OutInstruction)[6]) = Instruction->X86InVal & 0x1F;
				*OutSize = 7;
			}
		}
		return;
	}
	else
	{
		//mov ecx, shiftreg
		//shr reg, cl

		*OutSize = 0;

		//if our value is in memory move it to ecx if ecx is not already claimed
		//we don't assign it to ecx as it is probably in memory for a reason
		if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InMem)
		{
			if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
			{
				//ecx is in use and modified, write the value back out
				//mov ireg.gpr[ppcreg], ecx
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
				TempInst.X86InReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}

			//make sure our register is reset
			X86Regs[REG_ECX].PPCRegister = -1;

			//move our value to ecx
			TempInst.X86InMemory = (u32)&ireg.gpr[Instruction->InReg];
			TempInst.X86OutReg = REG_ECX;
			TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
			*OutSize += NewSize;

			//define our shift reg as ecx
			Instruction->X86InReg = REG_ECX;
		}

		if(Instruction->X86InReg != REG_ECX)
		{
			if(((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg) &&
				(Instruction->X86OutReg == REG_ECX))
			{
				//xchg outreg, inreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = Instruction->X86OutReg;
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg;
				GekkoRecIL(XCHG)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;

				//swap our registers
				TempReg = X86Regs[Instruction->X86InReg];
				X86Regs[Instruction->X86InReg] = X86Regs[Instruction->X86OutReg];
				X86Regs[Instruction->X86OutReg] = TempReg;

				//flip our registers for the instruction
				NewSize = Instruction->X86InReg;
				Instruction->X86InReg = Instruction->X86OutReg;
				Instruction->X86OutReg = NewSize;
			}
			else
			{
				if((X86Regs[REG_ECX].PPCRegister != -1) && X86Regs[REG_ECX].ValueChanged)
				{
					//ecx is in use and modified, write the value back out

					//mov ireg.gpr[ppcreg], ecx
					TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[REG_ECX].PPCRegister];
					TempInst.X86InReg = REG_ECX;
					TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
					GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
					*OutSize += NewSize;
				}

				//make sure our register is reset
				X86Regs[REG_ECX].PPCRegister = -1;

				//mov ecx, shiftreg
				TempInst.X86InReg = Instruction->X86InReg;
				TempInst.X86OutReg = REG_ECX;
				TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewSize, X86Regs, 0);
				*OutSize += NewSize;
			}
		}

		if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
		{
			//shr [mem], cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0, 5, 5) << 8) | 0xD3;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)Instruction->X86OutMemory;
			*OutSize += 6;
		}
		else
		{
			//shr reg, cl
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(3, 5, Instruction->X86OutReg) << 8) | 0xD3;
			*OutSize += 2;
		}
	}
}