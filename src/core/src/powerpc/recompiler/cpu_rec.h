////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec.h
// DESC:		Core Header
// CREATED:		Feb. 26, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007	Lightning
////////////////////////////////////////////////////////////

#ifndef _GEKKO_REC_H
#define _GEKKO_REC_H

#ifndef EMU_IGNORE_RECOMPILER

//#include "../../emu.h"
#include "common.h"
#include "../cpu_core.h"
//#include "../cpu_core_regs.h"

#undef GekkoCPUOpsGroup
#undef GekkoCPUOp
//#define GekkoCPUOpsGroup2(name)	GekkoCPU::OpData GekkoCPURecompiler::GekkoRecGroupOps##name
//#define GekkoCPUOp2				GekkoRecInt
#define GekkoCPUOpsGroup(name)	GekkoCPU::OpData GekkoCPURecompiler::GekkoRecGroupOps##name
#define GekkoCPUOp				GekkoRecInt

#define OPTYPE	void __fastcall
#define GekkoRecOp(name)	OPTYPE GekkoCPURecompiler::GekkoRecompiler_##name(void)
#define GekkoRec(name)		GekkoCPURecompiler::GekkoRecompiler_##name
#define GekkoRecIntOp(name)	OPTYPE GekkoCPURecompiler::GekkoRecompilerInt_##name(void)
#define GekkoRecInt(name)	GekkoCPURecompiler::GekkoRecompilerInt_##name

typedef void (__fastcall *RecCompInstructionPtr)(void);

//#ifndef MEM_NATIVE_LE32
//#define PTR_PC		BSWAP32(*pPC)
#define PTR_PC			*(u32*)(&Mem_RAM[ireg.PC & RAM_MASK]) //Memory_Read32(ireg.PC)
//#else 
//#define PTR_PC		*pPC
//#endif

#define X86_REG_COUNT	8
#define FPU_REG_COUNT	8
#define PPC_REG_COUNT	32

#pragma vtordisp(off)

class GekkoCPURecompiler : virtual public GekkoCPU
{
public:
	GekkoF	ExecuteInstruction();

private:
	#include "cpu_rec_memory.h"

	static u32 *	GekkoCPURecompiler::CompiledTable;
	static u32 *	GekkoCPURecompiler::CompiledTablePages;

	static u32	PageSize;

	static OpData GekkoRecGroupOps4XO0[];
	static OpData GekkoRecGroupOps4[];
	static OpData GekkoRecGroupOps19[];
	static OpData GekkoRecGroupOps31[];
	static OpData GekkoRecGroupOps59[];
	static OpData GekkoRecGroupOps63XO0[];
	static OpData GekkoRecGroupOps63[];
	static OpData GekkoRecGroupOpsMain[];

	static u32			branch;
	static u32			IsInterpret;
	static u32			RotMask[32][32];
	static u32			TotalInstCount;

	static u32			IntCalls[0x10000];

	//static u32		exception;
	static u32		is_dec;
	static u32		HLE_PC;

	static t128 FPR_SignBit;
	static t128 FPR_SignBitLow;
	static t128 FPR_SignBitInverse;
	static t128 FPR_SignBitInverseLow;
	static t128 FPR_Negative1;
	static t128 FPR_Negative1Low;

public:
	GekkoF DumpInternalData(u32 DumpPC, u32 DumpLen);

	static const t128 ldScale64bit[64];
	static const t128 ldScale64bitX1[64];
	static const t128 FPR1Float;

	static LONG __stdcall GekkoCPURecompiler::UnhandledException(EXCEPTION_POINTERS *ExceptionInfo);

	extern struct RecInstruction;
	extern struct PPCRegInfo;

	typedef void (__fastcall *RecCacheInstructionPtr)(RecInstruction *Instruction);
	typedef void (__fastcall *RecILInstructionPtr)(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs);

#define GekkoRecILOp(name)	OPTYPE GekkoCPURecompiler::GekkoILInstruction_##name(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs)
#define GekkoRecIL(name)	GekkoCPURecompiler::GekkoILInstruction_##name

	typedef struct
	{
		optable * CPUInstrTable;
		OpData	  RecOpcode;
	} RecOpData;

	typedef struct
	{
		u32		KnownValue;
		u32		Flags;
	} PPCKnownRegInfo;

	typedef struct PPCRegInfo
	{
		u32		PPCRegister;
		u32		ValueChanged;
	} PPCRegInfo;

	typedef struct JumpList
	{
		u32				ID;
		u32				Address;
		struct JumpList *Next;
	} JumpList;

	static JumpList	*JumpLabelEntries;
	static JumpList *JumpEntries;

	static PPCKnownRegInfo	PPCRegVals[128];
	static u8				XERKnown[4];
	static u8				CRKnown[4];
	static u8				SetXERKnown[4];
	static u8				SetCRKnown[4];

	#define GekkoRecCacheOp(name)	OPTYPE GekkoCPURecompiler::GekkoRecCache_##name(RecInstruction *Instruction)
	#define GekkoRecCache(name)		GekkoCPURecompiler::GekkoRecCache_##name

	static u32 __cdecl CheckRecPPCCache(RecInstruction *Instruction);
	static u32 __cdecl ProcessRecPPCCache(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs);
	static u32 __cdecl ProcessRecPPCCacheSpecial(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs);
	static GekkoRecCacheOp(ADD);
	static GekkoRecCacheOp(AND);
//	static GekkoRecCacheOp(IDIV);
//	static GekkoRecCacheOp(DIV);
	static GekkoRecCacheOp(OR);
	static GekkoRecCacheOp(SMUL);
//	static GekkoRecCacheOp(SMULH);
//	static GekkoRecCacheOp(UMULH);
	static GekkoRecCacheOp(NOT);
	static GekkoRecCacheOp(NEG);
	static GekkoRecCacheOp(SUB);
	static GekkoRecCacheOp(XOR);
	static GekkoRecCacheOp(MOVE);
	static GekkoRecCacheOp(ROL);
	static GekkoRecCacheOp(ROR);
	static GekkoRecCacheOp(SHL);
	static GekkoRecCacheOp(SHR);
	static GekkoRecCacheOp(SAL);
	static GekkoRecCacheOp(SAR);
	static GekkoRecCacheOp(UPDATECR0);
	static GekkoRecCacheOp(UPDATEXERSOOV);
	static GekkoRecCacheOp(ADDCARRY);
	static GekkoRecCacheOp(UPDATECARRY);
	static GekkoRecCacheOp(UPDATETEMPCARRY);
	static GekkoRecCacheOp(UPDATECARRYWTEMP);
	static GekkoRecCacheOp(SETCARRY);
	static GekkoRecCacheOp(CLEARCARRY);
	static GekkoRecCacheOp(COMPLIMENTCARRY);
	static GekkoRecCacheOp(CMPUPDATECR);
	static GekkoRecCacheOp(CMPUPDATECRSIGNED);

	#define RecCache_Known	0x01
    #define RecCache_WasKnown 0x02

	#define GekkoCPURecOpsGroup(name) RecOpData GekkoCPURecompiler::GekkoRecCompOps##name

	static RecOpData GekkoRecCompOpsGroups[];
	static RecOpData GekkoRecCompOpsMath[];
	static RecOpData GekkoRecCompOpsLoadStore[];
	static RecOpData GekkoRecCompOpsBranch[];
	static RecOpData GekkoRecCompOpsCompare[];
	static RecOpData GekkoRecCompOpsNOP[];

	static RecOpData GekkoRecCompOpsFPULoadStore[];
	static RecOpData GekkoRecCompOpsFPUMath[];

	static optable		GekkoCPURecOpsGroup4XO0Table[0x400];
	static optable		GekkoCPURecOpsGroup4Table[0x30];
	static optable		GekkoCPURecOpsGroup19Table[0x400];
	static optable		GekkoCPURecOpsGroup31Table[0x400];
	static optable		GekkoCPURecOpsGroup59Table[0x400];
	static optable		GekkoCPURecOpsGroup63XO0Table[0x400];
	static optable		GekkoCPURecOpsGroup63Table[0x20];
	static optable		GekkoCPURecOpset[0x40];

	static GekkoRecOp(Ops_Group4);
	static GekkoRecOp(Ops_Group19);
	static GekkoRecOp(Ops_Group31);
	static GekkoRecOp(Ops_Group59);
	static GekkoRecOp(Ops_Group63);

	static GekkoRecOp(Ops_Group4XO0);
	static GekkoRecOp(Ops_Group63XO0);

	static GekkoRecIntOp(Ops_Group4);
	static GekkoRecIntOp(Ops_Group19);
	static GekkoRecIntOp(Ops_Group31);
	static GekkoRecIntOp(Ops_Group59);
	static GekkoRecIntOp(Ops_Group63);

	static GekkoRecIntOp(Ops_Group4XO0);
	static GekkoRecIntOp(Ops_Group63XO0);

	static u32		ESPSave;
	static u32		OldPC;

	static u32		XER_CARRY;
	static u32		XER_SUMMARYOVERFLOW;
	static u32		XER_OVERFLOW;
	static u32		XER_WORDINDEX;
	static u32		XER_TEMPCARRY;
	static u32		CR[32];

	static void Gekko_CalculateCr0( u32 X );
	static void Gekko_CalculateCompare_Uint( u32 x, u32 y, u32 B );
	static void Gekko_CalculateCompare_Sint( s32 x, s32 y, u32 B );
	static void Gekko_CalculateCompareFloat( f64 x, f64 y, u32 B );
	static void Gekko_Carry(u32 _a, u32 _b, u32 _c);
	static void Gekko_CalculateXerSoOv( u64 X );
	static void Gekko_CalculateCr1( void );

	typedef struct CompiledBlock
	{
		GekkoFP					CodeBlock;
		u32						CodeStart;
		u32						CommandLen;
		u32						InstCount;
//		struct CompiledBlock	*ParentBlock;
		struct CompiledBlock	*NextBlock;
	} CompiledBlock;

	static CompiledBlock	CompileBlockNeeded;

	typedef struct RecInstruction
	{
		union
		{
			u32 InReg;		//register to read from
			u32 InVal;		//value to read
			f32	InFloat;	//float value input
			u32	InMem;		//memory to read from, note, this means a potential call to
							//Memory_Read*
		};

		union
		{
			u32	OutReg;		//register to store to
			u32	OutMem;		//memory to write to, note, this will call Memory_Write* due
							//to the potential of writing to code
		};

		union
		{
			struct
			{
				union
				{
					u32	X86InReg;
					u32	X86InMemory;
					u32	X86InVal;
					f32	X86InFloat;
				};
				u32	TempData;
			};

			u64	X86InValLarge;
			f64	X86InFloatLarge;
		};

		union
		{
			u32 X86OutVal;
			u32	X86OutReg;
			u32 X86OutMemory;
		};

		u32						Address;		//memory address of this instruction
		RecCacheInstructionPtr	PPCCacheInst;
		RecILInstructionPtr		Instruction;
		s32						X86Displacement;
		u32						X86CmdImmediateVal;
		u32					Flags;				//Bit 0-2	In Type PPC
												//			1 = Register
												//			2 = Value
												//			3 = Float
												//			4 = Memory
												//			5 = Float 64bit
												//			6 = Float Register
												//			7 = Known Value
												//Bit 3-4	Out Type PPC
												//			1 = Register
												//			2 = Memory
												//			3 = Float Register 64bit
												//			4 = Float Register 128bit
												//Bit 8-10	In Type x86
												//			1 = Register
												//			2 = Value
												//			3 = Float
												//			4 = Memory
												//			5 = Float 64bit
												//			6 = Float Register
												//			7 = Value 64bit
												//Bit 11-13	Out Type x86
												//			1 = Register
												//			2 = Memory
												//			3 = Float Register
												//			4 = Known Out Value (WriteMem)
												//Bit 14	FPU memory access allowed
												//Bit 115	Unused
												//Bit 16	Instruction can not be optimized
												//Bit 17	Do not count ppc registers
												//Bit 18	Set if a branch instruction
												//Bit 19	Unused
												//Bit 20	Known value for the result register
												//Bit 21	Known in value
												//Bit 22	Temp data
												//Bit 23	Known out value
												//Bit 24	Instruction is interpreted
		RecInstruction	*Prev;	//previous instruction
		RecInstruction	*Next;	//next instruction
	} RecInstruction;

#define RecInstrFlagPPCInReg			0x0001
#define RecInstrFlagPPCInVal			0x0002
#define	RecInstrFlagPPCInFloat			0x0003
#define RecInstrFlagPPCInMem			0x0004
#define	RecInstrFlagPPCInFloat64		0x0005
#define RecInstrFlagPPCInFloatReg		0x0006
#define RecInstrFlagPPCKnownPPCValue	0x0007
#define RecInstrFlagPPCOutReg			0x0008
#define	RecInstrFlagPPCOutMem			0x0010
#define RecInstrFlagPPCOutFloatReg		0x0018
//#define RecInstrFlagPPCOutFloat128Reg	0x0020
#define RecInstrFlagPPC_INMASK			0x0007
#define RecInstrFlagPPC_OUTMASK			0x0038
#define RecInstrFlagPPC_MASK			(RecInstrFlagPPC_INMASK | RecInstrFlagPPC_OUTMASK)


#define RecInstrFlagX86InReg			0x0100
#define RecInstrFlagX86InVal			0x0200
#define	RecInstrFlagX86InFloat			0x0300
#define RecInstrFlagX86InMem			0x0400
#define	RecInstrFlagX86InFloat64		0x0500
#define RecInstrFlagX86InFloatReg		0x0600
#define RecInstrFlagX86InVal64			0x0700
#define RecInstrFlagX86OutReg			0x0800
#define	RecInstrFlagX86OutMem			0x1000
#define RecInstrFlagX86OutFloatReg		0x1800
#define RecInstrFlagX86OutVal			0x2000		//used for WriteMem only
#define RecInstrFlagX86_INMASK			0x0700
#define RecInstrFlagX86_OUTMASK			0x3800
#define RecInstrFlagX86_MASK			(RecInstrFlagX86_INMASK | RecInstrFlagX86_OUTMASK)

#define RecInstrFlag_NoOptimize			0x10000
#define RecInstrFlag_NoOptimize_MASK	0x10000

#define RecInstrFlag_FPUMemoryAllowed	0x00004000
#define RecInstrFlag_NoPPCRegCount		0x00020000
#define RecInstrFlag_Branch				0x00040000
#define RecInstrFlag_MemoryWrite		0x00080000
#define RecInstrFlag_KnownValue			0x00100000
#define RecInstrFlag_KnownInValue		0x00200000
#define RecInstrFlag_TempData			0x00400000
#define RecInstrFlag_KnownOutValue		0x00800000
#define RecInstrFlag_IntInstruction		0x01000000
#define RecInstrFlag_MemoryRead			0x02000000
#define RecInstrFlag_OutSpecialCase		0x04000000

//REG_X86_CACHE is used during caching of register values
#define REG_X86_CACHE					0x000000040
#define REG_EAX							(0x00000000 | REG_X86_CACHE)
#define REG_ECX							(0x00000001 | REG_X86_CACHE)
#define REG_EDX							(0x00000002 | REG_X86_CACHE)
#define REG_EBX							(0x00000003 | REG_X86_CACHE)
#define REG_ESP							(0x00000004 | REG_X86_CACHE)
#define REG_EBP							(0x00000005 | REG_X86_CACHE)
#define REG_ESI							(0x00000006 | REG_X86_CACHE)
#define REG_EDI							(0x00000007 | REG_X86_CACHE)
#define REG_SPECIAL						0x80000000

#define REG_EAX_BIT						0x00000001
#define REG_ECX_BIT						0x00000002
#define REG_EDX_BIT						0x00000004
#define REG_EBX_BIT						0x00000008
#define REG_ESP_BIT						0x00000010
#define REG_EBP_BIT						0x00000020
#define REG_ESI_BIT						0x00000040
#define REG_EDI_BIT						0x00000080

#define REG_PPC_SPECIAL					(0x40000000)
#define REG_PPC_SPECIALMASK				(0x40FFF000)
#define REG_PPC_SPR						(0x00001000 | REG_PPC_SPECIAL)
#define REG_PPC_SR						(0x00002000 | REG_PPC_SPECIAL)
#define REG_PPC_CR						(0x00003000 | REG_PPC_SPECIAL)
#define REG_PPC_PC						(0x00004000 | REG_PPC_SPECIAL)
#define REG_PPC_MSR						(0x00005000 | REG_PPC_SPECIAL)
#define REG_PPC_TBL						(0x00006000 | REG_PPC_SPECIAL)
#define REG_PPC_TBU						(0x00007000 | REG_PPC_SPECIAL)
#define REG_PPC_BRANCH					(0x00008000 | REG_PPC_SPECIAL)
#define REG_PPC_XER_WORDINDEX			(0x00009000 | REG_PPC_SPECIAL)
#define REG_PPC_XER_CARRY				(0x0000A000 | REG_PPC_SPECIAL)
#define REG_PPC_XER_OVERFLOW			(0x0000B000 | REG_PPC_SPECIAL)
#define REG_PPC_XER_SUMMARYOVERFLOW		(0x0000C000 | REG_PPC_SPECIAL)
#define TEMP_FPU_REG					(0x0000D000 | REG_PPC_SPECIAL)
#define Branch_RFI						2
#define Branch_SC						4

	static void Unknown_Mask(char *Instr, long Mask);

	//if ANYONE knows how to #define a #define so i can stop duplicating below
	//PLEASE tell me - Lightning

	static GekkoRecILOp(ADD);
	#define REC_ADD(Out, In)		GekkoRecIL(ADD), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(ADD)
	#define REC_ADDI(Out, In)		GekkoRecIL(ADD), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(ADD)

	static GekkoRecILOp(ADDCARRY);
	#define REC_ADDCARRY(Out)		GekkoRecIL(ADDCARRY), Out, 0, RecInstrFlagPPCOutReg | RecInstrFlagX86InMem, GekkoRecCache(ADDCARRY)

	static GekkoRecILOp(ADC);
	#define REC_ADC(Out, In)		GekkoRecIL(ADC), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, 0
	#define REC_ADCI(Out, In)		GekkoRecIL(ADC), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, 0

	static GekkoRecILOp(AND);
	#define REC_AND(Out, In)		GekkoRecIL(AND), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(AND)
	#define REC_ANDI(Out, In)		GekkoRecIL(AND), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(AND)

	static GekkoRecILOp(CMP);
	#define REC_CMP(Out, In)		GekkoRecIL(CMP), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, 0
	#define REC_CMPI(Out, In)		GekkoRecIL(CMP), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, 0

	static GekkoRecILOp(DIV);
	#define REC_DIV(In)				GekkoRecIL(DIV), REG_SPECIAL | REG_EAX, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_OutSpecialCase, 0

	static GekkoRecILOp(IDIV);
	#define REC_IDIV(In)			GekkoRecIL(IDIV), REG_SPECIAL | REG_EAX, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_OutSpecialCase, 0

	static GekkoRecILOp(CDQ);
	#define REC_CDQ()				GekkoRecIL(CDQ), 0, 0, 0, 0

	static GekkoRecILOp(OR);
	#define REC_OR(Out, In)			GekkoRecIL(OR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(OR)
	#define REC_ORI(Out, In)		GekkoRecIL(OR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(OR)

	static GekkoRecILOp(SMUL);
	#define REC_SMUL(Out, In)		GekkoRecIL(SMUL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SMUL)
	#define REC_SMULI(Out, In)		GekkoRecIL(SMUL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SMUL)
/*
	static GekkoRecILOp(SMULH);
	#define REC_SMULH(Out, In)		GekkoRecIL(SMULH), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SMULH)
	#define REC_SMULHI(Out, In)		GekkoRecIL(SMULH), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SMULH)

	static GekkoRecILOp(UMULH);
	#define REC_UMULH(Out, In)		GekkoRecIL(UMULH), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(UMULH)
	#define REC_UMULHI(Out, In)		GekkoRecIL(UMULH), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(UMULH)
*/
	static GekkoRecILOp(NEG);
	#define REC_NEG(In)				GekkoRecIL(NEG), 0, In, RecInstrFlagPPCInReg, GekkoRecCache(NEG)

	static GekkoRecILOp(NOT);
	#define REC_NOT(In)				GekkoRecIL(NOT), 0, In, RecInstrFlagPPCInReg, GekkoRecCache(NOT)

	static GekkoRecILOp(NOP);
	#define REC_NOP					GekkoRecIL(NOP), 0, 0, RecInstrFlag_NoPPCRegCount, 0

	static GekkoRecILOp(ROL);
	#define REC_ROL(Out, In)		GekkoRecIL(ROL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(ROL)
	#define REC_ROLI(Out, In)		GekkoRecIL(ROL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(ROL)

	static GekkoRecILOp(ROR);
	#define REC_ROR(Out, In)		GekkoRecIL(ROR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(ROR)
	#define REC_RORI(Out, In)		GekkoRecIL(ROR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(ROR)

	static GekkoRecILOp(SETC);
	#define REC_SETC(Out)			GekkoRecIL(SETC), Out, 0, RecInstrFlagPPCOutReg, 0

	static GekkoRecILOp(SETE);
	#define REC_SETE(Out)			GekkoRecIL(SETE), Out, 0, RecInstrFlagX86OutReg, 0

	static GekkoRecILOp(SAL);
	#define REC_SAL(Out, In)		GekkoRecIL(SAL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SAL)
	#define REC_SALI(Out, In)		GekkoRecIL(SAL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SAL)

	static GekkoRecILOp(SAR);
	#define REC_SAR(Out, In)		GekkoRecIL(SAR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SAR)
	#define REC_SARI(Out, In)		GekkoRecIL(SAR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SAR)

	static GekkoRecILOp(SHL);
	#define REC_SHL(Out, In)		GekkoRecIL(SHL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SHL)
	#define REC_SHLI(Out, In)		GekkoRecIL(SHL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SHL)

	static GekkoRecILOp(SHR);
	#define REC_SHR(Out, In)		GekkoRecIL(SHR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SHR)
	#define REC_SHRI(Out, In)		GekkoRecIL(SHR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SHR)

	static GekkoRecILOp(SUB);
	#define REC_SUB(Out, In)		GekkoRecIL(SUB), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SUB)
	#define REC_SUBI(Out, In)		GekkoRecIL(SUB), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SUB)

	static GekkoRecILOp(XOR);
	#define REC_XOR(Out, In)		GekkoRecIL(XOR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(XOR)
	#define REC_XORI(Out, In)		GekkoRecIL(XOR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(XOR)

	static GekkoRecILOp(XCHG);

	static GekkoRecILOp(MOVE);
	#define REC_MOVE(Out, In)		GekkoRecIL(MOVE), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(MOVE)
	#define REC_MOVEI(Out, In)		GekkoRecIL(MOVE), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(MOVE)

	static GekkoRecILOp(READMEM8);
	#define REC_READMEM8(Out, In)		GekkoRecIL(READMEM8), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM8I(Out, In)		GekkoRecIL(READMEM8), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOp(READMEM8SX);
	#define REC_READMEM8SX(Out, In)		GekkoRecIL(READMEM8SX), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM8SXI(Out, In)	GekkoRecIL(READMEM8SX), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOp(READMEM16);
	#define REC_READMEM16(Out, In)		GekkoRecIL(READMEM16), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM16I(Out, In)		GekkoRecIL(READMEM16), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOp(READMEM16SX);
	#define REC_READMEM16SX(Out, In)	GekkoRecIL(READMEM16SX), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM16SXI(Out, In)	GekkoRecIL(READMEM16SX), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOp(READMEM32);
	#define REC_READMEM32(Out, In)		GekkoRecIL(READMEM32), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM32I(Out, In)		GekkoRecIL(READMEM32), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOp(READMEM64);
	#define REC_READMEM64(Out, In)		GekkoRecIL(READMEM64), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM64I(Out, In)		GekkoRecIL(READMEM64), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutFloatReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOp(WRITEMEM8);
	#define REC_WRITEMEM8(Out, In)		GekkoRecIL(WRITEMEM8), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM8I(Out, In)		GekkoRecIL(WRITEMEM8), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOp(WRITEMEM16);
	#define REC_WRITEMEM16(Out, In)		GekkoRecIL(WRITEMEM16), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM16I(Out, In)	GekkoRecIL(WRITEMEM16), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOp(WRITEMEM32);
	#define REC_WRITEMEM32(Out, In)		GekkoRecIL(WRITEMEM32), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM32I(Out, In)	GekkoRecIL(WRITEMEM32), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOp(WRITEMEM64);
	#define REC_WRITEMEM64(Out, In)		GekkoRecIL(WRITEMEM64), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM64I(Out, In)	GekkoRecIL(WRITEMEM64), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOp(SaveAllX86Regs);
	static GekkoRecILOp(SaveAllFPURegs);
	static GekkoRecILOp(SaveAllX86Regs2);
	static GekkoRecILOp(SaveAllFPURegs2);
	static GekkoRecILOp(GetAllX86Regs);
	static GekkoRecILOp(GetAllFPURegs);
	static GekkoRecILOp(SaveAllKnownX86ValRegs);

	#define REC_CALL(In)				GekkoRecIL(INT_INSTRUCTION), 0, In, 0, 0

	#define TEMP_REG 32
	static GekkoRecILOp(GETTEMPREG);
	#define REC_GETTEMPREG(RegMask)	GekkoRecIL(GETTEMPREG), 0, RegMask, 0, 0

	static GekkoRecILOp(FREETEMPREG);
	#define REC_FREETEMPREG()		GekkoRecIL(FREETEMPREG), 0, 0, 0, 0

	static GekkoRecILOp(GETTEMPFPUREG);
	#define REC_GETTEMPFPUREG(RegMask)	GekkoRecIL(GETTEMPFPUREG), 0, RegMask, 0, 0

	static GekkoRecILOp(FREETEMPFPUREG);
	#define REC_FREETEMPFPUREG(RegMask)	GekkoRecIL(FREETEMPFPUREG), 0, RegMask, 0, 0

	static u32 NextGetRegID_Val;
	#define NextGetRegID (++NextGetRegID_Val)

	static GekkoRecILOp(GETREG);
	#define REC_GETREG(In)			GekkoRecIL(GETREG), 0, In, 0, 0

	static GekkoRecILOp(STOREREG);
	#define REC_STOREREG(In)		GekkoRecIL(STOREREG), 0, In, 0, 0

	static GekkoRecILOp(FREEREG);
	#define REC_FREEREG(In)			GekkoRecIL(FREEREG), 0, In, 0, 0

	static GekkoRecILOp(UPDATECR0);
	#define REC_UPDATECR0(SkipCount)	GekkoRecIL(UPDATECR0), SkipCount, 0, RecInstrFlag_NoPPCRegCount, GekkoRecCache(UPDATECR0)

	static GekkoRecILOp(UPDATECRSO);
	#define REC_UPDATECRSO(Entry)	GekkoRecIL(UPDATECRSO), Entry, 0, RecInstrFlag_NoPPCRegCount | RecInstrFlag_TempData, 0
	static GekkoRecILOp(UPDATECRSIGNED);
	#define REC_UPDATECRSIGNED(Entry)	GekkoRecIL(UPDATECRSIGNED), Entry, 0, RecInstrFlag_NoPPCRegCount | RecInstrFlag_TempData, 0
	static GekkoRecILOp(UPDATECRUNSIGNED);
	#define REC_UPDATECRUNSIGNED(Entry)	GekkoRecIL(UPDATECRUNSIGNED), Entry, 0, RecInstrFlag_NoPPCRegCount | RecInstrFlag_TempData, 0

	static GekkoRecILOp(CMPUPDATECR);
	#define REC_CMPUPDATECR(Reg, Reg2)	GekkoRecIL(CMPUPDATECR), Reg, Reg2, RecInstrFlagPPCOutReg | RecInstrFlagPPCInReg, 0
	#define REC_CMPUPDATECRI(Reg, Val)	GekkoRecIL(CMPUPDATECR), Reg, Val, RecInstrFlagPPCOutReg | RecInstrFlagPPCInVal, 0

	static GekkoRecILOp(CMPUPDATECRSIGNED);
	#define REC_CMPUPDATECRSIGNED(Reg, Reg2)	GekkoRecIL(CMPUPDATECRSIGNED), Reg, Reg2, RecInstrFlagPPCOutReg | RecInstrFlagPPCInReg, 0
	#define REC_CMPUPDATECRSIGNEDI(Reg, Val)	GekkoRecIL(CMPUPDATECRSIGNED), Reg, Val, RecInstrFlagPPCOutReg | RecInstrFlagPPCInVal, 0

	static GekkoRecILOp(UPDATEXERSOOV);
	#define REC_UPDATEXERSOOV(In)	GekkoRecIL(UPDATEXERSOOV), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATEXERSOOV)

	static GekkoRecILOp(UPDATETEMPCARRY);
	static GekkoRecILOp(UPDATECARRYWTEMP);
	static GekkoRecILOp(UPDATECARRY);
	static GekkoRecILOp(SETCARRY);
	static GekkoRecILOp(CLEARCARRY);
	static GekkoRecILOp(COMPLIMENTCARRY);
	#define REC_UPDATETEMPCARRY(In)		GekkoRecIL(UPDATETEMPCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATETEMPCARRY)
	#define REC_UPDATECARRYWTEMP(In)	GekkoRecIL(UPDATECARRYWTEMP), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATECARRYWTEMP)
	#define REC_UPDATECARRY(In)			GekkoRecIL(UPDATECARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATECARRY)
	#define REC_SETCARRY(In)			GekkoRecIL(SETCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(SETCARRY)
	#define REC_CLEARCARRY(In)			GekkoRecIL(CLEARCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(CLEARCARRY)
	#define REC_COMPLIMENTCARRY(In)		GekkoRecIL(COMPLIMENTCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(COMPLIMENTCARRY)

	static GekkoRecILOp(PUSHA);
	static GekkoRecILOp(POPA);
	static GekkoRecILOp(RET);

	static GekkoRecILOp(COMPRESS_CR);
	static GekkoRecILOp(DECOMPRESS_CR);
	static GekkoRecILOp(COMPRESS_XER);
	static GekkoRecILOp(DECOMPRESS_XER);
	static GekkoRecILOp(INT_INSTRUCTION);
	#define REC_INT_INSTRUCTION(In)		GekkoRecIL(INT_INSTRUCTION), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInVal | RecInstrFlag_IntInstruction, 0

	static GekkoRecILOp(JUMPLABEL);
	static GekkoRecILOp(JUMP);
	#define REC_JUMPLABEL(LabelID)	GekkoRecIL(JUMPLABEL), 0, LabelID, RecInstrFlag_NoPPCRegCount, 0
/*
	#define REC_JMP(LabelID)		GekkoRecIL(JUMP), 0xEB, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JA(LabelID)			GekkoRecIL(JUMP), 0x77, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JAE(LabelID)		GekkoRecIL(JUMP), 0x73, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JB(LabelID)			GekkoRecIL(JUMP), 0x72, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JBE(LabelID)		GekkoRecIL(JUMP), 0x76, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JC(LabelID)			GekkoRecIL(JUMP), 0x72, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JE(LabelID)			GekkoRecIL(JUMP), 0x74, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JG(LabelID)			GekkoRecIL(JUMP), 0x7F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JGE(LabelID)		GekkoRecIL(JUMP), 0x7D, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JL(LabelID)			GekkoRecIL(JUMP), 0x7C, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JLE(LabelID)		GekkoRecIL(JUMP), 0x7E, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNA(LabelID)		GekkoRecIL(JUMP), 0x76, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNAE(LabelID)		GekkoRecIL(JUMP), 0x72, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNB(LabelID)		GekkoRecIL(JUMP), 0x73, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNBE(LabelID)		GekkoRecIL(JUMP), 0x77, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNC(LabelID)		GekkoRecIL(JUMP), 0x73, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNE(LabelID)		GekkoRecIL(JUMP), 0x75, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNG(LabelID)		GekkoRecIL(JUMP), 0x7E, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNGE(LabelID)		GekkoRecIL(JUMP), 0x7C, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNL(LabelID)		GekkoRecIL(JUMP), 0x7D, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNLE(LabelID)		GekkoRecIL(JUMP), 0x7F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNO(LabelID)		GekkoRecIL(JUMP), 0x71, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNP(LabelID)		GekkoRecIL(JUMP), 0x7B, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNS(LabelID)		GekkoRecIL(JUMP), 0x79, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNZ(LabelID)		GekkoRecIL(JUMP), 0x75, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JO(LabelID)			GekkoRecIL(JUMP), 0x70, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JP(LabelID)			GekkoRecIL(JUMP), 0x7A, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JPE(LabelID)		GekkoRecIL(JUMP), 0x7A, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JPO(LabelID)		GekkoRecIL(JUMP), 0x7B, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JS(LabelID)			GekkoRecIL(JUMP), 0x78, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JZ(LabelID)			GekkoRecIL(JUMP), 0x74, LabelID, RecInstrFlag_NoPPCRegCount, 0
*/
	#define REC_JMP(LabelID)		GekkoRecIL(JUMP), 0xE9, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JA(LabelID)			GekkoRecIL(JUMP), 0x870F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JAE(LabelID)		GekkoRecIL(JUMP), 0x830F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JB(LabelID)			GekkoRecIL(JUMP), 0x820F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JBE(LabelID)		GekkoRecIL(JUMP), 0x860F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JC(LabelID)			GekkoRecIL(JUMP), 0x820F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JE(LabelID)			GekkoRecIL(JUMP), 0x840F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JG(LabelID)			GekkoRecIL(JUMP), 0x8F0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JGE(LabelID)		GekkoRecIL(JUMP), 0x8D0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JL(LabelID)			GekkoRecIL(JUMP), 0x8C0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JLE(LabelID)		GekkoRecIL(JUMP), 0x8E0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNA(LabelID)		GekkoRecIL(JUMP), 0x860F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNAE(LabelID)		GekkoRecIL(JUMP), 0x820F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNB(LabelID)		GekkoRecIL(JUMP), 0x830F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNBE(LabelID)		GekkoRecIL(JUMP), 0x870F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNC(LabelID)		GekkoRecIL(JUMP), 0x830F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNE(LabelID)		GekkoRecIL(JUMP), 0x850F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNG(LabelID)		GekkoRecIL(JUMP), 0x8E0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNGE(LabelID)		GekkoRecIL(JUMP), 0x8C0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNL(LabelID)		GekkoRecIL(JUMP), 0x8D0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNLE(LabelID)		GekkoRecIL(JUMP), 0x8F0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNO(LabelID)		GekkoRecIL(JUMP), 0x810F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNP(LabelID)		GekkoRecIL(JUMP), 0x8B0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNS(LabelID)		GekkoRecIL(JUMP), 0x890F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JNZ(LabelID)		GekkoRecIL(JUMP), 0x850F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JO(LabelID)			GekkoRecIL(JUMP), 0x800F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JP(LabelID)			GekkoRecIL(JUMP), 0x8A0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JPE(LabelID)		GekkoRecIL(JUMP), 0x8A0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JPO(LabelID)		GekkoRecIL(JUMP), 0x8B0F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JS(LabelID)			GekkoRecIL(JUMP), 0x880F, LabelID, RecInstrFlag_NoPPCRegCount, 0
	#define REC_JZ(LabelID)			GekkoRecIL(JUMP), 0x840F, LabelID, RecInstrFlag_NoPPCRegCount, 0

	static void GekkoCPURecompiler::ProcessJump(JumpList *LabelEntry, JumpList *JumpEntry);
	static u32	NextJumpID_Val;
	#define NextJumpID (++NextJumpID_Val)

	#define REC_JUMPHINT_TAKEN		(0x01 << 24)
	#define REC_JUMPHINT_NOTTAKEN	(0x02 << 24)

	static u32 CachingEnabled;
	static GekkoRecILOp(TURNOFFCACHING);
	#define REC_TURNOFFCACHING()	GekkoRecIL(TURNOFFCACHING), 0, 0, 0, 0

	static GekkoRecILOp(TURNONCACHING);
	#define REC_TURNONCACHING()		GekkoRecIL(TURNONCACHING), 0, 0, 0, 0


	//fpu commands
	static GekkoRecILOp(ADDPD);
	#define REC_ADDPD(Out, In)		GekkoRecIL(ADDPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(ADDSD);
	#define REC_ADDSD(Out, In)		GekkoRecIL(ADDSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(DIVPD);
	#define REC_DIVPD(Out, In)		GekkoRecIL(DIVPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(DIVSD);
	#define REC_DIVSD(Out, In)		GekkoRecIL(DIVSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(MULPD);
	#define REC_MULPD(Out, In)		GekkoRecIL(MULPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(MULSD);
	#define REC_MULSD(Out, In)		GekkoRecIL(MULSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(SUBPD);
	#define REC_SUBPD(Out, In)		GekkoRecIL(SUBPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(SUBSD);
	#define REC_SUBSD(Out, In)		GekkoRecIL(SUBSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(MOVSD);
	#define REC_MOVSD(Out, In)		GekkoRecIL(MOVSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_MOVSDM(Out, In)		GekkoRecIL(MOVSD), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOp(MOVSS);
	#define REC_MOVSS(Out, In)		GekkoRecIL(MOVSS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_MOVSSM(Out, In)		GekkoRecIL(MOVSS), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOp(MOVAPD);
	#define REC_MOVAPD(Out, In)		GekkoRecIL(MOVAPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_MOVAPDM(Out, In)	GekkoRecIL(MOVAPD), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOp(MOVLHPS);
	#define REC_MOVLHPS(Out, In)	GekkoRecIL(MOVLHPS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(MOVHLPS);
	#define REC_MOVHLPS(Out, In)	GekkoRecIL(MOVHLPS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(MOVDDUP);
	#define REC_MOVDDUP(Out, In)	GekkoRecIL(MOVDDUP), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOp(MOVSLDUP);
	#define REC_MOVSLDUP(Out, In)	GekkoRecIL(MOVSLDUP), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOp(MOVD);
	#define REC_MOVD_TOFPU(Out, In)		GekkoRecIL(MOVD), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutFloatReg, 0
	#define REC_MOVD_FROMFPU(Out, In)	GekkoRecIL(MOVD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutReg, 0

	static GekkoRecILOp(CVTPS2PD);
	#define REC_CVTPS2PD(Out, In)	GekkoRecIL(CVTPS2PD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(CVTDQ2PD);
	#define REC_CVTDQ2PD(Out, In)	GekkoRecIL(CVTDQ2PD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(CVTPD2PS);
	#define REC_CVTPD2PS(Out, In)	GekkoRecIL(CVTPD2PS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(CVTSD2SS);
	#define REC_CVTSD2SS(Out, In)	GekkoRecIL(CVTSD2SS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(CVTSS2SD);
	#define REC_CVTSS2SD(Out, In)	GekkoRecIL(CVTSS2SD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(SHUFPD);
	#define REC_SHUFPD(Out, In)		GekkoRecIL(SHUFPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOp(PAND);
	#define REC_PAND(Out, In)		GekkoRecIL(PAND), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_PANDM(Out, In)	GekkoRecIL(PAND), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOp(POR);
	#define REC_POR(Out, In)		GekkoRecIL(POR), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_PORM(Out, In)	GekkoRecIL(POR), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOp(PXOR);
	#define REC_PXOR(Out, In)		GekkoRecIL(PXOR), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_PXORM(Out, In)	GekkoRecIL(PXOR), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static RecInstruction *LastInstruction;
	static RecInstruction *InstructionBlock;

	GekkoF	execStep();

	static GekkoF CompileInstruction();
	static GekkoF GekkoCPURecompiler::DoBranchChecks();

	GekkoCPURecompiler();
	~GekkoCPURecompiler();
	static GekkoF CheckMemoryWriteInternal(u32 Addr, u32 Size);

	static GekkoF CompileIL(u32 OldPC, u32 NewPC);
	static GekkoF AssignRegsAndCompile(PPCRegInfo *X86Regs, PPCRegInfo *FPURegs, u8 *X86Buffer, u32 *BufferSize);

#define X86AssignRegsOp(name)		OPTYPE GekkoCPURecompiler::AssignX86Regs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define X86AssignRegs(name)			GekkoCPURecompiler::AssignX86Regs_PPC##name
#define FPUAssignRegsOp(name)		OPTYPE GekkoCPURecompiler::AssignFPURegs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define FPUAssignRegs(name)			GekkoCPURecompiler::AssignFPURegs_PPC##name
#define X86FPUAssignRegsOp(name)	OPTYPE GekkoCPURecompiler::AssignX86FPURegs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define X86FPUAssignRegs(name)		GekkoCPURecompiler::AssignX86FPURegs_PPC##name
typedef void (__fastcall *RecX86FPUAssignRegsPtr)(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs);

	static X86AssignRegsOp(NONE);
	static X86AssignRegsOp(InReg_Special);
	static X86AssignRegsOp(InReg);
	static X86AssignRegsOp(OutReg);
	static X86AssignRegsOp(InVal);
	static X86AssignRegsOp(InReg_OutReg);
	static X86AssignRegsOp(InVal_OutReg);
	static X86AssignRegsOp(InReg_OutReg_Special);
	static X86AssignRegsOp(InVal_OutReg_Special);
	static X86AssignRegsOp(KnownPPCValue);
	static X86AssignRegsOp(InReg_OutMem);
	static X86AssignRegsOp(InVal_OutMem);

	static FPUAssignRegsOp(InMem_OutReg);
	static FPUAssignRegsOp(InReg_OutReg);
	static FPUAssignRegsOp(InReg_OutMem);
	static FPUAssignRegsOp(InReg);

	static X86FPUAssignRegsOp(InReg_OutFPUReg);
	static X86FPUAssignRegsOp(InFPUReg_OutReg);
	static X86FPUAssignRegsOp(InVal_OutFPUReg);

	static RecX86FPUAssignRegsPtr		RecX86FPUAssignRegs[];

	static u32	FindReplacableX86Reg(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, 
								  u32 FindReg, u32 *FindRegMem, u32 *FindCount,
								  u32 FindNextLowest);

	static u32	FindReplacableFPUReg(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, 
								  u32 FindReg, u32 *FindRegMem, u32 *FindCount,
								  u32 FindNextLowest);

	////////////////////////////////////////////////////////////

	// Control Functions
	//

	CPUType	GetCPUType();
	GekkoF	Open(u32 entry_point);
	GekkoF	Halt();
	GekkoF	Start();

	GekkoF Exception(tGekkoException which);
	static GekkoF Exception_REC(tGekkoException which);

	u32	GetTicksPerSecond();

	static GekkoF CreateRecInstruction(RecILInstructionPtr Instruction, u32 Out, u32 In, u32 Flags, RecCacheInstructionPtr CachePtr);
#define SetRecInstructionImm(ImmVal)	LastInstruction->X86CmdImmediateVal = ImmVal;
	static GekkoRecIntOp(DUMP_OPS);
	static GekkoRecIntOp(PS_ABS);
	static GekkoRecIntOp(PS_ADD);
	static GekkoRecIntOp(PS_CMPO0);
	static GekkoRecIntOp(PS_CMPO1);
	static GekkoRecIntOp(PS_CMPU0);
	static GekkoRecIntOp(PS_CMPU1);
	static GekkoRecIntOp(PS_MADD);
	static GekkoRecIntOp(PS_MADDS0);
	static GekkoRecIntOp(PS_MADDS1);
	static GekkoRecIntOp(PS_MERGE00);
	static GekkoRecIntOp(PS_MERGE01);
	static GekkoRecIntOp(PS_MERGE10);
	static GekkoRecIntOp(PS_MERGE11);
	static GekkoRecIntOp(PS_MR);
	static GekkoRecIntOp(PS_MSUB);
	static GekkoRecIntOp(PS_MUL);
	static GekkoRecIntOp(PS_MULS0);
	static GekkoRecIntOp(PS_MULS1);
	static GekkoRecIntOp(PS_NEG);
	static GekkoRecIntOp(PS_NMADD);
	static GekkoRecIntOp(PS_NMSUB);
	static GekkoRecIntOp(PS_RES);
	static GekkoRecIntOp(PS_RSQRTE);
	static GekkoRecIntOp(PS_SUB);
	static GekkoRecIntOp(PS_SUM0);
	static GekkoRecIntOp(PS_SUM1);
	static GekkoRecIntOp(PS_SEL);
	static GekkoRecIntOp(NI);
	static GekkoRecIntOp(ADD);
	static GekkoRecIntOp(ADDO);
	static GekkoRecIntOp(ADDC);
	static GekkoRecIntOp(ADDCO);
	static GekkoRecIntOp(ADDE);
	static GekkoRecIntOp(ADDEO);
	static GekkoRecIntOp(ADDI);
	static GekkoRecIntOp(ADDIC);
	static GekkoRecIntOp(ADDICD);
	static GekkoRecIntOp(ADDIS);
	static GekkoRecIntOp(ADDME);
	static GekkoRecIntOp(ADDMEO);
	static GekkoRecIntOp(ADDZE);
	static GekkoRecIntOp(ADDZEO);
	static GekkoRecIntOp(AND);
	static GekkoRecIntOp(ANDC);
	static GekkoRecIntOp(ANDID);
	static GekkoRecIntOp(ANDISD);
	static GekkoRecIntOp(B);
	static GekkoRecIntOp(BL);
	static GekkoRecIntOp(BA);
	static GekkoRecIntOp(BLA);
	static GekkoRecIntOp(BX);
	static GekkoRecIntOp(BCX);
	static GekkoRecIntOp(BCCTRX);
	static GekkoRecIntOp(BCLRX);
	static GekkoRecIntOp(CMP);
	static GekkoRecIntOp(CMPI);
	static GekkoRecIntOp(CMPL);
	static GekkoRecIntOp(CMPLI);
	static GekkoRecIntOp(CNTLZW);
	static GekkoRecIntOp(CRAND);
	static GekkoRecIntOp(CRANDC);
	static GekkoRecIntOp(CREQV);
	static GekkoRecIntOp(CRNAND);
	static GekkoRecIntOp(CRNOR);
	static GekkoRecIntOp(CROR);
	static GekkoRecIntOp(CRORC);
	static GekkoRecIntOp(CRXOR);
	static GekkoRecIntOp(DCBA);
	static GekkoRecIntOp(DCBF);
	static GekkoRecIntOp(DCBI);
	static GekkoRecIntOp(DCBST);
	static GekkoRecIntOp(DCBT);
	static GekkoRecIntOp(DCBTST);
	static GekkoRecIntOp(DCBZ);
	static GekkoRecIntOp(DCBZ_L);
	static GekkoRecIntOp(DIVW);
	static GekkoRecIntOp(DIVWO);
	static GekkoRecIntOp(DIVWOD);
	static GekkoRecIntOp(DIVWU);
	static GekkoRecIntOp(DIVWUO);
	static GekkoRecIntOp(ECIWX);
	static GekkoRecIntOp(ECOWX);
	static GekkoRecIntOp(EIEIO);
	static GekkoRecIntOp(EQV);
	static GekkoRecIntOp(EXTSB);
	static GekkoRecIntOp(EXTSH);
	static GekkoRecIntOp(FABS);
	static GekkoRecIntOp(FADD);
	static GekkoRecIntOp(FADDS);
	static GekkoRecIntOp(FCMPO);
	static GekkoRecIntOp(FCMPU);
	static GekkoRecIntOp(FCTIW);
	static GekkoRecIntOp(FCTIWZ);
	static GekkoRecIntOp(FDIV);
	static GekkoRecIntOp(FDIVS);
	static GekkoRecIntOp(FMADD);
	static GekkoRecIntOp(FMADDS);
	static GekkoRecIntOp(FMR);
	static GekkoRecIntOp(FMSUB);
	static GekkoRecIntOp(FMSUBS);
	static GekkoRecIntOp(FMUL);
	static GekkoRecIntOp(FMULS);
	static GekkoRecIntOp(FNABS);
	static GekkoRecIntOp(FNEG);
	static GekkoRecIntOp(FNMADD);
	static GekkoRecIntOp(FNMADDS);
	static GekkoRecIntOp(FNMSUB);
	static GekkoRecIntOp(FNMSUBS);
	static GekkoRecIntOp(FRES);
	static GekkoRecIntOp(FRSP);
	static GekkoRecIntOp(FRSQRTE);
	static GekkoRecIntOp(FSEL);
	static GekkoRecIntOp(FSQRT);
	static GekkoRecIntOp(FSQRTS);
	static GekkoRecIntOp(FSUB);
	static GekkoRecIntOp(FSUBS);
	static GekkoRecIntOp(ICBI);
	static GekkoRecIntOp(ISYNC);
	static GekkoRecIntOp(LBZ);
	static GekkoRecIntOp(LBZU);
	static GekkoRecIntOp(LBZUX);
	static GekkoRecIntOp(LBZX);
	static GekkoRecIntOp(LFD);
	static GekkoRecIntOp(LFDU);
	static GekkoRecIntOp(LFDUX);
	static GekkoRecIntOp(LFDX);
	static GekkoRecIntOp(LFS);
	static GekkoRecIntOp(LFSU);
	static GekkoRecIntOp(LFSUX);
	static GekkoRecIntOp(LFSX);
	static GekkoRecIntOp(LHA);
	static GekkoRecIntOp(LHAU);
	static GekkoRecIntOp(LHAUX);
	static GekkoRecIntOp(LHAX);
	static GekkoRecIntOp(LHBRX);
	static GekkoRecIntOp(LHZ);
	static GekkoRecIntOp(LHZU);
	static GekkoRecIntOp(LHZUX);
	static GekkoRecIntOp(LHZX);
	static GekkoRecIntOp(LMW);
	static GekkoRecIntOp(LSWI);
	static GekkoRecIntOp(LSWX);
	static GekkoRecIntOp(LWARX);
	static GekkoRecIntOp(LWBRX);
	static GekkoRecIntOp(LWZ);
	static GekkoRecIntOp(LWZU);
	static GekkoRecIntOp(LWZUX);
	static GekkoRecIntOp(LWZX);
	static GekkoRecIntOp(MCRF);
	static GekkoRecIntOp(MCRFS);
	static GekkoRecIntOp(MCRXR);
	static GekkoRecIntOp(MFCR);
	static GekkoRecIntOp(MFFS);
	static GekkoRecIntOp(MFMSR);
	static GekkoRecIntOp(MFSPR);
	static GekkoRecIntOp(MFSR);
	static GekkoRecIntOp(MFSRIN);
	static GekkoRecIntOp(MFTB);
	static GekkoRecIntOp(MTCRF);
	static GekkoRecIntOp(MTFSB0);
	static GekkoRecIntOp(MTFSB1);
	static GekkoRecIntOp(MTFSF);
	static GekkoRecIntOp(MTSFI);
	static GekkoRecIntOp(MTMSR);
	static GekkoRecIntOp(MTSPR);
	static GekkoRecIntOp(MTSR);
	static GekkoRecIntOp(MTSRIN);
	static GekkoRecIntOp(MULHW);
	static GekkoRecIntOp(MULHWD);
	static GekkoRecIntOp(MULHWU);
	static GekkoRecIntOp(MULHWUD);
	static GekkoRecIntOp(MULLI);
	static GekkoRecIntOp(MULLW);
	static GekkoRecIntOp(MULLWD);
	static GekkoRecIntOp(MULLWO);
	static GekkoRecIntOp(MULLWOD);
	static GekkoRecIntOp(NAND);
	static GekkoRecIntOp(NEG);
	static GekkoRecIntOp(NEGO);
	static GekkoRecIntOp(NOR);
	static GekkoRecIntOp(OR);
	static GekkoRecIntOp(ORC);
	static GekkoRecIntOp(ORI);
	static GekkoRecIntOp(ORIS);
	static GekkoRecIntOp(PS_DIV);
	static GekkoRecIntOp(PSQ_L);
	static GekkoRecIntOp(PSQ_LX);
	static GekkoRecIntOp(PSQ_LU);
	static GekkoRecIntOp(PSQ_LUX);
	static GekkoRecIntOp(PSQ_ST);
	static GekkoRecIntOp(PSQ_STX);
	static GekkoRecIntOp(PSQ_STU);
	static GekkoRecIntOp(PSQ_STUX);
	static GekkoRecIntOp(RFI);
	static GekkoRecIntOp(RLWIMI);
	static GekkoRecIntOp(RLWINM);
	static GekkoRecIntOp(RLWNM);
	static GekkoRecIntOp(SC);
	static GekkoRecIntOp(SLW);
	static GekkoRecIntOp(SRAW);
	static GekkoRecIntOp(SRAWI);
	static GekkoRecIntOp(SRW);
	static GekkoRecIntOp(STB);
	static GekkoRecIntOp(STBU);
	static GekkoRecIntOp(STBUX);
	static GekkoRecIntOp(STBX);
	static GekkoRecIntOp(STFD);
	static GekkoRecIntOp(STFDU);
	static GekkoRecIntOp(STFDUX);
	static GekkoRecIntOp(STFDX);
	static GekkoRecIntOp(STFIWX);
	static GekkoRecIntOp(STFS);
	static GekkoRecIntOp(STFSU);
	static GekkoRecIntOp(STFSUX);
	static GekkoRecIntOp(STFSX);
	static GekkoRecIntOp(STH);
	static GekkoRecIntOp(STHBRX);
	static GekkoRecIntOp(STHU);
	static GekkoRecIntOp(STHUX);
	static GekkoRecIntOp(STHX);
	static GekkoRecIntOp(STMW);
	static GekkoRecIntOp(STSWI);
	static GekkoRecIntOp(STSWX);
	static GekkoRecIntOp(STW);
	static GekkoRecIntOp(STWBRX);
	static GekkoRecIntOp(STWCXD);
	static GekkoRecIntOp(STWU);
	static GekkoRecIntOp(STWUX);
	static GekkoRecIntOp(STWX);
	static GekkoRecIntOp(SUBF);
	static GekkoRecIntOp(SUBFO);
	static GekkoRecIntOp(SUBFC);
	static GekkoRecIntOp(SUBFCO);
	static GekkoRecIntOp(SUBFE);
	static GekkoRecIntOp(SUBFEO);
	static GekkoRecIntOp(SUBFIC);
	static GekkoRecIntOp(SUBFME);
	static GekkoRecIntOp(SUBFMEO);
	static GekkoRecIntOp(SUBFZE);
	static GekkoRecIntOp(SUBFZEO);
	static GekkoRecIntOp(SYNC);
	static GekkoRecIntOp(TLBIA);
	static GekkoRecIntOp(TLBIE);
	static GekkoRecIntOp(TLBSYNC);
	static GekkoRecIntOp(TW);
	static GekkoRecIntOp(TWI);
	static GekkoRecIntOp(XOR);
	static GekkoRecIntOp(XORI);
	static GekkoRecIntOp(XORIS);
	static GekkoRecIntOp(STWCX);
	static GekkoRecIntOp(MTFSFI);
	static GekkoRecIntOp(HLE);

	static GekkoRecOp(ADD);
	static GekkoRecOp(ADDO);
	static GekkoRecOp(ADDC);
	static GekkoRecOp(ADDCO);
	static GekkoRecOp(ADDE);
	static GekkoRecOp(ADDEO);
	static GekkoRecOp(ADDI);
	static GekkoRecOp(ADDIC);
	static GekkoRecOp(ADDICD);
	static GekkoRecOp(ADDIS);
	static GekkoRecOp(ADDME);
	static GekkoRecOp(ADDMEO);
	static GekkoRecOp(ADDZE);
	static GekkoRecOp(ADDZEO);
	static GekkoRecOp(AND);
	static GekkoRecOp(ANDC);
	static GekkoRecOp(ANDID);
	static GekkoRecOp(ANDISD);
	static GekkoRecOp(CMP);
	static GekkoRecOp(CMPI);
	static GekkoRecOp(CMPL);
	static GekkoRecOp(CMPLI);
	static GekkoRecOp(CNTLZW);
	static GekkoRecOp(CRAND);
	static GekkoRecOp(CRANDC);
	static GekkoRecOp(CREQV);
	static GekkoRecOp(CRNAND);
	static GekkoRecOp(CRNOR);
	static GekkoRecOp(CROR);
	static GekkoRecOp(CRORC);
	static GekkoRecOp(CRXOR);
	static GekkoRecOp(DIVW);
	static GekkoRecOp(DIVWO);
	static GekkoRecOp(DIVWU);
	static GekkoRecOp(DIVWUO);
	static GekkoRecOp(EQV);
	static GekkoRecOp(EXTSB);
	static GekkoRecOp(EXTSH);
	static GekkoRecOp(MFCR);
	static GekkoRecOp(MTCRF);
	static GekkoRecOp(MULLI);
	static GekkoRecOp(MULLW);
	static GekkoRecOp(NAND);
	static GekkoRecOp(NEG);
	static GekkoRecOp(NEGO);
	static GekkoRecOp(NOR);
	static GekkoRecOp(OR);
	static GekkoRecOp(ORD);
	static GekkoRecOp(ORC);
	static GekkoRecOp(ORI);
	static GekkoRecOp(ORIS);
	static GekkoRecOp(RLWIMI);
	static GekkoRecOp(RLWINM);
	static GekkoRecOp(RLWNM);
	static GekkoRecOp(SLW);
	static GekkoRecOp(SRAWI);
	static GekkoRecOp(SRW);
	static GekkoRecOp(SUBF);
	static GekkoRecOp(SUBFO);
	static GekkoRecOp(SUBFC);
	static GekkoRecOp(SUBFCO);
	static GekkoRecOp(SUBFE);
	static GekkoRecOp(SUBFEO);
	static GekkoRecOp(SUBFI);
	static GekkoRecOp(SUBFIC);
	static GekkoRecOp(SUBFME);
	static GekkoRecOp(SUBFMEO);
	static GekkoRecOp(SUBFZE);
	static GekkoRecOp(SUBFZEO);
	static GekkoRecOp(XOR);
	static GekkoRecOp(XORD);
	static GekkoRecOp(XORI);
	static GekkoRecOp(XORIS);

	static GekkoRecOp(INT_INSTRUCTION);
	static GekkoRecOp(NOP);
	static GekkoRecOp(LBZ);
	static GekkoRecOp(LBZU);
	static GekkoRecOp(LBZUX);
	static GekkoRecOp(LBZX);
	static GekkoRecOp(LHA);
	static GekkoRecOp(LHAX);
	static GekkoRecOp(LHZ);
	static GekkoRecOp(LHZU);
	static GekkoRecOp(LHZUX);
	static GekkoRecOp(LHZX);
	static GekkoRecOp(LMW);
	static GekkoRecOp(LWZ);
	static GekkoRecOp(LWZU);
	static GekkoRecOp(LWZUX);
	static GekkoRecOp(LWZX);
	static GekkoRecOp(MFMSR);
	static GekkoRecOp(MFSPR);
	static GekkoRecOp(MFSR);
	static GekkoRecOp(MFTB);
	static GekkoRecOp(MTMSR);
	static GekkoRecOp(MTSPR);
	static GekkoRecOp(MTSR);
	static GekkoRecOp(STB);
	static GekkoRecOp(STBU);
	static GekkoRecOp(STBUX);
	static GekkoRecOp(STBX);
	static GekkoRecOp(STH);
	static GekkoRecOp(STHU);
	static GekkoRecOp(STHUX);
	static GekkoRecOp(STHX);
	static GekkoRecOp(STMW);
	static GekkoRecOp(STW);
	static GekkoRecOp(STWU);
	static GekkoRecOp(STWUX);
	static GekkoRecOp(STWX);

	static GekkoRecOp(BCLRX);
	static GekkoRecOp(BCCTRX);
	static GekkoRecOp(BCX);
	static GekkoRecOp(BX);
	static GekkoRecOp(DCBI);
	static GekkoRecOp(RFI);

	static GekkoRecOp(FABS);
	static GekkoRecOp(FADD);
	static GekkoRecOp(FADDS);
	static GekkoRecOp(FDIV);
	static GekkoRecOp(FDIVS);
	static GekkoRecOp(FMADD);
	static GekkoRecOp(FMADDS);
	static GekkoRecOp(FMR);
	static GekkoRecOp(FMSUB);
	static GekkoRecOp(FMSUBS);
	static GekkoRecOp(FMUL);
	static GekkoRecOp(FMULS);
	static GekkoRecOp(FNEG);
	static GekkoRecOp(FRSP);
	static GekkoRecOp(FSUB);
	static GekkoRecOp(FSUBS);
	static GekkoRecOp(PS_ABS);
	static GekkoRecOp(PS_ADD);
	static GekkoRecOp(PS_DIV);
	static GekkoRecOp(PS_MADD);
	static GekkoRecOp(PS_MADDS0);
	static GekkoRecOp(PS_MADDS1);
	static GekkoRecOp(PS_MERGE00);
	static GekkoRecOp(PS_MERGE01);
	static GekkoRecOp(PS_MR);
	static GekkoRecOp(PS_MSUB);
	static GekkoRecOp(PS_MUL);
	static GekkoRecOp(PS_MULS0);
	static GekkoRecOp(PS_NEG);
	static GekkoRecOp(PS_SUB);
	static GekkoRecOp(PS_SUM0);

	static GekkoRecOp(LFD);
	static GekkoRecOp(LFS);
	static GekkoRecOp(LFSU);
	static GekkoRecOp(LFSX);
	static GekkoRecOp(STFD);
	static GekkoRecOp(STFS);
	static GekkoRecOp(STFSU);
	static GekkoRecOp(STFSUX);
	static GekkoRecOp(STFSX);
	static GekkoRecOp(PSQ_L);
};

#endif

#endif