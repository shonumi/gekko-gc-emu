// emu.h
// (c) 2005,2006 Gekko Team

#ifndef _EMU_H_
#define _EMU_H_

////////////////////////////////////////////////////////////

#define APP_NAME		"Gekko GameCube Emulator"
#define APP_VERS		"0.2.5"
#define TITLE			"High Level"
#define	COPYRIGHT		"Copyright (C) 2005-2006 Gekko Team"

////////////////////////////////////////////////////////////

#include "general.h"

////////////////////////////////////////////////////////////

typedef struct t_emuMode
{
	u32		status;
	u32		cpu_opt;
	u32		other;

	BOOL	started;

	u8		flags;

	BOOL	keys[256];
	BOOL	compkeys[256];
	BOOL	keychange;
}emuMode;

extern emuMode	emu;
extern char		ProgramDirectory[1024];
extern char		LoadedGameFile[1024];

extern u32		DumpGCMBlockReads;
extern char		DumpDirectory[1024];
extern u32		DisplayFPS;
extern char		MemoryCardImage[1024];
extern char		DVDRootPath[1024];

////////////////////////////////////////////////////////////

#define EMU_QUIT				0
#define EMU_RUNNING				1
#define EMU_LOAD				2
#define EMU_HALTED				3
#define EMU_STALLED				7
#define EMU_FLAG_DEBUG			0x8

#define CPU_INTERP				1
#define CPU_DYNAREC				2

#define E_OK					0
#define E_ERR					1

////////////////////////////////////////////////////////////

#ifdef _DEBUG
	#ifndef _DEBUGSPEED
		#define DEBUG_EMU
		#define DEBUG_GX
	#endif
#endif

#ifdef DEBUG_EMU
#define ASSERT_T(cond,str) if((cond)) printf("#!\tERROR: ASSERTION FAILED: %s !\n", str);
#define ASSERT_F(cond,str) if(!(cond)) printf("#!\tERROR: ASSERTION FAILED: %s !\n", str);
#else
#define ASSERT_T(cond,str)
#define ASSERT_F(cond,str)
#endif

////////////////////////////////////////////////////////////

#include "resource.h"

#include "low level/flipper.h"
#include "memory.h"
#include "crc.h"
#include "opengl.h"

#include "cpu core/disassembler/ppc_disasm.h"
#include "cpu core/cpu_core.h"

#include "dvd interface/loaders.h"

#include "high level/highlevel.h"

#include "user interface/emu_ini.h"
#include "user interface/emu_browser.h"
#include "user interface/emu_config.h"
#include "user interface/emu_win.h"
#include "user interface/emu_dbg.h"

////////////////////////////////////////////////////////////

#define SINGLETHREADED

////////////////////////////////////////////////////////////

#endif