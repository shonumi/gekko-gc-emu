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
#include "video_core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

CPMemory g_cp_regs; ///< CP memory/registers

/// Write a BP register
void CP_RegisterWrite(u8 addr, u32 data) {
    // Skip register write (if nothing is new)
    if (g_cp_regs.mem[addr] == data) {
        return;
    }
    g_cp_regs.mem[addr] = data;

    switch (addr) {
    // Map all 8 CP_REG_VCD_LO registers to the base register
    case CP_REG_VCD_LO + 0:
    case CP_REG_VCD_LO + 1:
    case CP_REG_VCD_LO + 2:
    case CP_REG_VCD_LO + 3:
    case CP_REG_VCD_LO + 4:
    case CP_REG_VCD_LO + 5:
    case CP_REG_VCD_LO + 6:
    case CP_REG_VCD_LO + 7:
        g_cp_regs.mem[CP_REG_VCD_LO] = data;

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_Position,
                                                 gp::g_cp_regs.vcd_lo[0].pos_midx_enable);

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_0,
                                                 gp::g_cp_regs.vcd_lo[0].tex0_midx_enable); 

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_1,
                                                 gp::g_cp_regs.vcd_lo[0].tex1_midx_enable); 

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_2,
                                                 gp::g_cp_regs.vcd_lo[0].tex2_midx_enable); 

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_3,
                                                 gp::g_cp_regs.vcd_lo[0].tex3_midx_enable); 

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_4,
                                                 gp::g_cp_regs.vcd_lo[0].tex4_midx_enable); 

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_5,
                                                 gp::g_cp_regs.vcd_lo[0].tex5_midx_enable); 

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_6,
                                                 gp::g_cp_regs.vcd_lo[0].tex6_midx_enable); 

        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_MatrixIndexed_TexCoord_7,
                                                 gp::g_cp_regs.vcd_lo[0].tex7_midx_enable);
        break;

    // Map all 8 CP_REG_VCD_HI registers to the base register
    case CP_REG_VCD_HI + 0:
    case CP_REG_VCD_HI + 1:
    case CP_REG_VCD_HI + 2:
    case CP_REG_VCD_HI + 3:
    case CP_REG_VCD_HI + 4:
    case CP_REG_VCD_HI + 5:
    case CP_REG_VCD_HI + 6:
    case CP_REG_VCD_HI + 7:
        g_cp_regs.mem[CP_REG_VCD_HI] = data;
        break;
    }
}

/// Initialize CP
void CP_Init() {
    memset(&g_cp_regs, 0, sizeof(g_cp_regs));
}

} // namespace
