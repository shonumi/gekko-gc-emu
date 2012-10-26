/*!
* Copyright (C) 2005-2012 Gekko Emulator
*
* \file    xf_mem.h
* \author  ShizZy <shizzy247@gmail.com>
* \date    2012-03-12
* \brief   Implementation of CXF for the graphics processor
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

#include <GL/glew.h>

#include "common.h"
#include "crc.h"
#include "memory.h"

#include "renderer_gl3/renderer_gl3.h"
#include "renderer_gl3/shader_manager.h"

#include "video_core.h"

#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

#define XF_VIEWPORT_ZMAX            16777215.0f

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

u32         g_xf_mem[0x800];            ///< Transformation memory
XFMemory    g_xf_regs;                  ///< XF registers
f32         g_projection_matrix[16];    ///< Decoded projection matrix

void XFUpdateViewport() {
    int scissorXOff = g_bp_regs.scissor_offset.x * 2;
    int scissorYOff = g_bp_regs.scissor_offset.y * 2;

    // TODO: ceil, floor or just cast to int?
    int x = ((int)ceil(g_xf_regs.viewport.x_orig - g_xf_regs.viewport.wd - (float)scissorXOff));
    int y = ((int)ceil((float)480 - g_xf_regs.viewport.y_orig + g_xf_regs.viewport.ht + (float)scissorYOff));
    int width = ((int)ceil(2.0f * g_xf_regs.viewport.wd));
    int height = ((int)ceil(-2.0f * g_xf_regs.viewport.ht));
    double znear = (g_xf_regs.viewport.far_z - g_xf_regs.viewport.z_range) / XF_VIEWPORT_ZMAX;
    double zfar = g_xf_regs.viewport.far_z / 16777216.0f;
    if (width < 0) {
        x += width;
        width *= -1;
    }
    if (height < 0) {
        y += height;
        height *= -1;
    }

    // Update the view port
    video_core::g_renderer->SetViewport(x, y, width, height);
    video_core::g_renderer->SetDepthRange(znear, zfar);
}

void XFUpdateProjection() {
	t32 mtx[16] = {0};

	// is it orthographic mode...
	if(XF_PROJECTION_ORTHOGRAPHIC)
	{ 
		mtx[0]._u32 = XF_PROJECTION_A;
		mtx[5]._u32 = XF_PROJECTION_C;
		mtx[10]._u32 = XF_PROJECTION_E;
		mtx[12]._u32 = XF_PROJECTION_B;
		mtx[13]._u32 = XF_PROJECTION_D;
		mtx[14]._u32 = XF_PROJECTION_F;
		mtx[15]._f32 = 1.0f;
	// otherwise it is perspective mode
	}else{ 
		mtx[0]._u32 = XF_PROJECTION_A;
		mtx[5]._u32 = XF_PROJECTION_C;
		mtx[8]._u32 = XF_PROJECTION_B;
		mtx[9]._u32 = XF_PROJECTION_D;
		mtx[10]._u32 = XF_PROJECTION_E;
		mtx[11]._f32 = -1.0f;
		mtx[14]._u32 = XF_PROJECTION_F;
	}
    memcpy(g_projection_matrix, mtx, 16*4);
}

void XFLoad(u32 length, u32 base_addr, u32* data) {

    // Register write
    if (base_addr & 0x1000) {
        u8 addr = (base_addr & 0xff);
        memcpy(&g_xf_regs.mem[addr], data, length << 2);

        // Viewport register range
        if (addr >= 0x1A && addr <= 0x1F) {
            XFUpdateViewport();
        // Projection matrix register range
        } else if (addr >= 0x20 && addr <= 0x26) {
            XFUpdateProjection();
        }

    // Transformation memory
    } else if (base_addr < 0x100) {
        memcpy(&g_xf_mem[base_addr], data, length << 2);
        video_core::g_renderer->WriteXF(base_addr, length, data);
    }
}

/// Write data into a XF register indexed-form
void XFLoadIndexed(u8 n, u16 index, u8 length, u16 addr) {
    u32* data = (u32*)&Mem_RAM[CP_IDX_ADDR(index, n) & RAM_MASK];
    memcpy(&g_xf_mem[addr], data, length << 2);
    video_core::g_renderer->WriteXF(addr, length, data);
}

/// Initialize XF
void XFInit() {
    memset(g_xf_mem, 0, sizeof(g_xf_mem));
    memset(&g_xf_regs, 0, sizeof(g_xf_regs));
}

} // namespace