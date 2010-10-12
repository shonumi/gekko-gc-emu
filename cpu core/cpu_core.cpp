#include "../emu.h"
#include "cpu_core.h"
#include "cpu_core_regs.h"
#include "disassembler/ppc_disasm.h"
#include "../user interface/emu_win.h"
#include "../low level/hardware core/hw_vi.h"

//allow only 1 define of the registers, these two are global accessible
GekkoCPU*		cpu;
Gekko_Registers	ireg;
Gekko_Registers iregBackup;

optable		GekkoCPU::GekkoCPUOpsGroup4XO0Table[0x400];
optable		GekkoCPU::GekkoCPUOpsGroup4Table[0x20];
optable		GekkoCPU::GekkoCPUOpsGroup19Table[0x400];
optable		GekkoCPU::GekkoCPUOpsGroup31Table[0x400];
optable		GekkoCPU::GekkoCPUOpsGroup59Table[0x400];
optable		GekkoCPU::GekkoCPUOpsGroup63XO0Table[0x400];
optable		GekkoCPU::GekkoCPUOpsGroup63Table[0x20];
optable		GekkoCPU::GekkoCPUOpset[0x40];

HANDLE		GekkoCPU::PipeHandle;
u8 *		GekkoCPU::PipeHandleData;
u32			GekkoCPU::PipeIsClient;

//f32			GekkoCPU::ldScale[64];
//f32			GekkoCPU::stScale[64];

const f32 GekkoCPU::ldScale[] =
{
	1.0/(1 <<  0),
	1.0/(1 <<  1),
	1.0/(1 <<  2),
	1.0/(1 <<  3),
	1.0/(1 <<  4),
	1.0/(1 <<  5),
	1.0/(1 <<  6),
	1.0/(1 <<  7),
	1.0/(1 <<  8),
	1.0/(1 <<  9),
	1.0/(1 << 10),
	1.0/(1 << 11),
	1.0/(1 << 12),
	1.0/(1 << 13),
	1.0/(1 << 14),
	1.0/(1 << 15),
	1.0/(1 << 16),
	1.0/(1 << 17),
	1.0/(1 << 18),
	1.0/(1 << 19),
	1.0/(1 << 20),
	1.0/(1 << 21),
	1.0/(1 << 22),
	1.0/(1 << 23),
	1.0/(1 << 24),
	1.0/(1 << 25),
	1.0/(1 << 26),
	1.0/(1 << 27),
	1.0/(1 << 28),
	1.0/(1 << 29),
	1.0/(1 << 30),
	1.0/(1 << 31),

	(1ULL << 32),
	(1 << 31),
	(1 << 30),
	(1 << 29),
	(1 << 28),
	(1 << 27),
	(1 << 26),
	(1 << 25),
	(1 << 24),
	(1 << 23),
	(1 << 22),
	(1 << 21),
	(1 << 20),
	(1 << 19),
	(1 << 18),
	(1 << 17),
	(1 << 16),
	(1 << 15),
	(1 << 14),
	(1 << 13),
	(1 << 12),
	(1 << 11),
	(1 << 10),
	(1 <<  9),
	(1 <<  8),
	(1 <<  7),
	(1 <<  6),
	(1 <<  5),
	(1 <<  4),
	(1 <<  3),
	(1 <<  2),
	(1 <<  1),
};

// quantization factor
const f32 GekkoCPU::stScale[] =
{
	(1 <<  0),
	(1 <<  1),
	(1 <<  2),
	(1 <<  3),
	(1 <<  4),
	(1 <<  5),
	(1 <<  6),
	(1 <<  7),
	(1 <<  8),
	(1 <<  9),

	(1 << 10),
	(1 << 11),
	(1 << 12),
	(1 << 13),
	(1 << 14),
	(1 << 15),
	(1 << 16),
	(1 << 17),
	(1 << 18),
	(1 << 19),

	(1 << 20),
	(1 << 21),
	(1 << 22),
	(1 << 23),
	(1 << 24),
	(1 << 25),
	(1 << 26),
	(1 << 27),
	(1 << 28),
	(1 << 29),
	(1 << 30),
	(1 << 31),

	1.0/(1ULL << 32),
	1.0/(1 << 31),
	1.0/(1 << 30),

	1.0/(1 << 29),
	1.0/(1 << 28),
	1.0/(1 << 27),
	1.0/(1 << 26),
	1.0/(1 << 25),
	1.0/(1 << 24),
	1.0/(1 << 23),
	1.0/(1 << 22),
	1.0/(1 << 21),
	1.0/(1 << 20),

	1.0/(1 << 19),
	1.0/(1 << 18),
	1.0/(1 << 17),
	1.0/(1 << 16),
	1.0/(1 << 15),
	1.0/(1 << 14),
	1.0/(1 << 13),
	1.0/(1 << 12),
	1.0/(1 << 11),
	1.0/(1 << 10),

	1.0/(1 <<  9),
	1.0/(1 <<  8),
	1.0/(1 <<  7),
	1.0/(1 <<  6),
	1.0/(1 <<  5),
	1.0/(1 <<  4),
	1.0/(1 <<  3),
	1.0/(1 <<  2),
	1.0/(1 <<  1),
};

bool	GekkoCPU::is_on;
bool	GekkoCPU::loaded;
bool	GekkoCPU::pause;
bool	GekkoCPU::is_sc;
bool	GekkoCPU::is_dec;
bool	GekkoCPU::is_reserved;
bool	GekkoCPU::DumpOp0;
bool	GekkoCPU::PauseOnUnknownOp;

u32		GekkoCPU::reserved_addr;
u8		GekkoCPU::mode;

bool	GekkoCPU::step;
u32		GekkoCPU::opcode;

u32		GekkoCPU::PClast;

u32*	GekkoCPU::pPC;

//Generic CPU Core interface class
GekkoCPU::CPUType GekkoCPU::GetCPUType()
{
	return GekkoCPU::Unknown;
}

GekkoCPU::GekkoCPU()
{
	is_on = 0;
	loaded = 0;
	pause = 0;
	is_dec = 0;
	is_sc = 0;
	is_reserved = 0;
	DumpOp0 = 0;
	PauseOnUnknownOp = 0;

	reserved_addr = 0;
	mode = 0;

	step = 0;
	opcode = 0;

	PClast = 0;
	pPC = 0;

	hGekkoThread = NULL;
	ireg.TBR.TBR = 0;

	PipeHandle = 0;		//INVALID_HANDLE_VALUE
	PipeHandleData = 0;
	PipeIsClient = 0;
}

GekkoCPU::~GekkoCPU()
{
	if(hGekkoThread)
		this->Halt();

	if(PipeHandle)	//if(PipeHandle != INVALID_HANDLE_VALUE)
	{
		UnmapViewOfFile(PipeHandleData);
		CloseHandle(PipeHandle);
/*		if(PipeIsClient)
			CloseHandle(PipeHandle);
		else
			UnmapViewOfFile(PipeHandle);	//DisconnectNamedPipe(PipeHandle);
*/
	}

	hGekkoThread = NULL;
}

GekkoF GekkoCPU::ExecuteInstruction()
{
	printf("GekkoCPU:ExecuteInstruction\n");
}

GekkoF GekkoCPU::Open(u32 EntryPoint)
{
	printf("GekkoCPU:Open\n");
}

GekkoF GekkoCPU::Start()
{
	printf("GekkoCPU:Start\n");
}

GekkoF GekkoCPU::Halt()
{
	printf("GekkoCPU:Halt\n");
}

GekkoF GekkoCPU::Exception(tGekkoException which)
{
	printf("GekkoCPU:Exception\n");
}

u64 GekkoCPU::GetTicks()
{
	return ireg.TBR.TBR;
}

u32 GekkoCPU::GetTicksPerSecond()
{
	return GEKKO_CLOCK;
}

GekkoF GekkoCPU::execStep()
{
	ExecuteInstruction();
}

GekkoF GekkoCPU::StartPipe(u32 IsClient)
{
	if(IsClient)
	{
        PipeHandle = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE,FALSE,"GekkoCPURecompile");
		if(!PipeHandle)
			printf("Unable to create fucked up pipe!\n");
	}
	else
	{
		PipeHandle = CreateFileMapping((HANDLE)0xFFFFFFFF,NULL, PAGE_READWRITE, 
								0,sizeof(ireg) + 0x100,"GekkoCPURecompile");

		if(!PipeHandle)
			printf("Unable to create gnarly pipe!\n");
	}

	if(PipeHandle)
		PipeHandleData = (u8 *)MapViewOfFile(PipeHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);

	if(PipeHandleData)
	{
		memset(PipeHandleData, 0, sizeof(ireg) + 0x100);
		if(IsClient)
		{
			printf("Created fucked up pipe 0x%08X 0x%08X\n", PipeHandle, PipeHandleData);
			SetWindowText(wnd.hWnd, APP_NAME " - Fucked Up");
		}
		else
		{
			printf("Created gnarly pipe 0x%08X 0x%08X\n", PipeHandle, PipeHandleData);
			SetWindowText(wnd.hWnd, APP_NAME " - Gnarly");
		}
	}
	else
	{
		CloseHandle(PipeHandle);
		PipeHandle = 0;
		if(IsClient)
			printf("Unable to create fucked up pipe data connection!\n");
		else
			printf("Unable to create gnarly pipe data connection!\n");
	}

	PipeIsClient = IsClient;

	//wait around for the filename to load
	if(IsClient)
	{
		printf("CPU Compare waiting on game to load\n");
		ComparePipeData(0);
	}
}

GekkoF GekkoCPU::DumpInternalData(u32 DumpPC, u32 DumpLen)
{
	printf("No Internal data to dump!\n");
}

void GekkoCPU::HandleSpecialPipeData()
{
	char	Filename[1024];
	u32		CRC;
	u32		x;
	Gekko_Registers	CompareRegs;

	switch(PipeHandleData[1])
	{
		case 'F':
			//load a file
			PipeHandleData[0] = 0x00;
			memcpy(Filename, &PipeHandleData[2], strlen((char *)&PipeHandleData[2]) + 1);
			WIN_LoadFromString(Filename);
			break;

		case 'S':
			//start the emulation
			PipeHandleData[0] = 0x00;
			emu.started = true;
			emu.status = EMU_RUNNING;
			break;

		case 'I':
			//instruction count hit, check memory and registers
			CRC = GenerateCRC((u8 *)&ireg, sizeof(ireg));
			if(CRC != *(u32 *)(&PipeHandleData[2]))
			{
				printf("CPU Register mismatch before instruction count hit\n\tCRC: 0x%08X\tCorrect: 0x%08X\n", CRC, *(u32 *)(&PipeHandleData[2]));
				pause = true;
			}

			CRC = GenerateCRC(Mem_RAM, RAM_SIZE);
			if(CRC != *(u32 *)(&PipeHandleData[6]))
			{
				printf("Memory mismatch before instruction count hit\n\tCRC: 0x%08X\tCorrect: 0x%08X\n", CRC, *(u32 *)(&PipeHandleData[6]));
				pause = true;
			}

			//signal good or bad
			if(pause)
				PipeHandleData[0] = 0x03;
			else
			{
				PipeHandleData[0] = 0x02;	//all good
				printf("Synced with server at IC %08X\n", ireg.IC);
			}

			break;
	}
}

GekkoF GekkoCPU::ComparePipeData(u32 LastAddress)
{
	DWORD				BytesOut;
	u32					ReceivedPC;
	u32					x;
	u32					MemCmpCRC;
	u32					RegCmpCRC;
	u32					MemCRC;
	u32					RegCRC;
	u32					CurAddress;
	static u32			StartedCompare = 0;
	u32					StartTime;
	t_Gekko_Registers	TempReg;

#define CPU_IC	0x00000000
#define CPU_COMPARE_MEM 0

#if(AllowCompareAcrossEXEs)
	//server, send data
	u8 RegCompare[sizeof(Gekko_Registers)];
	u32	RegOffset;
#endif

	if(PipeIsClient)
	{
		for(;PipeHandleData[0] != 0x01 && PipeHandleData[0] != 0xFE;)
		{
			Sleep(0);
			if(PipeHandleData[0] == 0xFF)
			{
				HandleSpecialPipeData();
				return;
			}
		}

		if(!StartedCompare)
		{
			if(ireg.IC < CPU_IC)
				return;
			else
			{
				//set our flag and wait for the server to be ready
				StartedCompare = 1;
				printf("CPU Compare waiting on server for sync\n");
				for(;PipeHandleData[0] != 0xFE;)
				{
					Sleep(5);
				}
				HandleSpecialPipeData();
				return;

			}
		}

		RegCmpCRC = *(u32 *)(&PipeHandleData[1]);
//		memcpy(&TempReg, &ireg, sizeof(ireg));

/*		//due to a difference between int and rec, knock off the lower 32bits
		for(x = 0; x < 32; x++)
		{
			TempReg.fpr[x].ps0._u32[0] = 0;
			TempReg.fpr[x].ps0._u32[1] &= 0xFFFF0000;
			TempReg.fpr[x].ps1._u32[0] = 0;
			TempReg.fpr[x].ps1._u32[1] &= 0xFFFF0000;
		}
*/
#if(CPU_COMPARE_MEM)
		RegCRC = GenerateCRC(Mem_RAM, RAM_SIZE);
#else
		RegCRC = GenerateCRC((u8 *)&ireg, sizeof(ireg));
#endif


		//copy our keys
		if(PipeHandleData[5])
			memcpy(emu.keys, (void *)(&PipeHandleData[6]), sizeof(emu.keys));

		if(RegCRC == RegCmpCRC)
		{
			//return our position to show good
			PipeHandleData[0] = 0x02;
			return;
		}

		Gekko_Registers	CompareRegs;
		char			opcodeStr[32], operandStr[32];
		u32				target;
		u32				opcode;  

		printf("CPU Data Mismatch!\n");
		printf("------------------\n\n");
		printf("Reg CRC: 0x%08X\tCorrect: 0x%08X\n", RegCRC, RegCmpCRC);
//		printf("Mem CRC: 0x%08X\tCorrect: 0x%08X\n", MemCRC, MemCmpCRC);

		//bad, return an invalid pointer
		PipeHandleData[0] = 0x03;

		for(;PipeHandleData[0] != 0x01;){Sleep(0);}

		memcpy(&CompareRegs, &PipeHandleData[1], sizeof(CompareRegs));

		//something does not match
		pause = true;

		x = 0;
		CurAddress = LastAddress;
		while(x != BRANCH_OPCODE)
		{
			opcode = Memory_Read32(CurAddress);

			x = DisassembleGekko(opcodeStr, operandStr, opcode, CurAddress, &target);
			printf("%08X (%08X): %s\t%s\n", CurAddress, opcode, opcodeStr, operandStr);

			CurAddress += 4;
		};

		printf("\nPC: 0x%08X\tLast PC: 0x%08X", ireg.PC, iregBackup.PC);
		if(ireg.PC != CompareRegs.PC)
			printf("\tCorrect: 0x%08X\n", CompareRegs.PC);
		else
			printf("\n");

		printf("TBR: 0x%016I64X", ireg.TBR.TBR);
		if(ireg.TBR.TBR != CompareRegs.TBR.TBR)
			printf("\tCorrect: 0x%016I64X\n", CompareRegs.TBR.TBR);
		else
			printf("\n");

		printf("CR: 0x%08X", ireg.CR);
		if(ireg.CR != CompareRegs.CR)
			printf("\tCorrect: 0x%08X\n", CompareRegs.CR);
		else
			printf("\n");

		printf("IC: 0x%08X", ireg.IC);
		if(ireg.IC != CompareRegs.IC)
			printf("\tCorrect: 0x%08X\n", CompareRegs.IC);
		else
			printf("\n");

		printf("MSR: 0x%08X", ireg.MSR);
		if(ireg.MSR != CompareRegs.MSR)
			printf("\tCorrect: 0x%08X\n", CompareRegs.MSR);
		else
			printf("\n");

		printf("FPSCR: 0x%08X", ireg.FPSCR);
		if(ireg.FPSCR != CompareRegs.FPSCR)
			printf("\tCorrect: 0x%08X\n", CompareRegs.FPSCR);
		else
			printf("\n");

		printf("SRR0: 0x%08X\n", SRR0);
		printf("CTR: 0x%08X\n", CTR);

		//find out the invalid data
		for(x=0; x < 32; x++)
		{
			printf("GPR %d: Start: 0x%08X\tEnd: 0x%08X", x, iregBackup.gpr[x], ireg.gpr[x]);

			if(ireg.gpr[x] != CompareRegs.gpr[x])
				printf("\tCorrect: 0x%08X\n", CompareRegs.gpr[x]);
			else
				printf("\n");
		}

		//find out the invalid data
		for(x=0; x < 1024; x++)
		{
			if(ireg.spr[x] != CompareRegs.spr[x])
				printf("SPR %d: Start: 0x%08X\tEnd: 0x%08X\tCorrect: 0x%08X\n", x, iregBackup.spr[x], ireg.spr[x], CompareRegs.spr[x]);
		}

		for(x=0; x < 16; x++)
		{
			if(ireg.sr[x] != CompareRegs.sr[x])
				printf("SR %d: Start: 0x%08X\tEnd: 0x%08X\tCorrect: 0x%08X\n", x, iregBackup.sr[x], ireg.sr[x], CompareRegs.sr[x]);
		}

		for(x=0; x < 32; x++)
		{
			printf("FPR %d Start: 0x%016I64X-%016I64X\nFPR %d End:0x%016I64X-%016I64X\n", x, iregBackup.fpr[x].ps1._u64, iregBackup.fpr[x].ps0._u64, x, ireg.fpr[x].ps1._u64, ireg.fpr[x].ps0._u64);
//			if(((ireg.fpr[x].ps0._u32[1] & 0xFFFF0000) != (CompareRegs.fpr[x].ps0._u32[1] & 0xFFFF0000)) ||
//				((ireg.fpr[x].ps1._u32[1] & 0xFFFF0000) != (CompareRegs.fpr[x].ps1._u32[1] & 0xFFFF0000)))
			if((ireg.fpr[x].ps0._u32[1] != CompareRegs.fpr[x].ps0._u32[1]) ||
				(ireg.fpr[x].ps1._u32[1] != CompareRegs.fpr[x].ps1._u32[1]) ||
				(ireg.fpr[x].ps0._u32[0] != CompareRegs.fpr[x].ps0._u32[0]) ||
				(ireg.fpr[x].ps1._u32[0] != CompareRegs.fpr[x].ps1._u32[0]))
				printf("FPR %d Correct: 0x%016I64X-%016I64X\n", x, CompareRegs.fpr[x].ps1._u64, CompareRegs.fpr[x].ps0._u64);

			printf("\n");
		}

		//tell the cpu do dump it's data
		DumpInternalData(LastAddress, CurAddress - LastAddress);
	}
	else
	{
		if(!StartedCompare)
		{
			if(ireg.IC < CPU_IC)
			{
				//let it pass thru on the client side, client won't skip past due to needing other input
				PipeHandleData[0] = 0x01;
				return;
			}
			else
			{
				StartedCompare = 0x01;

				//generate the needed crc's for a full compare of memory and cpu
				RegCRC = GenerateCRC((u8 *)&ireg, sizeof(ireg));
				*(u32*)(&PipeHandleData[2]) = RegCRC;

				MemCRC = GenerateCRC(Mem_RAM, RAM_SIZE);
				*(u32*)(&PipeHandleData[6]) = MemCRC;

				PipeHandleData[1] = 'I';
				PipeHandleData[0] = 0xFE;

				printf("CPU Compare waiting on client for sync\n");
				for(;PipeHandleData[0] == 0xFE;){Sleep(5);}

				if(PipeHandleData[0] != 0x02)
				{
					printf("Memory or CPU does not match upon instruction count expiration\n");
					pause = true;
				}
				else
					printf("Synced at IC %08X\n", CPU_IC);
				return;
			}
		}

		//if paused then don't allow any of the data to be modified
		if(pause)
			return;

		//generate the register crc

		//due to a difference between int and rec, knock off the lower 32bits
//		memcpy(&TempReg, &ireg, sizeof(ireg));
/*		for(x = 0; x < 32; x++)
		{
			TempReg.fpr[x].ps0._u32[0] = 0;
			TempReg.fpr[x].ps0._u32[1] &= 0xFFFF0000;
			TempReg.fpr[x].ps1._u32[0] = 0;
			TempReg.fpr[x].ps1._u32[1] &= 0xFFFF0000;
		}
*/
#if(CPU_COMPARE_MEM)
		RegCRC = GenerateCRC(Mem_RAM, RAM_SIZE);
#else
		RegCRC = GenerateCRC((u8 *)&ireg, sizeof(ireg));
#endif
		*(u32*)&PipeHandleData[1] = RegCRC;

		//copy our keys
		if(emu.keychange) 
		{
			PipeHandleData[5] = 1;
			memcpy((void *)(&PipeHandleData[6]), emu.compkeys, sizeof(emu.compkeys));
			memcpy((void *)emu.keys, emu.compkeys, sizeof(emu.keys));
			emu.keychange = FALSE;
		}
		else
		{
			PipeHandleData[5] = 0;
		}

		//generate a CRC of the memory
//		MemCRC = GenerateCRC((u8 *)&Mem_RAM, sizeof(Mem_RAM));
//		*(u32*)&PipeHandleData[5] = MemCRC;

		PipeHandleData[0] = 0x01;

		StartTime = timeGetTime();
		for(;PipeHandleData[0] == 0x01;)
		{
			Sleep(0);
			if((timeGetTime() - StartTime) > 5000)
			{
				printf("Waiting on client, IC @ 0x%08X\n", ireg.IC);
				for(;PipeHandleData[0] == 0x01;){Sleep(0);}
			}
		}

		if(PipeHandleData[0] != 0x02)
		{
			printf("Fucked Up version fucked up\n");
			pause = true;

			memcpy(&PipeHandleData[1], &ireg, sizeof(ireg));
			PipeHandleData[0] = 0x01;
		}
	}
}

void GekkoCPU::SendPipeData(void *Data, u32 DataLen)
{
	if(!PipeIsClient)
	{
		memcpy((void *)&PipeHandleData[1], Data, DataLen);
		PipeHandleData[0] = 0xFF;
	}
}

u32 GekkoCPU::IsCPUCompareActive()
{
	u32	Ret;

	//return 0, 1, or 2 depending if cpu compare is active
	Ret = 0;
	if(PipeHandle)
	{
		if(PipeIsClient)
			Ret = 1;
		else
			Ret = 2;
	}

	return Ret;
}