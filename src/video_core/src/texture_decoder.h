/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    texture_decoder.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-06-01
 * @brief   Simple texture decoder for GameCube graphics texture formats
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

#ifndef VIDEO_CORE_TEXTURE_DECODER_H_
#define VIDEO_CORE_TEXTURE_DECODER_H_

#define TMEM_SIZE						0x100000
#define TMEM_MASK						0x0fffff
#define TEX_CACHE_LOCATION(n)       ((n & 0xffff) ^ (n >> 16))
#define PTEXTURE_MEM(addr)          &gp::tmem[addr & TMEM_MASK]
#define GX_TX_SETIMAGE_NUM(x)		(((x & 0x20) >> 3) | (x & 3))

namespace gp {

/// GameCube/GX texture formats
enum TextureFormat {
    kTextureFormat_Intensity4       = 0x0,
    kTextureFormat_Intensity8       = 0x1,
    kTextureFormat_IntensityAlpha4  = 0x2,
    kTextureFormat_IntensityAlpha8  = 0x3,
    kTextureFormat_RGB565           = 0x4,
    kTextureFormat_RGB5A3           = 0x5,
    kTextureFormat_RGBA8            = 0x6,
    kTextureFormat_C4               = 0x8,
    kTextureFormat_C8               = 0x9,
    kTextureFormat_C14X2            = 0xA,
    kTextureFormat_CMPR             = 0xE,
};

extern u8 tmem[TMEM_SIZE];
extern u32 texcache[TMEM_SIZE];

void DecodeTexture(u8 format, u32 hash, u32 addr, u16 height, u16 width);

} // namespace

#endif // VIDEO_CORE_TEXTURE_DECODER_H_