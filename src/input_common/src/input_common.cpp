/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    input_common.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-03
 * @brief   Base class for defining user input plugins
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

#include "input_common.h"
#include "gc_controller.h"
#include "sdl_keys/sdl_keys.h"
#include "sdl_joypads/sdl_joypads.h"

namespace input_common {

GCController*   g_controller_state[4];  ///< GC controller states of all button presses   
InputBase*      g_user_input;           ///< UserInput plugin pointer         

InputBase::InputBase() {
}

InputBase::~InputBase() {
}

bool InputBase::Init() {
    printf("InputBase::Init()\n");
    return true;
}

void InputBase::PollEvent() {
    printf("InputBase:PollEvent()\n");
}

void InputBase::ShutDown() {
    printf("InputBase::ShutDown()\n");
}

/// Initialize the user input system
void Init() {
    for (int i = 0; i < 4; i++) {
        delete g_controller_state[i];
        g_controller_state[i] = new GCController();
    }
    delete g_user_input;

    //Try to init joypads first
    g_user_input = new SDLJoypads();

    //If that fails, fallback on keyboard input
    if(!g_user_input->Init()) {
        delete g_user_input;
        g_user_input = new SDLKeys();
        g_user_input->Init();
    }
}

} // namespace