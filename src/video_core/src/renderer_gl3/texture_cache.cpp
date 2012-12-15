/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    texture_cache.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-12-05
 * @brief   Storage container for cached textures in the GL3 renderer
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

#include "texture_cache.h"

TextureCache::TextureCache() {
    cache_ = new TextureContainer;
}

TextureCache::~TextureCache() {
    delete cache_;
}

void TextureCache::AddTexture(CacheEntry texture) {
    cache_->Update(texture.hash, texture);
}

bool TextureCache::GetTexture(common::Hash64 hash, CacheEntry& texture) {
    if (E_ERR != cache_->Fetch(hash, texture)) {
        texture.last_frame_used = 0xDEADBEEF; // TODO(ShizZy): Set this to be valid here
        cache_->Update(hash, texture);
        return true;
    }
    return false;
}

void TextureCache::Purge() {
}
