/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_manager.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-09
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

#ifndef VIDEO_CORE_SHADER_BASE_TYPES_H_
#define VIDEO_CORE_SHADER_BASE_TYPES_H_

#include "renderer_gl3/renderer_gl3.h"

#include "gx_types.h"

namespace shader_manager {

extern GLuint g_current_shader_id;       ///< Handle to current shader program
extern GLuint g_shader_default_id;       ///< Handle to default shader program

/// Updates the uniform values for the current shader
void UpdateUniforms();

/**
 * @brief Sets the current shader program
 * @param shader_id Shader program to use
 */
void SetShader(GLuint shader_id);

/**
 * @brief Gets the shader ID of the current shader program
 * @return GLuint of current shader ID
 */
GLuint GetCurrentShaderID();

/**
 * @brief Compiles a shader program
 * @param vs Vertex shader program source string
 * @param gs Geometry shader program source string (optional)
 * @param fs Fragment shader program source string
 * @remark When geometry shaders are not available (e.g. OpenGL ES), the "gs" parameter is unused
 * @return GLuint of new shader program
 */
GLuint CompileShaderProgram(const char * vs, const char* gs, const char* fs);

/// Initialize the shader manager
void Init();

} // namespace

#endif // VIDEO_CORE_SHADER_BASE_TYPES_H_