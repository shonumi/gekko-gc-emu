/*!
* Copyright (C) 2005-2012 Gekko Emulator
*
* \file    vertex_loader.h
* \author  ShizZy <shizzy247@gmail.com>
* \date    2012-03-08
* \brief   Loads and decodes vertex data from CP mem
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

#include "common.h"
#include "memory.h"

#include "video_core.h"
#include "fifo.h"
#include "cp_mem.h"

namespace gp {

typedef void (*VertexLoaderTable)(u16);	

s16*    g_position_burst_ptr;
s16     g_position_burst_buffer[0x10000]; // TODO(ShizZy): Find a good size for this

u8*    g_color_burst_ptr;
u8     g_color_burst_buffer[0x10000]; // TODO(ShizZy): Find a good size for this

/*!
 * \brief Send a 2D position vertex to be rendered
 * \param x x-position
 * \param y y-position
 */
static inline void SendPositionXY(f32 x, f32 y) {
    *g_position_burst_ptr = x;
    g_position_burst_ptr++;
    *g_position_burst_ptr = y;
    g_position_burst_ptr++;
}

/*!
 * \brief Send a 3D position vertex to be rendered
 * \param x x-position
 * \param y y-position
 * \param z z-position
 */
static inline void SendPositionXYZ(s16 x, s16 y, s16 z) {
    *g_position_burst_ptr = x;
    g_position_burst_ptr++;
    *g_position_burst_ptr = y;
    g_position_burst_ptr++;
    *g_position_burst_ptr = z;
    g_position_burst_ptr++;

    LOG_DEBUG(TGP, "SentVertex-> %02.04f %02.04f %02.04f", x, y, z);
    //LOG_DEBUG(TGP, "SentPosition-> \t%f\t%f\t%f", x, y, z);
}

/*!
 * \brief Send a RGB color for the previously specified vertex
 * \param r Red component  
 * \param g Green component
 * \param b Blue component 
 * \param a Alpha component
 */
static inline void SendColorRGBA(u8 r, u8 g, u8 b, u8 a) {
    *g_color_burst_ptr = r;
    g_color_burst_ptr++;
    *g_color_burst_ptr = g;
    g_color_burst_ptr++;
    *g_color_burst_ptr = b;
    g_color_burst_ptr++;
   // *g_color_burst_ptr = a;
   // g_color_burst_ptr++;
  //  LOG_DEBUG(TGP, "SendColorRGBA-> \t%f\t%f\t%f\t%f", r, g, b, a);
}


////////////////////////////////////////////////////////////////////////////////////////////////////

__inline u16 MemoryRead16(u32 addr)
{
	if(!(addr & 1))
		return *(u16 *)(&Mem_RAM[(addr ^ 2) & RAM_MASK]);

	addr = addr & RAM_MASK;
	return (u16)(Mem_RAM[(addr + 0) ^ 3] << 8) |
		   (u16)(Mem_RAM[(addr + 1) ^ 3]);
}

__inline u32 MemoryRead32(u32 addr)
{
	addr &= RAM_MASK;
	if(!(addr & 3))
		return *(u32 *)(&Mem_RAM[addr]);

	return ((u32)Mem_RAM[(addr + 0) ^ 3] << 24) |
		   ((u32)Mem_RAM[(addr + 1) ^ 3] << 16) |
		   ((u32)Mem_RAM[(addr + 2) ^ 3] << 8) |
		   ((u32)Mem_RAM[(addr + 3) ^ 3]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Position Decoding

static void VertexPosition_Unk(u16 index) {
    LOG_ERROR(TGP, "Unknown Vertex position!!! Ask neobrain to implement me!");
    logger::Crash();
}

// correct
static void VertexPosition_IF32_XYZ(u16 index) {
	u32 v[3];
	v[0] = MemoryRead32(CP_DATA_POS_ADDR(index) + 0);
	v[1] = MemoryRead32(CP_DATA_POS_ADDR(index) + 4);
	v[2] = MemoryRead32(CP_DATA_POS_ADDR(index) + 8);


    LOG_DEBUG(TGP, "%f %f %f", *(f32*)&v[0], *(f32*)&v[1], *(f32*)&v[2]);

	video_core::g_renderer->VertexPosition_SendFloat((f32*)v);
}

static void VertexPosition_IS16_XYZ(u16 index)
{
	s16 v[3];
	u32 data = MemoryRead32(CP_DATA_POS_ADDR(index));

    //LOG_DEBUG(TGP, "%08x %08x %08x", CP_DATA_POS_ADDR(index), data, MemoryRead16(CP_DATA_POS_ADDR(index) + 4));

	v[0] = (s16)(data >> 16);
	v[1] = (s16)(data & 0xFFFF);
	v[2] = (s16)MemoryRead16(CP_DATA_POS_ADDR(index) + 4);
	video_core::g_renderer->VertexPosition_SendShort((u16*)v);
}


// unimplemented

#define VERTEXLOADER_POSITION_UNDEF(name)   void VertexPosition_##name(u16 index) { \
    LOG_ERROR(TGP, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__ ); \
    logger::Crash(); \
}

VERTEXLOADER_POSITION_UNDEF(DU8_XY);
VERTEXLOADER_POSITION_UNDEF(DS8_XY);
VERTEXLOADER_POSITION_UNDEF(DU16_XY);
VERTEXLOADER_POSITION_UNDEF(DS16_XY);
VERTEXLOADER_POSITION_UNDEF(DF32_XY);
VERTEXLOADER_POSITION_UNDEF(DU8_XYZ);
VERTEXLOADER_POSITION_UNDEF(DS8_XYZ);
VERTEXLOADER_POSITION_UNDEF(DU16_XYZ);
VERTEXLOADER_POSITION_UNDEF(DS16_XYZ);
VERTEXLOADER_POSITION_UNDEF(DF32_XYZ);

VERTEXLOADER_POSITION_UNDEF(IU8_XY);
VERTEXLOADER_POSITION_UNDEF(IS8_XY);
VERTEXLOADER_POSITION_UNDEF(IU16_XY);
VERTEXLOADER_POSITION_UNDEF(IS16_XY);
VERTEXLOADER_POSITION_UNDEF(IF32_XY);
VERTEXLOADER_POSITION_UNDEF(IU8_XYZ);
VERTEXLOADER_POSITION_UNDEF(IS8_XYZ);
VERTEXLOADER_POSITION_UNDEF(IU16_XYZ);
//VERTEXLOADER_POSITION_UNDEF(IS16_XYZ);
//VERTEXLOADER_POSITION_UNDEF(IF32_XYZ);

VertexLoaderTable VertexLoad[0x40] = {
    VertexPosition_Unk, VertexPosition_DU8_XY,  VertexPosition_IU8_XY,  VertexPosition_IU8_XY,  	 
    VertexPosition_Unk, VertexPosition_DS8_XY,  VertexPosition_IS8_XY,  VertexPosition_IS8_XY,  
    VertexPosition_Unk, VertexPosition_DU16_XY, VertexPosition_IU16_XY, VertexPosition_IU16_XY, 	 
    VertexPosition_Unk, VertexPosition_DS16_XY, VertexPosition_IS16_XY, VertexPosition_IS16_XY, 
    VertexPosition_Unk, VertexPosition_DF32_XY, VertexPosition_IF32_XY, VertexPosition_IF32_XY, 	 
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,    
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,     
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,     
    VertexPosition_Unk, VertexPosition_DU8_XYZ, VertexPosition_IU8_XYZ, VertexPosition_IU8_XYZ, 	 
    VertexPosition_Unk, VertexPosition_DS8_XYZ, VertexPosition_IS8_XYZ, VertexPosition_IS8_XYZ, 
    VertexPosition_Unk, VertexPosition_DU16_XYZ,VertexPosition_IU16_XYZ,VertexPosition_IU16_XYZ,	 
    VertexPosition_Unk, VertexPosition_DS16_XYZ,VertexPosition_IS16_XYZ,VertexPosition_IS16_XYZ, 
    VertexPosition_Unk, VertexPosition_DF32_XYZ,VertexPosition_IF32_XYZ,VertexPosition_IF32_XYZ,	 
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,		
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk,		 
    VertexPosition_Unk, VertexPosition_Unk,     VertexPosition_Unk,     VertexPosition_Unk	
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Color Decoding

// correct
static void VertexColor_I8_RGBA8(VertexData *vtx) {
	u32 rgba = MemoryRead32(CP_DATA_COL0_ADDR(vtx->index));
    u8 r = (rgba >> 24);
    u8 g = ((rgba >> 16) & 0xff);
    u8 b = ((rgba >> 8) & 0xff);
    u8 a = (rgba & 0xff);
   // SendColorRGBA(r, g, b, a);
}

////////////////////////////////////////////////////////////////////////////////
// PRIMITIVE DECODING

inline void DecodeVertexFormat(u8 vat, VertexData* pos, VertexData* nrm, VertexData* col0, VertexData* col1) {
    // Position
    pos->vcd = VCD_POS;
    if (pos->vcd) {
        pos->cnt = VAT_POSCNT;
        pos->fmt = VAT_POSFMT;
        pos->dqf = 1.0f / (1 << VAT_POSSHFT);
        pos->vtx_format = kVertexPositionSize[VTX_FORMAT_PTR(pos)];
       // pos->vtx_format_vcd = (void*)gx_send_pos_if32_XYz;
    }

    // Normal
    nrm->vcd = VCD_NRM;
    if (nrm->vcd) {
        nrm->cnt = VAT_NRMCNT;
        nrm->fmt = VAT_NRMFMT;
        nrm->vtx_format = kVertexNormalSize[VTX_FORMAT_PTR(nrm)];
//        nrm->vtx_format_vcd = (void*)gx_send_col_irgba8;
    }

    // Color 0
    col0->vcd = VCD_COL0;
    if (col0->vcd) {
        col0->cnt = VAT_COL0CNT;
        col0->fmt = VAT_COL0FMT;
        col0->vtx_format = kVertexColorSize[VTX_FORMAT_PTR(col0)];
       // col0->vtx_format_vcd = (void *)gx_send_col_irgba8;
    }

    // Color 1
    col1->vcd = VCD_COL1;
    if (col1->vcd) {
        col1->cnt = VAT_COL1CNT;
        col1->fmt = VAT_COL1FMT;
        col1->vtx_format = kVertexColorSize[VTX_FORMAT_PTR(col1)];
//        col1->vtx_format_vcd = (void *)gx_send_col[VTX_FORMAT_VCD(col1)];
    }
}


// send vertex to the renderer
inline void DecodeVertex(u8 vat, VertexData pos, VertexData nrm, VertexData col0, VertexData col1) {

    // DECODE POSITION FORMAT

    // get pos index (if used)
    switch(pos.vcd)
    {
    case 0: // no data present
        break;
    case 1: // direct
        //pos.position = offset; // store vertex data position
        //offset+=pos.vtx_format; // offset over data..
        //pos.vtx_format_vcd(&pos); 
        break;
    case 2: // 8bit index
        pos.index = FifoPop8();
        //VertexPosition_I8_F32_XYZ(pos.index);
        VertexLoad[VTX_FORMAT_VCD(pos)](pos.index);
        break;
    case 3: // 16bit index
        pos.index = FifoPop16();
        //pos.index = _gxlist->get16(offset);
        //offset+=2;
        //((gxtable)pos.vtx_format_vcd)(_gxlist, &pos); 
        break;
    }

    // DECODE NORMAL FORMAT (FAKE)

    switch(nrm.vcd)
    {
    case 0: // no data present
        break;
    case 1: // direct
        //nrm.position = offset; // store vertex data position
        //offset+=nrm.vtx_format; // offset over data
        //((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
        break;
    case 2: // 8bit index
        FifoPop8();
        //nrm.index = _gxlist->get8(offset++);
        //((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
        break;
    case 3: // 16bit index
        FifoPop16();
        //nrm.index = _gxlist->get16(offset);
        //offset+=2;
        //((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
        break;
    }

    // DECODE DIFFUSE COLOR FORMAT

    // get color index (if used)
    switch(col0.vcd)
    {
    case 0: // no data present
        //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        break;
    case 1: // direct
        //col0.position = offset; // store vertex data position
        //offset+=col0.vtx_format; // offset over data
        //((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
        //glColor4bv((GLbyte *)cv->color);


        break;
    case 2: // 8bit index
        //col0.index = _gxlist->get8(offset++);
        //((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
        //glColor4bv((GLbyte *)cv->color);

        col0.index = FifoPop8();
        VertexColor_I8_RGBA8(&col0);

        break;
    case 3: // 16bit index 
        FifoPop16();
        //col0.index = _gxlist->get16(offset);
        //offset+=2;
        //((gxtable)col0.vtx_format_vcd)(_gxlist, &col0); 
        //glColor4bv((GLbyte *)cv->color);
        break;
    }

    // DECODE SPECULAR COLOR FORMAT (FAKE)

    switch(col1.vcd)
    {
    case 0: // no data present
        break;
    case 1: // direct
        //col1.position = offset; // store vertex data position
        //offset+=col1.vtx_format; // offset over data
        //((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
        //glSecondaryColor3bv((GLbyte *)cv->color);
        break;
    case 2: // 8bit index
        FifoPop8();
        //col1.index = _gxlist->get8(offset++);
        //((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
        //glSecondaryColor3bv((GLbyte *)cv->color);
        break;
    case 3: // 16bit index
        FifoPop16();
        //col1.index = _gxlist->get16(offset);
        //offset+=2;
        //((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
        //glSecondaryColor3bv((GLbyte *)cv->color);
        break;
    }
}

// begin primitive drawing
void DecodePrimitive(GXPrimitive type, int count, u8 vat) {

    VertexData pos, nrm, col0, col1;
    DecodeVertexFormat(vat, &pos, &nrm, &col0, &col1);

    // set position transformation
    //if(vcd_pmidx_result) 
    //    glLoadIdentity();
    //else
    //    gx_transform::gx_tf_pos_hardware();

    // begin primitive type
    //glBegin(_type);

    // decode vertices
    //get_vertex(vat);

    video_core::g_renderer->BeginPrimitive(type, count);
    //video_core::g_renderer->VertexPosition_SetType(type, count);

    for (int i = 0; i < count; i++) {
        DecodeVertex(vat, pos, nrm, col0, col1);
        video_core::g_renderer->VertexNext();
    }
    
    video_core::g_renderer->EndPrimitive();

    // End of burst - Reset buffers
    g_position_burst_ptr    = g_position_burst_buffer;
    g_color_burst_ptr       = g_color_burst_buffer;
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

/// Initialize the Vertex Loader
void VertexLoaderInit() {
    g_position_burst_ptr = g_position_burst_buffer;
    g_color_burst_ptr = g_color_burst_buffer;
}

/// Shutdown the Vertex Loader
void VertexLoaderShutdown() {
}

} // namespace
