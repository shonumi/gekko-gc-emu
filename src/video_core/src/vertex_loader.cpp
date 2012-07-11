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

#include "video_core.h"
#include "fifo.h"
#include "cp_mem.h"
#include "xf_mem.h"

namespace gp {

u8 g_pm_index = 0;

typedef void (*VertexLoaderTable)(u16);	

////////////////////////////////////////////////////////////////////////////////////////////////////

__inline u16 MemoryRead16(u32 addr)
{
	if(!(addr & 1))
		return *(u16 *)(&Mem_RAM[(addr ^ 2) & RAM_MASK]);

	addr = addr & RAM_MASK;
	return (u16)(Mem_RAM[(addr + 0) ^ 3] << 8) |
		   (u16)(Mem_RAM[(addr + 1) ^ 3]);
}

__inline u32 MemoryRead32(u32 addr)
{
	addr &= RAM_MASK;
	if(!(addr & 3))
		return *(u32 *)(&Mem_RAM[addr]);

	return ((u32)Mem_RAM[(addr + 0) ^ 3] << 24) |
		   ((u32)Mem_RAM[(addr + 1) ^ 3] << 16) |
		   ((u32)Mem_RAM[(addr + 2) ^ 3] << 8) |
		   ((u32)Mem_RAM[(addr + 3) ^ 3]);
}

// transform 3d vertex position (software)
void TransformVertexPosition(f32* d, f32 *v)
{
	f32 *pmtx = XF_POSITION_MATRIX(g_pm_index);

	// transform 
	d[0] = (pmtx[0])*v[0] + (pmtx[1])*v[1] + (pmtx[ 2])*v[2] + (pmtx[ 3]);
	d[1] = (pmtx[4])*v[0] + (pmtx[5])*v[1] + (pmtx[ 6])*v[2] + (pmtx[ 7]);
	d[2] = (pmtx[8])*v[0] + (pmtx[9])*v[1] + (pmtx[10])*v[2] + (pmtx[11]);
}

static __inline void __vertex_pos_send_byte(u8* vec) {
    video_core::g_renderer->VertexPosition_SendByte(vec);
}

static __inline void __vertex_pos_send_short(u16* vec) {
    video_core::g_renderer->VertexPosition_SendShort(vec);
}

static __inline void __vertex_pos_send_float(f32* vec) {
    if (VCD_PMIDX) {
        f32 *pmtx = XF_POSITION_MATRIX(g_pm_index);
        f32 tf_vec[3];
    
        // transform 
        tf_vec[0] = (pmtx[0])*vec[0] + (pmtx[1])*vec[1] + (pmtx[ 2])*vec[2] + (pmtx[ 3]);
        tf_vec[1] = (pmtx[4])*vec[0] + (pmtx[5])*vec[1] + (pmtx[ 6])*vec[2] + (pmtx[ 7]);
        tf_vec[2] = (pmtx[8])*vec[0] + (pmtx[9])*vec[1] + (pmtx[10])*vec[2] + (pmtx[11]);

        video_core::g_renderer->VertexPosition_SendFloat(tf_vec);
    } else {
        video_core::g_renderer->VertexPosition_SendFloat(vec);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Position Decoding

static void VertexPosition_Unk(u16 index) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex position!!! Ask neobrain to implement me!");
}

static void VertexPosition_D8_XY(u16 index) {
	u16 v = FifoPop16();
	__vertex_pos_send_byte((u8*)&v);
}

static void VertexPosition_D16_XY(u16 index) {
	u16 v[2];
	v[0] = FifoPop16();
	v[1] = FifoPop16();
	__vertex_pos_send_short(v);
}

static void VertexPosition_D32_XY(u16 index) {
	u32 v[2];
	v[0] = FifoPop32();
	v[1] = FifoPop32();
	__vertex_pos_send_float((f32*)v);
}

static void VertexPosition_I8_XY(u16 index) {
	u8 v[2];
	
    u32 data = MemoryRead16(CP_DATA_POS_ADDR(index));
	v[0] = (u8)((data >> 8) & 0xFF);
	v[1] = (u8)(data & 0xFF);

	__vertex_pos_send_byte(v);
}

// correct
static void VertexPosition_I16_XY(u16 index) {
	u16 v[2];
	u32 data = MemoryRead32(CP_DATA_POS_ADDR(index));
	v[0] = (data >> 16);
	v[1] = (data & 0xFFFF);
	__vertex_pos_send_short(v);
}

// correct
static void VertexPosition_I32_XY(u16 index) {
	u32 v[2];
	v[0] = MemoryRead32(CP_DATA_POS_ADDR(index) + 0);
	v[1] = MemoryRead32(CP_DATA_POS_ADDR(index) + 4);
	__vertex_pos_send_float((f32*)v);
}

// correct
static void VertexPosition_D8_XYZ(u16 index) {
	u32 v = FifoPop24();
	__vertex_pos_send_byte((u8*)&v);
}

// correct
static void VertexPosition_D16_XYZ(u16 index) {
	u16 v[3];
	v[0] = FifoPop16();
	v[1] = FifoPop16();
	v[2] = FifoPop16();
	__vertex_pos_send_short(v);
}

// correct
static void VertexPosition_D32_XYZ(u16 index) {
	u32 v[3];
	v[0] = FifoPop32();
	v[1] = FifoPop32();
	v[2] = FifoPop32();
	__vertex_pos_send_float((f32*)v);
}

static void VertexPosition_I8_XYZ(u16 index) {
	u8 v[3];
	
    u32 data = MemoryRead32(CP_DATA_POS_ADDR(index));
	v[0] = (u8)((data >> 24) & 0xFF);
	v[1] = (u8)((data >> 16) & 0xFF);
	v[2] = (u8)((data >> 8) & 0xFF);

	__vertex_pos_send_byte(v);
}

// correct
static void VertexPosition_I16_XYZ(u16 index) {
	u16 v[3];
	u32 data = MemoryRead32(CP_DATA_POS_ADDR(index));
	v[0] = (data >> 16);
	v[1] = (data & 0xFFFF);
	v[2] = MemoryRead16(CP_DATA_POS_ADDR(index) + 4);
	__vertex_pos_send_short(v);
}

// correct
static void VertexPosition_I32_XYZ(u16 index) {
	u32 v[3];
	v[0] = MemoryRead32(CP_DATA_POS_ADDR(index) + 0);
	v[1] = MemoryRead32(CP_DATA_POS_ADDR(index) + 4);
	v[2] = MemoryRead32(CP_DATA_POS_ADDR(index) + 8);
	__vertex_pos_send_float((f32*)v);
}

// unimplemented
#define VERTEXLOADER_POSITION_UNDEF(name)   void VertexPosition_##name(u16 index) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
}

//VERTEXLOADER_POSITION_UNDEF(D8_XY);
//VERTEXLOADER_POSITION_UNDEF(D16_XY);
//VERTEXLOADER_POSITION_UNDEF(D32_XY);
//VERTEXLOADER_POSITION_UNDEF(D8_XYZ);
//VERTEXLOADER_POSITION_UNDEF(D16_XYZ);
//VERTEXLOADER_POSITION_UNDEF(D32_XYZ);

//VERTEXLOADER_POSITION_UNDEF(I8_XY);
//VERTEXLOADER_POSITION_UNDEF(I16_XY);
//VERTEXLOADER_POSITION_UNDEF(I32_XY);

//VERTEXLOADER_POSITION_UNDEF(I8_XYZ);
//VERTEXLOADER_POSITION_UNDEF(I16_XYZ);
//VERTEXLOADER_POSITION_UNDEF(I32_XYZ);

VertexLoaderTable LookupPosition[0x40] = {
    VertexPosition_Unk, VertexPosition_D8_XY,   VertexPosition_I8_XY,   VertexPosition_I8_XY,  	 
    VertexPosition_Unk, VertexPosition_D8_XY,   VertexPosition_I8_XY,   VertexPosition_I8_XY,  
    VertexPosition_Unk, VertexPosition_D16_XY,  VertexPosition_I16_XY,  VertexPosition_I16_XY, 	 
    VertexPosition_Unk, VertexPosition_D16_XY,  VertexPosition_I16_XY,  VertexPosition_I16_XY, 
    VertexPosition_Unk, VertexPosition_D32_XY,  VertexPosition_I32_XY,  VertexPosition_I32_XY, 	 
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,    
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,     
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,     
    VertexPosition_Unk, VertexPosition_D8_XYZ,  VertexPosition_I8_XYZ,  VertexPosition_I8_XYZ, 	 
    VertexPosition_Unk, VertexPosition_D8_XYZ,  VertexPosition_I8_XYZ,  VertexPosition_I8_XYZ, 
    VertexPosition_Unk, VertexPosition_D16_XYZ, VertexPosition_I16_XYZ, VertexPosition_I16_XYZ,	 
    VertexPosition_Unk, VertexPosition_D16_XYZ, VertexPosition_I16_XYZ, VertexPosition_I16_XYZ, 
    VertexPosition_Unk, VertexPosition_D32_XYZ, VertexPosition_I32_XYZ, VertexPosition_I32_XYZ,	 
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,		
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,		 
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk	
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Color Decoding


#define VERTEXLOADER_COLOR_UNDEF(name)      void VertexColor_##name(u16 index) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
}

static void VertexColor_Unk(u16 index) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex color!!! Ask neobrain to implement me!");
}

static void VertexColor_DRGB565(u16 index) {
    u16 rgb = FifoPop16();
    u8 r = ((rgb >> 11) & 0x1f) * 8;
    u8 g = ((rgb >> 5) & 0x3f) * 4;
    u8 b = ((rgb >> 0) & 0x1f) * 8;

    video_core::g_renderer->VertexColor0_Send((r << 24) | 
                                              (g << 16) |
                                              (b << 8) |
                                              0xff);
}

// correct
static void VertexColor_DRGB8(u16 index) {
    video_core::g_renderer->VertexColor0_Send((FifoPop24() << 8) | 0xff);
}

// correct
static void VertexColor_DRGBA8(u16 index) {
    video_core::g_renderer->VertexColor0_Send(FifoPop32());
}

static void VertexColor_IRGB8(u16 index) {
	u32 rgba = MemoryRead32(CP_DATA_COL0_ADDR(index)) | 0xff;
    video_core::g_renderer->VertexColor0_Send(rgba);
}

// correct
static void VertexColor_IRGBA8(u16 index) {
	u32 rgba = MemoryRead32(CP_DATA_COL0_ADDR(index));
    video_core::g_renderer->VertexColor0_Send(rgba);
}

//VERTEXLOADER_COLOR_UNDEF(DRGB565);
//VERTEXLOADER_COLOR_UNDEF(DRGB8)
VERTEXLOADER_COLOR_UNDEF(DRGBA4);
VERTEXLOADER_COLOR_UNDEF(DRGBA6);
//VERTEXLOADER_COLOR_UNDEF(DRGBA8);

VERTEXLOADER_COLOR_UNDEF(IRGB565);
//VERTEXLOADER_COLOR_UNDEF(IRGB8);
VERTEXLOADER_COLOR_UNDEF(IRGBA4);
VERTEXLOADER_COLOR_UNDEF(IRGBA6);
//VERTEXLOADER_COLOR_UNDEF(IRGBA8);

VertexLoaderTable LookupColor[0x40] = {
	VertexColor_Unk,    VertexColor_DRGB565,    VertexColor_IRGB565,    VertexColor_IRGB565, 
	VertexColor_Unk,    VertexColor_DRGB8,      VertexColor_IRGB8,      VertexColor_IRGB8, // 7
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_IRGB8,      VertexColor_IRGB8, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, // 15
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_IRGBA8,     VertexColor_IRGBA8, // 23
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, // 31
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, // 39
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_DRGBA4,     VertexColor_IRGBA4,     VertexColor_IRGBA4, // 47
	VertexColor_Unk,    VertexColor_DRGBA6,     VertexColor_IRGBA6,     VertexColor_IRGBA6, 
	VertexColor_Unk,    VertexColor_DRGBA8,     VertexColor_IRGBA8,     VertexColor_IRGBA8, // 55
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Normal Decoding

#define VERTEXLOADER_NORMAL_UNDEF(name)     void VertexNormal_##name(u16 index) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
}

static void VertexNormal_Unk(u16 index) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex normal!!! Ask neobrain to implement me!");
}

static void VertexNormal_D8_3(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop24();
}

static void VertexNormal_D16_3(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop16();
    FifoPop16();
    FifoPop16();
}

static void VertexNormal_D32_3(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop32();
    FifoPop32();
    FifoPop32();
}

static void VertexNormal_I8_3(u16 index) {
    // TODO(ShizZy): ImplementMe
}

static void VertexNormal_I16_3(u16 index) {
    // TODO(ShizZy): ImplementMe
}

static void VertexNormal_I32_3(u16 index) {
    // TODO(ShizZy): ImplementMe
}

//VERTEXLOADER_NORMAL_UNDEF(D8_3);
//VERTEXLOADER_NORMAL_UNDEF(D16_3);
//VERTEXLOADER_NORMAL_UNDEF(D32_3);
//VERTEXLOADER_NORMAL_UNDEF(I8_3);
//VERTEXLOADER_NORMAL_UNDEF(I16_3);
//VERTEXLOADER_NORMAL_UNDEF(I32_3);

// TODO(ShizZy): Decoding 9 normals.... needs to be added to this table
VertexLoaderTable LookupNormal[0x40] = {
	VertexNormal_Unk,   VertexNormal_D8_3,      VertexNormal_I8_3,      VertexNormal_I8_3, 
	VertexNormal_Unk,   VertexNormal_D8_3,      VertexNormal_I8_3,      VertexNormal_I8_3, // 7
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_D16_3,     VertexNormal_I16_3,     VertexNormal_I16_3, // 15
	VertexNormal_Unk,   VertexNormal_D32_3,     VertexNormal_I32_3,     VertexNormal_I32_3, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 23
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 31
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 39
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 47
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 55
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// TexCoord Decoding

#define VERTEXLOADER_TEXCOORD_UNDEF(name)   void VertexTexCoord_##name(u16 index) { \
    _ASSERT_MSG(TGP, 0, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__); \
}

static void VertexTexCoord_Unk(u16 index) {
    _ASSERT_MSG(TGP, 0, "Unknown Vertex texcoord!!! Ask neobrain to implement me!");
}

static void VertexTexCoord_D8_ST(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop16();
}

static void VertexTexCoord_D16_ST(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop32();
}

static void VertexTexCoord_D32_ST(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop32();FifoPop32();
}

static void VertexTexCoord_D8_S(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop8();
}

static void VertexTexCoord_D16_S(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop16();
}

static void VertexTexCoord_D32_S(u16 index) {
    // TODO(ShizZy): ImplementMe
    FifoPop32();
}

//----------

static void VertexTexCoord_I8_ST(u16 index) {
    // TODO(ShizZy): ImplementMe
}

static void VertexTexCoord_I16_ST(u16 index) {
    // TODO(ShizZy): ImplementMe
}

static void VertexTexCoord_I32_ST(u16 index) {
    // TODO(ShizZy): ImplementMe
}

static void VertexTexCoord_I8_S(u16 index) {
    // TODO(ShizZy): ImplementMe
}

static void VertexTexCoord_I16_S(u16 index) {
    // TODO(ShizZy): ImplementMe
}

static void VertexTexCoord_I32_S(u16 index) {
    // TODO(ShizZy): ImplementMe
}

/*
VERTEXLOADER_TEXCOORD_UNDEF(I8_S);
VERTEXLOADER_TEXCOORD_UNDEF(I16_S);
VERTEXLOADER_TEXCOORD_UNDEF(I32_S);
VERTEXLOADER_TEXCOORD_UNDEF(I8_ST);
VERTEXLOADER_TEXCOORD_UNDEF(I16_ST);
VERTEXLOADER_TEXCOORD_UNDEF(I32_ST);
*/
//VERTEXLOADER_TEXCOORD_UNDEF(D8_S);
//VERTEXLOADER_TEXCOORD_UNDEF(D16_S);
//VERTEXLOADER_TEXCOORD_UNDEF(D32_S);
//VERTEXLOADER_TEXCOORD_UNDEF(D8_ST);
//VERTEXLOADER_TEXCOORD_UNDEF(D16_ST);
//VERTEXLOADER_TEXCOORD_UNDEF(D32_ST);

VertexLoaderTable LookupTexCoord[0x40] = {
	VertexTexCoord_Unk, VertexTexCoord_D8_S,    VertexTexCoord_I8_S,    VertexTexCoord_I8_S, 
	VertexTexCoord_Unk, VertexTexCoord_D8_S,    VertexTexCoord_I8_S,    VertexTexCoord_I8_S,
	VertexTexCoord_Unk, VertexTexCoord_D16_S,   VertexTexCoord_I16_S,   VertexTexCoord_I16_S, 
	VertexTexCoord_Unk, VertexTexCoord_D16_S,   VertexTexCoord_I16_S,   VertexTexCoord_I16_S, 
	VertexTexCoord_Unk, VertexTexCoord_D32_S,   VertexTexCoord_I32_S,   VertexTexCoord_I32_S, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_D8_ST,   VertexTexCoord_I8_ST,   VertexTexCoord_I8_ST, 
	VertexTexCoord_Unk, VertexTexCoord_D8_ST,   VertexTexCoord_I8_ST,   VertexTexCoord_I8_ST, 
	VertexTexCoord_Unk, VertexTexCoord_D16_ST,  VertexTexCoord_I16_ST,  VertexTexCoord_I16_ST, 
	VertexTexCoord_Unk, VertexTexCoord_D16_ST,  VertexTexCoord_I16_ST,  VertexTexCoord_I16_ST, 
	VertexTexCoord_Unk, VertexTexCoord_D32_ST,  VertexTexCoord_I32_ST,  VertexTexCoord_I32_ST, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk
};

////////////////////////////////////////////////////////////////////////////////
// PRIMITIVE DECODING

// send vertex to the renderer
void DecodeVertex() {
   
    GXVertexFormat pos, nrm, col[2], tex[8];

    // Position
    pos.vcd = VCD_POS;
    if (pos.vcd) {
        pos.cnt = (GXCompCnt)VAT_POSCNT;
        pos.fmt = (GXCompType)VAT_POSFMT;
        pos.dqf = 1.0f / (1 << VAT_POSSHFT);
        video_core::g_renderer->VertexPosition_SetType(pos.fmt, pos.cnt);
    }


    // Normal
    nrm.vcd = VCD_NRM;
    if (nrm.vcd) {
        nrm.cnt = (GXCompCnt)VAT_NRMCNT;
        nrm.fmt = (GXCompType)VAT_NRMFMT;
    }

    // Color 0
    col[0].vcd = VCD_COL0;
    if (col[0].vcd) {
        col[0].cnt = (GXCompCnt)VAT_COL0CNT;
        col[0].fmt = (GXCompType)VAT_COL0FMT;
    }

    // Color 1
    col[1].vcd = VCD_COL1;
    if (col[1].vcd) {
        col[1].cnt = (GXCompCnt)VAT_COL1CNT;
        col[1].fmt = (GXCompType)VAT_COL1FMT;
    }

    if (VCD_MIDX) {
        //_ASSERT_MSG(TGP, 0, "VCD_MIDX decoding not implemented yet!");
    }
    if (VCD_PMIDX) {
        //_ASSERT_MSG(TGP, 0, "VCD_PMIDX decoding not implemented yet!");
        g_pm_index = FifoPop8();
    }
    // offset according to texture matrixes..
    u32 data = CP_VCD_LO(0);
    for(int i = 0; i < 8; i++) {
        data >>= 1;
        if(data & 1) {
            FifoPop8();
        }
    }

    // DECODE POSITION FORMAT

    // get pos index (if used)
    switch(pos.vcd) {
    case GX_VCD_DIRECT:
        LookupPosition[VTX_FORMAT_VCD(pos)](0);
        break;
    case GX_VCD_INDEX8:
        LookupPosition[VTX_FORMAT_VCD(pos)](FifoPop8());
        break;
    case GX_VCD_INDEX16:
        LookupPosition[VTX_FORMAT_VCD(pos)](FifoPop16());
        break;
    }

    // DECODE NORMAL FORMAT (FAKE)

    switch(nrm.vcd) {
    case GX_VCD_DIRECT:
        LookupNormal[VTX_FORMAT_VCD(nrm)](0);
        break;
    case GX_VCD_INDEX8:
        LookupNormal[VTX_FORMAT_VCD(nrm)](FifoPop8());
        break;
    case GX_VCD_INDEX16:
        LookupNormal[VTX_FORMAT_VCD(nrm)](FifoPop16());
        break;
    }

    // DECODE DIFFUSE COLOR FORMAT

    // get color index (if used)
    switch(col[0].vcd) {
    case GX_VCD_DIRECT:
        LookupColor[VTX_FORMAT_VCD(col[0])](0);
        break;
    case GX_VCD_INDEX8:
        LookupColor[VTX_FORMAT_VCD(col[0])](FifoPop8());
        break;
    case GX_VCD_INDEX16:
        LookupColor[VTX_FORMAT_VCD(col[0])](FifoPop16());
        break;
    }

    // DECODE SPECULAR COLOR FORMAT

    switch(col[1].vcd) {
    case GX_VCD_DIRECT:
        LookupColor[VTX_FORMAT_VCD(col[1])](0);
        break;
    case GX_VCD_INDEX8:
        LookupColor[VTX_FORMAT_VCD(col[1])](FifoPop8());
        break;
    case GX_VCD_INDEX16:
        LookupColor[VTX_FORMAT_VCD(col[1])](FifoPop16());
        break;
    }

    // DECODE TEXCOORDS
    
    for (int n = 0; n < 7; n++) {
        
        if (VCD_TEX(n)) {
            tex[n].vcd         = VCD_TEX(n);


            // TODO(ShizZy): Refactor this - it's terribly hax
            switch (n) {
            case 0:
                tex[n].cnt         = (GXCompCnt) VAT_TEX0CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX0FMT;
                break;
            case 1:
                tex[n].cnt         = (GXCompCnt) VAT_TEX1CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX1FMT;
                break;
            case 2:
                tex[n].cnt         = (GXCompCnt) VAT_TEX2CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX2FMT;
                break;
            case 3:
                tex[n].cnt         = (GXCompCnt) VAT_TEX3CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX3FMT;
                break;
            case 4:
                tex[n].cnt         = (GXCompCnt) VAT_TEX4CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX4FMT;
                break;
            case 5:
                tex[n].cnt         = (GXCompCnt) VAT_TEX5CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX5FMT;
                break;
            case 6:
                tex[n].cnt         = (GXCompCnt) VAT_TEX6CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX6FMT;
                break;
            case 7:
                tex[n].cnt         = (GXCompCnt) VAT_TEX7CNT;
                tex[n].fmt         = (GXCompType)VAT_TEX7FMT;
                break;
            }

            switch(tex[n].vcd) {
            case GX_VCD_DIRECT:
                LookupTexCoord[VTX_FORMAT_VCD(tex[n])](0);
                break;
            case GX_VCD_INDEX8:
                LookupTexCoord[VTX_FORMAT_VCD(tex[n])](FifoPop8());
                break;
            case GX_VCD_INDEX16:
                LookupTexCoord[VTX_FORMAT_VCD(tex[n])](FifoPop16());
                break;
            }
        }
    }
}

// begin primitive drawing
void DecodePrimitive(GXPrimitive type, int count) {

    video_core::g_renderer->BeginPrimitive(type, count);

    for (int i = 0; i < (count >> 3); i++) {
        DecodeVertex();
        video_core::g_renderer->VertexNext();
        DecodeVertex();
        video_core::g_renderer->VertexNext();
        DecodeVertex();
        video_core::g_renderer->VertexNext();
        DecodeVertex();
        video_core::g_renderer->VertexNext();
        DecodeVertex();
        video_core::g_renderer->VertexNext();
        DecodeVertex();
        video_core::g_renderer->VertexNext();
        DecodeVertex();
        video_core::g_renderer->VertexNext();
        DecodeVertex();
        video_core::g_renderer->VertexNext();
    }
    switch(count & 7) {
	case 7:
        DecodeVertex();
        video_core::g_renderer->VertexNext();
	case 6:
        DecodeVertex();
        video_core::g_renderer->VertexNext();
    case 5:
        DecodeVertex();
        video_core::g_renderer->VertexNext();
    case 4:
        DecodeVertex();
        video_core::g_renderer->VertexNext();
    case 3:
        DecodeVertex();
        video_core::g_renderer->VertexNext();
    case 2:
        DecodeVertex();
        video_core::g_renderer->VertexNext();
    case 1:
        DecodeVertex();
        video_core::g_renderer->VertexNext();
    }
    
    video_core::g_renderer->EndPrimitive();
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

/// Initialize the Vertex Loader
void VertexLoaderInit() {
}

/// Shutdown the Vertex Loader
void VertexLoaderShutdown() {
}

} // namespace
