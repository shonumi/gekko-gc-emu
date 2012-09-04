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

    enum GCControllerDirection {
        STICK_UP = 0,               ///< Up direction on controller stick/dpad
        STICK_DOWN,                 ///< Down direction on controller stick/dpad
        STICK_LEFT,                 ///< Left direction on controller stick/dpad
        STICK_RIGHT,                ///< Right direction on controller stick/dpad
        NUM_OF_DIRECTIONS           ///< Number of directions - should be last
    };

    GCController() {
        a_status_ = RELEASED;
        b_status_ = RELEASED;
        x_status_ = RELEASED;
        y_status_ = RELEASED;
        l_status_ = RELEASED;
        r_status_ = RELEASED;
        start_status_ = RELEASED;
        analog_stick_status_[STICK_UP] = RELEASED;
        analog_stick_status_[STICK_DOWN] = RELEASED;
        analog_stick_status_[STICK_LEFT] = RELEASED;
        analog_stick_status_[STICK_RIGHT] = RELEASED;
        c_stick_status_[STICK_UP] = RELEASED;
        c_stick_status_[STICK_DOWN] = RELEASED;
        c_stick_status_[STICK_LEFT] = RELEASED;
        c_stick_status_[STICK_RIGHT] = RELEASED;
        dpad_status_[STICK_UP] = RELEASED;
        dpad_status_[STICK_DOWN] = RELEASED;
        dpad_status_[STICK_LEFT] = RELEASED;
        dpad_status_[STICK_RIGHT] = RELEASED;
    }
    ~GCController() { }

    GCButtonState a_status() { return a_status_; }
    void set_a_status(GCButtonState val) { a_status_= val; }
    GCButtonState b_status() { return b_status_; }
    void set_b_status(GCButtonState val) { b_status_= val; }
    GCButtonState x_status() { return x_status_; }
    void set_x_status(GCButtonState val) { x_status_= val; }
    GCButtonState y_status() { return y_status_; }
    void set_y_status(GCButtonState val) { y_status_= val; }
    GCButtonState z_status() { return z_status_; }
    void set_z_status(GCButtonState val) { z_status_= val; }
    GCButtonState l_status() { return l_status_; }
    void set_l_status(GCButtonState val) { l_status_= val; }
    GCButtonState r_status() { return r_status_; }
    void set_r_status(GCButtonState val) { r_status_= val; }
    GCButtonState start_status() { return start_status_; }
    void set_start_status(GCButtonState val) { start_status_= val; }

    GCButtonState analog_stick_status(GCControllerDirection dir) { 
        return analog_stick_status_[dir];
    }
    void set_analog_stick_status(GCControllerDirection dir, GCButtonState val) {
        analog_stick_status_[dir] = val;
    }

    GCButtonState c_stick_status(GCControllerDirection dir) { 
        return c_stick_status_[dir];
    }
    void set_c_stick_status(GCControllerDirection dir, GCButtonState val) {
        c_stick_status_[dir] = val;
    }

    GCButtonState dpad_status(GCControllerDirection dir) { return dpad_status_[dir]; }
    void set_dpad_status(GCControllerDirection dir, GCButtonState val) { dpad_status_[dir] = val; }

private:
    GCButtonState a_status_;
    GCButtonState b_status_;
    GCButtonState x_status_;
    GCButtonState y_status_;
    GCButtonState z_status_;
    GCButtonState l_status_;
    GCButtonState r_status_;
    GCButtonState start_status_;

    GCButtonState analog_stick_status_[NUM_OF_DIRECTIONS];
    GCButtonState c_stick_status_[NUM_OF_DIRECTIONS];
    GCButtonState dpad_status_[NUM_OF_DIRECTIONS];

    DISALLOW_COPY_AND_ASSIGN(GCController);
};

} // namespace

#endif // INPUT_COMMON_GC_CONTROLLER_

