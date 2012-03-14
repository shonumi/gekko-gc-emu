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
    // ALL Values are hardcoded, just until the config file gets set up
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
        // Handle joystick axis motion events - PITA :/
        case SDL_JOYAXISMOTION:
            pad = 200;
            pad += (joyevent.jaxis.axis * 2);
            if(joyevent.jaxis.value > 0) { pad += 1; }
            
            int val = joyevent.jaxis.value; 

            // Check for Up Analog
            if((g_controller_state[0]->analog_stick_status(GCController::STICK_UP) == GCController::RELEASED)
                && (pad == common::g_config->controller_ports(0).pads.analog_up_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->analog_stick_status(GCController::STICK_UP) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.analog_up_key_code) && (DeadZone(val))){
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Down Analog
            if((g_controller_state[0]->analog_stick_status(GCController::STICK_DOWN) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.analog_down_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->analog_stick_status(GCController::STICK_DOWN) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.analog_down_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED); 
            }

            // Check for Left Analog
            if((g_controller_state[0]->analog_stick_status(GCController::STICK_LEFT) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.analog_left_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->analog_stick_status(GCController::STICK_LEFT) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.analog_left_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Right Analog
            if((g_controller_state[0]->analog_stick_status(GCController::STICK_RIGHT) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.analog_right_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->analog_stick_status(GCController::STICK_RIGHT) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.analog_right_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }
            
            // Check for Up C Stick
            if((g_controller_state[0]->c_stick_status(GCController::STICK_UP) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.c_up_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->c_stick_status(GCController::STICK_UP) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.c_up_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED); 
            }

            // Check for Down C Stick
            if((g_controller_state[0]->c_stick_status(GCController::STICK_DOWN) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.c_down_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->c_stick_status(GCController::STICK_DOWN) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.c_down_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Left C Stick
            if((g_controller_state[0]->c_stick_status(GCController::STICK_LEFT) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.c_left_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->c_stick_status(GCController::STICK_LEFT) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.c_left_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Right C Stick
            if((g_controller_state[0]->c_stick_status(GCController::STICK_RIGHT) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.c_right_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->c_stick_status(GCController::STICK_RIGHT) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.c_right_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Up D-Pad
            if((g_controller_state[0]->dpad_status(GCController::STICK_UP) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_up_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->dpad_status(GCController::STICK_UP) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_up_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Down D-Pad
            if((g_controller_state[0]->dpad_status(GCController::STICK_DOWN) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_down_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->dpad_status(GCController::STICK_DOWN) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_down_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Left D-pad
            if((g_controller_state[0]->dpad_status(GCController::STICK_LEFT) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_left_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->dpad_status(GCController::STICK_LEFT) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_left_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
            }

            // Check for Right D-Pad
            if((g_controller_state[0]->dpad_status(GCController::STICK_RIGHT) == GCController::RELEASED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_right_key_code) && (!DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::PRESSED);
            } else if((g_controller_state[0]->dpad_status(GCController::STICK_RIGHT) == GCController::PRESSED) 
                && (pad == common::g_config->controller_ports(0).pads.dpad_right_key_code) && (DeadZone(val))) {
                    SetControllerStatus(0, pad, GCController::RELEASED);
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

    jpad = NULL;
    jpad = SDL_JoystickOpen(0);
    
    //Check to see if joypad was not opened
    if(jpad == NULL) {
        LOG_NOTICE(TJOYPAD, "\"SDL joypad\" input plugin NOT initialized ok");
        return false;
    } else if(jpad != NULL) {
        LOG_NOTICE(TJOYPAD, "\"SDL joypad\" input plugin initialized ok");
        return true;
    }
}


} // namespace