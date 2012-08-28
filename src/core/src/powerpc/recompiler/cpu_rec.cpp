////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec.cpp
// DESC:		Core Recompiler
// CREATED:		Feb. 26, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

//don't forget REC_PRINT_INT_CALLS in cpu_rec_opcodes_branch.cpp
//#define REC_PRINT_INT_CALLS

#define _WIN32_WINNT 0x0400
#include "common.h"
#include "core.h"
#include "hw/hw.h"
#include "cpu_rec.h"
#include "powerpc/cpu_core_regs.h"

u32			GekkoCPURecompiler::RotMask[32][32];
u32			GekkoCPURecompiler::branch;
u32			GekkoCPURecompiler::IsInterpret;
//u32			GekkoCPURecompiler::exception;
u32			GekkoCPURecompiler::is_dec;
u32			GekkoCPURecompiler::HLE_PC;

#include "powerpc/cpu_opsgroup.h"
#include "cpu_rec_opsgroup.h"
#include "powerpc/disassembler/ppc_disasm.h"

GekkoCPURecompiler::RecInstruction *GekkoCPURecompiler::LastInstruction;
GekkoCPURecompiler::RecInstruction *GekkoCPURecompiler::InstructionBlock;

u32		GekkoCPURecompiler::ESPSave;
u32		GekkoCPURecompiler::OldPC;
u32		GekkoCPURecompiler::TotalInstCount;
u32 *	GekkoCPURecompiler::CompiledTable;
u32 *	GekkoCPURecompiler::CompiledTablePages;
u32		GekkoCPURecompiler::IntCalls[0x10000];
u32		GekkoCPURecompiler::PageSize;

optable		GekkoCPURecompiler::GekkoCPURecOpsGroup4XO0Table[0x400];
optable		GekkoCPURecompiler::GekkoCPURecOpsGroup4Table[0x30];
optable		GekkoCPURecompiler::GekkoCPURecOpsGroup19Table[0x400];
optable		GekkoCPURecompiler::GekkoCPURecOpsGroup31Table[0x400];
optable		GekkoCPURecompiler::GekkoCPURecOpsGroup59Table[0x400];
optable		GekkoCPURecompiler::GekkoCPURecOpsGroup63XO0Table[0x400];
optable		GekkoCPURecompiler::GekkoCPURecOpsGroup63Table[0x20];
optable		GekkoCPURecompiler::GekkoCPURecOpset[0x40];

//DO NOT MESS WITH THESE VALUES Look at the comment at the end of GekkoCPURecompiler::GekkoCPURecompiler()
/*
	Assembly:
		MOV EAX, 0x9A
		CMP DWORD PTR [ESP+8], 7
		JNZ ZeroEAX
		XOR EAX, EAX
	ZeroEAX:
		RETN 14
*/
BYTE PatchBytes[] = {0xB8, 0x9A, 0x00, 0x00, 0x00, 0x83, 0x7C, 0x24, 0x08, 0x07, 0x0F, 0x85, 0x00, 0x00, 0x00, 0x00, 0x33, 0xC0, 0xC2, 0x14, 0x00};
BYTE OriginalBytes[5] = {0};
BYTE PatchJump[5] = {0xE9, 0x00, 0x00, 0x00, 0x00};

bool EMU_FASTCALL WriteMemory( BYTE* pTarget, const BYTE* pSource, DWORD Size )
{
	DWORD ErrCode = 0;


	// Check parameters 

	if( pTarget == 0 )
	{
		printf(" - FAILED!\n");
		printf("WriteMemory: Target address is null!\n");
		return false;
	}

	if( pSource == 0 )
	{
		printf(" - FAILED!\n");
		printf("WriteMemory: Source address is null!\n");
		return false;
	}

	if( Size == 0 )
	{
		printf(" - FAILED!\n");
		printf("WriteMemory: Source size is null!\n");
		return false;
	}

	if( IsBadReadPtr( pSource, Size ) )
	{
		printf(" - FAILED!\n");
		printf("WriteMemory: Source is unreadable!\n");
		return false;
	}


	// Modify protection attributes of the target memory page 

	DWORD OldProtect = 0;

	if( !VirtualProtect( pTarget, Size, PAGE_EXECUTE_READWRITE, &OldProtect ) )
	{
		ErrCode = GetLastError();
		printf(" - FAILED!\n");
		printf("WriteMemory: VirtualProtect() failed! Error 0x%08X\n", ErrCode);
		return false;
	}


	// Write memory 

	memcpy( pTarget, pSource, Size );


	// Restore memory protection attributes of the target memory page 

	DWORD Temp = 0;

	if( !VirtualProtect( pTarget, Size, OldProtect, &Temp ) )
	{
		ErrCode = GetLastError();
		printf(" - FAILED!\n");
		printf("WriteMemory: VirtualProtect() failed! Error 0x%08X\n", ErrCode);
		return false;
	}


	// Success 

	return true;

}

GekkoCPU::CPUType GekkoCPURecompiler::GetCPUType()
{
	return GekkoCPU::DynaRec;
}

GekkoCPURecompiler::GekkoCPURecompiler()
{
	u32 mb, me;
	SYSTEM_INFO	SysInfo;
/*
	HANDLE f = CreateFile("e:\\gekko\\source\\memdump.txt", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	CloseHandle(f);
*/
	InitRecompileMemory();
	CompiledTable = (u32 *)RecompileAlloc(32 * 1024 * 1024);
	memset(CompiledTable, 0, 32 * 1024 * 1024);

	memset(IntCalls, 0, sizeof(IntCalls));

	u32 i;
	for(i=0; i < 0x400; i++)
		GekkoCPURecOpsGroup4XO0Table[i] = GekkoRecInt(NI);

	//set all the other tables to point to GekkoRecInt(NI)
	memcpy(GekkoCPURecOpsGroup4Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPURecOpsGroup4Table));
	memcpy(GekkoCPURecOpsGroup19Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPURecOpsGroup19Table));
	memcpy(GekkoCPURecOpsGroup31Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPURecOpsGroup31Table));
	memcpy(GekkoCPURecOpsGroup59Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPURecOpsGroup59Table));
	memcpy(GekkoCPURecOpsGroup63Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPURecOpsGroup63Table));
	memcpy(GekkoCPURecOpsGroup63XO0Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPURecOpsGroup63XO0Table));
	memcpy(GekkoCPURecOpset, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPURecOpset));
	memcpy(GekkoCPUOpsGroup4XO0Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup4XO0Table));
	memcpy(GekkoCPUOpsGroup4Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup4Table));
	memcpy(GekkoCPUOpsGroup19Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup19Table));
	memcpy(GekkoCPUOpsGroup31Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup31Table));
	memcpy(GekkoCPUOpsGroup59Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup59Table));
	memcpy(GekkoCPUOpsGroup63Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup63Table));
	memcpy(GekkoCPUOpsGroup63XO0Table, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpsGroup63XO0Table));
	memcpy(GekkoCPUOpset, GekkoCPURecOpsGroup4XO0Table, sizeof(GekkoCPUOpset));

	//parse up the info
	for(i=0; GekkoRecGroupOps4[i].Position != -1; i++)
	{
		GekkoCPURecOpsGroup4Table[GekkoRecGroupOps4[i].Position] = GekkoRecGroupOps4[i].OpPtr;
		GekkoCPUOpsGroup4Table[GekkoRecGroupOps4[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	for(i=0; GekkoRecGroupOps4XO0[i].Position != -1; i++)
	{
		GekkoCPURecOpsGroup4XO0Table[GekkoRecGroupOps4XO0[i].Position] = GekkoRecGroupOps4XO0[i].OpPtr;
		GekkoCPUOpsGroup4XO0Table[GekkoRecGroupOps4XO0[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	for(i=0; GekkoRecGroupOps19[i].Position != -1; i++)
	{
		GekkoCPURecOpsGroup19Table[GekkoRecGroupOps19[i].Position] = GekkoRecGroupOps19[i].OpPtr;
		GekkoCPUOpsGroup19Table[GekkoRecGroupOps19[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	for(i=0; GekkoRecGroupOps31[i].Position != -1; i++)
	{
		GekkoCPURecOpsGroup31Table[GekkoRecGroupOps31[i].Position] = GekkoRecGroupOps31[i].OpPtr;
		GekkoCPUOpsGroup31Table[GekkoRecGroupOps31[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	for(i=0; GekkoRecGroupOps59[i].Position != -1; i++)
	{
		GekkoCPURecOpsGroup59Table[GekkoRecGroupOps59[i].Position] = GekkoRecGroupOps59[i].OpPtr;
		GekkoCPUOpsGroup59Table[GekkoRecGroupOps59[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	for(i=0; GekkoRecGroupOps63[i].Position != -1; i++)
	{
		GekkoCPURecOpsGroup63Table[GekkoRecGroupOps63[i].Position] = GekkoRecGroupOps63[i].OpPtr;
		GekkoCPUOpsGroup63Table[GekkoRecGroupOps63[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	for(i=0; GekkoRecGroupOps63XO0[i].Position != -1; i++)
	{
		GekkoCPURecOpsGroup63XO0Table[GekkoRecGroupOps63XO0[i].Position] = GekkoRecGroupOps63XO0[i].OpPtr;
		GekkoCPUOpsGroup63XO0Table[GekkoRecGroupOps63XO0[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	for(i=0; i < 0x40; i++)
		GekkoCPUOpset[i] = GekkoRec(INT_INSTRUCTION);

	for(i=0; GekkoRecGroupOpsMain[i].Position != -1; i++)
	{
		GekkoCPURecOpset[GekkoRecGroupOpsMain[i].Position] = GekkoRecGroupOpsMain[i].OpPtr;
		GekkoCPUOpset[GekkoRecGroupOpsMain[i].Position] = GekkoRec(INT_INSTRUCTION);
	}

	//do the group functions
	for(i=0; GekkoRecCompOpsGroups[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsGroups[i].CPUInstrTable[GekkoRecCompOpsGroups[i].RecOpcode.Position] = GekkoRecCompOpsGroups[i].RecOpcode.OpPtr;

	//do the math functions
	for(i=0; GekkoRecCompOpsMath[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsMath[i].CPUInstrTable[GekkoRecCompOpsMath[i].RecOpcode.Position] = GekkoRecCompOpsMath[i].RecOpcode.OpPtr;

	//do the load/store functions
	for(i=0; GekkoRecCompOpsLoadStore[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsLoadStore[i].CPUInstrTable[GekkoRecCompOpsLoadStore[i].RecOpcode.Position] = GekkoRecCompOpsLoadStore[i].RecOpcode.OpPtr;

	//do the branch functions
	for(i=0; GekkoRecCompOpsBranch[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsBranch[i].CPUInstrTable[GekkoRecCompOpsBranch[i].RecOpcode.Position] = GekkoRecCompOpsBranch[i].RecOpcode.OpPtr;

	//do the compare functions
	for(i=0; GekkoRecCompOpsCompare[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsCompare[i].CPUInstrTable[GekkoRecCompOpsCompare[i].RecOpcode.Position] = GekkoRecCompOpsCompare[i].RecOpcode.OpPtr;

	//do the nop functions
	for(i=0; GekkoRecCompOpsNOP[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsNOP[i].CPUInstrTable[GekkoRecCompOpsNOP[i].RecOpcode.Position] = GekkoRecCompOpsNOP[i].RecOpcode.OpPtr;

	//do the FPU load/store functions
	for(i=0; GekkoRecCompOpsFPULoadStore[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsFPULoadStore[i].CPUInstrTable[GekkoRecCompOpsFPULoadStore[i].RecOpcode.Position] = GekkoRecCompOpsFPULoadStore[i].RecOpcode.OpPtr;

	//do the FPU math functions
	for(i=0; GekkoRecCompOpsFPUMath[i].CPUInstrTable != 0; i++)
		GekkoRecCompOpsFPUMath[i].CPUInstrTable[GekkoRecCompOpsFPUMath[i].RecOpcode.Position] = GekkoRecCompOpsFPUMath[i].RecOpcode.OpPtr;

	// Fill Rot Mask
    for(mb=0; mb<32; mb++)
    {
        for(me=0; me<32; me++)
        {
            u32 mask = ( (u32) - 1 >> mb) ^ ( ( me >= 31 ) ? 0 : ( (u32) - 1 ) >> ( me + 1 ) );
            RotMask[mb][me] = ( mb > me ) ? ~( mask ) : ( mask );
        }
    }

	FPR_SignBit.ps0._u64 = 0x8000000000000000;
	FPR_SignBit.ps1._u64 = 0x8000000000000000;
	FPR_SignBitLow.ps0._u64 = 0x8000000000000000;
	FPR_SignBitLow.ps1._u64 = 0;
	FPR_SignBitInverse.ps0._u64 = ~FPR_SignBit.ps0._u64;
	FPR_SignBitInverse.ps1._u64 = ~FPR_SignBit.ps1._u64;
	FPR_SignBitInverseLow.ps0._u64 = ~FPR_SignBitLow.ps0._u64;
	FPR_SignBitInverseLow.ps1._u64 = ~FPR_SignBitLow.ps1._u64;
	FPR_Negative1.ps0._u64 = 0xBFF0000000000000;
	FPR_Negative1.ps1._u64 = 0xBFF0000000000000;
	FPR_Negative1Low.ps0._u64 = 0xBFF0000000000000;
	FPR_Negative1Low.ps1._u64 = 0;

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

	memset(&CompileBlockNeeded, 0, sizeof(CompileBlockNeeded));
	CompileBlockNeeded.CodeBlock = (GekkoFP)&GekkoCPURecompiler::CompileInstruction;

	//setup to compile instructions
	for(i = 0; i < 8*1024*1024; i++)
		CompiledTable[i] = (u32)&CompileBlockNeeded;

	memset(&ireg, 0, sizeof(ireg));

	//setup the unhandled exception for memory writes
	GetSystemInfo(&SysInfo);
	PageSize = SysInfo.dwPageSize;
	void* var = SetUnhandledExceptionFilter(GekkoCPURecompiler::UnhandledException);

	//setup our page table entry
	CompiledTablePages = (u32 *)RecompileAlloc(((32 * 1024 * 1024) / PageSize) * 4);
	memset(CompiledTablePages, 0, ((32 * 1024 * 1024) / PageSize) * 4);

	//see if we are being debugged. Do not mess with this code unless you
	//truely know what you are doing. I am overwriting memory in windows
	//to gain control of what NtQueryInformationProcess returns when asked
	//about ProcessDebugPort. UnhandledExceptionFilter calls
	//NtQueryInformationProcess to determine if the debugger should be given
	//an unhandled exception or passed along to the program. We need Gekko
	//to handle the errors, not the debugger due to using it for memory write
	//validation
	if(0) //IsDebuggerPresent())
	{
		//patch NtQueryInformationProcess to allow unhandled exceptions to  be
		//passed along and not passed back to the debugger
		HMODULE hDLL = GetModuleHandle("ntdll.dll");
		BYTE *pTarget = (BYTE *)GetProcAddress(hDLL, "NtQueryInformationProcess");
		BYTE *PatchMem = (BYTE *)RecompileAlloc(sizeof(PatchBytes));
		memcpy(PatchMem, PatchBytes, sizeof(PatchBytes));
		memcpy( OriginalBytes, pTarget, sizeof(OriginalBytes) );

		//verify the first instruction is mov eax, 0x9A
		if(*(u32 *)&OriginalBytes[0] & 0xFFFF00FF != 0x000000B8)
		{
			printf("ERROR! Debugger detected but unable to hook NtQueryInformationProcess!\n");
			printf("Found the following bytes: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
					OriginalBytes[0], OriginalBytes[1], OriginalBytes[2], OriginalBytes[3],
					OriginalBytes[4]);
		}
		else
		{
			printf("Debugger Detected, patching NtQueryInformationProcess");
			*(u32*)&PatchJump[1] = (u32)PatchMem - (u32)pTarget - 5;
			*(u32*)&PatchBytes[12] = (u32)pTarget - (u32)&PatchBytes[12] + 1;
			PatchBytes[1] = OriginalBytes[1];
			if(WriteMemory(pTarget, PatchJump, sizeof(PatchJump)))
				printf(" - OK\n");
		}
	}
	LOG_NOTICE(TPOWERPC, "dynarec initialized ok");
}

LONG __stdcall GekkoCPURecompiler::UnhandledException(EXCEPTION_POINTERS *ExceptionInfo)
{
	DWORD	OldFlags;
	u32		MemAddr;

	if(ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		//verify the exception is inside our area, otherwise we do not want to handle it
		MemAddr = ExceptionInfo->ExceptionRecord->ExceptionInformation[1] & ~(PageSize - 1);
		if((MemAddr >= (u32)(&Mem_RAM[0])) && (MemAddr < (u32)(&Mem_RAM[RAM_SIZE])))
		{
			//memory write issue, unlock the page and erase the related data
			VirtualProtect((void *)(MemAddr), PageSize, PAGE_EXECUTE_READWRITE, &OldFlags);
			CheckMemoryWriteInternal(MemAddr, PageSize);
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else
			return EXCEPTION_CONTINUE_EXECUTION;
	}
	else if(ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO)
	{
		//verify the exception is inside our area, otherwise we do not want to handle it
		MemAddr = ExceptionInfo->ExceptionRecord->ExceptionInformation[1] & ~(PageSize - 1);
		if((MemAddr >= (u32)(&Mem_RAM[0])) && (MemAddr < (u32)(&Mem_RAM[RAM_SIZE])))
		{
			//continue execution, div is probably being used incorrectly and result is undefined
			//figure out if the div command is 2 or 6 bytes then skip the command itself
#ifdef EMU_ARCHITECTURE_X86
			OldFlags = ExceptionInfo->ContextRecord->Eip;
			MemAddr = *(u32 *)OldFlags;
			if((MemAddr & 0x0000C000) == 0x00)
				ExceptionInfo->ContextRecord->Eip += 6;
			else
				ExceptionInfo->ContextRecord->Eip += 2;
#endif
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else
			return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

GekkoCPURecompiler::~GekkoCPURecompiler()
{
	u32	x;
	u32 y;
	char opcodeStr[32], operandStr[32];
	u32 target;
	u32 opcode;
	u32 list[101];
	u32 listentry;

	//undo our NtQueryInformationProcess bypass
	if(IsDebuggerPresent())
	{
		HMODULE hDLL = GetModuleHandle("ntdll.dll");
		BYTE *pTarget = (BYTE *)GetProcAddress(hDLL, "NtQueryInformationProcess");
		WriteMemory(pTarget, OriginalBytes, sizeof(OriginalBytes));
	}

	//remove the unhandled exception filter
	SetUnhandledExceptionFilter(NULL);

	if(hGekkoThread)
	{
		Halt();
		hGekkoThread = NULL;
	}

	ReleaseRecompileMemory();

#ifdef REC_PRINT_INT_CALLS
	for(x=1; x < 201; x++)
		list[x] = 0;
	list[0] = 0xFFFFFFFF;

	for(listentry = 1; listentry < 200; listentry++)
	{
		for(x=0, y = 0; x < 0x10000; x++)
		{
			if(IntCalls[x] > IntCalls[y] && list[listentry-1] > IntCalls[x])
				y = x;
		}

		//exit when we don't have any more hits
		if(IntCalls[y] == 0)
			break;

		list[listentry] = IntCalls[y];

		opcode = ((y & 0x1FC00) >> 10 << 26) | ((y & 0x3FF) << 1);
		DisassembleGekko(opcodeStr, operandStr, opcode, 0, &target);
		printf("CPU DynaRec Interpreter: Instruction %s Opcode %d (ext: %d, y %05X, opcode %08X) had %d hits\n",opcodeStr, opcode>>26,((opcode>>1)&0x3FF), y, opcode, IntCalls[y]);
	}

	for(;;)
		SDL_Delay(1000);
#endif
}

GekkoF GekkoCPURecompiler::Open(u32 entry_point)
{
	is_on = 0;
	loaded = 0;
	pause = 0;
	is_sc = 0;
	//exception = 0;
	is_dec = 0;

	mode = 0;

	//TBR.TBR = 0;
	opcode = 0;
	PClast = 0;

//	pPC = 0;

	memset(&ireg, 0, sizeof(ireg));

	ireg.PC = entry_point;
	ireg.IC = 0x0;
	ireg.MSR = MSR_BIT_DR | MSR_BIT_IR;

	LR = GEX_RESET;
	SP = ARENA_HI;

	DEC = 0xFFFFFFFF;

//	pPC = (u32*)&RAM[ireg.PC & RAM_MASK];
	loaded = true;
	NextJumpID_Val = 0;
	NextGetRegID_Val = 0x50;

	CachingEnabled = 1;
	TotalInstCount = 0;

	LOG_NOTICE(TPOWERPC, "Recompiler opened");
}

////////////////////////////////////////////////////////////

// Desc: Run Main CPU Thread
//

unsigned __stdcall GekkoRecompiler_RunThread( void* pArguments )
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

GekkoF GekkoCPURecompiler::Start(void)
{
	DWORD GekkoThread = 0;
	LOG_NOTICE(TPOWERPC, "starting PowerPC core!");

	if( false == is_on )
	{
		is_on = true;
		pause = false;

	    //if (common::g_config->enable_multicore())
		if (0)
		{
			hGekkoThread = CreateThread(NULL,NULL,(LPTHREAD_START_ROUTINE)GekkoRecompiler_RunThread,NULL,NULL,&GekkoThread);
		}
	} else {
		printf(".CPU: Gekko_Recompiler_Start - Gekko Core Already Started!\n");
	}
}

// Desc: Halt Main CPU Thread
//
GekkoF GekkoCPURecompiler::Halt(void)
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
GekkoRecOp(Ops_Group4XO0)
{
//	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup4XO0Table[XO0];

	iPtr();
}

GekkoRecOp(Ops_Group4)
{
//	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup4Table[XO3];

	iPtr();
} 

GekkoRecOp(Ops_Group19)
{
//	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup19Table[XO0];

	iPtr();
}

GekkoRecOp(Ops_Group31)
{
//	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup31Table[XO0];

	iPtr();
}

GekkoRecOp(Ops_Group59)
{
//	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup59Table[XO3];

	iPtr();
}

GekkoRecOp(Ops_Group63XO0)
{
//	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup63XO0Table[XO0];

	iPtr();
}

GekkoRecOp(Ops_Group63)
{
//	opcode = PTR_PC;
	GekkoFP iPtr = GekkoCPUOpsGroup63Table[XO3];

	iPtr();
}

GekkoRecIntOp(Ops_Group4XO0)
{
	GekkoFP iPtr = GekkoCPUOpsGroup4XO0Table[XO0];

	iPtr();
}

GekkoRecIntOp(Ops_Group4)
{
	GekkoFP iPtr = GekkoCPUOpsGroup4Table[XO3];

	iPtr();
}

GekkoRecIntOp(Ops_Group19)
	{
	GekkoFP iPtr = GekkoCPUOpsGroup19Table[XO3];

	iPtr();
}

GekkoRecIntOp(Ops_Group31)
	{
	GekkoFP iPtr = GekkoCPUOpsGroup31Table[XO3];

	iPtr();
}

GekkoRecIntOp(Ops_Group59)
{
	GekkoFP iPtr = GekkoCPUOpsGroup59Table[XO3];

	iPtr();
}

GekkoRecIntOp(Ops_Group63XO0)
{
	GekkoFP iPtr = GekkoCPUOpsGroup63XO0Table[XO0];

	iPtr();
}

GekkoRecIntOp(Ops_Group63)
{
	GekkoFP iPtr = GekkoCPUOpsGroup63Table[XO3];

	iPtr();
}

// Desc: Handle a CPU Exception
//

GekkoF GekkoCPURecompiler::Exception(tGekkoException which)
{
	Exception_REC(which);
}

GekkoF GekkoCPURecompiler::Exception_REC(tGekkoException which)
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
	//branch = 1;
	//exception = 1;
}

u32 GekkoCPURecompiler::GetTicksPerSecond()
{
	return (GEKKO_CLOCK / 4) / 3;
}

// Desc: Gekko Execute Instruction
//

GekkoF GekkoCPURecompiler::execStep(void)
{
	ExecuteInstruction();
}

GekkoF GekkoCPURecompiler::ExecuteInstruction(void)
{
	CompiledBlock	*BlockPtr;
/*
#ifdef _DEBUG
#define FILENAME "c:\\temp\\data-debug.txt"
#else
#define FILENAME "c:\\temp\\data-release.txt"
#endif
	HANDLE f = CreateFile(FILENAME, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
	DWORD BytesWritten;
	DWORD Dummy[2];
	memset(Dummy, 0x99, sizeof(Dummy));
	SetFilePointer(f, 0, 0, SEEK_END);
	WriteFile(f, &Dummy, sizeof(Dummy), &BytesWritten, 0);
	WriteFile(f, &ireg.PC, sizeof(ireg.PC), &BytesWritten, 0);
	WriteFile(f, ireg.gpr, sizeof(ireg.gpr)-4, &BytesWritten, 0);
	CloseHandle(f);
	*/

	BlockPtr = (CompiledBlock *)CompiledTable[(ireg.PC >> 2) & 0x007FFFFF];
	BlockPtr->CodeBlock();
}

GekkoF GekkoCPURecompiler::CompileInstruction(void)
{
	u32 LastOp;
	CompiledBlock	*BlockPtr;
	CompiledBlock	*BlockPtr2;
	GekkoFP			iPtr;
	RecInstruction	*ILInstruction;
	u32				LastInstrIsInt;
	DWORD			OldFlags;

	LastOp = ireg.PC;

	branch = 0;

//	if(ireg.IC >= 0x008f35a0)
//		_asm{int 3};

//	if(ireg.PC == 0x8025214C)
//		_asm{int 3};

//	if(ireg.PC == 0x802531C0)
//		_asm{int 3};

	int WriteFlag = 0;
	if(ireg.PC == 0x80048f78)
		WriteFlag = 1;

	for(;;)
	{
		cpu->opcode = PTR_PC;
		iPtr = GekkoCPUOpset[OPCD];

		if(WriteFlag)
		{
			char opcodeStr[32], operandStr[32];
			u32 target;
			char Buffer[8192];

			DWORD DataLen;
#ifdef _DEBUG
#define FILENAME "c:\\temp\\data-debug.txt"
#else
#define FILENAME "c:\\temp\\data-release.txt"
#endif
			HANDLE f = CreateFile(FILENAME, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
			SetFilePointer(f, 0, 0, FILE_END);
			DisassembleGekko(opcodeStr, operandStr, opcode, ireg.PC, &target);
			sprintf(Buffer, "Opcode %d (ext: %d) (pc=%08X, op=%08X) instr=%s r%d, r%d, r%d\r\n",cpu->opcode>>26,((cpu->opcode>>1)&0x3FF),ireg.PC,cpu->opcode,opcodeStr, rD, rA, rB);
			WriteFile(f, Buffer, strlen(Buffer), &DataLen, 0);
			CloseHandle(f);
		}

		iPtr();

		if(branch) // || step)
			break;

		ireg.PC += 4;
	};

	TotalInstCount += ((ireg.PC - LastOp) / 4) + 1;

//	if(ireg.IC >= 0x09680650)
//		_asm{int 3};

	//compile it
	CompileIL(LastOp, ireg.PC);

	//reset the pointer moves
	ireg.PC = LastOp;

	BlockPtr = (CompiledBlock *)CompiledTable[(LastOp >> 2) & 0x007FFFFF];

	//lock the area we just compiled from being modified
    // TODO(ShizZy): Something funky is going here... The commented out line is the right one, but 
    // it now causes crashes. See Issue #1
    //VirtualProtect((void *)((u32)(&Mem_RAM[LastOp & RAM_MASK]) & ~(PageSize-1)), PageSize, PAGE_EXECUTE_READ, &OldFlags);
	VirtualProtect((void *)((u32)(&Mem_RAM[LastOp & RAM_MASK]) & ~(PageSize-1)), PageSize, PAGE_EXECUTE_READWRITE, &OldFlags);

	if(WriteFlag)
	{
#ifdef _DEBUG
#define FILENAME "c:\\temp\\compiled-debug.bin"
#else
#define FILENAME "c:\\temp\\compiled-release.bin"
#endif
		HANDLE f = CreateFile(FILENAME, GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
		DWORD WrittenLen;
		WriteFile(f, BlockPtr->CodeBlock, BlockPtr->CommandLen, &WrittenLen, 0);
		CloseHandle(f);
	}

	BlockPtr->CodeBlock();
}

GekkoF GekkoCPURecompiler::DoBranchChecks()
{
	static u32 LastOp;
	static u32 LastFinishedOp=0;
	u32 x;

	LastOp = 0;
	for(x = 0; x < 32; x++)
		LastOp = (LastOp << 1) | CR[x];
	ireg.CR = LastOp;
	ireg.gpr[TEMP_REG] = 0;

	XER = (XER_SUMMARYOVERFLOW << 31) | (XER_OVERFLOW << 30) |
		(XER_CARRY << 29) | XER_WORDINDEX;

	cpu->ComparePipeData(LastFinishedOp);
	LastFinishedOp = ireg.PC;

	//if we are debugging, backup the registers
#ifdef USE_INLINE_ASM_X86
	if(PipeIsClient)
	{
		//if we are paused then something went wrong, print out some debug info
		if(pause)
		{
			LOG_DEBUG(TPOWERPC, "Prev CR: 0x%08X\n\n", iregBackup.CR);
			LOG_DEBUG(TPOWERPC, "Memory locations\n");
			LOG_DEBUG(TPOWERPC, "----------------\n");
			LOG_DEBUG(TPOWERPC, "GPR: 0x%08X\tCR: 0x%08X\tSPR: 0x%08X\tFPR:0x%08X\n", (u32)&ireg.gpr[0], (u32)CR, (u32)&ireg.spr[0], (u32)&ireg.fpr[0]);
			LOG_DEBUG(TPOWERPC, "XER_CARRY: 0x%08X\tXER_OVERFLOW: 0x%08X\tXER_SUMMARYOVERFLOW: 0x%08X\n", (u32)&XER_CARRY, (u32)&XER_OVERFLOW, (u32)&XER_SUMMARYOVERFLOW);
			LOG_DEBUG(TPOWERPC, "RAM: 0x%08X\t\tL2: 0x%08X\n", Mem_RAM, Mem_L2);
		}

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
#else
    #pragma todo("IMPLEMENT me on x64 for recompiler")
#endif
}

GekkoF GekkoCPURecompiler::CheckMemoryWriteInternal(u32 Addr, u32 Size)
{
	CompiledBlock	*BlockPtr;
	CompiledBlock	*NextBlockPtr;
	CompiledBlock	*LargestBlock;
	u32				x;
	u32				y;
	u32				BlockSize;

	//modify Addr to offset from where our RAM block starts
	Addr = Addr - (u32)Mem_RAM;
	Addr = Addr & ~(PageSize - 1);

	//printf("Checking memory 0x%08X\n", Addr);

	for(x = Addr; x < (Addr + PageSize); x+=4)
		CompiledTable[(x >> 2) & 0x007FFFFF] = (u32)&CompileBlockNeeded;

	//now free anything in the chain
	NextBlockPtr = (CompiledBlock *)CompiledTablePages[((Addr & 0x1FFFFFF) / PageSize)];
	while(NextBlockPtr)
	{
		LargestBlock = NextBlockPtr;
		NextBlockPtr = NextBlockPtr->NextBlock;

		RecompileFree(LargestBlock->CodeBlock);
		RecompileFree(LargestBlock);
	}

	CompiledTablePages[((Addr & 0x1FFFFFF) / PageSize)] = 0;

/*
#pragma todo(CheckMemoryWrite needs to not scan the whole page. Need new method to store used blocks in a page)
//	printf("Checking memory 0x%08X\n", Addr);

	//go thru the memory provided and free anything compiled
	for(x = Addr; x < (Addr + PageSize); x+=4)
	{
		BlockPtr = (CompiledBlock *)CompiledTable[(x >> 2) & 0x007FFFFF];

		if(BlockPtr != &CompileBlockNeeded)
		{
			//go to the parent block then find the size of all blocks covering this area
			NextBlockPtr = BlockPtr->ParentBlock;
			BlockSize = 0;
			if(NextBlockPtr)
			{
				while(NextBlockPtr)
				{
					if(NextBlockPtr->InstCount > BlockSize)
					{
						BlockSize = NextBlockPtr->InstCount;
						LargestBlock = NextBlockPtr;
					}
					NextBlockPtr = NextBlockPtr->NextBlock;
				}

//				printf("Found Block at %08X, size %d\n", x, (BlockSize * 4) - (LargestBlock->CodeStart - BlockPtr->CodeStart)); 

				//we subtract the code starts as the largest block may start before the
				//block we currently have
				x += (BlockSize * 4) - 4 - (LargestBlock->CodeStart - BlockPtr->CodeStart);
			}
			else
			{
				LargestBlock = BlockPtr;
				BlockSize = BlockPtr->InstCount;

//				printf("Found Block at %08X, size %d\n", x, (BlockSize * 4)); 
				x += (BlockSize * 4) - 4;
			}

			//wipe out the largest block that also includes this block
			//all blocks end at the same branch instruction
			//memset(&CompiledTable[LargestBlock->CodeStart >> 2], 0, BlockSize * 4);
			for(y = 0; y < BlockSize; y++)
				CompiledTable[(LargestBlock->CodeStart >> 2) + y] = (u32)&CompileBlockNeeded;

			//now free anything in the chain
			NextBlockPtr = BlockPtr->ParentBlock;
			while(NextBlockPtr)
			{
				LargestBlock = NextBlockPtr;
				NextBlockPtr = NextBlockPtr->NextBlock;

//				if(BlockPtr == LargestBlock)
//					BlockPtr = 0;

				if(LargestBlock->CodeBlock != &GekkoCPURecompiler::CompileInstruction)
					RecompileFree(LargestBlock->CodeBlock);
				RecompileFree(LargestBlock);
			}
		}
	}
*/
}

GekkoF GekkoCPURecompiler::CreateRecInstruction(RecILInstructionPtr Instruction, u32 Out, u32 In, u32 Flags, RecCacheInstructionPtr CachePtr)
{
	RecInstruction *NewInstruction;

	NewInstruction = (RecInstruction *)RecompileAlloc(sizeof(RecInstruction));
	NewInstruction->Instruction = Instruction;

	if(Flags & RecInstrFlagX86_INMASK)
		NewInstruction->X86InReg = In;
	else
		NewInstruction->InReg = In;

	if(Flags & RecInstrFlagX86_OUTMASK)
		NewInstruction->X86OutReg = Out;
	else if(Flags & RecInstrFlag_TempData)
		NewInstruction->TempData = Out;
	else
		NewInstruction->OutReg = Out;

	NewInstruction->Flags = Flags;
	NewInstruction->Address = ireg.PC;
	NewInstruction->PPCCacheInst = CachePtr;
	NewInstruction->X86Displacement = 0;

	//add an instruction to the tree of current instructions
	NewInstruction->Prev = LastInstruction;
	NewInstruction->Next = 0;
	if(LastInstruction)
		LastInstruction->Next = NewInstruction;

	LastInstruction = NewInstruction;

	if(!InstructionBlock)
		InstructionBlock = NewInstruction;
}

GekkoF GekkoCPURecompiler::DumpInternalData(u32 DumpPC, u32 DumpLen)
{
	CompiledBlock	*BlockPtr;
	u32				x;
//	HANDLE			File;
	u32				CurPC;
/*
	printf("Dumping recompile memory to memdump.bin\n");
	File = CreateFile("memdump.bin", GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	WriteFile(File, StartMem, RECOMPILEMEM_SIZE, (LPDWORD)&x, 0);
	CloseHandle(File);
*/
	if(!DumpLen)
		DumpLen = 4;

	CurPC = DumpPC;
	while((CurPC - DumpPC) < DumpLen)
	{
		BlockPtr = (CompiledBlock *)CompiledTable[(CurPC >> 2) & 0x007FFFFF];

		printf("Code block 0x%08X (Offset 0x%08X from memory block start) for PC 0x%08X\n", (u32)BlockPtr, (u32)BlockPtr - (u32)StartMem, CurPC);
		printf("Block Info: Inst Count: %d, CmdLen: %d\n", BlockPtr->InstCount, BlockPtr->CommandLen);
		for(x = 0; x < BlockPtr->CommandLen; x++)
		{
			if(x % 32 == 0)
				printf("\n");
			printf("%02X ", ((u8 *)BlockPtr->CodeBlock)[x]);
		}
		printf("\n\n");

		CurPC += (BlockPtr->InstCount * 4);
		if(BlockPtr->InstCount == 0)
			break;
	}

	printf("Dump Complete\n");
}