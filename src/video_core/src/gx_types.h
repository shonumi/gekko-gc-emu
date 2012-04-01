/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    gx_types.h
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-29
 * \brief   Types and structs used in the GX graphics API for GameCube/Wii
 *
 * \section LICENSE
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

#ifndef VIDEO_CORE_GX_TYPES_H_
#define VIDEO_CORE_GX_TYPES_H_

/// GX API primitive types
enum GXPrimitive {
    GX_POINTS        = 0xb8,
    GX_LINES         = 0xa8,
    GX_LINESTRIP     = 0xb0,
    GX_TRIANGLES     = 0x90,
    GX_TRIANGLESTRIP = 0x98,
    GX_TRIANGLEFAN   = 0xa0,
    GX_QUADS         = 0x80
};

/// GX API vertex component types
enum GXCompType {
    GX_U8           = 0,
    GX_S8           = 1,
    GX_U16          = 2,
    GX_S16          = 3,
    GX_F32          = 4,

    GX_RGB565       = 0,
    GX_RGB8         = 1,
    GX_RGBX8        = 2,
    GX_RGBA4        = 3,
    GX_RGBA6        = 4,
    GX_RGBA8        = 5
};

/// GX API vertex component count
enum GXCompCnt {
    GX_POS_XY    = 0,
    GX_POS_XYZ   = 1,
    GX_NRM_XYZ   = 0,
    GX_NRM_NBT   = 1, // one index per NBT
    GX_NRM_NBT3  = 2, // one index per each of N/B/T
    GX_CLR_RGB   = 0,
    GX_CLR_RGBA  = 1,
    GX_TEX_S     = 0,
    GX_TEX_ST    = 1
};

#endif // VIDEO_CORE_GX_TYPES_H_