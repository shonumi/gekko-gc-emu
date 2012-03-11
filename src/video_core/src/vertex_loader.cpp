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

f32*    g_position_burst_ptr;
f32     g_position_burst_buffer[0x10000]; // TODO(ShizZy): Find a good size for this

f32*    g_color_burst_ptr;
f32     g_color_burst_buffer[0x10000]; // TODO(ShizZy): Find a good size for this

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
static inline void SendPositionXYZ(f32 x, f32 y, f32 z) {
    *g_position_burst_ptr = x;
    g_position_burst_ptr++;
    *g_position_burst_ptr = y;
    g_position_burst_ptr++;
    *g_position_burst_ptr = z;
    g_position_burst_ptr++;
    //LOG_DEBUG(TGP, "SentPosition-> \t%f\t%f\t%f", x, y, z);
}

/*!
 * \brief Send a RGB color for the previously specified vertex
 * \param r Red component (0.0f-1.0f)
 * \param g Green component (0.0f-1.0f)
 * \param b Blue component (0.0f-1.0f)
 * \param a Alpha component (0.0f-1.0f)
 */
static inline void SendColorRGBA(f32 r, f32 g, f32 b, f32 a) {
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

__inline u16 MemoryRead16(u32 addr) {
    return *(u16 *)(&Mem_RAM[addr & RAM_MASK]);
}

__inline u32 MemoryRead32(u32 addr) {
    return *(u32 *)(&Mem_RAM[addr & RAM_MASK]);
}

////////////////////////////////////////////////////////////////////////////////
// POSITION DECODING

// correct
static void VertexPosition_I8_F32_XYZ(VertexData *vtx) {
	t32 v[3];
	v[0]._u32 = MemoryRead32(CP_DATA_POS_ADDR(vtx->index) + 0);
	v[1]._u32 = MemoryRead32(CP_DATA_POS_ADDR(vtx->index) + 4);
	v[2]._u32 = MemoryRead32(CP_DATA_POS_ADDR(vtx->index) + 8);
	SendPositionXYZ(v[0]._f32, v[1]._f32, v[2]._f32);
}

// correct
static void VertexColor_I8_RGBA8(VertexData *vtx) {
	u32 rgba = MemoryRead32(CP_DATA_COL0_ADDR(vtx->index));
    f32 r = (rgba >> 24) / 255.0f;
    f32 g = ((rgba >> 16) & 0xff) / 255.0f;
    f32 b = ((rgba >> 8) & 0xff) / 255.0f;
    f32 a = (rgba & 0xff) / 255.0f;
    SendColorRGBA(r, g, b, a);
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
        pos->vtx_format = kVertexPositionSize[VTX_FORMAT(pos)];
       // pos->vtx_format_vcd = (void*)gx_send_pos_if32xyz;
    }

    // Normal
    nrm->vcd = VCD_NRM;
    if (nrm->vcd) {
        nrm->cnt = VAT_NRMCNT;
        nrm->fmt = VAT_NRMFMT;
        nrm->vtx_format = kVertexNormalSize[VTX_FORMAT(nrm)];
//        nrm->vtx_format_vcd = (void*)gx_send_col_irgba8;
    }

    // Color 0
    col0->vcd = VCD_COL0;
    if (col0->vcd) {
        col0->cnt = VAT_COL0CNT;
        col0->fmt = VAT_COL0FMT;
        col0->vtx_format = kVertexColorSize[VTX_FORMAT(col0)];
       // col0->vtx_format_vcd = (void *)gx_send_col_irgba8;
    }

    // Color 1
    col1->vcd = VCD_COL1;
    if (col1->vcd) {
        col1->cnt = VAT_COL1CNT;
        col1->fmt = VAT_COL1FMT;
        col1->vtx_format = kVertexColorSize[VTX_FORMAT(col1)];
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
        pos.index = FIFO_POP8();
        VertexPosition_I8_F32_XYZ(&pos);
        break;
    case 3: // 16bit index
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
        //nrm.index = _gxlist->get8(offset++);
        //((gxtable)nrm.vtx_format_vcd)(_gxlist, &nrm);
        break;
    case 3: // 16bit index
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

        col0.index = FIFO_POP8();
        VertexColor_I8_RGBA8(&col0);

        break;
    case 3: // 16bit index 
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
        //col1.index = _gxlist->get8(offset++);
        //((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
        //glSecondaryColor3bv((GLbyte *)cv->color);
        break;
    case 3: // 16bit index
        //col1.index = _gxlist->get16(offset);
        //offset+=2;
        //((gxtable)col1.vtx_format_vcd)(_gxlist, &col1); 
        //glSecondaryColor3bv((GLbyte *)cv->color);
        break;
    }
}

// begin primitive drawing
void DecodePrimitive(int type, int count, u8 vat) {

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


    for (int i = 0; i < count; i++) {
        DecodeVertex(vat, pos, nrm, col0, col1);
    }

    // end primitive type
    //glEnd();*/

    video_core::g_renderer->DrawPrimitive();

    // End of burst - Reset buffers
    g_position_burst_ptr = g_position_burst_buffer;
    g_color_burst_ptr = g_color_burst_buffer;
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
