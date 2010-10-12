// gx_states.cpp
// (c) 2005,2006 Gekko Team ~ShizZY

#include "..\emu.h"
#include "glew\glew.h"
#include "..\opengl.h"
#include "..\low level\hardware core\hw_pe.h"
#include "..\low level\hardware core\hw_gx.h"
#include "gx_states.h"
#include "gx_texture.h"

////////////////////////////////////////////////////////////////////////////////

u8 gx_states::efb_copy;
extern GLuint texcache[0x10000];

////////////////////////////////////////////////////////////////////////////////
// GX API STATES

// sets the culling mode
void gx_states::set_cullmode(void)
{
	if(bp.genmode.reject_en)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(gx_type_cullmode[bp.genmode.reject_en]);
	}else glDisable(GL_CULL_FACE); 
}

// sets the scissor box
void gx_states::set_scissors(void)
{
	int width = (BP_SCISSOR_X1 - BP_SCISSOR_X0);
	int height = (BP_SCISSOR_Y1 - BP_SCISSOR_Y0);

	glEnable(GL_SCISSOR_TEST);
	glScissor(
		(BP_SCISSOR_X0 * gl.video_width_scale), 
		((480 - (BP_SCISSOR_Y0 + height + 1)) * gl.video_height_scale), 
		((width + 1) * gl.video_width_scale), 
		((height + 1) * gl.video_height_scale));
}

// sets the size of lines and points
void gx_states::set_lpsize(void)
{
	glPointSize((f32)BP_POINT_SIZE);
	glLineWidth((f32)BP_LINE_SIZE);
}

// sets the current viewport
void gx_states::set_viewport(void)
{
	GLclampd znear, zfar;
	f32 w, h, x, y;

	// get z far distance
	zfar = ((GLclampd)toFLOAT(XF_VIEWPORT_OFFSET_Z) / GX_VIEWPORT_ZMAX);

	// get z near distance
	znear = (-((GLclampd)toFLOAT(XF_VIEWPORT_SCALE_Z) / 
		(GLclampd)GX_VIEWPORT_ZMAX) + zfar);
	
	// get viewport dimensions
	w = (toFLOAT(XF_VIEWPORT_SCALE_X) * 2);
	h = (-toFLOAT(XF_VIEWPORT_SCALE_Y) * 2);

	x = (toFLOAT(XF_VIEWPORT_OFFSET_X) - (342 + toFLOAT(XF_VIEWPORT_SCALE_X)));
	y = (toFLOAT(XF_VIEWPORT_OFFSET_Y) - (342 - toFLOAT(XF_VIEWPORT_SCALE_Y)));

	// send to efb
	glDepthRange(znear, zfar);
	glViewport(
		(x * gl.video_width_scale), 
		((480 - (y + h)) * gl.video_height_scale), 
		(w * gl.video_width_scale), 
		(h * gl.video_height_scale)
	);
}

// sets the cleared efb color
void gx_states::set_copyclearcolor(void)
{
	// unpack rgba8888 format
	f32 r = ((f32)(BP_PE_COPYCLEAR_AR & 0xff) / 255.0f);
	f32 g = ((f32)((BP_PE_COPYCLEAR_GB >> 8) & 0xff) / 255.0f);
	f32 b = ((f32)(BP_PE_COPYCLEAR_GB & 0xff) / 255.0f);
	f32 a = ((f32)((BP_PE_COPYCLEAR_AR >> 8) & 0xff) / 255.0f);

	// send to efb
	glClearColor(r, g, b, a);
}

// sets copy clear z
void gx_states::set_copyclearz(void)
{
	// unpack z data
	GLclampd z = ((GLclampd)BP_PE_COPYCLEAR_Z_VALUE) / GX_VIEWPORT_ZMAX;

	// send to efb
	glClearDepth(z);
}

// sets the current matrix projection
void gx_states::set_projection(void)
{
	_t32 mtx[16] = {0};

	glMatrixMode(GL_PROJECTION);

	// is it orthographic mode...
	if(XF_PROJECTION_ORTHOGRAPHIC)
	{ 
		mtx[0]._u32 = XF_PROJECTION_A;
		mtx[5]._u32 = XF_PROJECTION_C;
		mtx[10]._u32 = XF_PROJECTION_E;
		mtx[12]._u32 = XF_PROJECTION_B;
		mtx[13]._u32 = XF_PROJECTION_D;
		mtx[14]._u32 = XF_PROJECTION_F;
		mtx[15]._f32 = 1.0f;
		glLoadMatrixf((f32 *)mtx);
	// otherwise it is perspective mode
	}else{ 
		mtx[0]._u32 = XF_PROJECTION_A;
		mtx[5]._u32 = XF_PROJECTION_C;
		mtx[8]._u32 = XF_PROJECTION_B;
		mtx[9]._u32 = XF_PROJECTION_D;
		mtx[10]._u32 = XF_PROJECTION_E;
		mtx[11]._f32 = -1.0f;
		mtx[14]._u32 = XF_PROJECTION_F;
		glLoadMatrixf((f32 *)mtx);
	}

	// return back matrix mode
	glMatrixMode(GL_MODELVIEW);
}

// performs the render, at the end of a gx frame.
void gx_states::draw_done(void)
{
	if(bp.mem[0x45] & 0x2) // enable interrupt
		GX_PE_FINISH = 1; 
}

// performs the copy from the efb (gx) to xfb (television)
void gx_states::copy_efb(void)
{
	// clear buffer
	if(BP_PE_COPYEXECUTE_XFB)
	{
		if(BP_PE_CMODE0 & 8)
		{
			// render opengl
			OPENGL_Render();
		}

		if(BP_PE_COPYEXECUTE_CLEAR)
		{
//			glPushAttrib (GL_VIEWPORT_BIT);
//			glViewport(0, 0, gl.video_width, gl.video_height);
			//glScissor(0, 0, gl.video_width,gl.video_height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//			glPopAttrib();
		}
	}
}

// set the current z mode
void gx_states::set_zmode(void)
{
	// zmode depth testing is enabled
	if(BP_PE_ZMODE_ENABLE)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(BP_PE_ZMODE_MASK ? GL_TRUE : GL_FALSE);
		glDepthFunc(gx_type_zmode[BP_PE_ZMODE_FUNC]);
	}else{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
	}
}

// set the current color mode
void gx_states::set_cmode0(void)
{
	// cmode0 logic op is enabled
	if(BP_PE_CMODE0_LOGICOP_ENABLE)
	{
		glEnable(GL_COLOR_LOGIC_OP);
		glLogicOp(gx_type_logicop[BP_PE_CMODE0_LOGICOP]);
	}else glDisable(GL_COLOR_LOGIC_OP);

	// cmode0 dither is enabled
	if(BP_PE_CMODE0_DITHER_ENABLE)
		glEnable(GL_DITHER);
	else
		glDisable(GL_DITHER);

	// see if blending is enabled
	if(BP_PE_CMODE0_BLEND_ENABLE) {
		glEnable(GL_BLEND);
	}else glDisable(GL_BLEND);

	// Set the blend function
	if (BP_PE_CMODE0_BLENDOP)
		glBlendFunc(GL_ONE, GL_ONE);
	else glBlendFunc(
			gx_type_blendop_src[BP_PE_CMODE0_SFACTOR], 
			gx_type_blendop_dst[BP_PE_CMODE0_DFACTOR]
		);

	// set the blend equation
	glBlendEquation(BP_PE_CMODE0_BLENDOP ? GL_FUNC_REVERSE_SUBTRACT:GL_FUNC_ADD);
	
	// set the color and alpha mask
    if(BP_PE_CMODE0_ALPHA_MASK && BP_PE_CMODE0_COLOR_MASK)
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
    else if(BP_PE_CMODE0_ALPHA_MASK)
        glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_TRUE);
    else if(BP_PE_CMODE0_COLOR_MASK)
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_FALSE);
}

// handles alpha testing
void gx_states::set_alphafunc(void)
{
	// decode op
	u16 op0 = gx_type_compare[BP_TEV_ALPHAFUNC_OP0];
	u16 op1 = gx_type_compare[BP_TEV_ALPHAFUNC_OP1];

	// decode alpha
	u8 a0 = (BP_TEV_ALPHAFUNC_A0);
	u8 a1 = (BP_TEV_ALPHAFUNC_A1);

	// logic function
	// note: no function currently done - todo and/or/xor/xnor
	u8 logic = BP_TEV_ALPHAFUNC_LOGIC;

	// enable testing
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(op0, (f32)a0 / 255.0f);
}

// set texture mode 0
void gx_states::tx_setmode0(u8 _addr)
{
	GLint mag_filter = ((bp.mem[_addr] >> 4) & 1) ? GL_LINEAR : GL_NEAREST; 
	GLint wrap_s = gx_type_wrapst[bp.mem[_addr] & 3];
	GLint wrap_t = gx_type_wrapst[(bp.mem[_addr] >> 2) & 3];
	GLfloat lodbias = (((f32)(s8)((bp.mem[_addr] >> 9) & 0xff)) / 32.0f);
	int use_mips = ((bp.mem[_addr] >> 5) & 7); // use mip maps
	GLint min_filter = gx_min_filter[use_mips];
	use_mips &= 3;

	if(use_mips)
		glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	else
		glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

	glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, lodbias);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

#pragma todo(tx_setmod0: Missing LOD and BIAS clamping);
}

// load a texture into the cache
void gx_states::load_texture(u8 _idx)
{
	u32 tx_setimage0;
	u32 tx_setimage3;
	gx_texture_data tx;

	tx.num = GX_TX_SETIMAGE_NUM(_idx);

	if(tx.num < 4)
	{
		tx_setimage0 = bp.mem[0x88 + tx.num];
		tx_setimage3 = bp.mem[0x94 + tx.num];
	}
	else
	{
		tx_setimage0 = bp.mem[0xA8 + tx.num - 4];
		tx_setimage3 = bp.mem[0xB4 + tx.num - 4];
	}

	tx.addr = (tx_setimage3 & 0xffffff);

	if(texcache[TEX_CACHE_LOCATION(tx.addr)])
	{
		glBindTexture(GL_TEXTURE_2D, texcache[TEX_CACHE_LOCATION(tx.addr)]);

		if(tx.num < 4) 
			gx_states::tx_setmode0(0x80 + tx.num); 
		else 
			gx_states::tx_setmode0(0xa0 + (tx.num - 4));

		glEnable(GL_TEXTURE_2D);
	}else{
		tx.addr <<= 5;
		tx.fmt = ((tx_setimage0 >> 20) & 0xf);
		tx.height = ((tx_setimage0 >> 10) & 0x3ff) + 1;
		tx.width = (tx_setimage0 & 0x3ff) + 1;
		gx_texture::add(tx);
	}
}

// load data into tmem
void gx_states::load_tlut(void)
{
	u32 cnt = ((BP_TX_LOADTLUT1 >> 10) & 0x3ff) << 5;
	u32 mem_addr = (BP_TX_LOADTLUT0 & 0x1fffff) << 5;
	u32 tlut_addr = (BP_TX_LOADTLUT1 & 0x3ff) << 5;

	memcpy(PTEXTURE_MEM(tlut_addr), &Mem_RAM[mem_addr & RAM_MASK], cnt);
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

void gx_states::reinitialize(void)
{
	set_viewport();
	set_projection();
	copy_efb();
}

void gx_states::initialize(void)
{
	reinitialize();
}

void gx_states::destroy(void)
{
}

////////////////////////////////////////////////////////////////////////////////
// EOF
