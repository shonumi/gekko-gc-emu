/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    keyboard_input.h
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

#ifndef INPUT_COMMON_SDL_KEYS_
#define INPUT_COMMON_SDL_KEYS_

#include "common.h"
#include "input_common.h"

class EmuWindow;

namespace input_common {

class KeyboardInput : virtual public InputBase {
public:
    KeyboardInput(EmuWindow* emu_window);
    ~KeyboardInput();

    bool Init();
    void PollEvent();
    void ShutDown();

    /*!
     * \brief Sets the controller status from the keyboard (called from EmuWindow)
     * \param channel Channel of controller to set status of (0-3)
     * \param key ID of the affected key (frontend specific)
     * \param state GCController::GCButtonState we're setting
     * \return True if the key maps to one of the GC pad controls
     */
    bool SetControllerStatus(int channel, int key, GCController::GCButtonState state);

private:
    /*!
     * \brief Gets the controller status from the keyboard using EmuWindow interfaces
     * \param channel Channel of controller to set status of (0-3)
     * \param key Keboard input that needs to be checked
     */
    GCController::GCButtonState GetControllerStatus(int channel, int key);

    DISALLOW_COPY_AND_ASSIGN(KeyboardInput);

    // TODO: Probably not necessary
    EmuWindow* emuwindow_;
};

} // namepsace

#endif //INPUT_COMMON_SDL_KEYS_
