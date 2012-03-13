// gx_transform.h
// (c) 2005,2009 Gekko Team

#ifndef __gx_transform_H__
#define __gx_transform_H__

////////////////////////////////////////////////////////////////////////////////
// TRANSFORMATION ENGINE MACROS

#define XF_TEX(n)						xf.mem[0x40 + n]
// 0 - ST - (s,t): texmul is 2x4 / 1 - STQ - (s,t,q): texmul is 3x4
#define XF_TEX_PROJECTION(n)			((XF_TEX(n) >> 1) & 0x1)	
// 0 - AB11 - (A, B, 1.0, 1.0) (regular texture source) / 1- ABC1 - (A, B, C, 
// 1.0) (geometry or normal source)
#define XF_TEX_INPUT_FORM(n)			((XF_TEX(n) >> 2) & 0x1)
// should be masked to 0x7 but it's unknown what the MSB is for - format - see 
// enum gx_xf_tex_texgen_type
#define XF_TEX_TEXGEN_TYPE(n)			((XF_TEX(n) >> 4) & 0x7)		
// should be masked to 0x1f but it's unknown what the MSB is for - format - see 
// enum gx_xf_tex_source_row																	
#define XF_TEX_SOURCE_ROW(n)			((XF_TEX(n) >> 7) & 0x1f)		
// bump mapping source texture - format - use regular transformed tex(n) for 
// bump  mapping source
#define XF_TEX_EMBOSS_SOURCE(n)			((XF_TEX(n) >> 12) & 0x7)		
// bump mapping source light - format - use light #n for bump map direction 
// source (10 to 17)
#define XF_TEX_EMBOSS_LIGHT(n)			((XF_TEX(n) >> 15) & 0x7)		

// xf: memory reference
#define XF_POSITION_MATRIX			&*(f32*)&xfmem[(gx_vertex::pm_index * 4)];
#define XF_GEOMETRY_MATRIX			&*(f32*)&xfmem[(MIDX_POS * 4)];									
#define XF_TEX_MATRIX(n)			&*(f32*)&xfmem[(gx_vertex::tm_index[n] * 4)];
#define XF_TEXTURE_MATRIX03(n)		&*(f32*)&xfmem[(MIDX_TEX03(n) * 4)];
#define XF_TEXTURE_MATRIX47(n)		&*(f32*)&xfmem[(MIDX_TEX47(n) * 4)];

// xf: enumerations

// texture inrow source
//		Specifies location of incoming textures in vertex (row specific) 
//		(i.e.: geometry is row0, normal is row1, etc . . . ) for regular tfms 
enum gx_xf_tex_source_row
{
	XF_GEOM_INROW					= 0x0,	// vertex positions used as tfm src
	XF_NORMAL_INROW					= 0x1,	// vertex normals used as tfm src
	XF_COLORS_INROW					= 0x2,	// vertex colors used as tfm src
	XF_BINORMAL_T_INROW				= 0x3,
	XF_BINORMAL_B_INROW				= 0x4,
	XF_TEX0_INROW					= 0x5,
	XF_TEX1_INROW					= 0x6,
	XF_TEX2_INROW					= 0x7,
	XF_TEX3_INROW					= 0x8,
	XF_TEX4_INROW					= 0x9,
	XF_TEX5_INROW					= 0xa,
	XF_TEX6_INROW					= 0xb,
	XF_TEX7_INROW					= 0xc
};

// texture texgen type
enum gx_xf_tex_texgen_type
{
	// Regular transformation (transform incoming data)
	XF_REGULAR						= 0x0,	
	// texgen bump mapping
	XF_EMBOSS_MAP					= 0x1,	
	// color texgen: (s,t)=(r,g:b) (g and b are concatenated), color 0
	XF_COLOR_STRGBC0 				= 0x2,	
	// color texgen: (s,t)=(r,g:b) (g and b are concatenated), color 1
	XF_COLOR_STRGBC1				= 0x3		
};

////////////////////////////////////////////////////////////////////////////////
// TRANSFORMATION ENGINE NAMESPACE

namespace gx_transform
{
	// vector routines
	void vec_scale(void);
	void vec_multiply(void);
	void vec_copy(void);

	// transform 2d vertex position (software)
	void gx_tf_pos_xy(f32* d, f32 *v);		
	// transform 3d vertex position (software)
	void gx_tf_pos_xyz(f32* d, f32 *v);	
	// transform vertex (hardware accelerated)
	void gx_tf_pos_hardware(void);				
	// transform texture coordinate (2d)
	void gx_tf_tex_st(f32* d, f32 *v, u8 _indexed, u8 n);	
	// transform texture coordinate (3d - projected)
	void gx_tf_tex_stq(f32* d, f32 *v, u8 _indexed, u8 n);		

	// transformation
	void tex_gen(int _n);

	// namespace
	void initialize(void);
	void destroy(void);
};

////////////////////////////////////////////////////////////////////////////////

#endif

