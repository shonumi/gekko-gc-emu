/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    texture_cache.h
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

#ifndef VIDEO_CORE_TEXTURE_CACHE_H_
#define VIDEO_CORE_TEXTURE_CACHE_H_

#include <GL/glew.h>

#include "common.h"
#include "hash.h"
#include "hash_container.h"
#include "texture_decoder.h"
#include "texture_manager.h"

struct TextureCacheEntry {
    common::Hash64      hash;               ///< Hash of source texture raw data
    u32                 address;            ///< Source address of texture
    size_t              size;               ///< Source size of texture in bytes
    int                 width;              ///< Source texture width in pixels
    int                 height;             ///< Source texture height in pixels
    int                 last_valid_frame;   ///< Frame that texture expires at
    gp::TextureType     type;               ///< Texture type (raw RAM data or result of EFB-copy)
    gp::TextureFormat   format;             ///< Source texture format  (dest is always RGBA8)
    GLuint              gl_tex;             ///< Decoded OpenGL VRAM texture object
};

typedef HashContainer_STLHashMap<common::Hash64, TextureCacheEntry> TextureContainer;

/// Storage container for cached textures in the GL3 renderer
class TextureCache {

public:
    TextureCache();
    ~TextureCache();

private:



};

#endif // VIDEO_CORE_TEXTURE_CACHE_H_
