// gx_states.h
// (c) 2005,2009 Gekko Team

#ifndef __gx_states_H__
#define __gx_states_H__	

////////////////////////////////////////////////////////////////////////////////
// GX MACROS

// constant
#define GX_VIEWPORT_ZMAX				16777215.0f

// bp: register reference
#define BP_GEN_MODE					bp.mem[0x0]
#define BP_GEN_MODE_REJECTEN		((BP_GEN_MODE >> 14) & 3)
#define BP_SU_SCIS0					bp.mem[0x20]
#define BP_SU_SCIS1					bp.mem[0x21]
#define BP_SCISSOR_X0				(((BP_SU_SCIS0 >> 12) & 0xfff) - 342) 
#define BP_SCISSOR_Y0				((BP_SU_SCIS0 & 0xfff) - 342)
#define BP_SCISSOR_X1				(((BP_SU_SCIS1 >> 12) & 0xfff) - 342)
#define BP_SCISSOR_Y1				((BP_SU_SCIS1 & 0xfff) - 342)
#define BP_SCISSOR_XOFS				(((bp.mem[0x59] & 0x3ff) - 342) << 1)
#define BP_SCISSOR_YOFS				((((bp.mem[0x59] >> 10) & 0x3ff) - 342) << 1)
#define BP_SU_LPSIZE				bp.mem[0x22]
#define BP_POINT_SIZE				((f32)((BP_SU_LPSIZE >> 8) & 0xff) / 6.0f)
#define BP_LINE_SIZE				((f32)(BP_SU_LPSIZE & 0xff) / 6.0f)
#define BP_SU_SSIZE					bp.mem[0x30]	// all 8 textures?
#define BP_SU_TSIZE					bp.mem[0x31]	// ...			
#define BP_PE_ZMODE					bp.mem[0x40]
#define BP_PE_ZMODE_ENABLE			(BP_PE_ZMODE & 1)
#define BP_PE_ZMODE_FUNC			((BP_PE_ZMODE >> 1) & 7)
#define BP_PE_ZMODE_MASK			((BP_PE_ZMODE >> 4) & 1)
#define BP_PE_CMODE0				bp.mem[0x41]
#define BP_PE_CMODE0_LOGICOP		((BP_PE_CMODE0 >> 12) & 0xf)
#define BP_PE_CMODE0_BLENDOP		((BP_PE_CMODE0 >> 11) & 1)
#define BP_PE_CMODE0_LOGICOP_ENABLE	((BP_PE_CMODE0 >> 1) & 1)
#define BP_PE_CMODE0_DITHER_ENABLE	((BP_PE_CMODE0 >> 2) & 1)
#define BP_PE_CMODE0_COLOR_MASK		((BP_PE_CMODE0 >> 3) & 1)
#define BP_PE_CMODE0_ALPHA_MASK		((BP_PE_CMODE0 >> 4) & 1)
#define BP_PE_CMODE0_BLEND_ENABLE	(BP_PE_CMODE0 & 1)
#define BP_PE_CMODE0_SFACTOR		((BP_PE_CMODE0 >> 8) & 7)
#define BP_PE_CMODE0_DFACTOR		((BP_PE_CMODE0 >> 5) & 7)
#define BP_PE_COPYCLEAR_AR			bp.mem[0x4f]
#define BP_PE_COPYCLEAR_GB			bp.mem[0x50]
#define BP_PE_COPYCLEAR_Z			bp.mem[0x51]
#define BP_PE_COPYCLEAR_Z_VALUE		(BP_PE_COPYCLEAR_Z & 0xffffff)
#define BP_PE_COPYEXECUTE			bp.mem[0x52]
#define BP_PE_COPYEXECUTE_CLEAR		(BP_PE_COPYEXECUTE & 0x800)
#define BP_PE_COPYEXECUTE_XFB		(BP_PE_COPYEXECUTE & 0x4000)
#define BP_TX_LOADTLUT0				bp.mem[0x64]
#define BP_TX_LOADTLUT1				bp.mem[0x65]
#define BP_TX_SETMODE0_I0			bp.mem[0x80]
#define BP_TX_SETMODE0_I1			bp.mem[0x81]
#define BP_TX_SETMODE0_I2			bp.mem[0x82]
#define BP_TX_SETMODE0_I3			bp.mem[0x83]
#define BP_TX_SETMODE0_I4			bp.mem[0xa0]
#define BP_TX_SETMODE0_I5			bp.mem[0xa1]
#define BP_TX_SETMODE0_I6			bp.mem[0xa2]
#define BP_TX_SETMODE0_I7			bp.mem[0xa3]
#define BP_TX_SETIMAGE0_I0			bp.mem[0x88]
#define BP_TX_SETIMAGE0_I1			bp.mem[0x89]
#define BP_TX_SETIMAGE0_I2			bp.mem[0x8a]
#define BP_TX_SETIMAGE0_I3			bp.mem[0x8b]
#define BP_TX_SETIMAGE0_I4			bp.mem[0xa8]
#define BP_TX_SETIMAGE0_I5			bp.mem[0xa9]
#define BP_TX_SETIMAGE0_I6			bp.mem[0xaa]
#define BP_TX_SETIMAGE0_I7			bp.mem[0xab]
#define BP_TX_SETIMAGE3_I0			bp.mem[0x94]
#define BP_TX_SETIMAGE3_I1			bp.mem[0x95]
#define BP_TX_SETIMAGE3_I2			bp.mem[0x96]
#define BP_TX_SETIMAGE3_I3			bp.mem[0x97]
#define BP_TX_SETIMAGE3_I4			bp.mem[0xb4]
#define BP_TX_SETIMAGE3_I5			bp.mem[0xb5]
#define BP_TX_SETIMAGE3_I6			bp.mem[0xb6]
#define BP_TX_SETIMAGE3_I7			bp.mem[0xb7]
#define BP_TEV_ALPHAFUNC			bp.mem[0xf3]
#define BP_TEV_ALPHAFUNC_A0			(BP_TEV_ALPHAFUNC & 0xff)
#define BP_TEV_ALPHAFUNC_A1			((BP_TEV_ALPHAFUNC >> 8) & 0xff)
#define BP_TEV_ALPHAFUNC_OP0		((BP_TEV_ALPHAFUNC >> 16) & 0x7)
#define BP_TEV_ALPHAFUNC_OP1		((BP_TEV_ALPHAFUNC >> 19) & 0x7)
#define BP_TEV_ALPHAFUNC_LOGIC		((BP_TEV_ALPHAFUNC >> 22) & 0x3)

// xf: register reference
#define XF_VIEWPORT_SCALE_X			xf.mem[0x1a]
#define XF_VIEWPORT_SCALE_Y			xf.mem[0x1b]
#define XF_VIEWPORT_SCALE_Z			xf.mem[0x1c]
#define XF_VIEWPORT_OFFSET_X		xf.mem[0x1d]
#define XF_VIEWPORT_OFFSET_Y		xf.mem[0x1e]
#define XF_VIEWPORT_OFFSET_Z		xf.mem[0x1f]
#define XF_PROJECTION_A				xf.mem[0x20]
#define XF_PROJECTION_B				xf.mem[0x21]
#define XF_PROJECTION_C				xf.mem[0x22]
#define XF_PROJECTION_D				xf.mem[0x23]
#define XF_PROJECTION_E				xf.mem[0x24]
#define XF_PROJECTION_F				xf.mem[0x25]
#define XF_PROJECTION_ORTHOGRAPHIC	xf.mem[0x26]

// gx definitions
#define GX_TX_SETIMAGE_NUM(x)		(((x & 0x20) >> 3) | (x & 3))

////////////////////////////////////////////////////////////////////////////////
// GX ENUMERATIONS

// gx culling modes
static const GLenum gx_type_cullmode[4] =
{
	GL_NONE, 
	GL_BACK, 
	GL_FRONT,
	GL_FRONT_AND_BACK
};

// gx depth testing modes
static const GLenum gx_type_zmode[8] = 
{
	GL_NEVER,	
	GL_LESS,		
	GL_EQUAL,	
	GL_LEQUAL,
	GL_GREATER,	
	GL_NOTEQUAL,	
	GL_GEQUAL,	
	GL_ALWAYS
};

// gx logic op modes
static const GLenum gx_type_logicop[16] = 
{
    GL_CLEAR, 
	GL_SET, 
	GL_COPY, 
	GL_COPY_INVERTED, 
	GL_NOOP, 
	GL_INVERT, 
	GL_AND, 
	GL_NAND,
    GL_OR, 
	GL_NOR, 
	GL_XOR, 
	GL_EQUIV, 
	GL_AND_REVERSE, 
	GL_AND_INVERTED, 
	GL_OR_REVERSE, 
	GL_OR_INVERTED
};

// gx blend op modes - source
static const GLenum gx_type_blendop_src[8] =
{
    GL_ZERO,
    GL_ONE,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA
};

// gx blend op modes - destination
static const GLenum gx_type_blendop_dst[8] = {
    GL_ZERO, 
	GL_ONE, 
	GL_SRC_COLOR, 
	GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA, 
	GL_ONE_MINUS_SRC_ALPHA,  
	GL_DST_ALPHA, 
	GL_ONE_MINUS_DST_ALPHA
};

// gx texture wrapping
static const GLenum gx_type_wrapst[8] = 
{
	GL_CLAMP_TO_EDGE,                 
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_REPEAT
};

// filtering
static const GLenum gx_min_filter[8] =
{
	GL_NEAREST, 
	GL_NEAREST_MIPMAP_NEAREST, 
	GL_NEAREST_MIPMAP_LINEAR, 
	GL_NONE,
	GL_LINEAR, 
	GL_LINEAR_MIPMAP_NEAREST, 
	GL_LINEAR_MIPMAP_LINEAR, 
	GL_NONE,
};

// gx compare modes
static const GLenum gx_type_compare[8] = 
{
	GL_NEVER,
	GL_LESS,    
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS   
};

////////////////////////////////////////////////////////////////////////////////
// GX API NAMESPACE

namespace gx_states
{
	// gx state variables
	extern u8 efb_copy;

	// gx state control
	void set_viewport(void);
	void set_projection(void);
	void set_gamma(void);
	void set_scissors(void);
	void set_lpsize(void);
	void set_cullmode(void);
	void set_copyclearcolor(void);
	void set_copyclearz(void);
	void set_zmode(void);
	void set_cmode0(void);
	void set_alphafunc(void);
	void tx_setmode0(u8 _addr);
	void tx_setmode1(u8 _addr);
	void draw_done(void);
	void copy_efb(void);
	void load_texture(u8 _idx);
	void load_tlut(void);

	// namespace
	void reinitialize(void);
	void initialize(void);
	void destroy(void);
};

////////////////////////////////////////////////////////////////////////////////
// EOF

#endif