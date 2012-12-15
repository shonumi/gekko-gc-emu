/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    cp_mem.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-10
 * @brief   Implementation of CP for the graphics processor
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

#include "common.h"
#include "cp_mem.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

CPMemory g_cp_regs; ///< CP memory/registers

/// Write a BP register
void CP_RegisterWrite(u8 addr, u32 data) {
    g_cp_regs.mem[addr] = data;
}

/// Initialize CP
void CP_Init() {
    memset(&g_cp_regs, 0, sizeof(g_cp_regs));
}

} // namespace
