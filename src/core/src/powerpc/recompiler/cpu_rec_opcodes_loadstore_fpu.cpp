////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_opcodes_loadstore_fpu.cpp
// DESC:		Recompiler fpu load/store opcodes
// CREATED:		July 15, 2009
////////////////////////////////////////////////////////////
// Copyright (c) 2009 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "../cpu_core_regs.h"

const t128 GekkoCPURecompiler::ldScale64bit[] =
{
	{1.0f/(1 <<  0),  1.0f/(1 <<  0)},
	{1.0f/(1 <<  1),  1.0f/(1 <<  1)},
	{1.0f/(1 <<  2),  1.0f/(1 <<  2)},
	{1.0f/(1 <<  3),  1.0f/(1 <<  3)},
	{1.0f/(1 <<  4),  1.0f/(1 <<  4)},
	{1.0f/(1 <<  5),  1.0f/(1 <<  5)},
	{1.0f/(1 <<  6),  1.0f/(1 <<  6)},
	{1.0f/(1 <<  7),  1.0f/(1 <<  7)},
	{1.0f/(1 <<  8),  1.0f/(1 <<  8)},
	{1.0f/(1 <<  9),  1.0f/(1 <<  9)},
	{1.0f/(1 <<  10), 1.0f/(1 <<  10)},
	{1.0f/(1 <<  11), 1.0f/(1 <<  11)},
	{1.0f/(1 <<  12), 1.0f/(1 <<  12)},
	{1.0f/(1 <<  13), 1.0f/(1 <<  13)},
	{1.0f/(1 <<  14), 1.0f/(1 <<  14)},
	{1.0f/(1 <<  15), 1.0f/(1 <<  15)},
	{1.0f/(1 <<  16), 1.0f/(1 <<  16)},
	{1.0f/(1 <<  17), 1.0f/(1 <<  17)},
	{1.0f/(1 <<  18), 1.0f/(1 <<  18)},
	{1.0f/(1 <<  19), 1.0f/(1 <<  19)},
	{1.0f/(1 <<  20), 1.0f/(1 <<  20)},
	{1.0f/(1 <<  21), 1.0f/(1 <<  21)},
	{1.0f/(1 <<  22), 1.0f/(1 <<  22)},
	{1.0f/(1 <<  23), 1.0f/(1 <<  23)},
	{1.0f/(1 <<  24), 1.0f/(1 <<  24)},
	{1.0f/(1 <<  25), 1.0f/(1 <<  25)},
	{1.0f/(1 <<  26), 1.0f/(1 <<  26)},
	{1.0f/(1 <<  27), 1.0f/(1 <<  27)},
	{1.0f/(1 <<  28), 1.0f/(1 <<  28)},
	{1.0f/(1 <<  29), 1.0f/(1 <<  29)},
	{1.0f/(1 <<  30), 1.0f/(1 <<  30)},
	{1.0f/(1 <<  31), 1.0f/(1 <<  31)},
	{(1ULL << 32), (1ULL << 32)},
	{(1 << 31), (1 << 31)},
	{(1 << 30), (1 << 30)},
	{(1 << 29), (1 << 29)},
	{(1 << 28), (1 << 28)},
	{(1 << 27), (1 << 27)},
	{(1 << 26), (1 << 26)},
	{(1 << 25), (1 << 25)},
	{(1 << 24), (1 << 24)},
	{(1 << 23), (1 << 23)},
	{(1 << 22), (1 << 22)},
	{(1 << 21), (1 << 21)},
	{(1 << 20), (1 << 20)},
	{(1 << 19), (1 << 19)},
	{(1 << 18), (1 << 18)},
	{(1 << 17), (1 << 17)},
	{(1 << 16), (1 << 16)},
	{(1 << 15), (1 << 15)},
	{(1 << 14), (1 << 14)},
	{(1 << 13), (1 << 13)},
	{(1 << 12), (1 << 12)},
	{(1 << 11), (1 << 11)},
	{(1 << 10), (1 << 10)},
	{(1 << 9),  (1 << 9)},
	{(1 << 8),  (1 << 8)},
	{(1 << 7),  (1 << 7)},
	{(1 << 6),  (1 << 6)},
	{(1 << 5),  (1 << 5)},
	{(1 << 4),  (1 << 4)},
	{(1 << 3),  (1 << 3)},
	{(1 << 2),  (1 << 2)},
	{(1 << 1),  (1 << 1)},
};

const t128 GekkoCPURecompiler::ldScale64bitX1[] =
{
	{1.0f,  1.0f/(1 <<  0)},
	{1.0f,  1.0f/(1 <<  1)},
	{1.0f,  1.0f/(1 <<  2)},
	{1.0f,  1.0f/(1 <<  3)},
	{1.0f,  1.0f/(1 <<  4)},
	{1.0f,  1.0f/(1 <<  5)},
	{1.0f,  1.0f/(1 <<  6)},
	{1.0f,  1.0f/(1 <<  7)},
	{1.0f,  1.0f/(1 <<  8)},
	{1.0f,  1.0f/(1 <<  9)},
	{1.0f, 1.0f/(1 <<  10)},
	{1.0f, 1.0f/(1 <<  11)},
	{1.0f, 1.0f/(1 <<  12)},
	{1.0f, 1.0f/(1 <<  13)},
	{1.0f, 1.0f/(1 <<  14)},
	{1.0f, 1.0f/(1 <<  15)},
	{1.0f, 1.0f/(1 <<  16)},
	{1.0f, 1.0f/(1 <<  17)},
	{1.0f, 1.0f/(1 <<  18)},
	{1.0f, 1.0f/(1 <<  19)},
	{1.0f, 1.0f/(1 <<  20)},
	{1.0f, 1.0f/(1 <<  21)},
	{1.0f, 1.0f/(1 <<  22)},
	{1.0f, 1.0f/(1 <<  23)},
	{1.0f, 1.0f/(1 <<  24)},
	{1.0f, 1.0f/(1 <<  25)},
	{1.0f, 1.0f/(1 <<  26)},
	{1.0f, 1.0f/(1 <<  27)},
	{1.0f, 1.0f/(1 <<  28)},
	{1.0f, 1.0f/(1 <<  29)},
	{1.0f, 1.0f/(1 <<  30)},
	{1.0f, 1.0f/(1 <<  31)},
	{1.0f, (1ULL << 32)},
	{1.0f, (1 << 31)},
	{1.0f, (1 << 30)},
	{1.0f, (1 << 29)},
	{1.0f, (1 << 28)},
	{1.0f, (1 << 27)},
	{1.0f, (1 << 26)},
	{1.0f, (1 << 25)},
	{1.0f, (1 << 24)},
	{1.0f, (1 << 23)},
	{1.0f, (1 << 22)},
	{1.0f, (1 << 21)},
	{1.0f, (1 << 20)},
	{1.0f, (1 << 19)},
	{1.0f, (1 << 18)},
	{1.0f, (1 << 17)},
	{1.0f, (1 << 16)},
	{1.0f, (1 << 15)},
	{1.0f, (1 << 14)},
	{1.0f, (1 << 13)},
	{1.0f, (1 << 12)},
	{1.0f, (1 << 11)},
	{1.0f, (1 << 10)},
	{1.0f,  (1 << 9)},
	{1.0f,  (1 << 8)},
	{1.0f,  (1 << 7)},
	{1.0f,  (1 << 6)},
	{1.0f,  (1 << 5)},
	{1.0f,  (1 << 4)},
	{1.0f,  (1 << 3)},
	{1.0f,  (1 << 2)},
	{1.0f,  (1 << 1)},
};

const t128 GekkoCPURecompiler::FPR1Float = {1.0f, 1.0f};

GekkoRecOp(LFD)
{
//	FBRD = (rA) ? Memory_Read64( RRA + SIMM ) : Memory_Read64( SIMM );

	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
		CreateRecInstruction(REC_READMEM64(TEMP_FPU_REG, TEMP_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_READMEM64I(TEMP_FPU_REG, SIMM));

	CreateRecInstruction(REC_MOVSD(frD, TEMP_FPU_REG));
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(LFS)
{
	u32 Jump1 = NextJumpID;
	u32 Jump2 = NextJumpID;

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVEI(TEMP_REG, SIMM));

	if(rA)
		CreateRecInstruction(REC_ADD(TEMP_REG, rA));

	CreateRecInstruction(REC_READMEM32(TEMP_REG, TEMP_REG));
	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	CreateRecInstruction(REC_MOVD_TOFPU(TEMP_FPU_REG, TEMP_REG));
	CreateRecInstruction(REC_CVTPS2PD(TEMP_FPU_REG, TEMP_FPU_REG));

	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, TEMP_FPU_REG));
	CreateRecInstruction(REC_JMP(Jump2));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));
	//make sure the high 64bits are still proper
	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, TEMP_FPU_REG));

	CreateRecInstruction(REC_JUMPLABEL(Jump2));
	CreateRecInstruction(REC_FREETEMPREG());
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(LFSU)
{
	u32 Jump1 = NextJumpID;
	u32 Jump2 = NextJumpID;

	CreateRecInstruction(REC_ADDI(rA, SIMM));
	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_MOVE(TEMP_REG, rA));

	CreateRecInstruction(REC_READMEM32(TEMP_REG, TEMP_REG));

	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	CreateRecInstruction(REC_MOVD_TOFPU(TEMP_FPU_REG, TEMP_REG));
	CreateRecInstruction(REC_CVTPS2PD(TEMP_FPU_REG, TEMP_FPU_REG));

	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, TEMP_FPU_REG));
	CreateRecInstruction(REC_JMP(Jump2));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));
	//make sure the high 64bits are still proper
	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, TEMP_FPU_REG));

	CreateRecInstruction(REC_JUMPLABEL(Jump2));
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
	CreateRecInstruction(REC_FREETEMPREG());
}

GekkoRecOp(LFSX)
{
	u32 Jump1 = NextJumpID;
	u32 Jump2 = NextJumpID;

	CreateRecInstruction(REC_GETTEMPREG(0));
	
	CreateRecInstruction(REC_MOVE(TEMP_REG, rB));

	if(rA)
		CreateRecInstruction(REC_ADD(TEMP_REG, rA));

	CreateRecInstruction(REC_READMEM32(TEMP_REG, TEMP_REG));
	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	CreateRecInstruction(REC_MOVD_TOFPU(TEMP_FPU_REG, TEMP_REG));
	CreateRecInstruction(REC_CVTPS2PD(TEMP_FPU_REG, TEMP_FPU_REG));

	CreateRecInstruction(REC_MOVE(TEMP_REG, REG_PPC_SPR | I_HID2));
	CreateRecInstruction(REC_ANDI(TEMP_REG, HID2_PSE));
	CreateRecInstruction(REC_JZ(Jump1));
	//PS1 = PS0
	CreateRecInstruction(REC_MOVDDUP(frD, TEMP_FPU_REG));
	CreateRecInstruction(REC_JMP(Jump2));

	CreateRecInstruction(REC_JUMPLABEL(Jump1));
	//make sure the high 64bits are still proper
	CreateRecInstruction(REC_MOVAPD(frD, frD));
	CreateRecInstruction(REC_MOVSD(frD, TEMP_FPU_REG));

	CreateRecInstruction(REC_JUMPLABEL(Jump2));
	CreateRecInstruction(REC_FREETEMPREG());
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(STFD)
{
	CreateRecInstruction(REC_GETTEMPFPUREG(0));
	CreateRecInstruction(REC_MOVAPD(TEMP_FPU_REG, frS));
	if(rA)
	{
		CreateRecInstruction(REC_GETTEMPREG(0));
		CreateRecInstruction(REC_MOVE(TEMP_REG, rA));
		CreateRecInstruction(REC_ADDI(TEMP_REG, SIMM));
		CreateRecInstruction(REC_WRITEMEM64(TEMP_REG, TEMP_FPU_REG));
		CreateRecInstruction(REC_FREETEMPREG());
	}
	else
		CreateRecInstruction(REC_WRITEMEM64I(SIMM, TEMP_FPU_REG));

	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(STFS)
{
	CreateRecInstruction(REC_GETREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_GETREG(REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_MOVEI(REG_ECX | REG_SPECIAL, SIMM));

	if(rA)
		CreateRecInstruction(REC_ADD(REG_ECX | REG_SPECIAL, rA));

	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	CreateRecInstruction(REC_CVTPD2PS(TEMP_FPU_REG, frS));
	CreateRecInstruction(REC_MOVD_FROMFPU(REG_EDX | REG_SPECIAL, TEMP_FPU_REG));

	CreateRecInstruction(REC_WRITEMEM32(REG_ECX | REG_SPECIAL, REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_FREEREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_FREEREG(REG_EDX | REG_SPECIAL));
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(STFSU)
{
	CreateRecInstruction(REC_GETREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_GETREG(REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_ADDI(rA, SIMM));
	CreateRecInstruction(REC_MOVE(REG_ECX | REG_SPECIAL, rA));

	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	CreateRecInstruction(REC_CVTPD2PS(TEMP_FPU_REG, frS));
	CreateRecInstruction(REC_MOVD_FROMFPU(REG_EDX | REG_SPECIAL, TEMP_FPU_REG));

	CreateRecInstruction(REC_WRITEMEM32(REG_ECX | REG_SPECIAL, REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_FREEREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_FREEREG(REG_EDX | REG_SPECIAL));
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(STFSUX)
{
	CreateRecInstruction(REC_GETREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_GETREG(REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_ADD(rA, rB));
	CreateRecInstruction(REC_MOVE(REG_ECX | REG_SPECIAL, rA));

	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	CreateRecInstruction(REC_CVTPD2PS(TEMP_FPU_REG, frS));
	CreateRecInstruction(REC_MOVD_FROMFPU(REG_EDX | REG_SPECIAL, TEMP_FPU_REG));

	CreateRecInstruction(REC_WRITEMEM32(REG_ECX | REG_SPECIAL, REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_FREEREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_FREEREG(REG_EDX | REG_SPECIAL));
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}
GekkoRecOp(STFSX)
{
	CreateRecInstruction(REC_GETREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_GETREG(REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_MOVE(REG_ECX | REG_SPECIAL, rB));

	if(rA)
		CreateRecInstruction(REC_ADD(REG_ECX | REG_SPECIAL, rA));

	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	CreateRecInstruction(REC_CVTPD2PS(TEMP_FPU_REG, frS));
	CreateRecInstruction(REC_MOVD_FROMFPU(REG_EDX | REG_SPECIAL, TEMP_FPU_REG));

	CreateRecInstruction(REC_WRITEMEM32(REG_ECX | REG_SPECIAL, REG_EDX | REG_SPECIAL));

	CreateRecInstruction(REC_FREEREG(REG_ECX | REG_SPECIAL));
	CreateRecInstruction(REC_FREEREG(REG_EDX | REG_SPECIAL));
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}

GekkoRecOp(PSQ_L)
{
    int type = GQR_LD_TYPE(PSI);
    int scale = (u8)GQR_LD_SCALE(PSI);

	CreateRecInstruction(REC_GETTEMPREG(0));
	CreateRecInstruction(REC_GETTEMPFPUREG(0));

	//read in PS1 first if the PSW flag is not set
	if(PSW)
	{
		if(type & 4)
			CreateRecInstruction(REC_MOVEI(TEMP_REG, 0x00000001));
		else
			CreateRecInstruction(REC_MOVEI(TEMP_REG, 0x3F800000));
	}
	else
    {
		CreateRecInstruction(REC_MOVEI(TEMP_REG, PSIMM));

		//adjust the PSIMM value based on the amount of data read
		switch(type)
		{
			case 4:
			case 6:
				CreateRecInstruction(REC_ADDI(TEMP_REG, 1));
				break;

			case 5:
			case 7:
				CreateRecInstruction(REC_ADDI(TEMP_REG, 2));
				break;

			default:
				CreateRecInstruction(REC_ADDI(TEMP_REG, 4));
		}

		if(rA)
			CreateRecInstruction(REC_ADD(TEMP_REG, rA));

		switch(type)
		{
			case 4:
				CreateRecInstruction(REC_READMEM8(TEMP_REG, TEMP_REG));
				break;

			case 6:
				CreateRecInstruction(REC_READMEM8SX(TEMP_REG, TEMP_REG));
				break;

			case 5:
				CreateRecInstruction(REC_READMEM16(TEMP_REG, TEMP_REG));
				break;

			case 7:
				CreateRecInstruction(REC_READMEM16SX(TEMP_REG, TEMP_REG));
				break;

			default:
				CreateRecInstruction(REC_READMEM32(TEMP_REG, TEMP_REG));
		}
	}

	//move to the fpu and shift the data over
	CreateRecInstruction(REC_MOVD_TOFPU(TEMP_FPU_REG, TEMP_REG));
	CreateRecInstruction(REC_MOVSLDUP(frD, TEMP_FPU_REG));

	//now read in PS0
	CreateRecInstruction(REC_MOVEI(TEMP_REG, PSIMM));

	if(rA)
		CreateRecInstruction(REC_ADD(TEMP_REG, rA));

	switch(type)
	{
		case 4:
			CreateRecInstruction(REC_READMEM8(TEMP_REG, TEMP_REG));
			break;

		case 6:
			CreateRecInstruction(REC_READMEM8SX(TEMP_REG, TEMP_REG));
			break;

		case 5:
			CreateRecInstruction(REC_READMEM16(TEMP_REG, TEMP_REG));
			break;

		case 7:
			CreateRecInstruction(REC_READMEM16SX(TEMP_REG, TEMP_REG));
			break;

		default:
			CreateRecInstruction(REC_READMEM32(TEMP_REG, TEMP_REG));
	}

	//move to the fpu
	CreateRecInstruction(REC_MOVD_TOFPU(TEMP_FPU_REG, TEMP_REG));
	CreateRecInstruction(REC_MOVSS(frD, TEMP_FPU_REG));

	//frD low 2 dwords contain PS0 and PS1, convert as required
	if(type & 4)
	{
		CreateRecInstruction(REC_CVTDQ2PD(frD, frD));

		//scale it
		if(PSW)
			CreateRecInstruction(REC_MOVAPDM(TEMP_FPU_REG, (u32)&ldScale64bitX1[scale]));
		else
			CreateRecInstruction(REC_MOVAPDM(TEMP_FPU_REG, (u32)&ldScale64bit[scale]));
		CreateRecInstruction(REC_MULPD(frD, TEMP_FPU_REG));
	}
	else
	{
		CreateRecInstruction(REC_CVTPS2PD(frD, frD));
	}

 //   PS0D = (f64)dequantize(data0, type, scale);
 //   PS1D = (f64)dequantize(data1, type, scale);

	CreateRecInstruction(REC_FREETEMPREG());
	CreateRecInstruction(REC_FREETEMPFPUREG(0));
}