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
#define XF_VIEWPORT_SCALE_X			g_xf_regs.mem[0x1a]
#define XF_VIEWPORT_SCALE_Y			g_xf_regs.mem[0x1b]
#define XF_VIEWPORT_SCALE_Z			g_xf_regs.mem[0x1c]
#define XF_VIEWPORT_OFFSET_X		g_xf_regs.mem[0x1d]
#define XF_VIEWPORT_OFFSET_Y		g_xf_regs.mem[0x1e]
#define XF_VIEWPORT_OFFSET_Z		g_xf_regs.mem[0x1f]
#define XF_PROJECTION_A				g_xf_regs.mem[0x20]
#define XF_PROJECTION_B				g_xf_regs.mem[0x21]
#define XF_PROJECTION_C				g_xf_regs.mem[0x22]
#define XF_PROJECTION_D				g_xf_regs.mem[0x23]
#define XF_PROJECTION_E				g_xf_regs.mem[0x24]
#define XF_PROJECTION_F				g_xf_regs.mem[0x25]
#define XF_PROJECTION_ORTHOGRAPHIC	g_xf_regs.mem[0x26]

#define XF_ADDR_MASK                0xff00  ///< Mask the base of an xf address

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

// XF Num Colors
typedef struct _XFNumColors{
	u32 n;
}XFNumColors;

// XF Color
typedef struct {
	union{
		struct{
			u8 a, b, g, r;
		};
		u32 _u32;
	};
}XFColorChannel;

// XF Color
typedef struct {
	union{
		struct{
			unsigned materialsrc : 1;
			unsigned lightfunc : 1;
			unsigned light0 : 1;
			unsigned light1 : 1;
			unsigned light2 : 1;
			unsigned light3 : 1;
			unsigned ambientsrc : 1;
			unsigned diffuseatten : 1;
			unsigned attenenable : 1;
			unsigned attenselect : 1;
			unsigned light4 : 1;
			unsigned light5 : 1;
			unsigned light6 : 1;
			unsigned light7 : 1;
			unsigned rid : 18;
		};
		u32 _u32;
	};
}XFColorControl;

////////////////////////////////////////////////////////////////////////////////////////////////////
// XF constants

static const int kXFMemEntriesNum   = 64;   ///< Number of entries used in each block of XF memory

////////////////////////////////////////////////////////////////////////////////////////////////////
// XF Registers

/// XF memory uniion
union XFMemory{
    struct{
        u32             pad0[0x9];          ///< Padding - unused
        XFNumColors     numcolors;          ///< Number of colors
        XFColorChannel  ambient[0x2];       ///< Ambient color channels
        XFColorChannel  material[0x2];      ///< Material color channels
        XFColorControl  colorcontrol[0x2];  ///< Color control
        u32             pad1[0xf0];         ///< Padding - unused
    };
    u32 mem[0x100];                         ///< Addressable memory
};

extern u32      g_tf_mem[0x800];            ///< Transformation memory
extern XFMemory g_xf_regs;                  ///< XF registers
extern f32      g_projection_matrix[16];    ///< Decoded projection matrix
extern f32      g_position_matrix[16];      ///< Decoded position matrix
extern f32      g_view_matrix[16];          ///< Decoded view matrix

/*! 
 * \brief Write data into a XF register
 * \param length Length of write (in 32-bit words)
 * \param addr Starting addres to write to
 * \param regs Register data to write
 */
void XFRegisterWrite(u16 length, u16 addr, u32* regs);

/*! 
 * \brief Write data into a XF register indexed-form
 * \param n CP index address
 * \param length Length of write (in 32-bit words)
 * \param addr Starting addres to write to
 */
void XFLoadIndexed(u8 n, u16 index, u8 length, u16 addr);

/// Initialize XF
void XFInit();

} // namespace

#endif // VIDEO_CORE_XF_MEM_