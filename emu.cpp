// emu.cpp
// (c) 2005,2006 Gekko Team

#include "emu.h"
#include "cpu core\recompiler\cpu_rec.h"
#include "dvd interface\realdvd.h"
#include "gx\gx_fifo.h"

emuMode emu;

#if GEKKO_PLATFORM == GEKKO_PLATFORM_WINDOWS
char *kGekkoOS = "Windows";
#  elif GEKKO_PLATFORM == GEKKO_PLATFORM_MAC
char *kGekkoOS = "Mac OS X";
#  else // Assume linux if not mac or windows
char *kGekkoOS = "Linux";
#  endif


char ProgramDirectory[1024];
u32	 DumpGCMBlockReads;
char DumpDirectory[1024];
u32 DisplayFPS;
char MemoryCardImage[1024];
char DVDRootPath[1024];
char LoadedGameFile[1024];

////////////////////////////////////////////////////////////
void Read_INI()
{
	char	INIFile[1024];

	sprintf(INIFile, "%sGekko.ini", ProgramDirectory);

	//read the ini settings and set the menus accordingly
	DisableHLEPatches = GetPrivateProfileInt("Debug", "Disable HLE Patches", 0, INIFile);
	if(DisableHLEPatches)
	{
		CheckMenuItem(GetMenu(wnd.hWnd), ID_OPTIONS_DISABLEHLE, MF_CHECKED);
		printf("Disabling HLE Patches\n");
	}

	DisableINIPatches = GetPrivateProfileInt("Debug", "Disable INI Patches", 0, INIFile);
	if(DisableINIPatches)
	{
		CheckMenuItem(GetMenu(wnd.hWnd), ID_OPTIONS_DISABLEINIPATCHES, MF_CHECKED);
		printf("Disabling INI Patches\n");
	}

	if(GetPrivateProfileInt("System", "DynaRec", 0, INIFile) == 1)
	{
		delete cpu;
		cpu = new GekkoCPURecompiler();
		CheckMenuItem(GetMenu(wnd.hWnd), ID_SYSTEM_DYNAREC, MF_CHECKED);
	}

	cpu->DumpOp0 =  GetPrivateProfileInt("Debug", "Dump Opcode 0", 0, INIFile);
	if(cpu->DumpOp0)
	{
		CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUG_DUMPOPCODE0, MF_CHECKED);
		printf("Enabling Opcode 0 dump\n");
	}

	cpu->PauseOnUnknownOp =  GetPrivateProfileInt("Debug", "Pause On Unknown Opcode", 0, INIFile);
	if(cpu->PauseOnUnknownOp)
	{
		CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUG_PAUSEONUNKNOWNOPCODE, MF_CHECKED);
		printf("Enabling pause on unknown opcode\n");
	}

	cfg.enb_framebuffer = GetPrivateProfileInt("System", "Framebuffer", 0, INIFile);
	cfg.enb_framebuffer = !cfg.enb_framebuffer; // toggle
	CFG_Enable_Framebuffer(); // toggle back (updates menu)

	cfg.enb_autostart   = GetPrivateProfileInt("Emulation", "AutoStart", 0, INIFile);
  cfg.enb_autostart = !cfg.enb_autostart;
	CFG_Enable_AutoStart(); // toggle back (updates menu)

	DumpGCMBlockReads =  GetPrivateProfileInt("Debug", "Dump GCM Reads", 0, INIFile);
	if(DumpGCMBlockReads)
	{
		CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUGOPTIONS_DUMPBLOCKREADS, MF_CHECKED);
		printf("Dumping GCM Reads\n");
	}

	memset(DumpDirectory, 0, sizeof(DumpDirectory));
	GetPrivateProfileString("Debug", "Dump Directory", 0, DumpDirectory, 1024, INIFile);	

	memset(g_szRomPath, 0, sizeof(g_szRomPath));
	GetPrivateProfileString("System","ROM Path", 0, g_szRomPath, sizeof(g_szRomPath), INIFile);
	BrowserReloadWindow();

	DisplayFPS = GetPrivateProfileInt("System", "Display FPS", 0, INIFile);
	if(DisplayFPS)
		CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUGOPTIONS_DISPLAYFRAMERATE, MF_CHECKED);

	DVDRootPath[0] = 0;
	GetPrivateProfileString("ELF", "DVDRoot", 0, DVDRootPath, 1024, INIFile);	
}

int Init_Emu()
{
	memset(&emu, 0, sizeof(emu));
	emu.status = EMU_HALTED;

	return 1;
}

extern void WIN_LoadFromString(char _filename[]);
void StartGame(char *Filename)
{
	WIN_LoadFromString(Filename);

	sprintf(g_szRomPath,"%s",Filename);
	
	for(int i=strlen(g_szRomPath);i>0;i--)
	{
		if(g_szRomPath[i]=='\\') 
		{
			g_szRomPath[i]='\0';
			break;
		}
	}

	emu.status = EMU_RUNNING;
}

////////////////////////////////////////////////////////////
int __cdecl main(int argc, char **argv)
{
	u32	SSEVal;
	u32	RetVal;
	u32 Counter;

//print leak info if enabled
#ifdef LEAK_DETECT
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	COORD ConsoleCoords = {125, 1000};
	HANDLE ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleScreenBufferSize(ConsoleHandle, ConsoleCoords);

	printf(APP_NAME " Starting...\n");

	DumpGCMBlockReads = 0;

	GetModuleFileName(NULL,ProgramDirectory,sizeof(ProgramDirectory));
	for(int i=strlen(ProgramDirectory);i>0;i--)
	{
		if(ProgramDirectory[i - 1]=='\\') 
		{
			ProgramDirectory[i]='\0';
			break;
		}
	}

	//if using asm, see if this computer can process
#ifdef USE_INLINE_ASM
	printf("Compiled with inline assembly...");
	_asm
	{
		//lets see if we can execute CPUID by seeing if we can flip the 21st bit
		//of the flags register

		pushfd
		pop eax

		//save the original
		mov ecx, eax

		//swap the 21st bit
		xor eax, 200000h

		//push it back on the stack, set the flags
		push eax
		popfd

		//now get the flags
		pushfd
		pop eax

		//see if the values are the same
		xor eax, ecx
		jz NoCPUIDSupport
		
		//CPUID supported, go ahead and grab some info
		mov eax, 1
		CPUID

		and edx, 04000000h
		mov SSEVal, edx
		mov RetVal, 1

	NoCPUIDSupport:
	};

	if(!RetVal)
	{
		printf("Aborting Execution\n");
		printf("CPU does not support the CPUID instruction\n");
		return -1;
	}

	if(!SSEVal)
	{
		printf("Aborting Execution\n");
		printf("SSE2 is required for " APP_NAME " but not found on the chip\n");
		return -1;
	}
	else
	{
		printf("SSE2 found\n");
	}
#endif

	Memory_Open();

	if(Init_Emu()) printf(".Emu: Initialization Successful...\n");
	else
	{
		printf(".Emu: Initialization Failed... Aborting Emu.\n");
		emu.status = EMU_QUIT;
	}

	if(WIN_Create()) printf(".Emu: Window Initialization Successful...\n");
	else
	{
		printf(".Emu: Window Initialization Failed... Aborting Emu.\n");
		emu.status = EMU_QUIT;
	}

	CFG_Init(); // Auto Configure basic settings.
	Read_INI();
	Init_CRC32_Table();
	OPENGL_Create(wnd.hWnd);
//	StartGame("e:\\gekko\\tloz_windwaker_image.gcm");

	while( EMU_QUIT != emu.status )
	{
		if (EMU_RUNNING == emu.status)
		{
			if(cpu->is_on == false)
			{
				BrowserKill();
				cpu->Start();				// Initialize and start CPU.
				CFG_Video_Apply();
			}

#ifdef SINGLETHREADED
			if(cpu->is_on)
			{
#ifdef USE_INLINE_ASM
				_asm
				{
					mov Counter, 10000

			ContinueCPULoop:
					mov ecx, cpu
					cmp GekkoCPU::pause, 0
					jne CPULoopDone
					mov edx, [ecx]	//call cpu->ExecuteInstruction
					call [edx]
					mov ecx, cpu
					mov edx, [ecx]	//call cpu->ExecuteInstruction
					call [edx]
					mov ecx, cpu
					mov edx, [ecx]	//call cpu->ExecuteInstruction
					call [edx]
					mov ecx, cpu
					mov edx, [ecx]	//call cpu->ExecuteInstruction
					call [edx]
					mov ecx, cpu
					mov edx, [ecx]	//call cpu->ExecuteInstruction
					call [edx]
					cmp emu.status, EMU_RUNNING
					jne CPULoopDone
					sub Counter, 1
					jnz ContinueCPULoop
			CPULoopDone:
				};
#else
				// Single threaded mode (Currently only mode properly supported).
				for(int i = 0; i < 10000 && !cpu->pause && emu.status == EMU_RUNNING; ++i)
				{
					cpu->execStep();
				}
#endif
			}
#endif
		}
		else if (EMU_HALTED == emu.status)
		{
			if(cpu->is_on == true)
			{
				cpu->Halt();				// Stop CPU
				Flipper_Close();			// Stop Hardware
				BrowserReloadWindow();		// Reload Emu Window
			}
		}
		else if (emu.status == EMU_LOAD)
		{
			if(!cpu->loaded)
			{
				WIN_LoadFromString(LoadedGameFile);
				if (cfg.enb_autostart)
					SNDMSG(wnd.hWnd, WM_COMMAND, ID_EMULATION_START, 0);
			}
		}

		WIN_Msg();
	}
	
	Flipper_Close();
	RealDVDClose(-1);

	delete cpu;
	Memory_Close();

	gx_fifo::destroy();

	return E_OK;
}