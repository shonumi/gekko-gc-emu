/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    pcafe.cpp
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-02-11
 * \brief   Main entry point
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
#include "xml.h"
#include "x86_utils.h"

#include "core.h"
#include "dvd/loader.h"
#include "powerpc/cpu_core.h"
#include "hw/hw.h"
#include "video_core.h"

#ifndef USE_NEW_VIDEO_CORE
#include "video/opengl.h"
#endif
#include "emuwindow/emuwindow_glfw.h"

#include "gekko.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// This is needed to fix SDL in certain build environments
#ifdef main
#undef main
#endif

//#define PLAY_FIFO_RECORDING 1

/// Application entry point
int __cdecl main(int argc, char **argv)
{
    u32 tight_loop;
    LOG_NOTICE(TMASTER, APP_NAME " starting...\n");

    char program_dir[MAX_PATH];
    _getcwd(program_dir, MAX_PATH-1);
    size_t cwd_len = strlen(program_dir);
    program_dir[cwd_len] = '/';
    program_dir[cwd_len+1] = '\0';

    common::ConfigManager config_manager;
    config_manager.set_program_dir(program_dir, MAX_PATH);
    config_manager.ReloadConfig(NULL);
    core::SetConfigManager(&config_manager);

    EmuWindow_GLFW* emu_window = new EmuWindow_GLFW;

    if (E_OK != core::Init(emu_window)) {
        LOG_ERROR(TMASTER, "core initialization failed, exiting...");
        core::Kill();
        exit(1);
    }

    // TODO: Remove this once ppl implement proper controller config
    // Overrides the configuration from XML (which likely contains SDL-specific keycodes) with proper GLFW keybindings
    common::g_config->controller_ports(0).keys.enable = true;
    common::g_config->controller_ports(0).keys.a_key_code = GLFW_KEY_X;
    common::g_config->controller_ports(0).keys.b_key_code = GLFW_KEY_Y; // QWERTZ ftw
    common::g_config->controller_ports(0).keys.x_key_code = GLFW_KEY_A;
    common::g_config->controller_ports(0).keys.y_key_code = GLFW_KEY_S;
    common::g_config->controller_ports(0).keys.l_key_code = GLFW_KEY_Q;
    common::g_config->controller_ports(0).keys.r_key_code = GLFW_KEY_W;
    common::g_config->controller_ports(0).keys.start_key_code = GLFW_KEY_ENTER;
    common::g_config->controller_ports(0).keys.analog_left_key_code = GLFW_KEY_LEFT;
    common::g_config->controller_ports(0).keys.analog_right_key_code = GLFW_KEY_RIGHT;
    common::g_config->controller_ports(0).keys.analog_up_key_code = GLFW_KEY_UP;
    common::g_config->controller_ports(0).keys.analog_down_key_code = GLFW_KEY_DOWN;

    // Load a game or die...
    if (E_OK == dvd::LoadBootableFile(common::g_config->default_boot_file())) {
        if (common::g_config->enable_auto_boot()) {
            core::Start(emu_window);
        } else {
            LOG_ERROR(TMASTER, "Autoboot required in no-GUI mode... Exiting!\n");
        }
    } else {
        LOG_ERROR(TMASTER, "Failed to load a bootable file... Exiting!\n");
        exit(E_ERR);
    }

    while(core::SYS_DIE != core::g_state) {
        if (core::SYS_RUNNING == core::g_state) {
            if(!cpu->is_on) {
                cpu->Start(); // Initialize and start CPU.
            } else {
                for(tight_loop = 0; tight_loop < 10000; ++tight_loop) {
                    cpu->execStep();
                }
            }
        } else if (core::SYS_HALTED == core::g_state) {
            core::Stop();
        }
    }
    core::Kill();
    delete emu_window;

	return E_OK;
}
