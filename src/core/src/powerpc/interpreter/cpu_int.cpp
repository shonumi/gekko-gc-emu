////////////////////////////////////////////////////////////
// TITLE:		ShizZy ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Interpreter 0.2
// FILE:		cpu_int.cpp
// DESC:		Core Gekko Interpreter
// CREATED:		May. 19, 2006
////////////////////////////////////////////////////////////
// Copyright (c) 2005 ShizZy/_aprentice_
////////////////////////////////////////////////////////////

#include "common.h"
#include "log.h"

#include "core.h"
#include "cpu_int.h"
#include "hw/hw.h"
#include "powerpc/cpu_core.h"
#include "powerpc/cpu_core_regs.h"
#include "powerpc/cpu_opsgroup.h"
#include "debugger/DebugInterface.h"
#include "debugger/GekkoDbgInterface.h"

u32			GekkoCPUInterpreter::RotMask[32][32];

u32			GekkoCPUInterpreter::LastOps[CPU_OPSTORE_COUNT];
u32			GekkoCPUInterpreter::LastOpsPC[CPU_OPSTORE_COUNT];
u32			GekkoCPUInterpreter::LastNewR[CPU_OPSTORE_COUNT * 32];
u32			GekkoCPUInterpreter::LastNewLR[CPU_OPSTORE_COUNT];
u32			GekkoCPUInterpreter::LastNewSRR0[CPU_OPSTORE_COUNT];
u32			GekkoCPUInterpreter::LastNewStack[CPU_OPSTORE_COUNT * 64];
u32			GekkoCPUInterpreter::LastOpEntry;

u32			GekkoCPUInterpreter::branch;
u32			GekkoCPUInterpreter::exception;

u32			GekkoCPUInterpreter::LastFinishedOp;

//#define		PRINT_INSTR_USAGE
#ifdef PRINT_INSTR_USAGE
u32			InstrID;
u32			IntCalls[0x10000];
#endif

////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////

// Desc: Initialization
//

GekkoCPU::CPUType GekkoCPUInterpreter::GetCPUType()
{
	return GekkoCPU::Interpreter;
}

GekkoCPUInterpreter::GekkoCPUInterpreter()
{
#ifdef PRINT_INSTR_USAGE
	memset(IntCalls, 0, sizeof(IntCalls));
#endif

	//load up the op tables
	u32 i;
	for(i=0; i < 0x400; i++)
		GekkoCPUOpsGroup4XO0Table[i] = GekkoInt(NI);

	//set all the other tables to point to GekkoInt(NI)
	memcpy(GekkoCPUOpsGroup4Table, GekkoCPUOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup4Table));
	memcpy(GekkoCPUOpsGroup19Table, GekkoCPUOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup19Table));
	memcpy(GekkoCPUOpsGroup31Table, GekkoCPUOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup31Table));
	memcpy(GekkoCPUOpsGroup59Table, GekkoCPUOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup59Table));
	memcpy(GekkoCPUOpsGroup63Table, GekkoCPUOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup63Table));
	memcpy(GekkoCPUOpsGroup63XO0Table, GekkoCPUOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup63XO0Table));
	memcpy(GekkoCPUOpset, GekkoCPUOpsGroup4XO0Table, sizeof(GekkoCPUOpset));

	//parse up the info
	for(i=0; GekkoIntOpsGroup4[i].Position != -1; i++)
		GekkoCPUOpsGroup4Table[GekkoIntOpsGroup4[i].Position] = GekkoIntOpsGroup4[i].OpPtr;

	for(i=0; GekkoIntOpsGroup4XO0[i].Position != -1; i++)
		GekkoCPUOpsGroup4XO0Table[GekkoIntOpsGroup4XO0[i].Position] = GekkoIntOpsGroup4XO0[i].OpPtr;

	for(i=0; GekkoIntOpsGroup19[i].Position != -1; i++)
		GekkoCPUOpsGroup19Table[GekkoIntOpsGroup19[i].Position] = GekkoIntOpsGroup19[i].OpPtr;

	for(i=0; GekkoIntOpsGroup31[i].Position != -1; i++)
		GekkoCPUOpsGroup31Table[GekkoIntOpsGroup31[i].Position] = GekkoIntOpsGroup31[i].OpPtr;

	for(i=0; GekkoIntOpsGroup59[i].Position != -1; i++)
		GekkoCPUOpsGroup59Table[GekkoIntOpsGroup59[i].Position] = GekkoIntOpsGroup59[i].OpPtr;

	for(i=0; GekkoIntOpsGroup63[i].Position != -1; i++)
		GekkoCPUOpsGroup63Table[GekkoIntOpsGroup63[i].Position] = GekkoIntOpsGroup63[i].OpPtr;

	for(i=0; GekkoIntOpsGroup63XO0[i].Position != -1; i++)
		GekkoCPUOpsGroup63XO0Table[GekkoIntOpsGroup63XO0[i].Position] = GekkoIntOpsGroup63XO0[i].OpPtr;

	for(i=0; GekkoIntOpsGroupMain[i].Position != -1; i++)
		GekkoCPUOpset[GekkoIntOpsGroupMain[i].Position] = GekkoIntOpsGroupMain[i].OpPtr;

	memset(&ireg, 0, sizeof(ireg));
	LOG_NOTICE(TPOWERPC, "interpreter initialized ok");
}

GekkoCPUInterpreter::~GekkoCPUInterpreter()
{
	u32	x;
	u32 y;
	char opcodeStr[32], operandStr[32];
	u32 target;
	u32 opcode;
#ifdef PRINT_INSTR_USAGE
	u32 list[101];
	u32 listentry;

	for(x=1; x < 100; x++)
		list[x] = 0;
	list[0] = 0xFFFFFFFF;

	for(listentry = 1; listentry < 100; listentry++)
	{
		for(x=0, y = 0; x < 0x10000; x++)
		{
			if(IntCalls[x] > IntCalls[y] && list[listentry-1] > IntCalls[x])
				y = x;
		}

		list[listentry] = IntCalls[y];

		opcode = ((y & 0x1FC00) >> 10 << 26) | ((y & 0x3FF) << 1);
		DisassembleGekko(opcodeStr, operandStr, opcode, 0, &target);
		printf("CPU Interpreter: Instruction %s Opcode %d (ext: %d, y %05X, opcode %08X) had %d hits\n",opcodeStr, opcode>>26,((opcode>>1)&0x3FF), y, opcode, IntCalls[y]);
	}

	for(;;)
	{
		if(GetAsyncKeyState(VK_RETURN)) break;
		Sleep(1000);
	}
#endif

	if(hGekkoThread)
	{
		Halt();
		hGekkoThread = NULL;
	}
}

GekkoF GekkoCPUInterpreter::Open(u32 entry_point)
{
	is_on = 0;
	loaded = 0;
	pause = 0;
	is_sc = 0;

	mode = 0;

	ireg.TBR.TBR = 0;
	opcode = 0;
	PClast = 0;

//	pPC = 0;
	u32 mb, me;

	branch = 0;
	exception = 0 ;

	int i;
	for(i = 0; i < 32; i++) ireg.gpr[ i ] = 0x0;
	for(i = 0; i < 32; i++) PS0(i) = 0x0;
	for(i = 0; i < 32; i++) PS1(i) = 0x0;
	for(i = 0; i < 1024; i++) ireg.spr[ i ] = 0x0;
	for(i = 0; i < 16; i++) ireg.sr[i] = 0x0;

	// Fill Rot Mask
    for(mb=0; mb<32; mb++)
    {
        for(me=0; me<32; me++)
        {
            u32 mask = ( (u32) - 1 >> mb) ^ ( ( me >= 31 ) ? 0 : ( (u32) - 1 ) >> ( me + 1 ) );
            RotMask[mb][me] = ( mb > me ) ? ~( mask ) : ( mask );
        }
    }
/*
    // Build paired-single load scale
    for(u8 scale=0; scale<64; scale++)
    {
        int factor;
        if(scale & 0x20)
        {
            factor = -32 + (scale & 0x1f);
        }else{
            factor = 0 + (scale & 0x1f);
        }
        ldScale[scale] = powf(2, -1.0f * (f32)factor);
    }

    // Build paired-single store scale
    for(u8 scale=0; scale<64; scale++)
    {
        int factor;
        if(scale & 0x20)
        {
            factor = -32 + (scale & 0x1f);
        }else{
            factor = 0 + (scale & 0x1f);
        }
        stScale[scale] = powf(2, +1.0f * (f32)factor);
    }
*/
	ireg.PC = entry_point;
	ireg.IC = 0x0;
	ireg.MSR = MSR_BIT_DR | MSR_BIT_IR;

	LR = GEX_RESET;
	SP = ARENA_HI;

	DEC = 0xFFFFFFFF;

//	pPC = (u32*)&RAM[ireg.PC & RAM_MASK];
	loaded = true;

	if(DumpOp0)
		GekkoCPUOpset[0] = GekkoInt(DUMP_OPS);

	LastFinishedOp = entry_point;

	if(PipeHandle)
	{
		//if we are debugging, backup the registers
		if(PipeIsClient)
		{
			_asm
			{
				lea eax, ireg
				lea ebx, iregBackup
				mov ecx, size ireg
			CopyMemory:
				movdqu xmm0, [eax]
				movdqu xmm1, [eax+16]
				movdqu [ebx], xmm0
				movdqu [ebx+16], xmm1
				add eax, 32
				add ebx, 32
				sub ecx, 32
				jnz CopyMemory
			};
		}
	}
}

////////////////////////////////////////////////////////////

// Desc: Run Main CPU Thread
//

unsigned __stdcall GekkoInterpreter_RunThread( void* pArguments )
{
	for( ; cpu->is_on ; )
	{
		if(!cpu->pause)
		{
			cpu->execStep();
		}
	}

	return 0;
}

// Desc: Start Main CPU Thread
//

GekkoF GekkoCPUInterpreter::Start(void)
{
	DWORD GekkoThread = 0;
	LOG_NOTICE(TPOWERPC, "starting PowerPC core!");

	if( false == is_on )
	{
		is_on = true;
		pause = false;
#ifndef SINGLETHREADED
		hGekkoThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)GekkoInterpreter_RunThread,NULL,NULL,&GekkoThread);
#endif
	} else {
		printf(".CPU: Gekko_Interpreter_Start - Gekko Core Already Started!\n");
	}
}

// Desc: Halt Main CPU Thread
//
GekkoF GekkoCPUInterpreter::Halt(void)
{
	printf("Halting CPU Thread!\n");

	is_on = false;
	pause = false;
    core::SetState(core::SYS_HALTED);

#ifndef SINGLETHREADED
	TerminateThread(hGekkoThread,0);
#endif
}

////////////////////////////////////////////////////////////

// Desc: Fetch Instruction Pointer
//

GekkoIntOp(Ops_Group4XO0)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov ecx, edx
		shr ecx, 1
		and ecx, 3FFh

#ifdef PRINT_INSTR_USAGE
		or InstrID, ecx
#endif
		call [ecx*4 + GekkoCPUOpsGroup4XO0Table]
	};
#else
	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup4XO0Table[XO0];

#ifdef PRINT_INSTR_USAGE
	InstrID |= XO0;
#endif

	iPtr();
#endif
}

GekkoIntOp(Ops_Group4)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov ecx, edx
		shr ecx, 1
		and ecx, 1Fh
		
#ifdef PRINT_INSTR_USAGE
		or InstrID, ecx
#endif

		call [ecx*4 + GekkoCPUOpsGroup4Table]
	};
#else
	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup4Table[XO3];

#ifdef PRINT_INSTR_USAGE
	InstrID |= XO3;
#endif

	iPtr();
#endif
} 

GekkoIntOp(Ops_Group19)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov ecx, edx
		shr ecx, 1
		and ecx, 3FFh
		
#ifdef PRINT_INSTR_USAGE
		or InstrID, ecx
#endif

		call [ecx*4 + GekkoCPUOpsGroup19Table]
	};
#else
	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup19Table[XO0];

#ifdef PRINT_INSTR_USAGE
	InstrID |= XO0;
#endif

	iPtr();
#endif
}

GekkoIntOp(Ops_Group31)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov ecx, edx
		shr ecx, 1
		and ecx, 3FFh
		
#ifdef PRINT_INSTR_USAGE
		or InstrID, ecx
#endif

		call [ecx*4 + GekkoCPUOpsGroup31Table]
	};
#else
	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup31Table[XO0];

#ifdef PRINT_INSTR_USAGE
	InstrID |= XO0;
#endif

	iPtr();
#endif
}

GekkoIntOp(Ops_Group59)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		//edx is the opcode
		mov ecx, edx
		shr ecx, 1
		and ecx, 1Fh
		
#ifdef PRINT_INSTR_USAGE
		or InstrID, ecx
#endif

		call [ecx*4 + GekkoCPUOpsGroup59Table]
	};
#else
	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup59Table[XO3];

#ifdef PRINT_INSTR_USAGE
	InstrID |= XO3;
#endif

	iPtr();
#endif
}

GekkoIntOp(Ops_Group63XO0)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov ecx, edx
		shr ecx, 1
		and ecx, 3FFh

#ifdef PRINT_INSTR_USAGE
		or InstrID, ecx
#endif

		call [ecx*4 + GekkoCPUOpsGroup63XO0Table]
	};
#else
	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup63XO0Table[XO0];

#ifdef PRINT_INSTR_USAGE
	InstrID |= XO0;
#endif

	iPtr();
#endif
}

GekkoIntOp(Ops_Group63)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov ecx, edx
		shr ecx, 1
		and ecx, 1Fh
		
#ifdef PRINT_INSTR_USAGE
		or InstrID, ecx
#endif

		call [ecx*4 + GekkoCPUOpsGroup63Table]
	};
#else
	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup63Table[XO3];

#ifdef PRINT_INSTR_USAGE
	InstrID |= XO3;
#endif

	iPtr();
#endif
}

// Desc: Handle a CPU Exception
//

GekkoF GekkoCPUInterpreter::Exception(tGekkoException which)
{
	SRR0 = ireg.PC;
	SRR1 = ireg.MSR & 0x87c7ffff;

	ireg.MSR &= (MSR_BIT_ILE | MSR_BIT_IP | MSR_BIT_ME);

	if(which == GEX_CHECK)
		ireg.MSR &= ~(MSR_BIT_ME | 0x80000000);
	else if(which == GEX_RESET)
	{
		SRR1 |= MSR_BIT_RI;
		ireg.MSR |= MSR_BIT_RI;
	}

	if(ireg.MSR & MSR_BIT_ILE)
		ireg.MSR |= MSR_BIT_LE;

	ireg.PC = which;
	if(ireg.MSR & MSR_BIT_IP)
		ireg.PC |= 0xFFF00000;

//	pPC = (u32*)NEW_PC_PTR();
	branch = true;
	exception = 1;
}

u32 GekkoCPUInterpreter::GetTicksPerSecond()
{
	return (GEKKO_CLOCK / 4) / 3;
}

// Desc: Gekko Execute Instruction
//

GekkoF GekkoCPUInterpreter::ExecuteInstruction(void)
{
	static int IntChanged=0;
	static int is_dec=0;
	u32		InstCount;
	u32		Ret;

	if(DumpOp0)
	{
		LastOpEntry = (LastOpEntry+1) % CPU_OPSTORE_COUNT;
		LastOps[LastOpEntry] = PTR_PC;
		LastOpsPC[LastOpEntry] = ireg.PC;
	}

	branch = 0;

//	if(ireg.PC == 0x8025214C)
//		_asm{int 3};

#if(0) //#ifdef USE_INLINE_ASM
	//eliminate extra memory hits

	_asm
	{
		mov InstCount, 0

ExecLoop:
		mov ecx, ireg.PC
		call Memory_Read32
		mov ecx, eax
//		bswap ecx
		mov edx, ecx
		mov opcode, ecx
		shr ecx, 1Ah

#ifdef PRINT_INSTR_USAGE
		mov ebx, ecx
		shl ebx, 10
		mov InstrID, ebx
#endif

		call [ecx*4 + GekkoCPUOpset]

#ifdef PRINT_INSTR_USAGE
		mov ebx, InstrID
		add dword [ebx*4 + IntCalls], 1
#endif

		add InstCount, 1
		cmp branch, 0
		jnz ExitExecLoop
		cmp step, 0
		jnz ExitExecLoop

//		add pPC, 4
		add ireg.PC, 4
		jmp ExecLoop

ExitExecLoop:
		cmp step, 0
		jz SkippPCUpdate
		cmp branch, 0
		jnz SkippPCUpdate

//		add pPC, 4
		add ireg.PC, 4

SkippPCUpdate:
		mov edx, [ireg.spr + (4*I_DEC)]
		mov eax, InstCount
		add ireg.TBR.TBL, eax
		adc ireg.TBR.TBU, 0
		add ireg.IC, eax
		sub edx, eax
		jnc DontExceptDec

		mov is_dec, MSR_BIT_EE

DontExceptDec:
		and branch, OPCODE_RFI
		jnz SkipFlipperUpdate

		push edx
		call Flipper_Update
		pop edx
		cmp exception, 0
		jne DontExceptDec2
		mov ebx, ireg.MSR
		and ebx, is_dec
		jz DontExceptDec2

		push edx
		push 0x80000900
		mov eax, this
		mov is_dec, 0
		call Exception

		pop edx

DontExceptDec2:
		mov exception, 0

SkipFlipperUpdate:
		mov [ireg.spr + (4*I_DEC)], edx
		mov branch, 0
	};

//	Flipper_Update();
#else
	InstCount = 0;

	for(;;)
	{
//		if(ireg.PC == 0x8025225C)
//			_asm{int 3};

#pragma todo("renenable breakpoints later");
	/*	if(bpt.size() && di.isBreakpoint(ireg.PC) && !step)
		{
			printf("Debug breakpoint hit: 0x%08X\n", ireg.PC);
			pause = true;
			break;
		}*/

		opcode = PTR_PC;
		GekkoFP iPtr = GekkoCPUOpset[OPCD];

#ifdef PRINT_INSTR_USAGE
		InstrID = OPCD << 10;
#endif
		iPtr();
		InstCount++;

#ifdef PRINT_INSTR_USAGE
		IntCalls[InstrID]++;
#endif
		if(branch || step)
			break;

		ireg.PC += 4;
	}

	if(step && !branch)
		ireg.PC += 4;

	ireg.TBR.TBR+=InstCount;

	if(DEC < InstCount)
	{
		is_dec = MSR_BIT_EE;
	}

	DEC -= InstCount;
	ireg.IC += InstCount;

	if(branch && !(branch & OPCODE_RFI))
	{
		Ret = Flipper_Update();

		if(!Ret && (ireg.MSR & is_dec))
		{
			is_dec = 0;
			cpu->Exception(GEX_DEC);
		}

		exception = 0;
	}

	branch = 0;

#endif

	if(DumpOp0)
	{
		LastNewLR[LastOpEntry] = LR;
		LastNewSRR0[LastOpEntry] = SRR0;
		memcpy(&LastNewR[LastOpEntry * 32], ireg.gpr, sizeof(ireg.gpr));
		memcpy(&LastNewStack[LastOpEntry * 64], &Mem_RAM[SP & RAM_MASK], 64 * sizeof(u32));
	}

	if(PipeHandle)
	{
		ComparePipeData(LastFinishedOp);
		LastFinishedOp = ireg.PC;

		//if we are debugging, backup the registers
		if(PipeIsClient)
		{
			_asm
			{
				lea eax, ireg
				lea ebx, iregBackup
				mov ecx, size ireg
			CopyMemory:
				movdqu xmm0, [eax]
				movdqu xmm1, [eax+16]
				movdqu [ebx], xmm0
				movdqu [ebx+16], xmm1
				add eax, 32
				add ebx, 32
				sub ecx, 32
				jnz CopyMemory
			};
		}
	}
}

////////////////////////////////////////////////////////////