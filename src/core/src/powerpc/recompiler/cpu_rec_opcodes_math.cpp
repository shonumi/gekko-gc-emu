////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_opcodes_math.cpp
// DESC:		Recompiler Math Opcodes
// CREATED:		Feb. 26, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"

GekkoRecOp(ADD)
{
	if(rD == rB)
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_ADD(rD, rA));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(ADDO)
{
	if(rD == rB)
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_ADD(rD, rA));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_UPDATEXERSOOV(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(1));
}

GekkoRecOp(ADDC)
{
	if(rD == rB)
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_ADD(rD, rA));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_UPDATECARRY(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(1));
}

GekkoRecOp(ADDCO)
{
	if(rD == rB)
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_ADD(rD, rA));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(ADDE)
{
	if(rD == rB)
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_ADD(rD, rA));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(ADDEO)
{
	if(rD == rB)
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_ADD(rD, rA));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(3));
}

GekkoRecOp(ADDI)
{
	if(rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		if(SIMM != 0)
			CreateRecInstruction(REC_ADDI(rD, SIMM));
	}
	else
		CreateRecInstruction(REC_MOVEI(rD, SIMM));
}

GekkoRecOp(ADDIC)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_ADDI(rD, SIMM));
	CreateRecInstruction(REC_UPDATECARRY(rD));
}

GekkoRecOp(ADDICD)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_ADDI(rD, SIMM));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATECR0(1));
}

GekkoRecOp(ADDIS)
{
	if(rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		if(SIMM != 0)
			CreateRecInstruction(REC_ADDI(rD, ((u32)SIMM << 16)));
	}
	else
		CreateRecInstruction(REC_MOVEI(rD, ((u32)SIMM << 16)));
}

GekkoRecOp(ADDME)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_ADDI(rD, 0xFFFFFFFF));
	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));
	
	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(ADDMEO)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_ADDI(rD, 0xFFFFFFFF));
	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));
	
	if(RC)
		CreateRecInstruction(REC_UPDATECR0(3));
}

GekkoRecOp(ADDZE)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(1));
}

GekkoRecOp(ADDZEO)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(AND)
{
	if(rA == rB)
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		CreateRecInstruction(REC_AND(rA, rS));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_AND(rA, rB));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(ANDC)
{
	int i;

	if(rA == rS)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
		CreateRecInstruction(REC_NOT(TEMP_REG));
		CreateRecInstruction(REC_AND(TEMP_REG, rS));
		CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
		i=1;
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		CreateRecInstruction(REC_NOT(rA));
		CreateRecInstruction(REC_AND(rA, rS));
		i=0;
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(i));
}

GekkoRecOp(ANDID)
{
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_ANDI(rA, UIMM));

	CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(ANDISD)
{
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_ANDI(rA, ((u32)UIMM << 16)));

	CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(DIVW)
{
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_EAX));
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_EDX));
	CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_EAX, rA));
	CreateRecInstruction(REC_CDQ());
	CreateRecInstruction(REC_IDIV(rB));
	CreateRecInstruction(REC_MOVE(rD, REG_SPECIAL | REG_EAX));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_EAX));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_EDX));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}
/*
GekkoRecOp(DIVWO)
{
	CreateRecInstruction(REC_STOREREG(REG_EDX));
	CreateRecInstruction(REC_STOREMOVE(REG_EAX, rA));
	CreateRecInstruction(REC_GETTEMPREG(REG_EAX_BIT | REG_EDX_BIT));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
	CreateRecInstruction(REC_CMP(TEMP_REG, 0));
	CreateRecInstruction(REC_SETE(REG_EDX));
	CreateRecInstruction(REC_ADD(TEMP_REG, REG_EDX));
	CreateRecInstruction(REC_CDQ());
	CreateRecInstruction(REC_IDIV(TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
	CreateRecInstruction(REC_UPDATEXERSOOV());
	CreateRecInstruction(REC_MOVE(rD, REG_EAX));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}
*/

GekkoRecOp(DIVWU)
{
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_EAX));
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_EDX));
	CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_EAX, rA));
	CreateRecInstruction(REC_XOR(REG_SPECIAL | REG_EDX, REG_SPECIAL | REG_EDX));
	CreateRecInstruction(REC_DIV(rB));
	CreateRecInstruction(REC_MOVE(rD, REG_SPECIAL | REG_EAX));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_EAX));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_EDX));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

/*
GekkoRecOp(DIVWUO)
{
	CreateRecInstruction(REC_STOREREG(REG_EDX));
	CreateRecInstruction(REC_STOREMOVE(REG_EAX, rA));
	CreateRecInstruction(REC_GETTEMPREG(REG_EAX_BIT | REG_EDX_BIT));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
	CreateRecInstruction(REC_CMP(TEMP_REG, 0));
	CreateRecInstruction(REC_SETE(REG_EDX));
	CreateRecInstruction(REC_ADD(TEMP_REG, REG_EDX));
	CreateRecInstruction(REC_XOR(REG_EDX, REG_EDX));
	CreateRecInstruction(REC_DIV(TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
	CreateRecInstruction(REC_UPDATEXERSOOV());
	CreateRecInstruction(REC_MOVE(rD, REG_EAX));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}
*/
GekkoRecOp(EQV)
{
	if(rA == rB)
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		CreateRecInstruction(REC_XOR(rA, rS));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_XOR(rA, rB));
	}
	CreateRecInstruction(REC_NOT(rA));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}
/*
GekkoRecOp(MULLHW)
{
	if(rD == rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_IMULH(rD, rB));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_IMULH(rD, rA));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(MULLHWU)
{
	if(rD == rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_MULH(rD, rB));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_MULH(rD, rA));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}
*/
GekkoRecOp(MULLI)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_SMULI(rD, SIMM));
}

GekkoRecOp(MULLW)
{
	if(rD == rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_SMUL(rD, rB));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_SMUL(rD, rA));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(NAND)
{
	if(rA == rB)
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		CreateRecInstruction(REC_AND(rA, rS));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_AND(rA, rB));
	}
	CreateRecInstruction(REC_NOT(rA));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(NEG)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_NEG(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(NEGO)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_NEG(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(1));
}

GekkoRecOp(NOR)
{
	if(rA == rB)
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		CreateRecInstruction(REC_OR(rA, rS));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_OR(rA, rB));
	}
	CreateRecInstruction(REC_NOT(rA));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(OR)
{
	if(rA == rB)
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		if(rA != rS)
			CreateRecInstruction(REC_OR(rA, rS));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rS));
		if(rB != rS)
			CreateRecInstruction(REC_OR(rA, rB));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(ORC)
{
	int i;

	if(rA == rS)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
		CreateRecInstruction(REC_NOT(TEMP_REG));
		CreateRecInstruction(REC_OR(TEMP_REG, rS));
		CreateRecInstruction(REC_MOVE(rA, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
		i=1;
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		CreateRecInstruction(REC_NOT(rA));
		CreateRecInstruction(REC_OR(rA, rS));
		i=0;
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(i));
}

GekkoRecOp(ORI)
{
	//if a nop instruction then don't do anything
	if(rA == rS && UIMM == 0)
	{
		CreateRecInstruction(REC_NOP);
		return;
	}

	CreateRecInstruction(REC_MOVE(rA, rS));
	if(UIMM != 0)
		CreateRecInstruction(REC_ORI(rA, UIMM));
}

GekkoRecOp(ORIS)
{
	CreateRecInstruction(REC_MOVE(rA, rS));
	if(UIMM != 0)
		CreateRecInstruction(REC_ORI(rA, (UIMM << 16)));
}

GekkoRecOp(SLW)
{
	u32 Label1 = NextJumpID;
	u32 Label2 = NextJumpID;
	u32 NextReg = NextGetRegID;

#pragma todo("Implement caching code for jumps so functions like SLW can auto-detect known value shifts")
	CreateRecInstruction(REC_TURNOFFCACHING());
	CreateRecInstruction(REC_STOREREG(REG_ECX));
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_ECX));
	CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_ECX, rB));
	CreateRecInstruction(REC_ANDI(REG_SPECIAL | REG_ECX, 0x20));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_ECX));		//force ECX clear
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_ECX));
	CreateRecInstruction(REC_JNZ(Label1));

	if((rS == rB) || (rA == rB))
	{
		CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_ECX, rB));
		CreateRecInstruction(REC_GETREG(NextReg));
		CreateRecInstruction(REC_MOVE(NextReg, rS));
		CreateRecInstruction(REC_SHL(NextReg, REG_SPECIAL | REG_ECX));
		CreateRecInstruction(REC_MOVE(rA, NextReg));
		CreateRecInstruction(REC_FREEREG(NextReg));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_ECX, rB));
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_SHL(rA, REG_SPECIAL | REG_ECX));
	}

	CreateRecInstruction(REC_JMP(Label2));
	CreateRecInstruction(REC_JUMPLABEL(Label1));
	CreateRecInstruction(REC_MOVEI(rA, 0));
	CreateRecInstruction(REC_JUMPLABEL(Label2));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_ECX));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));

	//this is required as rS may not be read so force it to clear so it is re-read from the gpr
	if((rS != rB) && (rS != rA))
		CreateRecInstruction(REC_FREEREG(rS));

	CreateRecInstruction(REC_TURNONCACHING());
}

GekkoRecOp(SRAWI)
{
	u32 sh = SH, mask = BIT_0;
	u32 i;
	u32 JumpLabelID1 = NextJumpID;
	u32 JumpLabelID2 = NextJumpID;

    if( sh == 0 )
	{
		i = 1;
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_CLEARCARRY(0));
		CreateRecInstruction(REC_UPDATECARRY(0));
	}
    else
    {
		CreateRecInstruction(REC_CMPI(rS, 0x80000000));
		CreateRecInstruction(REC_JA(JumpLabelID1));
		CreateRecInstruction(REC_CLEARCARRY(0));
		CreateRecInstruction(REC_JMP(JumpLabelID2));
		CreateRecInstruction(REC_JUMPLABEL(JumpLabelID1));
		CreateRecInstruction(REC_SETCARRY(0));
		CreateRecInstruction(REC_JUMPLABEL(JumpLabelID2));
		CreateRecInstruction(REC_UPDATECARRY(0));

		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_SARI(rA, sh));

		i = 0;
    }

    if(RC)
		CreateRecInstruction(REC_UPDATECR0(i));
}

GekkoRecOp(SRW)
{
	u32 Label1 = NextJumpID;
	u32 Label2 = NextJumpID;
	u32 NextReg = NextGetRegID;

#pragma todo("Implement caching code for jumps so functions like SRW can auto-detect known value shifts")

	CreateRecInstruction(REC_TURNOFFCACHING());
	CreateRecInstruction(REC_STOREREG(REG_ECX));
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_ECX));
	CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_ECX, rB));
	CreateRecInstruction(REC_ANDI(REG_SPECIAL | REG_ECX, 0x20));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_ECX));		//force ECX clear
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_ECX));
	CreateRecInstruction(REC_JNZ(Label1));

	if((rS == rB) || (rA == rB))
	{
		CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_ECX, rB));
		CreateRecInstruction(REC_GETREG(NextReg));
		CreateRecInstruction(REC_MOVE(NextReg, rS));
		CreateRecInstruction(REC_SHR(NextReg, REG_SPECIAL | REG_ECX));
		CreateRecInstruction(REC_MOVE(rA, NextReg));
		CreateRecInstruction(REC_FREEREG(NextReg));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_ECX, rB));
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_SHR(rA, REG_SPECIAL | REG_ECX));
	}

	CreateRecInstruction(REC_JMP(Label2));
	CreateRecInstruction(REC_JUMPLABEL(Label1));
	CreateRecInstruction(REC_MOVEI(rA, 0));
	CreateRecInstruction(REC_JUMPLABEL(Label2));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_ECX));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));

	//this is required as rS may not be read so force it to clear so it is re-read from the gpr
	if((rS != rB) && (rS != rA))
		CreateRecInstruction(REC_FREEREG(rS));

	CreateRecInstruction(REC_TURNONCACHING());

/*
	CreateRecInstruction(REC_STOREREG(REG_ECX));
	CreateRecInstruction(REC_GETREG(REG_SPECIAL | REG_ECX));
	CreateRecInstruction(REC_MOVE(REG_SPECIAL | REG_ECX, rB));

	if((rS == rB) || (rA == rB))
	{
		CreateRecInstruction(REC_GETREG(NextReg));
		CreateRecInstruction(REC_MOVE(NextReg, rS));
		CreateRecInstruction(REC_SHR(NextReg, REG_SPECIAL | REG_ECX));
		CreateRecInstruction(REC_MOVE(rA, NextReg));
		CreateRecInstruction(REC_FREEREG(NextReg));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_SHR(rA, REG_SPECIAL | REG_ECX));
	}

	CreateRecInstruction(REC_ANDI(REG_SPECIAL | REG_ECX, 0x20));
	CreateRecInstruction(REC_FREEREG(REG_SPECIAL | REG_ECX));		//force ECX clear
	CreateRecInstruction(REC_JZ(Label1));
	CreateRecInstruction(REC_MOVEI(rA, 0));
	CreateRecInstruction(REC_JUMPLABEL(Label1));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));

	//this is required as rS may not be read so force it to clear so it is re-read from the gpr
	if((rS != rB) && (rS != rA))
		CreateRecInstruction(REC_FREEREG(rS));
*/
}

GekkoRecOp(SUBF)
{
	u32 x = 0;

	if(rD == rA)
	{
		if(rD == rB)
		{
			CreateRecInstruction(REC_MOVEI(rD, 0));
		}
		else
		{
			CreateRecInstruction(REC_GETTEMPREG(0));
			CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
			CreateRecInstruction(REC_SUB(TEMP_REG, rA));
			CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
			CreateRecInstruction(REC_FREETEMPREG());
			x = 1;
		}
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_SUB(rD, rA));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(x));
}

GekkoRecOp(SUBFO)
{
	u32 x = 0;

	if(rD == rA)
	{
		if(rD == rB)
		{
			CreateRecInstruction(REC_MOVEI(rD, 0));
		}
		else
		{
			CreateRecInstruction(REC_GETTEMPREG(0));
			CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
			CreateRecInstruction(REC_SUB(TEMP_REG, rA));
			CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
			CreateRecInstruction(REC_FREETEMPREG());
		}
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_SUB(rD, rA));
	}

	CreateRecInstruction(REC_UPDATEXERSOOV(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(SUBFC)
{
	u32 li_x = 0;

	if(rD == rA)
	{
		if(rD == rB)
		{
			CreateRecInstruction(REC_MOVEI(rD, 0));
			CreateRecInstruction(REC_SETCARRY(rD));
		}
		else
		{
			CreateRecInstruction(REC_GETTEMPREG(0));
			CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
			CreateRecInstruction(REC_SUB(TEMP_REG, rA));
			CreateRecInstruction(REC_COMPLIMENTCARRY(TEMP_REG));
			CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
			CreateRecInstruction(REC_FREETEMPREG());
		}
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_SUB(rD, rA));
		CreateRecInstruction(REC_COMPLIMENTCARRY(rD));
	}

	CreateRecInstruction(REC_UPDATECARRY(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(SUBFCO)
{
	u32 li_x = 0;

	if(rD == rA)
	{
		if(rD == rB)
		{
			CreateRecInstruction(REC_MOVEI(rD, 0));
			CreateRecInstruction(REC_SETCARRY(rD));
		}
		else
		{
			CreateRecInstruction(REC_GETTEMPREG(0));
			CreateRecInstruction(REC_MOVE(TEMP_REG, rB));
			CreateRecInstruction(REC_SUB(TEMP_REG, rA));
			CreateRecInstruction(REC_COMPLIMENTCARRY(TEMP_REG));
			CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
			CreateRecInstruction(REC_FREETEMPREG());
		}
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rB));
		CreateRecInstruction(REC_SUB(rD, rA));
		CreateRecInstruction(REC_COMPLIMENTCARRY(rD));
	}

	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(3));
}

GekkoRecOp(SUBFE)
{
	if(rA == rD)
	{
		if(rD == rB)
		{
			CreateRecInstruction(REC_MOVEI(rD, 0xFFFFFFFF));
			CreateRecInstruction(REC_CLEARCARRY(rD));
		}
		else
		{
			CreateRecInstruction(REC_MOVE(rD, rA));
			CreateRecInstruction(REC_NOT(rD));
			CreateRecInstruction(REC_ADD(rD, rB));
		}
	}
	else if(rD == rB)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_NOT(TEMP_REG));
		CreateRecInstruction(REC_ADD(TEMP_REG, rB));
		CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_NOT(rD));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));
	
	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(SUBFEO)
{
	if(rD == rA)
	{
		if(rD == rB)
		{
			CreateRecInstruction(REC_MOVEI(rD, 0xFFFFFFFF));
			CreateRecInstruction(REC_CLEARCARRY(rD));
		}
		else
		{
			CreateRecInstruction(REC_MOVE(rD, rA));
			CreateRecInstruction(REC_NOT(rD));
			CreateRecInstruction(REC_ADD(rD, rB));
		}
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_NOT(rD));
		CreateRecInstruction(REC_ADD(rD, rB));
	}

	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(3));
}

GekkoRecOp(SUBFIC)
{
	if(rD == rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVEI(TEMP_REG, SIMM));
		CreateRecInstruction(REC_SUB(TEMP_REG, rA));
		CreateRecInstruction(REC_COMPLIMENTCARRY(TEMP_REG));
		CreateRecInstruction(REC_MOVE(rD, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
	{
		CreateRecInstruction(REC_MOVEI(rD, SIMM));
		CreateRecInstruction(REC_SUB(rD, rA));
		CreateRecInstruction(REC_COMPLIMENTCARRY(rD));
	}

	CreateRecInstruction(REC_UPDATECARRY(rD));
}

GekkoRecOp(SUBFME)
{
	if(rD == rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_NOT(rD));
		CreateRecInstruction(REC_ADDI(rD, 0xFFFFFFFF));
	}
	else
	{
		CreateRecInstruction(REC_MOVEI(rD, 0xFFFFFFFF));
		CreateRecInstruction(REC_SUB(rD, rA));
		CreateRecInstruction(REC_COMPLIMENTCARRY(rD));
	}
	
	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));
	
	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(SUBFMEO)
{
	if(rD == rA)
	{
		CreateRecInstruction(REC_MOVE(rD, rA));
		CreateRecInstruction(REC_NOT(rD));
		CreateRecInstruction(REC_ADDI(rD, 0xFFFFFFFF));
	}
	else
	{
		CreateRecInstruction(REC_MOVEI(rD, 0xFFFFFFFF));
		CreateRecInstruction(REC_SUB(rD, rA));
		CreateRecInstruction(REC_COMPLIMENTCARRY(rD));
	}

	CreateRecInstruction(REC_UPDATETEMPCARRY(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));
	CreateRecInstruction(REC_UPDATECARRYWTEMP(rD));
	
	if(RC)
		CreateRecInstruction(REC_UPDATECR0(3));
}

GekkoRecOp(SUBFZE)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_NOT(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(1));
}

GekkoRecOp(SUBFZEO)
{
	CreateRecInstruction(REC_MOVE(rD, rA));
	CreateRecInstruction(REC_NOT(rD));
	CreateRecInstruction(REC_ADDCARRY(rD));
	CreateRecInstruction(REC_UPDATECARRY(rD));
	CreateRecInstruction(REC_UPDATEXERSOOV(rD));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(2));
}

GekkoRecOp(XOR)
{
	if(rA == rB)
	{
		CreateRecInstruction(REC_MOVE(rA, rB));
		CreateRecInstruction(REC_XOR(rA, rS));
	}
	else
	{
		CreateRecInstruction(REC_MOVE(rA, rS));
		CreateRecInstruction(REC_XOR(rA, rB));
	}

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(XORI)
{
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_XORI(rA, UIMM));
}

GekkoRecOp(XORIS)
{
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_XORI(rA, (UIMM << 16)));
}

GekkoRecOp(RLWIMI)
{
	u32 m = RotMask[ MB ][ ME ];
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rS));
	CreateRecInstruction(REC_ANDI(rA, ~m));
	CreateRecInstruction(REC_ROLI(TEMP_REG, SH));
	CreateRecInstruction(REC_ANDI(TEMP_REG, m));
	CreateRecInstruction(REC_OR(rA, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(RLWINM)
{
	u32 m = RotMask[ MB ][ ME ];
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_ROLI(rA, SH));
	CreateRecInstruction(REC_ANDI(rA, m));
	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(RLWNM)
{
	u32 m = RotMask[ MB ][ ME ];
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_ROLI(rA, SH));
	CreateRecInstruction(REC_ANDI(rA, m));
	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(EXTSB)
{
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_SHLI(rA, 24));
	CreateRecInstruction(REC_SARI(rA, 24));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}

GekkoRecOp(EXTSH)
{
	CreateRecInstruction(REC_MOVE(rA, rS));
	CreateRecInstruction(REC_SHLI(rA, 16));
	CreateRecInstruction(REC_SARI(rA, 16));

	if(RC)
		CreateRecInstruction(REC_UPDATECR0(0));
}