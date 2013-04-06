/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    xf_mem.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-12
 * @brief   Implementation of CXF for the graphics processor
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

#ifndef VIDEO_CORE_XF_MEM_
#define VIDEO_CORE_XF_MEM_

#include "common.h"
#include "cp_mem.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Transformation Engine decoding

#define XF_TEX(n)                       xf.mem[0x40 + n]
// 0 - ST - (s,t): texmul is 2x4 / 1 - STQ - (s,t,q): texmul is 3x4
#define XF_TEX_PROJECTION(n)            ((XF_TEX(n) >> 1) & 0x1)	
// 0 - AB11 - (A, B, 1.0, 1.0) (regular texture source) / 1- ABC1 - (A, B, C, 
// 1.0) (geometry or normal source)
#define XF_TEX_INPUT_FORM(n)            ((XF_TEX(n) >> 2) & 0x1)
// should be masked to 0x7 but it's unknown what the MSB is for - format - see 
// enum gx_xf_tex_texgen_type
#define XF_TEX_TEXGEN_TYPE(n)           ((XF_TEX(n) >> 4) & 0x7)		
// should be masked to 0x1f but it's unknown what the MSB is for - format - see 
// enum gx_xf_tex_source_row																	
#define XF_TEX_SOURCE_ROW(n)            ((XF_TEX(n) >> 7) & 0x1f)		
// bump mapping source texture - format - use regular transformed tex(n) for 
// bump  mapping source
#define XF_TEX_EMBOSS_SOURCE(n)         ((XF_TEX(n) >> 12) & 0x7)		
// bump mapping source light - format - use light #n for bump map direction 
// source (10 to 17)
#define XF_TEX_EMBOSS_LIGHT(n)          ((XF_TEX(n) >> 15) & 0x7)		

// xf: transformation memory reference
#define XF_POSITION_MATRIX(index)   (f32*)&gp::g_tf_mem[(index * 4)]
#define XF_MODELVIEW_MATRIX			(f32*)&gp::g_tf_mem[(MIDX_POS * 4)]									
#define XF_TEX_MATRIX(n)			&*(f32*)&gp::g_tf_mem[(gx_vertex::tm_index[n] * 4)]
#define XF_TEXTURE_MATRIX03(n)		&*(f32*)&gp::g_tf_mem[(MIDX_TEX03(n) * 4)]
#define XF_TEXTURE_MATRIX47(n)		&*(f32*)&gp::g_tf_mem[(MIDX_TEX47(n) * 4)]

// xf: register reference

#define XF_SIZE                 0x8000
#define XF_POSMATRICES          0x000
#define XF_POSMATRICES_END      0x100
#define XF_NORMALMATRICES       0x400
#define XF_NORMALMATRICES_END   0x460
#define XF_POSTMATRICES         0x500
#define XF_POSTMATRICES_END     0x600
#define XF_LIGHTS               0x600
#define XF_LIGHTS_END           0x680
#define XF_ERROR                0x1000
#define XF_DIAG                 0x1001
#define XF_STATE0               0x1002
#define XF_STATE1               0x1003
#define XF_CLOCK                0x1004
#define XF_CLIPDISABLE          0x1005
#define XF_SETGPMETRIC          0x1006
#define XF_VTXSPECS             0x1008
#define XF_SETNUMCHAN           0x1009
#define XF_SETCHAN0_AMBCOLOR    0x100a
#define XF_SETCHAN1_AMBCOLOR    0x100b
#define XF_SETCHAN0_MATCOLOR    0x100c
#define XF_SETCHAN1_MATCOLOR    0x100d
#define XF_SETCHAN0_COLOR       0x100e
#define XF_SETCHAN1_COLOR       0x100f
#define XF_SETCHAN0_ALPHA       0x1010
#define XF_SETCHAN1_ALPHA       0x1011
#define XF_DUALTEX              0x1012
#define XF_SETMATRIXINDA        0x1018
#define XF_SETMATRIXINDB        0x1019
#define XF_SETVIEWPORT          0x101a
#define XF_SETZSCALE            0x101c
#define XF_SETZOFFSET           0x101f
#define XF_SETPROJECTIONA       0x1020
#define XF_SETPROJECTIONB       0x1021
#define XF_SETPROJECTIONC       0x1022
#define XF_SETPROJECTIOND       0x1023
#define XF_SETPROJECTIONE       0x1024
#define XF_SETPROJECTIONF       0x1025
#define XF_SETPROJECTION_ORTHO1 0x1026
#define XF_SETPROJECTION_ORTHO2 0x1027
#define XF_SETNUMTEXGENS        0x103f
#define XF_SETTEXGENINFO        0x1040
#define XF_SETPOSMTXINFO        0x1050

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

// texture inrow source
//		Specifies location of incoming textures in vertex (row specific) 
//		(i.e.: geometry is row0, normal is row1, etc . . . ) for regular tfms 
enum XFTexSourceRow {
    XF_GEOM_INROW           = 0x0,	///< vertex positions used as tfm src
    XF_NORMAL_INROW	        = 0x1,	///< vertex normals used as tfm src
    XF_COLORS_INROW         = 0x2,	///< vertex colors used as tfm src
    XF_BINORMAL_T_INROW     = 0x3,
    XF_BINORMAL_B_INROW     = 0x4,
    XF_TEX0_INROW           = 0x5,
    XF_TEX1_INROW           = 0x6,
    XF_TEX2_INROW           = 0x7,
    XF_TEX3_INROW           = 0x8,
    XF_TEX4_INROW           = 0x9,
    XF_TEX5_INROW           = 0xa,
    XF_TEX6_INROW           = 0xb,
    XF_TEX7_INROW           = 0xc
};

/// Texture texgen type
enum XFTexGenType {
    XF_REGULAR              = 0x0,  ///< Regular transformation (transform incoming data)
    XF_EMBOSS_MAP           = 0x1,  ///< TexGen bump mapping
    XF_COLOR_STRGBC0        = 0x2,  ///< Color texgen: (s,t)=(r,g:b) (g/b are concatenated), col0
    XF_COLOR_STRGBC1        = 0x3   ///< Color texgen: (s,t)=(r,g:b) (g and b are concatenated), color 1
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// XF Decoding

/// XF number of colors - 0x1009
union XFNumColors {
    struct {
        u32 num : 2;
    };
    u32 _u32;
};

/// XF dual texture transform (enable - for all texcoords)
union XFDualTexTrans {
    struct {
        u32 enabled : 1;
    };
    u32 _u32;
};

// XFNumTexGens - 0x103f
union XFNumTexGens {
    struct {
        u32 num : 4;
    };
    u32 _u32;
};

/// XF color channel
union XFColorChannel {
    struct {
        u8 a, b, g, r;
    };
    u32 _u32;
};

/// XF viewport
struct XFViewport {
    f32 wd;
    f32 ht;
    f32 z_range;
    f32 x_orig;
    f32 y_orig;
    f32 far_z;
};

/// INVTXSPEC - 0x1008
union INVTXSPEC {
    struct {
        u32 numcolors   : 2;
        u32 numnormals  : 2; // 0 - nothing, 1 - just normal, 2 - normals and binormals
        u32 numtextures : 4;
        u32 unused  : 24;
    };
    u32 _u32;
};

/// XF lighting channel
union XFLitChannel {
    struct {
        u32 material_src    : 1;
        u32 enable_lighting : 1;
        u32 light_mask_0_3  : 4;
        u32 ambsource       : 1;
        u32 diffuse_func    : 2; 
        u32 attn_func       : 2;
        u32 light_mask_4_7  : 4;
    };
    u32 unused              : 32;
    struct  {
        u32 dummy0          : 7;
        u32 light_params    : 4;
        u32 dummy1          : 21;
    };
    unsigned int get_light_mask() const {
        return enable_lighting ? (light_mask_0_3 | (light_mask_4_7 << 4)) : 0;
    }
};

union XFTexGenInfo {
    struct {
        u32 unknown             : 1;
        u32 projection          : 1; ///< XF_TEXPROJ_X
        u32 input_form          : 2; ///< XF_TEXINPUT_X
        u32 texgen_type         : 3; ///< XF_TEXGEN_X
        u32 source_row          : 5; ///< XF_SRCGEOM_X
        u32 emboss_source_shift : 3; ///< What generated texcoord to use
        u32 emboss_light_shift  : 3; ///< Light index that is used
    };
    u32 _u32;
};

union XFPostMtxInfo {
    struct {
        u32 index   : 6; // base row of dual transform matrix
        u32 unused  : 2;
        u32 normalize : 1; // normalize before send operation
    };
    u32 _u32;
};

union XFProjectionMode { 
    struct {
        unsigned is_orthographic    : 1; 
        unsigned unused             :31;
    };
    u32 hex;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// XF constants

static const int kXFMemSize         = 0x460;    ///< Size of XF memory (in 32-bit words)

////////////////////////////////////////////////////////////////////////////////////////////////////
// XF Registers

/// XF memory uniion
union XFMemory {
    struct {
        u32                 error;                          ///< 0x1000
        u32                 diag;                           ///< 0x1001
        u32                 state_0;                        ///< 0x1002
        u32                 state_1;                        ///< 0x1003
        u32                 xf_clock;                       ///< 0x1004
        u32                 clip_disable;                   ///< 0x1005
        u32                 perf0;                          ///< 0x1006
        u32                 perf1;                          ///< 0x1007
        INVTXSPEC           hostinfo;                       ///< 0x1008
        XFNumColors         num_color_channels;             ///< 0x1009
        XFColorChannel      ambient[kGCMaxColorChannels];   ///< 0x100a, 0x100b - Ambient color channels
        XFColorChannel      material[kGCMaxColorChannels];  ///< 0x100c, 0x100d - Material color channels
        XFLitChannel        color[kGCMaxColorChannels];     ///< 0x100e, 0x100f
        XFLitChannel        alpha[kGCMaxColorChannels];     ///< 0x1010, 0x1011
        XFDualTexTrans      dual_tex_trans;                 ///< 0x1012
        u32                 pad1[0x5];                      ///< 0x1013-0x1017
        u32                 matrix_index_a;                 ///< 0x1018
        u32                 matrix_index_b;                 ///< 0x1019
        XFViewport          viewport;                       ///< 0x101a - 0x101f
        f32                 projection_matrix[0x6];         ///< 0x1020 - 0x1025
        XFProjectionMode    projection_mode;                ///< 0x1026
        u32                 pad2[0x18];                     ///< 0x1027 - 0x103e
        XFNumTexGens        num_texgens;                    ///< 0x103f
        XFTexGenInfo        texgen_info[kGCMaxTexGens];     ///< 0x1040 - 0x1047
        u32                 pad3[0x8];                      ///< 0x1048 - 0x104f
        XFPostMtxInfo       postMtxInfo[0x8];               ///< 0x1050 - 0x1057
    };
    u32 mem[0x100];                         ///< Addressable memory
};

extern u32      g_xf_mem[0x800];            ///< Transformation memory
extern XFMemory g_xf_regs;                  ///< XF registers
extern f32      g_projection_matrix[16];    ///< Decoded projection matrix

/** 
 * @brief Write data into a XF memory
 * @param length Length of write (in 32-bit words)
 * @param base_addr Starting addres to write to
 * @param data Register data to write
 */
void XF_Load(u32 length, u32 base_addr, u32* data);

/**
 * @brief Write data into a XF register indexed-form
 * @param n CP index address
 * @param length Length of write (in 32-bit words)
 * @param addr Starting addres to write to
 */
void XF_LoadIndexed(u8 n, u16 index, u8 length, u16 addr);

/// Updates the viewport
void XF_UpdateViewport();   // TODO(ShizZy): The reason for putting this here is to be able to call
                            // from the renderer, which is _bad_ - clean this up for renderer state
                            // management!
/// Initialize XF
void XF_Init();

} // namespace

#endif // VIDEO_CORE_XF_MEM_