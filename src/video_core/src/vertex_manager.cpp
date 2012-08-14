/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    vertex_manager.cpp
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

#include "renderer_gl3/renderer_gl3.h"
#include "vertex_manager.h"
#include "video_core.h"

namespace vertex_manager {

GXVertex*   g_vbo = NULL;           ///< Pointer to VBO data (when mapped, in GPU mem)
u32         g_vbo_offset = 0;       ///< Offset into VBO of current vertex writes
u32         g_vertex_num = 0;       ///< Current vertex number

/// Begin a primitive
void BeginPrimitive(GXPrimitive prim, int count) {
    g_vertex_num = 0;
    video_core::g_renderer->BeginPrimitive(prim, count, &g_vbo, g_vbo_offset);
}

/// End a primitive
void EndPrimitive() {
    video_core::g_renderer->EndPrimitive(g_vbo_offset, g_vertex_num);
    g_vbo_offset += g_vertex_num;
}

/// Flush the vertex manager
void Flush() {
    g_vbo_offset = 0;
    g_vertex_num = 0;
}

/// Initialize the vertex manager
void Init() { 
    g_vbo = NULL;
    g_vbo_offset = 0;
    g_vertex_num = 0;
    LOG_NOTICE(TGP, "vertex_manager initialized ok");
    return;
}

} // namespace
