////////////////////////////////////////////////////////////
// TITLE:		ShizZy ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Interpreter 0.1
// FILE:		cpu_int.h
// DESC:		Core Header
// CREATED:		Feb. 28, 2006
////////////////////////////////////////////////////////////
// Copyright (c) 2005 ShizZy/_aprentice_
////////////////////////////////////////////////////////////

#ifndef _GEKKO_INT_H
#define _GEKKO_INT_H

#include "../../emu.h"
#include "../cpu_core.h"
#include "../cpu_core_regs.h"

#define OPTYPE	void __fastcall
#define GekkoIntOp(name)	OPTYPE GekkoCPUInterpreter::GekkoInterpreter_##name(void)
#define GekkoInt(name)		GekkoCPUInterpreter::GekkoInterpreter_##name

#undef GekkoCPUOpsGroup
#undef GekkoCPUOp
#define GekkoCPUOpsGroup(name)	GekkoCPU::OpData GekkoCPUInterpreter::GekkoIntOpsGroup##name
#define GekkoCPUOp				GekkoInt

#define getCycles				1
/*
#define GekkoRead8 Memory_Read8
#define GekkoRead16 Memory_Read16
#define GekkoRead32 Memory_Read32
#define GekkoRead64 Memory_Read64

#define GekkoWrite8 Memory_Write8
#define GekkoWrite16 Memory_Write16
#define GekkoWrite32 Memory_Write32
#define GekkoWrite64 Memory_Write64
*/

////////////////////////////////////////////////////////////

// Gekko Structure
// Desc: Controls CPU Modes
/*
#ifndef MEM_NATIVE_LE32
#define PTR_PC		BSWAP32(*pPC)
#else 
#define PTR_PC		*pPC
#endif
*/
#define PTR_PC			*(u32*)(&Mem_RAM[ireg.PC & RAM_MASK]) //Memory_Read32(ireg.PC)

////////////////////////////////////////////////////////////

class GekkoCPUInterpreter : virtual public GekkoCPU
{
private:
	static u32			RotMask[32][32];
	static u32			LastFinishedOp;

	static OpData GekkoIntOpsGroup4XO0[];
	static OpData GekkoIntOpsGroup4[];
	static OpData GekkoIntOpsGroup19[];
	static OpData GekkoIntOpsGroup31[];
	static OpData GekkoIntOpsGroup59[];
	static OpData GekkoIntOpsGroup63XO0[];
	static OpData GekkoIntOpsGroup63[];
	static OpData GekkoIntOpsGroupMain[];

#define CPU_OPSTORE_COUNT	50000
	static u32			LastOps[CPU_OPSTORE_COUNT];
	static u32			LastOpsPC[CPU_OPSTORE_COUNT];
	static u32			LastNewR[CPU_OPSTORE_COUNT * 32];
	static u32			LastNewLR[CPU_OPSTORE_COUNT];
	static u32			LastNewSRR0[CPU_OPSTORE_COUNT];
	static u32			LastNewStack[CPU_OPSTORE_COUNT * 64];
	static u32			LastOpEntry;

	static u32			branch;
	static u32			exception;

#define OPCODE_BRANCH	1
#define OPCODE_RFI		2

	static GekkoIntOp(Ops_Group4);
	static GekkoIntOp(Ops_Group19);
	static GekkoIntOp(Ops_Group31);
	static GekkoIntOp(Ops_Group59);
	static GekkoIntOp(Ops_Group63);

	static GekkoIntOp(Ops_Group4XO0);
	static GekkoIntOp(Ops_Group63XO0);

protected:
	static GekkoF	Tick();

public:
	GekkoF	ExecuteInstruction();

	GekkoCPUInterpreter();
	~GekkoCPUInterpreter();

	////////////////////////////////////////////////////////////

	// Control Functions
	//

	CPUType	GetCPUType();
	GekkoF	Open(u32 entry_point);
	GekkoF	Halt();
	GekkoF	Start();

	GekkoF	Exception(tGekkoException which);

	u32	GetTicksPerSecond();

	static GekkoIntOp(DUMP_OPS);
	static GekkoIntOp(PS_ABS);
	static GekkoIntOp(PS_ADD);
	static GekkoIntOp(PS_CMPO0);
	static GekkoIntOp(PS_CMPO1);
	static GekkoIntOp(PS_CMPU0);
	static GekkoIntOp(PS_CMPU1);
	static GekkoIntOp(PS_MADD);
	static GekkoIntOp(PS_MADDS0);
	static GekkoIntOp(PS_MADDS1);
	static GekkoIntOp(PS_MERGE00);
	static GekkoIntOp(PS_MERGE01);
	static GekkoIntOp(PS_MERGE10);
	static GekkoIntOp(PS_MERGE11);
	static GekkoIntOp(PS_MR);
	static GekkoIntOp(PS_MSUB);
	static GekkoIntOp(PS_MUL);
	static GekkoIntOp(PS_MULS0);
	static GekkoIntOp(PS_MULS1);
	static GekkoIntOp(PS_NEG);
	static GekkoIntOp(PS_NMADD);
	static GekkoIntOp(PS_NMSUB);
	static GekkoIntOp(PS_RES);
	static GekkoIntOp(PS_RSQRTE);
	static GekkoIntOp(PS_SUB);
	static GekkoIntOp(PS_SUM0);
	static GekkoIntOp(PS_SUM1);
	static GekkoIntOp(PS_SEL);

	static GekkoIntOp(NI);
	static GekkoIntOp(ADD);
	static GekkoIntOp(ADDO);
	static GekkoIntOp(ADDC);
	static GekkoIntOp(ADDCO);
	static GekkoIntOp(ADDE);
	static GekkoIntOp(ADDEO);
	static GekkoIntOp(ADDI);
	static GekkoIntOp(ADDIC);
	static GekkoIntOp(ADDICD);
	static GekkoIntOp(ADDIS);
	static GekkoIntOp(ADDME);
	static GekkoIntOp(ADDMEO);
	static GekkoIntOp(ADDZE);
	static GekkoIntOp(ADDZEO);
	static GekkoIntOp(AND);
	static GekkoIntOp(ANDC);
	static GekkoIntOp(ANDID);
	static GekkoIntOp(ANDISD);
	static GekkoIntOp(B);
	static GekkoIntOp(BL);
	static GekkoIntOp(BA);
	static GekkoIntOp(BLA);
	static GekkoIntOp(BX);
	static GekkoIntOp(BCX);
	static GekkoIntOp(BCCTRX);
	static GekkoIntOp(BCLRX);
	static GekkoIntOp(CMP);
	static GekkoIntOp(CMPI);
	static GekkoIntOp(CMPL);
	static GekkoIntOp(CMPLI);
	static GekkoIntOp(CNTLZW);
	static GekkoIntOp(CRAND);
	static GekkoIntOp(CRANDC);
	static GekkoIntOp(CREQV);
	static GekkoIntOp(CRNAND);
	static GekkoIntOp(CRNOR);
	static GekkoIntOp(CROR);
	static GekkoIntOp(CRORC);
	static GekkoIntOp(CRXOR);
	static GekkoIntOp(DCBA);
	static GekkoIntOp(DCBF);
	static GekkoIntOp(DCBI);
	static GekkoIntOp(DCBST);
	static GekkoIntOp(DCBT);
	static GekkoIntOp(DCBTST);
	static GekkoIntOp(DCBZ);
	static GekkoIntOp(DCBZ_L);
	static GekkoIntOp(DIVW);
	static GekkoIntOp(DIVWO);
	static GekkoIntOp(DIVWOD);
	static GekkoIntOp(DIVWU);
	static GekkoIntOp(DIVWUO);
	static GekkoIntOp(ECIWX);
	static GekkoIntOp(ECOWX);
	static GekkoIntOp(EIEIO);
	static GekkoIntOp(EQV);
	static GekkoIntOp(EXTSB);
	static GekkoIntOp(EXTSH);
	static GekkoIntOp(FABS);
	static GekkoIntOp(FADD);
	static GekkoIntOp(FADDS);
	static GekkoIntOp(FCMPO);
	static GekkoIntOp(FCMPU);
	static GekkoIntOp(FCTIW);
	static GekkoIntOp(FCTIWZ);
	static GekkoIntOp(FDIV);
	static GekkoIntOp(FDIVS);
	static GekkoIntOp(FMADD);
	static GekkoIntOp(FMADDS);
	static GekkoIntOp(FMR);
	static GekkoIntOp(FMSUB);
	static GekkoIntOp(FMSUBS);
	static GekkoIntOp(FMUL);
	static GekkoIntOp(FMULS);
	static GekkoIntOp(FNABS);
	static GekkoIntOp(FNEG);
	static GekkoIntOp(FNMADD);
	static GekkoIntOp(FNMADDS);
	static GekkoIntOp(FNMSUB);
	static GekkoIntOp(FNMSUBS);
	static GekkoIntOp(FRES);
	static GekkoIntOp(FRSP);
	static GekkoIntOp(FRSQRTE);
	static GekkoIntOp(FSEL);
	static GekkoIntOp(FSQRT);
	static GekkoIntOp(FSQRTS);
	static GekkoIntOp(FSUB);
	static GekkoIntOp(FSUBS);
	static GekkoIntOp(ICBI);
	static GekkoIntOp(ISYNC);
	static GekkoIntOp(LBZ);
	static GekkoIntOp(LBZU);
	static GekkoIntOp(LBZUX);
	static GekkoIntOp(LBZX);
	static GekkoIntOp(LFD);
	static GekkoIntOp(LFDU);
	static GekkoIntOp(LFDUX);
	static GekkoIntOp(LFDX);
	static GekkoIntOp(LFS);
	static GekkoIntOp(LFSU);
	static GekkoIntOp(LFSUX);
	static GekkoIntOp(LFSX);
	static GekkoIntOp(LHA);
	static GekkoIntOp(LHAU);
	static GekkoIntOp(LHAUX);
	static GekkoIntOp(LHAX);
	static GekkoIntOp(LHBRX);
	static GekkoIntOp(LHZ);
	static GekkoIntOp(LHZU);
	static GekkoIntOp(LHZUX);
	static GekkoIntOp(LHZX);
	static GekkoIntOp(LMW);
	static GekkoIntOp(LSWI);
	static GekkoIntOp(LSWX);
	static GekkoIntOp(LWARX);
	static GekkoIntOp(LWBRX);
	static GekkoIntOp(LWZ);
	static GekkoIntOp(LWZU);
	static GekkoIntOp(LWZUX);
	static GekkoIntOp(LWZX);
	static GekkoIntOp(MCRF);
	static GekkoIntOp(MCRFS);
	static GekkoIntOp(MCRXR);
	static GekkoIntOp(MFCR);
	static GekkoIntOp(MFFS);
	static GekkoIntOp(MFMSR);
	static GekkoIntOp(MFSPR);
	static GekkoIntOp(MFSR);
	static GekkoIntOp(MFSRIN);
	static GekkoIntOp(MFTB);
	static GekkoIntOp(MTCRF);
	static GekkoIntOp(MTFSB0);
	static GekkoIntOp(MTFSB1);
	static GekkoIntOp(MTFSF);
	static GekkoIntOp(MTSFI);
	static GekkoIntOp(MTMSR);
	static GekkoIntOp(MTSPR);
	static GekkoIntOp(MTSR);
	static GekkoIntOp(MTSRIN);
	static GekkoIntOp(MULHW);
	static GekkoIntOp(MULHWD);
	static GekkoIntOp(MULHWU);
	static GekkoIntOp(MULHWUD);
	static GekkoIntOp(MULLI);
	static GekkoIntOp(MULLW);
	static GekkoIntOp(MULLWD);
	static GekkoIntOp(MULLWO);
	static GekkoIntOp(MULLWOD);
	static GekkoIntOp(NAND);
	static GekkoIntOp(NEG);
	static GekkoIntOp(NEGO);
	static GekkoIntOp(NOR);
	static GekkoIntOp(OR);
	static GekkoIntOp(ORD);
	static GekkoIntOp(ORC);
	static GekkoIntOp(ORI);
	static GekkoIntOp(ORIS);
	static GekkoIntOp(PS_DIV);
	static GekkoIntOp(PSQ_L);
	static GekkoIntOp(PSQ_LX);
	static GekkoIntOp(PSQ_LU);
	static GekkoIntOp(PSQ_LUX);
	static GekkoIntOp(PSQ_ST);
	static GekkoIntOp(PSQ_STX);
	static GekkoIntOp(PSQ_STU);
	static GekkoIntOp(PSQ_STUX);
	static GekkoIntOp(RFI);
	static GekkoIntOp(RLWIMI);
	static GekkoIntOp(RLWINM);
	static GekkoIntOp(RLWNM);
	static GekkoIntOp(SC);
	static GekkoIntOp(SLW);
	static GekkoIntOp(SRAW);
	static GekkoIntOp(SRAWI);
	static GekkoIntOp(SRW);
	static GekkoIntOp(STB);
	static GekkoIntOp(STBU);
	static GekkoIntOp(STBUX);
	static GekkoIntOp(STBX);
	static GekkoIntOp(STFD);
	static GekkoIntOp(STFDU);
	static GekkoIntOp(STFDUX);
	static GekkoIntOp(STFDX);
	static GekkoIntOp(STFIWX);
	static GekkoIntOp(STFS);
	static GekkoIntOp(STFSU);
	static GekkoIntOp(STFSUX);
	static GekkoIntOp(STFSX);
	static GekkoIntOp(STH);
	static GekkoIntOp(STHBRX);
	static GekkoIntOp(STHU);
	static GekkoIntOp(STHUX);
	static GekkoIntOp(STHX);
	static GekkoIntOp(STMW);
	static GekkoIntOp(STSWI);
	static GekkoIntOp(STSWX);
	static GekkoIntOp(STW);
	static GekkoIntOp(STWBRX);
	static GekkoIntOp(STWCXD);
	static GekkoIntOp(STWU);
	static GekkoIntOp(STWUX);
	static GekkoIntOp(STWX);
	static GekkoIntOp(SUBF);
	static GekkoIntOp(SUBFO);
	static GekkoIntOp(SUBFC);
	static GekkoIntOp(SUBFCO);
	static GekkoIntOp(SUBFE);
	static GekkoIntOp(SUBFEO);
	static GekkoIntOp(SUBFIC);
	static GekkoIntOp(SUBFME);
	static GekkoIntOp(SUBFMEO);
	static GekkoIntOp(SUBFZE);
	static GekkoIntOp(SUBFZEO);
	static GekkoIntOp(SYNC);
	static GekkoIntOp(TLBIA);
	static GekkoIntOp(TLBIE);
	static GekkoIntOp(TLBSYNC);
	static GekkoIntOp(TW);
	static GekkoIntOp(TWI);
	static GekkoIntOp(XOR);
	static GekkoIntOp(XORI);
	static GekkoIntOp(XORIS);

	static GekkoIntOp(STWCX);
	static GekkoIntOp(MTFSFI);
	static GekkoIntOp(HLE);
};

#endif

