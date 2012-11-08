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

#include "renderer_gl3.h"
#include "shader_base_types.h"

// Default shader header prefixed on all shaders. Contains minimum shader version and required 
// extensions enabled
static const char __default_shader_header[] = {
    "#version 140\n"
    "#extension GL_ARB_explicit_attrib_location : enable\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_ARB_uniform_buffer_object : enable\n"
};

namespace shader_manager {

GLuint g_current_shader_id = 0; ///< Handle to current shader program
GLuint g_shader_cache[256];     ///< Array of precompiled shader programs
int    g_num_shaders = 0;

/**
 * @brief Assign a binding point to an active uniform block
 * @param ubo_index The index of the active uniform block within program whose binding to assign
 * @param ubo_binding Specifies the binding point to which to bind the uniform block
 */
void BindUBO(GLuint ubo_index, GLuint ubo_binding) {
    for (int i = 0; i < g_num_shaders; i++) {
        glUniformBlockBinding(g_shader_cache[i], ubo_index, ubo_binding);
    }
}

/// Sets the current shader program based on a set of GP parameters
void SetShader() {
    int shader_id = (gp::g_bp_regs.genmode.num_tevstages << 2) | gp::g_bp_regs.alpha_func.logic;
    if (g_current_shader_id != g_shader_cache[shader_id]) {
        g_current_shader_id = g_shader_cache[shader_id];
        glUseProgram(g_current_shader_id);
    }
}

/**
 * @brief Gets the shader ID of the current shader program
 * @return GLuint of current shader ID
 */
GLuint GetCurrentShaderID() {
    return g_current_shader_id;
}

/// Updates the uniform values for the current shader
void UpdateUniforms() {

    // Projection matrix (already converted to GL4x4 format)
    glUniformMatrix4fv(glGetUniformLocation(g_current_shader_id, "projection_matrix"), 1, 
        GL_FALSE, gp::g_projection_matrix);

    // CP - position matrix index
    glUniform1i(glGetUniformLocation(g_current_shader_id, "cp_pos_matrix_index"), 
        gp::g_cp_regs.matrix_index_a.pos_normal_midx);

    // CP - vertex formats
    glUniform1i(glGetUniformLocation(g_current_shader_id, "cp_pos_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_format);
    glUniform1i(glGetUniformLocation(g_current_shader_id, "cp_col0_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_format);
    glUniform1i(glGetUniformLocation(g_current_shader_id, "cp_col1_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col1_format);

    // CP - dequantization shift values
	if (gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_format != GX_F32) {
		glUniform1f(glGetUniformLocation(g_current_shader_id, "cp_pos_dqf"), gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_pos_dqf());
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
    glUniform1fv(glGetUniformLocation(g_current_shader_id, "cp_tex_dqf"), 8, tex_dqf);

    // Textures
    const int tex_map[8] = { 0, 1, 2, 3, 4, 5, 6, 7 }; 
    glUniform1iv(glGetUniformLocation(g_current_shader_id, "texture"), 8, tex_map);
}

/**
 * @brief Compiles a shader program
 * @param vs Vertex shader program source string
 * @param gs Geometry shader program source string (optional)
 * @param fs Fragment shader program source string
 * @remark When geometry shaders are not available (e.g. OpenGL ES), the "gs" parameter is unused
 * @return GLuint of new shader program
 */
GLuint CompileShaderProgram(const char * vs, const char* fs, const char* preprocessor) {
    GLint res;

    // Create the shaders
    GLuint gs_id = 0;
    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Compile Vertex Shader
    const char *vs_sources[] = { __default_shader_header, vs };
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
    const char *fs_sources[] = { __default_shader_header, preprocessor, fs };
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
#ifdef USE_GEOMETRY_SHADERS
    if (NULL != gs) {
        glAttachShader(program_id, gs_id);
    }
#endif
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
#ifdef USE_GEOMETRY_SHADERS
    glDeleteShader(gs_id);
#endif
    glDeleteShader(fs_id);

    return program_id;
}

// Loads a shader from VS, GS, and FS paths (absolute). GS is ignored if the path is NULL.
// Returns 0 on error, otherwise the GLuint of the newly compiled shader.
void LoadShader(char* vs_path, char* fs_path) {
    std::ifstream vs_ifs(vs_path);
    if (vs_ifs.fail()) {
        LOG_ERROR(TVIDEO, "Failed to open shader %s", vs_path);
        return;
    }
    std::ifstream fs_ifs(fs_path);
    if (fs_ifs.fail()) {
        LOG_ERROR(TVIDEO, "Failed to fragment shader %s", fs_path);
        return;
    }
    std::string vs_str((std::istreambuf_iterator<char>(vs_ifs)), std::istreambuf_iterator<char>());
    std::string fs_str((std::istreambuf_iterator<char>(fs_ifs)), std::istreambuf_iterator<char>());

    // Compile shaders with prespecified macros
    char preprocessor_line[255];
    for (int i = 0; i < kGXNumTevStages; i++) {
        sprintf(preprocessor_line, "#define NUM_STAGES %d\n#define BP_ALPHA_FUNC_AND\n", (i + 1));
        g_shader_cache[(i * 4) + 0] = CompileShaderProgram(vs_str.c_str(), fs_str.c_str(), preprocessor_line);
        
        sprintf(preprocessor_line, "#define NUM_STAGES %d\n#define BP_ALPHA_FUNC_OR\n", (i + 1));
        g_shader_cache[(i * 4) + 1] = CompileShaderProgram(vs_str.c_str(), fs_str.c_str(), 
            preprocessor_line);

        sprintf(preprocessor_line, "#define NUM_STAGES %d\n#define BP_ALPHA_FUNC_XOR\n", (i + 1));
        g_shader_cache[(i * 4) + 2] = CompileShaderProgram(vs_str.c_str(), fs_str.c_str(), 
            preprocessor_line);

        sprintf(preprocessor_line, "#define NUM_STAGES %d\n#define BP_ALPHA_FUNC_XNOR\n", (i + 1));
        g_shader_cache[(i * 4) + 3] = CompileShaderProgram(vs_str.c_str(), fs_str.c_str(), 
            preprocessor_line);

        g_num_shaders += 4;
    }
}

/// Initialize the shader manager
void Init() {
    char vs_filename[MAX_PATH];
    char gs_filename[MAX_PATH];
    char fs_filename[MAX_PATH];
    char fs_quads_filename[MAX_PATH];

    strcpy(vs_filename, common::g_config->program_dir());
    strcat(vs_filename, "sys/shaders/default.vs");

    strcpy(fs_filename, common::g_config->program_dir());
    strcat(fs_filename, "sys/shaders/default.fs");

    LoadShader(vs_filename, fs_filename);
    
    SetShader();

    LOG_NOTICE(TGP, "shader_manager initialized ok");
}

} // namespace
