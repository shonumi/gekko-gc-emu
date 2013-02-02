/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_interface.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2013-01-29
 * @brief   Shader manager interface for the GL3 renderer
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

#include "file_utils.h"
#include "shader_interface.h"

// Default shader header prefixed on all shaders. Contains minimum shader version and required 
// extensions enabled
static const char __default_shader_header[] = {
    "#version 130\n"
    "#extension GL_ARB_explicit_attrib_location : enable\n"
    "#extension GL_ARB_separate_shader_objects : enable\n"
    "#extension GL_ARB_uniform_buffer_object : enable\n"
};

ShaderInterface::ShaderInterface(RendererGL3* parent) {
    parent_ = parent;

    std::string vs_path = std::string(common::g_config->program_dir()) + 
        std::string("sys/shaders/default.vs");
    std::string fs_path = std::string(common::g_config->program_dir()) + 
        std::string("sys/shaders/default.fs");

    // Load shader file base source
    common::ReadFileToString(true, vs_path.c_str(), __vs_base_src_);
    common::ReadFileToString(true, fs_path.c_str(), __fs_base_src_);
}

ShaderInterface::~ShaderInterface() {
}

/**
 * Create a new shader in the backend renderer
 * @param vs_header Vertex shader header definitions
 * @param fs_header Fragment shader header definitions
 * @return a pointer to CacheEntry::BackendData with renderer-specific shader data
 */
ShaderManager::CacheEntry::BackendData* ShaderInterface::Create(const char* vs_header, 
    const char* fs_header) {

    BackendData*    backend_data    = new BackendData();
    GLuint          vs_id           = glCreateShader(GL_VERTEX_SHADER);
    GLuint          fs_id           = glCreateShader(GL_FRAGMENT_SHADER);
    GLint           res             = 0;

    const char*     vs_source[]     = {__default_shader_header, vs_header, __vs_base_src_.c_str()};
    const char*     fs_source[]     = {__default_shader_header, fs_header, __fs_base_src_.c_str()};

    // Compile Vertex Shader
    glShaderSource(vs_id, 3, vs_source, NULL);
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
    glShaderSource(fs_id, 3, fs_source, NULL);
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
    backend_data->program_ = glCreateProgram();
    glAttachShader(backend_data->program_, vs_id);
    glAttachShader(backend_data->program_, fs_id);
    glLinkProgram(backend_data->program_);
    glGetShaderiv(backend_data->program_, GL_LINK_STATUS, &res);
    if (res == GL_FALSE) {
	    glGetShaderiv(backend_data->program_, GL_INFO_LOG_LENGTH, &res);
	    char* log = new char[res];
	    glGetShaderInfoLog(backend_data->program_, res, &res, log);
        _ASSERT_MSG(TVIDEO, 0, "Shader program linker failed! Error(s):\n%s", log);
        delete [] log;
    }
    // Setup dual-source blending
    glBindFragDataLocationIndexed(backend_data->program_, 0, 0, "col0");
    glBindFragDataLocationIndexed(backend_data->program_, 0, 1, "col1");

    // Cleanup
    glDeleteShader(vs_id);
    glDeleteShader(fs_id);

    parent_->uniform_manager_->AttachShader(backend_data->program_);

    if (parent_->uniform_manager_->ubo_fs_handle_ == 0) {
        parent_->uniform_manager_->Init(backend_data->program_);
    }

    return backend_data;
}

/**
 * Delete a shader from the backend renderer
 * @param backend_data Renderer-specific shader data used by renderer to remove it
 */
void ShaderInterface::Delete(ShaderManager::CacheEntry::BackendData* backend_data) {
    BackendData* data = static_cast<BackendData*>(backend_data);
    glDeleteShader(data->program_);
    delete backend_data;
}

/**
 * Binds a shader to the backend renderer
 * @param active_texture_unit Active texture unit to bind to
 * @param backend_data Pointer to renderer-specific data used for binding
 */
void ShaderInterface::Bind(const ShaderManager::CacheEntry::BackendData* backend_data) {
    static const int texture_locations[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    const BackendData* data = static_cast<const BackendData*>(backend_data);
    
    // TODO(ShizZy): move this to the uniform manager
    glUniform1iv(glGetUniformLocation(data->program_, "texture"), 8, texture_locations);
    
    glUseProgram(data->program_);
}