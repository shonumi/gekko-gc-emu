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

#define VBO_SIZE                    (1024 * 1024 * 32)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Manager

namespace gp {

extern GXVertex*   g_vbo;               ///< Pointer to VBO data (when mapped, in GPU mem)
extern u32         g_vertex_num;        ///< Current vertex number

/// Used for specifying next GX vertex is being sent to the renderer
void VertexManager_NextVertex();

/// Begin a primitive
void VertexManager_BeginPrimitive(GXPrimitive prim, int count);

/// End a primitive
void VertexManager_EndPrimitive();

/// Flush the vertex manager
void VertexManager_Flush();

/// Initialize the vertex manager
void VertexManager_Init();

/// Shutdown the vertex manager
void VertexManager_Shutdown();

} // namespace

#endif // VIDEO_CORE_VERTEX_MANAGER_H_
