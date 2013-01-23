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

    switch (cache_entry.type_) {

    // Normal texture from RAM
    case TextureManager::kSourceType_Normal:
        glGenTextures(1, &backend_data->color_texture_);    
        glBindTexture(GL_TEXTURE_2D, backend_data->color_texture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cache_entry.width_, cache_entry.height_, 0, 
            GL_RGBA, GL_UNSIGNED_BYTE, raw_data);
        break;

    // Texture is the result of an EFB copy
    case TextureManager::kSourceType_EFBCopy:
        {
            GLuint textures[2];
            glGenTextures(2, textures);  

            backend_data->color_texture_ = textures[0];
            backend_data->depth_texture_ = textures[1];
            backend_data->is_depth_copy = (cache_entry.efb_copy_data_.pixel_format_ == gp::kPixelFormat_Z24);

            // Create the color component texture
            glBindTexture(GL_TEXTURE_2D, backend_data->color_texture_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, cache_entry.width_, cache_entry.height_, 0, 
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);
        
            // Create the depth component texture
            glBindTexture(GL_TEXTURE_2D, backend_data->depth_texture_);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, cache_entry.width_, 
                cache_entry.height_, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
            glBindTexture(GL_TEXTURE_2D, 0);
        
            // Create the FBO and attach color/depth textures
            glGenFramebuffers(1, &backend_data->efb_framebuffer_); // Generate framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backend_data->efb_framebuffer_);

            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                backend_data->color_texture_, 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
                backend_data->depth_texture_, 0);
        
            // Check for completeness
            GLenum res = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
            _ASSERT_MSG(TGP, GL_FRAMEBUFFER_COMPLETE == res, "couldn't OpenGL EFB copy FBO!");

            //////////////////////

            /*static u8 raw_data[1024 * 1024 * 4];
            static int num = 0;
            num++;
            glBindFramebuffer(GL_READ_FRAMEBUFFER, parent_->fbo_[RendererBase::kFramebuffer_EFB]);
            glReadBuffer(GL_DEPTH_ATTACHMENT); 
            glReadPixels(cache_entry.efb_copy_data_.src_rect_.x0_,cache_entry.efb_copy_data_.src_rect_.y0_, cache_entry.efb_copy_data_.src_rect_.width(), cache_entry.efb_copy_data_.src_rect_.height(), GL_DEPTH_COMPONENT24, GL_UNSIGNED_INT, raw_data);

            // Optionally dump texture to TGA...
            if (backend_data->is_depth_copy) {//common::g_config->current_renderer_config().enable_texture_dumping) {
                std::string filepath = common::g_config->program_dir() + std::string("/dump");
                mkdir(filepath.c_str());
                filepath = filepath + std::string("/efb-copies");
                mkdir(filepath.c_str());
                filepath = common::FormatStr("%s/%08x_%d.tga", filepath.c_str(), gp::g_bp_regs.efb_copy_addr << 5, num);
                video_core::DumpTGA(filepath, cache_entry.efb_copy_data_.src_rect_.width(), cache_entry.efb_copy_data_.src_rect_.height(), raw_data);
            }
            glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);*/

            //////////////////////////////

            // Rebind EFB
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, parent_->fbo_[RendererBase::kFramebuffer_EFB]);
        }

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

    glDeleteTextures(1, &data->color_texture_);

    if (data->depth_texture_) glDeleteTextures(1, &data->depth_texture_);
    if (data->efb_framebuffer_) glDeleteFramebuffers(1, &data->efb_framebuffer_);

    delete backend_data;
}

/** 
 * Call to update a texture with a new EFB copy of the region specified by rect
 * @param src_rect Source rectangle to copy from EFB
 * @param dst_rect Destination rectange to copy to
 * @param backend_data Pointer to renderer-specific data used for the EFB copy
 */
void TextureInterface::CopyEFB(const Rect& src_rect, const Rect& dst_rect,
    const TextureManager::CacheEntry::BackendData* backend_data) {
    const BackendData* data = static_cast<const BackendData*>(backend_data);

    parent_->ResetRenderState();

    // Render target is destination framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, data->efb_framebuffer_);
    //glViewport(0, 0, rect.width, rect.height);

    // Render source is our EFB
    glBindFramebuffer(GL_READ_FRAMEBUFFER, parent_->fbo_[RendererBase::kFramebuffer_EFB]);
    glReadBuffer(data->is_depth_copy ? GL_DEPTH_ATTACHMENT : GL_COLOR_ATTACHMENT0);

    // Blit
    glBlitFramebuffer(src_rect.x0_, src_rect.y0_, src_rect.x1_, src_rect.y1_, 
                      dst_rect.x0_, dst_rect.y1_, dst_rect.x1_, dst_rect.y0_,
                      data->is_depth_copy ? GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT, GL_LINEAR);

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
    const BackendData* data = static_cast<const BackendData*>(backend_data);
    glActiveTexture(GL_TEXTURE0 + active_texture_unit);
    glBindTexture(GL_TEXTURE_2D, data->is_depth_copy ? data->depth_texture_ : data->color_texture_);
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
