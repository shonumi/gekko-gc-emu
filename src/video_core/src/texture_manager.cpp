/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    texture_manager.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-12-20
 * @brief   Manages textures, texture cache, and defines renderer interface
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

#include "texture_manager.h"
#include "utils.h"
#include "config.h"

TextureManager::TextureManager(const BackendInterface* backend_interface) {
    backend_interface_  = const_cast<BackendInterface*>(backend_interface);
    cache_              = new CacheContainer();
    memset(active_textures_, 0, sizeof(active_textures_));
}

TextureManager::~TextureManager() {
    delete cache_;
}

/**
 * Updates texture data in the video core for a given active texture unit
 * @param active_texture_unit Texture unit to update (0-7)
 * @param tex_image_0 BP TexImage0 register to use for the update
 * @param tex_image_3 BP TexImage3 register to use for the update
 */
void TextureManager::UpdateData(int active_texture_unit, const gp::BPTexImage0& tex_image_0, 
    const gp::BPTexImage3& tex_image_3) {
    static CacheEntry   cache_entry;
    static u8           raw_data[kGCMaxTextureWidth * kGCMaxTextureHeight * 4];

    cache_entry.address_    = tex_image_3.image_base << 5;
    cache_entry.width_      = tex_image_0.width + 1;
    cache_entry.height_     = tex_image_0.height + 1;
    cache_entry.type_       = kTextureType_Normal;
    cache_entry.format_     = (gp::TextureFormat)tex_image_0.format;
    cache_entry.size_       = gp::TextureDecoder_GetSize(cache_entry.format_, 
                                                         cache_entry.width_, 
                                                         cache_entry.height_);
    cache_entry.hash_       = common::GetHash64(&Mem_RAM[cache_entry.address_ & RAM_MASK],
                                                cache_entry.size_, 
                                                kHashSamples);
    // Query cache for texture existance...
    active_textures_[active_texture_unit] = cache_->FetchFromHash(cache_entry.hash_);

    // Create and add to cache if texture does not exists...
    if (NULL == active_textures_[active_texture_unit]) {
        // Decode texture from source data to RGBA8 raw data...
        gp::TextureDecoder_Decode(cache_entry.format_, 
                                  cache_entry.width_,
                                  cache_entry.height_,
                                  &Mem_RAM[cache_entry.address_ & RAM_MASK],
                                  raw_data);

        // Optionally dump texture to TGA...
        if (common::g_config->current_renderer_config().enable_texture_dumping) {
            std::string filepath = common::g_config->program_dir() + std::string("/dump");
            mkdir(filepath.c_str());
            filepath = filepath + std::string("/textures");
            mkdir(filepath.c_str());
            filepath = common::FormatStr("%s/%08X.TGA", filepath.c_str(), cache_entry.hash_);
            video_core::DumpTGA(filepath, cache_entry.width_, cache_entry.height_, raw_data);
        }

        // Create a texture in VRAM from raw data...
        cache_entry.backend_data_ = backend_interface_->Create(active_texture_unit, 
                                                               cache_entry,
                                                               raw_data);
        // Update cache with new information...
        active_textures_[active_texture_unit] = cache_->Update(cache_entry.hash_, cache_entry);
    }
    active_textures_[active_texture_unit]->frame_used_ = video_core::g_current_frame;
}

/**
 * Updates the texture parameters
 * @param active_texture_unit Active texture unit to update the parameters for
 * @param tex_mode_0 BP TexMode0 register to use for the update
 * @param tex_mode_1 BP TexMode1 register to use for the update
 */
void TextureManager::UpdateParameters(int active_texture_unit, const gp::BPTexMode0& tex_mode_0,
    const gp::BPTexMode1& tex_mode_1) {
    backend_interface_->UpdateParameters(active_texture_unit, tex_mode_0, tex_mode_1);
}

/**
 * Binds the most recently updated texture for a given active texture unit
 * @param active_texture_unit Texture unit to bind (0-7)
 */
void TextureManager::Bind(int active_texture_unit) {
    backend_interface_->Bind(active_texture_unit, 
        active_textures_[active_texture_unit]->backend_data_);
}

/**
 * Gets cached CacheEntry object from a index into the texture cache
 * @param index Index into texture cache of texture to select
 * @param res Result CacheEntry object
 * @return True if lookup succeeded, false if failed
 */
const TextureManager::CacheEntry& TextureManager::Fetch(int index) {
    return *cache_->FetchFromIndex(index);
}

/**
 * Gets the number of active textures in the texture cache
 * @return Integer number of active textures in the texture cache
 */
int TextureManager::Size() {
    return cache_->Size();
}

/**
 * Purges expired textures (textures that are older than current_frame + age_limit)
 * @param age_limit Acceptable age limit (in frames) for textures to still be considered fresh
 */
void TextureManager::Purge(int age_limit) {
    CacheEntry* cache_entry = NULL;
    for (int i = 0; i < this->Size(); i++) {
        cache_entry = cache_->FetchFromIndex(i);
        if ((cache_entry->frame_used_ + age_limit) < video_core::g_current_frame) {
            backend_interface_->Delete(cache_entry->backend_data_);
            cache_->Remove(cache_entry->hash_);
        }
    }
}
