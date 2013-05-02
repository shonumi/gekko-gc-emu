/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    gc_controller.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-03
 * @brief   GC controller state for use with input plugins
 *
 * @section LICENSE
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

#ifndef INPUT_COMMON_GC_CONTROLLER_
#define INPUT_COMMON_GC_CONTROLLER_

#include "common.h"
#include "config.h"

/// Macro for checking if a button is pressed
#define IS_GCBUTTON_PRESSED(val)  (val == input_common::GCController::PRESSED)

namespace input_common {

class GCController {
public:

    /// Used for contoller button state
    enum GCButtonState {
        GC_CONTROLLER_NULL = 0,     ///< Undefined state
        RELEASED,                   ///< Controller button is pressed
        PRESSED                     ///< COntroller button is released
    };

    GCController() {
        for (unsigned int i = 0; i < common::Config::NUM_CONTROLS; ++i)
            status_[i] = RELEASED;
    }
    ~GCController() { }

    GCButtonState control_status(common::Config::Control control) { return status_[control]; }
    void set_control_status(common::Config::Control control, GCButtonState val) { status_[control] = val; }

    void set_rumble_status(bool r_stat) { rumble_status = r_stat; }
    bool get_rumble_status() { return rumble_status; }

private:
    GCButtonState status_[common::Config::NUM_CONTROLS];
    bool rumble_status;

    DISALLOW_COPY_AND_ASSIGN(GCController);
};

} // namespace

#endif // INPUT_COMMON_GC_CONTROLLER_

