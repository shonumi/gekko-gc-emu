// gx_transform.cpp
// (c) 2005,2009 Gekko Team

#include "common.h"
#include "memory.h"
#include "opengl.h"
#include "hw/hw_gx.h"
#include "gx_fifo.h"
#include "gx_vertex.h"
#include "gx_transform.h"
#include "powerpc/cpu_core.h"
#include "powerpc/cpu_core_regs.h"

////////////////////////////////////////////////////////////////////////////////
// POSITION TRANSFORMATION

// transform 2d vertex position (software)
void gx_transform::gx_tf_pos_xy(f32* d, f32 *v)
{
	f32 *pmtx = XF_POSITION_MATRIX;

	// transform 
	d[0] = (pmtx[0])*v[0] + (pmtx[1])*v[1] + (pmtx[ 3]);
	d[1] = (pmtx[4])*v[0] + (pmtx[5])*v[1] + (pmtx[ 7]);
	d[2] = (pmtx[8])*v[0] + (pmtx[9])*v[1] + (pmtx[11]);
}

// transform 3d vertex position (software)
void gx_transform::gx_tf_pos_xyz(f32* d, f32 *v)
{
	f32 *pmtx = XF_POSITION_MATRIX;

	// transform 
	d[0] = (pmtx[0])*v[0] + (pmtx[1])*v[1] + (pmtx[ 2])*v[2] + (pmtx[ 3]);
	d[1] = (pmtx[4])*v[0] + (pmtx[5])*v[1] + (pmtx[ 6])*v[2] + (pmtx[ 7]);
	d[2] = (pmtx[8])*v[0] + (pmtx[9])*v[1] + (pmtx[10])*v[2] + (pmtx[11]);
}

// transform vertex (hardware accelerated)
void gx_transform::gx_tf_pos_hardware(void)
{
	f32 mtx[16], *pmtx = XF_GEOMETRY_MATRIX;

	// convert 4x3 ode to gl 4x4
	mtx[0]  = pmtx[0]; mtx[1]  = pmtx[4]; mtx[2]  = pmtx[8]; mtx[3]  = 0;
	mtx[4]  = pmtx[1]; mtx[5]  = pmtx[5]; mtx[6]  = pmtx[9]; mtx[7]  = 0;
	mtx[8]  = pmtx[2]; mtx[9]  = pmtx[6]; mtx[10] = pmtx[10];mtx[11] = 0;
	mtx[12] = pmtx[3]; mtx[13] = pmtx[7]; mtx[14] = pmtx[11]; mtx[15] = 1;

	// send to efb 
	glLoadMatrixf(mtx);
}

////////////////////////////////////////////////////////////////////////////////
// TEXTURE TRANSFORMATION

// transform texture coordinate (2d)
void gx_transform::gx_tf_tex_st(f32* d, f32 *v, u8 _indexed, u8 n)
{ // possibly incorrect ?
	f32 *tmtx;

	// get texture matrix
	if(_indexed)
	{
		tmtx = XF_TEX_MATRIX(n);
	}else{
		if(n > 3)
		{
			tmtx = XF_TEXTURE_MATRIX47(n);
		}else{
			tmtx = XF_TEXTURE_MATRIX03(n);
		}
	}
	// transform 
	d[0] = (tmtx[0])*v[0] + (tmtx[1])*v[1] + (tmtx[ 3]);
	d[1] = (tmtx[4])*v[0] + (tmtx[5])*v[1] + (tmtx[ 7]);
	d[2] = (tmtx[8])*v[0] + (tmtx[9])*v[1] + (tmtx[11]);
}

// transform texture coordinate (3d - projected)
void gx_transform::gx_tf_tex_stq(f32* d, f32 *v, u8 _indexed, u8 n)
{ // possibly incorrect ?
	f32 *tmtx;

	// get texture matrix
	if(_indexed)
	{
		tmtx = XF_TEX_MATRIX(n);
	}else{
		if(n > 3)
		{
			tmtx = XF_TEXTURE_MATRIX47(n);
		}else{
			tmtx = XF_TEXTURE_MATRIX03(n);
		}
	}
	// transform 
	d[0] = (tmtx[0])*v[0] + (tmtx[1])*v[1] + (tmtx[ 2])*v[2] + (tmtx[ 3]);
	d[1] = (tmtx[4])*v[0] + (tmtx[5])*v[1] + (tmtx[ 6])*v[2] + (tmtx[ 7]);
	d[2] = (tmtx[8])*v[0] + (tmtx[9])*v[1] + (tmtx[10])*v[2] + (tmtx[11]);
}

//////////////////////////////////////////////////////////////////////
// TEXGEN - GX Hardware Texture Coordinate Generation

// You don’t always need explicit texture coordinates if you want to use a texture. For example, you could compute 
// texture coordinates from the vertex positions, or you could use one set of texture coordinates for different texture 
// units.  Texgen (‘texture coord generation’) is a means to accomplish that. This section discusses the texgen unit.
// 
// void GX_SetNumTexGens(u32 nr); - to specifies for how many texture units you want to enable texgen for.
// 
// void GX_SetTexCoordGen(u16 texcoord,u32 tgen_typ,u32 tgen_src,u32 mtxsrc); - controls how texture coordinates are computed
//		texcoord - specifies texgen for which texture coordinate you’re going to configure, and can be one of
// 			GX_TEXCOORD0
// 			GX_TEXCOORD1
// 			GX_TEXCOORD2
// 			GX_TEXCOORD3
// 			GX_TEXCOORD4
// 			GX_TEXCOORD5
// 			GX_TEXCOORD6
// 			GX_TEXCOORD7
//		tgen_typ - used to specify how texture coordinates are generated from the inputs. I don’t really know how all 
//		these functions work, but I provide a few guesses. I have no idea at all how the BUMP stuff works.
//			GX_TG_MTX3x4
//			    output = 3x4matrix * input (u, v, w generated)
//			GX_TG_MTX2x4
//			    output = 2x4matrix * input (u, v generated)
//			GX_TG_BUMP0
//			GX_TG_BUMP1
//			GX_TG_BUMP2
//			GX_TG_BUMP3
//			GX_TG_BUMP4
//			GX_TG_BUMP5
//			GX_TG_BUMP6
//			GX_TG_BUMP7
//			GX_TG_SRTG
//			    I believe this is 'spherical reflection coord generation' like
//			    implemented in OpenGL
//		tgen_src - used to tell the texgen unit what is the input for the generation function. I don’t know what the 
//		TG_TEXn constants are for, the rest is self-explanatory.
//			GX_TG_POS
//			GX_TG_NRM
//			GX_TG_BINRM
//			GX_TG_TANGENT
//			GX_TG_TEX0-7
//			GX_TG_TEXCOORD0-6
//			GX_TG_COLOR0
//			GX_TG_COLOR1
//
// void GX_SetTexCoordGen2(u16 texcoord,u32 tgen_typ,u32 tgen_src,u32 mtxsrc, u32 normalize,u32 postmtx); - like GX_SetTexCoordGen 
//		with a few additional parameters. I don’t know what this does (I guess it does the same as GX_SetTexCoordGen, but 
//		optionally normalizes the generated texture coordinate to unit length and multiplies it with a post processing matrix).

void gx_transform::tex_gen(int _n) // _n - texcoord number
{
	f32 src[4];

	switch(XF_TEX_SOURCE_ROW(_n))
	{
	case XF_GEOM_INROW:
#pragma todo(XMM instruction faster?)
		src[0] = cv->pos[0];
		src[1] = cv->pos[1];
		src[2] = cv->pos[2];
		break;
	case XF_NORMAL_INROW: 
#pragma todo(XMM instruction faster?)
		src[0] = cv->nrm[0];
		src[1] = cv->nrm[1];
		src[2] = cv->nrm[2];
		break;
	//case XF_COLORS_INROW: break;
	//case XF_BINORMAL_T_INROW: break;
	//case XF_BINORMAL_B_INROW: break;
	case XF_TEX0_INROW:
	case XF_TEX1_INROW:
	case XF_TEX2_INROW:
	case XF_TEX3_INROW:
	case XF_TEX4_INROW:
	case XF_TEX5_INROW:
	case XF_TEX6_INROW:
	case XF_TEX7_INROW:
		src[0] = cv->tex[XF_TEX_SOURCE_ROW(_n) - 0x5][0];
		src[1] = cv->tex[XF_TEX_SOURCE_ROW(_n) - 0x5][1];
		break;
	default: 
		printf("gx_transform - texgen - unsupported texture source row %d", XF_TEX_SOURCE_ROW(_n));
		break;
	}

	if(XF_TEX_INPUT_FORM(_n) == 0) src[2] = 1.0f;		// 3rd input dependent on vertex size (0 for 2d, 1 for 3d)
	//src[3] = 1.0f;										// 4th input always 1.0f

	switch(XF_TEX_TEXGEN_TYPE(_n))
	{
	case XF_REGULAR: 
		gx_transform::gx_tf_tex_stq(&cv->ttex[0], src, gx_vertex::tm_index[_n], _n); // transforms texture position
		break; 
	default: 
		printf("gx_transform - texgen - unsupported texgen type %d", XF_TEX_TEXGEN_TYPE(_n));
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

void gx_transform::initialize(void)
{
}

void gx_transform::destroy(void)
{
}

////////////////////////////////////////////////////////////////////////////////
// EOF