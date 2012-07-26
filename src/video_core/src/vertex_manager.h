/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    vertex_manager.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-07-23
 * @brief   Video core vertex manager
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

#ifndef VIDEO_CORE_VERTEX_MANAGER_H_
#define VIDEO_CORE_VERTEX_MANAGER_H_

#include "common.h"

#include "gx_types.h"

#define USE_GEOMETRY_SHADERS
#define VBO_SIZE                    (1024 * 1024 * 32)

namespace vertex_manager {

extern GXVertex*   g_vbo;               ///< Pointer to VBO data (when mapped, in GPU mem)
extern u32         g_vertex_num;        ///< Current vertex number
extern u8          g_cur_texcoord_ofs;  ///< Current texcoord, 0-15
extern u8          g_cur_color_ofs;     ///< Current color, 0-1

/**
 * Send a position vector to the renderer as 32-bit floating point
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
static inline void Position_SendFloat(f32* vec) {
    f32* ptr = (f32*)g_vbo->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Send a position vector to the renderer as 16-bit short (signed or unsigned)
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
static inline void Position_SendShort(u16* vec) {
    u16* ptr = (u16*)g_vbo->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Send a position vector to the renderer an 8-bit byte (signed or unsigned)
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
static inline void Position_SendByte(u8* vec) {
    u8* ptr = (u8*)g_vbo->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Send a vertex color to the renderer (RGB8 or RGBA8, as set by VertexColor_SetType)
 * @param color Color to send, packed as RRGGBBAA or RRGGBB00
 */
static inline void Color_Send(u32 rgba) {
    g_vbo->color[g_cur_color_ofs++] = rgba;
}

/**
 * Send a texcoord vector to the renderer as 32-bit floating point
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
static inline void Texcoord_SendFloat(f32* vec) {
    f32* ptr = (f32*)&(g_vbo->texcoords[g_cur_texcoord_ofs]);
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    g_cur_texcoord_ofs += 2;
}

/**
 * Send a texcoord vector to the renderer as 16-bit short (signed or unsigned)
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
static inline void Texcoord_SendShort(u16* vec) {
    u16* ptr = (u16*)&(g_vbo->texcoords[g_cur_texcoord_ofs]);
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    g_cur_texcoord_ofs += 2;
}

/**
 * Send a texcoord vector to the renderer as 8-bit byte (signed or unsigned)
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
static inline void Texcoord_SendByte(u8* vec) {
    u8* ptr = (u8*)&(g_vbo->texcoords[g_cur_texcoord_ofs]);
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    g_cur_texcoord_ofs += 2;
}

/**
 * @brief Sends position and texcoord matrix indices to the renderer
 * @param pm_idx Position matrix index
 * @param tm_idx Texture matrix indices,
 */
static inline void SendMatrixIndices(u8 pm_idx, u8 tm_idx[]) {
    g_vbo->pm_idx = pm_idx;
   /* (*g_vbo_ptr)->tm_idx[0] = tm_idx[0];
    (*g_vbo_ptr)->tm_idx[1] = tm_idx[1];
    (*g_vbo_ptr)->tm_idx[2] = tm_idx[2];
    (*g_vbo_ptr)->tm_idx[3] = tm_idx[3];
    (*g_vbo_ptr)->tm_idx[4] = tm_idx[4];
    (*g_vbo_ptr)->tm_idx[5] = tm_idx[5];
    (*g_vbo_ptr)->tm_idx[6] = tm_idx[6];
    (*g_vbo_ptr)->tm_idx[7] = tm_idx[7]; */
}

/// Used for specifying next GX vertex is being sent to the renderer
static inline void NextVertex() {
    g_cur_texcoord_ofs = 0;
    g_cur_color_ofs = 0;
    g_vbo++;
    g_vertex_num++;
}

/// Begin a primitive
void BeginPrimitive(GXPrimitive prim, int count);

/// End a primitive
void EndPrimitive();

/// Flush the vertex manager
void Flush();

/// Initialize the vertex manager
void Init();

} // namespace


#endif // VIDEO_CORE_VERTEX_MANAGER_H_