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

namespace gp {

typedef void (*VertexLoaderTable)(u32);	

////////////////////////////////////////////////////////////////////////////////////////////////////

static __inline u16 MemoryRead16(u32 addr)
{
    if(!(addr & 1))
        return *(u16 *)(&Mem_RAM[(addr ^ 2) & RAM_MASK]);

    addr = addr & RAM_MASK;
    return (u16)(Mem_RAM[(addr + 0) ^ 3] << 8) |
        (u16)(Mem_RAM[(addr + 1) ^ 3]);
}

static __inline u32 MemoryRead32(u32 addr)
{
    addr &= RAM_MASK;
    if(!(addr & 3))
        return *(u32 *)(&Mem_RAM[addr]);

    return ((u32)Mem_RAM[(addr + 0) ^ 3] << 24) |
        ((u32)Mem_RAM[(addr + 1) ^ 3] << 16) |
        ((u32)Mem_RAM[(addr + 2) ^ 3] << 8) |
        ((u32)Mem_RAM[(addr + 3) ^ 3]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Position Decoding

static void VertexPosition_Unk(u32 addr) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex position!!! Ask neobrain to implement me!");
}

static void VertexPosition_D8_XY(u32 addr) {
    u8 v[3];
    v[0] = FifoPop8();
    v[1] = FifoPop8();
    v[2] = 0;
    vertex_manager::Position_SendByte(v);
}

static void VertexPosition_D16_XY(u32 addr) {
    u16 v[3];
    v[0] = FifoPop16();
    v[1] = FifoPop16();
    v[2] = 0;
    vertex_manager::Position_SendShort(v);
}

static void VertexPosition_D32_XY(u32 addr) {
    u32 v[3];
    v[0] = FifoPop32();
    v[1] = FifoPop32();
    v[2] = 0;
    vertex_manager::Position_SendFloat((f32*)v);
}

static void VertexPosition_I8_XY(u32 addr) {
    u8 v[3];
    u16 data = MemoryRead16(addr);
    v[0] = (u8)((data >> 8) & 0xFF);
    v[1] = (u8)(data & 0xFF);
    v[2] = 0;
    vertex_manager::Position_SendByte(v);
}

// correct
static void VertexPosition_I16_XY(u32 addr) {
    u16 v[3];
    u32 data = MemoryRead32(addr);
    v[0] = (data >> 16);
    v[1] = (data & 0xFFFF);
    v[2] = 0;
    vertex_manager::Position_SendShort(v);
}

// correct
static void VertexPosition_I32_XY(u32 addr) {
    u32 v[3];
    v[0] = MemoryRead32(addr + 0);
    v[1] = MemoryRead32(addr + 4);
    v[2] = 0;
    vertex_manager::Position_SendFloat((f32*)v);
}

// correct
static void VertexPosition_D8_XYZ(u32 addr) {
    u8 v[3];
    v[0] = FifoPop8();
    v[1] = FifoPop8();
    v[2] = FifoPop8();
    vertex_manager::Position_SendByte(v);
}

// correct
static void VertexPosition_D16_XYZ(u32 addr) {
    u16 v[3];
    v[0] = FifoPop16();
    v[1] = FifoPop16();
    v[2] = FifoPop16();
    vertex_manager::Position_SendShort(v);
}

// correct
static void VertexPosition_D32_XYZ(u32 addr) {
    u32 v[3] = {FifoPop32(), FifoPop32(), FifoPop32()};
    vertex_manager::Position_SendFloat((f32*)v);
}

// Correct
static void VertexPosition_I8_XYZ(u32 addr) {
    u8 v[3];

    u32 data = MemoryRead32(addr);
    v[0] = (u8)((data >> 24) & 0xFF);
    v[1] = (u8)((data >> 16) & 0xFF);
    v[2] = (u8)((data >> 8) & 0xFF);

    vertex_manager::Position_SendByte(v);
}

// correct
static void VertexPosition_I16_XYZ(u32 addr) {
    u16 v[3];
    u32 data = MemoryRead32(addr);
    v[0] = (data >> 16);
    v[1] = (data & 0xFFFF);
    v[2] = MemoryRead16(addr + 4);
    vertex_manager::Position_SendShort(v);
}

// correct
static void VertexPosition_I32_XYZ(u32 addr) {
    u32 v[3];
    v[0] = MemoryRead32(addr + 0);
    v[1] = MemoryRead32(addr + 4);
    v[2] = MemoryRead32(addr + 8);
    vertex_manager::Position_SendFloat((f32*)v);
}

// unimplemented
#define VERTEXLOADER_POSITION_UNDEF(name)   void VertexPosition_##name(u32 addr) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
}

VertexLoaderTable LookupPositionDirect[0x10] = {
    VertexPosition_D8_XY,   VertexPosition_D8_XY,   VertexPosition_D16_XY,  VertexPosition_D16_XY,
    VertexPosition_D32_XY,  VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,     
    VertexPosition_D8_XYZ,  VertexPosition_D8_XYZ,  VertexPosition_D16_XYZ, VertexPosition_D16_XYZ, 
    VertexPosition_D32_XYZ, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk
};

VertexLoaderTable LookupPositionIndexed[0x10] = {
    VertexPosition_I8_XY,   VertexPosition_I8_XY,   VertexPosition_I16_XY,  VertexPosition_I16_XY,
    VertexPosition_I32_XY,  VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,     
    VertexPosition_I8_XYZ,  VertexPosition_I8_XYZ,  VertexPosition_I16_XYZ, VertexPosition_I16_XYZ, 
    VertexPosition_I32_XYZ, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Color Decoding

#define VERTEXLOADER_COLOR_UNDEF(name)      void VertexColor_##name(u32 addr) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
    }

static void VertexColor_Unk(u32 addr) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex color count: %d format: %d", 
        g_cp_regs.vat_reg_a[g_cur_vat].col0_count, g_cp_regs.vat_reg_a[g_cur_vat].col0_format);
}

// correct
static void VertexColor_DRGB565(u32 addr) {
    vertex_manager::Color_Send(FifoPop16());
}

// correct
static void VertexColor_DRGB8(u32 addr) {
    vertex_manager::Color_Send(FifoPop24());
}

// correct
static void VertexColor_DRGBA4(u32 addr) {
    vertex_manager::Color_Send(FifoPop16());
}

// correct
static void VertexColor_DRGBA6(u32 addr) {
    vertex_manager::Color_Send(FifoPop24());
}

// correct
static void VertexColor_DRGBA8(u32 addr) {
    vertex_manager::Color_Send(FifoPop32());
}

// correct
static void VertexColor_IRGB565(u32 addr) {
    vertex_manager::Color_Send(MemoryRead16(addr));
}

// correct
static void VertexColor_IRGB8(u32 addr) {
    vertex_manager::Color_Send(MemoryRead32(addr) >> 8);
}

// correct
static void VertexColor_IRGBA4(u32 addr) {
    vertex_manager::Color_Send(MemoryRead16(addr));
}

// correct
static void VertexColor_IRGBA6(u32 addr) {
    vertex_manager::Color_Send(MemoryRead32(addr) >> 8);
}

// correct
static void VertexColor_IRGBA8(u32 addr) {
    vertex_manager::Color_Send(MemoryRead32(addr));
}


// Not sure how some of this is supposed to work - the table is count << 3 | format. Count can be
// GX_CLR_RGB (0) or 1 (GX_CLR_RGBA). I've entered all formats for all positions, even though some
// don't make sense (with the count taken into account) - ??. Seems correct. -ShizZy  
VertexLoaderTable LookupColorDirect[0x10] = {
    VertexColor_DRGB565,    VertexColor_DRGB8,  VertexColor_DRGBA8, VertexColor_DRGBA4,
    VertexColor_DRGBA6,     VertexColor_DRGBA8, VertexColor_Unk,    VertexColor_Unk,
    VertexColor_DRGB565,    VertexColor_DRGB8,  VertexColor_DRGBA8, VertexColor_DRGBA4,
    VertexColor_DRGBA6,     VertexColor_DRGBA8, VertexColor_Unk,    VertexColor_Unk,
};

VertexLoaderTable LookupColorIndexed[0x10] = {
    VertexColor_IRGB565,    VertexColor_IRGB8,  VertexColor_IRGBA8, VertexColor_IRGBA4,
    VertexColor_IRGBA6,     VertexColor_IRGBA8, VertexColor_Unk,    VertexColor_Unk,
    VertexColor_IRGB565,    VertexColor_IRGB8,  VertexColor_IRGBA8,    VertexColor_IRGBA4,
    VertexColor_IRGBA6,     VertexColor_IRGBA8, VertexColor_Unk,    VertexColor_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Normal Decoding

#define VERTEXLOADER_NORMAL_UNDEF(name)     void VertexNormal_##name(u32 addr) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
    }

static void VertexNormal_Unk(u32 addr) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex normal!!! Ask neobrain to implement me!");
}

static void VertexNormal_D8_3(u32 addr) {
    // TODO(ShizZy): ImplementMe
    FifoPop8();
    FifoPop8();
    FifoPop8();
}

static void VertexNormal_D16_3(u32 addr) {
    // TODO(ShizZy): ImplementMe
    FifoPop16();
    FifoPop16();
    FifoPop16();
}

static void VertexNormal_D32_3(u32 addr) {
    // TODO(ShizZy): ImplementMe
    FifoPop32();
    FifoPop32();
    FifoPop32();
}

static void VertexNormal_D8_9(u32 addr) {
    // TODO(ShizZy): ImplementMe
    FifoPop8();
    FifoPop8();
    FifoPop8();
    FifoPop8();
    FifoPop8();
    FifoPop8();
    FifoPop8();
    FifoPop8();
    FifoPop8();
}

static void VertexNormal_D16_9(u32 addr) {
    // TODO(ShizZy): ImplementMe
    FifoPop16();
    FifoPop16();
    FifoPop16();
    FifoPop16();
    FifoPop16();
    FifoPop16();
    FifoPop16();
    FifoPop16();
    FifoPop16();
}

static void VertexNormal_D32_9(u32 addr) {
    // TODO(ShizZy): ImplementMe
    FifoPop32();
    FifoPop32();
    FifoPop32();
    FifoPop32();
    FifoPop32();
    FifoPop32();
    FifoPop32();
    FifoPop32();
    FifoPop32();
}

static void VertexNormal_I8_3(u32 addr) {
    // TODO(ShizZy): ImplementMe
}

static void VertexNormal_I16_3(u32 addr) {
    // TODO(ShizZy): ImplementMe
}

static void VertexNormal_I32_3(u32 addr) {
    // TODO(ShizZy): ImplementMe
}

static void VertexNormal_I8_9(u32 addr) {
    // TODO(ShizZy): ImplementMe
}

static void VertexNormal_I16_9(u32 addr) {
    // TODO(ShizZy): ImplementMe
}

static void VertexNormal_I32_9(u32 addr) {
    // TODO(ShizZy): ImplementMe
}


// TODO(ShizZy): Decoding 9 normals.... needs to be added to this table

VertexLoaderTable LookupNormalDirect[0x10] = {
    VertexNormal_D8_3,  VertexNormal_D8_3,  VertexNormal_D16_3, VertexNormal_D16_3,
    VertexNormal_D32_3, VertexNormal_Unk,   VertexNormal_Unk,   VertexNormal_Unk,
    VertexNormal_D8_9,  VertexNormal_D8_9,  VertexNormal_D16_9, VertexNormal_D16_9,
    VertexNormal_D32_9, VertexNormal_Unk,   VertexNormal_Unk,   VertexNormal_Unk
};

VertexLoaderTable LookupNormalIndexed[0x10] = {
    VertexNormal_I8_3,  VertexNormal_I8_3,  VertexNormal_I16_3, VertexNormal_I16_3,
    VertexNormal_I32_3, VertexNormal_Unk,   VertexNormal_Unk,   VertexNormal_Unk,
    VertexNormal_I8_9,  VertexNormal_I8_9,  VertexNormal_I16_9, VertexNormal_I16_9,
    VertexNormal_I32_9, VertexNormal_Unk,   VertexNormal_Unk,   VertexNormal_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// TexCoord Decoding

#define VERTEXLOADER_TEXCOORD_UNDEF(name)   void VertexTexCoord_##name(u32 addr) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
    }

static void VertexTexCoord_Unk(u32 addr) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex texcoord!!! Ask neobrain to implement me!");
}

static void VertexTexCoord_D8_ST(u32 addr) {
	u8 v[2];
    v[0] = FifoPop8();
    v[1] = FifoPop8();
	vertex_manager::Texcoord_SendByte(v);
}

// correct
static void VertexTexCoord_D16_ST(u32 addr) {
	u16 v[2];
    v[0] = FifoPop16();
    v[1] = FifoPop16();
	vertex_manager::Texcoord_SendShort(v);
}

static void VertexTexCoord_D32_ST(u32 addr) {
	u32 v[2];
	v[0] = FifoPop32();
	v[1] = FifoPop32();
	vertex_manager::Texcoord_SendFloat((f32*)v);
}

static void VertexTexCoord_D8_S(u32 addr) {
    u8 v = FifoPop8();
    vertex_manager::Texcoord_SendByte(&v);
}

static void VertexTexCoord_D16_S(u32 addr) {
    u16 v = FifoPop16();
    vertex_manager::Texcoord_SendShort(&v);
}

static void VertexTexCoord_D32_S(u32 addr) {
    u32 v = FifoPop32();
    vertex_manager::Texcoord_SendFloat((f32*)&v);
}

static void VertexTexCoord_I8_ST(u32 addr) {
    u8 v[2];
    u16 data = MemoryRead16(addr);
    v[0] = (u8)((data >> 8) & 0xFF);
    v[1] = (u8)(data & 0xFF);
    vertex_manager::Texcoord_SendByte(v);
}

// correct
static void VertexTexCoord_I16_ST(u32 addr) {
    u16 v[2];
    u32 data = MemoryRead32(addr);
    v[0] = (data >> 16);
    v[1] = (data & 0xFFFF);
    vertex_manager::Texcoord_SendShort(v);
}

static void VertexTexCoord_I32_ST(u32 addr) {
    u32 v[2];
    v[0] = MemoryRead32(addr + 0);
    v[1] = MemoryRead32(addr + 4);
    vertex_manager::Texcoord_SendFloat((f32*)v);
}

static void VertexTexCoord_I8_S(u32 addr) {
    u8 v = Mem_RAM[addr & RAM_MASK];
    vertex_manager::Texcoord_SendByte(&v);
}

static void VertexTexCoord_I16_S(u32 addr) {
    u16 v = MemoryRead16(addr);
    vertex_manager::Texcoord_SendShort(&v);
}

static void VertexTexCoord_I32_S(u32 addr) {
    u32 v = MemoryRead32(addr);
    vertex_manager::Texcoord_SendFloat((f32*)&v);
}

VertexLoaderTable LookupTexCoordDirect[0x10] = {
    VertexTexCoord_D8_S,    VertexTexCoord_D8_S,    VertexTexCoord_D16_S,   VertexTexCoord_D16_S,
    VertexTexCoord_D32_S,   VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk,   
    VertexTexCoord_D8_ST,   VertexTexCoord_D8_ST,   VertexTexCoord_D16_ST,  VertexTexCoord_D16_ST,
    VertexTexCoord_D32_ST,  VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk
};

VertexLoaderTable LookupTexCoordIndexed[0x10] = {
    VertexTexCoord_I8_S,    VertexTexCoord_I8_S,    VertexTexCoord_I16_S,   VertexTexCoord_I16_S, 
    VertexTexCoord_I32_S,   VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
    VertexTexCoord_I8_ST,   VertexTexCoord_I8_ST,   VertexTexCoord_I16_ST,  VertexTexCoord_I16_ST, 
    VertexTexCoord_I32_ST,  VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Primitive decoding

/// Decode a primitive that's currently in the FIFO
void DecodePrimitive(GXPrimitive type, int count) {

    CPVatRegA* vat_a = &g_cp_regs.vat_reg_a[g_cur_vat];
    CPVatRegB* vat_b = &g_cp_regs.vat_reg_b[g_cur_vat];
    CPVatRegC* vat_c = &g_cp_regs.vat_reg_c[g_cur_vat];

    u8 pm_midx = 0, tm_midx[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    u32 pos_base        = g_cp_regs.array_base[0].addr_base;
    u8  pos_stride      = g_cp_regs.array_stride[0].addr_stride;
    u32 normal_base     = g_cp_regs.array_base[1].addr_base;
    u8  normal_stride   = g_cp_regs.array_stride[1].addr_stride;
    u32 col0_base       = g_cp_regs.array_base[2].addr_base;
    u8  col0_stride     = g_cp_regs.array_stride[2].addr_stride;
    u32 col1_base       = g_cp_regs.array_base[3].addr_base;
    u8  col1_stride     = g_cp_regs.array_stride[3].addr_stride;
    u32 tex0_base       = g_cp_regs.array_base[4].addr_base;
    u8  tex0_stride     = g_cp_regs.array_stride[4].addr_stride;
    u32 tex1_base       = g_cp_regs.array_base[5].addr_base;
    u8  tex1_stride     = g_cp_regs.array_stride[5].addr_stride;
    u32 tex2_base       = g_cp_regs.array_base[6].addr_base;
    u8  tex2_stride     = g_cp_regs.array_stride[6].addr_stride;
    u32 tex3_base       = g_cp_regs.array_base[7].addr_base;
    u8  tex3_stride     = g_cp_regs.array_stride[7].addr_stride;
    u32 tex4_base       = g_cp_regs.array_base[8].addr_base;
    u8  tex4_stride     = g_cp_regs.array_stride[8].addr_stride;
    u32 tex5_base       = g_cp_regs.array_base[9].addr_base;
    u8  tex5_stride     = g_cp_regs.array_stride[9].addr_stride;
    u32 tex6_base       = g_cp_regs.array_base[10].addr_base;
    u8  tex6_stride     = g_cp_regs.array_stride[10].addr_stride;
    u32 tex7_base       = g_cp_regs.array_base[11].addr_base;
    u8  tex7_stride     = g_cp_regs.array_stride[11].addr_stride;

    // Configure renderer to begin a new primitive
    vertex_manager::BeginPrimitive(type, count);

    // Decode and apply texture
	if(g_bp_regs.tevorder[0].get_enable(0))
	{
		glActiveTexture(GL_TEXTURE0);
		LoadTexture(0x94 + 0);
	}
	/*if(g_bp_regs.tevorder[1 >> 1].get_enable(1))
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		LoadTexture(0x94 + 1);
	}
	if(g_bp_regs.tevorder[2 >> 1].get_enable(2))
	{
		glActiveTexture(GL_TEXTURE0 + 2);
		LoadTexture(0x94 + 2);
	}
	if(g_bp_regs.tevorder[3 >> 1].get_enable(3))
	{
		glActiveTexture(GL_TEXTURE0 + 3);
		LoadTexture(0x94 + 3);
	}
	if(g_bp_regs.tevorder[4 >> 1].get_enable(4))
	{
		glActiveTexture(GL_TEXTURE4 + 0);
		LoadTexture(0xb4 + 0);
	}
	if(g_bp_regs.tevorder[5 >> 1].get_enable(5))
	{
		glActiveTexture(GL_TEXTURE4 + 1);
		LoadTexture(0xb4 + 1);
	}
	if(g_bp_regs.tevorder[6 >> 1].get_enable(6))
	{
		glActiveTexture(GL_TEXTURE4 + 2);
		LoadTexture(0xb4 + 2);
	}
	if(g_bp_regs.tevorder[7 >> 1].get_enable(7))
	{
		glActiveTexture(GL_TEXTURE4 + 3);
		LoadTexture(0xb4 + 3);
	}*/

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
        if (g_cp_regs.vcd_lo[0].pos_midx_enable) 
            pm_midx = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex0_midx_enable) 
            tm_midx[0] = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex1_midx_enable) 
            tm_midx[1] = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex2_midx_enable) 
            tm_midx[2] = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex3_midx_enable) 
            tm_midx[3] = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex4_midx_enable) 
            tm_midx[4] = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex5_midx_enable) 
            tm_midx[5] = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex6_midx_enable) 
            tm_midx[6] = FifoPop8();
        if (g_cp_regs.vcd_lo[0].tex7_midx_enable) 
            tm_midx[7] = FifoPop8();

        vertex_manager::SendMatrixIndices(pm_midx, tm_midx);

        // Decode position
        switch (g_cp_regs.vcd_lo[0].position) {
        case CP_DIRECT:
            LookupPositionDirect[vat_a->get_pos()](0);
            break;
        case CP_INDEX8:
            LookupPositionIndexed[vat_a->get_pos()](pos_base + (FifoPop8() * pos_stride));
            break;
        case CP_INDEX16:
            LookupPositionIndexed[vat_a->get_pos()](pos_base + (FifoPop16() * pos_stride));
            break;
        }
        // Decode normal
        switch (g_cp_regs.vcd_lo[0].normal) {
        case CP_DIRECT:
            LookupNormalDirect[vat_a->get_normal()](0);
            break;
        case CP_INDEX8:
            LookupNormalIndexed[vat_a->get_normal()](normal_base + (FifoPop8() * normal_stride));
            break;
        case CP_INDEX16:
            LookupNormalIndexed[vat_a->get_normal()](normal_base + (FifoPop16() * normal_stride));
            break;
        }
        // Decode color 0
        switch (g_cp_regs.vcd_lo[0].color0) {
        case CP_NOT_PRESENT:
            // Not sure if this is right, but assume white if disabled (not black)
            vertex_manager::Color_Send(0xffffffff);
            break;
        case CP_DIRECT:
            LookupColorDirect[vat_a->get_col0()](0);
            break;
        case CP_INDEX8:
            LookupColorIndexed[vat_a->get_col0()](col0_base + (FifoPop8() * col0_stride));
            break;
        case CP_INDEX16:
            LookupColorIndexed[vat_a->get_col0()](col0_base + (FifoPop16() * col0_stride));
            break;
        }
        // Decode color 1
        switch (gp::g_cp_regs.vcd_lo[0].color1) {
        case CP_DIRECT:
            LookupColorDirect[vat_a->get_col1()](0);
            break;
        case CP_INDEX8:
            LookupColorIndexed[vat_a->get_col1()](col1_base + (FifoPop8() * col1_stride));
            break;
        case CP_INDEX16:
            LookupColorIndexed[vat_a->get_col1()](col1_base + (FifoPop16() * col1_stride));
            break;
        }
        // Decode texcoord 0
        if (g_cp_regs.vcd_hi[0].tex0_coord) {
            switch(g_cp_regs.vcd_hi[0].tex0_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_a->get_tex0()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_a->get_tex0()](tex0_base + (FifoPop8() * tex0_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_a->get_tex0()](tex0_base + (FifoPop16() * tex0_stride));
                break;
            }
        }
        // Decode texcoord 1
        if (g_cp_regs.vcd_hi[0].tex1_coord) {
            switch(g_cp_regs.vcd_hi[0].tex1_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex1()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex1()](tex1_base + (FifoPop8() * tex1_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex1()](tex1_base + (FifoPop16() * tex1_stride));
                break;
            }
        }
        // Decode texcoord 2
        if (g_cp_regs.vcd_hi[0].tex2_coord) {
            switch(g_cp_regs.vcd_hi[0].tex2_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex2()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex2()](tex2_base + (FifoPop8() * tex2_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex2()](tex2_base + (FifoPop16() * tex2_stride));
                break;
            }
        }
        // Decode texcoord 3
        if (g_cp_regs.vcd_hi[0].tex3_coord) {
            switch(g_cp_regs.vcd_hi[0].tex3_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex3()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex3()](tex3_base + (FifoPop8() * tex3_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex3()](tex3_base + (FifoPop16() * tex3_stride));
                break;
            }
        }
        // Decode texcoord 4
        if (g_cp_regs.vcd_hi[0].tex4_coord) {
            switch(g_cp_regs.vcd_hi[0].tex4_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_b->get_tex4()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_b->get_tex4()](tex4_base + (FifoPop8() * tex4_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_b->get_tex4()](tex4_base + (FifoPop16() * tex4_stride));
                break;
            }
        }
        // Decode texcoord 5
        if (g_cp_regs.vcd_hi[0].tex5_coord) {
            switch(g_cp_regs.vcd_hi[0].tex5_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex5()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex5()](tex5_base + (FifoPop8() * tex5_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex5()](tex5_base + (FifoPop16() * tex5_stride));
                break;
            }
        }
        // Decode texcoord 6
        if (g_cp_regs.vcd_hi[0].tex6_coord) {
            switch(g_cp_regs.vcd_hi[0].tex6_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex6()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex6()](tex6_base + (FifoPop8() * tex6_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex6()](tex6_base + (FifoPop16() * tex6_stride));
                break;
            }
        }
        // Decode texcoord 7
        if (g_cp_regs.vcd_hi[0].tex7_coord) {
            switch(g_cp_regs.vcd_hi[0].tex7_coord) {
            case CP_DIRECT:
                LookupTexCoordDirect[vat_c->get_tex7()](0);
                break;
            case CP_INDEX8:
                LookupTexCoordIndexed[vat_c->get_tex7()](tex7_base + (FifoPop8() * tex7_stride));
                break;
            case CP_INDEX16:
                LookupTexCoordIndexed[vat_c->get_tex7()](tex7_base + (FifoPop16() * tex7_stride));
                break;
            }
        }
        
        vertex_manager::NextVertex();
    }

    vertex_manager::EndPrimitive();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/// Initialize the Vertex Loader
void VertexLoaderInit() {
}

/// Shutdown the Vertex Loader
void VertexLoaderShutdown() {
}

} // namespace
