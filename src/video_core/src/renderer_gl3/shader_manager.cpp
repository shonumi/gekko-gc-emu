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

#include "renderer_gl3.h"
#include "gx_types.h"
#include "shader_base_types.h"

namespace shader_manager {

GLuint current_shader_id;       ///< Handle to current shader program

GLuint shader_default_id;       ///< Handle to default shader program
GLuint shader_default_quads_id; ///< Handle to default quads shader program

/**
 * @brief Sets the primitive type for shader use
 * @param type GXPrimitive type of current primitive
 */
void SetPrimitive(GXPrimitive type) {
    if (type == GX_QUADS) {
        current_shader_id = shader_default_quads_id;
    } else {
        current_shader_id = shader_default_id;
    }
    glUseProgram(current_shader_id);
}

/**
 * @brief Gets the shader ID of the current shader program
 * @return GLuint of current shader ID
 */
GLuint GetCurrentShaderID() {
    return current_shader_id;
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
    }
#endif

    // Compile Vertex Shader
    glShaderSource(vs_id, 1, &vs , NULL);
    glCompileShader(vs_id);
 
    // Compile Fragment Shader
    glShaderSource(fs_id, 1, &fs , NULL);
    glCompileShader(fs_id);
 
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

    // Cleanup
    glDeleteShader(vs_id);
#ifdef USE_GEOMETRY_SHADERS
    glDeleteShader(gs_id);
#endif
    glDeleteShader(fs_id);


    return program_id;
}

/// Initialize the shader manager
void Init() {

    shader_default_id = CompileShaderProgram(kDefaultVertexShader, 
                                             NULL, 
                                             kDefaultFragmentShader);
#ifdef USE_GEOMETRY_SHADERS
    shader_default_quads_id = CompileShaderProgram(kDefaultVertexShader, 
                                                   kDefaultGeometryShaderQuads,
                                                   kDefaultFragmentShaderQuads);
#else
    shader_default_quads_id = shader_default_id;
#endif

    LOG_NOTICE(TGP, "shader_manager initialized ok");
}

} // namespace