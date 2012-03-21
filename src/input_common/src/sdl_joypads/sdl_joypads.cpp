/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    sdl_joypads.cpp
 * \author  Shonumi <shonumi@gmail.com>
 * \date    2012-03-12
 * \brief   Implementation of a SDL joypad GC controller interface
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
#include "sdl_joypads.h"

namespace input_common {

/// Sets the controller status from the joypad using SDL
void SDLJoypads::SetControllerStatus(int channel, int pad,
        GCController::GCButtonState state) {
    // Buttons
    if (pad == common::g_config->controller_ports(channel).pads.start_key_code) {
        g_controller_state[channel]->set_start_status(state);
    } else if (pad == common::g_config->controller_ports(channel).pads.a_key_code) {
        g_controller_state[channel]->set_a_status(state);
    } else if (pad == common::g_config->controller_ports(channel).pads.b_key_code) {
        g_controller_state[channel]->set_b_status(state);
    } else if (pad == common::g_config->controller_ports(channel).pads.x_key_code) {
        g_controller_state[channel]->set_x_status(state);
    } else if (pad == common::g_config->controller_ports(channel).pads.y_key_code) {
        g_controller_state[channel]->set_y_status(state);
    } else if (pad == common::g_config->controller_ports(channel).pads.l_key_code) {
        g_controller_state[channel]->set_l_status(state);
    } else if (pad == common::g_config->controller_ports(channel).pads.r_key_code) {
        g_controller_state[channel]->set_r_status(state);
    } else if (pad == common::g_config->controller_ports(channel).pads.z_key_code) {
        g_controller_state[channel]->set_z_status(state);
        
    // Analog stick
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_up_key_code) {
        g_controller_state[channel]->set_analog_stick_status(GCController::STICK_UP, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_down_key_code) {
       g_controller_state[channel]->set_analog_stick_status(GCController::STICK_DOWN, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_left_key_code) {
        g_controller_state[channel]->set_analog_stick_status(GCController::STICK_LEFT, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_right_key_code) {
        g_controller_state[channel]->set_analog_stick_status(GCController::STICK_RIGHT, state);

    // C stick
    } else if (pad == common::g_config->controller_ports(channel).pads.c_up_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_UP, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.c_down_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_DOWN, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.c_left_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_LEFT, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.c_right_key_code) {
        g_controller_state[channel]->set_c_stick_status(GCController::STICK_RIGHT, state); 

    // D-pad
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_up_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_UP, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_down_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_DOWN, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_left_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_LEFT, state);
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_right_key_code) {
        g_controller_state[channel]->set_dpad_status(GCController::STICK_RIGHT, state);
    }   
}

/// Gets the controller status from the joypad using SDL
GCController::GCButtonState SDLJoypads::GetControllerStatus(int channel, int pad) {
    // Buttons
    if (pad == common::g_config->controller_ports(channel).pads.start_key_code) {
        return g_controller_state[channel]->start_status();
    } else if (pad == common::g_config->controller_ports(channel).pads.a_key_code) {
        return g_controller_state[channel]->a_status();
    } else if (pad == common::g_config->controller_ports(channel).pads.b_key_code) {
        return g_controller_state[channel]->b_status();
    } else if (pad == common::g_config->controller_ports(channel).pads.x_key_code) {
        return g_controller_state[channel]->x_status();
    } else if (pad == common::g_config->controller_ports(channel).pads.y_key_code) {
        return g_controller_state[channel]->y_status();
    } else if (pad == common::g_config->controller_ports(channel).pads.l_key_code) {
        return g_controller_state[channel]->l_status();
    } else if (pad == common::g_config->controller_ports(channel).pads.r_key_code) {
        return g_controller_state[channel]->r_status();
    } else if (pad == common::g_config->controller_ports(channel).pads.z_key_code) {
        return g_controller_state[channel]->z_status();
        
    // Analog stick
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_up_key_code) {
        return g_controller_state[channel]->analog_stick_status(GCController::STICK_UP);
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_down_key_code) {
       return g_controller_state[channel]->analog_stick_status(GCController::STICK_DOWN);
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_left_key_code) {
        return g_controller_state[channel]->analog_stick_status(GCController::STICK_LEFT);
    } else if (pad == common::g_config->controller_ports(channel).pads.analog_right_key_code) {
        return g_controller_state[channel]->analog_stick_status(GCController::STICK_RIGHT);

    // C stick
    } else if (pad == common::g_config->controller_ports(channel).pads.c_up_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_UP);
    } else if (pad == common::g_config->controller_ports(channel).pads.c_down_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_DOWN);
    } else if (pad == common::g_config->controller_ports(channel).pads.c_left_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_LEFT);
    } else if (pad == common::g_config->controller_ports(channel).pads.c_right_key_code) {
        return g_controller_state[channel]->c_stick_status(GCController::STICK_RIGHT); 

    // D-pad
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_up_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_UP);
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_down_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_DOWN);
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_left_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_LEFT);
    } else if (pad == common::g_config->controller_ports(channel).pads.dpad_right_key_code) {
        return g_controller_state[channel]->dpad_status(GCController::STICK_RIGHT);
    }

    // Return GC_CONTROLLER_NULL if unmapped pad
    return GCController::GC_CONTROLLER_NULL;   
}

/// Checks whether joystick is in dead zone or not
bool SDLJoypads::DeadZone(int val) {
    if((val < -8000) || (val > 8000)) {
        return false;
    } else if((val > -8000) && (val < 8000)) {
        return true;
    }
}

/// Poll for joypad input
void SDLJoypads::PollEvent() {
    SDL_Event joyevent;

    while (SDL_PollEvent(&joyevent)) {

        //Unique number to id joypad input
        int pad = 0;
        int val = 0;

        switch(joyevent.type) {

        // Handle joystick button press events
        case SDL_JOYBUTTONDOWN:
            pad = 100;
            pad += joyevent.jbutton.button;
            SetControllerStatus(0, pad, GCController::PRESSED);
            break;
        // Handle joystick button release events
        case SDL_JOYBUTTONUP:
            pad = 100;
            pad += joyevent.jbutton.button;
            SetControllerStatus(0, pad, GCController::RELEASED);
            break;
        // Handle joystick axis motion events - Super simplified :)
        case SDL_JOYAXISMOTION:
            pad = 200;
            pad += (joyevent.jaxis.axis * 2);
            val = joyevent.jaxis.value; 

            if(val > 0) { 
                pad += 1; 
            }

            // Check for joy axis input on corresponding pads
            if((GetControllerStatus(0, pad) == GCController::RELEASED) && (!DeadZone(val))) {
                SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((GetControllerStatus(0, pad) == GCController::PRESSED) && (DeadZone(val))) {
                SetControllerStatus(0, pad, GCController::RELEASED);
            }
            break;
        // Handle joystick hat motion events
        case SDL_JOYHATMOTION:
            pad = 300;
            pad += joyevent.jhat.hat * 4;

            // Check for joy hat input on corresponding pads
            // This _should_ work for multiple hats, untested though
            if(joyevent.jhat.value == SDL_HAT_LEFT) {
                SetControllerStatus(0, pad, GCController::PRESSED);
                SetControllerStatus(0, pad+1, GCController::RELEASED);
                SetControllerStatus(0, pad+2, GCController::RELEASED);
                SetControllerStatus(0, pad+3, GCController::RELEASED);
            } else if(joyevent.jhat.value == SDL_HAT_LEFTUP) {
                SetControllerStatus(0, pad, GCController::PRESSED);
                SetControllerStatus(0, pad+1, GCController::RELEASED);
                SetControllerStatus(0, pad+2, GCController::PRESSED);
                SetControllerStatus(0, pad+3, GCController::RELEASED);
            } else if(joyevent.jhat.value == SDL_HAT_LEFTDOWN) {
                SetControllerStatus(0, pad, GCController::PRESSED);
                SetControllerStatus(0, pad+1, GCController::RELEASED);
                SetControllerStatus(0, pad+2, GCController::RELEASED);
                SetControllerStatus(0, pad+3, GCController::PRESSED);
            } else if(joyevent.jhat.value == SDL_HAT_RIGHT) {
                SetControllerStatus(0, pad, GCController::RELEASED);
                SetControllerStatus(0, pad+1, GCController::PRESSED);
                SetControllerStatus(0, pad+2, GCController::RELEASED);
                SetControllerStatus(0, pad+3, GCController::RELEASED);
            } else if(joyevent.jhat.value == SDL_HAT_RIGHTUP) {
                SetControllerStatus(0, pad, GCController::RELEASED);
                SetControllerStatus(0, pad+1, GCController::PRESSED);
                SetControllerStatus(0, pad+2, GCController::PRESSED);
                SetControllerStatus(0, pad+3, GCController::RELEASED);
            } else if(joyevent.jhat.value == SDL_HAT_RIGHTDOWN) {
                SetControllerStatus(0, pad, GCController::RELEASED);
                SetControllerStatus(0, pad+1, GCController::PRESSED);
                SetControllerStatus(0, pad+2, GCController::RELEASED);
                SetControllerStatus(0, pad+3, GCController::PRESSED);
            } else if(joyevent.jhat.value == SDL_HAT_UP) {
                SetControllerStatus(0, pad, GCController::RELEASED);
                SetControllerStatus(0, pad+1, GCController::RELEASED);
                SetControllerStatus(0, pad+2, GCController::PRESSED);
                SetControllerStatus(0, pad+3, GCController::RELEASED);
            } else if(joyevent.jhat.value == SDL_HAT_DOWN) {
                SetControllerStatus(0, pad, GCController::RELEASED);
                SetControllerStatus(0, pad+1, GCController::RELEASED);
                SetControllerStatus(0, pad+2, GCController::RELEASED);
                SetControllerStatus(0, pad+3, GCController::PRESSED);
            } else if(joyevent.jhat.value == SDL_HAT_CENTERED) {
                SetControllerStatus(0, pad, GCController::RELEASED);
                SetControllerStatus(0, pad+1, GCController::RELEASED);
                SetControllerStatus(0, pad+2, GCController::RELEASED);
                SetControllerStatus(0, pad+3, GCController::RELEASED);
            }
            break;
        }
    }
}

void SDLJoypads::ShutDown() {
    SDL_JoystickClose(0);
}

// Initialize the joypad - Only init 1, for testing now
bool SDLJoypads::Init() {
    //Initalize SDL joypad subsystem first of all
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);

    // TODO - Initialize multiple joypads
    jpad = NULL;
    jpad = SDL_JoystickOpen(0);

    //Check to see if joypad was not opened
    if(jpad == NULL) {
        LOG_NOTICE(TJOYPAD, "\"SDL joypads\" input plugin not initialized");
        return false;
    } else if(jpad != NULL) {
        LOG_NOTICE(TJOYPAD, "Joypad detected");
        LOG_NOTICE(TJOYPAD, SDL_JoystickName(0));
        LOG_NOTICE(TJOYPAD, "\"SDL joypads\" input plugin initialized ok");
        return true;
    }
}


} // namespace