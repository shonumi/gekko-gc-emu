/*!
* Copyright (C) 2005-2012 Gekko Emulator
*
* \file    cp_mem.h
* \author  ShizZy <shizzy247@gmail.com>
* \date    2012-03-08
* \brief   Implementation of CP for the graphics processor
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

#ifndef VIDEO_CORE_CP_MEM_
#define VIDEO_CORE_CP_MEM_

#include "common.h"

#include "gx_types.h"

////////////////////////////////////////////////////////////////////////////////
// VERTEX PROCESSOR MACROS

// vertex processor send data
#define GX_SEND_COLOR_RGB(r, g, b)		cv->color[0] = r; cv->color[1] = g; cv->color[2] = b; cv->color[3] = 127;
#define GX_SEND_COLOR_RGBA(r, g, b, a)	cv->color[0] = r; cv->color[1] = g; cv->color[2] = b; cv->color[3] = a;
#define GX_SEND_POSITION_XY(x, y)		cv->pos[0] = (x * _vtx->dqf); cv->pos[1] = (y * _vtx->dqf);
#define GX_SEND_POSITION_XYZ(x, y, z)	cv->pos[0] = (x * _vtx->dqf); cv->pos[1] = (y * _vtx->dqf); cv->pos[2] = (z * _vtx->dqf);	
#define GX_SEND_TEXCOORD_S(i, s)		cv->tex[i][0] = (s * _vtx->dqf);
#define GX_SEND_TEXCOORD_ST(i, s, t)	cv->tex[i][0] = (s * _vtx->dqf); cv->tex[i][1] = (t * _vtx->dqf);

// cp: register reference
#define CP_VCD_LO(idx)					gp::g_cp_regs.mem[0x50 + idx]
#define CP_VCD_HI(idx)					gp::g_cp_regs.mem[0x60 + idx]
#define CP_VAT_A						gp::g_cp_regs.mem[0x70 + vat]
#define CP_VAT_B						gp::g_cp_regs.mem[0x80 + vat]
#define CP_VAT_C						gp::g_cp_regs.mem[0x90 + vat]
#define CP_DATA_POS_ADDR(idx)			(gp::g_cp_regs.mem[0xa0] + (idx) * gp::g_cp_regs.mem[0xb0])
#define CP_DATA_NRM_ADDR(idx)			(gp::g_cp_regs.mem[0xa1] + (idx) * gp::g_cp_regs.mem[0xb1])
#define CP_DATA_COL0_ADDR(idx)			(gp::g_cp_regs.mem[0xa2] + (idx) * gp::g_cp_regs.mem[0xb2])
#define CP_DATA_TEX_ADDR(idx, n)		(gp::g_cp_regs.mem[0xa4 + n] + (idx) * gp::g_cp_regs.mem[0xb4 + n])		
#define CP_MATIDX_REG_A					gp::g_cp_regs.mem[0x30]
#define CP_MATIDX_REG_B					gp::g_cp_regs.mem[0x40]

// Address reference (used for XF)
#define CP_IDX_ADDR(idx, n)				(gp::g_cp_regs.mem[0xac + n] + (idx) * gp::g_cp_regs.mem[0xbc + n])

// midx: matrix indexes
// index for position/normal matrix	
#define MIDX_POS						(CP_MATIDX_REG_A & 0x3f)	
// index for texture matrices 0-3
#define MIDX_TEX03(n)					((CP_MATIDX_REG_A >> ((n * 6) + 6)) & 0x3f)	
// index for texture matrices 4-7
#define MIDX_TEX47(n)					((CP_MATIDX_REG_B >> (((n - 4) * 6))) & 0x3f)		

// vcd: stores format type	
#define VCD_MIDX						(CP_VCD_LO(0) & 0x1ff)	
// position matrix
#define VCD_PMIDX						(CP_VCD_LO(0) & 1)	
// texture marix
#define VCD_TMIDX(n)					((CP_VCD_LO(0) >> (1 + n)) & 1)	
// position
#define VCD_POS							((CP_VCD_LO(0) >> 9) & 3)		
// normal
#define VCD_NRM							((CP_VCD_LO(0) >> 11) & 3)			
// color 0 (diff)
#define VCD_COL0						((CP_VCD_LO(0) >> 13) & 3)		
// color 1 (spec)
#define VCD_COL1						((CP_VCD_LO(0) >> 15) & 3)		
// texture coordinates	
#define VCD_TEX(n)						((CP_VCD_HI(0) >> (n * 2)) & 3)		

// vat: stores format kind				
// position count
#define VAT_POSCNT						(CP_VAT_A & 1)	
// position format
#define VAT_POSFMT						((CP_VAT_A >> 1) & 7)	
// position shift
#define VAT_POSSHFT						((CP_VAT_A >> 4) & 0x1f)	
// diffuse color count
#define VAT_COL0CNT						((CP_VAT_A >> 13) & 1)	
// diffuse color format
#define VAT_COL0FMT						((CP_VAT_A >> 14) & 7)	
// specular color count
#define VAT_COL1CNT						((CP_VAT_A >> 17) & 1)	
// specular color format
#define VAT_COL1FMT						((CP_VAT_A >> 18) & 7)	
// normal count
#define VAT_NRMCNT						((CP_VAT_A >> 9) & 1)	
// normal format
#define VAT_NRMFMT						((CP_VAT_A >> 10) & 7)			
// 1:shift u8/s8/u16/s16, 0:shift u16/s16
#define VAT_BYTEDEQUANT					((CP_VAT_A >> 30) & 1)				
#define VAT_TEX0CNT						((CP_VAT_A >> 21) & 1)				 
#define VAT_TEX0FMT						((CP_VAT_A >> 22) & 7)				 
#define VAT_TEX0SHFT					((CP_VAT_A >> 25) & 0x1f)			
#define VAT_TEX1CNT						((CP_VAT_B >> 0) & 1)				 
#define VAT_TEX1FMT						((CP_VAT_B >> 1) & 7)				 
#define VAT_TEX1SHFT					((CP_VAT_B >> 4) & 0x1f)			
#define VAT_TEX2CNT						((CP_VAT_B >> 9) & 1)				 
#define VAT_TEX2FMT						((CP_VAT_B >> 10) & 7)				 
#define VAT_TEX2SHFT					((CP_VAT_B >> 13) & 0x1f)			
#define VAT_TEX3CNT						((CP_VAT_B >> 18) & 1)				 
#define VAT_TEX3FMT						((CP_VAT_B >> 19) & 7)				 
#define VAT_TEX3SHFT					((CP_VAT_B >> 22) & 0x1f)			
#define VAT_TEX4CNT						((CP_VAT_B >> 27) & 1)				 
#define VAT_TEX4FMT						((CP_VAT_B >> 28) & 7)				 
#define VAT_TEX4SHFT					(CP_VAT_C & 0x1f)					
#define VAT_TEX5CNT						((CP_VAT_C >> 5) & 1)				 
#define VAT_TEX5FMT						((CP_VAT_C >> 6) & 7)				 
#define VAT_TEX5SHFT					((CP_VAT_C >> 9) & 0x1f)			
#define VAT_TEX6CNT						((CP_VAT_C >> 14) & 1)				 
#define VAT_TEX6FMT						((CP_VAT_C >> 15) & 7)				 
#define VAT_TEX6SHFT					((CP_VAT_C >> 18) & 0x1f)			
#define VAT_TEX7CNT						((CP_VAT_C >> 23) & 1)				 
#define VAT_TEX7FMT						((CP_VAT_C >> 24) & 7)				 
#define VAT_TEX7SHFT					((CP_VAT_C >> 27) & 0x1f)			

// format decoding
#define VTX_FORMAT(vtx)					((vtx.cnt << 3) | vtx.fmt)
#define VTX_FORMAT_PTR(vtx)             ((vtx->cnt << 3) | vtx->fmt)
#define VTX_FORMAT_VCD(vtx)				((VTX_FORMAT(vtx) << 2) | vtx.vcd)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

// gx vertex 
struct Vertex
{
    f32 pos[4];
    f32 tpos[4];
    f32 tex[8][2];
    f32 tex_temp[2];
    f32 ttex[4];
    f32 nrm[4];
    s8  color[4], col0[4], col1[3];
    u8  is3d;
};

/// vertex base data structure
struct VertexData
{
    GXCompCnt   cnt;            // count
    GXCompType  fmt;            // format
    u8          vcd;            // type
    u8          num;            // number (textures)
    u16         index;          // offset (indexed, 8 or 18bit)
    u32         position;       // offset (direct)
    f32         dqf;            // scale factor
    void*       vtx_format_vcd; // ptr to func[(vtx_format << 2) | vcd](...);
    u32         vtx_format;     // (cnt << 3) | fmt
};

////////////////////////////////////////////////////////////////////////////////
// VERTEX PROCESSOR SIZE ARRAYS

/// size (in bytes) of position format
static const u8 kVertexPositionSize[16] = {
    2,  2,  4,  4,  8,  0,  0,  0,  // two pos xy
    3,  3,  6,  6,  12, 0,  0,  0   // three pos xyz
};

/// size (in bytes) of color format
static const u8 kVertexColorSize[16] = {
    2,  3,  4,  0,  0,  0,  0,  0,  // col rgb
    0,  0,  0,  2,  3,  4,  0,  0   // col rgba
};

/// size (in bytes) of normal format
static const u8 kVertexNormalSize[16] = {
    0,  3,  0,  6,  12, 0,  0,  0,  // three nrms
    0,  9,  0,  18, 36, 0,  0,  0   // nine nrms
};

/// size (in bytes) of texture format
static const u8 kVertexTextureSize[16] = {
    1,  1,  2,  2,  4,  0,  0,  0,  // one coord s
    2,  2,  4,  4,  8,  0,  0,  0   // two coords st
};

/// CP memory
union CPMemory {
    u32 mem[0x100];
};

extern CPMemory g_cp_regs; ///< CP memory/registers

/*!
 * \brief Write a CP register
 * \param addr Address (8-bit) of register
 * \param data Data (32-bit) to write to register
 */
void CPRegisterWrite(u8 addr, u32 data);

/// Initialize CP
void CPInit();

} // namespace

#endif // VIDEO_CORE_CP_MEM_