/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_manager.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-04-22
 * @brief   Managers shaders for the GL3 renderer
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

#include "renderer_gl3.h"

// Default shader header prefixed on all shaders. Contains minimum shader version and required 
// extensions enabled
static const char __default_shader_header[] = {
    "#version 140\n"
    "#extension GL_ARB_explicit_attrib_location : enable\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_ARB_uniform_buffer_object : enable\n"
};

/// ShaderManager constructor
ShaderManager::ShaderManager() {
    cache_ = new ShaderCache();
}

/// ShaderManager destructor
ShaderManager::~ShaderManager() {
    delete cache_;
}

/// Updates the uniform values for the current shader
void ShaderManager::UpdateUniforms() {
    static const int texture_locations[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(glGetUniformLocation(current_shader_, "texture"), 8, texture_locations);
}

/**
 * Compiles a shader program
 * @param vs_def Preprocessor string to include before vertex shader program
 * @param fs_def Preprocessor string to include before fragment shader program
 * @return GLuint of new shader program
 */
GLuint ShaderManager::CompileShaderProgram(const char* vs_def, const char* fs_def) {
    GLint res;

    // Create the shaders
    GLuint gs_id = 0;
    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile Vertex Shader
    const char *vs_sources[] = { __default_shader_header, vs_def, vertex_shader_src_.c_str() };
    glShaderSource(vs_id, 3, vs_sources, NULL);
    glCompileShader(vs_id);
    glGetShaderiv(vs_id, GL_COMPILE_STATUS, &res);

    if (res == GL_FALSE) {
	    glGetShaderiv(vs_id, GL_INFO_LOG_LENGTH, &res);
	    char* log = new char[res];
	    glGetShaderInfoLog(vs_id, res, &res, log);
        _ASSERT_MSG(TVIDEO, 0, "Vertex shader failed to compile! Error(s):\n%s", log);
        delete [] log;
    }

    // Compile Fragment Shader
    const char *fs_sources[] = { __default_shader_header, fs_def, fragment_shader_src_.c_str() };
    glShaderSource(fs_id, 3, fs_sources, NULL);
    glCompileShader(fs_id);
    glGetShaderiv(fs_id, GL_COMPILE_STATUS, &res);

    if (res == GL_FALSE) {
	    glGetShaderiv(fs_id, GL_INFO_LOG_LENGTH, &res);
	    char* log = new char[res];
	    glGetShaderInfoLog(fs_id, res, &res, log);
        _ASSERT_MSG(TVIDEO, 0, "Fragment shader failed to compile! Error(s):\n%s", log);
        delete [] log;
    }

    // Create the program
    GLuint program_id = glCreateProgram();
    glAttachShader(program_id, vs_id);
    glAttachShader(program_id, fs_id);
    glLinkProgram(program_id);
    glGetShaderiv(program_id, GL_LINK_STATUS, &res);
    if (res == GL_FALSE) {
	    glGetShaderiv(program_id, GL_INFO_LOG_LENGTH, &res);
	    char* log = new char[res];
	    glGetShaderInfoLog(program_id, res, &res, log);
        _ASSERT_MSG(TVIDEO, 0, "Shader program linker failed! Error(s):\n%s", log);
        delete [] log;
    }
    // Cleanup
    glDeleteShader(vs_id);
    glDeleteShader(fs_id);

    return program_id;
}

/**
 * Compute a 32-bit hash for the current TEV state, used for identifying the current shader to use
 * @return Unsigned int hash
 */
#define CRC_ROTL(crc) crc32_table[3][((crc) & 0xFF)] ^ crc32_table[2][((crc >> 8) & 0xFF)] ^ \
        crc32_table[1][((crc >> 16) & 0xFF)] ^ crc32_table[0][((crc >> 24))]
u32 ShaderManager::GetCurrentHash() {
	u32 crc = - 1;
    const u8 cur_alpha_func = ((gp::g_bp_regs.alpha_func.comp1 << 5) | 
        (gp::g_bp_regs.alpha_func.comp0 << 2) | (gp::g_bp_regs.alpha_func.logic));
    u32 cur_clamp = 0;

	// Generate a hash based off of CRC32 to attempt to avoid collisions
	crc ^= (((gp::g_cp_regs.vcd_lo[0]._u32 & 0x1FF) << 16) | 
        (gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_pos_dqf_enabled() << 15) | 
        (gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_format << 12) | (cur_alpha_func << 4) | 
        gp::g_bp_regs.genmode.num_tevstages);
	crc = CRC_ROTL(crc);

    for (int stage = 0; stage < (gp::g_bp_regs.genmode.num_tevstages + 1); stage++) {
        int reg_index = stage >> 1;

        crc ^= ((gp::g_bp_regs.combiner[stage].color._u32 & 0xFFFF) << 16) | 
            (gp::g_bp_regs.combiner[stage].alpha._u32 & 0xFFF0) | 
            (gp::g_bp_regs.combiner[stage].color.clamp << 3) | 
            (gp::g_bp_regs.combiner[stage].alpha.clamp << 2 | 
            gp::g_bp_regs.tevorder[reg_index].get_enable(stage));
	    crc = CRC_ROTL(crc);

        crc ^= (gp::g_bp_regs.combiner[stage].color.dest << 30) | 
            (gp::g_bp_regs.combiner[stage].color.dest << 28) | 
            (gp::g_bp_regs.tevorder[reg_index].get_texcoord(stage) << 25) | 
            (gp::g_bp_regs.tevorder[reg_index].get_texmap(stage) << 22);
	    crc = CRC_ROTL(crc);
    }
	return crc;
}

#define _SHADER_VSDEF(...) _vs_offset += sprintf(&_vs_def[_vs_offset], __VA_ARGS__)
#define _SHADER_FSDEF(...) _fs_offset += sprintf(&_fs_def[_fs_offset], __VA_ARGS__)
/// Compiles a shader program given the specified shader inputs
GLuint ShaderManager::LoadShader() {

    static const char* vertex_color[] = { "RGB565", "RGB8", "RGBX8", "RGBA4", "RGBA6", "RGBA8" };
    static const char* clamp[] = { "val", "clamp(val, 0.0, 1.0)" };
    static const char* alpha_logic[] = { "&&", "||", "!=", "==" };
    static const char* alpha_compare_0[] = { "(false)", "(val < ref0)", "(val == ref0)", 
        "(val <= ref0)", "(val > ref0)", "(val != ref0)", "(val >= ref0)", "(true)" };
    static const char* alpha_compare_1[] = { "(false)", "(val < ref1)", "(val == ref1)", 
        "(val <= ref1)", "(val > ref1)", "(val != ref1)", "(val >= ref1)", "(true)" };
    static const char* texture[] = { "vec4(1.0f, 1.0f, 1.0f, 1.0f)", 
        "texture2D(texture[%d], vtx_texcoord[%d])" };
    static const char* tev_color_input[] = { "prev.rgb",  "prev.aaa", "color0.rgb", "color0.aaa",
        "color1.rgb", "color1.aaa", "color2.rgb", "color2.aaa", "tex.rgb", "tex.aaa", "ras.rgb",
        "ras.aaa", "vec3(1.0f, 1.0f, 1.0f)", "vec3(0.5f, 0.5f, 0.5f)", "konst.rgb", 
        "vec3(0.0f, 0.0f, 0.0f)"  };
    static const char* tev_alpha_input[] = { "prev.a", "color0.a", "color1.a", "color2.a", "tex.a",
        "ras.a", "konst.a", "0.0f" };
    static const char* tev_dest[] = { "prev", "color0", "color1", "color2" };

    int _vs_offset = 0;
    int _fs_offset = 0;
    char temp[256];
    char _vs_def[1024];
    char _fs_def[8192];

    // Generate vertex preprocessor
    // ----------------------------

    if (gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_pos_dqf_enabled()) 
        _SHADER_VSDEF("#define __VSDEF_POS_DQF\n");
    if (gp::g_cp_regs.vcd_lo[0].pos_midx_enable) _SHADER_VSDEF("#define __VSDEF_POS_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex0_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_0_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex1_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_1_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex2_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_2_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex3_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_3_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex4_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_4_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex5_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_5_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex6_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_6_MIDX\n");
    if (gp::g_cp_regs.vcd_lo[0].tex7_midx_enable) _SHADER_VSDEF("#define __VSDEF_TEX_7_MIDX\n");

    _SHADER_VSDEF("#define __VSDEF_COLOR0_%s\n", 
        vertex_color[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_format]);

    // Generate fragment preprocessor
    // ------------------------------

    _SHADER_FSDEF("#define __FSDEF_NUM_STAGES %d\n", gp::g_bp_regs.genmode.num_tevstages);
    _SHADER_FSDEF("#define __FSDEF_ALPHA_COMPARE(val, ref0, ref1) (!(%s %s %s))\n",
        alpha_compare_0[gp::g_bp_regs.alpha_func.comp0],
        alpha_logic[gp::g_bp_regs.alpha_func.logic],
        alpha_compare_1[gp::g_bp_regs.alpha_func.comp1]);

    for (int stage = 0; stage <= gp::g_bp_regs.genmode.num_tevstages; stage++) {
        int reg_index = stage >> 1;

        _SHADER_FSDEF("#define __FSDEF_CLAMP_COLOR_%d(val) %s\n", stage, 
            clamp[gp::g_bp_regs.combiner[stage].color.clamp]);
        _SHADER_FSDEF("#define __FSDEF_CLAMP_ALPHA_%d(val) %s\n", stage, 
            clamp[gp::g_bp_regs.combiner[stage].alpha.clamp]);
        _SHADER_FSDEF("#define __FSDEF_STAGE_DEST vec4(%s.rgb, %s.a)\n", 
            tev_dest[gp::g_bp_regs.combiner[gp::g_bp_regs.genmode.num_tevstages].color.dest], 
            tev_dest[gp::g_bp_regs.combiner[gp::g_bp_regs.genmode.num_tevstages].alpha.dest]);
        sprintf(temp, "#define __FSDEF_TEXTURE_%d %s\n", stage, 
            texture[gp::g_bp_regs.tevorder[reg_index].get_enable(stage)]);
        if (gp::g_bp_regs.tevorder[reg_index].get_enable(stage)) {
            _SHADER_FSDEF(temp, gp::g_bp_regs.tevorder[reg_index].get_texmap(stage), 
                gp::g_bp_regs.tevorder[reg_index].get_texcoord(stage));
        } else {
            _SHADER_FSDEF(temp);
        }
        _SHADER_FSDEF("#define __FSDEF_COMBINER_COLOR_A_%d %s\n", stage, 
            tev_color_input[gp::g_bp_regs.combiner[stage].color.sel_a]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_COLOR_B_%d %s\n", stage, 
            tev_color_input[gp::g_bp_regs.combiner[stage].color.sel_b]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_COLOR_C_%d %s\n", stage, 
            tev_color_input[gp::g_bp_regs.combiner[stage].color.sel_c]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_COLOR_D_%d %s\n", stage, 
            tev_color_input[gp::g_bp_regs.combiner[stage].color.sel_d]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_COLOR_DEST_%d %s.rgb\n", stage, 
            tev_dest[gp::g_bp_regs.combiner[stage].color.dest]);

        _SHADER_FSDEF("#define __FSDEF_COMBINER_ALPHA_A_%d %s\n", stage, 
            tev_alpha_input[gp::g_bp_regs.combiner[stage].alpha.sel_a]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_ALPHA_B_%d %s\n", stage, 
            tev_alpha_input[gp::g_bp_regs.combiner[stage].alpha.sel_b]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_ALPHA_C_%d %s\n", stage, 
            tev_alpha_input[gp::g_bp_regs.combiner[stage].alpha.sel_c]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_ALPHA_D_%d %s\n", stage, 
            tev_alpha_input[gp::g_bp_regs.combiner[stage].alpha.sel_d]);
        _SHADER_FSDEF("#define __FSDEF_COMBINER_ALPHA_DEST_%d %s.a\n", stage, 
            tev_dest[gp::g_bp_regs.combiner[stage].alpha.dest]);
    }
    return this->CompileShaderProgram(_vs_def, _fs_def);
}

/// Sets the current shader program based on a set of GP parameters
void ShaderManager::SetShader() {
    u32 hash = this->GetCurrentHash(); // Compute current shader hash
    const GLuint* res = cache_->FetchFromHash(hash); // Fetch the shader program from the cache

    // Generate shader if it does not already exist...
    if (NULL == res) {
        current_shader_ = LoadShader();
        cache_->Update(hash, current_shader_);
        uniform_manager_->AttachShader(current_shader_);

    // Set the shader program if it is not the current shader...
    } else {
        current_shader_ = *res;
    }
    glUseProgram(current_shader_);
    this->UpdateUniforms();
}

/**
 * Gets the default shader
 * @returns Handle to the default shader program
 */
GLuint ShaderManager::GetDefaultShader() {
    return default_shader_;   
}

/*
 * Initialize the shader manager
 * @param uniform_manager Handle to the UniformManager instance that handles uniform data
 */
void ShaderManager::Init(UniformManager* uniform_manager) {
    // Load vertex shader source
    strcpy(vertex_shader_path_, common::g_config->program_dir());
    strcat(vertex_shader_path_, "sys/shaders/default.vs");
    std::ifstream vs_ifs(vertex_shader_path_);
    if (vs_ifs.fail()) {
        LOG_ERROR(TVIDEO, "Failed to open shader %s", vertex_shader_path_);
        return;
    }
    vertex_shader_src_ = std::string((std::istreambuf_iterator<char>(vs_ifs)), 
        std::istreambuf_iterator<char>());

    // Load fragment shader source
    strcpy(fragment_shader_path_, common::g_config->program_dir());
    strcat(fragment_shader_path_, "sys/shaders/default.fs");
    std::ifstream fs_ifs(fragment_shader_path_);
    if (fs_ifs.fail()) {
        LOG_ERROR(TVIDEO, "Failed to fragment shader %s", fragment_shader_path_);
        return;
    }
    fragment_shader_src_ = std::string((std::istreambuf_iterator<char>(fs_ifs)), 
        std::istreambuf_iterator<char>());

    // Build and assign default shader
    default_shader_ = LoadShader();
    current_shader_ = default_shader_;
    glUseProgram(current_shader_);

    uniform_manager_ = uniform_manager;

    LOG_NOTICE(TGP, "shader manager initialized ok");
}
