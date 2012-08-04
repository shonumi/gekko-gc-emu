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

namespace shader_manager {

GLuint g_current_shader_id;       ///< Handle to current shader program

GLuint shader_default_id;       ///< Handle to default shader program
GLuint shader_default_quads_id; ///< Handle to default quads shader program

/**
 * @brief Sets the primitive type for shader use
 * @param type GXPrimitive type of current primitive
 */
void SetPrimitive(GXPrimitive type) {
    if (type == GX_QUADS) {
        g_current_shader_id = shader_default_quads_id;
    } else {
        g_current_shader_id = shader_default_id;
    }
    glUseProgram(g_current_shader_id);
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

    // XF - modelview matrix, raw ODE4x3 format (3 vec4's)
    f32* modelview = (f32*)&gp::g_xf_mem[(gp::g_cp_regs.matrix_index_a.pos_normal_midx * 4)];
    glUniform4fv(glGetUniformLocation(g_current_shader_id, "xf_modelview_vectors"), 3, modelview);

    // XF - positition matrices
    if (gp::g_cp_regs.vcd_lo[0].pos_midx_enable) {
        glUniform4fv(glGetUniformLocation(g_current_shader_id, "xf_position_vectors"), 
            gp::kXFMemEntriesNum, (f32*)gp::g_xf_mem);
    }
    // CP - Vertex formats
    glUniform1i(glGetUniformLocation(g_current_shader_id, "pos_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_format);
    glUniform1i(glGetUniformLocation(g_current_shader_id, "col0_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_format);
    glUniform1i(glGetUniformLocation(g_current_shader_id, "col1_format"), 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col1_format);

    // CP - dequantization shift values
    glUniform1i(glGetUniformLocation(g_current_shader_id, "cp_pos_shift"), VAT_POSSHFT);
    glUniform1i(glGetUniformLocation(g_current_shader_id, "cp_tex_shift_0"), VAT_TEX0SHFT);

    // Textures
    // TODO(ShizZy): Enable the remaining textures
    int tex_enable[8] = { gp::g_bp_regs.tevorder[0].get_enable(0), 0, 0, 0, 0, 0, 0, 0 };
    glUniform1i(glGetUniformLocation(g_current_shader_id, "texture0"), 0);
    glUniform1iv(glGetUniformLocation(g_current_shader_id, "tex_enable"), 8, tex_enable);
}

/**
 * @brief Compiles a shader program
 * @param vs Vertex shader program source string
 * @param gs Geometry shader program source string (optional)
 * @param fs Fragment shader program source string
 * @remark When geometry shaders are not available (e.g. OpenGL ES), the "gs" parameter is unused
 * @return GLuint of new shader program
 */
GLuint CompileShaderProgram(const char * vs, const char* gs, const char* fs) {
    GLint res;

    // Create the shaders
    GLuint gs_id = 0;
    GLuint vs_id = glCreateShader(GL_VERTEX_SHADER);
    GLuint fs_id = glCreateShader(GL_FRAGMENT_SHADER);

#ifdef USE_GEOMETRY_SHADERS
    if (NULL != gs) {
        gs_id = glCreateShader(GL_GEOMETRY_SHADER);

        // Compile Geometry Shader
        glShaderSource(gs_id, 1, &gs , NULL);
        glCompileShader(gs_id);
        glGetShaderiv(gs_id, GL_COMPILE_STATUS, &res);
        if (res == GL_FALSE) {
	        glGetShaderiv(gs_id, GL_INFO_LOG_LENGTH, &res);
	        char* log = new char[res];
	        glGetShaderInfoLog(gs_id, res, &res, log);
            _ASSERT_MSG(TVIDEO, 0, "Geometry shader failed to compile! Error(s):\n%s", log);
            delete [] log;
        }
    }
#endif
    // Compile Vertex Shader
    glShaderSource(vs_id, 1, &vs , NULL);
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
    glShaderSource(fs_id, 1, &fs , NULL);
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
GLuint LoadShader(char* vs_path, char* gs_path, char* fs_path) {
    std::ifstream vs_ifs(vs_path);
    if (vs_ifs.fail()) {
        LOG_ERROR(TVIDEO, "Failed to open shader %s", vs_path);
        return 0;
    }
    std::ifstream fs_ifs(fs_path);
    if (fs_ifs.fail()) {
        LOG_ERROR(TVIDEO, "Failed to fragment shader %s", fs_path);
        return 0;
    }
    std::string vs_str((std::istreambuf_iterator<char>(vs_ifs)), std::istreambuf_iterator<char>());
    std::string fs_str((std::istreambuf_iterator<char>(fs_ifs)), std::istreambuf_iterator<char>());

    if (gs_path != NULL) {
        std::ifstream gs_ifs(gs_path);
        if (gs_ifs.fail()) {
            LOG_ERROR(TVIDEO, "Failed to geometry shader %s", gs_path);
            return 0;
        }
        std::string gs_str((std::istreambuf_iterator<char>(gs_ifs)), 
            std::istreambuf_iterator<char>());

        return CompileShaderProgram(vs_str.c_str(), gs_str.c_str(), fs_str.c_str());
    } else {
        return CompileShaderProgram(vs_str.c_str(), NULL, fs_str.c_str());
    }
    return 0;
}

/// Initialize the shader manager
void Init() {
    char vs_filename[MAX_PATH];
    char gs_filename[MAX_PATH];
    char fs_filename[MAX_PATH];
    char fs_quads_filename[MAX_PATH];

    strcpy_s(vs_filename, MAX_PATH, common::g_config->program_dir());
    strcat_s(vs_filename, MAX_PATH, "sys/shaders/default.vs");

    strcpy_s(gs_filename, MAX_PATH, common::g_config->program_dir());
    strcat_s(gs_filename, MAX_PATH, "sys/shaders/default.gs");

    strcpy_s(fs_filename, MAX_PATH, common::g_config->program_dir());
    strcat_s(fs_filename, MAX_PATH, "sys/shaders/default.fs");

    strcpy_s(fs_quads_filename, MAX_PATH, common::g_config->program_dir());
    strcat_s(fs_quads_filename, MAX_PATH, "sys/shaders/default_quads.fs");

    shader_default_id = LoadShader(vs_filename, NULL, fs_filename);
    shader_default_quads_id = LoadShader(vs_filename, gs_filename, fs_quads_filename);

    LOG_NOTICE(TGP, "shader_manager initialized ok");
}

} // namespace
