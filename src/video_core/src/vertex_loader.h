/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    vertex_loader.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-10
 * @brief   Loads and decodes vertex data from CP mem
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

#ifndef VIDEO_CORE_VERTEX_LOADER_H_
#define VIDEO_CORE_VERTEX_LOADER_H_

#include "gx_types.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Loader

namespace gp {

/// Vertex component
struct VertexComponent {
    GXAttrType  attr_type;  ///< Attribute type (direct, indexed8, etc.)
    GXCompType  comp_type;  ///< Component type (u32, f32, etc.)
    GXCompCnt   comp_count; ///< Component count (XY, XYZ, RGB, RGBA, etc.)
};

/// Vertex state
struct VertexState {
    VertexComponent pos;
    VertexComponent col[kGCMaxColorChannels];
    VertexComponent nrm;
    VertexComponent tex[kGCMaxActiveTextures];
};

/**
 * @brief Decode a primitive type
 * @param type Type of primitive (e.g. points, lines, triangles, etc.)
 * @param count Number of vertices
 */
void VertexLoader_DecodePrimitive(GXPrimitive type, int count); 

/**
 * @brief Gets the size of the next vertex to be decoded
 * @return Size of the next vertex to be decoded
 */
int VertexLoader_GetVertexSize();

/// Initialize the Vertex Loader
void VertexLoader_Init();

/// Shutdown the Vertex Loader
void VertexLoader_Shutdown();

} // namespace

#endif // VIDEO_CORE_VERTEX_LOADER_H_
