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
#define GekkoCPUOpsGroup(name)	GekkoCPU::OpData EMU_FASTCALL GekkoCPURecompiler::GekkoRecGroupOps##name
#define GekkoCPUOpsGroupHeader(name)	GekkoCPU::OpData EMU_FASTCALL GekkoRecGroupOps##name
#define GekkoCPUOp				GekkoRecInt

#define OPTYPE              void
#define GekkoRecOp(name)		OPTYPE GekkoCPURecompiler::GekkoRecompiler_##name(void)
#define GekkoRecOpHeader(name)	OPTYPE GekkoRecompiler_##name(void)
#define GekkoRec(name)		GekkoCPURecompiler::GekkoRecompiler_##name
#define GekkoRecIntOp(name)	OPTYPE GekkoCPURecompiler::GekkoRecompilerInt_##name(void)
#define GekkoRecIntOpHeader(name)	OPTYPE GekkoRecompilerInt_##name(void)
#define GekkoRecInt(name)	GekkoCPURecompiler::GekkoRecompilerInt_##name

typedef void (EMU_FASTCALL *RecCompInstructionPtr)(void);

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

	typedef void (*RecCacheInstructionPtr)(RecInstruction *Instruction);
	typedef void (*RecILInstructionPtr)(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs);

#define GekkoRecILOp(name)	OPTYPE GekkoCPURecompiler::GekkoILInstruction_##name(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs)
#define GekkoRecILOpHeader(name)	OPTYPE GekkoILInstruction_##name(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs)
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
	#define GekkoRecCacheOpHeader(name)	OPTYPE GekkoRecCache_##name(RecInstruction *Instruction)
	#define GekkoRecCache(name)		GekkoCPURecompiler::GekkoRecCache_##name

	static u32 __cdecl CheckRecPPCCache(RecInstruction *Instruction);
	static u32 __cdecl ProcessRecPPCCache(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs);
	static u32 __cdecl ProcessRecPPCCacheSpecial(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs);
	static GekkoRecCacheOpHeader(ADD);
	static GekkoRecCacheOpHeader(AND);
//	static GekkoRecCacheOpHeader(IDIV);
//	static GekkoRecCacheOpHeader(DIV);
	static GekkoRecCacheOpHeader(OR);
	static GekkoRecCacheOpHeader(SMUL);
//	static GekkoRecCacheOpHeader(SMULH);
//	static GekkoRecCacheOpHeader(UMULH);
	static GekkoRecCacheOpHeader(NOT);
	static GekkoRecCacheOpHeader(NEG);
	static GekkoRecCacheOpHeader(SUB);
	static GekkoRecCacheOpHeader(XOR);
	static GekkoRecCacheOpHeader(MOVE);
	static GekkoRecCacheOpHeader(ROL);
	static GekkoRecCacheOpHeader(ROR);
	static GekkoRecCacheOpHeader(SHL);
	static GekkoRecCacheOpHeader(SHR);
	static GekkoRecCacheOpHeader(SAL);
	static GekkoRecCacheOpHeader(SAR);
	static GekkoRecCacheOpHeader(UPDATECR0);
	static GekkoRecCacheOpHeader(UPDATEXERSOOV);
	static GekkoRecCacheOpHeader(ADDCARRY);
	static GekkoRecCacheOpHeader(UPDATECARRY);
	static GekkoRecCacheOpHeader(UPDATETEMPCARRY);
	static GekkoRecCacheOpHeader(UPDATECARRYWTEMP);
	static GekkoRecCacheOpHeader(SETCARRY);
	static GekkoRecCacheOpHeader(CLEARCARRY);
	static GekkoRecCacheOpHeader(COMPLIMENTCARRY);
	static GekkoRecCacheOpHeader(CMPUPDATECR);
	static GekkoRecCacheOpHeader(CMPUPDATECRSIGNED);

	#define RecCache_Known	0x01
    #define RecCache_WasKnown 0x02

	#define GekkoCPURecOpsGroup(name) RecOpData GekkoCPURecompiler::GekkoRecCompOps##name
	#define GekkoCPURecOpsGroupHeader(name) RecOpData GekkoCPURecompiler::GekkoRecCompOps##name

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

	static GekkoRecOpHeader(Ops_Group4);
	static GekkoRecOpHeader(Ops_Group19);
	static GekkoRecOpHeader(Ops_Group31);
	static GekkoRecOpHeader(Ops_Group59);
	static GekkoRecOpHeader(Ops_Group63);

	static GekkoRecOpHeader(Ops_Group4XO0);
	static GekkoRecOpHeader(Ops_Group63XO0);

	static GekkoRecIntOpHeader(Ops_Group4);
	static GekkoRecIntOpHeader(Ops_Group19);
	static GekkoRecIntOpHeader(Ops_Group31);
	static GekkoRecIntOpHeader(Ops_Group59);
	static GekkoRecIntOpHeader(Ops_Group63);

	static GekkoRecIntOpHeader(Ops_Group4XO0);
	static GekkoRecIntOpHeader(Ops_Group63XO0);

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

	static void EMU_FASTCALL Unknown_Mask(char *Instr, long Mask);

	//if ANYONE knows how to #define a #define so i can stop duplicating below
	//PLEASE tell me - Lightning

	static GekkoRecILOpHeader(ADD);
	#define REC_ADD(Out, In)		GekkoRecIL(ADD), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(ADD)
	#define REC_ADDI(Out, In)		GekkoRecIL(ADD), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(ADD)

	static GekkoRecILOpHeader(ADDCARRY);
	#define REC_ADDCARRY(Out)		GekkoRecIL(ADDCARRY), Out, 0, RecInstrFlagPPCOutReg | RecInstrFlagX86InMem, GekkoRecCache(ADDCARRY)

	static GekkoRecILOpHeader(ADC);
	#define REC_ADC(Out, In)		GekkoRecIL(ADC), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, 0
	#define REC_ADCI(Out, In)		GekkoRecIL(ADC), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, 0

	static GekkoRecILOpHeader(AND);
	#define REC_AND(Out, In)		GekkoRecIL(AND), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(AND)
	#define REC_ANDI(Out, In)		GekkoRecIL(AND), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(AND)

	static GekkoRecILOpHeader(CMP);
	#define REC_CMP(Out, In)		GekkoRecIL(CMP), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, 0
	#define REC_CMPI(Out, In)		GekkoRecIL(CMP), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, 0

	static GekkoRecILOpHeader(DIV);
	#define REC_DIV(In)				GekkoRecIL(DIV), REG_SPECIAL | REG_EAX, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_OutSpecialCase, 0

	static GekkoRecILOpHeader(IDIV);
	#define REC_IDIV(In)			GekkoRecIL(IDIV), REG_SPECIAL | REG_EAX, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_OutSpecialCase, 0

	static GekkoRecILOpHeader(CDQ);
	#define REC_CDQ()				GekkoRecIL(CDQ), 0, 0, 0, 0

	static GekkoRecILOpHeader(OR);
	#define REC_OR(Out, In)			GekkoRecIL(OR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(OR)
	#define REC_ORI(Out, In)		GekkoRecIL(OR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(OR)

	static GekkoRecILOpHeader(SMUL);
	#define REC_SMUL(Out, In)		GekkoRecIL(SMUL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SMUL)
	#define REC_SMULI(Out, In)		GekkoRecIL(SMUL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SMUL)
/*
	static GekkoRecILOpHeader(SMULH);
	#define REC_SMULH(Out, In)		GekkoRecIL(SMULH), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SMULH)
	#define REC_SMULHI(Out, In)		GekkoRecIL(SMULH), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SMULH)

	static GekkoRecILOpHeader(UMULH);
	#define REC_UMULH(Out, In)		GekkoRecIL(UMULH), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(UMULH)
	#define REC_UMULHI(Out, In)		GekkoRecIL(UMULH), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(UMULH)
*/
	static GekkoRecILOpHeader(NEG);
	#define REC_NEG(In)				GekkoRecIL(NEG), 0, In, RecInstrFlagPPCInReg, GekkoRecCache(NEG)

	static GekkoRecILOpHeader(NOT);
	#define REC_NOT(In)				GekkoRecIL(NOT), 0, In, RecInstrFlagPPCInReg, GekkoRecCache(NOT)

	static GekkoRecILOpHeader(NOP);
	#define REC_NOP					GekkoRecIL(NOP), 0, 0, RecInstrFlag_NoPPCRegCount, 0

	static GekkoRecILOpHeader(ROL);
	#define REC_ROL(Out, In)		GekkoRecIL(ROL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(ROL)
	#define REC_ROLI(Out, In)		GekkoRecIL(ROL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(ROL)

	static GekkoRecILOpHeader(ROR);
	#define REC_ROR(Out, In)		GekkoRecIL(ROR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(ROR)
	#define REC_RORI(Out, In)		GekkoRecIL(ROR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(ROR)

	static GekkoRecILOpHeader(SETC);
	#define REC_SETC(Out)			GekkoRecIL(SETC), Out, 0, RecInstrFlagPPCOutReg, 0

	static GekkoRecILOpHeader(SETE);
	#define REC_SETE(Out)			GekkoRecIL(SETE), Out, 0, RecInstrFlagX86OutReg, 0

	static GekkoRecILOpHeader(SAL);
	#define REC_SAL(Out, In)		GekkoRecIL(SAL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SAL)
	#define REC_SALI(Out, In)		GekkoRecIL(SAL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SAL)

	static GekkoRecILOpHeader(SAR);
	#define REC_SAR(Out, In)		GekkoRecIL(SAR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SAR)
	#define REC_SARI(Out, In)		GekkoRecIL(SAR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SAR)

	static GekkoRecILOpHeader(SHL);
	#define REC_SHL(Out, In)		GekkoRecIL(SHL), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SHL)
	#define REC_SHLI(Out, In)		GekkoRecIL(SHL), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SHL)

	static GekkoRecILOpHeader(SHR);
	#define REC_SHR(Out, In)		GekkoRecIL(SHR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SHR)
	#define REC_SHRI(Out, In)		GekkoRecIL(SHR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SHR)

	static GekkoRecILOpHeader(SUB);
	#define REC_SUB(Out, In)		GekkoRecIL(SUB), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(SUB)
	#define REC_SUBI(Out, In)		GekkoRecIL(SUB), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(SUB)

	static GekkoRecILOpHeader(XOR);
	#define REC_XOR(Out, In)		GekkoRecIL(XOR), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(XOR)
	#define REC_XORI(Out, In)		GekkoRecIL(XOR), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(XOR)

	static GekkoRecILOpHeader(XCHG);

	static GekkoRecILOpHeader(MOVE);
	#define REC_MOVE(Out, In)		GekkoRecIL(MOVE), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg, GekkoRecCache(MOVE)
	#define REC_MOVEI(Out, In)		GekkoRecIL(MOVE), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg, GekkoRecCache(MOVE)

	static GekkoRecILOpHeader(READMEM8);
	#define REC_READMEM8(Out, In)		GekkoRecIL(READMEM8), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM8I(Out, In)		GekkoRecIL(READMEM8), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOpHeader(READMEM8SX);
	#define REC_READMEM8SX(Out, In)		GekkoRecIL(READMEM8SX), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM8SXI(Out, In)	GekkoRecIL(READMEM8SX), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOpHeader(READMEM16);
	#define REC_READMEM16(Out, In)		GekkoRecIL(READMEM16), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM16I(Out, In)		GekkoRecIL(READMEM16), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOpHeader(READMEM16SX);
	#define REC_READMEM16SX(Out, In)	GekkoRecIL(READMEM16SX), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM16SXI(Out, In)	GekkoRecIL(READMEM16SX), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOpHeader(READMEM32);
	#define REC_READMEM32(Out, In)		GekkoRecIL(READMEM32), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM32I(Out, In)		GekkoRecIL(READMEM32), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOpHeader(READMEM64);
	#define REC_READMEM64(Out, In)		GekkoRecIL(READMEM64), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_MemoryRead, 0
	#define REC_READMEM64I(Out, In)		GekkoRecIL(READMEM64), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutFloatReg | RecInstrFlag_MemoryRead, 0

	static GekkoRecILOpHeader(WRITEMEM8);
	#define REC_WRITEMEM8(Out, In)		GekkoRecIL(WRITEMEM8), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM8I(Out, In)		GekkoRecIL(WRITEMEM8), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOpHeader(WRITEMEM16);
	#define REC_WRITEMEM16(Out, In)		GekkoRecIL(WRITEMEM16), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM16I(Out, In)	GekkoRecIL(WRITEMEM16), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOpHeader(WRITEMEM32);
	#define REC_WRITEMEM32(Out, In)		GekkoRecIL(WRITEMEM32), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM32I(Out, In)	GekkoRecIL(WRITEMEM32), Out, In, RecInstrFlagPPCInVal | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOpHeader(WRITEMEM64);
	#define REC_WRITEMEM64(Out, In)		GekkoRecIL(WRITEMEM64), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutReg | RecInstrFlag_MemoryWrite, 0
	#define REC_WRITEMEM64I(Out, In)	GekkoRecIL(WRITEMEM64), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutMem | RecInstrFlag_MemoryWrite, 0

	static GekkoRecILOpHeader(SaveAllX86Regs);
	static GekkoRecILOpHeader(SaveAllFPURegs);
	static GekkoRecILOpHeader(SaveAllX86Regs2);
	static GekkoRecILOpHeader(SaveAllFPURegs2);
	static GekkoRecILOpHeader(GetAllX86Regs);
	static GekkoRecILOpHeader(GetAllFPURegs);
	static GekkoRecILOpHeader(SaveAllKnownX86ValRegs);

	#define REC_CALL(In)				GekkoRecIL(INT_INSTRUCTION), 0, In, 0, 0

	#define TEMP_REG 32
	static GekkoRecILOpHeader(GETTEMPREG);
	#define REC_GETTEMPREG(RegMask)	GekkoRecIL(GETTEMPREG), 0, RegMask, 0, 0

	static GekkoRecILOpHeader(FREETEMPREG);
	#define REC_FREETEMPREG()		GekkoRecIL(FREETEMPREG), 0, 0, 0, 0

	static GekkoRecILOpHeader(GETTEMPFPUREG);
	#define REC_GETTEMPFPUREG(RegMask)	GekkoRecIL(GETTEMPFPUREG), 0, RegMask, 0, 0

	static GekkoRecILOpHeader(FREETEMPFPUREG);
	#define REC_FREETEMPFPUREG(RegMask)	GekkoRecIL(FREETEMPFPUREG), 0, RegMask, 0, 0

	static u32 NextGetRegID_Val;
	#define NextGetRegID (++NextGetRegID_Val)

	static GekkoRecILOpHeader(GETREG);
	#define REC_GETREG(In)			GekkoRecIL(GETREG), 0, In, 0, 0

	static GekkoRecILOpHeader(STOREREG);
	#define REC_STOREREG(In)		GekkoRecIL(STOREREG), 0, In, 0, 0

	static GekkoRecILOpHeader(FREEREG);
	#define REC_FREEREG(In)			GekkoRecIL(FREEREG), 0, In, 0, 0

	static GekkoRecILOpHeader(UPDATECR0);
	#define REC_UPDATECR0(SkipCount)	GekkoRecIL(UPDATECR0), SkipCount, 0, RecInstrFlag_NoPPCRegCount, GekkoRecCache(UPDATECR0)

	static GekkoRecILOpHeader(UPDATECRSO);
	#define REC_UPDATECRSO(Entry)	GekkoRecIL(UPDATECRSO), Entry, 0, RecInstrFlag_NoPPCRegCount | RecInstrFlag_TempData, 0
	static GekkoRecILOpHeader(UPDATECRSIGNED);
	#define REC_UPDATECRSIGNED(Entry)	GekkoRecIL(UPDATECRSIGNED), Entry, 0, RecInstrFlag_NoPPCRegCount | RecInstrFlag_TempData, 0
	static GekkoRecILOpHeader(UPDATECRUNSIGNED);
	#define REC_UPDATECRUNSIGNED(Entry)	GekkoRecIL(UPDATECRUNSIGNED), Entry, 0, RecInstrFlag_NoPPCRegCount | RecInstrFlag_TempData, 0

	static GekkoRecILOpHeader(CMPUPDATECR);
	#define REC_CMPUPDATECR(Reg, Reg2)	GekkoRecIL(CMPUPDATECR), Reg, Reg2, RecInstrFlagPPCOutReg | RecInstrFlagPPCInReg, 0
	#define REC_CMPUPDATECRI(Reg, Val)	GekkoRecIL(CMPUPDATECR), Reg, Val, RecInstrFlagPPCOutReg | RecInstrFlagPPCInVal, 0

	static GekkoRecILOpHeader(CMPUPDATECRSIGNED);
	#define REC_CMPUPDATECRSIGNED(Reg, Reg2)	GekkoRecIL(CMPUPDATECRSIGNED), Reg, Reg2, RecInstrFlagPPCOutReg | RecInstrFlagPPCInReg, 0
	#define REC_CMPUPDATECRSIGNEDI(Reg, Val)	GekkoRecIL(CMPUPDATECRSIGNED), Reg, Val, RecInstrFlagPPCOutReg | RecInstrFlagPPCInVal, 0

	static GekkoRecILOpHeader(UPDATEXERSOOV);
	#define REC_UPDATEXERSOOV(In)	GekkoRecIL(UPDATEXERSOOV), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATEXERSOOV)

	static GekkoRecILOpHeader(UPDATETEMPCARRY);
	static GekkoRecILOpHeader(UPDATECARRYWTEMP);
	static GekkoRecILOpHeader(UPDATECARRY);
	static GekkoRecILOpHeader(SETCARRY);
	static GekkoRecILOpHeader(CLEARCARRY);
	static GekkoRecILOpHeader(COMPLIMENTCARRY);
	#define REC_UPDATETEMPCARRY(In)		GekkoRecIL(UPDATETEMPCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATETEMPCARRY)
	#define REC_UPDATECARRYWTEMP(In)	GekkoRecIL(UPDATECARRYWTEMP), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATECARRYWTEMP)
	#define REC_UPDATECARRY(In)			GekkoRecIL(UPDATECARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(UPDATECARRY)
	#define REC_SETCARRY(In)			GekkoRecIL(SETCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(SETCARRY)
	#define REC_CLEARCARRY(In)			GekkoRecIL(CLEARCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(CLEARCARRY)
	#define REC_COMPLIMENTCARRY(In)		GekkoRecIL(COMPLIMENTCARRY), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInReg, GekkoRecCache(COMPLIMENTCARRY)

	static GekkoRecILOpHeader(PUSHA);
	static GekkoRecILOpHeader(POPA);
	static GekkoRecILOpHeader(RET);

	static GekkoRecILOpHeader(COMPRESS_CR);
	static GekkoRecILOpHeader(DECOMPRESS_CR);
	static GekkoRecILOpHeader(COMPRESS_XER);
	static GekkoRecILOpHeader(DECOMPRESS_XER);
	static GekkoRecILOpHeader(INT_INSTRUCTION);
	#define REC_INT_INSTRUCTION(In)		GekkoRecIL(INT_INSTRUCTION), 0, In, RecInstrFlag_NoPPCRegCount | RecInstrFlagPPCInVal | RecInstrFlag_IntInstruction, 0

	static GekkoRecILOpHeader(JUMPLABEL);
	static GekkoRecILOpHeader(JUMP);
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
	static GekkoRecILOpHeader(TURNOFFCACHING);
	#define REC_TURNOFFCACHING()	GekkoRecIL(TURNOFFCACHING), 0, 0, 0, 0

	static GekkoRecILOpHeader(TURNONCACHING);
	#define REC_TURNONCACHING()		GekkoRecIL(TURNONCACHING), 0, 0, 0, 0


	//fpu commands
	static GekkoRecILOpHeader(ADDPD);
	#define REC_ADDPD(Out, In)		GekkoRecIL(ADDPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(ADDSD);
	#define REC_ADDSD(Out, In)		GekkoRecIL(ADDSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(DIVPD);
	#define REC_DIVPD(Out, In)		GekkoRecIL(DIVPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(DIVSD);
	#define REC_DIVSD(Out, In)		GekkoRecIL(DIVSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(MULPD);
	#define REC_MULPD(Out, In)		GekkoRecIL(MULPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(MULSD);
	#define REC_MULSD(Out, In)		GekkoRecIL(MULSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(SUBPD);
	#define REC_SUBPD(Out, In)		GekkoRecIL(SUBPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(SUBSD);
	#define REC_SUBSD(Out, In)		GekkoRecIL(SUBSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(MOVSD);
	#define REC_MOVSD(Out, In)		GekkoRecIL(MOVSD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_MOVSDM(Out, In)		GekkoRecIL(MOVSD), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOpHeader(MOVSS);
	#define REC_MOVSS(Out, In)		GekkoRecIL(MOVSS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_MOVSSM(Out, In)		GekkoRecIL(MOVSS), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOpHeader(MOVAPD);
	#define REC_MOVAPD(Out, In)		GekkoRecIL(MOVAPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_MOVAPDM(Out, In)	GekkoRecIL(MOVAPD), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOpHeader(MOVLHPS);
	#define REC_MOVLHPS(Out, In)	GekkoRecIL(MOVLHPS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(MOVHLPS);
	#define REC_MOVHLPS(Out, In)	GekkoRecIL(MOVHLPS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(MOVDDUP);
	#define REC_MOVDDUP(Out, In)	GekkoRecIL(MOVDDUP), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOpHeader(MOVSLDUP);
	#define REC_MOVSLDUP(Out, In)	GekkoRecIL(MOVSLDUP), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOpHeader(MOVD);
	#define REC_MOVD_TOFPU(Out, In)		GekkoRecIL(MOVD), Out, In, RecInstrFlagPPCInReg | RecInstrFlagPPCOutFloatReg, 0
	#define REC_MOVD_FROMFPU(Out, In)	GekkoRecIL(MOVD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutReg, 0

	static GekkoRecILOpHeader(CVTPS2PD);
	#define REC_CVTPS2PD(Out, In)	GekkoRecIL(CVTPS2PD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(CVTDQ2PD);
	#define REC_CVTDQ2PD(Out, In)	GekkoRecIL(CVTDQ2PD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(CVTPD2PS);
	#define REC_CVTPD2PS(Out, In)	GekkoRecIL(CVTPD2PS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(CVTSD2SS);
	#define REC_CVTSD2SS(Out, In)	GekkoRecIL(CVTSD2SS), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(CVTSS2SD);
	#define REC_CVTSS2SD(Out, In)	GekkoRecIL(CVTSS2SD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(SHUFPD);
	#define REC_SHUFPD(Out, In)		GekkoRecIL(SHUFPD), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg, 0

	static GekkoRecILOpHeader(PAND);
	#define REC_PAND(Out, In)		GekkoRecIL(PAND), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_PANDM(Out, In)	GekkoRecIL(PAND), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOpHeader(POR);
	#define REC_POR(Out, In)		GekkoRecIL(POR), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_PORM(Out, In)	GekkoRecIL(POR), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static GekkoRecILOpHeader(PXOR);
	#define REC_PXOR(Out, In)		GekkoRecIL(PXOR), Out, In, RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0
	#define REC_PXORM(Out, In)	GekkoRecIL(PXOR), Out, In, RecInstrFlagPPCInMem | RecInstrFlagPPCOutFloatReg | RecInstrFlag_FPUMemoryAllowed, 0

	static RecInstruction *LastInstruction;
	static RecInstruction *InstructionBlock;

	GekkoF	execStep();

	static GekkoF EMU_FASTCALL CompileInstruction();
	static GekkoF EMU_FASTCALL DoBranchChecks();

	GekkoCPURecompiler();
	~GekkoCPURecompiler();
	static GekkoF EMU_FASTCALL CheckMemoryWriteInternal(u32 Addr, u32 Size);

	static GekkoF EMU_FASTCALL CompileIL(u32 OldPC, u32 NewPC);
	static GekkoF EMU_FASTCALL AssignRegsAndCompile(PPCRegInfo *X86Regs, PPCRegInfo *FPURegs, u8 *X86Buffer, u32 *BufferSize);

#define X86AssignRegsOp(name)		OPTYPE GekkoCPURecompiler::AssignX86Regs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define X86AssignRegsOpHeader(name)		OPTYPE AssignX86Regs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define X86AssignRegs(name)			GekkoCPURecompiler::AssignX86Regs_PPC##name
#define FPUAssignRegsOp(name)		OPTYPE GekkoCPURecompiler::AssignFPURegs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define FPUAssignRegsOpHeader(name)		OPTYPE AssignFPURegs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define FPUAssignRegs(name)			GekkoCPURecompiler::AssignFPURegs_PPC##name
#define X86FPUAssignRegsOp(name)	OPTYPE GekkoCPURecompiler::AssignX86FPURegs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define X86FPUAssignRegsOpHeader(name)	OPTYPE AssignX86FPURegs_PPC##name(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs)
#define X86FPUAssignRegs(name)		GekkoCPURecompiler::AssignX86FPURegs_PPC##name
typedef void (*RecX86FPUAssignRegsPtr)(RecInstruction *Instruction, PPCRegInfo *AssignedRegs, PPCRegInfo *FPURegs);

	static X86AssignRegsOpHeader(NONE);
	static X86AssignRegsOpHeader(InReg_Special);
	static X86AssignRegsOpHeader(InReg);
	static X86AssignRegsOpHeader(OutReg);
	static X86AssignRegsOpHeader(InVal);
	static X86AssignRegsOpHeader(InReg_OutReg);
	static X86AssignRegsOpHeader(InVal_OutReg);
	static X86AssignRegsOpHeader(InReg_OutReg_Special);
	static X86AssignRegsOpHeader(InVal_OutReg_Special);
	static X86AssignRegsOpHeader(KnownPPCValue);
	static X86AssignRegsOpHeader(InReg_OutMem);
	static X86AssignRegsOpHeader(InVal_OutMem);

	static FPUAssignRegsOpHeader(InMem_OutReg);
	static FPUAssignRegsOpHeader(InReg_OutReg);
	static FPUAssignRegsOpHeader(InReg_OutMem);
	static FPUAssignRegsOpHeader(InReg);

	static X86FPUAssignRegsOpHeader(InReg_OutFPUReg);
	static X86FPUAssignRegsOpHeader(InFPUReg_OutReg);
	static X86FPUAssignRegsOpHeader(InVal_OutFPUReg);

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
	static GekkoF EMU_FASTCALL Exception_REC(tGekkoException which);

	u32	GetTicksPerSecond();

	static GekkoF EMU_FASTCALL CreateRecInstruction(RecILInstructionPtr Instruction, u32 Out, u32 In, u32 Flags, RecCacheInstructionPtr CachePtr);
#define SetRecInstructionImm(ImmVal)	LastInstruction->X86CmdImmediateVal = ImmVal;
	static GekkoRecIntOpHeader(DUMP_OPS);
	static GekkoRecIntOpHeader(PS_ABS);
	static GekkoRecIntOpHeader(PS_ADD);
	static GekkoRecIntOpHeader(PS_CMPO0);
	static GekkoRecIntOpHeader(PS_CMPO1);
	static GekkoRecIntOpHeader(PS_CMPU0);
	static GekkoRecIntOpHeader(PS_CMPU1);
	static GekkoRecIntOpHeader(PS_MADD);
	static GekkoRecIntOpHeader(PS_MADDS0);
	static GekkoRecIntOpHeader(PS_MADDS1);
	static GekkoRecIntOpHeader(PS_MERGE00);
	static GekkoRecIntOpHeader(PS_MERGE01);
	static GekkoRecIntOpHeader(PS_MERGE10);
	static GekkoRecIntOpHeader(PS_MERGE11);
	static GekkoRecIntOpHeader(PS_MR);
	static GekkoRecIntOpHeader(PS_MSUB);
	static GekkoRecIntOpHeader(PS_MUL);
	static GekkoRecIntOpHeader(PS_MULS0);
	static GekkoRecIntOpHeader(PS_MULS1);
	static GekkoRecIntOpHeader(PS_NEG);
	static GekkoRecIntOpHeader(PS_NMADD);
	static GekkoRecIntOpHeader(PS_NMSUB);
	static GekkoRecIntOpHeader(PS_RES);
	static GekkoRecIntOpHeader(PS_RSQRTE);
	static GekkoRecIntOpHeader(PS_SUB);
	static GekkoRecIntOpHeader(PS_SUM0);
	static GekkoRecIntOpHeader(PS_SUM1);
	static GekkoRecIntOpHeader(PS_SEL);
	static GekkoRecIntOpHeader(NI);
	static GekkoRecIntOpHeader(ADD);
	static GekkoRecIntOpHeader(ADDO);
	static GekkoRecIntOpHeader(ADDC);
	static GekkoRecIntOpHeader(ADDCO);
	static GekkoRecIntOpHeader(ADDE);
	static GekkoRecIntOpHeader(ADDEO);
	static GekkoRecIntOpHeader(ADDI);
	static GekkoRecIntOpHeader(ADDIC);
	static GekkoRecIntOpHeader(ADDICD);
	static GekkoRecIntOpHeader(ADDIS);
	static GekkoRecIntOpHeader(ADDME);
	static GekkoRecIntOpHeader(ADDMEO);
	static GekkoRecIntOpHeader(ADDZE);
	static GekkoRecIntOpHeader(ADDZEO);
	static GekkoRecIntOpHeader(AND);
	static GekkoRecIntOpHeader(ANDC);
	static GekkoRecIntOpHeader(ANDID);
	static GekkoRecIntOpHeader(ANDISD);
	static GekkoRecIntOpHeader(B);
	static GekkoRecIntOpHeader(BL);
	static GekkoRecIntOpHeader(BA);
	static GekkoRecIntOpHeader(BLA);
	static GekkoRecIntOpHeader(BX);
	static GekkoRecIntOpHeader(BCX);
	static GekkoRecIntOpHeader(BCCTRX);
	static GekkoRecIntOpHeader(BCLRX);
	static GekkoRecIntOpHeader(CMP);
	static GekkoRecIntOpHeader(CMPI);
	static GekkoRecIntOpHeader(CMPL);
	static GekkoRecIntOpHeader(CMPLI);
	static GekkoRecIntOpHeader(CNTLZW);
	static GekkoRecIntOpHeader(CRAND);
	static GekkoRecIntOpHeader(CRANDC);
	static GekkoRecIntOpHeader(CREQV);
	static GekkoRecIntOpHeader(CRNAND);
	static GekkoRecIntOpHeader(CRNOR);
	static GekkoRecIntOpHeader(CROR);
	static GekkoRecIntOpHeader(CRORC);
	static GekkoRecIntOpHeader(CRXOR);
	static GekkoRecIntOpHeader(DCBA);
	static GekkoRecIntOpHeader(DCBF);
	static GekkoRecIntOpHeader(DCBI);
	static GekkoRecIntOpHeader(DCBST);
	static GekkoRecIntOpHeader(DCBT);
	static GekkoRecIntOpHeader(DCBTST);
	static GekkoRecIntOpHeader(DCBZ);
	static GekkoRecIntOpHeader(DCBZ_L);
	static GekkoRecIntOpHeader(DIVW);
	static GekkoRecIntOpHeader(DIVWO);
	static GekkoRecIntOpHeader(DIVWOD);
	static GekkoRecIntOpHeader(DIVWU);
	static GekkoRecIntOpHeader(DIVWUO);
	static GekkoRecIntOpHeader(ECIWX);
	static GekkoRecIntOpHeader(ECOWX);
	static GekkoRecIntOpHeader(EIEIO);
	static GekkoRecIntOpHeader(EQV);
	static GekkoRecIntOpHeader(EXTSB);
	static GekkoRecIntOpHeader(EXTSH);
	static GekkoRecIntOpHeader(FABS);
	static GekkoRecIntOpHeader(FADD);
	static GekkoRecIntOpHeader(FADDS);
	static GekkoRecIntOpHeader(FCMPO);
	static GekkoRecIntOpHeader(FCMPU);
	static GekkoRecIntOpHeader(FCTIW);
	static GekkoRecIntOpHeader(FCTIWZ);
	static GekkoRecIntOpHeader(FDIV);
	static GekkoRecIntOpHeader(FDIVS);
	static GekkoRecIntOpHeader(FMADD);
	static GekkoRecIntOpHeader(FMADDS);
	static GekkoRecIntOpHeader(FMR);
	static GekkoRecIntOpHeader(FMSUB);
	static GekkoRecIntOpHeader(FMSUBS);
	static GekkoRecIntOpHeader(FMUL);
	static GekkoRecIntOpHeader(FMULS);
	static GekkoRecIntOpHeader(FNABS);
	static GekkoRecIntOpHeader(FNEG);
	static GekkoRecIntOpHeader(FNMADD);
	static GekkoRecIntOpHeader(FNMADDS);
	static GekkoRecIntOpHeader(FNMSUB);
	static GekkoRecIntOpHeader(FNMSUBS);
	static GekkoRecIntOpHeader(FRES);
	static GekkoRecIntOpHeader(FRSP);
	static GekkoRecIntOpHeader(FRSQRTE);
	static GekkoRecIntOpHeader(FSEL);
	static GekkoRecIntOpHeader(FSQRT);
	static GekkoRecIntOpHeader(FSQRTS);
	static GekkoRecIntOpHeader(FSUB);
	static GekkoRecIntOpHeader(FSUBS);
	static GekkoRecIntOpHeader(ICBI);
	static GekkoRecIntOpHeader(ISYNC);
	static GekkoRecIntOpHeader(LBZ);
	static GekkoRecIntOpHeader(LBZU);
	static GekkoRecIntOpHeader(LBZUX);
	static GekkoRecIntOpHeader(LBZX);
	static GekkoRecIntOpHeader(LFD);
	static GekkoRecIntOpHeader(LFDU);
	static GekkoRecIntOpHeader(LFDUX);
	static GekkoRecIntOpHeader(LFDX);
	static GekkoRecIntOpHeader(LFS);
	static GekkoRecIntOpHeader(LFSU);
	static GekkoRecIntOpHeader(LFSUX);
	static GekkoRecIntOpHeader(LFSX);
	static GekkoRecIntOpHeader(LHA);
	static GekkoRecIntOpHeader(LHAU);
	static GekkoRecIntOpHeader(LHAUX);
	static GekkoRecIntOpHeader(LHAX);
	static GekkoRecIntOpHeader(LHBRX);
	static GekkoRecIntOpHeader(LHZ);
	static GekkoRecIntOpHeader(LHZU);
	static GekkoRecIntOpHeader(LHZUX);
	static GekkoRecIntOpHeader(LHZX);
	static GekkoRecIntOpHeader(LMW);
	static GekkoRecIntOpHeader(LSWI);
	static GekkoRecIntOpHeader(LSWX);
	static GekkoRecIntOpHeader(LWARX);
	static GekkoRecIntOpHeader(LWBRX);
	static GekkoRecIntOpHeader(LWZ);
	static GekkoRecIntOpHeader(LWZU);
	static GekkoRecIntOpHeader(LWZUX);
	static GekkoRecIntOpHeader(LWZX);
	static GekkoRecIntOpHeader(MCRF);
	static GekkoRecIntOpHeader(MCRFS);
	static GekkoRecIntOpHeader(MCRXR);
	static GekkoRecIntOpHeader(MFCR);
	static GekkoRecIntOpHeader(MFFS);
	static GekkoRecIntOpHeader(MFMSR);
	static GekkoRecIntOpHeader(MFSPR);
	static GekkoRecIntOpHeader(MFSR);
	static GekkoRecIntOpHeader(MFSRIN);
	static GekkoRecIntOpHeader(MFTB);
	static GekkoRecIntOpHeader(MTCRF);
	static GekkoRecIntOpHeader(MTFSB0);
	static GekkoRecIntOpHeader(MTFSB1);
	static GekkoRecIntOpHeader(MTFSF);
	static GekkoRecIntOpHeader(MTSFI);
	static GekkoRecIntOpHeader(MTMSR);
	static GekkoRecIntOpHeader(MTSPR);
	static GekkoRecIntOpHeader(MTSR);
	static GekkoRecIntOpHeader(MTSRIN);
	static GekkoRecIntOpHeader(MULHW);
	static GekkoRecIntOpHeader(MULHWD);
	static GekkoRecIntOpHeader(MULHWU);
	static GekkoRecIntOpHeader(MULHWUD);
	static GekkoRecIntOpHeader(MULLI);
	static GekkoRecIntOpHeader(MULLW);
	static GekkoRecIntOpHeader(MULLWD);
	static GekkoRecIntOpHeader(MULLWO);
	static GekkoRecIntOpHeader(MULLWOD);
	static GekkoRecIntOpHeader(NAND);
	static GekkoRecIntOpHeader(NEG);
	static GekkoRecIntOpHeader(NEGO);
	static GekkoRecIntOpHeader(NOR);
	static GekkoRecIntOpHeader(OR);
	static GekkoRecIntOpHeader(ORC);
	static GekkoRecIntOpHeader(ORI);
	static GekkoRecIntOpHeader(ORIS);
	static GekkoRecIntOpHeader(PS_DIV);
	static GekkoRecIntOpHeader(PSQ_L);
	static GekkoRecIntOpHeader(PSQ_LX);
	static GekkoRecIntOpHeader(PSQ_LU);
	static GekkoRecIntOpHeader(PSQ_LUX);
	static GekkoRecIntOpHeader(PSQ_ST);
	static GekkoRecIntOpHeader(PSQ_STX);
	static GekkoRecIntOpHeader(PSQ_STU);
	static GekkoRecIntOpHeader(PSQ_STUX);
	static GekkoRecIntOpHeader(RFI);
	static GekkoRecIntOpHeader(RLWIMI);
	static GekkoRecIntOpHeader(RLWINM);
	static GekkoRecIntOpHeader(RLWNM);
	static GekkoRecIntOpHeader(SC);
	static GekkoRecIntOpHeader(SLW);
	static GekkoRecIntOpHeader(SRAW);
	static GekkoRecIntOpHeader(SRAWI);
	static GekkoRecIntOpHeader(SRW);
	static GekkoRecIntOpHeader(STB);
	static GekkoRecIntOpHeader(STBU);
	static GekkoRecIntOpHeader(STBUX);
	static GekkoRecIntOpHeader(STBX);
	static GekkoRecIntOpHeader(STFD);
	static GekkoRecIntOpHeader(STFDU);
	static GekkoRecIntOpHeader(STFDUX);
	static GekkoRecIntOpHeader(STFDX);
	static GekkoRecIntOpHeader(STFIWX);
	static GekkoRecIntOpHeader(STFS);
	static GekkoRecIntOpHeader(STFSU);
	static GekkoRecIntOpHeader(STFSUX);
	static GekkoRecIntOpHeader(STFSX);
	static GekkoRecIntOpHeader(STH);
	static GekkoRecIntOpHeader(STHBRX);
	static GekkoRecIntOpHeader(STHU);
	static GekkoRecIntOpHeader(STHUX);
	static GekkoRecIntOpHeader(STHX);
	static GekkoRecIntOpHeader(STMW);
	static GekkoRecIntOpHeader(STSWI);
	static GekkoRecIntOpHeader(STSWX);
	static GekkoRecIntOpHeader(STW);
	static GekkoRecIntOpHeader(STWBRX);
	static GekkoRecIntOpHeader(STWCXD);
	static GekkoRecIntOpHeader(STWU);
	static GekkoRecIntOpHeader(STWUX);
	static GekkoRecIntOpHeader(STWX);
	static GekkoRecIntOpHeader(SUBF);
	static GekkoRecIntOpHeader(SUBFO);
	static GekkoRecIntOpHeader(SUBFC);
	static GekkoRecIntOpHeader(SUBFCO);
	static GekkoRecIntOpHeader(SUBFE);
	static GekkoRecIntOpHeader(SUBFEO);
	static GekkoRecIntOpHeader(SUBFIC);
	static GekkoRecIntOpHeader(SUBFME);
	static GekkoRecIntOpHeader(SUBFMEO);
	static GekkoRecIntOpHeader(SUBFZE);
	static GekkoRecIntOpHeader(SUBFZEO);
	static GekkoRecIntOpHeader(SYNC);
	static GekkoRecIntOpHeader(TLBIA);
	static GekkoRecIntOpHeader(TLBIE);
	static GekkoRecIntOpHeader(TLBSYNC);
	static GekkoRecIntOpHeader(TW);
	static GekkoRecIntOpHeader(TWI);
	static GekkoRecIntOpHeader(XOR);
	static GekkoRecIntOpHeader(XORI);
	static GekkoRecIntOpHeader(XORIS);
	static GekkoRecIntOpHeader(STWCX);
	static GekkoRecIntOpHeader(MTFSFI);
	static GekkoRecIntOpHeader(HLE);

	static GekkoRecOpHeader(ADD);
	static GekkoRecOpHeader(ADDO);
	static GekkoRecOpHeader(ADDC);
	static GekkoRecOpHeader(ADDCO);
	static GekkoRecOpHeader(ADDE);
	static GekkoRecOpHeader(ADDEO);
	static GekkoRecOpHeader(ADDI);
	static GekkoRecOpHeader(ADDIC);
	static GekkoRecOpHeader(ADDICD);
	static GekkoRecOpHeader(ADDIS);
	static GekkoRecOpHeader(ADDME);
	static GekkoRecOpHeader(ADDMEO);
	static GekkoRecOpHeader(ADDZE);
	static GekkoRecOpHeader(ADDZEO);
	static GekkoRecOpHeader(AND);
	static GekkoRecOpHeader(ANDC);
	static GekkoRecOpHeader(ANDID);
	static GekkoRecOpHeader(ANDISD);
	static GekkoRecOpHeader(CMP);
	static GekkoRecOpHeader(CMPI);
	static GekkoRecOpHeader(CMPL);
	static GekkoRecOpHeader(CMPLI);
	static GekkoRecOpHeader(CNTLZW);
	static GekkoRecOpHeader(CRAND);
	static GekkoRecOpHeader(CRANDC);
	static GekkoRecOpHeader(CREQV);
	static GekkoRecOpHeader(CRNAND);
	static GekkoRecOpHeader(CRNOR);
	static GekkoRecOpHeader(CROR);
	static GekkoRecOpHeader(CRORC);
	static GekkoRecOpHeader(CRXOR);
	static GekkoRecOpHeader(DIVW);
	static GekkoRecOpHeader(DIVWO);
	static GekkoRecOpHeader(DIVWU);
	static GekkoRecOpHeader(DIVWUO);
	static GekkoRecOpHeader(EQV);
	static GekkoRecOpHeader(EXTSB);
	static GekkoRecOpHeader(EXTSH);
	static GekkoRecOpHeader(MFCR);
	static GekkoRecOpHeader(MTCRF);
	static GekkoRecOpHeader(MULLI);
	static GekkoRecOpHeader(MULLW);
	static GekkoRecOpHeader(NAND);
	static GekkoRecOpHeader(NEG);
	static GekkoRecOpHeader(NEGO);
	static GekkoRecOpHeader(NOR);
	static GekkoRecOpHeader(OR);
	static GekkoRecOpHeader(ORD);
	static GekkoRecOpHeader(ORC);
	static GekkoRecOpHeader(ORI);
	static GekkoRecOpHeader(ORIS);
	static GekkoRecOpHeader(RLWIMI);
	static GekkoRecOpHeader(RLWINM);
	static GekkoRecOpHeader(RLWNM);
	static GekkoRecOpHeader(SLW);
	static GekkoRecOpHeader(SRAWI);
	static GekkoRecOpHeader(SRW);
	static GekkoRecOpHeader(SUBF);
	static GekkoRecOpHeader(SUBFO);
	static GekkoRecOpHeader(SUBFC);
	static GekkoRecOpHeader(SUBFCO);
	static GekkoRecOpHeader(SUBFE);
	static GekkoRecOpHeader(SUBFEO);
	static GekkoRecOpHeader(SUBFI);
	static GekkoRecOpHeader(SUBFIC);
	static GekkoRecOpHeader(SUBFME);
	static GekkoRecOpHeader(SUBFMEO);
	static GekkoRecOpHeader(SUBFZE);
	static GekkoRecOpHeader(SUBFZEO);
	static GekkoRecOpHeader(XOR);
	static GekkoRecOpHeader(XORD);
	static GekkoRecOpHeader(XORI);
	static GekkoRecOpHeader(XORIS);

	static GekkoRecOpHeader(INT_INSTRUCTION);
	static GekkoRecOpHeader(NOP);
	static GekkoRecOpHeader(LBZ);
	static GekkoRecOpHeader(LBZU);
	static GekkoRecOpHeader(LBZUX);
	static GekkoRecOpHeader(LBZX);
	static GekkoRecOpHeader(LHA);
	static GekkoRecOpHeader(LHAX);
	static GekkoRecOpHeader(LHZ);
	static GekkoRecOpHeader(LHZU);
	static GekkoRecOpHeader(LHZUX);
	static GekkoRecOpHeader(LHZX);
	static GekkoRecOpHeader(LMW);
	static GekkoRecOpHeader(LWZ);
	static GekkoRecOpHeader(LWZU);
	static GekkoRecOpHeader(LWZUX);
	static GekkoRecOpHeader(LWZX);
	static GekkoRecOpHeader(MFMSR);
	static GekkoRecOpHeader(MFSPR);
	static GekkoRecOpHeader(MFSR);
	static GekkoRecOpHeader(MFTB);
	static GekkoRecOpHeader(MTMSR);
	static GekkoRecOpHeader(MTSPR);
	static GekkoRecOpHeader(MTSR);
	static GekkoRecOpHeader(STB);
	static GekkoRecOpHeader(STBU);
	static GekkoRecOpHeader(STBUX);
	static GekkoRecOpHeader(STBX);
	static GekkoRecOpHeader(STH);
	static GekkoRecOpHeader(STHU);
	static GekkoRecOpHeader(STHUX);
	static GekkoRecOpHeader(STHX);
	static GekkoRecOpHeader(STMW);
	static GekkoRecOpHeader(STW);
	static GekkoRecOpHeader(STWU);
	static GekkoRecOpHeader(STWUX);
	static GekkoRecOpHeader(STWX);

	static GekkoRecOpHeader(BCLRX);
	static GekkoRecOpHeader(BCCTRX);
	static GekkoRecOpHeader(BCX);
	static GekkoRecOpHeader(BX);
	static GekkoRecOpHeader(DCBI);
	static GekkoRecOpHeader(RFI);

	static GekkoRecOpHeader(FABS);
	static GekkoRecOpHeader(FADD);
	static GekkoRecOpHeader(FADDS);
	static GekkoRecOpHeader(FDIV);
	static GekkoRecOpHeader(FDIVS);
	static GekkoRecOpHeader(FMADD);
	static GekkoRecOpHeader(FMADDS);
	static GekkoRecOpHeader(FMR);
	static GekkoRecOpHeader(FMSUB);
	static GekkoRecOpHeader(FMSUBS);
	static GekkoRecOpHeader(FMUL);
	static GekkoRecOpHeader(FMULS);
	static GekkoRecOpHeader(FNEG);
	static GekkoRecOpHeader(FRSP);
	static GekkoRecOpHeader(FSUB);
	static GekkoRecOpHeader(FSUBS);
	static GekkoRecOpHeader(PS_ABS);
	static GekkoRecOpHeader(PS_ADD);
	static GekkoRecOpHeader(PS_DIV);
	static GekkoRecOpHeader(PS_MADD);
	static GekkoRecOpHeader(PS_MADDS0);
	static GekkoRecOpHeader(PS_MADDS1);
	static GekkoRecOpHeader(PS_MERGE00);
	static GekkoRecOpHeader(PS_MERGE01);
	static GekkoRecOpHeader(PS_MR);
	static GekkoRecOpHeader(PS_MSUB);
	static GekkoRecOpHeader(PS_MUL);
	static GekkoRecOpHeader(PS_MULS0);
	static GekkoRecOpHeader(PS_NEG);
	static GekkoRecOpHeader(PS_SUB);
	static GekkoRecOpHeader(PS_SUM0);

	static GekkoRecOpHeader(LFD);
	static GekkoRecOpHeader(LFS);
	static GekkoRecOpHeader(LFSU);
	static GekkoRecOpHeader(LFSX);
	static GekkoRecOpHeader(STFD);
	static GekkoRecOpHeader(STFS);
	static GekkoRecOpHeader(STFSU);
	static GekkoRecOpHeader(STFSUX);
	static GekkoRecOpHeader(STFSX);
	static GekkoRecOpHeader(PSQ_L);
};

#endif

#endif