/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    sdl_joypads.h
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

#ifndef INPUT_COMMON_SDL_JOYPADS_
#define INPUT_COMMON_SDL_JOYPADS_

#include "common.h"
#include "input_common.h"

namespace input_common {

class SDLJoypads  : virtual public InputBase {
public:
    SDLJoypads() {};
    ~SDLJoypads() {};

    bool Init();
    void PollEvent();
    void ShutDown();

private:
    /*!
     * \brief Sets the controller status from the joystick using SDL
     * \param channel Channel of controller to set status of (0-3)
     * \param pad Joypad input that was activated or released
     * \param state GCController::GCButtonState we're setting
     */
    void SetControllerStatus(int channel, int pad, GCController::GCButtonState state);

    /*!
     * \brief Gets the controller status from the joystick using SDL
     * \param channel Channel of controller to set status of (0-3)
     * \param pad Joypad input that needs to be checked
     */
    GCController::GCButtonState GetControllerStatus(int channel, int pad);

    /*!
     * \brief Checks whether joystick is in dead zone or not
     * \param value Integer from jaxis.value
     * \return true if in dead zone, false otherwise
     */
    static inline bool DeadZone(int val);

    SDL_Joystick *jpad;

    DISALLOW_COPY_AND_ASSIGN(SDLJoypads);
};

} // namepsace

#endif //INPUT_COMMON_SDL_JOYPADS_
