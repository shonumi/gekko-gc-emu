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
    ubo_fs_handle_ = 0;
    ubo_vs_handle_ = 0;
    ubo_fs_block_index_ = 0;
    ubo_vs_block_index_ = 0;
    last_invalid_region_xf_ = 0;
    memset(invalid_regions_xf_, 0, sizeof(invalid_regions_xf_));
    memset(&staged_uniform_data_, 0, sizeof(staged_uniform_data_));
    memset(&__uniform_data_, 0, sizeof(__uniform_data_));
    memset(&konst_, 0, sizeof(konst_));
}

/**
 * Lookup the TEV konst color value for a given kont selector
 * @param sel Konst selector corresponding to the desired konst color
 */
Vec4Color UniformManager::GetTevKonst(int sel) {
    switch(sel) {
    case 0:  return Vec4Color(1.0, 1.0, 1.0, 1.0);
    case 1:  return Vec4Color(0.875, 0.875, 0.875, 0.875);
    case 2:  return Vec4Color(0.75, 0.75, 0.75, 0.75);
    case 3:  return Vec4Color(0.625, 0.625,0.625, 0.625);
    case 4:  return Vec4Color(0.5, 0.5, 0.5, 0.5);
    case 5:  return Vec4Color(0.375, 0.375, 0.375,  0.375);
    case 6:  return Vec4Color(0.25, 0.25, 0.25, 0.25);
    case 7:  return Vec4Color(0.125, 0.125, 0.125, 0.125);
    case 12: return konst_[0];
    case 13: return konst_[1];
    case 14: return konst_[2];
    case 15: return konst_[3];
    case 16: return Vec4Color(konst_[0].r, konst_[0].r, konst_[0].r, konst_[0].r);
    case 17: return Vec4Color(konst_[1].r, konst_[1].r, konst_[1].r, konst_[1].r);
    case 18: return Vec4Color(konst_[2].r, konst_[2].r, konst_[2].r, konst_[2].r);
    case 19: return Vec4Color(konst_[3].r, konst_[3].r, konst_[3].r, konst_[3].r);
    case 20: return Vec4Color(konst_[0].g, konst_[0].g, konst_[0].g, konst_[0].g);
    case 21: return Vec4Color(konst_[1].g, konst_[1].g, konst_[1].g, konst_[1].g);
    case 22: return Vec4Color(konst_[2].g, konst_[2].g, konst_[2].g, konst_[2].g);
    case 23: return Vec4Color(konst_[3].g, konst_[3].g, konst_[3].g, konst_[3].g);
    case 24: return Vec4Color(konst_[0].b, konst_[0].b, konst_[0].b, konst_[0].b);
    case 25: return Vec4Color(konst_[1].b, konst_[1].b, konst_[1].b, konst_[1].b);
    case 26: return Vec4Color(konst_[2].b, konst_[2].b, konst_[2].b, konst_[2].b);
    case 27: return Vec4Color(konst_[3].b, konst_[3].b, konst_[3].b, konst_[3].b);
    case 28: return Vec4Color(konst_[0].a, konst_[0].a, konst_[0].a, konst_[0].a);
    case 29: return Vec4Color(konst_[1].a, konst_[1].a, konst_[1].a, konst_[1].a);
    case 30: return Vec4Color(konst_[2].a, konst_[2].a, konst_[2].a, konst_[2].a);
    case 31: return Vec4Color(konst_[3].a, konst_[3].a, konst_[3].a, konst_[3].a);
    default: LOG_ERROR(TGP, "Unknown TEV konst lookup index = %d", sel);
    }
    return Vec4Color();
}

/**
 * Write data to BP for renderer internal use (e.g. direct to shader)
 * @param addr BP register address
 * @param data Value to write to BP register
 */
void UniformManager::WriteBP(u8 addr, u32 data) {
    static const f32 tev_scale[] = { 1.0, 2.0, 4.0, 0.5 };
    static const f32 tev_sub[] = { 1.0, -1.0 };
    static const f32 tev_bias[] = { 0.0, 0.5, -0.5, 0.0 };

    switch(addr) {
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
            staged_uniform_data_.fs_ubo.tev_stages[stage].color_bias = 
				tev_bias[gp::g_bp_regs.combiner[stage].color.bias];
            staged_uniform_data_.fs_ubo.tev_stages[stage].color_sub = 
				tev_sub[gp::g_bp_regs.combiner[stage].color.sub];
            staged_uniform_data_.fs_ubo.tev_stages[stage].color_scale = 
				tev_scale[gp::g_bp_regs.combiner[stage].color.shift];
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
            staged_uniform_data_.fs_ubo.tev_stages[stage].alpha_bias = 
				tev_bias[gp::g_bp_regs.combiner[stage].alpha.bias];
            staged_uniform_data_.fs_ubo.tev_stages[stage].alpha_sub = 
				tev_sub[gp::g_bp_regs.combiner[stage].alpha.sub];
            staged_uniform_data_.fs_ubo.tev_stages[stage].alpha_scale = 
				tev_scale[gp::g_bp_regs.combiner[stage].alpha.shift];
        }
        break;

	case 0xe0: // TEV_REGISTERL_0
	case 0xe2: // TEV_REGISTERL_1
	case 0xe4: // TEV_REGISTERL_2
	case 0xe6: // TEV_REGISTERL_3
	case 0xe1: // TEV_REGISTERH_0
	case 0xe3: // TEV_REGISTERH_1
	case 0xe5: // TEV_REGISTERH_2
	case 0xe7: // TEV_REGISTERH_3
        {
            int index = ((addr >> 1) - 0x70);

            if (addr & 1) { // green/blue
                if (!(data >> 23)) {
                    // unpack
                    staged_uniform_data_.fs_ubo.tev_state.color[index].g = 
                        ((data >> 12) & 0xff) / 255.0f;
                    staged_uniform_data_.fs_ubo.tev_state.color[index].b = 
                        ((data >> 0) & 0xff) / 255.0f;
                } else { // konstant
                    // unpack
                    konst_[index].g = ((data >> 12) & 0xff) / 255.0f;
                    konst_[index].b = ((data >> 0) & 0xff) / 255.0f;
                }
            } else { // red/alpha
                if (!(data >> 23)) {
                    // unpack
                    staged_uniform_data_.fs_ubo.tev_state.color[index].a = 
                        ((data >> 12) & 0xff) / 255.0f;
                    staged_uniform_data_.fs_ubo.tev_state.color[index].r = 
                        ((data >> 0) & 0xff) / 255.0f;
                } else { // konstant
                    // unpack
                    konst_[index].a = ((data >> 12) & 0xff) / 255.0f;
                    konst_[index].r = ((data >> 0) & 0xff) / 255.0f;
                }
            }
        }
		break;

    case BP_REG_ALPHACOMPARE:
        staged_uniform_data_.fs_ubo.tev_state.alpha_func_ref0 = 
            gp::g_bp_regs.alpha_func.ref0;
        staged_uniform_data_.fs_ubo.tev_state.alpha_func_ref1 = 
            gp::g_bp_regs.alpha_func.ref1;
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
        GenerateCRC((u8*)&__uniform_data_.vs_ubo.xf_mem[addr], bytelen)) {

        // Update data block
        memcpy(&__uniform_data_.vs_ubo.xf_mem[addr], data, bytelen);

        // Invalidate GPU data block region
        invalid_regions_xf_[last_invalid_region_xf_].offset = (addr << 2);
        invalid_regions_xf_[last_invalid_region_xf_].length = bytelen;

        last_invalid_region_xf_++;
    }
}

/// Updates any staged data to be written in the next uniform data upload
void UniformManager::UpdateStagedData() {

    // Vertex shader uniforms
    // ----------------------

    const int tex_matrix_offsets[8] = {
        gp::g_cp_regs.matrix_index_a.tex0_midx, gp::g_cp_regs.matrix_index_a.tex1_midx,
        gp::g_cp_regs.matrix_index_a.tex2_midx, gp::g_cp_regs.matrix_index_a.tex3_midx,
        gp::g_cp_regs.matrix_index_b.tex4_midx, gp::g_cp_regs.matrix_index_b.tex5_midx,
        gp::g_cp_regs.matrix_index_b.tex6_midx, gp::g_cp_regs.matrix_index_b.tex7_midx
    };
	const f32 tex_dqf[8] = {
		gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_tex0_dqf(),
		gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex1_dqf(),
		gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex2_dqf(),
		gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex3_dqf(),
		gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex4_dqf(),
		gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex5_dqf(),
		gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex6_dqf(),
		gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex7_dqf() 
	};
    memcpy(staged_uniform_data_.vs_ubo.state.projection_matrix, gp::g_projection_matrix, 64);

    staged_uniform_data_.vs_ubo.state.cp_pos_matrix_offset = 
        gp::g_cp_regs.matrix_index_a.pos_normal_midx;

    if (gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_format != GX_F32) {
        staged_uniform_data_.vs_ubo.state.cp_pos_dqf = 
            gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_pos_dqf();
    }
    memcpy(staged_uniform_data_.vs_ubo.state.cp_tex_matrix_offset, tex_matrix_offsets, 
        sizeof(tex_matrix_offsets));
    memcpy(staged_uniform_data_.vs_ubo.state.cp_tex_dqf, tex_dqf, sizeof(tex_dqf));

    // Fragment shader uniforms
    // ------------------------

    for (int stage = 0; stage < kGXNumTevStages; stage++) {
        int reg_index = stage >> 1;

        // Konst color
        staged_uniform_data_.fs_ubo.tev_stages[stage].konst = 
            GetTevKonst(gp::g_bp_regs.ksel[reg_index].get_konst_color_sel(stage));
        staged_uniform_data_.fs_ubo.tev_stages[stage].konst.a = 
            GetTevKonst(gp::g_bp_regs.ksel[reg_index].get_konst_alpha_sel(stage)).a;
    }
}

#define _COMBINE_BP_UBO_WRITES
/// Apply any uniform changes to the shader
void UniformManager::ApplyChanges() {

    this->UpdateStagedData(); // Grabs latest data to update

    // Update invalid regions vertex shader UBO
    // ----------------------------------------

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_vs_handle_);
    if (!(__uniform_data_.vs_ubo.state == staged_uniform_data_.vs_ubo.state)) {
        __uniform_data_.vs_ubo.state = staged_uniform_data_.vs_ubo.state;
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformStruct_VertexState), 
            &__uniform_data_.vs_ubo.state);
    }
    for (int i = 0; i < last_invalid_region_xf_; i++) {
        glBufferSubData(GL_UNIFORM_BUFFER, 
            invalid_regions_xf_[i].offset + sizeof(UniformStruct_VertexState), 
            invalid_regions_xf_[i].length, 
            &__uniform_data_.vs_ubo.xf_mem[invalid_regions_xf_[i].offset >> 2]);
    }
    last_invalid_region_xf_ = 0;

    // Update invalid regions fragment shader UBO
    // ------------------------------------------

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_fs_handle_);

    // Init changeset markers
#ifdef _COMBINE_BP_UBO_WRITES
    int changeset_start = -1;
    int changeset_length = 0;
    int num_stage_iterations = kGXNumTevStages + 1;
#else
    int num_stage_iterations = kGXNumTevStages;
#endif
    if (!(__uniform_data_.fs_ubo.tev_state == 
        staged_uniform_data_.fs_ubo.tev_state) || 
        !(__uniform_data_.fs_ubo.tev_stages[0] == 
        staged_uniform_data_.fs_ubo.tev_stages[0])) {

        __uniform_data_.fs_ubo.tev_state = staged_uniform_data_.fs_ubo.tev_state;
        __uniform_data_.fs_ubo.tev_stages[0] = 
            staged_uniform_data_.fs_ubo.tev_stages[0];

        glBufferSubData(GL_UNIFORM_BUFFER, 0, 
            sizeof(UniformStruct_TevState) + sizeof(UniformStruct_TevStageParams), 
            &__uniform_data_.fs_ubo);
    }

    // Iterate through each stage of UBO data, upload to GPU memory if a change is detected, 
    // otherwise ignore. If _COMBINE_BP_UBO_WRITES is defined, sequentially changes will be combined
    // when uploaded. Otherwise, they will be uploaed individually.
    for (int stage = 1; stage < num_stage_iterations; stage++) {

        // No change found
        // ---------------

        if ((staged_uniform_data_.fs_ubo.tev_stages[stage] == 
            __uniform_data_.fs_ubo.tev_stages[stage]) || (stage >= kGXNumTevStages)) {

            // Upload last changeset
#ifdef _COMBINE_BP_UBO_WRITES               
            if (changeset_start != -1) {
                int byte_offset = sizeof(UniformStruct_TevState) + 
                    (changeset_start * sizeof(UniformStruct_TevStageParams));
                glBufferSubData(GL_UNIFORM_BUFFER, 
                    byte_offset, 
	                sizeof(UniformStruct_TevStageParams) * changeset_length, 
	                &__uniform_data_.fs_ubo.tev_stages[changeset_start]);

                // Reset changeset markers
                changeset_start = -1;
                changeset_length = 0;
            }
#endif
            continue;

        // Change found
        // ------------

        } else {
#ifdef _COMBINE_BP_UBO_WRITES 
            // If new changeset, mark start point of next uniform burst
            if (changeset_start == -1) {
                changeset_start = stage;
            }
            __uniform_data_.fs_ubo.tev_stages[stage] = 
                staged_uniform_data_.fs_ubo.tev_stages[stage];
            changeset_length++;
#else
            __uniform_data_.fs_ubo.tev_stages[stage] = 
                staged_uniform_data_.fs_ubo.tev_stages[stage];

            int byte_offset = sizeof(UniformStruct_TevState) + 
                (stage * sizeof(UniformStruct_TevStageParams));

            glBufferSubData(GL_UNIFORM_BUFFER, byte_offset, sizeof(UniformStruct_TevStageParams), 
	            &__uniform_data_.fs_ubo.tev_stages[stage]);
#endif
        }
    }
}

/**
 * Attach a shader to the Uniform Manager for uniform binding
 * @param shader Compiled GLSL shader program
 */
void UniformManager::AttachShader(GLuint shader) {
    glUniformBlockBinding(shader, ubo_fs_block_index_, 0);
    glUniformBlockBinding(shader, ubo_vs_block_index_, 1);
}

/// Initialize the Uniform Manager
void UniformManager::Init(GLuint default_shader) {
    // Initialize BP UBO(s)
    ubo_fs_block_index_ = glGetUniformBlockIndex(default_shader, "_FS_UBO");
    glGenBuffers(1, &ubo_fs_handle_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_fs_handle_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(__uniform_data_.fs_ubo), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_fs_handle_);

    // Initialize XF UBO
    ubo_vs_block_index_ = glGetUniformBlockIndex(default_shader, "_VS_UBO");
    glGenBuffers(1, &ubo_vs_handle_);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_vs_handle_);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(__uniform_data_.vs_ubo), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_vs_handle_);
}
