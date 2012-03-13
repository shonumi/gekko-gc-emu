#include "common.h"
#include "cpu_rec.h"

//0x20 is the temp PPC register. I can either offset all x86 entries by 1, pain in the neck, or
//just offset them to the next available bit for the mask and adjust accordingly below
GekkoCPURecompiler::PPCKnownRegInfo	GekkoCPURecompiler::PPCRegVals[128];
u8 GekkoCPURecompiler::XERKnown[4];		//0 - temp carry, 1 - carry, 2 - overflow, 3 - summary
u8 GekkoCPURecompiler::CRKnown[4];		//0 - negative, 2 - positive, 1 - zero, 3 - summary
u8 GekkoCPURecompiler::SetXERKnown[4];
u8 GekkoCPURecompiler::SetCRKnown[4];

u32 GekkoCPURecompiler::CachingEnabled;

//mask to the in or out register for cache being other flags may be set
#define REG_CACHE_MASK	0x7F

u32 __cdecl GekkoCPURecompiler::CheckRecPPCCache(RecInstruction *Instruction)
{
	u32		OriginalFlags;

	//check the current instruction to see if the value is a known value that is being used
	//and adjust the instruction accordingly

	if(!CachingEnabled)
		Instruction->PPCCacheInst = 0;

	//first see if our input is known
	OriginalFlags = Instruction->Flags;
	if((Instruction->Flags & RecInstrFlagPPC_INMASK) == RecInstrFlagPPCInReg)
	{
		if(!(Instruction->InReg & REG_PPC_SPECIAL) && PPCRegVals[(Instruction->InReg & REG_CACHE_MASK)].Flags)
			Instruction->Flags = (Instruction->Flags & ~RecInstrFlagPPC_INMASK) |
								  RecInstrFlag_KnownInValue | RecInstrFlagPPCInVal;
	}

	//now see if out is a hard set value
	if(Instruction->PPCCacheInst && !(Instruction->OutReg & REG_PPC_SPECIAL))
	{
		if((Instruction->Flags & RecInstrFlagPPC_MASK) == (RecInstrFlagPPCOutReg | RecInstrFlagPPCInVal))
		{
			//if out is known, or a move, then process
			if(PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags || Instruction->Instruction == GekkoRecIL(MOVE))
			{
				Instruction->Flags |= RecInstrFlag_KnownValue;
				PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags = RecCache_Known;
			}
		}
		else if((Instruction->Flags & RecInstrFlagPPC_MASK) == RecInstrFlagPPCInVal)
		{
			//have just an in (neg, not, etc)
			Instruction->Flags |= RecInstrFlag_KnownValue;
			PPCRegVals[(Instruction->InReg & REG_CACHE_MASK)].Flags = RecCache_Known;
		}
		else if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutReg)
		{
			if(Instruction->Instruction == GekkoRecIL(ADDCARRY) && PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags &&
			   SetXERKnown[1])
				Instruction->Flags |= RecInstrFlag_KnownValue;
			else
				//if the out was known, set it so it will be cleared
				if(PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags && 
				   Instruction->Instruction != GekkoRecIL(MOVE))
				{
					//out was known and not overwriting with a move
					PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags = RecCache_WasKnown;
					Instruction->Flags |= RecInstrFlag_KnownOutValue;
				}
				else
					PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags = 0;
		}
	}
	else if((((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutReg) ||
			((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutMem)) &&
			!(Instruction->OutReg & REG_PPC_SPECIAL))
	{
		if(PPCRegVals[Instruction->OutReg & REG_CACHE_MASK].Flags)
		{
			//if a memory write and out value is known then return the known value
			if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutMem)
			{
				//turn off the out flags for PPC and set our out value flag
				Instruction->Flags = (Instruction->Flags & ~RecInstrFlagPPC_OUTMASK);
				Instruction->Flags |= RecInstrFlagX86OutVal;
				Instruction->X86OutVal = PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].KnownValue;
			}
			else
			{
				//if not a memory write then mark as was known as it will change
				if(!(Instruction->Flags & RecInstrFlag_MemoryWrite))
					PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags = RecCache_WasKnown;
				else if((Instruction->Flags & RecInstrFlagPPC_MASK) == (RecInstrFlagPPCOutReg | RecInstrFlagPPCInVal))
					Instruction->Flags |= RecInstrFlag_KnownValue;

				Instruction->Flags |= RecInstrFlag_KnownOutValue;
			}
		}
	}

	return (Instruction->Flags & RecInstrFlag_KnownValue);
}

u32 __cdecl GekkoCPURecompiler::ProcessRecPPCCache(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs)
{
	u32				InReg;
	RecInstruction	*NewInstruction;

	//insert actual known values into the instructions and process as needed

	//first see if our input is known
	if(Instruction->Flags & RecInstrFlag_KnownInValue)
	{
		InReg = Instruction->InReg & REG_CACHE_MASK;
		Instruction->InVal = PPCRegVals[InReg].KnownValue;
		Instruction->X86InVal = PPCRegVals[InReg].KnownValue;
		Instruction->Flags |= RecInstrFlagX86InVal;
	}

	//now see if out is a hard set value
	if(Instruction->PPCCacheInst)
	{
		if(Instruction->Flags & RecInstrFlag_KnownValue)
		{
			if((Instruction->Flags & RecInstrFlagPPC_MASK) == (RecInstrFlagPPCOutReg | RecInstrFlagPPCInVal))
			{
				//execute
				Instruction->Instruction = GekkoRecIL(NOP);
				Instruction->PPCCacheInst(Instruction);
				PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags = RecCache_Known;
			}
			else if((Instruction->Flags & RecInstrFlagPPC_MASK) == RecInstrFlagPPCInVal)
			{
				//have just an in (neg, not, etc), make out the proper register and call the function
				Instruction->OutReg = InReg;
				Instruction->Instruction = GekkoRecIL(NOP);
				Instruction->PPCCacheInst(Instruction);
				PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags = RecCache_Known;
			}
			else
			{
				//just run the command
				Instruction->Instruction = GekkoRecIL(NOP);
				Instruction->PPCCacheInst(Instruction);
			}
		}
		else if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutReg)
		{
			if(Instruction->OutReg & REG_PPC_SPECIAL)
			{
				ProcessRecPPCCacheSpecial(Instruction, OutInstruction, OutSize, X86Regs, FPURegs);
			}
			else
			{
				if(PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags && Instruction->Instruction != GekkoRecIL(MOVE))
				{
					//need to move our value into a register or memory
					NewInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
					if(Instruction->OutReg & REG_X86_CACHE)
					{
						NewInstruction->X86OutReg = Instruction->OutReg & REG_CACHE_MASK & ~REG_X86_CACHE;
						NewInstruction->Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutReg;
					}
					else
					{
						NewInstruction->X86OutReg = Instruction->X86OutReg;
						NewInstruction->Flags = (Instruction->Flags & RecInstrFlagX86_OUTMASK) |
												RecInstrFlagX86InVal;
					}

					NewInstruction->X86InVal = PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].KnownValue;
					NewInstruction->InReg = Instruction->InReg;
					NewInstruction->OutReg = Instruction->OutReg;
					InReg = 0;
					GekkoRecIL(MOVE)(NewInstruction, OutInstruction, &InReg, X86Regs, 0);
					(*OutSize) += InReg;
				}
				PPCRegVals[Instruction->OutReg].Flags = 0;
			}
		}
	}
	else if((Instruction->Flags & RecInstrFlagPPC_OUTMASK) == RecInstrFlagPPCOutReg)
	{
		if(Instruction->OutReg & REG_PPC_SPECIAL)
		{
			ProcessRecPPCCacheSpecial(Instruction, OutInstruction, OutSize, X86Regs, FPURegs);
		}
		else if(PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags && (Instruction->Flags & RecInstrFlag_MemoryWrite))
		{
			//memory write and out is known
			Instruction->X86OutVal = PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].KnownValue;
			Instruction->Flags = (Instruction->Flags & ~RecInstrFlagX86_OUTMASK) | RecInstrFlagX86OutVal;
		}
		else
		{
			if(PPCRegVals[Instruction->OutReg].Flags && Instruction->Instruction != GekkoRecIL(MOVE) && !(Instruction->Flags & RecInstrFlag_MemoryRead))
			{
				//need to move our value into a register or memory
				NewInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				if(Instruction->OutReg & REG_X86_CACHE)
				{
					NewInstruction->X86OutReg = Instruction->OutReg & REG_CACHE_MASK & ~REG_X86_CACHE;
					NewInstruction->Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutReg;
				}
				else
				{
					NewInstruction->X86OutReg = Instruction->X86OutReg;
					NewInstruction->Flags = (Instruction->Flags & RecInstrFlagX86_OUTMASK) |
											RecInstrFlagX86InVal;
				}

				NewInstruction->X86InVal = PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].KnownValue;
				NewInstruction->InReg = Instruction->InReg;
				NewInstruction->OutReg = Instruction->OutReg;
				InReg = 0;
				GekkoRecIL(MOVE)(NewInstruction, OutInstruction, &InReg, X86Regs,  0);
				(*OutSize) += InReg;

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}

			PPCRegVals[(Instruction->OutReg & REG_CACHE_MASK)].Flags = 0;
		}
	}
	return (Instruction->Instruction == GekkoRecIL(NOP));
}

u32 __cdecl GekkoCPURecompiler::ProcessRecPPCCacheSpecial(RecInstruction *Instruction, void *OutInstruction, u32 *OutSize, PPCRegInfo *X86Regs, PPCRegInfo *FPURegs)
{
	RecInstruction	*NewInstruction;
	u32				InReg;

	//have a special out value that we have cached
	switch(Instruction->OutReg & REG_PPC_SPECIALMASK)
	{
		case REG_PPC_CR:
			//need to move our value into a register or memory
			if(SetCRKnown[Instruction->OutReg & 0x1F] & 0x80)
			{
				NewInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
				NewInstruction->X86OutReg = Instruction->X86OutReg;
				NewInstruction->X86InVal = SetCRKnown[Instruction->OutReg & 0x1F] & 1;
				NewInstruction->InReg = Instruction->InReg;
				NewInstruction->OutReg = Instruction->OutReg;
				NewInstruction->Flags = (Instruction->Flags & RecInstrFlagX86_OUTMASK) | RecInstrFlagX86InVal;
				InReg = 0;
				GekkoRecIL(MOVE)(NewInstruction, OutInstruction, &InReg, X86Regs,0);
				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
				(*OutSize) += InReg;
				RecompileFree(NewInstruction);
			}
			break;

		case REG_PPC_SPR:
			if((Instruction->OutReg & 0x3FF) == I_XER)
			{
				LOG_ERROR(TPOWERPC, "ProcessRecPPCCacheSpecial: XER Encountered!\n");
				cpu->pause = true;
			}
			return 0;

		case REG_PPC_SR:
		case REG_PPC_PC:
		case REG_PPC_MSR:
		case REG_PPC_TBL:
		case REG_PPC_TBU:
		case REG_PPC_BRANCH:
			return 0;

		default:
			LOG_ERROR(TPOWERPC, "ProcessRecPPCCacheSpecial: Invalid register 0x%08X!\n", Instruction->OutReg);
			cpu->pause = true;
			return 0;
	};

	return (Instruction->Instruction == GekkoRecIL(NOP));
}

GekkoRecCacheOp(ADD)
{
#ifdef USE_INLINE_ASM_X86
    _asm
	{
		mov eax, Instruction
		mov ebx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ebx, [ebx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ecx, edx
		mov edx, [edx+eax*8]
		add edx, ebx
		setc XERKnown[1]
		seto XERKnown[2]
		cmp word ptr XERKnown[2], 0x0001
		setge XERKnown[3]
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ecx+eax*8], edx
	}
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(SUB)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ebx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ebx, [ebx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ecx, edx
		mov edx, [edx+eax*8]
		sub edx, ebx
		setc XERKnown[1]
		seto XERKnown[2]
		cmp word ptr XERKnown[2], 0x0001
		setge XERKnown[3]
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ecx+eax*8], edx
	}
	//PPCRegVals[Instruction->OutReg].KnownValue -= Instruction->InVal;
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(XOR)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ebx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ebx, [ebx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ecx, edx
		mov edx, [edx+eax*8]
		xor edx, ebx
		setc XERKnown[1]
		seto XERKnown[2]
		cmp word ptr XERKnown[2], 0x0001
		setge XERKnown[3]
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ecx+eax*8], edx
	}
//	PPCRegVals[Instruction->OutReg].KnownValue ^= Instruction->InVal;
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(OR)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ebx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ebx, [ebx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ecx, edx
		mov edx, [edx+eax*8]
		or edx, ebx
		setc XERKnown[1]
		seto XERKnown[2]
		cmp word ptr XERKnown[2], 0x0001
		setge XERKnown[3]
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ecx+eax*8], edx
	}
//	PPCRegVals[Instruction->OutReg].KnownValue |= Instruction->InVal;
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(AND)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ebx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ebx, [ebx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ecx, edx
		mov edx, [edx+eax*8]
		and edx, ebx
		setc XERKnown[1]
		seto XERKnown[2]
		cmp word ptr XERKnown[2], 0x0001
		setge XERKnown[3]
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ecx+eax*8], edx
	}
//	PPCRegVals[Instruction->OutReg].KnownValue &= Instruction->InVal;
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(MOVE)
{
	PPCRegVals[Instruction->OutReg].KnownValue = Instruction->InVal;
}

GekkoRecCacheOp(NOT)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ebx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ebx, [ebx + RecInstruction::InVal]
		lea edx, PPCRegVals
		not ebx
		setc XERKnown[1]
		seto XERKnown[2]
		cmp word ptr XERKnown[2], 0x0001
		setge XERKnown[3]
		cmp ebx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [edx+eax*8], ebx
	}
	//PPCRegVals[Instruction->OutReg].KnownValue = !Instruction->InVal;
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(NEG)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ebx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ebx, [ebx + RecInstruction::InVal]
		lea edx, PPCRegVals
		neg ebx
		setc XERKnown[1]
		seto XERKnown[2]
		cmp word ptr XERKnown[2], 0x0001
		setge XERKnown[3]
		cmp ebx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [edx+eax*8], ebx
	}
	//PPCRegVals[Instruction->OutReg].KnownValue = -Instruction->InVal;
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(UPDATECR0)
{
	*(u32 *)(SetCRKnown) = *(u32 *)CRKnown | 0x80808080;
}

GekkoRecCacheOp(UPDATEXERSOOV)
{
	*(u16 *)(&SetXERKnown[2]) = *(u16 *)(&XERKnown[2]) | 0x8080;
}

GekkoRecCacheOp(ADDCARRY)
{
	PPCRegVals[Instruction->OutReg].KnownValue += XERKnown[1];
}

GekkoRecCacheOp(UPDATECARRY)
{
	SetXERKnown[1] = XERKnown[1] | 0x80;
}

GekkoRecCacheOp(UPDATETEMPCARRY)
{
	SetXERKnown[0] = XERKnown[1] | 0x80;
}

GekkoRecCacheOp(UPDATECARRYWTEMP)
{
	SetXERKnown[1] |= XERKnown[1] | SetXERKnown[0] | 0x80;
	SetXERKnown[0] = 0;
}

GekkoRecCacheOp(SETCARRY)
{
	XERKnown[1] = 1;
}

GekkoRecCacheOp(CLEARCARRY)
{
	XERKnown[1] = 0;
}

GekkoRecCacheOp(COMPLIMENTCARRY)
{
	XERKnown[1] ^= 1;
}

GekkoRecCacheOp(ROL)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ecx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ecx, [ecx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ebx, edx
		mov edx, [edx+eax*8]
		rol edx, cl
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ebx+eax*8], edx
	}
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(ROR)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ecx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ecx, [ecx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ebx, edx
		mov edx, [edx+eax*8]
		ror edx, cl
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ebx+eax*8], edx
	}
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(SHL)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ecx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ecx, [ecx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ebx, edx
		cmp ecx, 31
		ja SHL_0Shift
		mov edx, [edx+eax*8]
		shl edx, cl
		jmp SHL_End

		SHL_0Shift:
			mov edx, 0

		SHL_End:
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ebx+eax*8], edx
	}
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(SHR)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ecx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ecx, [ecx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ebx, edx
		cmp ecx, 31
		ja SHR_0Shift
		mov edx, [edx+eax*8]
		shr edx, cl
		jmp SHR_End

		SHR_0Shift:
			mov edx, 0

		SHR_End:
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ebx+eax*8], edx
	}
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(SAL)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ecx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ecx, [ecx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ebx, edx
		cmp ecx, 31
		ja SAL_0Shift
		mov edx, [edx+eax*8]
		sal edx, cl
		jmp SAL_End

		SAL_0Shift:
			mov edx, 0

		SAL_End:
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ebx+eax*8], edx
	}
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(SAR)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ecx, eax
		mov eax, [eax + RecInstruction::OutReg]
		mov ecx, [ecx + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ebx, edx
		cmp ecx, 31
		ja SAR_0Shift
		mov edx, [edx+eax*8]
		sar edx, cl
		jmp SAR_End

		SAR_0Shift:
			mov edx, 0

		SAR_End:
		cmp edx, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ebx+eax*8], edx
	}
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoRecCacheOp(SMUL)
{
#ifdef USE_INLINE_ASM_X86
	_asm
	{
		mov eax, Instruction
		mov ecx, eax
		mov ecx, [ecx + RecInstruction::OutReg]
		mov eax, [eax + RecInstruction::InVal]
		lea edx, PPCRegVals
		mov ebx, edx
		mov edx, [edx+ecx*8]
		imul edx
		cmp eax, 0
		setl CRKnown[0]
		setg CRKnown[1]
		setz CRKnown[2]
		mov [ebx+ecx*8], eax
    }	
#else
#pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}