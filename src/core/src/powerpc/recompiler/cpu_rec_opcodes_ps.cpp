////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_opcodes_ps.cpp
// DESC:		Recompiler Paired Single Opcodes
// CREATED:		Jun. 30, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "../cpu_core_regs.h"

GekkoRecOp(PS_ABS)
{
	CreateRecInstruction(REC_MOVAPD(frD, frB));
	CreateRecInstruction(REC_PANDM(frD, (u32)&GekkoCPURecompiler::FPR_SignBitInverse));
}

GekkoRecOp(PS_ADD)
{
	if(frD == frB)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frB));
		CreateRecInstruction(REC_ADDPD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_ADDPD(frD, frB));
	}
}

/*
GekkoRecOp(PS_CMPO0)
{
}

GekkoRecOp(PS_CMPO1)
{
}

GekkoRecOp(PS_CMPU0)
{
}

GekkoRecOp(PS_CMPU1)
{
}
*/

GekkoRecOp(PS_DIV)
{
	u32		Reg2;

	if((frD == frB) && (frD != frA))
	{
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		Reg2 = TEMP_FPU_REG;
	}
	else
		Reg2 = frB;

	CreateRecInstruction(REC_MOVAPD(frD, frA));
	CreateRecInstruction(REC_DIVPD(frD, Reg2));

	if(frD == frB)
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
}


GekkoRecOp(PS_MADD)
{
	u32		RegMatch;

	RegMatch = 0;
	if(frD == frB)
		RegMatch |= 1;
	if(frD == frC)
	{
		RegMatch |= 2;
	}

	if(!RegMatch)
	{
		//no match, frD == frA
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_MULPD(frD, frC));
		CreateRecInstruction(REC_ADDPD(frD, frB));
	}
	else if(RegMatch == 1)
	{
		//frD == frB, frD == frA & frB
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_MULPD(frD, frC));
		CreateRecInstruction(REC_ADDPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else if(RegMatch == 2)
	{
		//frD == frC, frD == frA & frC
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_ADDPD(frD, frB));
	}
	else
	{
		//frD == frB & frC, frD == frA, frB, frC
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_ADDPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
}


GekkoRecOp(PS_MADDS0)
{
	u32		RegMatch;

	RegMatch = 0;
	if(frD == frB)
		RegMatch |= 1;
	if(frD == frC)
	{
		RegMatch |= 2;
	}

	if(!RegMatch)
	{
		//no match, frD == frA
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frC));
		CreateRecInstruction(REC_SHUFPD(TEMP_FPU_REG, TEMP_FPU_REG));
		SetRecInstructionImm(0x00);
		CreateRecInstruction(REC_MULPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
		CreateRecInstruction(REC_ADDPD(frD, frB));
	}
	else if(RegMatch == 1)
	{
		//frD == frB, frD == frA & frB
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_GETTEMPFPUREG(1));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG | 1, frC));
		CreateRecInstruction(REC_SHUFPD(TEMP_FPU_REG | 1, TEMP_FPU_REG | 1));
		SetRecInstructionImm(0x00);
		CreateRecInstruction(REC_MULPD(frD, TEMP_FPU_REG | 1));
		CreateRecInstruction(REC_FREETEMPFPUREG(1));
		CreateRecInstruction(REC_ADDPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else if(RegMatch == 2)
	{
		//frD == frC, frD == frA & frC
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_SHUFPD(frD, frD));
		SetRecInstructionImm(0x00);
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_ADDPD(frD, frB));
	}
	else
	{
		//frD == frB & frC, frD == frA, frB, frC
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_SHUFPD(frD, frD));
		SetRecInstructionImm(0x00);
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_ADDPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}

	CreateRecInstruction(REC_CVTPD2PS(frD, frD));
	CreateRecInstruction(REC_CVTPS2PD(frD, frD));
}

GekkoRecOp(PS_MADDS1)
{
	u32		RegMatch;

	RegMatch = 0;
	if(frD == frB)
		RegMatch |= 1;
	if(frD == frC)
	{
		RegMatch |= 2;
	}

	if(!RegMatch)
	{
		//no match, frD == frA
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frC));
		CreateRecInstruction(REC_SHUFPD(TEMP_FPU_REG, TEMP_FPU_REG));
		SetRecInstructionImm(0x03);
		CreateRecInstruction(REC_MULPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
		CreateRecInstruction(REC_ADDPD(frD, frB));
	}
	else if(RegMatch == 1)
	{
		//frD == frB, frD == frA & frB
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_GETTEMPFPUREG(1));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG | 1, frC));
		CreateRecInstruction(REC_SHUFPD(TEMP_FPU_REG | 1, TEMP_FPU_REG | 1));
		SetRecInstructionImm(0x03);
		CreateRecInstruction(REC_MULPD(frD, TEMP_FPU_REG | 1));
		CreateRecInstruction(REC_FREETEMPFPUREG(1));
		CreateRecInstruction(REC_ADDPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else if(RegMatch == 2)
	{
		//frD == frC, frD == frA & frC
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_SHUFPD(frD, frD));
		SetRecInstructionImm(0x03);
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_ADDPD(frD, frB));
	}
	else
	{
		//frD == frB & frC, frD == frA, frB, frC
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_SHUFPD(frD, frD));
		SetRecInstructionImm(0x03);
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_ADDPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}

	CreateRecInstruction(REC_CVTPD2PS(frD, frD));
	CreateRecInstruction(REC_CVTPS2PD(frD, frD));
}


GekkoRecOp(PS_MERGE00)
{
	if(frD == frB)
	{
		CreateRecInstruction(REC_MOVDDUP(frD, frB));
		if(frD != frA)
			CreateRecInstruction(REC_MOVSD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_MOVLHPS(frD, frB));
	}
}

GekkoRecOp(PS_MERGE01)
{
	if((frD == frA) && (frD == frB))
		return;

	if(frD == frB)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frB));
		CreateRecInstruction(REC_MOVSD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_SHUFPD(frD, frB));
		SetRecInstructionImm(0x02);
	}
}
/*
GekkoRecOp(PS_MERGE10)
{
	f64 ps0b = PS0B;
	PS0D = PS1A;
	PS1D = ps0b;
}

GekkoRecOp(PS_MERGE11)
{
	//punpckhqdq if D = A
	PS0D = PS1A;
	PS1D = PS1B;
}
*/
GekkoRecOp(PS_MR)
{
	CreateRecInstruction(REC_MOVAPD(frD, frB));
}

GekkoRecOp(PS_MSUB)
{
	u32		RegMatch;

	RegMatch = 0;
	if(frD == frB)
		RegMatch |= 1;
	if(frD == frC)
	{
		RegMatch |= 2;
	}

	if(!RegMatch)
	{
		//no match or frD == frA
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_MULPD(frD, frC));
		CreateRecInstruction(REC_SUBPD(frD, frB));
	}
	else if(RegMatch == 1)
	{
		//frD == frB or frD == frA & frB
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_MULPD(frD, frC));
		CreateRecInstruction(REC_SUBPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else if(RegMatch == 2)
	{
		//frD == frC or frD == frA & frC
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_SUBPD(frD, frB));
	}
	else
	{
		//frD == frB & frC or frD == frA, frB, or frC
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_MULPD(frD, frA));
		CreateRecInstruction(REC_SUBPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
}

GekkoRecOp(PS_MUL)
{
	if(frD == frC)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_MULPD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVAPD(frD, frA));
		CreateRecInstruction(REC_MULPD(frD, frC));
	}
}

GekkoRecOp(PS_MULS0)
{
	if(rD != rA)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_SHUFPD(frD, frD));
		SetRecInstructionImm(0x00);
		CreateRecInstruction(REC_MULPD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frA));
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_SHUFPD(frD, frD));
		SetRecInstructionImm(0x00);
		CreateRecInstruction(REC_MULPD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
}

/*
GekkoRecOp(PS_MULS1)
{
}
*/
GekkoRecOp(PS_NEG)
{
	CreateRecInstruction(REC_MOVAPD(frD, frB));
	CreateRecInstruction(REC_PXORM(frD, (u32)&GekkoCPURecompiler::FPR_SignBit));
}
/*
GekkoRecOp(PS_NMADD)
{
}

GekkoRecOp(PS_NMSUB)
{
}
*/

GekkoRecOp(PS_SUB)
{
	u32		Reg2;

	if((frD == frB) && (frD != frA))
	{
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		Reg2 = TEMP_FPU_REG;
	}
	else
		Reg2 = frB;

	CreateRecInstruction(REC_MOVAPD(frD, frA));
	CreateRecInstruction(REC_SUBPD(frD, Reg2));

	if((frD == frB) && (frD != frA))
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

/*
GekkoRecOp(PS_RES)
{
}

GekkoRecOp(PS_RSQRTE)
{
}
*/

GekkoRecOp(PS_SUM0)
{
	CreateRecInstruction(REC_GETTEMPFPUREG(0));
	CreateRecInstruction(REC_MOVHLPS(TEMP_FPU_REG, frB));
	CreateRecInstruction(REC_ADDSD(TEMP_FPU_REG, frA));
	CreateRecInstruction(REC_MOVAPD(frD, frC));
	CreateRecInstruction(REC_MOVSD(frD, TEMP_FPU_REG));
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

/*
GekkoRecOp(PS_SUM1)
{
}

GekkoRecOp(PS_SEL)
{
}
*/