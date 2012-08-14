/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    core.cpp
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-02-11
 * \brief   Controls the whole emulation core
 *
 * \section LICENSE
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */

#include "common.h"
#include "config.h"
#include "crc.h"

#include "input_common.h"

#include "core.h"
#include "memory.h"
#include "hw/hw.h"
#include "dvd/realdvd.h"
#include "powerpc/cpu_core.h"
#include "powerpc/interpreter/cpu_int.h"
#include "powerpc/recompiler/cpu_rec.h"

// TODO: Include logic is stupid... video_core shouldn't be included if USE_NEW_VIDEO_CORE is false, but that variable is defined in that header..
#include "video_core.h"
#ifndef USE_NEW_VIDEO_CORE
#include "video/opengl.h"
#include "video/gx_fifo.h"
#endif

#include "dvd/loader.h"

namespace core {

common::Config*         g_config;
common::ConfigManager*  g_config_manager;
SystemState             g_state    = SYS_NULL;  ///< State of the emulator
bool                    g_started  = false;     ///< Whether or not the emulator has been started

/// Sets the system state
void EMU_FASTCALL SetState(SystemState state) {
    static const char state_labels[9] = "-IRLHSDD";
    g_state = state;
    LOG_DEBUG(TCORE, "g_state = %x [%c]", state, state_labels[state]);
}

// Set the system configuration manager
void SetConfigManager(common::ConfigManager* config_manager) {
    g_config_manager = config_manager;
    LOG_NOTICE(TCORE, "config manager set ok");
}

/// Start the core
void Start(EmuWindow* emu_window) {
    SetState(SYS_RUNNING);
#ifdef USE_NEW_VIDEO_CORE
    video_core::Start(emu_window);
#endif // USE_NEW_VIDEO_CORE
}

/// Kill the core
void Kill() {
    Flipper_Close();
	dvd::RealDVDClose(-1);
	delete cpu;
    cpu = NULL;
	Memory_Close();
// TODO: Do anything about new video core here?
#ifndef USE_NEW_VIDEO_CORE
	gx_fifo::destroy();
#endif
}

/// Stop the core
void Stop() {
    if(cpu->is_on == true) {
        cpu->Halt();        // Stop CPU
        Flipper_Close();    // Stop Hardware
    }
}

// Initialize the core
int Init(EmuWindow* emu_window) {
    logger::Init();
    Memory_Open();          // Init main memory
    Init_CRC32_Table();     // Init CRC table
    input_common::Init(emu_window);   // Init user input plugin

    if (common::g_config->powerpc_core() == common::Config::CPU_INTERPRETER) {
        delete cpu; // TODO: STUPID!
        cpu = new GekkoCPUInterpreter();
    } else {

#ifndef EMU_IGNORE_RECOMPILER

#ifdef EMU_ARCHITECTURE_X86
        delete cpu;
       // cpu = new GekkoCPURecompiler();
#else
        LOG_ERROR(TCORE, "Recompiler only x86 - Please switch your configuration to the interpreter!\n");
        return E_ERR;
#endif // EMU_ARCHITECTURE_X86

#else
        LOG_ERROR(TCORE, "Recompiler removed from this build - Please switch your configuration to the interpreter!\n");
        return E_ERR;
#endif // EMU_IGNORE_RECOMPILER

    }
	SetState(SYS_IDLE);
    g_started = false;
	
#ifndef USE_NEW_VIDEO_CORE
    OPENGL_Create();
#endif // !USE_NEW_VIDEO_CORE

    return E_OK;
}

} // namespace

