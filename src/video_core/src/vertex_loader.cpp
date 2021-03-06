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

#include "renderer_gl3/renderer_gl3.h"

#include "video_core.h"
#include "vertex_manager.h"
#include "vertex_loader.h"
#include "fifo.h"
#include "cp_mem.h"
#include "xf_mem.h"

namespace gp {

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
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].pos_type);
}

/// Unknown color vertex component
static void __col_unknown(u32 addr, u32* data) {
    _ASSERT_MSG(TGP, 0, "Unknown vertex color - count: %d format: %d", 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_count, 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].col0_type);
}

/// Unknown normal vertex component
static void __normal_unknown(u32 addr, u32* data) {
    _ASSERT_MSG(TGP, 0, "Unknown vertex normal - count: %d format: %d", 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].normal_count, 
        gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].normal_type);
}

/// Unknown texture coordinate vertex component
static void __texcoord_unknown(u32 addr, u32* data) {
    _ASSERT_MSG(TGP, 0, "Unknown texture coordinate normal")
}

/// Direct vertex component 1 byte
static void __direct_byte_1(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::Fifo_Pop8();
}

/// Direct vertex component 2 bytes
static void __direct_byte_2(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::Fifo_Pop8();
    v[1] = gp::Fifo_Pop8();
}

/// Direct vertex component 3 bytes
static void __direct_byte_3(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::Fifo_Pop8();
    v[1] = gp::Fifo_Pop8();
    v[2] = gp::Fifo_Pop8();
}

/// Direct vertex component 9 bytes
static void __direct_byte_9(u32 addr, u32* dest) {
    u8* v = (u8*)dest;
    v[0] = gp::Fifo_Pop8();
    v[1] = gp::Fifo_Pop8();
    v[2] = gp::Fifo_Pop8();
    v[3] = gp::Fifo_Pop8();
    v[4] = gp::Fifo_Pop8();
    v[5] = gp::Fifo_Pop8();
    v[6] = gp::Fifo_Pop8();
    v[7] = gp::Fifo_Pop8();
    v[8] = gp::Fifo_Pop8();
    _ASSERT_MSG(TGP, 0, "Unimplemented method %s (%s line %s)", __FUNCTION__, __FILE__, __LINE__);
}

/// Direct vertex component 1 short
static void __direct_short_1(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::Fifo_Pop16();
}

/// Direct vertex component 2 shorts
static void __direct_short_2(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::Fifo_Pop16();
    v[1] = gp::Fifo_Pop16();
}

/// Direct vertex component 3 shorts
static void __direct_short_3(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::Fifo_Pop16();
    v[1] = gp::Fifo_Pop16();
    v[2] = gp::Fifo_Pop16();
}

/// Direct vertex component 9 shorts
static void __direct_short_9(u32 addr, u32* dest) {
    u16* v = (u16*)dest;
    v[0] = gp::Fifo_Pop16();
    v[1] = gp::Fifo_Pop16();
    v[2] = gp::Fifo_Pop16();
    v[3] = gp::Fifo_Pop16();
    v[4] = gp::Fifo_Pop16();
    v[5] = gp::Fifo_Pop16();
    v[6] = gp::Fifo_Pop16();
    v[7] = gp::Fifo_Pop16();
    v[8] = gp::Fifo_Pop16();
    _ASSERT_MSG(TGP, 0, "Unimplemented method %s (%s line %s)", __FUNCTION__, __FILE__, __LINE__);
}

/// Direct vertex component 1 float
static void __direct_float_1(u32 addr, u32* dest) {
    dest[0] = gp::Fifo_Pop32();
}

/// Direct vertex component 2 floats
static void __direct_float_2(u32 addr, u32* dest) {
    dest[0] = gp::Fifo_Pop32();
    dest[1] = gp::Fifo_Pop32();
}

/// Direct vertex component 3 floats
static void __direct_float_3(u32 addr, u32* dest) {
    dest[0] = gp::Fifo_Pop32();
    dest[1] = gp::Fifo_Pop32();
    dest[2] = gp::Fifo_Pop32();
}

/// Direct vertex component 9 floats
static void __direct_float_9(u32 addr, u32* dest) {
    dest[0] = gp::Fifo_Pop32();
    dest[1] = gp::Fifo_Pop32();
    dest[2] = gp::Fifo_Pop32();
    dest[3] = gp::Fifo_Pop32();
    dest[4] = gp::Fifo_Pop32();
    dest[5] = gp::Fifo_Pop32();
    dest[6] = gp::Fifo_Pop32();
    dest[7] = gp::Fifo_Pop32();
    dest[8] = gp::Fifo_Pop32();
    _ASSERT_MSG(TGP, 0, "Unimplemented method %s (%s line %s)", __FUNCTION__, __FILE__, __LINE__);
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
    _ASSERT_MSG(TGP, 0, "Unimplemented method %s (%s line %s)", __FUNCTION__, __FILE__, __LINE__);
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
    _ASSERT_MSG(TGP, 0, "Unimplemented method %s (%s line %s)", __FUNCTION__, __FILE__, __LINE__);
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
    _ASSERT_MSG(TGP, 0, "Unimplemented method %s (%s line %s)", __FUNCTION__, __FILE__, __LINE__);
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
int VertexLoader_GetVertexSize() {
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
    case GX_DIRECT:
        size+=gp::kVertexPositionSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_pos()];
        break;
    case GX_INDEX8: 
        size+=1; 
        break;
    case GX_INDEX16: 
        size+=2; 
        break;
    }
    // Vertex normal size
    switch(gp::g_cp_regs.vcd_lo[0].normal) {	
    case GX_DIRECT:
        size+=gp::kVertexNormalSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_normal()];
        break;
    case GX_INDEX8:
        size+=1; 
        break;
    case GX_INDEX16:
        size+=2; 
        break;
    }
    // Vertex color 0 size
    switch(gp::g_cp_regs.vcd_lo[0].color0) {	
    case GX_DIRECT:
        size+=gp::kVertexColorSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_col0()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Vertex color 1 size
    switch(gp::g_cp_regs.vcd_lo[0].color1) {	
    case GX_DIRECT:
        size+=gp::kVertexColorSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_col1()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 0 size
    switch(gp::g_cp_regs.vcd_hi[0].tex0_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_a[gp::g_cur_vat].get_tex0()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 1 size
    switch(gp::g_cp_regs.vcd_hi[0].tex1_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex1()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 2 size
    switch(gp::g_cp_regs.vcd_hi[0].tex2_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex2()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 3 size
    switch(gp::g_cp_regs.vcd_hi[0].tex3_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex3()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 4 size
    switch(gp::g_cp_regs.vcd_hi[0].tex4_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_b[gp::g_cur_vat].get_tex4()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 5 size
    switch(gp::g_cp_regs.vcd_hi[0].tex5_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex5()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 6 size
    switch(gp::g_cp_regs.vcd_hi[0].tex6_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex6()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    // Texture coordinate 7 size
    switch(gp::g_cp_regs.vcd_hi[0].tex7_coord) {
    case GX_DIRECT:
        size+=gp::kVertexTextureSize[gp::g_cp_regs.vat_reg_c[gp::g_cur_vat].get_tex7()];
        break;
    case GX_INDEX8:
        size+=1;
        break;
    case GX_INDEX16:
        size+=2;
        break;
    }
    return size;
}

void SetVertexState() {
}


/**
 * @brief Decode a primitive type
 * @param type Type of primitive (e.g. points, lines, triangles, etc.)
 * @param count Number of vertices
 */
void VertexLoader_DecodePrimitive(GXPrimitive type, int count) {
    static VertexState state;
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

    // Set renderer types
    state.pos.attr_type     = (GXAttrType)gp::g_cp_regs.vcd_lo[0].position;
    state.pos.comp_count    = (GXCompCnt)vat_a->pos_count;
    state.pos.comp_type     = (GXCompType)vat_a->pos_type;
    state.nrm.attr_type     = (GXAttrType)gp::g_cp_regs.vcd_lo[0].normal;
    state.nrm.comp_count    = (GXCompCnt)vat_a->normal_count;
    state.nrm.comp_type     = (GXCompType)vat_a->normal_type;
    state.col[0].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_lo[0].color0;
    state.col[0].comp_count = (GXCompCnt)vat_a->col0_count;
    state.col[0].comp_type  = (GXCompType)vat_a->col0_type;
    state.col[1].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_lo[0].color1;
    state.col[1].comp_count = (GXCompCnt)vat_a->col1_count;
    state.col[1].comp_type  = (GXCompType)vat_a->col1_type;
    state.tex[0].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex0_coord;
    state.tex[0].comp_count = (GXCompCnt)vat_a->tex0_count;
    state.tex[0].comp_type  = (GXCompType)vat_a->tex0_type;
    state.tex[1].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex1_coord;
    state.tex[1].comp_count = (GXCompCnt)vat_b->tex1_count;
    state.tex[1].comp_type  = (GXCompType)vat_b->tex1_type;
    state.tex[2].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex2_coord;
    state.tex[2].comp_count = (GXCompCnt)vat_b->tex2_count;
    state.tex[2].comp_type  = (GXCompType)vat_b->tex2_type;
    state.tex[3].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex3_coord;
    state.tex[3].comp_count = (GXCompCnt)vat_b->tex3_count;
    state.tex[3].comp_type  = (GXCompType)vat_b->tex3_type;
    state.tex[4].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex4_coord;
    state.tex[4].comp_count = (GXCompCnt)vat_b->tex4_count;
    state.tex[4].comp_type  = (GXCompType)vat_b->tex4_type;
    state.tex[5].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex5_coord;
    state.tex[5].comp_count = (GXCompCnt)vat_c->tex5_count;
    state.tex[5].comp_type  = (GXCompType)vat_c->tex5_type;
    state.tex[6].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex6_coord;
    state.tex[6].comp_count = (GXCompCnt)vat_c->tex6_count;
    state.tex[6].comp_type  = (GXCompType)vat_c->tex6_type;
    state.tex[7].attr_type  = (GXAttrType)gp::g_cp_regs.vcd_hi[0].tex7_coord;
    state.tex[7].comp_count = (GXCompCnt)vat_c->tex7_count;
    state.tex[7].comp_type  = (GXCompType)vat_c->tex7_type;

    video_core::g_renderer->SetVertexState(state);
    video_core::g_shader_manager->UpdateVertexState(state);
    video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_VertexPostition_DQF, 
        vat_a->get_pos_dqf_enabled());

    // Configure renderer to begin a new primitive
    VertexManager_BeginPrimitive(type, count);

    for (int i = 0; i < count; i++) {

        // Matrix indices
        if (gp::g_cp_regs.vcd_lo[0].pos_midx_enable)
            g_vbo->pm_idx = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex0_midx_enable)
            g_vbo->tm_idx[0] = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex1_midx_enable)
            g_vbo->tm_idx[1] = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex2_midx_enable)
            g_vbo->tm_idx[2] = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex3_midx_enable)
            g_vbo->tm_idx[3] = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex4_midx_enable) 
            g_vbo->tm_idx[4] = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex5_midx_enable)
            g_vbo->tm_idx[5] = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex6_midx_enable)
            g_vbo->tm_idx[6] = gp::Fifo_Pop8();
        if (gp::g_cp_regs.vcd_lo[0].tex7_midx_enable)
            g_vbo->tm_idx[7] = gp::Fifo_Pop8();

        // Decode position
        switch (gp::g_cp_regs.vcd_lo[0].position) {
        case GX_DIRECT:
            LookupPositionDirect[vat_a->get_pos()](0, g_vbo->position);
            break;
        case GX_INDEX8:
            LookupPositionIndexed[vat_a->get_pos()](pos_base + (gp::Fifo_Pop8() * pos_stride),
                g_vbo->position);
            break;
        case GX_INDEX16:
            LookupPositionIndexed[vat_a->get_pos()](pos_base + (gp::Fifo_Pop16() * pos_stride),
                g_vbo->position);
            break;
        }
        // Decode normal
        switch (gp::g_cp_regs.vcd_lo[0].normal) {
        case GX_DIRECT:
            LookupNormalDirect[vat_a->get_normal()](0, g_vbo->normal);
            break;
        case GX_INDEX8:
            LookupNormalIndexed[vat_a->get_normal()](normal_base + (gp::Fifo_Pop8() * normal_stride),
                g_vbo->normal);
            break;
        case GX_INDEX16:
            LookupNormalIndexed[vat_a->get_normal()](normal_base + (gp::Fifo_Pop16() * normal_stride),
                g_vbo->normal);
            break;
        }
        // Decode color 0
        switch (gp::g_cp_regs.vcd_lo[0].color0) {
        case GX_NONE:
            g_vbo->color[0] = 0xffffffff;
            break;
        case GX_DIRECT:
            LookupColorDirect[vat_a->get_col0()](0, &g_vbo->color[0]);
            break;
        case GX_INDEX8:
            LookupColorIndexed[vat_a->get_col0()](col0_base + (gp::Fifo_Pop8() * col0_stride),
                &g_vbo->color[0]);
            break;
        case GX_INDEX16:
            LookupColorIndexed[vat_a->get_col0()](col0_base + (gp::Fifo_Pop16() * col0_stride),
                &g_vbo->color[0]);
            break;
        }
        // Decode color 1
        switch (gp::g_cp_regs.vcd_lo[0].color1) {
        case GX_NONE:
            g_vbo->color[1] = 0xffffffff;
            break;
        case GX_DIRECT:
            LookupColorDirect[vat_a->get_col1()](0, &g_vbo->color[1]);
            break;
        case GX_INDEX8:
            LookupColorIndexed[vat_a->get_col1()](col1_base + (gp::Fifo_Pop8() * col1_stride),
                &g_vbo->color[1]);
            break;
        case GX_INDEX16:
            LookupColorIndexed[vat_a->get_col1()](col1_base + (gp::Fifo_Pop16() * col1_stride),
                &g_vbo->color[1]);
            break;
        }

        // Decode texcoord 0
        if (gp::g_cp_regs.vcd_hi[0].tex0_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex0_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_a->get_tex0()](0, &g_vbo->texcoords[0 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_a->get_tex0()](tex0_base + (gp::Fifo_Pop8() * tex0_stride), 
                    &g_vbo->texcoords[0 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_a->get_tex0()](tex0_base + (gp::Fifo_Pop16() * tex0_stride),
                    &g_vbo->texcoords[0 << 1]);
                break;
            }
        }
        // Decode texcoord 1
        if (gp::g_cp_regs.vcd_hi[0].tex1_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex1_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex1()](0, &g_vbo->texcoords[1 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex1()](tex1_base + (gp::Fifo_Pop8() * tex1_stride),
                    &g_vbo->texcoords[1 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex1()](tex1_base + (gp::Fifo_Pop16() * tex1_stride),
                    &g_vbo->texcoords[1 << 1]);
                break;
            }
        }
        // Decode texcoord 2
        if (gp::g_cp_regs.vcd_hi[0].tex2_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex2_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex2()](0, &g_vbo->texcoords[2 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex2()](tex2_base + (gp::Fifo_Pop8() * tex2_stride),
                    &g_vbo->texcoords[2 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex2()](tex2_base + (gp::Fifo_Pop16() * tex2_stride),
                    &g_vbo->texcoords[2 << 1]);
                break;
            }
        }
        // Decode texcoord 3
        if (gp::g_cp_regs.vcd_hi[0].tex3_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex3_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex3()](0, &g_vbo->texcoords[3 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex3()](tex3_base + (gp::Fifo_Pop8() * tex3_stride),
                    &g_vbo->texcoords[3 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex3()](tex3_base + (gp::Fifo_Pop16() * tex3_stride),
                    &g_vbo->texcoords[3 << 1]);
                break;
            }
        }
        // Decode texcoord 4
        if (gp::g_cp_regs.vcd_hi[0].tex4_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex4_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex4()](0, &g_vbo->texcoords[4 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex4()](tex4_base + (gp::Fifo_Pop8() * tex4_stride),
                    &g_vbo->texcoords[4 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex4()](tex4_base + (gp::Fifo_Pop16() * tex4_stride),
                    &g_vbo->texcoords[4 << 1]);
                break;
            }
        }
        // Decode texcoord 5
        if (gp::g_cp_regs.vcd_hi[0].tex5_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex5_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex5()](0, &g_vbo->texcoords[5 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex5()](tex5_base + (gp::Fifo_Pop8() * tex5_stride),
                    &g_vbo->texcoords[5 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex5()](tex5_base + (gp::Fifo_Pop16() * tex5_stride),
                    &g_vbo->texcoords[5 << 1]);
                break;
            }
        }
        // Decode texcoord 6
        if (gp::g_cp_regs.vcd_hi[0].tex6_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex6_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex6()](0, &g_vbo->texcoords[6 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex6()](tex6_base + (gp::Fifo_Pop8() * tex6_stride),
                    &g_vbo->texcoords[6 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex6()](tex6_base + (gp::Fifo_Pop16() * tex6_stride),
                    &g_vbo->texcoords[6 << 1]);
                break;
            }
        }
        // Decode texcoord 7
        if (gp::g_cp_regs.vcd_hi[0].tex7_coord) {
            switch(gp::g_cp_regs.vcd_hi[0].tex7_coord) {
            case GX_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex7()](0, &g_vbo->texcoords[7 << 1]);
                break;
            case GX_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex7()](tex7_base + (gp::Fifo_Pop8() * tex7_stride),
                    &g_vbo->texcoords[7 << 1]);
                break;
            case GX_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex7()](tex7_base + (gp::Fifo_Pop16() * tex7_stride),
                    &g_vbo->texcoords[7 << 1]);
                break;
            }
        }   
        VertexManager_NextVertex();
    }
    VertexManager_EndPrimitive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// Initialize the Vertex Loader
void VertexLoader_Init() {
}

/// Shutdown the Vertex Loader
void VertexLoader_Shutdown() {
}

} // namespace
