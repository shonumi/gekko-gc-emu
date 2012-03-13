////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_opcodes_math_fpu.cpp
// DESC:		Recompiler fpu math opcodes
// CREATED:		May 22, 2009
////////////////////////////////////////////////////////////
// Copyright (c) 2009 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "powerpc/cpu_core_regs.h"

t128 GekkoCPURecompiler::FPR_SignBit;
t128 GekkoCPURecompiler::FPR_SignBitLow;
t128 GekkoCPURecompiler::FPR_SignBitInverse;
t128 GekkoCPURecompiler::FPR_SignBitInverseLow;
t128 GekkoCPURecompiler::FPR_Negative1;
t128 GekkoCPURecompiler::FPR_Negative1Low;


GekkoRecOp(FABS)
{
	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frB));
	CreateRecInstruction(REC_PANDM(frD, (u32)&GekkoCPURecompiler::FPR_SignBitInverseLow));
}

GekkoRecOp(FADD)
{
	if(frD == frB)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frB));
		CreateRecInstruction(REC_ADDSD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVAPD(frD, frD));
		CreateRecInstruction(REC_MOVSD(frD, frA));
		CreateRecInstruction(REC_ADDSD(frD, frB));
	}
}

GekkoRecOp(FADDS)
{
	u32 Jump1 = NextJumpID;

	if(frD == frB)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frB));
		CreateRecInstruction(REC_ADDSD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVAPD(frD, frD));
		CreateRecInstruction(REC_MOVSD(frD, frA));
		CreateRecInstruction(REC_ADDSD(frD, frB));
	}

	CreateRecInstruction(REC_CVTSD2SS(frD, frD));
	CreateRecInstruction(REC_CVTSS2SD(frD, frD));

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, frD));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));

	CreateRecInstruction(REC_FREETEMPREG());
}

/*
GekkoRecOp(FCMPO)
{
}

GekkoRecOp(FCMPU)
{
}

GekkoRecOp(FCTIW)
{
}

GekkoRecOp(FCTIWZ)
{
}
*/

GekkoRecOp(FDIV)
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

	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frA));
	CreateRecInstruction(REC_DIVSD(frD, Reg2));

	if((frD == frB) && (frD != frA))
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(FDIVS)
{
	u32 Jump1 = NextJumpID;

	u32		Reg2;

	if((frD == frB) && (frD != frA))
	{
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		Reg2 = TEMP_FPU_REG;
	}
	else
		Reg2 = frB;

	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frA));
	CreateRecInstruction(REC_DIVSD(frD, Reg2));

	CreateRecInstruction(REC_CVTSD2SS(frD, frD));
	CreateRecInstruction(REC_CVTSS2SD(frD, frD));

	if((frD == frB) && (frD != frA))
		CreateRecInstruction(REC_FREETEMPFPUREG(0));

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, frD));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));

	CreateRecInstruction(REC_FREETEMPREG());
}


GekkoRecOp(FMADD)
{
	if(frD == frB)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frB));
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVSD(TEMP_FPU_REG, frA));
		CreateRecInstruction(REC_MULSD(TEMP_FPU_REG, frC));
		CreateRecInstruction(REC_ADDSD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else
	{
		if(frD == frA && frD != frC)
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frA));
			CreateRecInstruction(REC_MULSD(frD, frC));
			CreateRecInstruction(REC_ADDSD(frD, frB));
		}
		else
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frC));
			CreateRecInstruction(REC_MULSD(frD, frA));
			CreateRecInstruction(REC_ADDSD(frD, frB));
		}
	}
}

GekkoRecOp(FMADDS)
{
	u32 Jump1 = NextJumpID;

	if(frD == frB)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frB));
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVSD(TEMP_FPU_REG, frA));
		CreateRecInstruction(REC_MULSD(TEMP_FPU_REG, frC));
		CreateRecInstruction(REC_ADDSD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else
	{
		if(frD == frA && frD != frC)
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frA));
			CreateRecInstruction(REC_MULSD(frD, frC));
			CreateRecInstruction(REC_ADDSD(frD, frB));
		}
		else
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frC));
			CreateRecInstruction(REC_MULSD(frD, frA));
			CreateRecInstruction(REC_ADDSD(frD, frB));
		}
	}

	CreateRecInstruction(REC_CVTSD2SS(frD, frD));
	CreateRecInstruction(REC_CVTSS2SD(frD, frD));

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, frD));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));

	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(FMR)
{
	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frB));
}

GekkoRecOp(FMSUB)
{
	if(frD == frB)
	{
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(frD, frD));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVSD(frD, frA));
		CreateRecInstruction(REC_MULSD(frD, frC));
		CreateRecInstruction(REC_SUBSD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else
	{
		if(frD == frA && frD != frC)
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frA));
			CreateRecInstruction(REC_MULSD(frD, frC));
			CreateRecInstruction(REC_SUBSD(frD, frB));
		}
		else
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frC));
			CreateRecInstruction(REC_MULSD(frD, frA));
			CreateRecInstruction(REC_SUBSD(frD, frB));
		}
	}
}

GekkoRecOp(FMSUBS)
{
	u32 Jump1 = NextJumpID;

	if(frD == frB)
	{
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(frD, frD));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		CreateRecInstruction(REC_MOVSD(frD, frA));
		CreateRecInstruction(REC_MULSD(frD, frC));
		CreateRecInstruction(REC_SUBSD(frD, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
	}
	else
	{
		if(frD == frA && frD != frC)
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frA));
			CreateRecInstruction(REC_MULSD(frD, frC));
			CreateRecInstruction(REC_SUBSD(frD, frB));
		}
		else
		{
			CreateRecInstruction(REC_MOVAPD(frD, frD));
			CreateRecInstruction(REC_MOVSD(frD, frC));
			CreateRecInstruction(REC_MULSD(frD, frA));
			CreateRecInstruction(REC_SUBSD(frD, frB));
		}
	}

	CreateRecInstruction(REC_CVTSD2SS(frD, frD));
	CreateRecInstruction(REC_CVTSS2SD(frD, frD));

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, frD));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));

	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(FMUL)
{
	if(frD == frC)
	{
		CreateRecInstruction(REC_MOVAPD(frD, frC));
		CreateRecInstruction(REC_MULSD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVAPD(frD, frD));
		CreateRecInstruction(REC_MOVSD(frD, frA));
		CreateRecInstruction(REC_MULSD(frD, frC));
	}
}

GekkoRecOp(FMULS)
{
	u32 Jump1 = NextJumpID;

	CreateRecInstruction(REC_MOVAPD(frD, frD));
	if(frD == frC)
	{
		CreateRecInstruction(REC_MOVSD(frD, frC));
		CreateRecInstruction(REC_MULSD(frD, frA));
	}
	else
	{
		CreateRecInstruction(REC_MOVSD(frD, frA));
		CreateRecInstruction(REC_MULSD(frD, frC));
	}

	CreateRecInstruction(REC_CVTSD2SS(frD, frD));
	CreateRecInstruction(REC_CVTSS2SD(frD, frD));

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, frD));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));

	CreateRecInstruction(REC_FREETEMPREG());
}

/*
GekkoRecOp(FNABS)
{
}
*/
GekkoRecOp(FNEG)
{
	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frB));
	CreateRecInstruction(REC_PXORM(frD, (u32)&GekkoCPURecompiler::FPR_SignBitLow));
}
/*
GekkoRecOp(FNMADD)
{
}

GekkoRecOp(FNMADDS)
{
}

GekkoRecOp(FNMSUB)
{
}

GekkoRecOp(FNMSUBS)
{
}

GekkoRecOp(FRES)
{
}
*/
GekkoRecOp(FRSP)
{
	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frB));
	CreateRecInstruction(REC_CVTSD2SS(frD, frD));
	CreateRecInstruction(REC_CVTSS2SD(frD, frD));
}

/*
GekkoRecOp(FRSQRTE)
{
}

GekkoRecOp(FSEL)
{
}

GekkoRecOp(FSQRT)
{
}

GekkoRecOp(FSQRTS)
{
}
*/

GekkoRecOp(FSUB)
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

	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frA));
	CreateRecInstruction(REC_SUBSD(frD, Reg2));

	if((frD == frB) && (frD != frA))
		CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(FSUBS)
{
	u32 Jump1 = NextJumpID;
	u32	Reg2;

	if((frD == frB) && (frD != frA))
	{
		CreateRecInstruction(REC_GETTEMPFPUREG(0));
		CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frB));
		Reg2 = TEMP_FPU_REG;
	}
	else
		Reg2 = frB;

	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, frA));
	CreateRecInstruction(REC_SUBSD(frD, Reg2));

	CreateRecInstruction(REC_CVTSD2SS(frD, frD));
	CreateRecInstruction(REC_CVTSS2SD(frD, frD));

	if((frD == frB) && (frD != frA))
		CreateRecInstruction(REC_FREETEMPFPUREG(0));

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, frD));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));

	CreateRecInstruction(REC_FREETEMPREG());
}