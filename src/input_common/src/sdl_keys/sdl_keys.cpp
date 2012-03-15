/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    sdl_keys.cpp
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-03
 * \brief   Implementation of a SDL keyboard GC controller interface
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

#include <SDL.h>

#include "common.h"
#include "config.h"

#include "input_common.h"
#include "gc_controller.h"
#include "sdl_keys.h"

namespace input_common {

/// Sets the controller status from the keyboard using SDL
void SDLKeys::SetControllerStatus(int channel, u16 key,
        GCController::GCButtonState state) {
    // Buttons
    if (key == common::g_config->controller_ports(channel).keys.start_key_code) {
        g_controller_state[channel]->set_start_status(state);
    } else if (key == common::g_config->controller_ports(channel).keys.a_key_code) {
        g_controller_state[channel]->set_a_status(state);
    } else if (key == common::g_config->controller_ports(channel).keys.b_key_code) {
        g_controller_state[channel]->set_b_status(state);
    } else if (key == common::g_config->controller_ports(channel).keys.x_key_code) {
        g_controller_state[channel]->set_x_status(state);
    } else if (key == common::g_config->controller_ports(channel).keys.y_key_code) {
        g_controller_state[channel]->set_y_status(state);
    } else if (key == common::g_config->controller_ports(channel).keys.l_key_code) {
        g_controller_state[channel]->set_l_status(state);
    } else if (key == common::g_config->controller_ports(channel).keys.r_key_code) {
        g_controller_state[channel]->set_r_status(state);
    } else if (key == common::g_config->controller_ports(channel).keys.z_key_code) {
        g_controller_state[channel]->set_z_status(state);
        
    // Analog stick
    } else if (key == common::g_config->controller_ports(channel).keys.analog_up_key_code) {
        g_controller_state[channel]->set_analog_stick_status(GCController::STICK_UP, state);
    } else if (key == common::g_config->controller_ports(channel).keys.analog_down_key_code) {
        g_controller_state[channel]->set_analog_stick_status(GCController::STICK_DOWN, state);
    } else if (key == common::g_config->controller_ports(channel).keys.analog_left_key_code) {
        g_controller_state[channel]->set_analog_stick_status(GCController::STICK_LEFT, state);
    } else if (key == common::g_config->controller_ports(channel).keys.analog_right_key_code) {
        g_controller_state[channel]->set_analog_stick_status(GCController::STICK_RIGHT, state);

    // C stick
    } else if (key == common::g_config->controller_ports(channel).keys.c_up_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_UP, state);
    } else if (key == common::g_config->controller_ports(channel).keys.c_down_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_DOWN, state);
    } else if (key == common::g_config->controller_ports(channel).keys.c_left_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_LEFT, state);
    } else if (key == common::g_config->controller_ports(channel).keys.c_right_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_RIGHT, state);

    // D-pad
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_up_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_UP, state);
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_down_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_DOWN, state);
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_left_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_LEFT, state);
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_right_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_RIGHT, state);
    }  
}

/// Poll for key presses
void SDLKeys::PollEvent() {
    SDL_Event keyevent;

    while (SDL_PollEvent(&keyevent)) {
        switch(keyevent.type) {

        // Handle controller button press events
        case SDL_KEYDOWN:
            SetControllerStatus(0, keyevent.key.keysym.sym, GCController::PRESSED);
            break;
        // Handle controller button release events
        case SDL_KEYUP:
            SetControllerStatus(0, keyevent.key.keysym.sym, GCController::RELEASED);
            break;
        }
    }
}

void SDLKeys::ShutDown() {
}

/// Any initialization goes here...
bool SDLKeys::Init() {
    LOG_NOTICE(TJOYPAD, "\"SDL keys\" input plugin initialized ok");
    return true;
}


} // namespace