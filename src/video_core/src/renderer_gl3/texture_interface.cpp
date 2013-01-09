/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    texture_interface.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-12-05
 * @brief   Texture manager interface for the GL3 renderer
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

#include "texture_interface.h"
#include "utils.h"

TextureInterface::TextureInterface(RendererGL3* parent) {
    parent_ = parent;
}

TextureInterface::~TextureInterface() {
}

/**
 * Create a new texture in the backend renderer
 * @param active_texture_unit Active texture unit to bind to for creation
 * @param cache_entry CacheEntry to create texture for
 * @param raw_data Raw texture data
 * @return a pointer to CacheEntry::BackendData with renderer-specific texture data
 */
TextureManager::CacheEntry::BackendData* TextureInterface::Create(int active_texture_unit, 
    const TextureManager::CacheEntry& cache_entry, u8* raw_data) {

    BackendData* backend_data = new BackendData();

    glActiveTexture(GL_TEXTURE0 + active_texture_unit);
    glGenTextures(1, &backend_data->handle_);    
    glBindTexture(GL_TEXTURE_2D, backend_data->handle_);

    switch (cache_entry.type_) {

    // Normal texture from RAM
    case TextureManager::kSourceType_Normal:

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, cache_entry.width_, cache_entry.height_, 0, GL_RGBA,
            GL_UNSIGNED_BYTE, raw_data);

        break;

    // Texture is the result of an EFB copy
    case TextureManager::kSourceType_EFBCopy:

        // Create the texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cache_entry.efb_copy_rect_.width, 
            cache_entry.efb_copy_rect_.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        //glBindTexture(GL_TEXTURE_2D, 0);
        
        // Generate depth buffer storage
        glGenRenderbuffers(1, &backend_data->efb_depthbuffer_); // Generate depth buffer
        glBindRenderbuffer(GL_RENDERBUFFER, backend_data->efb_depthbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, cache_entry.efb_copy_rect_.width,
            cache_entry.efb_copy_rect_.height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        
        // Create the FBO and attach texture/depth buffer
        glGenFramebuffers(1, &backend_data->efb_framebuffer_); // Generate framebuffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backend_data->efb_framebuffer_);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, backend_data->handle_, 0);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, backend_data->efb_depthbuffer_);
        
        // Check for completeness
        _ASSERT_MSG(TGP, GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER),
            "couldn't create OpenGL FBO from new EFB copy!");

        /*glBindFramebuffer(GL_READ_FRAMEBUFFER, efb_copy_fbo->framebuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);

        static u8 raw_data[1024 * 1024 * 4];
        static int num = 0;
        num++;
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);
        glReadBuffer(GL_COLOR_ATTACHMENT0_EXT); 
        glReadPixels(0,0, rect.width, rect.height, GL_RGBA, GL_UNSIGNED_BYTE, raw_data);

        // Optionally dump texture to TGA...
        if (common::g_config->current_renderer_config().enable_texture_dumping) {
            std::string filepath = common::g_config->program_dir() + std::string("/dump");
            mkdir(filepath.c_str());
            filepath = filepath + std::string("/efb-copies");
            mkdir(filepath.c_str());
            filepath = common::FormatStr("%s/%08x_%d.tga", filepath.c_str(), gp::g_bp_regs.efb_copy_addr << 5, num);
            video_core::DumpTGA(filepath, rect.width, rect.height, raw_data);
        }
        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);*/

        // Rebind EFB
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, parent_->fbo_[RendererBase::kFramebuffer_EFB]);

        break;

    // Unknown texture source
    default:
        _ASSERT_MSG(TGP, 0, "Unknown texture source %d", (int)cache_entry.type_);
        break;

    }
    return backend_data;
}

/**
 * Delete a texture from the backend renderer
 * @param backend_data Renderer-specific texture data used by renderer to remove it
 */
void TextureInterface::Delete(TextureManager::CacheEntry::BackendData* backend_data) {
    BackendData* data = static_cast<BackendData*>(backend_data);

    glDeleteTextures(1, &data->handle_);

    if (data->efb_depthbuffer_) glDeleteRenderbuffers(1, &data->efb_depthbuffer_);
    if (data->efb_framebuffer_) glDeleteFramebuffers(1, &data->efb_framebuffer_);

    delete backend_data;
}

/** 
 * Call to update a texture with a new EFB copy of the region specified by rect
 * @param rect EFB rectangle to copy
 * @param backend_data Pointer to renderer-specific data used for the EFB copy
 */
void TextureInterface::CopyEFB(Rect rect, 
    const TextureManager::CacheEntry::BackendData* backend_data) {
    const BackendData* data = static_cast<const BackendData*>(backend_data);

    parent_->ResetRenderState();

    // Render target is destination framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data->efb_framebuffer_);
    //glViewport(0, 0, rect.width, rect.height);

    // Render source is our EFB
    glBindFramebuffer(GL_READ_FRAMEBUFFER, parent_->fbo_[RendererBase::kFramebuffer_EFB]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // Blit
    glBlitFramebuffer(rect.x, rect.y, rect.width, rect.height, 0, 0, rect.width, rect.height,
        GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    parent_->RestoreRenderState();
}

/**
 * Binds a texture to the backend renderer
 * @param active_texture_unit Active texture unit to bind to
 * @param backend_data Pointer to renderer-specific data used for binding
 */
void TextureInterface::Bind(int active_texture_unit, 
    const TextureManager::CacheEntry::BackendData* backend_data) {
    glActiveTexture(GL_TEXTURE0 + active_texture_unit);
    glBindTexture(GL_TEXTURE_2D, static_cast<const BackendData*>(backend_data)->handle_);
}

/**
 * Updates the texture parameters
 * @param active_texture_unit Active texture unit to update the parameters for
 * @param tex_mode_0 BP TexMode0 register to use for the update
 * @param tex_mode_1 BP TexMode1 register to use for the update
 */
void TextureInterface::UpdateParameters(int active_texture_unit, const gp::BPTexMode0& tex_mode_0,
    const gp::BPTexMode1& tex_mode_1) {
    static const GLint gl_tex_wrap[4] = {
        GL_CLAMP_TO_EDGE,
        GL_REPEAT,
        GL_MIRRORED_REPEAT,
        GL_REPEAT
    };
    static const GLint gl_mag_filter[2] = {
        GL_NEAREST,
        GL_LINEAR
    };
    static const GLint gl_min_filter[8] = {
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_NEAREST_MIPMAP_LINEAR,
        GL_NEAREST,
        GL_LINEAR,
        GL_LINEAR_MIPMAP_NEAREST,
        GL_LINEAR_MIPMAP_LINEAR,
        GL_LINEAR
    };
    glActiveTexture(GL_TEXTURE0 + active_texture_unit);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mag_filter[tex_mode_0.mag_filter]);
    /* TODO(ShizZy): Replace this code. Works sortof for autogenerated mip maps, but it's deprecated
            OpenGL. Currently, forward compatability is enabled, so anything deprecated will not work.
    if (tex_mode_0.use_mipmaps()) {
        glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
            gl_min_filter[tex_mode_0.min_filter & 7]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, tex_mode_1.min_lod >> 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, tex_mode_1.max_lod >> 4);
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, (tex_mode_0.lod_bias / 31.0f));
    } else {*/
        glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
            (tex_mode_0.min_filter >= 4) ? GL_LINEAR : GL_NEAREST);
    //}
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_tex_wrap[tex_mode_0.wrap_s]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_tex_wrap[tex_mode_0.wrap_t]);

    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2);
}
