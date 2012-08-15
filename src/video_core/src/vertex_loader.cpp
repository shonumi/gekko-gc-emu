/**
* Copyright (C) 2005-2012 Gekko Emulator
*
* @file    vertex_loader.h
* @author  ShizZy <shizzy247@gmail.com>
* @date    2012-03-08
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

#include "common.h"
#include "memory.h"

#include "renderer_gl3\renderer_gl3.h"

#include "video_core.h"
#include "vertex_manager.h"
#include "vertex_loader.h"
#include "fifo.h"
#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

namespace vertex_loader {

typedef void (*VertexLoaderTable)(u32, u32*);	

////////////////////////////////////////////////////////////////////////////////////////////////////
// Memory access

/// Memory read for indexed 16-bit vertex components
static __inline u16 _indexed_read_16(u32 addr) {
    if(!(addr & 1))
        return *(u16 *)(&Mem_RAM[(addr ^ 2) & RAM_MASK]);

    addr = addr & RAM_MASK;
    return (u16)(Mem_RAM[(addr + 0) ^ 3] << 8) |
        (u16)(Mem_RAM[(addr + 1) ^ 3]);
}

/// Memory read for indexed 32-bit vertex components
static __inline u32 _indexed_read_32(u32 addr) {
    addr &= RAM_MASK;
    if(!(addr & 3))
        return *(u32 *)(&Mem_RAM[addr]);

    return ((u32)Mem_RAM[(addr + 0) ^ 3] << 24) |
        ((u32)Mem_RAM[(addr + 1) ^ 3] << 16) |
        ((u32)Mem_RAM[(addr + 2) ^ 3] << 8) |
        ((u32)Mem_RAM[(addr + 3) ^ 3]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex decoding

/// Unknown position vertex component
static void __pos_unknown(u32 addr, u32* data) {
    _ASSERT_MSG(TGP, 0, "Unknown vertex position - count: %d format: %d", 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_count, 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_format);
}

/// Unknown color vertex component
static void __col_unknown(u32 addr, u32* data) {
    _ASSERT_MSG(TGP, 0, "Unknown vertex color - count: %d format: %d", 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_count, 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_format);
}

/// Unknown normal vertex component
static void __normal_unknown(u32 addr, u32* data) {
    _ASSERT_MSG(TGP, 0, "Unknown vertex normal - count: %d format: %d", 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].normal_count, 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].normal_format);
}

/// Unknown texture coordinate vertex component
static void __texcoord_unknown(u32 addr, u32* data) {
    _ASSERT_MSG(TGP, 0, "Unknown texture coordinate normal")
}

/// Direct vertex component 1 byte
static void __direct_byte_1(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::FifoPop8();
}

/// Direct vertex component 2 bytes
static void __direct_byte_2(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::FifoPop8();
    v[1] = gp::FifoPop8();
}

/// Direct vertex component 3 bytes
static void __direct_byte_3(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::FifoPop8();
    v[1] = gp::FifoPop8();
    v[2] = gp::FifoPop8();
}

/// Direct vertex component 9 bytes
static void __direct_byte_9(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::FifoPop8();
    v[1] = gp::FifoPop8();
    v[2] = gp::FifoPop8();
    v[3] = gp::FifoPop8();
    v[4] = gp::FifoPop8();
    v[5] = gp::FifoPop8();
    v[6] = gp::FifoPop8();
    v[7] = gp::FifoPop8();
    v[8] = gp::FifoPop8();
}

/// Direct vertex component 1 short
static void __direct_short_1(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::FifoPop16();
}

/// Direct vertex component 2 shorts
static void __direct_short_2(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::FifoPop16();
    v[1] = gp::FifoPop16();
}

/// Direct vertex component 3 shorts
static void __direct_short_3(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::FifoPop16();
    v[1] = gp::FifoPop16();
    v[2] = gp::FifoPop16();
}

/// Direct vertex component 9 shorts
static void __direct_short_9(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::FifoPop16();
    v[1] = gp::FifoPop16();
    v[2] = gp::FifoPop16();
    v[3] = gp::FifoPop16();
    v[4] = gp::FifoPop16();
    v[5] = gp::FifoPop16();
    v[6] = gp::FifoPop16();
    v[7] = gp::FifoPop16();
    v[8] = gp::FifoPop16();
}

/// Direct vertex component 1 float
static void __direct_float_1(u32 addr, u32* dest) {
    dest[0] = gp::FifoPop32();
}

/// Direct vertex component 2 floats
static void __direct_float_2(u32 addr, u32* dest) {
    dest[0] = gp::FifoPop32();
    dest[1] = gp::FifoPop32();
}

/// Direct vertex component 3 floats
static void __direct_float_3(u32 addr, u32* dest) {
    dest[0] = gp::FifoPop32();
    dest[1] = gp::FifoPop32();
    dest[2] = gp::FifoPop32();
}

/// Direct vertex component 9 floats
static void __direct_float_9(u32 addr, u32* dest) {
    dest[0] = gp::FifoPop32();
    dest[1] = gp::FifoPop32();
    dest[2] = gp::FifoPop32();
    dest[3] = gp::FifoPop32();
    dest[4] = gp::FifoPop32();
    dest[5] = gp::FifoPop32();
    dest[6] = gp::FifoPop32();
    dest[7] = gp::FifoPop32();
    dest[8] = gp::FifoPop32();
}

/// Indexed vertex component 1 byte
static void __indexed_byte_1(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = Mem_RAM[addr & RAM_MASK];
}

/// Indexed vertex component 2 bytes
static void __indexed_byte_2(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    u16 temp = _indexed_read_16(addr);
    v[0] = (u8)((temp >> 8) & 0xFF);
    v[1] = (u8)(temp & 0xFF);
}

/// Indexed vertex component 3 bytes
static void __indexed_byte_3(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    u32 temp = _indexed_read_32(addr);
    v[0] = (u8)((temp >> 24) & 0xFF);
    v[1] = (u8)((temp >> 16) & 0xFF);
    v[2] = (u8)((temp >> 8) & 0xFF);
}

/// Indexed vertex component 9 bytes
static void __indexed_byte_9(u32 addr, u32* dest) {
    // Unimplemented
    LOG_INFO(TGP, "Unimplemented method %s (%s line %s)", __FUNCTION__, __FILE__, __LINE__);
}

/// Indexed vertex component 1 short
static void __indexed_short_1(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = _indexed_read_16(addr);
}

/// Indexed vertex component 2 shorts
static void __indexed_short_2(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    u32 temp = _indexed_read_32(addr);
    v[0] = (temp >> 16);
    v[1] = (temp & 0xFFFF);
}

/// Indexed vertex component 3 shorts
static void __indexed_short_3(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    u32 temp = _indexed_read_32(addr);
    v[0] = (temp >> 16);
    v[1] = (temp & 0xFFFF);
    v[2] = _indexed_read_16(addr + 4);
}

/// Indexed vertex component 9 shorts
static void __indexed_short_9(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = _indexed_read_16(addr);
    v[1] = _indexed_read_16(addr + 2);
    v[2] = _indexed_read_16(addr + 4);
    v[3] = _indexed_read_16(addr + 6);
    v[4] = _indexed_read_16(addr + 8);
    v[5] = _indexed_read_16(addr + 10);
    v[6] = _indexed_read_16(addr + 12);
    v[7] = _indexed_read_16(addr + 14);
    v[8] = _indexed_read_16(addr + 16);
}

/// Indexed vertex component 1 float
static void __indexed_float_1(u32 addr, u32* dest) {
    dest[0] = _indexed_read_32(addr);
}

/// Indexed vertex component 2 floats
static void __indexed_float_2(u32 addr, u32* dest) {
    dest[0] = _indexed_read_32(addr);
    dest[1] = _indexed_read_32(addr + 4);
}

/// Indexed vertex component 3 floats
static void __indexed_float_3(u32 addr, u32* dest) {
    dest[0] = _indexed_read_32(addr);
    dest[1] = _indexed_read_32(addr + 4);
    dest[2] = _indexed_read_32(addr + 8);
}

/// Indexed vertex component 9 floats
static void __indexed_float_9(u32 addr, u32* dest) {
    dest[0] = _indexed_read_32(addr);
    dest[1] = _indexed_read_32(addr + 4);
    dest[2] = _indexed_read_32(addr + 8);
    dest[3] = _indexed_read_32(addr + 12);
    dest[4] = _indexed_read_32(addr + 16);
    dest[5] = _indexed_read_32(addr + 20);
    dest[6] = _indexed_read_32(addr + 24);
    dest[7] = _indexed_read_32(addr + 28);
    dest[8] = _indexed_read_32(addr + 32);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Lookup tables for decoding vertex components

// Position decoding
// -----------------

VertexLoaderTable LookupPositionDirect[0x10] = {
    __direct_byte_2,    __direct_byte_2,    __direct_short_2,   __direct_short_2,
    __direct_float_2,   __pos_unknown,      __pos_unknown,      __pos_unknown,     
    __direct_byte_3,    __direct_byte_3,    __direct_short_3,   __direct_short_3, 
    __direct_float_3,   __pos_unknown,      __pos_unknown,      __pos_unknown
};
VertexLoaderTable LookupPositionIndexed[0x10] = {
    __indexed_byte_2,   __indexed_byte_2,   __indexed_short_2,  __indexed_short_2,
    __indexed_float_2,  __pos_unknown,      __pos_unknown,      __pos_unknown,     
    __indexed_byte_3,   __indexed_byte_3,   __indexed_short_3,  __indexed_short_3, 
    __indexed_float_3,  __pos_unknown,      __pos_unknown,      __pos_unknown
};

// Color decoding
// --------------

VertexLoaderTable LookupColorDirect[0x10] = {
    __direct_short_1,   __direct_byte_3,    __direct_float_1,   __direct_short_1,
    __direct_byte_3,    __direct_float_1,   __col_unknown,      __col_unknown,
    __direct_short_1,   __direct_byte_3,    __direct_float_1,   __direct_short_1,
    __direct_byte_3,    __direct_float_1,   __col_unknown,      __col_unknown,
};
VertexLoaderTable LookupColorIndexed[0x10] = {
    __indexed_short_1,  __indexed_byte_3,   __indexed_float_1,  __indexed_short_1,
    __indexed_byte_3,   __indexed_float_1,  __col_unknown,      __col_unknown,
    __indexed_short_1,  __indexed_byte_3,   __indexed_float_1,  __indexed_short_1,
    __indexed_byte_3,   __indexed_float_1,  __col_unknown,      __col_unknown
};                                          

// Normal decoding
// ---------------

VertexLoaderTable LookupNormalDirect[0x10] = {
    __direct_byte_3,    __direct_byte_3,    __direct_short_3,   __direct_short_3,
    __direct_float_3,   __normal_unknown,   __normal_unknown,   __normal_unknown,
    __direct_byte_9,    __direct_byte_9,    __direct_short_9,   __direct_short_9,
    __direct_float_9,   __normal_unknown,   __normal_unknown,   __normal_unknown
};
VertexLoaderTable LookupNormalIndexed[0x10] = {
    __indexed_byte_3,   __indexed_byte_3,   __indexed_short_3,  __indexed_short_3,
    __indexed_float_3,  __normal_unknown,   __normal_unknown,   __normal_unknown,
    __indexed_byte_9,   __indexed_byte_9,   __indexed_short_9,  __indexed_short_9,
    __indexed_float_9,  __normal_unknown,   __normal_unknown,   __normal_unknown
};

// Texture coordinate decoding
// ---------------------------

VertexLoaderTable LookupTexCoordDirect[0x10] = {
    __direct_byte_1,    __direct_byte_1,    __direct_short_1,   __direct_short_1,
    __direct_float_1,   __texcoord_unknown, __texcoord_unknown, __texcoord_unknown,   
    __direct_byte_2,    __direct_byte_2,    __direct_short_2,   __direct_short_2,
    __direct_float_2,   __texcoord_unknown, __texcoord_unknown, __texcoord_unknown
};
VertexLoaderTable LookupTexCoordIndexed[0x10] = {
    __indexed_byte_1,   __indexed_byte_1,   __indexed_short_1,  __indexed_short_1, 
    __indexed_float_1,  __texcoord_unknown, __texcoord_unknown, __texcoord_unknown, 
    __indexed_byte_2,   __indexed_byte_2,   __indexed_short_2,  __indexed_short_2, 
    __indexed_float_2,  __texcoord_unknown, __texcoord_unknown, __texcoord_unknown
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Primitive decoding

/// Gets the size of the next vertex to be decoded
int GetVertexSize() {
    u16 size = 0;

    if (gp::g_cp_regs.vcd_lo[0].pos_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex0_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex1_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex2_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex3_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex4_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex5_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex6_midx_enable) size += 1;
    if (gp::g_cp_regs.vcd_lo[0].tex7_midx_enable) size += 1;

    // Vertex position size
    switch(gp::g_cp_regs.vcd_lo[0].position) {	
    case CP_DIRECT:
        size+=gp::kVertexPositionSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_pos()];
        break;
    case CP_INDEX8: 
        size+=1; 
        break;
    case CP_INDEX16: 
        size+=2; 
        break;
    }
    // Vertex normal size
    switch(gp::g_cp_regs.vcd_lo[0].normal) {	
    case CP_DIRECT:
        size+=gp::kVertexNormalSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_normal()];
        break;
    case CP_INDEX8:
        size+=1; 
        break;
    case CP_INDEX16:
        size+=2; 
        break;
    }
    // Vertex color 0 size
    switch(gp::g_cp_regs.vcd_lo[0].color0) {	
    case CP_DIRECT:
        size+=gp::kVertexColorSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_col0()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Vertex color 1 size
    switch(gp::g_cp_regs.vcd_lo[0].color1) {	
    case CP_DIRECT:
        size+=gp::kVertexColorSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_col1()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 0 size
    switch(gp::g_cp_regs.vcd_hi[0].tex0_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_tex0()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 1 size
    switch(gp::g_cp_regs.vcd_hi[0].tex1_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex1()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 2 size
    switch(gp::g_cp_regs.vcd_hi[0].tex2_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex2()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 3 size
    switch(gp::g_cp_regs.vcd_hi[0].tex3_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex3()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 4 size
    switch(gp::g_cp_regs.vcd_hi[0].tex4_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex4()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 5 size
    switch(gp::g_cp_regs.vcd_hi[0].tex5_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex5()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 6 size
    switch(gp::g_cp_regs.vcd_hi[0].tex6_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex6()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 7 size
    switch(gp::g_cp_regs.vcd_hi[0].tex7_coord) {
    case CP_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex7()];
        break;
    case CP_INDEX8:
        size+=1;
        break;
    case CP_INDEX16:
        size+=2;
        break;
    }
    return size;
}

/// Decode a primitive that's currently in the FIFO
void DecodePrimitive(GXPrimitive type, int count) {

    CPVatRegA* vat_a = &gp::g_cp_regs.vat_reg_a[gp::g_cur_vat];
    CPVatRegB* vat_b = &gp::g_cp_regs.vat_reg_b[gp::g_cur_vat];
    CPVatRegC* vat_c = &gp::g_cp_regs.vat_reg_c[gp::g_cur_vat];

    u32 pos_base        = gp::g_cp_regs.array_base[0].addr_base;
    u8  pos_stride      = gp::g_cp_regs.array_stride[0].addr_stride;
    u32 normal_base     = gp::g_cp_regs.array_base[1].addr_base;
    u8  normal_stride   = gp::g_cp_regs.array_stride[1].addr_stride;
    u32 col0_base       = gp::g_cp_regs.array_base[2].addr_base;
    u8  col0_stride     = gp::g_cp_regs.array_stride[2].addr_stride;
    u32 col1_base       = gp::g_cp_regs.array_base[3].addr_base;
    u8  col1_stride     = gp::g_cp_regs.array_stride[3].addr_stride;
    u32 tex0_base       = gp::g_cp_regs.array_base[4].addr_base;
    u8  tex0_stride     = gp::g_cp_regs.array_stride[4].addr_stride;
    u32 tex1_base       = gp::g_cp_regs.array_base[5].addr_base;
    u8  tex1_stride     = gp::g_cp_regs.array_stride[5].addr_stride;
    u32 tex2_base       = gp::g_cp_regs.array_base[6].addr_base;
    u8  tex2_stride     = gp::g_cp_regs.array_stride[6].addr_stride;
    u32 tex3_base       = gp::g_cp_regs.array_base[7].addr_base;
    u8  tex3_stride     = gp::g_cp_regs.array_stride[7].addr_stride;
    u32 tex4_base       = gp::g_cp_regs.array_base[8].addr_base;
    u8  tex4_stride     = gp::g_cp_regs.array_stride[8].addr_stride;
    u32 tex5_base       = gp::g_cp_regs.array_base[9].addr_base;
    u8  tex5_stride     = gp::g_cp_regs.array_stride[9].addr_stride;
    u32 tex6_base       = gp::g_cp_regs.array_base[10].addr_base;
    u8  tex6_stride     = gp::g_cp_regs.array_stride[10].addr_stride;
    u32 tex7_base       = gp::g_cp_regs.array_base[11].addr_base;
    u8  tex7_stride     = gp::g_cp_regs.array_stride[11].addr_stride;

    // Configure renderer to begin a new primitive
    vertex_manager::BeginPrimitive(type, count);

    // Decode and apply texture
    /*
    glActiveTexture(GL_TEXTURE0 + 0);
    if(gp::g_bp_regs.tevorder[0 >> 1].get_enable(0)) gp::LoadTexture(0);
    glActiveTexture(GL_TEXTURE0 + 1);
    if(gp::g_bp_regs.tevorder[1 >> 1].get_enable(1)) gp::LoadTexture(1);
    glActiveTexture(GL_TEXTURE0 + 2);
    if(gp::g_bp_regs.tevorder[2 >> 1].get_enable(2)) gp::LoadTexture(2);
    glActiveTexture(GL_TEXTURE0 + 3);
    if(gp::g_bp_regs.tevorder[3 >> 1].get_enable(3)) gp::LoadTexture(3);
    glActiveTexture(GL_TEXTURE0 + 4);
    if(gp::g_bp_regs.tevorder[4 >> 1].get_enable(4)) gp::LoadTexture(4);
    glActiveTexture(GL_TEXTURE0 + 5);
    if(gp::g_bp_regs.tevorder[5 >> 1].get_enable(5)) gp::LoadTexture(5);
    glActiveTexture(GL_TEXTURE0 + 6);
    if(gp::g_bp_regs.tevorder[6 >> 1].get_enable(6)) gp::LoadTexture(6);
    glActiveTexture(GL_TEXTURE0 + 7);
    if(gp::g_bp_regs.tevorder[7 >> 1].get_enable(7)) gp::LoadTexture(7);
    */

    for (int i = 0; i < 8; i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        gp::LoadTexture(i);
    }
    // Set renderer types
    video_core::g_renderer->VertexPosition_SetType((GXCompType)vat_a->pos_format,
        (GXCompCnt)vat_a->pos_count);
    video_core::g_renderer->VertexColor_SetType(0, (GXCompType)vat_a->col0_format, 
        (GXCompCnt)vat_a->col0_count);
    video_core::g_renderer->VertexColor_SetType(1, (GXCompType)vat_a->col1_format, 
        (GXCompCnt)vat_a->col1_count);
    video_core::g_renderer->VertexTexcoord_SetType(0, (GXCompType)vat_a->tex0_format,
        (GXCompCnt)vat_a->tex0_count);
    video_core::g_renderer->VertexTexcoord_SetType(1, (GXCompType)vat_b->tex1_format,
        (GXCompCnt)vat_b->tex1_count);
    video_core::g_renderer->VertexTexcoord_SetType(2, (GXCompType)vat_b->tex2_format,
        (GXCompCnt)vat_b->tex2_count);
    video_core::g_renderer->VertexTexcoord_SetType(3, (GXCompType)vat_b->tex3_format,
        (GXCompCnt)vat_b->tex3_count);
    video_core::g_renderer->VertexTexcoord_SetType(4, (GXCompType)vat_b->tex4_format,
        (GXCompCnt)vat_b->tex4_count);
    video_core::g_renderer->VertexTexcoord_SetType(5, (GXCompType)vat_c->tex5_format,
        (GXCompCnt)vat_c->tex5_count);
    video_core::g_renderer->VertexTexcoord_SetType(6, (GXCompType)vat_c->tex6_format,
        (GXCompCnt)vat_c->tex6_count);
    video_core::g_renderer->VertexTexcoord_SetType(7, (GXCompType)vat_c->tex7_format,
        (GXCompCnt)vat_c->tex7_count);

    for (int i = 0; i < count; i++) {

        // Matrix indices
        if (gp::g_cp_regs.vcd_lo[0].pos_midx_enable)
            vertex_manager::g_vbo->pm_idx = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex0_midx_enable)
            vertex_manager::g_vbo->tm_idx[0] = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex1_midx_enable)
            vertex_manager::g_vbo->tm_idx[1] = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex2_midx_enable)
            vertex_manager::g_vbo->tm_idx[2] = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex3_midx_enable)
            vertex_manager::g_vbo->tm_idx[3] = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex4_midx_enable) 
            vertex_manager::g_vbo->tm_idx[4] = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex5_midx_enable)
            vertex_manager::g_vbo->tm_idx[5] = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex6_midx_enable)
            vertex_manager::g_vbo->tm_idx[6] = gp::FifoPop8();
        if (gp::g_cp_regs.vcd_lo[0].tex7_midx_enable)
            vertex_manager::g_vbo->tm_idx[7] = gp::FifoPop8();

        // Decode position
        switch (gp::g_cp_regs.vcd_lo[0].position) {
        case CP_DIRECT:
            LookupPositionDirect[vat_a->get_pos()](0, vertex_manager::g_vbo->position);
            break;
        case CP_INDEX8:
            LookupPositionIndexed[vat_a->get_pos()](pos_base + (gp::FifoPop8() * pos_stride),
                vertex_manager::g_vbo->position);
            break;
        case CP_INDEX16:
            LookupPositionIndexed[vat_a->get_pos()](pos_base + (gp::FifoPop16() * pos_stride),
                vertex_manager::g_vbo->position);
            break;
        }
        // Decode normal
        switch (gp::g_cp_regs.vcd_lo[0].normal) {
        case CP_DIRECT:
            LookupNormalDirect[vat_a->get_normal()](0, vertex_manager::g_vbo->normal);
            break;
        case CP_INDEX8:
            LookupNormalIndexed[vat_a->get_normal()](normal_base + (gp::FifoPop8() * normal_stride),
                vertex_manager::g_vbo->normal);
            break;
        case CP_INDEX16:
            LookupNormalIndexed[vat_a->get_normal()](normal_base + (gp::FifoPop16() * normal_stride),
                vertex_manager::g_vbo->normal);
            break;
        }
        // Decode color 0
        switch (gp::g_cp_regs.vcd_lo[0].color0) {
        case CP_NOT_PRESENT:
            vertex_manager::g_vbo->color[0] = 0xffffffff;
            break;
        case CP_DIRECT:
            LookupColorDirect[vat_a->get_col0()](0, &vertex_manager::g_vbo->color[0]);
            break;
        case CP_INDEX8:
            LookupColorIndexed[vat_a->get_col0()](col0_base + (gp::FifoPop8() * col0_stride),
                &vertex_manager::g_vbo->color[0]);
            break;
        case CP_INDEX16:
            LookupColorIndexed[vat_a->get_col0()](col0_base + (gp::FifoPop16() * col0_stride),
                &vertex_manager::g_vbo->color[0]);
            break;
        }
        // Decode color 1
        switch (gp::g_cp_regs.vcd_lo[0].color1) {
        case CP_NOT_PRESENT:
            vertex_manager::g_vbo->color[1] = 0xffffffff;
            break;
        case CP_DIRECT:
            LookupColorDirect[vat_a->get_col1()](0, &vertex_manager::g_vbo->color[1]);
            break;
        case CP_INDEX8:
            LookupColorIndexed[vat_a->get_col1()](col1_base + (gp::FifoPop8() * col1_stride),
                &vertex_manager::g_vbo->color[1]);
            break;
        case CP_INDEX16:
            LookupColorIndexed[vat_a->get_col1()](col1_base + (gp::FifoPop16() * col1_stride),
                &vertex_manager::g_vbo->color[1]);
            break;
        }
        // Decode texcoord 0
        if (gp::g_cp_regs.vcd_hi[0].tex0_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex0_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_a->get_tex0()](0, &vertex_manager::g_vbo->texcoords[0 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_a->get_tex0()](tex0_base + (gp::FifoPop8() * tex0_stride), 
                    &vertex_manager::g_vbo->texcoords[0 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_a->get_tex0()](tex0_base + (gp::FifoPop16() * tex0_stride),
                    &vertex_manager::g_vbo->texcoords[0 << 1]);
                break;
            }
        }
        // Decode texcoord 1
        if (gp::g_cp_regs.vcd_hi[0].tex1_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex1_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex1()](0, &vertex_manager::g_vbo->texcoords[1 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex1()](tex1_base + (gp::FifoPop8() * tex1_stride),
                    &vertex_manager::g_vbo->texcoords[1 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex1()](tex1_base + (gp::FifoPop16() * tex1_stride),
                    &vertex_manager::g_vbo->texcoords[1 << 1]);
                break;
            }
        }
        // Decode texcoord 2
        if (gp::g_cp_regs.vcd_hi[0].tex2_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex2_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex2()](0, &vertex_manager::g_vbo->texcoords[2 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex2()](tex2_base + (gp::FifoPop8() * tex2_stride),
                    &vertex_manager::g_vbo->texcoords[2 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex2()](tex2_base + (gp::FifoPop16() * tex2_stride),
                    &vertex_manager::g_vbo->texcoords[2 << 1]);
                break;
            }
        }
        // Decode texcoord 3
        if (gp::g_cp_regs.vcd_hi[0].tex3_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex3_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex3()](0, &vertex_manager::g_vbo->texcoords[3 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex3()](tex3_base + (gp::FifoPop8() * tex3_stride),
                    &vertex_manager::g_vbo->texcoords[3 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex3()](tex3_base + (gp::FifoPop16() * tex3_stride),
                    &vertex_manager::g_vbo->texcoords[3 << 1]);
                break;
            }
        }
        // Decode texcoord 4
        if (gp::g_cp_regs.vcd_hi[0].tex4_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex4_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex4()](0, &vertex_manager::g_vbo->texcoords[4 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex4()](tex4_base + (gp::FifoPop8() * tex4_stride),
                    &vertex_manager::g_vbo->texcoords[4 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex4()](tex4_base + (gp::FifoPop16() * tex4_stride),
                    &vertex_manager::g_vbo->texcoords[4 << 1]);
                break;
            }
        }
        // Decode texcoord 5
        if (gp::g_cp_regs.vcd_hi[0].tex5_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex5_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex5()](0, &vertex_manager::g_vbo->texcoords[5 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex5()](tex5_base + (gp::FifoPop8() * tex5_stride),
                    &vertex_manager::g_vbo->texcoords[5 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex5()](tex5_base + (gp::FifoPop16() * tex5_stride),
                    &vertex_manager::g_vbo->texcoords[5 << 1]);
                break;
            }
        }
        // Decode texcoord 6
        if (gp::g_cp_regs.vcd_hi[0].tex6_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex6_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex6()](0, &vertex_manager::g_vbo->texcoords[6 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex6()](tex6_base + (gp::FifoPop8() * tex6_stride),
                    &vertex_manager::g_vbo->texcoords[6 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex6()](tex6_base + (gp::FifoPop16() * tex6_stride),
                    &vertex_manager::g_vbo->texcoords[6 << 1]);
                break;
            }
        }
        // Decode texcoord 7
        if (gp::g_cp_regs.vcd_hi[0].tex7_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex7_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex7()](0, &vertex_manager::g_vbo->texcoords[7 << 1]);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex7()](tex7_base + (gp::FifoPop8() * tex7_stride),
                    &vertex_manager::g_vbo->texcoords[7 << 1]);
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex7()](tex7_base + (gp::FifoPop16() * tex7_stride),
                    &vertex_manager::g_vbo->texcoords[7 << 1]);
                break;
            }
        }   
        vertex_manager::NextVertex();
    }
    vertex_manager::EndPrimitive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// Initialize the Vertex Loader
void Init() {
}

/// Shutdown the Vertex Loader
void Shutdown() {
}

} // namespace
