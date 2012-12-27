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

namespace gp {

GXVertex*   g_hardware_vbo = NULL;  ///< Pointer to hardware VBO (always GPU mem)
GXVertex*   g_vbo = NULL;           ///< Pointer to VBO data (can be GPU or sys mem)
GXVertex    g_quads_vbo[4];         ///< Temporary storage for a quad in the pipeline
u32         g_vbo_offset = 0;       ///< Offset into VBO of current vertex writes
u32         g_vertex_num = 0;       ///< Current vertex number
int         g_convert_quads_to_triangles = 0;
int g_quad_counter = 0;

void VertexManager_NextVertex() {
    // Mark the vertex position XF index as "used" to renderer
    video_core::g_renderer->VertexPosition_UseIndexXF(g_vbo->pm_idx);

    g_quad_counter++;

    // Convert quads to triangles on every 4th vertex
    if (g_convert_quads_to_triangles && (g_quad_counter == 4)) {
        g_quad_counter = 0;
        g_vbo -= 3;

        g_hardware_vbo[0] = g_vbo[0];
        g_hardware_vbo[1] = g_vbo[1];
        g_hardware_vbo[2] = g_vbo[2];
        
        g_hardware_vbo[3] = g_vbo[2];
        g_hardware_vbo[4] = g_vbo[3];
        g_hardware_vbo[5] = g_vbo[0];
        
        g_hardware_vbo += 6;
        g_vbo = g_quads_vbo;
        g_vertex_num += 3;
    } else {
        g_vbo++;
        g_vertex_num++;
    }
}

/// Begin a primitive
void VertexManager_BeginPrimitive(GXPrimitive prim, int count) {
    g_vertex_num = 0;
    g_quad_counter = 0;
    
    if (GX_QUADS == prim) {
        prim = GX_TRIANGLES;    // Most hardware doesn't support quads, so convert to triangles
        count += count >> 1;    // Every 4 verts (quad) will be converted to 6 verts (triangle)
        g_convert_quads_to_triangles = 1;
        g_vbo = g_quads_vbo;
        video_core::g_renderer->BeginPrimitive(prim, count, &g_hardware_vbo, g_vbo_offset);
    } else {
        video_core::g_renderer->BeginPrimitive(prim, count, &g_vbo, g_vbo_offset);
    }
}

/// End a primitive
void VertexManager_EndPrimitive() {
    video_core::g_renderer->EndPrimitive(g_vbo_offset, g_vertex_num);
    g_vbo_offset += g_vertex_num;
    g_convert_quads_to_triangles = 0;
}

/// Flush the vertex manager
void VertexManager_Flush() {
    g_vbo_offset = 0;
    g_vertex_num = 0;
}

/// Initialize the vertex manager
void VertexManager_Init() { 
    g_vbo = NULL;
    g_vbo_offset = 0;
    g_vertex_num = 0;
    LOG_NOTICE(TGP, "vertex manager initialized ok");
    return;
}

/// Shutdown the vertex manager
void VertexManager_Shutdown() {
}

} // namespace
