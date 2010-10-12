////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_opcodes_ps.cpp
// DESC:		Recompiler Paired Single Opcodes
// CREATED:		Jun. 30, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2008 Lightning
////////////////////////////////////////////////////////////

//#define REC_PRINT_INT_CALLS

#include "cpu_rec.h"
#include "../cpu_core_regs.h"
#include "../../high level/highlevel.h"
#include "../../high level/hle_func.h"

GekkoRecOp(INT_INSTRUCTION)
{
	GekkoFP		iPtr;
	u32			InstrID;

	//look up the int function for the opcode
	iPtr = GekkoCPURecOpset[OPCD];
	InstrID = OPCD << 10;

	if(iPtr == GekkoCPUOp(Ops_Group4))
	{
		iPtr = GekkoCPURecOpsGroup4Table[XO3];
		InstrID |= XO3;
		if(iPtr == GekkoCPUOp(Ops_Group4XO0))
		{
			iPtr = GekkoCPURecOpsGroup4XO0Table[XO0];
			InstrID |= XO0;
		}
	}
	else if(iPtr == GekkoCPUOp(Ops_Group19))
	{
		iPtr = GekkoCPURecOpsGroup19Table[XO0];
		InstrID |= XO0;
		if(iPtr == GekkoCPUOp(RFI))
			branch=true;
		else if(iPtr == GekkoCPUOp(BCLRX))
			branch=true;
		else if(iPtr == GekkoCPUOp(BCCTRX))
			branch=true;
	}
	else if(iPtr == GekkoCPUOp(Ops_Group31))
	{
		iPtr = GekkoCPURecOpsGroup31Table[XO0];
		InstrID |= XO0;
	}
	else if(iPtr == GekkoCPUOp(Ops_Group59))
	{
		iPtr = GekkoCPURecOpsGroup59Table[XO3];
		InstrID |= XO3;
	}
	else if(iPtr == GekkoCPUOp(Ops_Group63))
	{
		iPtr = GekkoCPURecOpsGroup63Table[XO3];
		InstrID |= XO3;
		if(iPtr == GekkoCPUOp(Ops_Group63XO0))
		{
			InstrID |= XO0;
			iPtr = GekkoCPURecOpsGroup63XO0Table[XO0];
		}
	}
	else if(iPtr == GekkoCPUOp(BCX))
		branch=true;
	else if(iPtr == GekkoCPUOp(BX))
		branch=true;
	else if(iPtr == GekkoCPUOp(NI))
		branch=true;
	else if(iPtr == GekkoCPUOp(SC))
		branch=true;
	else if(iPtr == GekkoCPUOp(HLE))
	{
		//hle, store our PC value for hle to use
		CreateRecInstruction(REC_MOVE(0,0));
		LastInstruction->Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem;
		LastInstruction->X86InVal = *(u32 *)(&Mem_RAM[(ireg.PC + 8) & RAM_MASK]); // Memory_Read32(ireg.PC + 8);
		LastInstruction->X86OutMemory = (u32)&HLE_PC;
	}

	//IntCalls[InstrID]++;

	CreateRecInstruction(REC_MOVE(0,0));
	LastInstruction->Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem;
	LastInstruction->X86InVal = OPCODE;
	LastInstruction->X86OutMemory = (u32)&OPCODE;

	CreateRecInstruction(REC_INT_INSTRUCTION((u32)iPtr));

#ifdef REC_PRINT_INT_CALLS
	CreateRecInstruction(REC_ADD(0, 0));
	LastInstruction->Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem;
	LastInstruction->X86InVal = 1;
	LastInstruction->X86OutMemory = (u32)&IntCalls[InstrID];
#endif

/*
	if(!branch)
	{
		CreateRecInstruction(REC_ADD(0,0));
		LastInstruction->Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem;
		LastInstruction->X86InVal = 4;
		LastInstruction->X86OutMemory = (u32)&ireg.PC;

//		CreateRecInstruction(REC_ADD(0,0));
//		LastInstruction->Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem;
//		LastInstruction->X86InVal = 4;
//		LastInstruction->X86OutMemory = (u32)&pPC;
	}
*/
//	IsInterpret = 1;
}

GekkoRecOp(NOP)
{
	CreateRecInstruction(REC_NOP());
}

GekkoRecOp(BX)
{
	if(AALK & 1)
	{
		CreateRecInstruction(REC_MOVEI(REG_PPC_SPR + I_LR, ireg.PC + 4));
	}

	if(AALK & 2)
	{
		CreateRecInstruction(REC_MOVEI(REG_PPC_PC, EXTS(LI, 26)));
	}
	else
	{
		CreateRecInstruction(REC_MOVEI(REG_PPC_PC, ireg.PC + EXTS(LI, 26)));
	}

	LastInstruction->Flags |= RecInstrFlag_Branch;

	branch = 1;
}

GekkoRecOp(BCX)
{
	u32		Jump1 = NextJumpID;
	u32		Jump2 = NextJumpID;
	u32		Jump1Used = 0;

	//decrement CTR if need be
	if(!(BO & 0x04))
		CreateRecInstruction(REC_SUBI(REG_PPC_SPR + I_CTR, 1));

	//figure out what type of compares are required
	//if((((BO >> 2) | ((CTR != 0) ^ (BO >> 1))) & 1) &
	//   (((BO >> 4) | (B == ((BO >> 3) & 1)))) & 1)
	if(!((BO >> 2) & 1))
	{
		//if the sub above is not done then compare
		if((BO & 0x04))
			CreateRecInstruction(REC_CMPI(REG_PPC_SPR + I_CTR, 0));

		if((BO >> 1) & 1)
			CreateRecInstruction(REC_JNZ(Jump1));
		else
			CreateRecInstruction(REC_JZ(Jump1));

		Jump1Used = 1;
	}
	if(!((BO >> 4) & 1))
	{
//		if(LastInstruction)
//			if(!(BO & 0x04) || (LastInstruction->Instruction != GekkoRecIL(CMP)))
				CreateRecInstruction(REC_CMPI(REG_PPC_CR + BI, ((BO >> 3) & 1)));

		CreateRecInstruction(REC_JNZ(Jump1));

		Jump1Used = 1;
	}

	if(LK)
		CreateRecInstruction(REC_MOVEI(REG_PPC_SPR + I_LR, ireg.PC + 4));

	if(AA)
	{
		CreateRecInstruction(REC_MOVEI(REG_PPC_PC, EXTS16(BD)));
	}
	else
	{
		CreateRecInstruction(REC_MOVEI(REG_PPC_PC, ireg.PC + EXTS16(BD)));
	}

	if(Jump1Used)
	{
		CreateRecInstruction(REC_JMP(Jump2));

		CreateRecInstruction(REC_JUMPLABEL(Jump1));
		CreateRecInstruction(REC_MOVEI(REG_PPC_PC, ireg.PC + 4));

		CreateRecInstruction(REC_JUMPLABEL(Jump2));
	}

//	CreateRecInstruction(REC_MOVEI(REG_PPC_BRANCH, 1));

	LastInstruction->Flags |= RecInstrFlag_Branch;
	branch = 1;
}

GekkoRecOp(BCCTRX)
{
	u32		Jump1 = NextJumpID;
	u32		Jump2 = NextJumpID;

	if(!((BO >> 4) & 1))
	{
		CreateRecInstruction(REC_CMPI(REG_PPC_CR + BI, ((BO >> 3) & 1)));
		CreateRecInstruction(REC_JNE(Jump1));
	}

	if(LK)
		CreateRecInstruction(REC_MOVEI(REG_PPC_SPR + I_LR, ireg.PC + 4));

	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR + I_CTR));
	CreateRecInstruction(REC_ANDI(TEMP_REG, ~3));
	CreateRecInstruction(REC_MOVE(REG_PPC_PC, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());

	if(((BO >> 4) & 1))
	{
		CreateRecInstruction(REC_JMP(Jump2));

		CreateRecInstruction(REC_JUMPLABEL(Jump1));

		CreateRecInstruction(REC_MOVEI(REG_PPC_PC, ireg.PC + 4));

		CreateRecInstruction(REC_JUMPLABEL(Jump2));
	}

	LastInstruction->Flags |= RecInstrFlag_Branch;
	branch = 1;
}

GekkoRecOp(BCLRX)
{
	u32 Jump1 = NextJumpID;
	u32 Jump2 = NextJumpID;
	u32 Jump1Used = 0;

	//decrement CTR if need be
	if(!(BO & 0x04))
		CreateRecInstruction(REC_SUBI(REG_PPC_SPR + I_CTR, 1));

	//figure out what type of compares are required
	//if((((BO >> 2) | ((CTR != 0) ^ (BO >> 1))) & 1) &
	//   (((BO >> 4) | (B == ((BO >> 3) & 1)))) & 1)
	if(!((BO >> 2) & 1))
	{
		CreateRecInstruction(REC_CMPI(REG_PPC_SPR + I_CTR, 0));
		if((BO >> 1) & 1)
			CreateRecInstruction(REC_JNE(Jump1));
		else
			CreateRecInstruction(REC_JE(Jump1));
		Jump1Used = 1;
	}
	if(!((BO >> 4) & 1))
	{
		CreateRecInstruction(REC_CMPI(REG_PPC_CR + BI, ((BO >> 3) & 1)));
		CreateRecInstruction(REC_JNE(Jump1));
		Jump1Used = 1;
	}

	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR + I_LR));
	CreateRecInstruction(REC_ANDI(TEMP_REG, ~3));
	CreateRecInstruction(REC_MOVE(REG_PPC_PC, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());

	if(LK)
		CreateRecInstruction(REC_MOVEI(REG_PPC_SPR + I_LR, ireg.PC + 4));

	if(Jump1Used)
	{
		CreateRecInstruction(REC_JMP(Jump2));

		CreateRecInstruction(REC_JUMPLABEL(Jump1));
		CreateRecInstruction(REC_MOVEI(REG_PPC_PC, ireg.PC + 4));

		CreateRecInstruction(REC_JUMPLABEL(Jump2));
	}

	LastInstruction->Flags |= RecInstrFlag_Branch;
	branch = 1;
}

GekkoRecOp(RFI)
{
	//leave MSR bits 1-4,10-12,14-15,24,28-29 alone
	//turn msr bit 13 off
	//copy ssr1 bits 0,5-9,16-23,25-27,30-31 to msr
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR + I_SRR1));
	CreateRecInstruction(REC_ANDI(REG_PPC_MSR, ~0x87C4FF73));
	CreateRecInstruction(REC_ANDI(TEMP_REG, 0x87C0FF73));
	CreateRecInstruction(REC_OR(REG_PPC_MSR, TEMP_REG));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR + I_SRR0));
	CreateRecInstruction(REC_ANDI(TEMP_REG, ~3));
	CreateRecInstruction(REC_MOVE(REG_PPC_PC, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());

	LastInstruction->Flags |= RecInstrFlag_Branch;
	branch = 1 | Branch_RFI;
}

GekkoRecOp(DCBI)
{
	u32 Jump1 = NextJumpID;

	CreateRecInstruction(REC_GETREG(REG_ECX));
	CreateRecInstruction(REC_MOVE(REG_ECX, REG_PPC_MSR));
	CreateRecInstruction(REC_ANDI(REG_ECX, MSR_BIT_POW));
	CreateRecInstruction(REC_JZ(Jump1));


	CreateRecInstruction(GekkoRecIL(MOVE), 0, 0, RecInstrFlagX86InVal | RecInstrFlagX86OutReg, 0);
	LastInstruction->X86InVal = GekkoCPU::GEX_DEC;
	LastInstruction->X86OutReg = GEX_PROG;

	CreateRecInstruction(REC_CALL((u32)&GekkoCPURecompiler::Exception_REC));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));
	CreateRecInstruction(REC_FREEREG(REG_ECX));

//	LastInstruction->Flags |= RecInstrFlag_Branch;
//	branch = 1;
}
