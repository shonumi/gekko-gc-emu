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
#include "fifo.h"

enum {
    CP_NOT_PRESENT  = 0,
    CP_DIRECT       = 1,   ///< GX vertex VCD type direct data
    CP_INDEX8       = 2,   ///< GX vertex VCD type indexed 8-bit
    CP_INDEX16      = 3,   ///< GX vertex VCD type indexed 16-bit
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CP register decoding

/// Vertex descripter (VCD) low
union CPVertDescLo {
    struct {
        // 0: not present
        // 1: present
        u32 pos_midx_enable     : 1;
        u32 tex0_midx_enable    : 1;
        u32 tex1_midx_enable    : 1;
        u32 tex2_midx_enable    : 1;
        u32 tex3_midx_enable    : 1;
        u32 tex4_midx_enable    : 1;
        u32 tex5_midx_enable    : 1;
        u32 tex6_midx_enable    : 1;
        u32 tex7_midx_enable    : 1;
        u32 position            : 2;
        u32 normal              : 2;
        u32 color0              : 2;
        u32 color1              : 2;
        u32                     : 15;
    };
    u32 _u32;
};

/// Vertex descripter (VCD) high
union CPVertDescHi {
    struct {
        u32 tex0_coord          : 2;
        u32 tex1_coord          : 2;
        u32 tex2_coord          : 2;
        u32 tex3_coord          : 2;
        u32 tex4_coord          : 2;
        u32 tex5_coord          : 2;
        u32 tex6_coord          : 2;
        u32 tex7_coord          : 2;
        u32                     : 16;
    };
    u32 _u32;
};

/// Vertex attribute table (VAT) register A
union CPVatRegA {
    struct {
        // 0:8
        u32 pos_count       : 1;
        u32 pos_format      : 3; 
        u32 pos_shift       : 5; 
        // 9:12
        u32 normal_count    : 1; 
        u32 normal_format   : 3; 
        // 13:16
        u32 col0_count      : 1;
        u32 col0_format     : 3; 
        // 17:20
        u32 col1_count      : 1;
        u32 col1_format     : 3; 
        // 21:29
        u32 tex0_count      : 1;
        u32 tex0_format     : 3; 
        u32 tex0_shift      : 5;
        // 30:31
        u32 byte_dequant    : 1;
        u32 normal_index_3  : 1;
    };
    u32 _u32;
    
    inline u32 get_pos() { return ((pos_count << 3) | pos_format); }
    inline u32 get_normal() { return ((normal_count << 3) | normal_format); }
    inline u32 get_col0() { return ((col0_count << 3) | col0_format); }
    inline u32 get_col1() { return ((col1_count << 3) | col1_format); }
    inline u32 get_tex0() { return ((tex0_count << 3) | tex0_format); }
};

/// Vertex attribute table (VAT) register B
union CPVatRegB {
    struct {
        // 0:8
        u32 tex1_count      : 1;
        u32 tex1_format     : 3; 
        u32 tex1_shift      : 5;
        // 9:17
        u32 tex2_count      : 1;
        u32 tex2_format     : 3; 
        u32 tex2_shift      : 5;
        // 18:26
        u32 tex3_count      : 1;
        u32 tex3_format     : 3; 
        u32 tex3_shift      : 5;
        // 27:30
        u32 tex4_count      : 1;
        u32 tex4_format     : 3; 
        // 
        u32                 : 1;
    };
    u32 _u32;

    inline u32 get_tex1() { return ((tex1_count << 3) | tex1_format); }
    inline u32 get_tex2() { return ((tex2_count << 3) | tex2_format); }
    inline u32 get_tex3() { return ((tex3_count << 3) | tex3_format); }
    inline u32 get_tex4() { return ((tex4_count << 3) | tex4_format); }
};

/// Vertex attribute table (VAT) register C
union CPVatRegC {
    struct  {
        // 0:4
        u32 tex4_shift      : 5;
        // 5:13
        u32 tex5_count      : 1;
        u32 tex5_format     : 3; 
        u32 tex5_shift      : 5;
        // 14:22
        u32 tex6_count      : 1;
        u32 tex6_format     : 3; 
        u32 tex6_shift      : 5;
        // 23:31
        u32 tex7_count      : 1;
        u32 tex7_format     : 3; 
        u32 tex7_shift      : 5;
    };
    u32 _u32;

    inline u32 get_tex5() { return ((tex5_count << 3) | tex5_format); }
    inline u32 get_tex6() { return ((tex6_count << 3) | tex6_format); }
    inline u32 get_tex7() { return ((tex7_count << 3) | tex7_format); }
};


/// Matrix index A
union CPMatrixIndexA {
    struct {
        u32 pos_normal_midx : 6;
        u32 tex0_midx       : 6;
        u32 tex1_midx       : 6;
        u32 tex2_midx       : 6;
        u32 tex3_midx       : 6;
        u32                 : 2;
    };
    u32 _u32;
};

/// Matrix index B
union CPMatrixIndexB {
    struct {
        u32 tex4_midx   : 6;
        u32 tex5_midx   : 6;
        u32 tex6_midx   : 6;
        u32 tex7_midx   : 6;
        u32             : 8;
    };
    u32 _u32;
};

// cp: register reference
#define CP_VCD_LO(idx)					gp::g_cp_regs.mem[0x50 + idx]
#define CP_VCD_HI(idx)					gp::g_cp_regs.mem[0x60 + idx]
#define CP_VAT_A						gp::g_cp_regs.mem[0x70 + gp::g_cur_vat]
#define CP_VAT_B						gp::g_cp_regs.mem[0x80 + gp::g_cur_vat]
#define CP_VAT_C						gp::g_cp_regs.mem[0x90 + gp::g_cur_vat]
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
//#define MIDX_POS						(CP_MATIDX_REG_A & 0x3f)	
// index for texture matrices 0-3
//#define MIDX_TEX03(n)					((CP_MATIDX_REG_A >> ((n * 6) + 6)) & 0x3f)	
// index for texture matrices 4-7
//#define MIDX_TEX47(n)					((CP_MATIDX_REG_B >> (((n - 4) * 6))) & 0x3f)		

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



    
#define VTX_FORMAT_VCD(vtx)				((VTX_FORMAT(vtx) << 2) | vtx.vcd)


////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

////////////////////////////////////////////////////////////////////////////////////////////////////
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


#define CP_VCD_LO(idx)					gp::g_cp_regs.mem[0x50 + idx]
#define CP_VCD_HI(idx)					gp::g_cp_regs.mem[0x60 + idx]
#define CP_VAT_A						gp::g_cp_regs.mem[0x70 + gp::g_cur_vat]
#define CP_VAT_B						gp::g_cp_regs.mem[0x80 + gp::g_cur_vat]
#define CP_VAT_C						gp::g_cp_regs.mem[0x90 + gp::g_cur_vat]
#define CP_DATA_POS_ADDR(idx)			(gp::g_cp_regs.mem[0xa0] + (idx) * gp::g_cp_regs.mem[0xb0])
#define CP_DATA_NRM_ADDR(idx)			(gp::g_cp_regs.mem[0xa1] + (idx) * gp::g_cp_regs.mem[0xb1])
#define CP_DATA_COL0_ADDR(idx)			(gp::g_cp_regs.mem[0xa2] + (idx) * gp::g_cp_regs.mem[0xb2])
#define CP_DATA_TEX_ADDR(idx, n)		(gp::g_cp_regs.mem[0xa4 + n] + (idx) * gp::g_cp_regs.mem[0xb4 + n])		
#define CP_MATIDX_REG_A					gp::g_cp_regs.mem[0x30]
#define CP_MATIDX_REG_B					gp::g_cp_regs.mem[0x40]

/// CP memory
union CPMemory {
    struct {
        u32             pad0[0x30];
        CPMatrixIndexA  matrix_index_a; // 0x30
        u32             pad1[0xF];
        CPMatrixIndexB  matrix_index_b; // 0x40
        u32             pad2[0xF];
        CPVertDescLo    vcd_lo[0x8];         // 0x50
        u32             pad3[0x8];
        CPVertDescHi    vcd_hi[0x8];         // 0x60
        u32             pad4[0x8];

        CPVatRegA       vat_reg_a[0x8];     // 0x70
        u32             pad5[0x8];
        CPVatRegB       vat_reg_b[0x8];     // 0x80
        u32             pad6[0x8];
        CPVatRegC       vat_reg_c[0x8];     // 0x90
        u32             pad7[0x8];

        u32 pad8[0x60];
        
    };
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