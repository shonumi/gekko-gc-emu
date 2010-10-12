////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_opcodes_loadstore.cpp
// DESC:		Recompiler Load/Store Opcodes
// CREATED:		Jun. 30, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "../cpu_core_regs.h"
#include "../../low level/hardware core/hw_cp.h"

GekkoRecOp(LBZ)
{
	if(rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADDI(rD, SIMM));
		CreateRecInstruction(REC_READMEM8(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM8I(rD, SIMM));
}

GekkoRecOp(LBZU)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
	CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
	CreateRecInstruction(REC_READMEM8(rD, rD));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LBZUX)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADD(TEMP_REG, rB));
	CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
	CreateRecInstruction(REC_READMEM8(rD, rD));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LBZX)
{
	if(rA)
	{
		if(rD == rA)
		{
			CreateRecInstruction(REC_MOVE(rD, rA));
			CreateRecInstruction(REC_ADD(rD, rB));
		}
		else
		{
			CreateRecInstruction(REC_MOVE(rD, rB));
			CreateRecInstruction(REC_ADD(rD, rA));
		}
		CreateRecInstruction(REC_READMEM8(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM8(rD, rB));
}

GekkoRecOp(LHA)
{
	if(rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADDI(rD, SIMM));
		CreateRecInstruction(REC_READMEM16(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM16I(rD, SIMM));

	CreateRecInstruction(REC_SHLI(rD, 16));
	CreateRecInstruction(REC_SARI(rD, 16));
}

GekkoRecOp(LHAX)
{
	if(rA)
	{
		if(rD == rA)
		{
			CreateRecInstruction(REC_MOVE(rD, rA));
			CreateRecInstruction(REC_ADD(rD, rB));
		}
		else
		{
			CreateRecInstruction(REC_MOVE(rD, rB));
			CreateRecInstruction(REC_ADD(rD, rA));
		}
		CreateRecInstruction(REC_READMEM16(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM16I(rD, SIMM));

	CreateRecInstruction(REC_SHLI(rD, 16));
	CreateRecInstruction(REC_SARI(rD, 16));
}

GekkoRecOp(LHZ)
{
	if(rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADDI(rD, SIMM));
		CreateRecInstruction(REC_READMEM16(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM16I(rD, SIMM));
}

GekkoRecOp(LHZU)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
	CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
	CreateRecInstruction(REC_READMEM16(rD, rD));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LHZUX)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADD(TEMP_REG, rB));
	CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
	CreateRecInstruction(REC_READMEM16(rD, rD));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LHZX)
{
	if(rA)
	{
		if(rD == rA)
		{
			CreateRecInstruction(REC_MOVE(rD, rA));
			CreateRecInstruction(REC_ADD(rD, rB));
		}
		else
		{
			CreateRecInstruction(REC_MOVE(rD, rB));
			CreateRecInstruction(REC_ADD(rD, rA));
		}
		CreateRecInstruction(REC_READMEM16(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM16(rD, rB));
}

GekkoRecOp(LMW)
{
	int i;

	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
	}

	for(i = rD; i < 32; i++)
	{
		if(rA)
		{
			if(i != rD)
				CreateRecInstruction(REC_ADDI(TEMP_REG, 4));
			CreateRecInstruction(REC_MOVE(i, TEMP_REG));
			CreateRecInstruction(REC_READMEM32(i, i));
		}
		else
			CreateRecInstruction(REC_READMEM32I(i, SIMM + ((i - rD) * 4)));
	}

	if(rA)
		CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LWZ)
{
	if(rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADDI(rD, SIMM));
		CreateRecInstruction(REC_READMEM32(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM32I(rD, SIMM));
}

GekkoRecOp(LWZU)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_ADDI(rD, SIMM));
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rD));
	CreateRecInstruction(REC_READMEM32(rD, rD));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LWZUX)
{
	if(rB == rD)
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_ADD(rD, rA));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rD));
	CreateRecInstruction(REC_READMEM32(rD, rD));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LWZX)
{
	if(rA)
	{
		if(rB == rD)
		{
			CreateRecInstruction(REC_MOVE(rD, rB));
			CreateRecInstruction(REC_ADD(rD, rA));
		}
		else
		{
			CreateRecInstruction(REC_MOVE(rD, rA));
			CreateRecInstruction(REC_ADD(rD, rB));
		}
		CreateRecInstruction(REC_READMEM32(rD, rD));
	}
	else
		CreateRecInstruction(REC_READMEM32(rD, rB));
}

GekkoRecOp(MFMSR)
{
	CreateRecInstruction(REC_MOVE(rD, REG_PPC_MSR));
}

GekkoRecOp(MFSPR)
{
	u32 reg = ((rB << 5) | rA);

	switch(reg)
	{
		case I_TBL:
			CreateRecInstruction(REC_MOVE(rD, REG_PPC_TBL));
			break;

		case I_TBU:
			CreateRecInstruction(REC_MOVE(rD, REG_PPC_TBU));
			break;

		case I_XER:
			CreateRecInstruction(REC_GETREG(REG_EDX | REG_SPECIAL));
			CreateRecInstruction(REC_GETREG(REG_EBX | REG_SPECIAL));
			CreateRecInstruction(REC_MOVE(REG_EDX | REG_SPECIAL, REG_PPC_XER_SUMMARYOVERFLOW));
			CreateRecInstruction(REC_MOVE(REG_EBX | REG_SPECIAL, REG_PPC_XER_CARRY));

			CreateRecInstruction(REC_SHLI(REG_EDX | REG_SPECIAL, 1));
			CreateRecInstruction(REC_SHLI(REG_EBX | REG_SPECIAL, 29));

			CreateRecInstruction(REC_OR(REG_EDX | REG_SPECIAL, REG_PPC_XER_OVERFLOW));
			CreateRecInstruction(REC_OR(REG_EBX | REG_SPECIAL, REG_PPC_XER_WORDINDEX));

			CreateRecInstruction(REC_SHLI(REG_EDX | REG_SPECIAL, 30));
			CreateRecInstruction(REC_OR(REG_EDX | REG_SPECIAL, REG_EBX | REG_SPECIAL));

			CreateRecInstruction(REC_MOVE(rD, REG_EDX | REG_SPECIAL));
			CreateRecInstruction(REC_FREEREG(REG_EDX | REG_SPECIAL));
			CreateRecInstruction(REC_FREEREG(REG_EBX | REG_SPECIAL));
//			GekkoRec(INT_INSTRUCTION)();
			break;

		default:
			CreateRecInstruction(REC_MOVE(rD, REG_PPC_SPR | reg));
			break;

	}
}

GekkoRecOp(MFSR)
{
	CreateRecInstruction(REC_MOVE(rD, REG_PPC_SR | (rA & 0x0f)));
}

/*
GekkoRecOp(MFSRIN)
{
	RRD = ireg.sr[RRB >> 28];
}
*/

GekkoRecOp(MFTB)
{
	if(tbr == 268) CreateRecInstruction(REC_MOVE(rD, REG_PPC_TBL));
	else if(tbr == 269) CreateRecInstruction(REC_MOVE(rD, REG_PPC_TBU));
	else GekkoRec(INT_INSTRUCTION)();
}

/*
GekkoRecOp(MTCRF)
{
	u32 crm = 0;
	s32 x;

	crm = ((CRM & 0x80) ? 0xf0000000 : 0) | ((CRM & 0x40) ? 0x0f000000:0) | ((CRM & 0x20) ? 0x00f00000 : 0) | ((crm & 0x10 )? 0x000f0000 : 0) |
	      ((CRM & 0x08) ? 0x0000f000 : 0) | ((CRM & 0x04) ? 0x00000f00:0) | ((CRM & 0x02) ? 0x000000f0 : 0) | ((crm & 0x01) ? 0x0000000f : 0);

	for(x = 0; x < 32; x++)
	{
		//if the bit is set then update with RRS, else, leave it alone
		if(crm & 0x80000000)
			CR[x] = (RRS >> (31-x)) & 1;
		crm <<= 1;
	}
}
*/
GekkoRecOp(MTMSR)
{
	CreateRecInstruction(REC_MOVE(REG_PPC_MSR, rS));
}

GekkoRecOp(MTSPR)
{
	u32 reg = ((rB << 5) | rA);
	u32 JumpLabel1;
	u32 JumpLabel2;
	u32 JumpLabel3;
	u32 JumpLabel4;
	u32 JumpLabel5;

	switch(reg)
	{
		case I_XER:
//			CreateRecInstruction(REC_MOVE(REG_PPC_SPR | reg, rS));
			GekkoRec(INT_INSTRUCTION)();
break;

			CreateRecInstruction(REC_GETREG(REG_EAX | REG_SPECIAL));
			CreateRecInstruction(REC_GETREG(REG_EBX | REG_SPECIAL));
			CreateRecInstruction(REC_MOVE(REG_EAX | REG_SPECIAL, rS));
			CreateRecInstruction(REC_MOVE(REG_EBX | REG_SPECIAL, rS));
			CreateRecInstruction(REC_MOVE(REG_PPC_SPR | reg, rS));
			CreateRecInstruction(REC_ANDI(REG_EAX | REG_SPECIAL, 0x3F));
			CreateRecInstruction(REC_MOVE(REG_PPC_XER_WORDINDEX, REG_EAX));

			CreateRecInstruction(REC_MOVE(REG_EAX | REG_SPECIAL, rS));
			CreateRecInstruction(REC_SHRI(REG_EBX | REG_SPECIAL, 30));
			CreateRecInstruction(REC_SETC(REG_PPC_XER_CARRY));

			CreateRecInstruction(REC_SHRI(REG_EAX | REG_SPECIAL, 31));
			CreateRecInstruction(REC_SETC(REG_PPC_XER_OVERFLOW));

			CreateRecInstruction(REC_SHRI(REG_EBX | REG_SPECIAL, 2));
			CreateRecInstruction(REC_SETC(REG_PPC_XER_SUMMARYOVERFLOW));

			CreateRecInstruction(REC_FREEREG(REG_EAX | REG_SPECIAL));
			CreateRecInstruction(REC_FREEREG(REG_EBX | REG_SPECIAL));
			break;

//			GekkoRec(INT_INSTRUCTION)();
//			break;

		case I_DMAL:
			CreateRecInstruction(REC_MOVE(REG_PPC_SPR | reg, rS));
			GekkoRec(INT_INSTRUCTION)();
break;
			CreateRecInstruction(REC_GETTEMPREG(0));
			CreateRecInstruction(REC_MOVE(TEMP_REG, rS));
			CreateRecInstruction(REC_ANDI(TEMP_REG, 2));

			//if(!(rS & 2)) goto ExitDMAL
			JumpLabel1 = NextJumpID;
			CreateRecInstruction(REC_JZ(JumpLabel1));

			//if(!(ireg.spr[I_HID2] & HID2_LCE)) goto ExitDMAL
			CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
			CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_LCE));
			CreateRecInstruction(REC_FREETEMPREG());
			CreateRecInstruction(REC_JZ(JumpLabel1));

			CreateRecInstruction(REC_GETREG(REG_EAX | REG_SPECIAL));
			CreateRecInstruction(REC_GETREG(REG_EBX | REG_SPECIAL));
			CreateRecInstruction(REC_GETREG(REG_ECX | REG_SPECIAL));
			CreateRecInstruction(REC_GETREG(REG_EDX | REG_SPECIAL));

//			dma_len = (((ireg.spr[I_DMAU] & 0x1f) << 2) | ((ireg.spr[I_DMAL] >> 2) & 3)) ?
//				      (((ireg.spr[I_DMAU] & 0x1f) << 2) | ((ireg.spr[I_DMAL] >> 2) & 3)) :
//			          (0x80);
			CreateRecInstruction(REC_MOVE(REG_EDX | REG_SPECIAL, rS));
			CreateRecInstruction(REC_MOVE(REG_EBX | REG_SPECIAL, rS));
			CreateRecInstruction(REC_MOVE(REG_ECX | REG_SPECIAL, REG_PPC_SPR | I_DMAU));
			CreateRecInstruction(REC_SHRI(REG_EDX | REG_SPECIAL, 2));
			CreateRecInstruction(REC_SHLI(REG_ECX | REG_SPECIAL, 2));
			CreateRecInstruction(REC_ANDI(REG_EBX | REG_SPECIAL, ~0x1F));
			CreateRecInstruction(REC_MOVE(REG_EAX | REG_SPECIAL, REG_EBX | REG_SPECIAL));
			CreateRecInstruction(REC_ANDI(REG_EDX | REG_SPECIAL, 3));
			CreateRecInstruction(REC_ANDI(REG_ECX | REG_SPECIAL, 0x3C));
			CreateRecInstruction(REC_OR(REG_ECX | REG_SPECIAL, REG_EDX | REG_SPECIAL));

			JumpLabel2 = NextJumpID;
			CreateRecInstruction(REC_JNZ(JumpLabel2));
			CreateRecInstruction(REC_MOVEI(REG_ECX | REG_SPECIAL, 0x80));

			//dma_len <<= 3;
			CreateRecInstruction(REC_JUMPLABEL(JumpLabel2));
			CreateRecInstruction(REC_SHLI(REG_ECX | REG_SPECIAL, 3));
			
			CreateRecInstruction(REC_MOVE(REG_EDX | REG_SPECIAL, REG_PPC_SPR | I_DMAU));

			//if((ireg.spr[I_DMAL] >> 4) & 1)
			CreateRecInstruction(REC_ANDI(REG_EDX | REG_SPECIAL, ~0x1F));
//			CreateRecInstruction(REC_ADDI(REG_EDX | REG_SPECIAL, (u32)&Mem_RAM));
//			CreateRecInstruction(REC_ADDI(REG_EBX | REG_SPECIAL, (u32)&Mem_RAM));
			CreateRecInstruction(REC_ANDI(REG_EAX | REG_SPECIAL, 0x10));

			JumpLabel3 = NextJumpID;
			CreateRecInstruction(REC_JZ(JumpLabel3));

			JumpLabel4 = NextJumpID;

//			for(i = 0; i < (dma_len >> 2); i++)
//			{
//				Val = Memory_Read32((ireg.spr[I_DMAU] &~ 0x1f) + (i * 4));
//				Memory_Write32((ireg.spr[I_DMAL] &~ 0x1f) + (i * 4), Val);
//			}

			//EDX = I_DMAU
			//ECX = dma_len
			//EBX = I_DMAL
			//eax = temp data
			CreateRecInstruction(REC_JUMPLABEL(JumpLabel4));
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			LastInstruction->X86InReg = REG_EDX;
			LastInstruction->X86OutReg = REG_EAX;
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86OutMem;
			LastInstruction->X86InReg = REG_EAX;
			LastInstruction->X86OutReg = REG_EBX;
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			LastInstruction->X86InReg = REG_EDX;
			LastInstruction->X86OutReg = REG_EAX;
			LastInstruction->X86Displacement = 4;
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86OutMem;
			LastInstruction->X86InReg = REG_EAX;
			LastInstruction->X86OutReg = REG_EBX;
			LastInstruction->X86Displacement = 4;
			CreateRecInstruction(REC_ADDI(REG_EDX | REG_SPECIAL, 8));
			CreateRecInstruction(REC_ADDI(REG_EBX | REG_SPECIAL, 8));
			CreateRecInstruction(REC_SUBI(REG_ECX | REG_SPECIAL, 2));
			CreateRecInstruction(REC_JGE(JumpLabel4));

			//else
			CreateRecInstruction(REC_JUMPLABEL(JumpLabel3));
//			for(i = 0; i < (dma_len >> 2); i++)
//			{
//				Val = Memory_Read32((ireg.spr[I_DMAL] &~ 0x1f) + (i * 4));
//				Memory_Write32((ireg.spr[I_DMAU] &~ 0x1f) + (i * 4), Val);
//			}

			//EDX = I_DMAU
			//ECX = dma_len
			//EBX = I_DMAL
			//eax = temp data
			JumpLabel5 = NextJumpID;
			CreateRecInstruction(REC_JUMPLABEL(JumpLabel5));
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			LastInstruction->X86InReg = REG_EBX;
			LastInstruction->X86OutReg = REG_EAX;
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86OutMem;
			LastInstruction->X86InReg = REG_EAX;
			LastInstruction->X86OutReg = REG_EDX;
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
			LastInstruction->X86InReg = REG_EBX;
			LastInstruction->X86OutReg = REG_EAX;
			LastInstruction->X86Displacement = 4;
			CreateRecInstruction(REC_MOVE(0,0));
			LastInstruction->Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutReg | RecInstrFlagX86OutMem;
			LastInstruction->X86InReg = REG_EAX;
			LastInstruction->X86OutReg = REG_EDX;
			LastInstruction->X86Displacement = 4;
			CreateRecInstruction(REC_ADDI(REG_EDX | REG_SPECIAL, 8));
			CreateRecInstruction(REC_ADDI(REG_EBX | REG_SPECIAL, 8));
			CreateRecInstruction(REC_SUBI(REG_ECX | REG_SPECIAL, 2));
			CreateRecInstruction(REC_JGE(JumpLabel5));

			//ExitDMAL
			CreateRecInstruction(REC_JUMPLABEL(JumpLabel1));

			break;
		
		case I_TBL:
			CreateRecInstruction(REC_MOVE(REG_PPC_TBL, rS));
			CreateRecInstruction(REC_MOVE(REG_PPC_SPR | reg, rS));
			break;

		case I_TBU:
			CreateRecInstruction(REC_MOVE(REG_PPC_TBU, rS));
			CreateRecInstruction(REC_MOVE(REG_PPC_SPR | reg, rS));
			break;

		case I_WPAR:
			//put rS into ECX, done here incase it is already in use
			CreateRecInstruction(REC_MOVE(REG_ECX | REG_SPECIAL, rS));
			CreateRecInstruction(REC_MOVE(REG_PPC_SPR | reg, rS));

			//save off any registers needed to be saved
			CreateRecInstruction(GekkoRecIL(SaveAllX86Regs), 0, 0, 0, 0);
			CreateRecInstruction(GekkoRecIL(SaveAllFPURegs), 0, 0, 0, 0);
			CreateRecInstruction(GekkoRecIL(SaveAllKnownX86ValRegs), 0, 0, 0, 0);

			//call the CP_UPDATE function
			CreateRecInstruction(REC_CALL((u32)&CP_Update));
			break;

		default:
			CreateRecInstruction(REC_MOVE(REG_PPC_SPR | reg, rS));
	}
}

GekkoRecOp(MTSR)
{
	CreateRecInstruction(REC_MOVE(REG_PPC_SR | (rA & 0x0f), rS));
}

GekkoRecOp(STB)
{
	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
		CreateRecInstruction(REC_WRITEMEM8(TEMP_REG, rS));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_WRITEMEM8I(SIMM, rS));
}

GekkoRecOp(STBU)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
	CreateRecInstruction(REC_WRITEMEM8(TEMP_REG, rS));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(STBUX)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADD(TEMP_REG, rB));
	CreateRecInstruction(REC_WRITEMEM8(TEMP_REG, rS));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(STBX)
{
	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADD(TEMP_REG, rB));
		CreateRecInstruction(REC_WRITEMEM8(TEMP_REG, rS));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_WRITEMEM8(rB, rS));
}

GekkoRecOp(STH)
{
	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
		CreateRecInstruction(REC_WRITEMEM16(TEMP_REG, rS));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_WRITEMEM16I(SIMM, rS));
}

GekkoRecOp(STHU)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
	CreateRecInstruction(REC_WRITEMEM16(TEMP_REG, rS));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(STHUX)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADD(TEMP_REG, rB));
	CreateRecInstruction(REC_WRITEMEM16(TEMP_REG, rS));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(STHX)
{
	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADD(TEMP_REG, rB));
		CreateRecInstruction(REC_WRITEMEM16(TEMP_REG, rS));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_WRITEMEM16(rB, rS));
}

GekkoRecOp(STMW)
{
	int i;

	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
	}

	for(i = rD; i < 32; i++)
	{
		if(rA)
		{
			if(i != rD)
				CreateRecInstruction(REC_ADDI(TEMP_REG, 4));
			CreateRecInstruction(REC_WRITEMEM32(TEMP_REG, i));
		}
		else
		{
			CreateRecInstruction(REC_WRITEMEM32(SIMM + ((i - rD) * 4), i));
			LastInstruction->Flags = (LastInstruction->Flags & ~RecInstrFlagPPC_OUTMASK) | RecInstrFlagPPCOutMem;
		}
	}

	if(rA)
		CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(STW)
{
	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
		CreateRecInstruction(REC_WRITEMEM32(TEMP_REG, rS));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_WRITEMEM32I(SIMM, rS));
}

GekkoRecOp(STWU)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
	CreateRecInstruction(REC_WRITEMEM32(TEMP_REG, rS));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(STWUX)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
	CreateRecInstruction(REC_ADD(TEMP_REG, rB));
	CreateRecInstruction(REC_WRITEMEM32(TEMP_REG, rS));
	CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(STWX)
{
	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADD(TEMP_REG, rB));
		CreateRecInstruction(REC_WRITEMEM32(TEMP_REG, rS));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_WRITEMEM32(rB, rS));
}
