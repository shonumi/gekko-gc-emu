/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    cp_mem.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-08
 * @brief   Implementation of CP for the graphics processor
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

#ifndef VIDEO_CORE_CP_MEM_
#define VIDEO_CORE_CP_MEM_

#include "common.h"

#include "gx_types.h"
#include "fifo.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// CP register decoding

#define CP_REG_VCD_LO   0x50 // 0x50 + 7
#define CP_REG_VCD_HI   0x60 // 0x60 + 7

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
        u32 pos_type        : 3; 
        u32 pos_shift       : 5; 
        // 9:12
        u32 normal_count    : 1; 
        u32 normal_type     : 3; 
        // 13:16
        u32 col0_count      : 1;
        u32 col0_type       : 3; 
        // 17:20
        u32 col1_count      : 1;
        u32 col1_type       : 3; 
        // 21:29
        u32 tex0_count      : 1;
        u32 tex0_type       : 3; 
        u32 tex0_shift      : 5;
        // 30:31
        u32 byte_dequant    : 1;
        u32 normal_index_3  : 1;
    };
    u32 _u32;
    
    inline u32 get_pos() { return ((pos_count << 3) | pos_type); }
    inline u32 get_normal() { return ((normal_count << 3) | normal_type); }
    inline u32 get_col0() { return ((col0_count << 3) | col0_type); }
    inline u32 get_col1() { return ((col1_count << 3) | col1_type); }
    inline u32 get_tex0() { return ((tex0_count << 3) | tex0_type); }
    inline u32 get_pos_dqf_enabled() { return (GX_F32 != pos_type) ? 1 : 0; }
	inline f32 get_pos_dqf() { return (1.0f / f32(1 << pos_shift)); }
	inline f32 get_tex0_dqf() { return (1.0f / f32(1 << tex0_shift)); }
};

/// Vertex attribute table (VAT) register B
union CPVatRegB {
    struct {
        // 0:8
        u32 tex1_count      : 1;
        u32 tex1_type       : 3; 
        u32 tex1_shift      : 5;
        // 9:17
        u32 tex2_count      : 1;
        u32 tex2_type       : 3; 
        u32 tex2_shift      : 5;
        // 18:26
        u32 tex3_count      : 1;
        u32 tex3_type       : 3; 
        u32 tex3_shift      : 5;
        // 27:30
        u32 tex4_count      : 1;
        u32 tex4_type       : 3; 
        // 
        u32                 : 1;
    };
    u32 _u32;

    inline u32 get_tex1() { return ((tex1_count << 3) | tex1_type); }
    inline u32 get_tex2() { return ((tex2_count << 3) | tex2_type); }
    inline u32 get_tex3() { return ((tex3_count << 3) | tex3_type); }
    inline u32 get_tex4() { return ((tex4_count << 3) | tex4_type); }

	inline f32 get_tex1_dqf() { return (1.0f / f32(1 << tex1_shift)); }
	inline f32 get_tex2_dqf() { return (1.0f / f32(1 << tex2_shift)); }
	inline f32 get_tex3_dqf() { return (1.0f / f32(1 << tex3_shift)); }
};

/// Vertex attribute table (VAT) register C
union CPVatRegC {
    struct  {
        // 0:4
        u32 tex4_shift      : 5;
        // 5:13
        u32 tex5_count      : 1;
        u32 tex5_type       : 3; 
        u32 tex5_shift      : 5;
        // 14:22
        u32 tex6_count      : 1;
        u32 tex6_type       : 3; 
        u32 tex6_shift      : 5;
        // 23:31
        u32 tex7_count      : 1;
        u32 tex7_type       : 3; 
        u32 tex7_shift      : 5;
    };
    u32 _u32;

    inline u32 get_tex5() { return ((tex5_count << 3) | tex5_type); }
    inline u32 get_tex6() { return ((tex6_count << 3) | tex6_type); }
    inline u32 get_tex7() { return ((tex7_count << 3) | tex7_type); }

	inline f32 get_tex4_dqf() { return (1.0f / f32(1 << tex4_shift)); }
	inline f32 get_tex5_dqf() { return (1.0f / f32(1 << tex5_shift)); }
	inline f32 get_tex6_dqf() { return (1.0f / f32(1 << tex6_shift)); }
	inline f32 get_tex7_dqf() { return (1.0f / f32(1 << tex7_shift)); }
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

/// Array base
union CPArrayBase {
    struct {
        u32 addr_base       : 26;
        u32 unused          : 6;
    };
    u32 _u32;
};

/// Array stride
union CPArrayStride {
    struct {
        u32 addr_stride     : 8;
        u32 unused          : 24;
    };
    u32 _u32;
};

// cp: register reference
#define CP_VCD_LO(idx)					gp::g_cp_regs.mem[0x50 + idx]
#define CP_VCD_HI(idx)					gp::g_cp_regs.mem[0x60 + idx]
#define CP_VAT_A						gp::g_cp_regs.mem[0x70 + gp::g_cur_vat]
#define CP_VAT_B						gp::g_cp_regs.mem[0x80 + gp::g_cur_vat]
#define CP_VAT_C						gp::g_cp_regs.mem[0x90 + gp::g_cur_vat]#define CP_MATIDX_REG_A					gp::g_cp_regs.mem[0x30]
#define CP_MATIDX_REG_B					gp::g_cp_regs.mem[0x40]

// Address reference (used for XF)
#define CP_IDX_ADDR(idx, n)				(gp::g_cp_regs.mem[0xac + n] + (idx) * gp::g_cp_regs.mem[0xbc + n])

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
    2,  3,  4,  2,  3,  4,  0,  0,  // col rgb
    2,  3,  4,  2,  3,  4,  0,  0   // col rgba
};

/// size (in bytes) of normal format
static const u8 kVertexNormalSize[16] = {
    3,  3,  6,  6,  12, 0,  0,  0,  // three nrms
    9,  9,  18, 18, 36, 0,  0,  0   // nine nrms
};

/// size (in bytes) of texture format
static const u8 kVertexTextureSize[16] = {
    1,  1,  2,  2,  4,  0,  0,  0,  // one coord s
    2,  2,  4,  4,  8,  0,  0,  0   // two coords st
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// CP registers

#define CP_REG_VCD_LO       0x50
#define CP_REG_VCD_HI       0x60
#define CP_REG_VAT_A        0x70
#define CP_REG_VAT_B        0x80
#define CP_REG_VAT_C        0x90

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
        CPArrayBase     array_base[0x10];   // 0xA0
        CPArrayStride   array_stride[0x10]; // 0xB0
        u32 pad8[0x40];
    };
    u32 mem[0x100];
};

extern CPMemory g_cp_regs; ///< CP memory/registers

/**
 * Write a CP register
 * @param addr Address (8-bit) of register
 * @param data Data (32-bit) to write to register
 */
void CP_RegisterWrite(u8 addr, u32 data);

/// Initialize CP
void CP_Init();

} // namespace

#endif // VIDEO_CORE_CP_MEM_