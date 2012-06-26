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

static void VertexPosition_DF32_XYZ(u16 index) {
	u32 v[3];
	v[0] = FifoPop32();
	v[1] = FifoPop32();
	v[2] = FifoPop32();
	video_core::g_renderer->VertexPosition_SendFloat((f32*)v);
}

// correct
static void VertexPosition_IF32_XYZ(u16 index) {
	u32 v[3];
	v[0] = MemoryRead32(CP_DATA_POS_ADDR(index) + 0);
	v[1] = MemoryRead32(CP_DATA_POS_ADDR(index) + 4);
	v[2] = MemoryRead32(CP_DATA_POS_ADDR(index) + 8);
	video_core::g_renderer->VertexPosition_SendFloat((f32*)v);
}

// correct
static void VertexPosition_IS16_XYZ(u16 index)
{
	s16 v[3];
	u32 data = MemoryRead32(CP_DATA_POS_ADDR(index));
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
//VERTEXLOADER_POSITION_UNDEF(DF32_XYZ);

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

VertexLoaderTable LookupPosition[0x40] = {
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


#define VERTEXLOADER_COLOR_UNDEF(name)      void VertexColor_##name(u16 index) { \
    LOG_ERROR(TGP, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__ ); \
    logger::Crash(); \
}

static void VertexColor_Unk(u16 index) {
    LOG_ERROR(TGP, "Unknown Vertex color!!! Ask neobrain to implement me!");
    logger::Crash();
}

static void VertexColor_DRGBA8(u16 index) {
    video_core::g_renderer->VertexColor0_Send(FifoPop32());
}

// correct
static void VertexColor_IRGBA8(u16 index) {
	u32 rgba = MemoryRead32(CP_DATA_COL0_ADDR(index));
    video_core::g_renderer->VertexColor0_Send(rgba);
}

VERTEXLOADER_COLOR_UNDEF(DRGB565);
VERTEXLOADER_COLOR_UNDEF(DRGB8)
VERTEXLOADER_COLOR_UNDEF(DRGBA4);
VERTEXLOADER_COLOR_UNDEF(DRGBA6);
//VERTEXLOADER_COLOR_UNDEF(DRGBA8);

VERTEXLOADER_COLOR_UNDEF(IRGB565);
VERTEXLOADER_COLOR_UNDEF(IRGB8);
VERTEXLOADER_COLOR_UNDEF(IRGBA4);
VERTEXLOADER_COLOR_UNDEF(IRGBA6);
//VERTEXLOADER_COLOR_UNDEF(IRGBA8);

VertexLoaderTable LookupColor[0x40] = {
	VertexColor_Unk,    VertexColor_DRGB565,    VertexColor_IRGB565,    VertexColor_IRGB565, 
	VertexColor_Unk,    VertexColor_DRGB8,      VertexColor_IRGB8,      VertexColor_IRGB8, // 7
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_IRGB8,      VertexColor_IRGB8, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, // 15
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_IRGBA8,     VertexColor_IRGBA8, // 23
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, // 31
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, // 39
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_DRGBA4,     VertexColor_IRGBA4,     VertexColor_IRGBA4, // 47
	VertexColor_Unk,    VertexColor_DRGBA6,     VertexColor_IRGBA6,     VertexColor_IRGBA6, 
	VertexColor_Unk,    VertexColor_DRGBA8,     VertexColor_IRGBA8,     VertexColor_IRGBA8, // 55
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk, 
	VertexColor_Unk,    VertexColor_Unk,        VertexColor_Unk,        VertexColor_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Normal Decoding

#define VERTEXLOADER_NORMAL_UNDEF(name)     void VertexNormal_##name(u16 index) { \
    LOG_ERROR(TGP, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__ ); \
    logger::Crash(); \
}

static void VertexNormal_Unk(u16 index) {
    LOG_ERROR(TGP, "Unknown Vertex normal!!! Ask neobrain to implement me!");
    logger::Crash();
}

VERTEXLOADER_NORMAL_UNDEF(DU8_3);
VERTEXLOADER_NORMAL_UNDEF(DS8_3);
VERTEXLOADER_NORMAL_UNDEF(DS16_3);
VERTEXLOADER_NORMAL_UNDEF(DF32_3);
VERTEXLOADER_NORMAL_UNDEF(IU8_3);
VERTEXLOADER_NORMAL_UNDEF(IS8_3);
VERTEXLOADER_NORMAL_UNDEF(IS16_3);
VERTEXLOADER_NORMAL_UNDEF(IF32_3);

// TODO(ShizZy): Decoding 9 normals.... needs to be added to this table
VertexLoaderTable LookupNormal[0x40] = {
	VertexNormal_Unk,   VertexNormal_DU8_3,     VertexNormal_IU8_3,    VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_DS8_3,     VertexNormal_IS8_3,    VertexNormal_IS8_3, // 7
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_DS16_3,    VertexNormal_IS16_3,   VertexNormal_IS16_3, // 15
	VertexNormal_Unk,   VertexNormal_DF32_3,    VertexNormal_IF32_3,   VertexNormal_IF32_3, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 23
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 31
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 39
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 47
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, // 55
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk, 
	VertexNormal_Unk,   VertexNormal_Unk,       VertexNormal_Unk,       VertexNormal_Unk
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// TexCoord Decoding

#define VERTEXLOADER_TEXCOORD_UNDEF(name)   void VertexTexCoord_##name(u16 index) { \
    LOG_ERROR(TGP, "Unimplemented function %s !!! Ask neobrain to implement me!", __FUNCTION__ ); \
    logger::Crash(); \
}

static void VertexTexCoord_Unk(u16 index) {
    LOG_ERROR(TGP, "Unknown Vertex texture coordinate!!! Ask neobrain to implement me!");
    logger::Crash();
}

VERTEXLOADER_TEXCOORD_UNDEF(DU8_S);
VERTEXLOADER_TEXCOORD_UNDEF(DS8_S);
VERTEXLOADER_TEXCOORD_UNDEF(DU16_S);
VERTEXLOADER_TEXCOORD_UNDEF(DS16_S);
VERTEXLOADER_TEXCOORD_UNDEF(DF32_S);
VERTEXLOADER_TEXCOORD_UNDEF(DU8_ST);
VERTEXLOADER_TEXCOORD_UNDEF(DS8_ST);
VERTEXLOADER_TEXCOORD_UNDEF(DU16_ST);
VERTEXLOADER_TEXCOORD_UNDEF(DS16_ST);
VERTEXLOADER_TEXCOORD_UNDEF(DF32_ST);
VERTEXLOADER_TEXCOORD_UNDEF(IU8_S);
VERTEXLOADER_TEXCOORD_UNDEF(IS8_S);
VERTEXLOADER_TEXCOORD_UNDEF(IU16_S);
VERTEXLOADER_TEXCOORD_UNDEF(IS16_S);
VERTEXLOADER_TEXCOORD_UNDEF(IF32_S);
VERTEXLOADER_TEXCOORD_UNDEF(IU8_ST);
VERTEXLOADER_TEXCOORD_UNDEF(IS8_ST);
VERTEXLOADER_TEXCOORD_UNDEF(IU16_ST);
VERTEXLOADER_TEXCOORD_UNDEF(IS16_ST);
VERTEXLOADER_TEXCOORD_UNDEF(IF32_ST);

VertexLoaderTable LookupTexCoord[0x40] = {
	VertexTexCoord_Unk, VertexTexCoord_DU8_S,   VertexTexCoord_IU8_S,   VertexTexCoord_IU8_S, 
	VertexTexCoord_Unk, VertexTexCoord_DS8_S,   VertexTexCoord_IS8_S,   VertexTexCoord_IS8_S,
	VertexTexCoord_Unk, VertexTexCoord_DU16_S,  VertexTexCoord_IU16_S,  VertexTexCoord_IU16_S, 
	VertexTexCoord_Unk, VertexTexCoord_DS16_S,  VertexTexCoord_IS16_S,  VertexTexCoord_IS16_S, 
	VertexTexCoord_Unk, VertexTexCoord_DF32_S,  VertexTexCoord_IF32_S,  VertexTexCoord_IF32_S, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_DU8_ST,  VertexTexCoord_IU8_ST,  VertexTexCoord_IU8_ST, 
	VertexTexCoord_Unk, VertexTexCoord_DS8_ST,  VertexTexCoord_IS8_ST,  VertexTexCoord_IS8_ST, 
	VertexTexCoord_Unk, VertexTexCoord_DU16_ST, VertexTexCoord_IU16_ST, VertexTexCoord_IU16_ST, 
	VertexTexCoord_Unk, VertexTexCoord_DS16_ST, VertexTexCoord_IS16_ST, VertexTexCoord_IS16_ST, 
	VertexTexCoord_Unk, VertexTexCoord_DF32_ST, VertexTexCoord_IF32_ST, VertexTexCoord_IF32_ST, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk, 
	VertexTexCoord_Unk, VertexTexCoord_Unk,     VertexTexCoord_Unk,     VertexTexCoord_Unk
};

////////////////////////////////////////////////////////////////////////////////
// PRIMITIVE DECODING

inline void DecodeVertexFormat(u8 vat, VertexData* pos, VertexData* nrm, VertexData* col0, VertexData* col1) {
    // Position
    pos->vcd = VCD_POS;
    if (pos->vcd) {
        pos->cnt = (GXCompCnt)VAT_POSCNT;
        pos->fmt = (GXCompType)VAT_POSFMT;
        pos->dqf = 1.0f / (1 << VAT_POSSHFT);
        pos->vtx_format = kVertexPositionSize[VTX_FORMAT_PTR(pos)];
       // pos->vtx_format_vcd = (void*)gx_send_pos_if32_XYz;
    }

    // Normal
    nrm->vcd = VCD_NRM;
    if (nrm->vcd) {
        nrm->cnt = (GXCompCnt)VAT_NRMCNT;
        nrm->fmt = (GXCompType)VAT_NRMFMT;
        nrm->vtx_format = kVertexNormalSize[VTX_FORMAT_PTR(nrm)];
//        nrm->vtx_format_vcd = (void*)gx_send_col_irgba8;
    }

    // Color 0
    col0->vcd = VCD_COL0;
    if (col0->vcd) {
        col0->cnt = (GXCompCnt)VAT_COL0CNT;
        col0->fmt = (GXCompType)VAT_COL0FMT;
        col0->vtx_format = kVertexColorSize[VTX_FORMAT_PTR(col0)];
       // col0->vtx_format_vcd = (void *)gx_send_col_irgba8;
    }

    // Color 1
    col1->vcd = VCD_COL1;
    if (col1->vcd) {
        col1->cnt = (GXCompCnt)VAT_COL1CNT;
        col1->fmt = (GXCompType)VAT_COL1FMT;
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
        LookupPosition[VTX_FORMAT_VCD(pos)](0);
        break;
    case 2: // 8bit index
        pos.index = FifoPop8();
        LookupPosition[VTX_FORMAT_VCD(pos)](pos.index);
        break;
    case 3: // 16bit index
        pos.index = FifoPop16();
        LookupPosition[VTX_FORMAT_VCD(pos)](pos.index);
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
        break;
    case 1: // direct
        LookupColor[VTX_FORMAT_VCD(col0)](0);
        break;
    case 2: // 8bit index
        col0.index = FifoPop8();
        LookupColor[VTX_FORMAT_VCD(col0)](col0.index);
        break;
    case 3: // 16bit index 
        col0.index = FifoPop16();
        LookupColor[VTX_FORMAT_VCD(col0)](col0.index);
        break;
    }

    // DECODE SPECULAR COLOR FORMAT (FAKE)

    switch(col1.vcd)
    {
    case 0: // no data present
        break;
    case 1: // direct
        LookupColor[VTX_FORMAT_VCD(col0)](0);
        break;
    case 2: // 8bit index
        col1.index = FifoPop8();
        LookupColor[VTX_FORMAT_VCD(col1)](col1.index);
        break;
    case 3: // 16bit index
        col1.index = FifoPop16();
        LookupColor[VTX_FORMAT_VCD(col1)](col1.index);
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
    video_core::g_renderer->VertexPosition_SetType(pos.fmt, pos.cnt);

    for (int i = 0; i < count; i++) {
        DecodeVertex(vat, pos, nrm, col0, col1);
        video_core::g_renderer->VertexNext();
    }
    
    video_core::g_renderer->EndPrimitive();
}

////////////////////////////////////////////////////////////////////////////////
// MAIN CONTROL

/// Initialize the Vertex Loader
void VertexLoaderInit() {
}

/// Shutdown the Vertex Loader
void VertexLoaderShutdown() {
}

} // namespace
