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
#include "config.h"

#include "gx_types.h"
#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

#include "shader_manager.h"
#include "uniform_manager.h"

#include "renderer_gl3.h"

/// Const defines for alpha compare logic
static const char* kDefAlphaCompareLogic[4] = {
    "BP_ALPHA_FUNC_AND",
    "BP_ALPHA_FUNC_OR",
    "BP_ALPHA_FUNC_XOR",
    "BP_ALPHA_FUNC_XNOR"
};

// Default shader header prefixed on all shaders. Contains minimum shader version and required 
// extensions enabled
static const char __default_shader_header[] = {
    "#version 140\n"
    "#extension GL_ARB_explicit_attrib_location : enable\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_ARB_uniform_buffer_object : enable\n"
};

ShaderManager::ShaderManager() {
    memset(shader_cache_, 0, sizeof(shader_cache_));
    num_shaders_ = 0;

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

    uniform_manager_ = NULL;

    // Build and assign default shader
    default_shader_ = LoadShader();
    current_shader_ = default_shader_;
    glUseProgram(current_shader_);
}

/// Updates the uniform values for the current shader
void ShaderManager::UpdateUniforms() {

    // Projection matrix (already converted to GL4x4 format)
    glUniformMatrix4fv(glGetUniformLocation(current_shader_, "projection_matrix"), 1, 
        GL_FALSE, gp::g_projection_matrix);

    // CP - position matrix index
    glUniform1i(glGetUniformLocation(current_shader_, "cp_pos_matrix_index"), 
        gp::g_cp_regs.matrix_index_a.pos_normal_midx);

    // CP - vertex formats
    glUniform1i(glGetUniformLocation(current_shader_, "cp_pos_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_format);
    glUniform1i(glGetUniformLocation(current_shader_, "cp_col0_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_format);
    glUniform1i(glGetUniformLocation(current_shader_, "cp_col1_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col1_format);

    // CP - dequantization shift values
	if (gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_format != GX_F32) {
		glUniform1f(glGetUniformLocation(current_shader_, "cp_pos_dqf"), 
            gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_pos_dqf());
	}

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
    glUniform1fv(glGetUniformLocation(current_shader_, "cp_tex_dqf"), 8, tex_dqf);

    // Textures
    const int tex_map[16] = { 
        gp::g_bp_regs.tevorder[0].get_texmap(0),
        gp::g_bp_regs.tevorder[0].get_texmap(1),
        gp::g_bp_regs.tevorder[1].get_texmap(2),
        gp::g_bp_regs.tevorder[1].get_texmap(3),
        gp::g_bp_regs.tevorder[2].get_texmap(4),
        gp::g_bp_regs.tevorder[2].get_texmap(5),
        gp::g_bp_regs.tevorder[3].get_texmap(6),
        gp::g_bp_regs.tevorder[3].get_texmap(7),
        gp::g_bp_regs.tevorder[4].get_texmap(8),
        gp::g_bp_regs.tevorder[4].get_texmap(9),
        gp::g_bp_regs.tevorder[5].get_texmap(10),
        gp::g_bp_regs.tevorder[5].get_texmap(11),
        gp::g_bp_regs.tevorder[6].get_texmap(12),
        gp::g_bp_regs.tevorder[6].get_texmap(13),
        gp::g_bp_regs.tevorder[7].get_texmap(14),
        gp::g_bp_regs.tevorder[7].get_texmap(15)
    }; 
    glUniform1iv(glGetUniformLocation(current_shader_, "texture"), 16, tex_map);
}

/**
 * Compiles a shader program
 * @param preprocessor Preprocessor string to include before shader program
 * @return GLuint of new shader program
 */
GLuint ShaderManager::CompileShaderProgram(const char* preprocessor) {
    GLint res;

    // Create the shaders
    GLuint gs_id = 0;
    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile Vertex Shader
    const char *vs_sources[] = { __default_shader_header, vertex_shader_src_.c_str() };
    glShaderSource(vs_id, 2, vs_sources, NULL);
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
    const char *fs_sources[] = { __default_shader_header, preprocessor, fragment_shader_src_.c_str() };
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

#define _SHADER_PREDEF(...) offset += sprintf(&_shader_predef[offset], __VA_ARGS__)
/// Compiles a shader program given the specified shader inputs
GLuint ShaderManager::LoadShader() {
    static const char* alpha_logic[] = { "&&", "||", "|=", "==" };
    static const char* alpha_compare_0[] = { "false", "(val < ref0)", "(val == ref0)", "(val <= ref0)", 
        "(val > ref0)", "(val != ref0)", "(val >= ref0)", "true" };
    static const char* alpha_compare_1[] = { "false", "(val < ref1)", "(val == ref1)", "(val <= ref1)", 
        "(val > ref1)", "(val != ref1)", "(val >= ref1)", "true" };
    int offset = 0;
    char _shader_predef[1024];

    _SHADER_PREDEF("#define __PREDEF_NUM_STAGES %d\n", gp::g_bp_regs.genmode.num_tevstages + 1); 
    _SHADER_PREDEF("#define __PREDEF_ALPHA_COMPARE(val, ref0, ref1) (%s %s %s)\n",
        alpha_compare_0[gp::g_bp_regs.alpha_func.comp0],
        alpha_logic[gp::g_bp_regs.alpha_func.logic],
        alpha_compare_1[gp::g_bp_regs.alpha_func.comp1]);

    return CompileShaderProgram(_shader_predef);
}

/// Sets the current shader program based on a set of GP parameters
void ShaderManager::SetShader() {
    static int last_shader_index = 0;
    int shader_index = (gp::g_bp_regs.alpha_func.comp1 << 9) | (gp::g_bp_regs.alpha_func.comp0 << 6)
        | (gp::g_bp_regs.alpha_func.logic << 4) | (gp::g_bp_regs.genmode.num_tevstages);
    if (shader_index != last_shader_index) {
        if (0 == shader_cache_[shader_index]) {
            shader_cache_[shader_index] = LoadShader();
            uniform_manager_->AttachShader(shader_cache_[shader_index]);
        }
        current_shader_ = shader_cache_[shader_index];
        glUseProgram(current_shader_);
    }
    last_shader_index = shader_index;
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
    uniform_manager_ = uniform_manager;
    LOG_NOTICE(TGP, "shader_manager initialized ok");
}
