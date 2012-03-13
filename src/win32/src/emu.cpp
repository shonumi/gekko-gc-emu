// emu.cpp
// (c) 2005,2006 Gekko Team

#include "SDL.h"
#include "emu.h"
#include "powerpc/recompiler/cpu_rec.h"
#include "dvd/realdvd.h"
#include "video/gx_fifo.h"


emuMode emu;

#if EMU_PLATFORM == PLATFORM_NULL
char *kGekkoOS = "Unknown";
#elif EMU_PLATFORM == PLATFORM_WINDOWS
char *kGekkoOS = "Windows";
#elif EMU_PLATFORM == PLATFORM_MACOSX
char *kGekkoOS = "Mac OS X";
#elif EMU_PLATFORM == PLATFORM_LINUX
char *kGekkoOS = "Linux";
#elif EMU_PLATFORM == PLATFORM_ANDROID
char *kGekkoOS = "Android";
#elif EMU_PLATFORM == PLATFORM_IOS
char *kGekkoOS = "iOS";
#else
char *kGekkoOS = "Other";
#endif


char ProgramDirectory[1024];
extern u32	 DumpGCMBlockReads;
extern char DumpDirectory[1024];
u32 DisplayFPS;
char MemoryCardImage[1024];
char DVDRootPath[1024];
char LoadedGameFile[1024];

////////////////////////////////////////////////////////////
void Read_INI()
{
	char	INIFile[1024];

	sprintf(INIFile, "%sGekko.ini", ProgramDirectory);

	////read the ini settings and set the menus accordingly
	//DisableHLEPatches = GetPrivateProfileInt((LPCWSTR)"Debug", (LPCWSTR)"Disable HLE Patches", 0, (LPCWSTR)INIFile);
	//if(DisableHLEPatches)
	//{
	//	CheckMenuItem(GetMenu(wnd.hWnd), ID_OPTIONS_DISABLEHLE, MF_CHECKED);
	//	printf("Disabling HLE Patches\n");
	//}

	//DisableINIPatches = GetPrivateProfileInt((LPCWSTR)"Debug", (LPCWSTR)"Disable INI Patches", 0, (LPCWSTR)INIFile);
	//if(DisableINIPatches)
	//{
	//	CheckMenuItem(GetMenu(wnd.hWnd), ID_OPTIONS_DISABLEINIPATCHES, MF_CHECKED);
	//	printf("Disabling INI Patches\n");
	//}

	//if(GetPrivateProfileInt((LPCWSTR)"System", (LPCWSTR)"DynaRec", 0, (LPCWSTR)INIFile) == 1)
	if (1)
	{
		delete cpu;
		cpu = new GekkoCPURecompiler();
		CheckMenuItem(GetMenu(wnd.hWnd), ID_SYSTEM_DYNAREC, MF_CHECKED);
	}

	//cpu->DumpOp0 =  GetPrivateProfileInt((LPCWSTR)"Debug", (LPCWSTR)"Dump Opcode 0", 0, (LPCWSTR)INIFile);
	//if(cpu->DumpOp0)
	//{
	//	CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUG_DUMPOPCODE0, MF_CHECKED);
	//	printf("Enabling Opcode 0 dump\n");
	//}

	//cpu->PauseOnUnknownOp =  GetPrivateProfileInt((LPCWSTR)"Debug", (LPCWSTR)"Pause On Unknown Opcode", 0, (LPCWSTR)INIFile);
	//if(cpu->PauseOnUnknownOp)
	//{
	//	CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUG_PAUSEONUNKNOWNOPCODE, MF_CHECKED);
	//	printf("Enabling pause on unknown opcode\n");
	//}

	//cfg.enb_framebuffer = GetPrivateProfileInt((LPCWSTR)"System", (LPCWSTR)"Framebuffer", 0, (LPCWSTR)INIFile);
	//cfg.enb_framebuffer = !cfg.enb_framebuffer; // toggle
	//CFG_Enable_Framebuffer(); // toggle back (updates menu)

	//cfg.enb_autostart   = GetPrivateProfileInt((LPCWSTR)"Emulation", (LPCWSTR)"AutoStart", 0, (LPCWSTR)INIFile);
 // cfg.enb_autostart = !cfg.enb_autostart;
	//CFG_Enable_AutoStart(); // toggle back (updates menu)

	//DumpGCMBlockReads =  GetPrivateProfileInt((LPCWSTR)"Debug", (LPCWSTR)"Dump GCM Reads", 0, (LPCWSTR)INIFile);
	//if(DumpGCMBlockReads)
	//{
	//	CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUGOPTIONS_DUMPBLOCKREADS, MF_CHECKED);
	//	printf("Dumping GCM Reads\n");
	//}

	//memset(DumpDirectory, 0, sizeof(DumpDirectory));
	//GetPrivateProfileString((LPCWSTR)"Debug", (LPCWSTR)"Dump Directory", 0, (LPWSTR)DumpDirectory, 1024, (LPCWSTR)INIFile);	

	//memset(g_szRomPath, 0, sizeof(g_szRomPath));
	//GetPrivateProfileString((LPCWSTR)"System", (LPCWSTR)"ROM Path", 0, (LPWSTR)g_szRomPath, sizeof(g_szRomPath), (LPCWSTR)INIFile);
	//BrowserReloadWindow();

	//DisplayFPS = GetPrivateProfileInt((LPCWSTR)"System", (LPCWSTR)"Display FPS", 0, (LPCWSTR)INIFile);
	//if(DisplayFPS)
	//	CheckMenuItem(GetMenu(wnd.hWnd), ID_DEBUGOPTIONS_DISPLAYFRAMERATE, MF_CHECKED);

	//DVDRootPath[0] = 0;
	//GetPrivateProfileString((LPCWSTR)"ELF", (LPCWSTR)"DVDRoot", 0, (LPWSTR)DVDRootPath, 1024, (LPCWSTR)INIFile);	
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

#ifdef _WIN32
#undef main
#endif

////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
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

	GetModuleFileName(NULL, (LPWSTR)ProgramDirectory, sizeof(ProgramDirectory));
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

	//CFG_Init(); // Auto Configure basic settings.
	Read_INI();
	Init_CRC32_Table();
	OPENGL_Create(NULL);
	StartGame("G:\\Software\\Games\\Nintendo.Gamecube\\ww-demo.gcm");

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
	dvd::RealDVDClose(-1);

	delete cpu;
	Memory_Close();

	gx_fifo::destroy();

	return E_OK;
}
