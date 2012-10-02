/**
* Copyright (C) 2005-2012 Gekko Emulator
*
* @file    uniform_manager.h
* @author  ShizZy <shizzy247@gmail.com>
* @date    2012-09-07
* @brief   Managers shader uniform data for the GL3 renderer
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
#include "crc.h"
#include "config.h"

#include "gx_types.h"
#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

#include "shader_manager.h"
#include "uniform_manager.h"

UniformManager::UniformManager() {
    ubo_handle_bp_ = 0;
    ubo_handle_xf_ = 0;
    last_invalid_region_bp_ = 0;
    last_invalid_region_xf_ = 0;
    memset(invalid_regions_xf_, 0, sizeof(invalid_regions_xf_));
    //memset(invalid_regions_bp_, 0, sizeof(invalid_regions_bp_));
    memset(&uniform_blocks_, 0, sizeof(uniform_blocks_));
    memset(invalid_bp_tev_stages_, 0, sizeof(invalid_bp_tev_stages_));
}

/**
 * Write data to BP for renderer internal use (e.g. direct to shader)
 * @param addr BP register address
 * @param data Value to write to BP register
 */
void UniformManager::WriteBP(u8 addr, u32 data) {
    static char uniform_name[256];
    static u8   test_byte = 0;


    //if (data == gp::g_bp_regs.mem[addr]) {
    //	return;
    //}
    //sprintf(uniform_name, "bp_mem[%d]", addr);
    //glUniform1i(glGetUniformLocation(shader_manager::g_shader_default_id, uniform_name), data);
    switch(addr) {
    case BP_REG_GENMODE:
        {
            gp::BPGenMode gen_mode;
            gen_mode._u32 = data;
            glUniform1i(glGetUniformLocation(shader_manager::g_shader_default_id, "bp_genmode_num_stages"),
                (gen_mode.num_tevstages+1));
        }
        break;

    case BP_REG_TEV_COLOR_ENV + 0:
    case BP_REG_TEV_COLOR_ENV + 2:
    case BP_REG_TEV_COLOR_ENV + 4:
    case BP_REG_TEV_COLOR_ENV + 6:
    case BP_REG_TEV_COLOR_ENV + 8:
    case BP_REG_TEV_COLOR_ENV + 10:
    case BP_REG_TEV_COLOR_ENV + 12:
    case BP_REG_TEV_COLOR_ENV + 14:
    case BP_REG_TEV_COLOR_ENV + 16:
    case BP_REG_TEV_COLOR_ENV + 18:
    case BP_REG_TEV_COLOR_ENV + 20:
    case BP_REG_TEV_COLOR_ENV + 22:
    case BP_REG_TEV_COLOR_ENV + 24:
    case BP_REG_TEV_COLOR_ENV + 26:
    case BP_REG_TEV_COLOR_ENV + 28:
    case BP_REG_TEV_COLOR_ENV + 30:
        {
            int stage = (addr - BP_REG_TEV_COLOR_ENV) >> 1;

            UniformStruct_TevStageParams tev_stage = uniform_blocks_.bp_regs.tev_stage[stage];

            tev_stage.color_sel_a   = gp::g_bp_regs.combiner[stage].color.sel_a;
            tev_stage.color_sel_b   = gp::g_bp_regs.combiner[stage].color.sel_b;
            tev_stage.color_sel_c   = gp::g_bp_regs.combiner[stage].color.sel_c;
            tev_stage.color_sel_d   = gp::g_bp_regs.combiner[stage].color.sel_d;
            tev_stage.color_bias    = gp::g_bp_regs.combiner[stage].color.bias;
            tev_stage.color_sub     = gp::g_bp_regs.combiner[stage].color.sub;
            tev_stage.color_clamp   = gp::g_bp_regs.combiner[stage].color.clamp;
            tev_stage.color_shift   = gp::g_bp_regs.combiner[stage].color.shift;
            tev_stage.color_dest    = gp::g_bp_regs.combiner[stage].color.dest;

            // If region is already invalid, just write data
            if (invalid_bp_tev_stages_[stage]) {
                uniform_blocks_.bp_regs.tev_stage[stage] = tev_stage;

            // Otherwise, invalidate region in UBO if a change is detected
            } else {
                u32 new_hash = GenerateCRC((u8*)&tev_stage, sizeof(UniformStruct_TevStageParams));
                u32 old_hash = GenerateCRC((u8*)&uniform_blocks_.bp_regs.tev_stage[stage], 
                    sizeof(UniformStruct_TevStageParams));

                if (new_hash != old_hash) {
                    uniform_blocks_.bp_regs.tev_stage[stage] = tev_stage;
                    invalid_bp_tev_stages_[stage] = 1;
                }
            }
        }
        break;
        
    case BP_REG_TEV_ALPHA_ENV + 0:
    case BP_REG_TEV_ALPHA_ENV + 2:
    case BP_REG_TEV_ALPHA_ENV + 4:
    case BP_REG_TEV_ALPHA_ENV + 6:
    case BP_REG_TEV_ALPHA_ENV + 8:
    case BP_REG_TEV_ALPHA_ENV + 10:
    case BP_REG_TEV_ALPHA_ENV + 12:
    case BP_REG_TEV_ALPHA_ENV + 14:
    case BP_REG_TEV_ALPHA_ENV + 16:
    case BP_REG_TEV_ALPHA_ENV + 18:
    case BP_REG_TEV_ALPHA_ENV + 20:
    case BP_REG_TEV_ALPHA_ENV + 22:
    case BP_REG_TEV_ALPHA_ENV + 24:
    case BP_REG_TEV_ALPHA_ENV + 26:
    case BP_REG_TEV_ALPHA_ENV + 28:
    case BP_REG_TEV_ALPHA_ENV + 30:
        {
            int stage = (addr - BP_REG_TEV_ALPHA_ENV) >> 1;

            UniformStruct_TevStageParams tev_stage = uniform_blocks_.bp_regs.tev_stage[stage];

            tev_stage.alpha_sel_a   = gp::g_bp_regs.combiner[stage].alpha.sel_a;
            tev_stage.alpha_sel_b   = gp::g_bp_regs.combiner[stage].alpha.sel_b;
            tev_stage.alpha_sel_c   = gp::g_bp_regs.combiner[stage].alpha.sel_c;
            tev_stage.alpha_sel_d   = gp::g_bp_regs.combiner[stage].alpha.sel_d;
            tev_stage.alpha_bias    = gp::g_bp_regs.combiner[stage].alpha.bias;
            tev_stage.alpha_sub     = gp::g_bp_regs.combiner[stage].alpha.sub;
            tev_stage.alpha_clamp   = gp::g_bp_regs.combiner[stage].alpha.clamp;
            tev_stage.alpha_shift   = gp::g_bp_regs.combiner[stage].alpha.shift;
            tev_stage.alpha_dest    = gp::g_bp_regs.combiner[stage].alpha.dest;

            // If region is already invalid, just write data
            if (invalid_bp_tev_stages_[stage]) {
                uniform_blocks_.bp_regs.tev_stage[stage] = tev_stage;

            // Otherwise, invalidate region in UBO if a change is detected
            } else {
                u32 new_hash = GenerateCRC((u8*)&tev_stage, sizeof(UniformStruct_TevStageParams));
                u32 old_hash = GenerateCRC((u8*)&uniform_blocks_.bp_regs.tev_stage[stage], 
                    sizeof(UniformStruct_TevStageParams));

                if (new_hash != old_hash) {
                    uniform_blocks_.bp_regs.tev_stage[stage] = tev_stage;
                    invalid_bp_tev_stages_[stage] = 1;
                }
            }
        }
        break;

    case BP_REG_ALPHACOMPARE:
        {
            gp::BPAlphaFunc alpha_func;
            alpha_func._u32 = data;
            glUniform1i(glGetUniformLocation(shader_manager::g_shader_default_id, "bp_alpha_func_ref0"),
                alpha_func.ref0);
            glUniform1i(glGetUniformLocation(shader_manager::g_shader_default_id, "bp_alpha_func_ref1"),
                alpha_func.ref1);
            glUniform1i(glGetUniformLocation(shader_manager::g_shader_default_id, "bp_alpha_func_comp0"),
                alpha_func.comp0);
            glUniform1i(glGetUniformLocation(shader_manager::g_shader_default_id, "bp_alpha_func_comp1"),
                alpha_func.comp1);
            glUniform1i(glGetUniformLocation(shader_manager::g_shader_default_id, "bp_alpha_func_logic"),
                alpha_func.logic);
        }
        break;

    case BP_REG_TEV_KSEL:
        /*case BP_REG_TEV_KSEL + 1:
        case BP_REG_TEV_KSEL + 2:
        case BP_REG_TEV_KSEL + 3:
        case BP_REG_TEV_KSEL + 4:
        case BP_REG_TEV_KSEL + 5:
        case BP_REG_TEV_KSEL + 6:
        case BP_REG_TEV_KSEL + 7:*/
        {
            int stage = (addr - BP_REG_TEV_KSEL) << 1;
            gp::BPTevKSel ksel;
            ksel._u32 = data;

            int temp0[2] = { ksel.kcsel0, ksel.kasel0 };
            int temp1[2] = { ksel.kcsel1, ksel.kasel1 };

            sprintf(uniform_name, "bp_tev_ksel[%d]", stage*2);
            glUniform1iv(glGetUniformLocation(shader_manager::g_shader_default_id, uniform_name), 2, temp0);

            sprintf(uniform_name, "bp_tev_ksel[%d]", stage+1);
            glUniform1iv(glGetUniformLocation(shader_manager::g_shader_default_id, uniform_name), 2, temp1);

        }
        break;
    }
}

/**
 * Write data to XF for renderer internal use (e.g. direct to shader)
 * @param addr XF address
 * @param length Length (in 32-bit words) to write to XF
 * @param data Data buffer to write to XF
 */
void UniformManager::WriteXF(u16 addr, int length, u32* data) {
    int bytelen = length << 2;

    // Invalidate region in UBO if a change is detected
    if (GenerateCRC((u8*)data, bytelen) != 
        GenerateCRC((u8*)&uniform_blocks_.xf_regs.pos_mem[addr], bytelen)) {

        // Update data block
        memcpy(&uniform_blocks_.xf_regs.pos_mem[addr], data, bytelen);

        // Invalidate GPU data block region
        invalid_regions_xf_[last_invalid_region_xf_].offset = addr << 2;
        invalid_regions_xf_[last_invalid_region_xf_].length = bytelen;
        //invalid_regions_xf_[last_invalid_region_xf_].start_addr = (u8*)&uniform_blocks_.xf_regs.pos_mem[addr];

        last_invalid_region_xf_++;
    }
}

/// Apply any uniform changes to the shader
void UniformManager::ApplyChanges() {
    // Update invalid regions in XF UBO
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle_xf_);
    for (int i = 0; i < last_invalid_region_xf_; i++) {
        glBufferSubData(GL_UNIFORM_BUFFER, 
            invalid_regions_xf_[i].offset, 
            invalid_regions_xf_[i].length, 
            &uniform_blocks_.xf_regs.pos_mem[invalid_regions_xf_[i].offset >> 2]);
    }
    last_invalid_region_xf_ = 0;

    // Update invalid regions in BP UBO
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle_bp_);
    for (int stage = 0; stage < 16; stage++) {

        if (!invalid_bp_tev_stages_[stage]) continue;

        invalid_bp_tev_stages_[stage] = 0;

        glBufferSubData(GL_UNIFORM_BUFFER, 
            stage * sizeof(UniformStruct_TevStageParams), 
            sizeof(UniformStruct_TevStageParams), 
            &uniform_blocks_.bp_regs.tev_stage[stage]);
    }
}

/// Initialize the Uniform Manager
void UniformManager::Init() {
    GLuint block_index;

    // Initialize BP UBO
    // -----------------

    block_index = glGetUniformBlockIndex(shader_manager::g_shader_default_id, "BPRegisters");
    glUniformBlockBinding(shader_manager::g_shader_default_id, block_index, 0);

    glGenBuffers(1, &ubo_handle_bp_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle_bp_);

    glBufferData(GL_UNIFORM_BUFFER, 1280, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_handle_bp_);

    // Initialize XF UBO
    // -----------------

    block_index = glGetUniformBlockIndex(shader_manager::g_shader_default_id, "XFRegisters");
    glUniformBlockBinding(shader_manager::g_shader_default_id, block_index, 1);

    glGenBuffers(1, &ubo_handle_xf_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle_xf_);

    glBufferData(GL_UNIFORM_BUFFER, 0x400, NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle_xf_);
}
