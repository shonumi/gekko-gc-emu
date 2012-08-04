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
#include "memory.h"

#include "video_core.h"

#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

#define XF_VIEWPORT_ZMAX            16777216.0f

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

u32         g_xf_mem[0x800];            ///< Transformation memory
XFMemory    g_xf_regs;                  ///< XF registers
f32         g_projection_matrix[16];    ///< Decoded projection matrix
//f32         g_position_matrix[16];      ///< Decoded position matrix
//f32         g_view_matrix[16];          ///< Decoded view matrix

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
    // Set orthographic mode...
    if(XF_PROJECTION_ORTHOGRAPHIC) { 
        g_projection_matrix[0] = toFLOAT(XF_PROJECTION_A);
        g_projection_matrix[5] = toFLOAT(XF_PROJECTION_C);
        g_projection_matrix[10] = toFLOAT(XF_PROJECTION_E);
        g_projection_matrix[12] = toFLOAT(XF_PROJECTION_B);
        g_projection_matrix[13] = toFLOAT(XF_PROJECTION_D);
        g_projection_matrix[14] = toFLOAT(XF_PROJECTION_F);
        g_projection_matrix[15] = 1.0f;
        // Set perspective mode
    }else{ 
        g_projection_matrix[0] = toFLOAT(XF_PROJECTION_A);
        g_projection_matrix[5] = toFLOAT(XF_PROJECTION_C);
        g_projection_matrix[8] = toFLOAT(XF_PROJECTION_B);
        g_projection_matrix[9] = toFLOAT(XF_PROJECTION_D);
        g_projection_matrix[10] = toFLOAT(XF_PROJECTION_E);
        g_projection_matrix[11] = -1.0f;
        g_projection_matrix[14] = toFLOAT(XF_PROJECTION_F);
    }
}

void XFLoad(u32 length, u32 base_addr, u32* data) {
    int i;

    u32 addr = base_addr;

    // Write data to xf memory/registers
    switch((addr & XF_ADDR_MASK) >> 8) {
    case 0x00:
    case 0x01:
    case 0x02:
    case 0x03: // matrix transformation memory
    case 0x04: // normal transformation memory
    case 0x05: // dual texture transformation memory
    case 0x06: 
    case 0x07: // light transformation memory
        for (i = 0; i < length; i++){		
            ((f32*)g_xf_mem)[addr + i] = toFLOAT(data[i]);
        }
        break;

    case 0x10: // registers
        u8 maddr = (addr & 0xff);
        for (i = 0; i < length; i++) {
            g_xf_regs.mem[maddr + i] = data[i];
        }

        switch(maddr) {
        case 0x1a:
        case 0x1b:
        case 0x1d:
        case 0x1e:
        case 0x1f:
            XFUpdateViewport();
            break;

        case 0x20:
	        XFUpdateProjection();
            break;
        }
        break;
    }
}

/// Write data into a XF register indexed-form
void XFLoadIndexed(u8 n, u16 index, u8 length, u16 addr) {
    for (int i = 0; i < length; i++) {
        g_xf_mem[addr + i] = Memory_Read32(CP_IDX_ADDR(index, n) + (i * 4));
    }
}

/// Initialize XF
void XFInit() {
    memset(g_xf_mem, 0, sizeof(g_xf_mem));
    memset(&g_xf_regs, 0, sizeof(g_xf_regs));
}

} // namespace