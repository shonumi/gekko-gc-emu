/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    keyboard_input.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-03
 * @brief   Implementation of a keyboard GC controller interface
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

#include <SDL.h>

#include "common.h"
#include "config.h"

#include "input_common.h"
#include "gc_controller.h"
#include "keyboard_input.h"
#include "video/emuwindow.h"

namespace input_common {

bool KeyboardInput::SetControllerStatus(int channel, int key, GCController::GCButtonState state)
{
    if (common::g_config->controller_ports(channel).enable == false ||
        common::g_config->controller_ports(channel).keys.enable == false)
        return false;

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
    } else {
        return false;
    }

    return true;
}

GCController::GCButtonState KeyboardInput::GetControllerStatus(int channel, int key) {
    // Buttons
    if (key == common::g_config->controller_ports(channel).keys.start_key_code) {
        return g_controller_state[channel]->start_status();
    } else if (key == common::g_config->controller_ports(channel).keys.a_key_code) {
        return g_controller_state[channel]->a_status();
    } else if (key == common::g_config->controller_ports(channel).keys.b_key_code) {
        return g_controller_state[channel]->b_status();
    } else if (key == common::g_config->controller_ports(channel).keys.x_key_code) {
        return g_controller_state[channel]->x_status();
    } else if (key == common::g_config->controller_ports(channel).keys.y_key_code) {
        return g_controller_state[channel]->y_status();
    } else if (key == common::g_config->controller_ports(channel).keys.l_key_code) {
        return g_controller_state[channel]->l_status();
    } else if (key == common::g_config->controller_ports(channel).keys.r_key_code) {
        return g_controller_state[channel]->r_status();
    } else if (key == common::g_config->controller_ports(channel).keys.z_key_code) {
        return g_controller_state[channel]->z_status();
        
    // Analog stick
    } else if (key == common::g_config->controller_ports(channel).keys.analog_up_key_code) {
        return g_controller_state[channel]->analog_stick_status(GCController::STICK_UP);
    } else if (key == common::g_config->controller_ports(channel).keys.analog_down_key_code) {
       return g_controller_state[channel]->analog_stick_status(GCController::STICK_DOWN);
    } else if (key == common::g_config->controller_ports(channel).keys.analog_left_key_code) {
        return g_controller_state[channel]->analog_stick_status(GCController::STICK_LEFT);
    } else if (key == common::g_config->controller_ports(channel).keys.analog_right_key_code) {
        return g_controller_state[channel]->analog_stick_status(GCController::STICK_RIGHT);

    // C stick
    } else if (key == common::g_config->controller_ports(channel).keys.c_up_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_UP);
    } else if (key == common::g_config->controller_ports(channel).keys.c_down_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_DOWN);
    } else if (key == common::g_config->controller_ports(channel).keys.c_left_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_LEFT);
    } else if (key == common::g_config->controller_ports(channel).keys.c_right_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_RIGHT); 

    // D-pad
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_up_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_UP);
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_down_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_DOWN);
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_left_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_LEFT);
    } else if (key == common::g_config->controller_ports(channel).keys.dpad_right_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_RIGHT);
    }

    // Return GC_CONTROLLER_NULL if unmapped pad
    return GCController::GC_CONTROLLER_NULL;   
}

void KeyboardInput::PollEvents() {
    emuwindow_->PollEvents();
}

void KeyboardInput::ShutDown() {
}

bool KeyboardInput::Init() {
    return true;
}

KeyboardInput::KeyboardInput(EmuWindow* emu_window) : emuwindow_(emu_window)
{
    emuwindow_->SetControllerInterface(this);
}

KeyboardInput::~KeyboardInput()
{
    emuwindow_->SetControllerInterface(NULL);
}



} // namespace
