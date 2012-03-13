/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    input_common.h
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-03
 * \brief   Base class for defining user input plugins
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

#ifndef INPUT_COMMON_INPUT_COMMON_H_
#define INPUT_COMMON_INPUT_COMMON_H_

#include "common.h"
#include "gc_controller.h"

namespace input_common {

class InputBase {
public:
	InputBase();
	virtual ~InputBase();

    virtual void Init();
    virtual void PollEvent();
    virtual void ShutDown();
private:
    DISALLOW_COPY_AND_ASSIGN(InputBase);
};

extern GCController*    g_controller_state[4];  ///< Current controller states
extern InputBase*       g_user_input;           ///< Pointer to the user input plugin we are using

/// Initialize the user input system
void EMU_FASTCALL Init();
 
} // namespace

#endif // INPUT_COMMON_INPUT_COMMON_H_