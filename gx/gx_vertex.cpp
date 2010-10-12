// gx_vertex.cpp
// (c) 2005,2009 Gekko Team

#include "..\emu.h"
#include "glew\glew.h"
#include "..\low level\hardware core\hw_gx.h"
#include "..\opengl.h"
#include "gx_fifo.h"
#include "gx_tev.h"
#include "gx_vertex.h"
#include "gx_transform.h"
#include "gx_states.h"

////////////////////////////////////////////////////////////////////////////////

u8		gx_vertex::pm_index;			// position matrix index
u8		gx_vertex::tm_index[8];		// texture matrix indexes
f32		gx_vertex::dqfactor;			// dequantization facotr`
char	gx_vertex::outbuff[64];		// debug out buffer
char	gx_vertex::outbuff2[64];		// ...
char	gx_vertex::outbuff3[64];		// ...
char	gx_vertex::outbuff4[64];		// ...

Vertex	vtxarray[1];						// potential array of cached vertex data
Vertex	*cv;								// current vertex


gx_vertex::VertexListStruct gx_vertex::VertexList[VertexListSize];
u32 gx_vertex::vcd_midx_result;
u32 gx_vertex::vcd_pmidx_result;

gx_vertex_data	pos, nrm, col0, col1, tex[8];

__inline u16 gxMemoryRead16(u32 addr)
{
	if(!(addr & 1))
		return *(u16 *)(&Mem_RAM[(addr ^ 2) & RAM_MASK]);

	addr = addr & RAM_MASK;
	return (u16)(Mem_RAM[(addr + 0) ^ 3] << 8) |
		   (u16)(Mem_RAM[(addr + 1) ^ 3]);
}

__inline u32 gxMemoryRead32(u32 addr)
{
	addr &= RAM_MASK;
	if(!(addr & 3))
		return *(u32 *)(&Mem_RAM[addr]);

	return ((u32)Mem_RAM[(addr + 0) ^ 3] << 24) |
		   ((u32)Mem_RAM[(addr + 1) ^ 3] << 16) |
		   ((u32)Mem_RAM[(addr + 2) ^ 3] << 8) |
		   ((u32)Mem_RAM[(addr + 3) ^ 3]);
}

////////////////////////////////////////////////////////////////////////////////
// POSITION DECODING

static void gx_send_pos_unk(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
#ifdef DEBUG_GX
	printf(gx_vertex::outbuff);
#else
	printf("Error unsupported position format!");
#endif
}

// U8 XY

// correct
static void gx_send_pos_du8xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u16 data = _gxlist->get16_noswap(_vtx->position);
	v[0] = (f32)(u8)(data & 0xFF);
	v[1] = (f32)(u8)(data >> 8);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// correct
static void gx_send_pos_iu8xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u16 data = (gxMemoryRead16(CP_DATA_POS_ADDR(_vtx->index)));
	v[0] = (f32)(u8)(data >> 8);
	v[1] = (f32)(u8)(data & 0xFF);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// S8 XY

// correct
static void gx_send_pos_ds8xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u16	data = _gxlist->get16_noswap(_vtx->position);
	v[0] = (f32)(s8)(data & 0xFF);
	v[1] = (f32)(s8)(data >> 8);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// correct
static void gx_send_pos_is8xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u16 data = (gxMemoryRead16(CP_DATA_POS_ADDR(_vtx->index)));
	v[0] = (f32)(s8)(data >> 8);
	v[1] = (f32)(s8)(data & 0xFF);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// U16 XY

// correct
static void gx_send_pos_du16xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u32	data = _gxlist->get32(_vtx->position);
	v[0] = (f32)(u16)(data >> 16);
	v[1] = (f32)(u16)(data & 0xFFFF);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// correct
static void gx_send_pos_iu16xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u32 data = (gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index)));
	v[0] = (f32)(u16)(data >> 16);
	v[1] = (f32)(u16)(data & 0xFFFF);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// S16 XY

// correct
static void gx_send_pos_ds16xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u32 data = _gxlist->get32(_vtx->position);
	v[0] = (f32)(s16)(data >> 16);
	v[1] = (f32)(s16)(data & 0xFFFF);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// correct
static void gx_send_pos_is16xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[2];
	u32 data = (gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index)));
	v[0] = (f32)(s16)(data >> 16);
	v[1] = (f32)(s16)(data & 0xFFFF);
	GX_SEND_POSITION_XY(v[0], v[1]);
}

// F32 XY

// correct
static void gx_send_pos_df32xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[2];
	v[0]._u32 = _gxlist->get32(_vtx->position + 0);
	v[1]._u32 = _gxlist->get32(_vtx->position + 4);
	GX_SEND_POSITION_XY(v[0]._f32, v[1]._f32);
}

// correct
static void gx_send_pos_if32xy(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[2];
	v[0]._u32 = (gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index) + 0));
	v[1]._u32 = (gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index) + 4));
	GX_SEND_POSITION_XY(v[0]._f32, v[1]._f32);
}

// U8 XYZ

// correct
static void gx_send_pos_du8xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = _gxlist->get32_noswap(_vtx->position);
	v[0] = (f32)(u8)(data & 0xFF);
	v[1] = (f32)(u8)((data >> 8) & 0xFF);
	v[2] = (f32)(u8)((data >> 16) & 0xFF);
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// correct
static void gx_send_pos_iu8xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index));
	v[0] = (f32)(u8)((data >> 24) & 0xFF);
	v[1] = (f32)(u8)((data >> 16) & 0xFF);
	v[2] = (f32)(u8)((data >> 8) & 0xFF);
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// S8 XYZ

// correct
static void gx_send_pos_ds8xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = _gxlist->get32_noswap(_vtx->position);
	v[0] = (f32)(s8)(data & 0xFF);
	v[1] = (f32)(s8)((data >> 8) & 0xFF);
	v[2] = (f32)(s8)((data >> 16) & 0xFF);
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// correct
static void gx_send_pos_is8xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index));
	v[0] = (f32)(s8)((data >> 24) & 0xFF);
	v[1] = (f32)(s8)((data >> 16) & 0xFF);
	v[2] = (f32)(s8)((data >> 8) & 0xFF);
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// U16 XYZ

// correct
static void gx_send_pos_du16xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = _gxlist->get32(_vtx->position);
	v[0] = (f32)(u16)(data >> 16);
	v[1] = (f32)(u16)(data & 0xFFFF);
	v[2] = (f32)(u16)(_gxlist->get16(_vtx->position + 4));
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// correct
static void gx_send_pos_iu16xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index));
	v[0] = (f32)(u16)(data >> 16);
	v[1] = (f32)(u16)(data & 0xFFFF);
	v[2] = (f32)(u16)gxMemoryRead16(CP_DATA_POS_ADDR(_vtx->index) + 4);
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// S16 XYZ

// correct
static void gx_send_pos_ds16xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = _gxlist->get32(_vtx->position);
	v[0] = (f32)(s16)(data >> 16);
	v[1] = (f32)(s16)(data & 0xFFFF);
	v[2] = (f32)(s16)(_gxlist->get16(_vtx->position + 4));
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// correct
static void gx_send_pos_is16xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index));
	v[0] = (f32)(s16)(data >> 16);
	v[1] = (f32)(s16)(data & 0xFFFF);
	v[2] = (f32)(s16)gxMemoryRead16(CP_DATA_POS_ADDR(_vtx->index) + 4);
	GX_SEND_POSITION_XYZ(v[0], v[1], v[2]);
}

// F32 XYZ

// correct
static void gx_send_pos_df32xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];
	v[0]._u32 = _gxlist->get32(_vtx->position + 0);
	v[1]._u32 = _gxlist->get32(_vtx->position + 4);
	v[2]._u32 = _gxlist->get32(_vtx->position + 8);
	GX_SEND_POSITION_XYZ(v[0]._f32, v[1]._f32, v[2]._f32);
}

// correct
static void gx_send_pos_if32xyz(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];
	v[0]._u32 = (gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index) + 0));
	v[1]._u32 = (gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index) + 4));
	v[2]._u32 = (gxMemoryRead32(CP_DATA_POS_ADDR(_vtx->index) + 8));
	GX_SEND_POSITION_XYZ(v[0]._f32, v[1]._f32, v[2]._f32);
}

// POSITION TABLE

gxtable	gx_send_pos[0x40] = {
	gx_send_pos_unk,	gx_send_pos_du8xy,	gx_send_pos_iu8xy,	gx_send_pos_iu8xy,		 
	gx_send_pos_unk,	gx_send_pos_ds8xy,	gx_send_pos_is8xy,	gx_send_pos_is8xy,	
	gx_send_pos_unk,	gx_send_pos_du16xy,	gx_send_pos_iu16xy,	gx_send_pos_iu16xy,		 
	gx_send_pos_unk,	gx_send_pos_ds16xy,	gx_send_pos_is16xy,	gx_send_pos_is16xy, 
	gx_send_pos_unk,	gx_send_pos_df32xy,	gx_send_pos_if32xy,	gx_send_pos_if32xy,		 
	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,	
	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,		 
	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,		 
	gx_send_pos_unk,	gx_send_pos_du8xyz,	gx_send_pos_iu8xyz,	gx_send_pos_iu8xyz,		 
	gx_send_pos_unk,	gx_send_pos_ds8xyz,	gx_send_pos_is8xyz,	gx_send_pos_is8xyz,
	gx_send_pos_unk,	gx_send_pos_du16xyz,gx_send_pos_iu16xyz,gx_send_pos_iu16xyz,		 
	gx_send_pos_unk,	gx_send_pos_ds16xyz,gx_send_pos_is16xyz,gx_send_pos_is16xyz,	 
	gx_send_pos_unk,	gx_send_pos_df32xyz,gx_send_pos_if32xyz,gx_send_pos_if32xyz,		 
	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,		
	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,		 
	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk,	gx_send_pos_unk	
	
};

////////////////////////////////////////////////////////////////////////////////
// COLOR DIFFUSE DECODING

static void gx_send_col_unk(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
#ifdef DEBUG_GX
	printf(gx_vertex::outbuff2);
#else
	printf("Error unsupported color format!!!");
#endif
}

// color diffuse direct

// correct
static void gx_send_col_drgb565(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u16 data = (_gxlist->get16(_vtx->position));
	cv->color[0] = 4 * ((data >> 11) & 0x1f);
	cv->color[1] = 2 * ((data >> 5) & 0x3f);
	cv->color[2] = 4 * (data & 0x1f);
}

// correct
static void gx_send_col_drgb8(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u32 data = (_gxlist->get32(_vtx->position));
	cv->color[0] = (data >> 25) & 0x7f;
	cv->color[1] = (data >> 17) &0x7f;
	cv->color[2] = (data >> 9) & 0x7f;
	cv->color[3] = 0x7f;
}

// correct
static void gx_send_col_drgba4(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u16 data = _gxlist->get16_noswap(_vtx->position);
	cv->color[0] = ((data >> 4)&0xf) * 8;
	cv->color[1] = ((data >> 0)&0xf) * 8;	
	cv->color[2] = ((data >> 12)&0xf) * 8;
	cv->color[3] = ((data >> 8)&0xf) * 8;
}

// correct
static void gx_send_col_drgba6(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u32 data = (_gxlist->get32_noswap(_vtx->position));
	cv->color[0] = (data & 0x3f) << 1;
	cv->color[1] = ((data >> 6) & 0x3f) << 1;
	cv->color[2] = ((data >> 12) & 0x3f) << 1;
	cv->color[3] = ((data >> 18) & 0x3f) << 1;
}

// correct
static void gx_send_col_drgba8(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u32 data = _gxlist->get32_noswap(_vtx->position);
	*(u32 *)cv->color = (((data & 0xFEFEFEFE) >> 1));
}

// color diffuse 16bit index

// correct
static void gx_send_col_irgb565(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u16 data = gxMemoryRead16(CP_DATA_COL0_ADDR(_vtx->index));
	cv->color[0] = 4 * ((data >> 11) & 0x1f);
	cv->color[1] = 2 * ((data >> 5) & 0x3f);
	cv->color[2] = 4 * (data & 0x1f);
}

// correct
static void gx_send_col_irgb8(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u32 data = gxMemoryRead32(CP_DATA_COL0_ADDR(_vtx->index));
	cv->color[0] = (data >> 25) & 0x7f;
	cv->color[1] = (data >> 17) &0x7f;
	cv->color[2] = (data >> 9) & 0x7f;
	cv->color[3] = 0x7f;
}

// correct
static void gx_send_col_irgba4(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u16 data = gxMemoryRead16(CP_DATA_COL0_ADDR(_vtx->index));
	cv->color[0] = ((data >> 12)&0xf) * 8;
	cv->color[1] = ((data >> 8)&0xf) * 8;
	cv->color[2] = ((data >> 4)&0xf) * 8;
	cv->color[3] = ((data >> 0)&0xf) * 8;	
}

// correct
static void gx_send_col_irgba6(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u32 data = BSWAP32(gxMemoryRead32(CP_DATA_COL0_ADDR(_vtx->index)));
	cv->color[0] = (data & 0x3f) << 1;
	cv->color[1] = ((data >> 6) & 0x3f) << 1;
	cv->color[2] = ((data >> 12) & 0x3f) << 1;
	cv->color[3] = ((data >> 18) & 0x3f) << 1;
}

// correct
static void gx_send_col_irgba8(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	u32 data = BSWAP32(gxMemoryRead32(CP_DATA_COL0_ADDR(_vtx->index)));
	*(u32 *)cv->color = (((data & 0xFEFEFEFE) >> 1));
}

// color diffuse table

gxtable	gx_send_col[0x40] = {
	gx_send_col_unk,		gx_send_col_drgb565,	gx_send_col_irgb565,	gx_send_col_irgb565, 
	gx_send_col_unk,		gx_send_col_drgb8,		gx_send_col_irgb8,		gx_send_col_irgb8, // 7
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_irgb8,		gx_send_col_irgb8, 
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, // 15
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, 
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_irgba8,		gx_send_col_irgba8, // 23
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, 
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, // 31
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, 
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, // 39
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, 
	gx_send_col_unk,		gx_send_col_drgba4,		gx_send_col_irgba4,	gx_send_col_irgba4, // 47
	gx_send_col_unk,		gx_send_col_drgba6,		gx_send_col_irgba6,	gx_send_col_irgba6, 
	gx_send_col_unk,		gx_send_col_drgba8,		gx_send_col_irgba8,	gx_send_col_irgba8, // 55
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk, 
	gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk,		gx_send_col_unk
};

////////////////////////////////////////////////////////////////////////////////
// NORMAL DECODING

static void gx_send_nrm_unk(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
#ifdef DEBUG_GX
	printf(gx_vertex::outbuff4);
#else
	printf("Error unsupported normal format!!!");
#endif
}

static void gx_send_nrm_ign(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	// ignore due to hardware compatability
	// ...?? how the fuck do I do 9 normals ?
}

// normal direct

static void gx_send_nrm_du8three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];
	u32 data = _gxlist->get32_noswap(_vtx->position);

	// convert data to float
	v[0]._u32 = (data & 0xFF);
	v[1]._u32 = ((data >> 8) & 0xFF);
	v[2]._u32 = ((data >> 16) & 0xFF);

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_ds8three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];
	u32 data = _gxlist->get32_noswap(_vtx->position);

	// convert data to float
	v[0]._u32 = (data & 0xFF);
	v[1]._u32 = ((data >> 8) & 0xFF);
	v[2]._u32 = ((data >> 16) & 0xFF);

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_ds16three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];
	u32 data = _gxlist->get32(_vtx->position);

	// convert data to float
	v[0]._u32 = (data >> 16);
	v[1]._u32 = (data & 0xFFFF);
	v[2]._u32 = (_gxlist->get16(_vtx->position + 4));

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_df32three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];

	// convert data to float
	v[0]._u32 = _gxlist->get32(_vtx->position);
	v[1]._u32 = _gxlist->get32(_vtx->position + 4);
	v[2]._u32 = _gxlist->get32(_vtx->position + 8);

	// send
	glNormal3fv((GLfloat *)v);
}

// normal 8bit index

static void gx_send_nrm_i8u8three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index & 0xff));

	// convert data to float -> dq that shit
	v[0] = (f32)(u8)(data & 0xFF);
	v[1] = (f32)(u8)((data >> 8) & 0xFF);
	v[2] = (f32)(u8)((data >> 16) & 0xFF);

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_i8s8three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index & 0xff));

	// convert data to float -> dq that shit
	v[0] = (f32)(s8)(data & 0xFF);
	v[1] = (f32)(s8)((data >> 8) & 0xFF);
	v[2] = (f32)(s8)((data >> 16) & 0xFF);

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_i8s16three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index & 0xff));

	// convert data to float -> dq that shit
	v[0] = (f32)(s16)(data & 0xFFFF);
	v[1] = (f32)(s16)(data >> 16);
	v[2] = (f32)(s16)gxMemoryRead16(CP_DATA_NRM_ADDR(_vtx->index & 0xff) + 4);

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_i8f32three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];

	// convert data to float
	v[0]._u32 = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index & 0xff));
	v[1]._u32 = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index & 0xff) + 4);
	v[2]._u32 = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index & 0xff) + 8);

	// send
	glNormal3fv((GLfloat *)v);
}

// normal 16bit index

static void gx_send_nrm_i16s8three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index));

	// convert data to float -> dq that shit
	v[0] = (f32)(s8)(data & 0xFF);
	v[1] = (f32)(s8)((data >> 8) & 0xFF);
	v[2] = (f32)(s8)((data >> 16) & 0xFF);

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_i16s16three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	f32 v[3];
	u32 data = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index));

	// convert data to float -> dq that shit
	v[0] = (f32)(s16)(data & 0xFFFF);
	v[1] = (f32)(s16)(data >> 16);
	v[2] = (f32)(s16)(gxMemoryRead16(CP_DATA_NRM_ADDR(_vtx->index) + 4));

	// send
	glNormal3fv((GLfloat *)v);
}

static void gx_send_nrm_i16f32three(cgxlist* _gxlist, gx_vertex_data *_vtx)
{
	_t32 v[3];

	// convert data to float
	v[0]._u32 = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index));
	v[1]._u32 = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index) + 4);
	v[2]._u32 = gxMemoryRead32(CP_DATA_NRM_ADDR(_vtx->index) + 8);

	// send
	glNormal3fv((GLfloat *)v);
}

// normal table

gxtable	gx_send_nrm[0x40] = {
	gx_send_nrm_unk,		gx_send_nrm_du8three,	gx_send_nrm_i8u8three,	gx_send_nrm_unk, 
	gx_send_nrm_unk,		gx_send_nrm_ds8three,	gx_send_nrm_i8s8three,	gx_send_nrm_i16s8three, // 7
	gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk, 
	gx_send_nrm_unk,		gx_send_nrm_ds16three,	gx_send_nrm_i8s16three,	gx_send_nrm_i16s16three, // 15
	gx_send_nrm_unk,		gx_send_nrm_df32three,	gx_send_nrm_i8f32three,	gx_send_nrm_i16f32three, 
	gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk, // 23
	gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk, 
	gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk,		gx_send_nrm_unk, // 31
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign, 
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign, // 39
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign, 
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign, // 47
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign, 
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign, // 55
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign, 
	gx_send_nrm_unk,		gx_send_nrm_ign,		gx_send_nrm_ign,		gx_send_nrm_ign
};

////////////////////////////////////////////////////////////////////////////////
// TEXTURE COORD DECODING

static void gx_send_tex_unk(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
#ifdef DEBUG_GX
	printf(gx_vertex::outbuff3);
#else
	printf("Error unsupported texture coord format!!!");
#endif
}

// U8 S

// correct
static void gx_send_tex_du8s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(u8)(_gxlist->get8(_vtx->position));
	GX_SEND_TEXCOORD_S(i, t);
}

// correct
static void gx_send_tex_iu8s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(u8)(Mem_RAM[((CP_DATA_TEX_ADDR(_vtx->index, _vtx->num)) & RAM_MASK) ^ 3]);
	GX_SEND_TEXCOORD_S(i, t);
}

// S8 S

// correct
static void gx_send_tex_ds8s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(s8)(_gxlist->get8(_vtx->position));
	GX_SEND_TEXCOORD_S(i, t);
}

// correct
static void gx_send_tex_is8s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(s8)(Mem_RAM[((CP_DATA_TEX_ADDR(_vtx->index, _vtx->num)) & RAM_MASK) ^ 3]);
	GX_SEND_TEXCOORD_S(i, t);
}

// U16 S

// correct
static void gx_send_tex_du16s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(u16)(_gxlist->get16(_vtx->position));
	GX_SEND_TEXCOORD_S(i, t);
}

// correct
static void gx_send_tex_iu16s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(u16)gxMemoryRead16(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num));
	GX_SEND_TEXCOORD_S(i, t);
}

// S16 S

// correct
static void gx_send_tex_ds16s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(s16)(_gxlist->get16(_vtx->position));
	GX_SEND_TEXCOORD_S(i, t);
}

// correct
static void gx_send_tex_is16s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t;
	t = (f32)(s16)gxMemoryRead16(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num));
	GX_SEND_TEXCOORD_S(i, t);
}

// F32 S

// correct
static void gx_send_tex_df32s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	_t32 t;
	t._u32 = _gxlist->get32(_vtx->position + 0);
	GX_SEND_TEXCOORD_S(i, t._f32);
}

// correct
static void gx_send_tex_if32s(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	_t32 t;
	t._u32 = gxMemoryRead32(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num) + 0);
	GX_SEND_TEXCOORD_S(i, t._f32);
}

// U8 ST

// correct
static void gx_send_tex_du8st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u16 data = _gxlist->get16_noswap(_vtx->position);
	t[0] = (f32)(u8)(data & 0xFF);
	t[1] = (f32)(u8)(data >> 8);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// correct
static void gx_send_tex_iu8st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u16 data = gxMemoryRead16(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num));
	t[0] = (f32)(u8)(data >> 8);
	t[1] = (f32)(u8)(data & 0xFF);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// S8 ST

// correct
static void gx_send_tex_ds8st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u16 data = _gxlist->get16_noswap(_vtx->position);
	t[0] = (f32)(s8)(data & 0xFF);
	t[1] = (f32)(s8)(data >> 8);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// correct
static void gx_send_tex_is8st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u16 data = gxMemoryRead16(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num));
	t[0] = (f32)(s8)(data >> 8);
	t[1] = (f32)(s8)(data & 0xFF);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// U16 ST

// correct
static void gx_send_tex_du16st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u32 data = _gxlist->get32(_vtx->position);
	t[0] = (f32)(u16)(data >> 16);
	t[1] = (f32)(u16)(data & 0xFFFF);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// correct
static void gx_send_tex_iu16st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u32 data = gxMemoryRead32(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num));
	t[0] = (f32)(u16)(data >> 16);
	t[1] = (f32)(u16)(data & 0xFFFF);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// S16 ST

// correct
static void gx_send_tex_ds16st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u32 data = _gxlist->get32(_vtx->position);
	t[0] = (f32)(s16)(data >> 16);
	t[1] = (f32)(s16)(data & 0xFFFF);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// correct
static void gx_send_tex_is16st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	f32 t[2];
	u32 data = gxMemoryRead32(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num));
	t[0] = (f32)(s16)(data >> 16);
	t[1] = (f32)(s16)(data & 0xFFFF);
	GX_SEND_TEXCOORD_ST(i, t[0], t[1]);
}

// F32 ST

// correct
static void gx_send_tex_df32st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	_t32 t[2];
	t[0]._u32 = _gxlist->get32(_vtx->position + 0);
	t[1]._u32 = _gxlist->get32(_vtx->position + 4);
	GX_SEND_TEXCOORD_ST(i, t[0]._f32, t[1]._f32);
}

// correct
static void gx_send_tex_if32st(cgxlist* _gxlist, gx_vertex_data *_vtx, int i)
{
	_t32 t[2];
	t[0]._u32 = gxMemoryRead32(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num) + 0);
	t[1]._u32 = gxMemoryRead32(CP_DATA_TEX_ADDR(_vtx->index, _vtx->num) + 4);
	GX_SEND_TEXCOORD_ST(i, t[0]._f32, t[1]._f32);
}

// texture coord table

gxtextable	gx_send_tex[0x40] = {
	gx_send_tex_unk,		gx_send_tex_du8s,		gx_send_tex_iu8s,		gx_send_tex_iu8s, 
	gx_send_tex_unk,		gx_send_tex_ds8s,		gx_send_tex_is8s,		gx_send_tex_is8s,
	gx_send_tex_unk,		gx_send_tex_du16s,		gx_send_tex_iu16s,		gx_send_tex_iu16s, 
	gx_send_tex_unk,		gx_send_tex_ds16s,		gx_send_tex_is16s,		gx_send_tex_is16s, 
	gx_send_tex_unk,		gx_send_tex_df32s,		gx_send_tex_if32s,		gx_send_tex_if32s, 
	gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk, 
	gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk, 
	gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk, 
	gx_send_tex_unk,		gx_send_tex_du8st,		gx_send_tex_iu8st,		gx_send_tex_iu8st, 
	gx_send_tex_unk,		gx_send_tex_ds8st,		gx_send_tex_is8st,		gx_send_tex_is8st, 
	gx_send_tex_unk,		gx_send_tex_du16st,		gx_send_tex_iu16st,		gx_send_tex_iu16st, 
	gx_send_tex_unk,		gx_send_tex_ds16st,		gx_send_tex_is16st,		gx_send_tex_is16st, 
	gx_send_tex_unk,		gx_send_tex_df32st,		gx_send_tex_if32st,		gx_send_tex_if32st, 
	gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk, 
	gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk, 
	gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk,		gx_send_tex_unk
};

////////////////////////////////////////////////////////////////////////////////
// PRIMITIVE DECODING

inline void gx_vertex::get_vertex(u8 _vat)
{
	// position
	pos.vcd = VCD_POS;
	if(pos.vcd)
	{
		pos.cnt = cv->is3d = VAT_POSCNT;
		pos.fmt = VAT_POSFMT;
		pos.dqf = 1.0f / (1 << VAT_POSSHFT);
		pos.vtx_format = gx_poscoord_size[VTX_FORMAT(pos)];
		pos.vtx_format_vcd = (void *)gx_send_pos[VTX_FORMAT_VCD(pos)];
	}

	// normal
	nrm.vcd = VCD_NRM;
	if(nrm.vcd)
	{
		nrm.cnt = VAT_NRMCNT;
		nrm.fmt = VAT_NRMFMT;
		nrm.vtx_format = gx_normal_size[VTX_FORMAT(nrm)];
		nrm.vtx_format_vcd = (void *)gx_send_nrm[VTX_FORMAT_VCD(nrm)];
	}

	// color 0
	col0.vcd = VCD_COL0;
	if(col0.vcd)
	{
		col0.cnt = VAT_COL0CNT;
		col0.fmt = VAT_COL0FMT;
		col0.vtx_format = gx_color_size[VTX_FORMAT(col0)];
		col0.vtx_format_vcd = (void *)gx_send_col[VTX_FORMAT_VCD(col0)];
	}

	// color 1
	col1.vcd = VCD_COL1;
	if(col1.vcd)
	{
		col1.cnt = VAT_COL1CNT;
		col1.fmt = VAT_COL1FMT;
		col1.vtx_format = gx_color_size[VTX_FORMAT(col1)];
		col1.vtx_format_vcd = (void *)gx_send_col[VTX_FORMAT_VCD(col1)];
	}

	// texture 0
	tex[0].vcd = VCD_TEX(0);
	if(tex[0].vcd)
	{
		tex[0].dqf = 1.0f / (1 << VAT_TEX0SHFT);
		tex[0].cnt = VAT_TEX0CNT;
		tex[0].fmt = VAT_TEX0FMT;
		tex[0].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[0])];
		tex[0].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[0])];
	}

	// texture 1
	tex[1].vcd = VCD_TEX(1);
	if(tex[1].vcd)
	{
		tex[1].dqf = 1.0f / (1 << VAT_TEX1SHFT);
		tex[1].cnt = VAT_TEX1CNT;
		tex[1].fmt = VAT_TEX1FMT;
		tex[1].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[1])];
		tex[1].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[1])];
	}

	// texture 2
	tex[2].vcd = VCD_TEX(2);
	if(tex[2].vcd)
	{
		tex[2].dqf = 1.0f / (1 << VAT_TEX2SHFT);
		tex[2].cnt = VAT_TEX2CNT;
		tex[2].fmt = VAT_TEX2FMT;
		tex[2].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[2])];
		tex[2].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[2])];
	}

	// texture 3
	tex[3].vcd = VCD_TEX(3);
	if(tex[3].vcd)
	{
		tex[3].dqf = 1.0f / (1 << VAT_TEX3SHFT);
		tex[3].cnt = VAT_TEX3CNT;
		tex[3].fmt = VAT_TEX3FMT;
		tex[3].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[3])];
		tex[3].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[3])];
	}

	// texture 4
	tex[4].vcd = VCD_TEX(4);
	if(tex[4].vcd)
	{
		tex[4].dqf = 1.0f / (1 << VAT_TEX4SHFT);
		tex[4].cnt = VAT_TEX4CNT;
		tex[4].fmt = VAT_TEX4FMT;
		tex[4].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[4])];
		tex[4].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[4])];
	}

	// texture 5
	tex[5].vcd = VCD_TEX(5);
	if(tex[5].vcd)
	{
		tex[5].dqf = 1.0f / (1 << VAT_TEX5SHFT);
		tex[5].cnt = VAT_TEX5CNT;
		tex[5].fmt = VAT_TEX5FMT;
		tex[5].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[5])];
		tex[5].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[5])];
	}

	// texture 6
	tex[6].vcd = VCD_TEX(6);
	if(tex[6].vcd)
	{
		tex[6].dqf = 1.0f / (1 << VAT_TEX6SHFT);
		tex[6].cnt = VAT_TEX6CNT;
		tex[6].fmt = VAT_TEX6FMT;
		tex[6].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[6])];
		tex[6].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[6])];
	}

	// texture 7
	tex[7].vcd = VCD_TEX(7);
	if(tex[7].vcd)
	{
		tex[7].dqf = 1.0f / (1 << VAT_TEX7SHFT);
		tex[7].cnt = VAT_TEX7CNT;
		tex[7].fmt = VAT_TEX7FMT;
		tex[7].vtx_format = gx_texcoord_size[VTX_FORMAT(tex[8])];
		tex[7].vtx_format_vcd = (void *)gx_send_tex[VTX_FORMAT_VCD(tex[7])];
	}
}

// retrieve the size of the next vertex in the fifo
int gx_vertex::get_size(u8 _vat)
{
	u16 size = 0;
	u8 count = 0;
	int i = 0;
	
	if(VCD_PMIDX) size+=1;

	for(; i < 8; i++)
	{
//		if(VCD_TMIDX(i))
//			size+=1;
		//already checking the last bit, dont compare, just add
		size += VCD_TMIDX(i);
	}

	switch(VCD_POS)
	{	case 1: // direct
			count = VAT_POSCNT + 2;
			switch(VAT_POSFMT)
			{
			case 0: // ubyte
			case 1: size += (1 * count); break; // byte
			case 2: // uhalf
			case 3: size += (2 * count); break; // half
			case 4: size += (4 * count); break; // float
			}
			break;
		case 2: size+=1; break; // index 8
		case 3: size+=2; break; // index 16
	}

	switch(VCD_NRM)
	{	case 1: size+=gx_normal_size[((VAT_NRMCNT << 3) | VAT_NRMFMT)]; break; // direct
		case 2: size+=1; break; // index 8
		case 3: size+=2; break; // index 16
	}

	switch(VCD_COL0)
	{	case 1: // direct
			switch(VAT_COL0FMT)
			{
			case 0: size += 2; break; // rgb565
			case 1: size += 3; break; // rgb888
			case 2: size += 4; break; // rgb888x
			case 3: size += 2; break; // rgba4444
			case 4: size += 3; break; // rgba6666
			case 5: size += 4; break; // rgba8888
			}
			break;
		case 2: size+=1; break; // index 8
		case 3: size+=2; break; // index 16
	}

	switch(VCD_COL1)
	{	case 1: // direct
			switch(VAT_COL1FMT)
			{
			case 0: size += 2; break; // rgb565
			case 1: size += 3; break; // rgb888
			case 2: size += 4; break; // rgb888x
			case 3: size += 2; break; // rgba4444
			case 4: size += 3; break; // rgba6666
			case 5: size += 4; break; // rgba8888
			}
			break;
		case 2: size+=1; break; // index 8
		case 3: size+=2; break; // index 16
	}

	TEXSIZELOOP(0, VAT_TEX0CNT, VAT_TEX0FMT);
	TEXSIZELOOP(1, VAT_TEX1CNT, VAT_TEX1FMT);
	TEXSIZELOOP(2, VAT_TEX2CNT, VAT_TEX2FMT);
	TEXSIZELOOP(3, VAT_TEX3CNT, VAT_TEX3FMT);
	TEXSIZELOOP(4, VAT_TEX4CNT, VAT_TEX4FMT);
	TEXSIZELOOP(5, VAT_TEX5CNT, VAT_TEX5FMT);
	TEXSIZELOOP(6, VAT_TEX6CNT, VAT_TEX6FMT);
	TEXSIZELOOP(7, VAT_TEX7CNT, VAT_TEX7FMT);

	return size;
}

// send vertex to the renderer
void gx_vertex::send_vertex(cgxlist* _gxlist, u8 _vat)
{
	u32				offset = 0;

	// DECODE POSITION FORMAT

	// get pos index (if used)
	switch(pos.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		pos.position = offset; // store vertex data position
		offset+=pos.vtx_format; // offset over data..
		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 
		break;
	case 2: // 8bit index
		pos.index = _gxlist->get8(offset++);
		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 
		break;
	case 3: // 16bit index
		pos.index = _gxlist->get16(offset);
		offset+=2;
		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 
		break;
	}

	// DECODE NORMAL FORMAT (FAKE)

	switch(nrm.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		nrm.position = offset; // store vertex data position
		offset+=nrm.vtx_format; // offset over data
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	case 2: // 8bit index
		nrm.index = _gxlist->get8(offset++);
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	case 3: // 16bit index
		nrm.index = _gxlist->get16(offset);
		offset+=2;
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	}

	// DECODE DIFFUSE COLOR FORMAT

	// get color index (if used)
	switch(col0.vcd)
	{
	case 0: // no data present
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		break;
	case 1: // direct
		col0.position = offset; // store vertex data position
		offset+=col0.vtx_format; // offset over data
		((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
		glColor4bv((GLbyte *)cv->color);
		break;
	case 2: // 8bit index
 		col0.index = _gxlist->get8(offset++);
		((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
		glColor4bv((GLbyte *)cv->color);
		break;
	case 3: // 16bit index 
		col0.index = _gxlist->get16(offset);
		offset+=2;
		((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
		glColor4bv((GLbyte *)cv->color);
		break;
	}

	// DECODE SPECULAR COLOR FORMAT (FAKE)

	switch(col1.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		col1.position = offset; // store vertex data position
		offset+=col1.vtx_format; // offset over data
		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		glSecondaryColor3bv((GLbyte *)cv->color);
		break;
	case 2: // 8bit index
		col1.index = _gxlist->get8(offset++);
		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		glSecondaryColor3bv((GLbyte *)cv->color);
		break;
	case 3: // 16bit index
		col1.index = _gxlist->get16(offset);
		offset+=2;
		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		glSecondaryColor3bv((GLbyte *)cv->color);
		break;
	}

	// DECODE TEXTURE COORDINATE FORMAT
	TEXLOOP(0);
	TEXLOOP(1);
	TEXLOOP(2);
	TEXLOOP(3);
	TEXLOOP(4);
	TEXLOOP(5);
	TEXLOOP(6);
	TEXLOOP(7);

	// send data to the renderer
	if(pos.vcd)
		glVertex3fv(cv->pos);

	//printf("POSITION: %f  %f  %f\n",cv->pos[0],cv->pos[1],cv->pos[2]);

	// pop all used data off the fifo..
	_gxlist->popoff(offset);
}

//send_vertex with only midx set
void gx_vertex::send_vertex_midx(cgxlist* _gxlist, u8 _vat)
{
	int				i;
	u32				offset = 0;
	u32				data;

	// DECODE MATRIX INDEX FORMAT

	// offset according to position matrix..
	memset(tm_index, 0, sizeof(tm_index));

	// offset according to texture matrixes..
	data = CP_VCD_LO(0);
	for(i = 0; i < 8; i++)
	{
		data = data >> 1;
		if(data & 1)
			tm_index[i] = _gxlist->pop8();
	}

	// DECODE POSITION FORMAT

	// get pos index (if used)
	switch(pos.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		pos.position = offset; // store vertex data position
		offset+=pos.vtx_format; // offset over data..
#ifdef DEBUG_GX
		sprintf(outbuff, "pos vcd: direct, format type %02x", VTX_FORMAT_VCD(pos));
#endif

		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 
		break;
	case 2: // 8bit index
		pos.index = _gxlist->get8(offset);
		offset++;
#ifdef DEBUG_GX
		sprintf(outbuff, "pos vcd: 8bit indexed, format type %02x", VTX_FORMAT_VCD(pos));
#endif

		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 
		break;
	case 3: // 16bit index
		pos.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff, "pos vcd: 16bit indexed, format type %02x", VTX_FORMAT_VCD(pos));
#endif

		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 
		break;
	}

	// DECODE NORMAL FORMAT (FAKE)

	switch(nrm.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		nrm.position = offset; // store vertex data position
		offset+=nrm.vtx_format; // offset over data
#ifdef DEBUG_GX
		sprintf(outbuff4, "normal vcd: direct, cnt %02x fmt %02x, method %02x", nrm.cnt, nrm.fmt, VTX_FORMAT_VCD(nrm));
#endif
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	case 2: // 8bit index
		nrm.index = _gxlist->get8(offset);
		offset++;
#ifdef DEBUG_GX
		sprintf(outbuff4, "normal vcd: 8bit indexed, cnt %02x fmt %02x, method %02x", nrm.cnt, nrm.fmt, VTX_FORMAT_VCD(nrm));
#endif
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	case 3: // 16bit index
		nrm.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff4, "normal vcd: 16bit indexed, cnt %02x fmt %02x, method %02x", nrm.cnt, nrm.fmt, VTX_FORMAT_VCD(nrm));
#endif
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	}

	// DECODE DIFFUSE COLOR FORMAT

	// get color index (if used)
	switch(col0.vcd)
	{
	case 0: // no data present
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		break;
	case 1: // direct
		col0.position = offset; // store vertex data position
		offset+=col0.vtx_format; // offset over data
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: direct, cnt %02x fmt %02x, method %02x", col0.cnt, col0.fmt, VTX_FORMAT_VCD(col0));
#endif

		#ifdef FORCE_WHITE
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		#else
			((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
			glColor4bv((GLbyte *)cv->color);
		#endif

		break;
	case 2: // 8bit index
 		col0.index = _gxlist->get8(offset);
		offset++;
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 8bit indexed, cnt %02x fmt %02x, method %02x", col0.cnt, col0.fmt, VTX_FORMAT_VCD(col0));
#endif

		#ifdef FORCE_WHITE
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		#else
			((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
			glColor4bv((GLbyte *)cv->color);
		#endif

		break;
	case 3: // 16bit index 
		col0.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 16bit indexed, cnt %02x fmt %02x, method %02x", col0.cnt, col0.fmt, VTX_FORMAT_VCD(col0));
#endif

		#ifdef FORCE_WHITE
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		#else
			((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
			glColor4bv((GLbyte *)cv->color);
		#endif

		break;
	}

	// DECODE SPECULAR COLOR FORMAT (FAKE)

	switch(col1.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		col1.position = offset; // store vertex data position
		offset+=col1.vtx_format; // offset over data
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: direct, cnt %02x fmt %02x, method %02x", col1.cnt, col1.fmt, VTX_FORMAT_VCD(col1));
#endif

		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		glSecondaryColor3bv((GLbyte *)cv->color);

		break;
	case 2: // 8bit index
		col1.index = _gxlist->get8(offset);
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 8bit indexed, cnt %02x fmt %02x, method %02x", col1.cnt, col1.fmt, VTX_FORMAT_VCD(col1));
#endif

		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		//*(u32 *)(cv->col1) = *(u32 *)((GLbyte *)cv->color) & 0x00FFFFFF;
		glSecondaryColor3bv((GLbyte *)cv->color);

		break;
	case 3: // 16bit index
		col1.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 16bit indexed, cnt %02x fmt %02x, method %02x", col1.cnt, col1.fmt, VTX_FORMAT_VCD(col1));
#endif

		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		glSecondaryColor3bv((GLbyte *)cv->color);

		break;
	}

	// DECODE TEXTURE COORDINATE FORMAT
	TEXLOOP(0);
	TEXLOOP(1);
	TEXLOOP(2);
	TEXLOOP(3);
	TEXLOOP(4);
	TEXLOOP(5);
	TEXLOOP(6);
	TEXLOOP(7);

	// send data to the renderer
	if(pos.vcd)
		glVertex3fv(cv->pos);

	// pop all used data off the fifo..
	_gxlist->popoff(offset);
}

//send_vertex with midx and pmidx set
void gx_vertex::send_vertex_pmidx(cgxlist* _gxlist, u8 _vat)
{
	int				i;
	u32				offset = 0;
	u32				data;

	// DECODE MATRIX INDEX FORMAT

	// offset according to position matrix..
	pm_index = _gxlist->pop8();

	memset(tm_index, 0, sizeof(tm_index));

	// offset according to texture matrixes..
	data = CP_VCD_LO(0);
	for(i = 0; i < 8; i++)
	{
		data = data >> 1;
		if(data & 1)
			tm_index[i] = _gxlist->pop8();
	}

	// DECODE POSITION FORMAT

	// get pos index (if used)
	switch(pos.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		pos.position = offset; // store vertex data position
		offset+=pos.vtx_format; // offset over data..
#ifdef DEBUG_GX
		sprintf(outbuff, "pos vcd: direct, format type %02x", VTX_FORMAT_VCD(pos));
#endif

		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 

		if(cv->is3d)
		{
			gx_transform::gx_tf_pos_xyz(&cv->tpos[0], cv->pos);
		}else{
			gx_transform::gx_tf_pos_xy(&cv->tpos[0], cv->pos);
		}
		break;
	case 2: // 8bit index
		pos.index = _gxlist->get8(offset);
		offset++;
#ifdef DEBUG_GX
		sprintf(outbuff, "pos vcd: 8bit indexed, format type %02x", VTX_FORMAT_VCD(pos));
#endif

		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 

		if(cv->is3d)
		{
			gx_transform::gx_tf_pos_xyz(&cv->tpos[0], cv->pos);
		}else{
			gx_transform::gx_tf_pos_xy(&cv->tpos[0], cv->pos);
		}
		break;
	case 3: // 16bit index
		pos.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff, "pos vcd: 16bit indexed, format type %02x", VTX_FORMAT_VCD(pos));
#endif

		((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 

		if(cv->is3d)
		{
			gx_transform::gx_tf_pos_xyz(&cv->tpos[0], cv->pos);
		}else{
			gx_transform::gx_tf_pos_xy(&cv->tpos[0], cv->pos);
		}
		break;
	}

	// DECODE NORMAL FORMAT (FAKE)

	switch(nrm.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		nrm.position = offset; // store vertex data position
		offset+=nrm.vtx_format; // offset over data
#ifdef DEBUG_GX
		sprintf(outbuff4, "normal vcd: direct, cnt %02x fmt %02x, method %02x", nrm.cnt, nrm.fmt, VTX_FORMAT_VCD(nrm));
#endif
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	case 2: // 8bit index
		nrm.index = _gxlist->get8(offset);
		offset++;
#ifdef DEBUG_GX
		sprintf(outbuff4, "normal vcd: 8bit indexed, cnt %02x fmt %02x, method %02x", nrm.cnt, nrm.fmt, VTX_FORMAT_VCD(nrm));
#endif
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	case 3: // 16bit index
		nrm.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff4, "normal vcd: 16bit indexed, cnt %02x fmt %02x, method %02x", nrm.cnt, nrm.fmt, VTX_FORMAT_VCD(nrm));
#endif
		((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
		break;
	}

	// DECODE DIFFUSE COLOR FORMAT

	// get color index (if used)
	switch(col0.vcd)
	{
	case 0: // no data present
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		break;
	case 1: // direct
		col0.position = offset; // store vertex data position
		offset+=col0.vtx_format; // offset over data
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: direct, cnt %02x fmt %02x, method %02x", col0.cnt, col0.fmt, VTX_FORMAT_VCD(col0));
#endif

		#ifdef FORCE_WHITE
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		#else
			((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
			glColor4bv((GLbyte *)cv->color);
		#endif

		break;
	case 2: // 8bit index
 		col0.index = _gxlist->get8(offset);
		offset++;
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 8bit indexed, cnt %02x fmt %02x, method %02x", col0.cnt, col0.fmt, VTX_FORMAT_VCD(col0));
#endif

		#ifdef FORCE_WHITE
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		#else
			((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
			glColor4bv((GLbyte *)cv->color);
		#endif

		break;
	case 3: // 16bit index 
		col0.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 16bit indexed, cnt %02x fmt %02x, method %02x", col0.cnt, col0.fmt, VTX_FORMAT_VCD(col0));
#endif

		#ifdef FORCE_WHITE
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		#else
			((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
			glColor4bv((GLbyte *)cv->color);
		#endif

		break;
	}

	// DECODE SPECULAR COLOR FORMAT (FAKE)

	switch(col1.vcd)
	{
	case 0: // no data present
		break;
	case 1: // direct
		col1.position = offset; // store vertex data position
		offset+=col1.vtx_format; // offset over data
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: direct, cnt %02x fmt %02x, method %02x", col1.cnt, col1.fmt, VTX_FORMAT_VCD(col1));
#endif

		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		glSecondaryColor3bv((GLbyte *)cv->color);

		break;
	case 2: // 8bit index
		col1.index = _gxlist->get8(offset);
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 8bit indexed, cnt %02x fmt %02x, method %02x", col1.cnt, col1.fmt, VTX_FORMAT_VCD(col1));
#endif

		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		//*(u32 *)(cv->col1) = *(u32 *)((GLbyte *)cv->color) & 0x00FFFFFF;
		glSecondaryColor3bv((GLbyte *)cv->color);

		break;
	case 3: // 16bit index
		col1.index = _gxlist->get16(offset);
		offset+=2;
#ifdef DEBUG_GX
		sprintf(outbuff2, "color vcd: 16bit indexed, cnt %02x fmt %02x, method %02x", col1.cnt, col1.fmt, VTX_FORMAT_VCD(col1));
#endif

		((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
		glSecondaryColor3bv((GLbyte *)cv->color);

		break;
	}

	// DECODE TEXTURE COORDINATE FORMAT
	TEXLOOP(0);
	TEXLOOP(1);
	TEXLOOP(2);
	TEXLOOP(3);
	TEXLOOP(4);
	TEXLOOP(5);
	TEXLOOP(6);
	TEXLOOP(7);

	// send data to the renderer
	if(pos.vcd)
		glVertex3fv(cv->tpos);

	// pop all used data off the fifo..
	_gxlist->popoff(offset);
}

// begin primitive drawing
void gx_vertex::draw_primitive(cgxlist* _gxlist, GLenum _type, int _count, u8 _vat)
{
	void	*BuffPtr;
	u32		BuffSize;
	u32		BuffCRC;
	u32		CRCCounter;
	u32		CRCCounter2;

	vcd_midx_result = VCD_MIDX;
	vcd_pmidx_result = VCD_PMIDX;

	// set position transformation
	if(vcd_pmidx_result) 
		glLoadIdentity();
	else
		gx_transform::gx_tf_pos_hardware();

	// decode/bind textures
	if(gx_cfg.shader_mode)
	{
/*
		for(int k = 0; k < 4; k++)
		{
			glActiveTexture(GL_TEXTURE0 + k);
			gx_states::load_texture(0x94 + k);
			glActiveTexture(GL_TEXTURE4 + k);
			gx_states::load_texture(0xb4 + k);
		}
*/
		if(bp.tevorder[0 >> 1].get_enable(0))
		{
			glActiveTexture(GL_TEXTURE0 + 0);
			gx_states::load_texture(0x94 + 0);
		}
		if(bp.tevorder[1 >> 1].get_enable(1))
		{
			glActiveTexture(GL_TEXTURE0 + 1);
			gx_states::load_texture(0x94 + 1);
		}
		if(bp.tevorder[2 >> 1].get_enable(2))
		{
			glActiveTexture(GL_TEXTURE0 + 2);
			gx_states::load_texture(0x94 + 2);
		}
		if(bp.tevorder[3 >> 1].get_enable(3))
		{
			glActiveTexture(GL_TEXTURE0 + 3);
			gx_states::load_texture(0x94 + 3);
		}
		if(bp.tevorder[4 >> 1].get_enable(4))
		{
			glActiveTexture(GL_TEXTURE4 + 0);
			gx_states::load_texture(0xb4 + 0);
		}
		if(bp.tevorder[5 >> 1].get_enable(5))
		{
			glActiveTexture(GL_TEXTURE4 + 1);
			gx_states::load_texture(0xb4 + 1);
		}
		if(bp.tevorder[6 >> 1].get_enable(6))
		{
			glActiveTexture(GL_TEXTURE4 + 2);
			gx_states::load_texture(0xb4 + 2);
		}
		if(bp.tevorder[7 >> 1].get_enable(7))
		{
			glActiveTexture(GL_TEXTURE4 + 3);
			gx_states::load_texture(0xb4 + 3);
		}
	}else{
		glActiveTexture(GL_TEXTURE0);
		gx_states::load_texture(0x94);
	}

	if(gx_cfg.shader_mode && gx_tev::is_modified)
		gx_tev::generate_shader(); // generate tev shader

//a quick hack to see speed of using a vertex list. at the moment other bottlenecks makes this
//minimal so not worth it. it will need to be expanded to validate memory values too and not
//just the buffer
/*
	// do a quick crc of the buffer to see if it is already used
	// needs to be expanded to account for memory changes
	BuffPtr = _gxlist->getbufferptr();
	BuffSize = get_size(_vat) * _count;
	BuffCRC = GenerateCRC((u8 *)BuffPtr, BuffSize);

	//see if we can find it. Grab part of the crc for our lookup, if the crc
	//is not there and the hash in that position is used then
	//do a full scan for it
	CRCCounter = BuffCRC & (VertexListSize - 1);
	CRCCounter2 = 0;
	if(VertexList[CRCCounter].CRCHash != BuffCRC && VertexList[CRCCounter].CRCHash != 0)
	{
		for(CRCCounter2 = 1; VertexList[CRCCounter + CRCCounter2].CRCHash != 0 && ((CRCCounter + CRCCounter2) < VertexListSize) && (CRCCounter2 < 50); CRCCounter2++)
		{
			if(VertexList[CRCCounter + CRCCounter2].CRCHash == BuffCRC)
				break;
		}

		CRCCounter += CRCCounter2;
		if(CRCCounter2 < 50)
			CRCCounter2 = 0;
	}


	//if the buffer is full then use the crc to pick a spot
	if(CRCCounter2 >= 50 || ((CRCCounter + CRCCounter2) >= VertexListSize))
	{
		CRCCounter = BuffCRC & (VertexListSize - 1);
		VertexList[CRCCounter].CRCHash = 0;
	}

	//if we didn't find a crc then use a spot
	if(VertexList[CRCCounter].CRCHash == 0)
	{
		if(VertexList[CRCCounter].GLVertexList == 0)
			VertexList[CRCCounter].GLVertexList = (int)glGenLists(1);
		glNewList(VertexList[CRCCounter].GLVertexList, GL_COMPILE_AND_EXECUTE);
	}
	else
	{
		//we found it, don't decode everything, just render and pop
		_gxlist->popoff(BuffSize);
		glCallList(VertexList[CRCCounter].GLVertexList);
		return;
	}
*/
	// begin primitive type
	glBegin(_type);

	// decode vertices
	get_vertex(_vat);

	//to avoid extra checks, pick which send to call
	if(vcd_pmidx_result)
	{
		for(int i = 0; i < (_count >> 3); i++)
		{
			send_vertex_pmidx(_gxlist, _vat);
			send_vertex_pmidx(_gxlist, _vat);
			send_vertex_pmidx(_gxlist, _vat);
			send_vertex_pmidx(_gxlist, _vat);
			send_vertex_pmidx(_gxlist, _vat);
			send_vertex_pmidx(_gxlist, _vat);
			send_vertex_pmidx(_gxlist, _vat);
			send_vertex_pmidx(_gxlist, _vat);
		}

		switch(_count & 7)
		{
			case 7:
				send_vertex_pmidx(_gxlist, _vat);
			case 6:
				send_vertex_pmidx(_gxlist, _vat);
			case 5:
				send_vertex_pmidx(_gxlist, _vat);
			case 4:
				send_vertex_pmidx(_gxlist, _vat);
			case 3:
				send_vertex_pmidx(_gxlist, _vat);
			case 2:
				send_vertex_pmidx(_gxlist, _vat);
			case 1:
				send_vertex_pmidx(_gxlist, _vat);
		}
	}
	else if(vcd_midx_result)
	{
		for(int i = 0; i < (_count >> 3); i++)
		{
			send_vertex_midx(_gxlist, _vat);
			send_vertex_midx(_gxlist, _vat);
			send_vertex_midx(_gxlist, _vat);
			send_vertex_midx(_gxlist, _vat);
			send_vertex_midx(_gxlist, _vat);
			send_vertex_midx(_gxlist, _vat);
			send_vertex_midx(_gxlist, _vat);
			send_vertex_midx(_gxlist, _vat);
		}

		switch(_count & 7)
		{
			case 7:
				send_vertex_midx(_gxlist, _vat);
			case 6:
				send_vertex_midx(_gxlist, _vat);
			case 5:
				send_vertex_midx(_gxlist, _vat);
			case 4:
				send_vertex_midx(_gxlist, _vat);
			case 3:
				send_vertex_midx(_gxlist, _vat);
			case 2:
				send_vertex_midx(_gxlist, _vat);
			case 1:
				send_vertex_midx(_gxlist, _vat);
		}
	}
	else
	{
		for(int i = 0; i < (_count >> 3); i++)
		{
			send_vertex(_gxlist, _vat);
			send_vertex(_gxlist, _vat);
			send_vertex(_gxlist, _vat);
			send_vertex(_gxlist, _vat);
			send_vertex(_gxlist, _vat);
			send_vertex(_gxlist, _vat);
			send_vertex(_gxlist, _vat);
			send_vertex(_gxlist, _vat);
		}

		switch(_count & 7)
		{
			case 7:
				send_vertex(_gxlist, _vat);
			case 6:
				send_vertex(_gxlist, _vat);
			case 5:
				send_vertex(_gxlist, _vat);
			case 4:
				send_vertex(_gxlist, _vat);
			case 3:
				send_vertex(_gxlist, _vat);
			case 2:
				send_vertex(_gxlist, _vat);
			case 1:
				send_vertex(_gxlist, _vat);
		}
	}

	// end primitive type
	glEnd();
/*
	//if we didn't find a crc then use a spot
	if(VertexList[CRCCounter].CRCHash == 0)
	{
		glEndList();
		VertexList[CRCCounter].CRCHash = BuffCRC;
	}
*/
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

void gx_vertex::initialize(void)
{
	cv = &vtxarray[0];
	memset(&VertexList, 0, sizeof(VertexList));
}

void gx_vertex::destroy(void)
{
}

////////////////////////////////////////////////////////////////////////////////
// EOF
