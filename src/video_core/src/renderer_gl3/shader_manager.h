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

#ifndef VIDEO_CORE_SHADER_MANAGER_H_
#define VIDEO_CORE_SHADER_MANAGER_H_

#include <GL/glew.h>

#include "common.h"
#include "gx_types.h"
#include "uniform_manager.h"

#define MAX_SHADERS 512

class ShaderManager {

public:

    ShaderManager();
    ~ShaderManager() {};

    /// Updates the uniform values for the current shader
    void UpdateUniforms();

    /// Sets the current shader program based on a set of GP parameters
    void SetShader();

    /**
     * Gets the default shader
     * @returns Handle to the default shader program
     */
    GLuint GetDefaultShader();

    /*
     * Initialize the shader manager
     * @param uniform_manager Handle to the UniformManager instance that handles uniform data
     */
    void Init(UniformManager* uniform_manager);

private:

    /**
     * Compiles a shader program
     * @param preprocessor Preprocessor string to include before shader program
     * @return GLuint of new shader program
     */
    GLuint ShaderManager::CompileShaderProgram(const char* preprocessor);

    /**
     * Compiles a shader program given the specified shader inputs
     * @param num_stages: Number of TEV stages to compile program for
     * @param alpha_compare_function: Alpha comparision function logic
     */
    GLuint ShaderManager::LoadShader(int num_stages, int alpha_compare_function);

    GLuint current_shader_;             ///< Handle to current shader program
    GLuint default_shader_;             ///< Handle to default shader program
    GLuint shader_cache_[MAX_SHADERS];  ///< Array of precompiled shader programs
    
    int num_shaders_;

    std::string vertex_shader_src_;
    std::string fragment_shader_src_;

    char vertex_shader_path_[MAX_PATH];
    char fragment_shader_path_[MAX_PATH];

    UniformManager* uniform_manager_;
};

#endif // VIDEO_CORE_SHADER_MANAGER_H_