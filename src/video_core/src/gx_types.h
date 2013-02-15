/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    gx_types.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-29
 * @brief   Types and structs used in the GX graphics API for GameCube/Wii
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

#ifndef VIDEO_CORE_GX_TYPES_H_
#define VIDEO_CORE_GX_TYPES_H_

#include "types.h"

static const int kGCEFBWidth            = 640;
static const int kGCEFBHeight           = 528;

static const int kGCMaxTextureWidth     = 1024;
static const int kGCMaxTextureHeight    = 1024;

static const int kGCMaxTextureMaps      = 8;    ///< Number of textures maps supported
static const int kGCMaxActiveTextures   = 8;    ///< Number of textures supported
static const int kGCMaxTevStages        = 16;   ///< Number of TEV stages supported
static const int kGCMaxVertexColors     = 2;    ///< Number of vertex color channels supported
static const int kGCMaxLights           = 8;    ///< Number of lights

/**
 * General struct used for describing a GX Vertex
 *
 * @remark
 * This struct is designed to hold the maximum possible information describing a GX vertex, it's
 * not necessarily all used.  Individual fields are tightly packed.  For example, if the position
 * format is GX_S16, 'position' would look like XXXXYYYY ZZZZ0000 0000000.
 */
struct GXVertex {
    u32 position[3];                ///< Position coords - XY or XYZ - Offset 0
    u32 color[2];                   ///< Color0/1 RGB/A8 - Offset 12, 16
    u32 normal[9];                  ///< Normals (3 or 9) - Offset 20
    u32 texcoords[16];              ///< Texture 0 coordinates (S or ST) - Offset 56
    u8 pm_idx;                      ///< Position matrix index - Offset 120
    u8 pad[3];
    u8 tm_idx[8];                   ///< Texture coord matrix index - Offset 124
};

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

/// GX API logic operations
enum GXLogicOp {
    GX_LO_CLEAR,
    GX_LO_AND,
    GX_LO_REVAND,
    GX_LO_COPY,
    GX_LO_INVAND,
    GX_LO_NOOP,
    GX_LO_XOR,
    GX_LO_OR,
    GX_LO_NOR,
    GX_LO_EQUIV,
    GX_LO_INV,
    GX_LO_REVOR,
    GX_LO_INVCOPY,
    GX_LO_INVOR,
    GX_LO_NAND,
    GX_LO_SET
};

/// GX API blend factors
enum GXBlendFactor {
    GX_BL_ZERO,
    GX_BL_ONE,
    GX_BL_SRCCLR,
    GX_BL_INVSRCCLR,
    GX_BL_SRCALPHA,
    GX_BL_INVSRCALPHA,
    GX_BL_DSTALPHA,
    GX_BL_INVDSTALPHA,

    GX_BL_DSTCLR = GX_BL_SRCCLR,
    GX_BL_INVDSTCLR = GX_BL_INVSRCCLR
};

/// GX API diffuse function
enum GXDiffuseFn {
    GX_DF_NONE = 0,
    GX_DF_SIGN,
    GX_DF_CLAMP
};

/**
 * @brief Vertex format for an individual vertex component (e.g. position)
 * @todo Clean up unused data here
 */
struct GXVertexFormat {
    GXCompCnt   cnt;            ///< Count
    GXCompType  fmt;            ///< Format
    u8          vcd;            ///< Type
    f32         dqf;            ///< Scale factor
};

#endif // VIDEO_CORE_GX_TYPES_H_