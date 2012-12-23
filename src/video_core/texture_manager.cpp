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

TextureManager::TextureManager(const BackendInterface* backend_interface) {
    backend_interface_ = const_cast<BackendInterface*>(backend_interface);
    cache_ = new CacheContainer();
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
    CacheEntry* tex = &active_textures_[active_texture_unit];
    static u8 raw_data[kGCMaxTextureWidth * kGCMaxTextureHeight * 4];

    tex->address_   = tex_image_3.image_base << 5;
    tex->width_     = tex_image_0.width + 1;
    tex->height_    = tex_image_0.height + 1;
    tex->type_      = kTextureType_Normal;
    tex->format_    = (gp::TextureFormat)tex_image_0.format;
    tex->size_      = gp::TextureDecoder_GetSize(tex->format_, tex->width_, tex->height_);
    tex->set_hash();

    if (E_ERR == cache_->FetchFromHash(tex->hash(), *tex)) {
        gp::TextureDecoder_Decode(tex->format_, tex->width_, tex->height_, 
            &Mem_RAM[tex->address_ & RAM_MASK], raw_data);
        tex->backend_data_ = backend_interface_->Create(active_texture_unit, *tex, raw_data);
    }
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
    CacheEntry* tex = &active_textures_[active_texture_unit];
    backend_interface_->Bind(active_texture_unit, tex->backend_data_);
    tex->set_frame_used();
    cache_->Update(tex->hash(), *tex);
}

/**
 * Gets cached CacheEntry object from a index into the texture cache
 * @param index Index into texture cache of texture to select
 * @param res Result CacheEntry object
 * @return True if lookup succeeded, false if failed
 */
const TextureManager::CacheEntry& TextureManager::Fetch(int index) {
    TextureManager::CacheEntry texture;
    cache_->FetchFromIndex(index, texture);
    return texture;
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
    CacheEntry cache_entry;
    for (int i = 0; i < this->Size(); i++) {
        cache_->FetchFromIndex(i, cache_entry);
        if ((cache_entry.frame_used() + age_limit) < video_core::g_current_frame) {
            backend_interface_->Delete(cache_entry.backend_data_);
            cache_->Remove(cache_entry.hash());
        }
    }
}
