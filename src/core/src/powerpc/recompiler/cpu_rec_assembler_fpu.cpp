////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_assembler_fpu.cpp
// DESC:		Assembler of the IL opcodes for the recompiler
// CREATED:		May 22, 2009
////////////////////////////////////////////////////////////
// Copyright (c) 2009 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "cpu_rec_assembler.h"
#include "powerpc/cpu_core_regs.h"

FPUInstruction3Byte(ADDPD, 0x580F66);
FPUInstruction3Byte(ADDSD, 0x580FF2);
FPUInstruction3Byte(DIVPD, 0x5E0F66);
FPUInstruction3Byte(DIVSD, 0x5E0FF2);
FPUInstruction3Byte(MULPD, 0x590F66);
FPUInstruction3Byte(MULSD, 0x590FF2);
FPUInstruction3Byte(SUBPD, 0x5C0F66);
FPUInstruction3Byte(SUBSD, 0x5C0FF2);
FPUInstruction2ByteInverse(MOVLHPS, 0x160F);
FPUInstruction2ByteInverse(MOVHLPS, 0x120F);
FPUInstruction3ByteImm8(SHUFPD, 0xC60F66);
FPUInstruction2Byte(CVTPS2PD, 0x5A0F);
FPUInstruction3Byte(CVTPD2PS, 0x5A0F66);
FPUInstruction3Byte(CVTDQ2PD, 0xE60FF3);
FPUInstruction3Byte(CVTSD2SS, 0x5A0FF2);
FPUInstruction3Byte(CVTSS2SD, 0x5A0FF3);
FPUInstruction3Byte(MOVSLDUP, 0x120FF3);

FPUInstructionMem3Byte(MOVDDUP, 0x120FF2, 0x120FF2);
FPUInstructionMem3Byte(PAND, 0xDB0F66, 0xDB0F66);
FPUInstructionMem3Byte(POR, 0xEB0F66, 0xEB0F66);
FPUInstructionMem3Byte(PXOR, 0xEF0F66, 0xEF0F66);
FPUInstructionMove3Byte(MOVAPD, 0x280F66, 0x290F66);
FPUInstructionMem3Byte(MOVSD, 0x100FF2, 0x110FF2);
FPUInstructionMem3Byte(MOVSS, 0x100FF3, 0x110FF3);

GekkoRecILOp(GETTEMPFPUREG)
{
	//fake a stupid byte to cause microsoft to stop fucking with us as functions that do nothing are combined
	//so all of our code relying on unique functions fail
	*(u8 *)OutInstruction = 0;
	*OutSize = 0;
}

GekkoRecILOp(FREETEMPFPUREG)
{
	*(u8 *)OutInstruction = 1;
	*OutSize = 0;
}

GekkoRecILOp(MOVD)
{
	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutFloatReg):
			{
				*(u32 *)OutInstruction = (ModRM(0x00, Instruction->X86OutReg & ~REG_X86_CACHE,
										0x05) << 24) | 0x6E0F66;
				*(u32 *)(&((u8 *)OutInstruction)[4]) = Instruction->X86InMemory;
				*OutSize = 8;

				FPURegs[Instruction->X86OutReg & ~REG_X86_CACHE].ValueChanged = 1;
			}
			break;

		case (RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg):
			{
				*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86OutReg & ~REG_X86_CACHE,
										Instruction->X86InReg & ~REG_X86_CACHE) << 24) | 0x6E0F66;
				*OutSize = 4;

				FPURegs[Instruction->X86OutReg & ~REG_X86_CACHE].ValueChanged = 1;
			}
			break;

		case (RecInstrFlagX86InFloatReg | RecInstrFlagX86OutReg):
			{
				*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86InReg & ~REG_X86_CACHE,
										Instruction->X86OutReg & ~REG_X86_CACHE) << 24) | 0x7E0F66;
				*OutSize = 4;

				X86Regs[Instruction->X86OutReg & ~REG_X86_CACHE].ValueChanged = 1;
			}
			break;

		case (RecInstrFlagX86InFloatReg | RecInstrFlagX86OutMem):
			{
				*(u32 *)OutInstruction = (ModRM(0x03, Instruction->X86InReg & ~REG_X86_CACHE,
										0x05) << 24) | 0x7E0F66;
				*(u32 *)(&((u8 *)OutInstruction)[4]) = Instruction->X86OutMemory;
				*OutSize = 8;
			}
			break;

		default:
			Unknown_Mask("MOVD", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}