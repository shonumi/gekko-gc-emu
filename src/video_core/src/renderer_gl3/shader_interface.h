/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_interface.h
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

#ifndef VIDEO_CORE_RENDERER_GL3_SHADER_INTERFACE_H_
#define VIDEO_CORE_RENDERER_GL3_SHADER_INTERFACE_H_

#include <GL/glew.h>

#include "shader_manager.h"
#include "renderer_gl3.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// GL3 texture interface implementation

/// Storage container for cached textures in the GL3 renderer
class ShaderInterface : virtual public ShaderManager::BackendInterface {
public:

    ShaderInterface(RendererGL3* parent);
    ~ShaderInterface();

    /**
     * Create a new shader in the backend renderer
     * @param vs_header Vertex shader header definitions
     * @param fs_header Fragment shader header definitions
     * @return a pointer to CacheEntry::BackendData with renderer-specific shader data
     */
    ShaderManager::CacheEntry::BackendData* Create(const char* vs_header, const char* fs_header);

    /**
     * Delete a shader from the backend renderer
     * @param backend_data Renderer-specific shader data used by renderer to remove it
     */
    void Delete(ShaderManager::CacheEntry::BackendData* backend_data);

    /**
     * Binds a shader to the backend renderer
     * @param active_texture_unit Active texture unit to bind to
     * @param backend_data Pointer to renderer-specific data used for binding
     */
    void Bind(const ShaderManager::CacheEntry::BackendData* backend_data);

private:

    RendererGL3* parent_;

    std::string __vs_base_src_;
    std::string __fs_base_src_;

    class BackendData : public ShaderManager::CacheEntry::BackendData {
    public:
        BackendData() : program_(0) {
        }
        ~BackendData() {
        }
        GLuint program_;      ///< GL handle to the compiled shader program
    };

    DISALLOW_COPY_AND_ASSIGN(ShaderInterface);
};

#endif // VIDEO_CORE_RENDERER_GL3_SHADER_INTERFACE_H_
