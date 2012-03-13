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

#if(0) //#ifdef USE_INLINE_ASM
#pragma todo(gx_tf_pos_xy may need asm rewrite to avoid unneeded mul/add)
	_asm
	{
		mov eax, pmtx
		mov ebx, v
		mov dword ptr [ebx+ 8], 0x00000000	//v[2] = 0.0f;
		mov dword ptr [ebx+12], 0x3F800000	//v[3] = 1.0f;
		mov edx, d
		movups xmm0, [eax+0]		//tmtx.3,tmtx.2,tmtx.1,tmtx.0
		movups xmm1, [eax+16]		//tmtx.7,tmtx.6,tmtx.5,tmtx.4
		movups xmm2, [eax+32]		//tmtx.11,tmtx.10,tmtx.9,tmtx.8
		movups xmm3, [ebx]			//v.3,v.2,v.1,v.0

		//tmtx * v
		mulps xmm0, xmm3			//tmtx.3*v.3,tmtx.2*v.2,tmtx.1*v.1,tmtx.0*v.0
		mulps xmm1, xmm3			//tmtx.7*v.3,tmtx.6*v.2,tmtx.5*v.1,tmtx.4*v.0
		mulps xmm2, xmm3			//tmtx.11*v.3,tmtx.10*v.2,tmtx.9*v.1,tmtx.8*v.0

		//swap the register around for the add
		movhlps xmm4, xmm0			//xmm4 = ?,?,tmtx.3,tmtx.2
		movlhps xmm5, xmm1			//xmm5 = tmtx.5,tmtx.4,?,?
		movhlps xmm6, xmm2			//xmm6 = ?,?,tmtx.11,tmtx.10

		addps xmm0, xmm4			//?,?,tmtx.1+3,tmtx.0+2
		addps xmm1, xmm5			//tmtx.7+5,tmtx.6+4,?,?
		addps xmm2, xmm6			//?,?,tmtx.9+11,tmtx.8+10

		movsd xmm1, xmm0			//tmtx.5+7,tmtx.4+6,tmtx.1+3,tmtx.0+2

#pragma todo(SSE3 will make this block 2 instructions with MOVSHDUP)
		movaps xmm0, xmm1
		shufps xmm1, xmm1, 11110101b	//tmtx.5+7,tmtx.5+7,tmtx.1+3,tmtx.1+3
		movaps xmm3, xmm2
		shufps xmm3, xmm3, 00000101b	//?,?,tmtx.9+11,tmtx.9+11

		addps xmm0, xmm1				//?,tmtx.4567,?,tmtx.0123
		addps xmm3, xmm2				//?,?,?,tmtx.891011

		//combine our registers now
		shufps xmm0, xmm2, 00001000b

		movups [edx], xmm0
	};
#else
	// transform 
	d[0] = (pmtx[0])*v[0] + (pmtx[1])*v[1] + (pmtx[ 3]);
	d[1] = (pmtx[4])*v[0] + (pmtx[5])*v[1] + (pmtx[ 7]);
	d[2] = (pmtx[8])*v[0] + (pmtx[9])*v[1] + (pmtx[11]);
#endif
}

// transform 3d vertex position (software)
void gx_transform::gx_tf_pos_xyz(f32* d, f32 *v)
{
	f32 *pmtx = XF_POSITION_MATRIX;

#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov eax, pmtx
		mov ebx, v
		mov dword ptr [ebx+12], 0x3F800000	//v[3] = 1.0f;
		mov edx, d
		movups xmm0, [eax+0]		//tmtx.3,tmtx.2,tmtx.1,tmtx.0
		movups xmm1, [eax+16]		//tmtx.7,tmtx.6,tmtx.5,tmtx.4
		movups xmm2, [eax+32]		//tmtx.11,tmtx.10,tmtx.9,tmtx.8
		movups xmm3, [ebx]			//v.3,v.2,v.1,v.0

		//tmtx * v
		mulps xmm0, xmm3			//tmtx.3*v.3,tmtx.2*v.2,tmtx.1*v.1,tmtx.0*v.0
		mulps xmm1, xmm3			//tmtx.7*v.3,tmtx.6*v.2,tmtx.5*v.1,tmtx.4*v.0
		mulps xmm2, xmm3			//tmtx.11*v.3,tmtx.10*v.2,tmtx.9*v.1,tmtx.8*v.0

		//swap the register around for the add
		movhlps xmm4, xmm0			//xmm4 = ?,?,tmtx.3,tmtx.2
		movlhps xmm5, xmm1			//xmm5 = tmtx.5,tmtx.4,?,?
		movhlps xmm6, xmm2			//xmm6 = ?,?,tmtx.11,tmtx.10

		addps xmm0, xmm4			//?,?,tmtx.1+3,tmtx.0+2
		addps xmm1, xmm5			//tmtx.7+5,tmtx.6+4,?,?
		addps xmm2, xmm6			//?,?,tmtx.9+11,tmtx.8+10

		movsd xmm1, xmm0			//tmtx.5+7,tmtx.4+6,tmtx.1+3,tmtx.0+2

#pragma todo(SSE3 will make this block 2 instructions with MOVSHDUP)
		movaps xmm0, xmm1
		shufps xmm1, xmm1, 11110101b	//tmtx.5+7,tmtx.5+7,tmtx.1+3,tmtx.1+3
		movaps xmm3, xmm2
		shufps xmm3, xmm3, 00000101b	//?,?,tmtx.9+11,tmtx.9+11

		addps xmm0, xmm1				//?,tmtx.4567,?,tmtx.0123
		addps xmm3, xmm2				//?,?,?,tmtx.891011

		//combine our registers now
		shufps xmm0, xmm2, 00001000b

		movups [edx], xmm0
	};
#else
	// transform 
	d[0] = (pmtx[0])*v[0] + (pmtx[1])*v[1] + (pmtx[ 2])*v[2] + (pmtx[ 3]);
	d[1] = (pmtx[4])*v[0] + (pmtx[5])*v[1] + (pmtx[ 6])*v[2] + (pmtx[ 7]);
	d[2] = (pmtx[8])*v[0] + (pmtx[9])*v[1] + (pmtx[10])*v[2] + (pmtx[11]);
#endif
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

#if(0) //#ifdef USE_INLINE_ASM
#pragma todo(gx_tf_tex_st may need asm rewrite to avoid unneeded mul/add)
	_asm
	{
		mov eax, tmtx
		mov ebx, v
		mov dword ptr [ebx+ 8], 0x00000000	//v[2] = 0.0f;
		mov dword ptr [ebx+12], 0x3F800000	//v[3] = 1.0f;
		mov edx, d
		movups xmm0, [eax+0]		//tmtx.3,tmtx.2,tmtx.1,tmtx.0
		movups xmm1, [eax+16]		//tmtx.7,tmtx.6,tmtx.5,tmtx.4
		movups xmm2, [eax+32]		//tmtx.11,tmtx.10,tmtx.9,tmtx.8
		movups xmm3, [ebx]			//v.3,v.2,v.1,v.0

		//tmtx * v
		mulps xmm0, xmm3			//tmtx.3*v.3,tmtx.2*v.2,tmtx.1*v.1,tmtx.0*v.0
		mulps xmm1, xmm3			//tmtx.7*v.3,tmtx.6*v.2,tmtx.5*v.1,tmtx.4*v.0
		mulps xmm2, xmm3			//tmtx.11*v.3,tmtx.10*v.2,tmtx.9*v.1,tmtx.8*v.0

		//swap the register around for the add
		movhlps xmm4, xmm0			//xmm4 = ?,?,tmtx.3,tmtx.2
		movlhps xmm5, xmm1			//xmm5 = tmtx.5,tmtx.4,?,?
		movhlps xmm6, xmm2			//xmm6 = ?,?,tmtx.11,tmtx.10

		addps xmm0, xmm4			//?,?,tmtx.1+3,tmtx.0+2
		addps xmm1, xmm5			//tmtx.7+5,tmtx.6+4,?,?
		addps xmm2, xmm6			//?,?,tmtx.9+11,tmtx.8+10

		movsd xmm1, xmm0			//tmtx.5+7,tmtx.4+6,tmtx.1+3,tmtx.0+2

#pragma todo(SSE3 will make this block 2 instructions with MOVSHDUP)
		movaps xmm0, xmm1
		shufps xmm1, xmm1, 11110101b	//tmtx.5+7,tmtx.5+7,tmtx.1+3,tmtx.1+3
		movaps xmm3, xmm2
		shufps xmm3, xmm3, 00000101b	//?,?,tmtx.9+11,tmtx.9+11

		addps xmm0, xmm1				//?,tmtx.4567,?,tmtx.0123
		addps xmm3, xmm2				//?,?,?,tmtx.891011

		//combine our registers now
		shufps xmm0, xmm2, 00001000b

		movups [edx], xmm0
	};
#else
	// transform 
	d[0] = (tmtx[0])*v[0] + (tmtx[1])*v[1] + (tmtx[ 3]);
	d[1] = (tmtx[4])*v[0] + (tmtx[5])*v[1] + (tmtx[ 7]);
	d[2] = (tmtx[8])*v[0] + (tmtx[9])*v[1] + (tmtx[11]);
#endif
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

#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov eax, tmtx
		mov ebx, v
		mov dword ptr [ebx+12], 0x3F800000	//v[3] = 1.0f;
		mov edx, d
		movups xmm0, [eax+0]		//tmtx.3,tmtx.2,tmtx.1,tmtx.0
		movups xmm1, [eax+16]		//tmtx.7,tmtx.6,tmtx.5,tmtx.4
		movups xmm2, [eax+32]		//tmtx.11,tmtx.10,tmtx.9,tmtx.8
		movups xmm3, [ebx]			//v.3,v.2,v.1,v.0

		//tmtx * v
		mulps xmm0, xmm3			//tmtx.3*v.3,tmtx.2*v.2,tmtx.1*v.1,tmtx.0*v.0
		mulps xmm1, xmm3			//tmtx.7*v.3,tmtx.6*v.2,tmtx.5*v.1,tmtx.4*v.0
		mulps xmm2, xmm3			//tmtx.11*v.3,tmtx.10*v.2,tmtx.9*v.1,tmtx.8*v.0

		//swap the register around for the add
		movhlps xmm4, xmm0			//xmm4 = ?,?,tmtx.3,tmtx.2
		movlhps xmm5, xmm1			//xmm5 = tmtx.5,tmtx.4,?,?
		movhlps xmm6, xmm2			//xmm6 = ?,?,tmtx.11,tmtx.10

		addps xmm0, xmm4			//?,?,tmtx.1+3,tmtx.0+2
		addps xmm1, xmm5			//tmtx.7+5,tmtx.6+4,?,?
		addps xmm2, xmm6			//?,?,tmtx.9+11,tmtx.8+10

		movsd xmm1, xmm0			//tmtx.5+7,tmtx.4+6,tmtx.1+3,tmtx.0+2

#pragma todo(SSE3 will make this block 2 instructions with MOVSHDUP)
		movaps xmm0, xmm1
		shufps xmm1, xmm1, 11110101b	//tmtx.5+7,tmtx.5+7,tmtx.1+3,tmtx.1+3
		movaps xmm3, xmm2
		shufps xmm3, xmm3, 00000101b	//?,?,tmtx.9+11,tmtx.9+11

		addps xmm0, xmm1				//?,tmtx.4567,?,tmtx.0123
		addps xmm3, xmm2				//?,?,?,tmtx.891011

		//combine our registers now
		shufps xmm0, xmm2, 00001000b

		movups [edx], xmm0
	};
#else

	// transform 
	d[0] = (tmtx[0])*v[0] + (tmtx[1])*v[1] + (tmtx[ 2])*v[2] + (tmtx[ 3]);
	d[1] = (tmtx[4])*v[0] + (tmtx[5])*v[1] + (tmtx[ 6])*v[2] + (tmtx[ 7]);
	d[2] = (tmtx[8])*v[0] + (tmtx[9])*v[1] + (tmtx[10])*v[2] + (tmtx[11]);
#endif

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