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

#include "powerpc/cpu_core.h"
#include "powerpc/cpu_core_regs.h"

#define OPTYPE                  void
#define GekkoIntOpDecl(name)	OPTYPE GekkoInterpreter_##name(void)
#define GekkoIntOp(name)	    OPTYPE GekkoCPUInterpreter::GekkoInterpreter_##name(void)
#define GekkoInt(name)		    GekkoCPUInterpreter::GekkoInterpreter_##name

#undef GekkoCPUOpsGroup
#undef GekkoCPUOp
#define GekkoCPUOpsGroup(name)	GekkoCPU::OpData GekkoCPUInterpreter::GekkoIntOpsGroup##name
#define GekkoCPUOp				GekkoInt

#define getCycles				1

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

	static GekkoIntOpDecl(Ops_Group4);
	static GekkoIntOpDecl(Ops_Group19);
	static GekkoIntOpDecl(Ops_Group31);
	static GekkoIntOpDecl(Ops_Group59);
	static GekkoIntOpDecl(Ops_Group63);

	static GekkoIntOpDecl(Ops_Group4XO0);
	static GekkoIntOpDecl(Ops_Group63XO0);

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

	static GekkoIntOpDecl(DUMP_OPS);
	static GekkoIntOpDecl(PS_ABS);
	static GekkoIntOpDecl(PS_ADD);
	static GekkoIntOpDecl(PS_CMPO0);
	static GekkoIntOpDecl(PS_CMPO1);
	static GekkoIntOpDecl(PS_CMPU0);
	static GekkoIntOpDecl(PS_CMPU1);
	static GekkoIntOpDecl(PS_MADD);
	static GekkoIntOpDecl(PS_MADDS0);
	static GekkoIntOpDecl(PS_MADDS1);
	static GekkoIntOpDecl(PS_MERGE00);
	static GekkoIntOpDecl(PS_MERGE01);
	static GekkoIntOpDecl(PS_MERGE10);
	static GekkoIntOpDecl(PS_MERGE11);
	static GekkoIntOpDecl(PS_MR);
	static GekkoIntOpDecl(PS_MSUB);
	static GekkoIntOpDecl(PS_MUL);
	static GekkoIntOpDecl(PS_MULS0);
	static GekkoIntOpDecl(PS_MULS1);
	static GekkoIntOpDecl(PS_NEG);
	static GekkoIntOpDecl(PS_NMADD);
	static GekkoIntOpDecl(PS_NMSUB);
	static GekkoIntOpDecl(PS_RES);
	static GekkoIntOpDecl(PS_RSQRTE);
	static GekkoIntOpDecl(PS_SUB);
	static GekkoIntOpDecl(PS_SUM0);
	static GekkoIntOpDecl(PS_SUM1);
	static GekkoIntOpDecl(PS_SEL);

	static GekkoIntOpDecl(NI);
	static GekkoIntOpDecl(ADD);
	static GekkoIntOpDecl(ADDO);
	static GekkoIntOpDecl(ADDC);
	static GekkoIntOpDecl(ADDCO);
	static GekkoIntOpDecl(ADDE);
	static GekkoIntOpDecl(ADDEO);
	static GekkoIntOpDecl(ADDI);
	static GekkoIntOpDecl(ADDIC);
	static GekkoIntOpDecl(ADDICD);
	static GekkoIntOpDecl(ADDIS);
	static GekkoIntOpDecl(ADDME);
	static GekkoIntOpDecl(ADDMEO);
	static GekkoIntOpDecl(ADDZE);
	static GekkoIntOpDecl(ADDZEO);
	static GekkoIntOpDecl(AND);
	static GekkoIntOpDecl(ANDC);
	static GekkoIntOpDecl(ANDID);
	static GekkoIntOpDecl(ANDISD);
	static GekkoIntOpDecl(B);
	static GekkoIntOpDecl(BL);
	static GekkoIntOpDecl(BA);
	static GekkoIntOpDecl(BLA);
	static GekkoIntOpDecl(BX);
	static GekkoIntOpDecl(BCX);
	static GekkoIntOpDecl(BCCTRX);
	static GekkoIntOpDecl(BCLRX);
	static GekkoIntOpDecl(CMP);
	static GekkoIntOpDecl(CMPI);
	static GekkoIntOpDecl(CMPL);
	static GekkoIntOpDecl(CMPLI);
	static GekkoIntOpDecl(CNTLZW);
	static GekkoIntOpDecl(CRAND);
	static GekkoIntOpDecl(CRANDC);
	static GekkoIntOpDecl(CREQV);
	static GekkoIntOpDecl(CRNAND);
	static GekkoIntOpDecl(CRNOR);
	static GekkoIntOpDecl(CROR);
	static GekkoIntOpDecl(CRORC);
	static GekkoIntOpDecl(CRXOR);
	static GekkoIntOpDecl(DCBA);
	static GekkoIntOpDecl(DCBF);
	static GekkoIntOpDecl(DCBI);
	static GekkoIntOpDecl(DCBST);
	static GekkoIntOpDecl(DCBT);
	static GekkoIntOpDecl(DCBTST);
	static GekkoIntOpDecl(DCBZ);
	static GekkoIntOpDecl(DCBZ_L);
	static GekkoIntOpDecl(DIVW);
	static GekkoIntOpDecl(DIVWO);
	static GekkoIntOpDecl(DIVWOD);
	static GekkoIntOpDecl(DIVWU);
	static GekkoIntOpDecl(DIVWUO);
	static GekkoIntOpDecl(ECIWX);
	static GekkoIntOpDecl(ECOWX);
	static GekkoIntOpDecl(EIEIO);
	static GekkoIntOpDecl(EQV);
	static GekkoIntOpDecl(EXTSB);
	static GekkoIntOpDecl(EXTSH);
	static GekkoIntOpDecl(FABS);
	static GekkoIntOpDecl(FADD);
	static GekkoIntOpDecl(FADDS);
	static GekkoIntOpDecl(FCMPO);
	static GekkoIntOpDecl(FCMPU);
	static GekkoIntOpDecl(FCTIW);
	static GekkoIntOpDecl(FCTIWZ);
	static GekkoIntOpDecl(FDIV);
	static GekkoIntOpDecl(FDIVS);
	static GekkoIntOpDecl(FMADD);
	static GekkoIntOpDecl(FMADDS);
	static GekkoIntOpDecl(FMR);
	static GekkoIntOpDecl(FMSUB);
	static GekkoIntOpDecl(FMSUBS);
	static GekkoIntOpDecl(FMUL);
	static GekkoIntOpDecl(FMULS);
	static GekkoIntOpDecl(FNABS);
	static GekkoIntOpDecl(FNEG);
	static GekkoIntOpDecl(FNMADD);
	static GekkoIntOpDecl(FNMADDS);
	static GekkoIntOpDecl(FNMSUB);
	static GekkoIntOpDecl(FNMSUBS);
	static GekkoIntOpDecl(FRES);
	static GekkoIntOpDecl(FRSP);
	static GekkoIntOpDecl(FRSQRTE);
	static GekkoIntOpDecl(FSEL);
	static GekkoIntOpDecl(FSQRT);
	static GekkoIntOpDecl(FSQRTS);
	static GekkoIntOpDecl(FSUB);
	static GekkoIntOpDecl(FSUBS);
	static GekkoIntOpDecl(ICBI);
	static GekkoIntOpDecl(ISYNC);
	static GekkoIntOpDecl(LBZ);
	static GekkoIntOpDecl(LBZU);
	static GekkoIntOpDecl(LBZUX);
	static GekkoIntOpDecl(LBZX);
	static GekkoIntOpDecl(LFD);
	static GekkoIntOpDecl(LFDU);
	static GekkoIntOpDecl(LFDUX);
	static GekkoIntOpDecl(LFDX);
	static GekkoIntOpDecl(LFS);
	static GekkoIntOpDecl(LFSU);
	static GekkoIntOpDecl(LFSUX);
	static GekkoIntOpDecl(LFSX);
	static GekkoIntOpDecl(LHA);
	static GekkoIntOpDecl(LHAU);
	static GekkoIntOpDecl(LHAUX);
	static GekkoIntOpDecl(LHAX);
	static GekkoIntOpDecl(LHBRX);
	static GekkoIntOpDecl(LHZ);
	static GekkoIntOpDecl(LHZU);
	static GekkoIntOpDecl(LHZUX);
	static GekkoIntOpDecl(LHZX);
	static GekkoIntOpDecl(LMW);
	static GekkoIntOpDecl(LSWI);
	static GekkoIntOpDecl(LSWX);
	static GekkoIntOpDecl(LWARX);
	static GekkoIntOpDecl(LWBRX);
	static GekkoIntOpDecl(LWZ);
	static GekkoIntOpDecl(LWZU);
	static GekkoIntOpDecl(LWZUX);
	static GekkoIntOpDecl(LWZX);
	static GekkoIntOpDecl(MCRF);
	static GekkoIntOpDecl(MCRFS);
	static GekkoIntOpDecl(MCRXR);
	static GekkoIntOpDecl(MFCR);
	static GekkoIntOpDecl(MFFS);
	static GekkoIntOpDecl(MFMSR);
	static GekkoIntOpDecl(MFSPR);
	static GekkoIntOpDecl(MFSR);
	static GekkoIntOpDecl(MFSRIN);
	static GekkoIntOpDecl(MFTB);
	static GekkoIntOpDecl(MTCRF);
	static GekkoIntOpDecl(MTFSB0);
	static GekkoIntOpDecl(MTFSB1);
	static GekkoIntOpDecl(MTFSF);
	static GekkoIntOpDecl(MTSFI);
	static GekkoIntOpDecl(MTMSR);
	static GekkoIntOpDecl(MTSPR);
	static GekkoIntOpDecl(MTSR);
	static GekkoIntOpDecl(MTSRIN);
	static GekkoIntOpDecl(MULHW);
	static GekkoIntOpDecl(MULHWD);
	static GekkoIntOpDecl(MULHWU);
	static GekkoIntOpDecl(MULHWUD);
	static GekkoIntOpDecl(MULLI);
	static GekkoIntOpDecl(MULLW);
	static GekkoIntOpDecl(MULLWD);
	static GekkoIntOpDecl(MULLWO);
	static GekkoIntOpDecl(MULLWOD);
	static GekkoIntOpDecl(NAND);
	static GekkoIntOpDecl(NEG);
	static GekkoIntOpDecl(NEGO);
	static GekkoIntOpDecl(NOR);
	static GekkoIntOpDecl(OR);
	static GekkoIntOpDecl(ORD);
	static GekkoIntOpDecl(ORC);
	static GekkoIntOpDecl(ORI);
	static GekkoIntOpDecl(ORIS);
	static GekkoIntOpDecl(PS_DIV);
	static GekkoIntOpDecl(PSQ_L);
	static GekkoIntOpDecl(PSQ_LX);
	static GekkoIntOpDecl(PSQ_LU);
	static GekkoIntOpDecl(PSQ_LUX);
	static GekkoIntOpDecl(PSQ_ST);
	static GekkoIntOpDecl(PSQ_STX);
	static GekkoIntOpDecl(PSQ_STU);
	static GekkoIntOpDecl(PSQ_STUX);
	static GekkoIntOpDecl(RFI);
	static GekkoIntOpDecl(RLWIMI);
	static GekkoIntOpDecl(RLWINM);
	static GekkoIntOpDecl(RLWNM);
	static GekkoIntOpDecl(SC);
	static GekkoIntOpDecl(SLW);
	static GekkoIntOpDecl(SRAW);
	static GekkoIntOpDecl(SRAWI);
	static GekkoIntOpDecl(SRW);
	static GekkoIntOpDecl(STB);
	static GekkoIntOpDecl(STBU);
	static GekkoIntOpDecl(STBUX);
	static GekkoIntOpDecl(STBX);
	static GekkoIntOpDecl(STFD);
	static GekkoIntOpDecl(STFDU);
	static GekkoIntOpDecl(STFDUX);
	static GekkoIntOpDecl(STFDX);
	static GekkoIntOpDecl(STFIWX);
	static GekkoIntOpDecl(STFS);
	static GekkoIntOpDecl(STFSU);
	static GekkoIntOpDecl(STFSUX);
	static GekkoIntOpDecl(STFSX);
	static GekkoIntOpDecl(STH);
	static GekkoIntOpDecl(STHBRX);
	static GekkoIntOpDecl(STHU);
	static GekkoIntOpDecl(STHUX);
	static GekkoIntOpDecl(STHX);
	static GekkoIntOpDecl(STMW);
	static GekkoIntOpDecl(STSWI);
	static GekkoIntOpDecl(STSWX);
	static GekkoIntOpDecl(STW);
	static GekkoIntOpDecl(STWBRX);
	static GekkoIntOpDecl(STWCXD);
	static GekkoIntOpDecl(STWU);
	static GekkoIntOpDecl(STWUX);
	static GekkoIntOpDecl(STWX);
	static GekkoIntOpDecl(SUBF);
	static GekkoIntOpDecl(SUBFO);
	static GekkoIntOpDecl(SUBFC);
	static GekkoIntOpDecl(SUBFCO);
	static GekkoIntOpDecl(SUBFE);
	static GekkoIntOpDecl(SUBFEO);
	static GekkoIntOpDecl(SUBFIC);
	static GekkoIntOpDecl(SUBFME);
	static GekkoIntOpDecl(SUBFMEO);
	static GekkoIntOpDecl(SUBFZE);
	static GekkoIntOpDecl(SUBFZEO);
	static GekkoIntOpDecl(SYNC);
	static GekkoIntOpDecl(TLBIA);
	static GekkoIntOpDecl(TLBIE);
	static GekkoIntOpDecl(TLBSYNC);
	static GekkoIntOpDecl(TW);
	static GekkoIntOpDecl(TWI);
	static GekkoIntOpDecl(XOR);
	static GekkoIntOpDecl(XORI);
	static GekkoIntOpDecl(XORIS);

	static GekkoIntOpDecl(STWCX);
	static GekkoIntOpDecl(MTFSFI);
	static GekkoIntOpDecl(HLE);
};

#undef GekkoIntOpDecl

#endif

