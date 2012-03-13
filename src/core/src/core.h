/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    core.h
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

#ifndef CORE_CORE_H_
#define CORE_CORE_H_

////////////////////////////////////////////////////////////////////////////////////////////////////

#include "common.h"
#include "config.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace core {

// State of the full emulator
typedef enum {
    SYS_NULL = 0,   ///< System is in null state, nothing initialized
    SYS_IDLE,       ///< System is in an initialized state, but not running
    SYS_RUNNING,    ///< System is running
    SYS_LOADING,    ///< System is loading a ROM
    SYS_HALTED,     ///< System is halted (error)
    SYS_STALLED,    ///< System is stalled (unused)
    SYS_DEBUG,      ///< System is in a special debug mode (unused)
    SYS_DIE         ///< System is shutting down
} SystemState;

/*!
 * \brief Sets the system state
 * \param state SystemState type to set state to
 */
void SetState(SystemState state);

/*!
 * \brief Set the system configuration manager
 * \param config_manager Pointer to common::ConfigManager instance to set the config manager to
 * \todo shouldn't be fastcall (gekko-qt workaround)
 */
void EMU_FASTCALL SetConfigManager(common::ConfigManager* config_manager);

/// Start the core
// TODO: EMU_FASTCALL shouldn't be necessary (gekko-qt workaround)
void EMU_FASTCALL Start();

/// Kill the core
// TODO: EMU_FASTCALL shouldn't be necessary (gekko-qt workaround)
void EMU_FASTCALL Kill();

/// Stop the core
// TODO: EMU_FASTCALL shouldn't be necessary (gekko-qt workaround)
void EMU_FASTCALL Stop();

/// Initialize the core
// TODO: EMU_FASTCALL shouldn't be necessary (gekko-qt workaround)
int EMU_FASTCALL Init();

extern common::ConfigManager*   g_config_manager;           ///< Global system configuration manager
extern SystemState              g_state;                    ///< State of the emulator
extern bool                     g_started;      ///< Whether or not the emulator has been started

}; // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // CORE_CORE_H_