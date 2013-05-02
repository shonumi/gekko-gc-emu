/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    sdl_joypads.cpp
 * @author  Shonumi <shonumi@gmail.com>
 * @date    2012-03-12
 * @brief   Implementation of a SDL joypad GC controller interface
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
#include "sdl_joypads.h"

//TODO: Set rumble strength
//TODO: Choose rumble types (sine, triangle, ...)
//TODO: Handle multiple joypads

namespace input_common {

/**
 * Gets the name of the current joystick, for UI purposes
 * @return String of the joystick's name
 */
std::string SDLJoypads::GetName() {
    return name;
}

/**
 * Sets the controller status from the joystick using SDL
 * @param channel Channel of controller to set status of (0-3)
 * @param pad Joypad input that was activated or released
 * @param state GCController::GCButtonState we're setting
 */
void SDLJoypads::SetControllerStatus(int channel, int pad, GCController::GCButtonState state) {
    for (unsigned int i = 0; i < common::Config::NUM_CONTROLS; ++i) {
        if (pad == common::g_config->controller_ports(channel).pads.key_code[i]) {
            g_controller_state[channel]->set_control_status((common::Config::Control)i, state);
        }
    }
}

/**
 * Gets the controller status from the joystick using SDL
 * @param channel Channel of controller to set status of (0-3)
 * @param pad Joypad input that needs to be checked
 */
GCController::GCButtonState SDLJoypads::GetControllerStatus(int channel, int pad) {
    for (unsigned int i = 0; i < common::Config::NUM_CONTROLS; ++i) {
        if (pad == common::g_config->controller_ports(channel).pads.key_code[i]) {
            return g_controller_state[channel]->control_status((common::Config::Control)i);
        }
    }

    // Return GC_CONTROLLER_NULL if unmapped pad
    return GCController::GC_CONTROLLER_NULL;   
}

/**
 * Checks whether joystick is in dead zone or not
 * @param value Integer from jaxis.value
 * @return true if in dead zone, false otherwise
 */
bool SDLJoypads::DeadZone(int val) {
    if((val < -8000) || (val > 8000)) {
        return false;
    } else if((val > -8000) && (val < 8000)) {
        return true;
    }
}

/// Poll for joypad input
void SDLJoypads::PollEvents() {
    SDL_Event joyevent;

    //Frequently check rumble status even if no joystick events
    if((g_controller_state[0]->get_rumble_status()) && (haptic_support) && (!is_rumbling)) {
        SDL_HapticRumblePlay(rumble, 0.5, -1);
        is_rumbling = true;
    } else if((!g_controller_state[0]->get_rumble_status()) && (haptic_support) && (is_rumbling)) {
        SDL_HapticRumbleStop(rumble);
        is_rumbling = false;
    }

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
            if(!DeadZone(val)) {
                SetControllerStatus(0, pad, GCController::PRESSED);

                //Release opposite axis, prevents input from getting stuck
                if(pad % 2 == 0) {
                    SetControllerStatus(0, pad+1, GCController::RELEASED);
                } else {
                    SetControllerStatus(0, pad-1, GCController::RELEASED);
                }
            } else if(DeadZone(val)) {
                SetControllerStatus(0, pad, GCController::RELEASED);

                //Release opposite axis, prevents input from getting stuck
                if(pad % 2 == 0) {
                    SetControllerStatus(0, pad+1, GCController::RELEASED);
                } else {
                    SetControllerStatus(0, pad-1, GCController::RELEASED);
                }
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
    SDL_JoystickClose(jpad);
    SDL_HapticClose(rumble);
}

// Initialize the joypad - Only init 1, for testing now
bool SDLJoypads::Init() {

    // Init SDL VIDEO for input events, don't init twice
    if(SDL_WasInit(SDL_INIT_VIDEO) == 0) {
        
        if(SDL_Init(SDL_INIT_VIDEO) != 0) {
            LOG_NOTICE(TJOYPAD, "\"SDL joypads\" input plugin could not initialize");
            return false;
        }
        
        // Quit SDL later if initialized here
        atexit(SDL_Quit);        
    }

    //Initalize SDL joypad subsystem first of all
    SDL_InitSubSystem(SDL_INIT_JOYSTICK);
    SDL_InitSubSystem(SDL_INIT_HAPTIC);

    jpad = NULL;
    jpad = SDL_JoystickOpen(0);

    //Check to see if joypad was not opened or was not enabled
    if(jpad == NULL) {
        LOG_NOTICE(TJOYPAD, "\"SDL joypads\" input plugin could not initialize");
        return false;
    } else if(!common::g_config->controller_ports(0).pads.enable) {
        return false;
    } else {
        LOG_NOTICE(TJOYPAD, "Joypad detected");
        LOG_NOTICE(TJOYPAD, SDL_JoystickName(jpad));
        LOG_NOTICE(TJOYPAD, "\"SDL joypads\" input plugin initialized ok");
    }

    //Initialize haptic rumbling
    rumble = SDL_HapticOpenFromJoystick(jpad);
    
    if(rumble == NULL) {
        LOG_NOTICE(TJOYPAD, "\"SDL joypads\" rumbling not supported on this device or driver");
    }

    if(SDL_HapticRumbleInit(rumble) != 0) {
        LOG_NOTICE(TJOYPAD, "\"SDL joypads\" could not enable haptic on device or driver");
        haptic_support = false;
    } else {
        haptic_support = true;
    }

    name = SDL_JoystickName(jpad);
    is_rumbling = false;
    return true;
}


} // namespace
