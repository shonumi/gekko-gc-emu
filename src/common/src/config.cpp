/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    config.cpp
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-02-19
 * \brief   Emulator configuration class - all config settings stored here
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

namespace common {

Config* g_config;

/*!
 * \brief Reload a game-specific configuration
 * \param id Game id (to load game specific configuration)
 */
void ConfigManager::ReloadGameConfig(const char* id) {
    char full_filename[MAX_PATH];
    sprintf_s(full_filename, MAX_PATH, "user/games/%s.xml", id);
    common::LoadXMLConfig(*g_config, full_filename);
}

/// Reload the userconfig file
void ConfigManager::ReloadUserConfig() {
    common::LoadXMLConfig(*g_config, "userconf.xml");
}

/// Reload the sysconfig file
void ConfigManager::ReloadSysConfig() {
    common::LoadXMLConfig(*g_config, "sysconf.xml");
}

/// Reload all configurations
void ConfigManager::ReloadConfig(const char* game_id) {
    delete g_config;
	g_config = new Config();
    g_config->set_program_dir(program_dir_, MAX_PATH);
    ReloadSysConfig();
    ReloadUserConfig();
    ReloadGameConfig(game_id);
}

} // namspace