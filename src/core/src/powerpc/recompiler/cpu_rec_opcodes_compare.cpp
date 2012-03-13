////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_opcodes_compare.cpp
// DESC:		Recompiler Compare and Compare Register Opcodes
// CREATED:		Feb. 26, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"

GekkoRecOp(CMP)
{
	CreateRecInstruction(REC_CMPUPDATECRSIGNED(rA, rB));
	LastInstruction->TempData = CRFD;
}

GekkoRecOp(CMPI)
{
	CreateRecInstruction(REC_CMPUPDATECRSIGNEDI(rA, SIMM));
	LastInstruction->TempData = CRFD;
}

GekkoRecOp(CMPL)
{
	CreateRecInstruction(REC_CMPUPDATECR(rA, rB));
	LastInstruction->TempData = CRFD;
}

GekkoRecOp(CMPLI)
{
	CreateRecInstruction(REC_CMPUPDATECRI(rA, UIMM));
	LastInstruction->TempData = CRFD;
}

GekkoRecOp(CNTLZW)
{
	u32 Jump1 = NextJumpID;
	u32 Jump2 = NextJumpID;
	u32 GetReg = NextGetRegID;

	u32 TempRegUsed;
	u32 OutReg;

	CreateRecInstruction(REC_TURNOFFCACHING());

	if(rA == rS)
	{
		TempRegUsed = 1;
		OutReg = TEMP_REG;
		CreateRecInstruction(REC_GETTEMPREG(0));
	}
	else
	{
		TempRegUsed = 0;
		OutReg = rA;
	}

	CreateRecInstruction(REC_MOVEI(OutReg, -1));

	CreateRecInstruction(REC_GETREG(GetReg));
	CreateRecInstruction(REC_MOVE(GetReg, rS));

	//loop as long as carry is not set
	CreateRecInstruction(REC_JUMPLABEL(Jump1));
	CreateRecInstruction(REC_ADDI(OutReg, 1));
	CreateRecInstruction(REC_CMPI(OutReg, 32));
	CreateRecInstruction(REC_JAE(Jump2));
	CreateRecInstruction(REC_SHLI(GetReg, 1));
	CreateRecInstruction(REC_JNC(Jump1));

	CreateRecInstruction(REC_JUMPLABEL(Jump2));

	CreateRecInstruction(REC_FREEREG(GetReg));

	if(TempRegUsed)
	{
		CreateRecInstruction(REC_MOVE(rA, OutReg));
		CreateRecInstruction(REC_FREETEMPREG());

		if(RC)
			CreateRecInstruction(REC_UPDATECR0(1));
	}
	else
	{
		if(RC)
			CreateRecInstruction(REC_UPDATECR0(5));
	}

	CreateRecInstruction(REC_TURNONCACHING());
}

GekkoRecOp(CRAND)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBA));
	CreateRecInstruction(REC_AND(TEMP_REG, REG_PPC_CR + CRBB));
	CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(CRANDC)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBB));
	CreateRecInstruction(REC_NOT(TEMP_REG));
	CreateRecInstruction(REC_AND(TEMP_REG, REG_PPC_CR + CRBA));
	CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(CREQV)
{
	//CRSET
	if(CRBA == CRBB)
	{
		CreateRecInstruction(REC_MOVEI(REG_PPC_CR + CRBD, 1));
	}
	else
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBA));
		CreateRecInstruction(REC_NOT(TEMP_REG));
		CreateRecInstruction(REC_XOR(TEMP_REG, REG_PPC_CR + CRBB));
		CreateRecInstruction(REC_ANDI(REG_PPC_CR + CRBD, 1));
		CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
}

GekkoRecOp(CRNAND)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBA));
	CreateRecInstruction(REC_NOT(TEMP_REG));
	CreateRecInstruction(REC_AND(TEMP_REG, REG_PPC_CR + CRBB));
	CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(CRNOR)
{
	//CRNOT
	if(CRBA == CRBB)
	{
		CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, REG_PPC_CR + CRBA));
		CreateRecInstruction(REC_NOT(REG_PPC_CR + CRBD));
	}
	else
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBA));
		CreateRecInstruction(REC_OR(TEMP_REG, REG_PPC_CR + CRBB));
		CreateRecInstruction(REC_NOT(TEMP_REG));
		CreateRecInstruction(REC_ANDI(TEMP_REG, 1));
		CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
}

GekkoRecOp(CROR)
{
	//CRMOVE
	if(CRBA == CRBB)
		CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, REG_PPC_CR + CRBA));
	else
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBA));
		CreateRecInstruction(REC_OR(TEMP_REG, REG_PPC_CR + CRBB));
		CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
}

GekkoRecOp(CRORC)
{
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBB));
	CreateRecInstruction(REC_NOT(TEMP_REG));
	CreateRecInstruction(REC_OR(TEMP_REG, REG_PPC_CR + CRBA));
	CreateRecInstruction(REC_ANDI(TEMP_REG, 1));
	CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(CRXOR)
{
	//CRCLR
	if(CRBA == CRBB)
		CreateRecInstruction(REC_MOVEI(REG_PPC_CR + CRBD, 0));
	else
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + CRBA));
		CreateRecInstruction(REC_XOR(TEMP_REG, REG_PPC_CR + CRBB));
		CreateRecInstruction(REC_MOVE(REG_PPC_CR + CRBD, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
}

GekkoRecOp(MFCR)
{
	u32	x;

	CreateRecInstruction(REC_TURNOFFCACHING());
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_CR + 0));

	for(x = 1; x < 32; x++)
	{
		CreateRecInstruction(REC_SHLI(TEMP_REG, 1));
		CreateRecInstruction(REC_OR(TEMP_REG, REG_PPC_CR + x));
	}

	CreateRecInstruction(REC_MOVE(rD, TEMP_REG));

	CreateRecInstruction(REC_FREETEMPREG());
	CreateRecInstruction(REC_TURNONCACHING());
}

GekkoRecOp(MTCRF)
{
	u32 crm = 0;
	s32 x;
	s32 LastShift;

	crm = ((CRM & 0x80) ? 0xf0000000 : 0) | ((CRM & 0x40) ? 0x0f000000:0) | ((CRM & 0x20) ? 0x00f00000 : 0) | ((crm & 0x10 )? 0x000f0000 : 0) |
	      ((CRM & 0x08) ? 0x0000f000 : 0) | ((CRM & 0x04) ? 0x00000f00:0) | ((CRM & 0x02) ? 0x000000f0 : 0) | ((crm & 0x01) ? 0x0000000f : 0);

	if(crm)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rS));

		LastShift = 31;
		for(x = 31; (x >= 0) && crm; x--)
		{
			//if the bit is set then update with RRS, else, leave it alone
			if(crm & 0x00000001)
			{
				CreateRecInstruction(REC_SHRI(TEMP_REG, LastShift - x));
				CreateRecInstruction(REC_MOVE(REG_PPC_CR + x, TEMP_REG));
				CreateRecInstruction(REC_ANDI(REG_PPC_CR + x, 1));
				LastShift = x;
			}

			crm >>= 1;
		}

		CreateRecInstruction(REC_FREETEMPREG());
	}
}
