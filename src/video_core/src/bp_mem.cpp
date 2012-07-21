/*!
* Copyright (C) 2005-2012 Gekko Emulator
*
* \file    bp_mem.cpp
* \author  ShizZy <shizzy247@gmail.com>
* \date    2012-03-10
* \brief   Implementation of BP for the graphics processor
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

#include <GL/glew.h>

#include "hw/hw_pe.h"

#include "video_core.h"
#include "fifo.h"
#include "bp_mem.h"
#include "texture_decoder.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

#undef LOG_DEBUG
#define LOG_DEBUG(x,y, ...)

#define BP_PE_COPYCLEAR_Z			gp::g_bp_regs.mem[0x51]
#define BP_PE_COPYCLEAR_Z_VALUE		(BP_PE_COPYCLEAR_Z & 0xffffff)
#define GX_VIEWPORT_ZMAX				16777215.0f
namespace gp {

BPMemory g_bp_regs; ///< BP memory/registers

/// Write a BP register
void BPRegisterWrite(u8 addr, u32 data) {
    LOG_DEBUG(TGP, "BP_LOAD [%02x] = %08x", addr, data);

    // write data to bp memory
    g_bp_regs.mem[addr] = data;

    // adjust gx globals accordingly
    switch(addr) {
    case BP_REG_GENMODE: // GEN_MODE
        video_core::g_renderer->SetGenerationMode();
        break;

    case BP_REG_SCISSORTL: // SU_SCIS0 - Scissorbox Top Left Corner
    case BP_REG_SCISSORBR: // SU_SCIS1 - Scissorbox Bottom Right Corner
        //gx_states::set_scissors();
        LOG_DEBUG(TGP, "BP-> SU_SCISx");
        break;

    case BP_REG_LINEPTWIDTH: // SU_LPSIZE - Line and Point Size
        //gx_states::set_lpsize();
        LOG_DEBUG(TGP, "BP-> SU_LPSIZE");
        break;

    case BP_REG_TREF + 0: // RAS_TREF0
    case BP_REG_TREF + 1: // RAS_TREF1
    case BP_REG_TREF + 2: // RAS_TREF2
    case BP_REG_TREF + 3: // RAS_TREF3
    case BP_REG_TREF + 4: // RAS_TREF4
    case BP_REG_TREF + 5: // RAS_TREF5
    case BP_REG_TREF + 6: // RAS_TREF6
    case BP_REG_TREF + 7: // RAS_TREF7
        //gx_tev::set_modifed();
        LOG_DEBUG(TGP, "BP-> RAS_TREFx");
        break;

    case BP_REG_PE_ZMODE: // PE_ZMODE set z mode
        video_core::g_renderer->SetDepthMode();
        break;

    case BP_REG_PE_CMODE0: // PE_CMODE0 dithering / blend mode/color_update/alpha_update/set_dither
        //gx_states::set_cmode0();
        if (data & 0xFFFF) {
            // Set LogicOp Blending Mode
            if (data & 2) {
                video_core::g_renderer->SetLogicOpMode();
            }
            // Set Dithering Mode
            if (data & 4) {
                video_core::g_renderer->SetDitherMode();
            }
            // Set Blending Mode
            if (data & 0xFE1) {
                video_core::g_renderer->SetBlendMode(false);
            }
            // Set Color Mask
            if (data & 0x18) {
                // TODO(ShizZy): Renable when we have EFB emulated
                //video_core::g_renderer->SetColorMask();
            }
        }
        break;

    case BP_REG_PE_CMODE1: // PE_CMODE1 destination alpha
        //gx_tev::set_modifed();
        LOG_DEBUG(TGP, "BP-> PE_CMODE1");
        break;

    case BP_REG_PE_CONTROL: // PE_CONTROL comp z location z_comp_loc(0x43000040)pixel_fmt(0x43000041)
        //video_core::g_renderer->SetColorMask(); TODO(ShizZy): Renable when we have EFB emulated
        break;

    case BP_REG_PE_DRAWDONE: // PE_DONE - draw done
        LOG_DEBUG(TGP, "BP-> PE_DONE");

	    if (g_bp_regs.mem[0x45] & 0x2) { // enable interrupt

            FifoReset();
            GX_PE_FINISH = 1;
            video_core::g_renderer->SwapBuffers();
        }
        break;

    case BP_REG_PE_TOKEN: // PE_TOKEN
        GX_PE_TOKEN_VALUE = (data & 0xffff);
        LOG_DEBUG(TGP, "BP-> PE_TOKEN");
        break;

    case BP_REG_PE_TOKEN_INT: // PE_TOKEN_INT
        GX_PE_TOKEN_VALUE = (data & 0xffff); 
        GX_PE_TOKEN = 1;
        LOG_DEBUG(TGP, "BP-> PE_TOKEN_INT");
        break;

    case BP_REG_PE_CLEAR_AR: // PE copy clear AR - set clear alpha and red components
    case BP_REG_PE_CLEAR_GB: // PE copy clear GB - green and blue
        //gx_states::set_copyclearcolor();
        LOG_DEBUG(TGP, "BP-> PE_COPY_CLEAR_COLOR");
        break;

    case BP_REG_PE_CLEAR_Z: // PE copy clear Z - 24-bit Z value
        //gx_states::set_copyclearz();
	    // unpack z data
	    // send to efb
	    //glClearDepth(((GLclampd)BP_PE_COPYCLEAR_Z_VALUE) / GX_VIEWPORT_ZMAX);
        LOG_DEBUG(TGP, "BP-> PE_COPY_CLEAR_X");
        break;

    case BP_REG_PE_COPY_EXECUTE: // pe copy execute

        LOG_DEBUG(TGP, "BP-> PE_COPY_EFB");

        BPPECopyExecute pe_copy;
        pe_copy._u32 = data;

        if (pe_copy.copy_to_xfb) {
            video_core::g_renderer->CopyEFB(RendererBase::kFramebuffer_VirtualXFB);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        } else {
            // TODO(ShizZy): Implement copy to texture
        }
        break;

    case BP_REG_SCISSOROFFSET: // Scissorbox Offset
        //gx_states::set_scissors();
        LOG_DEBUG(TGP, "BP-> Set scissors");
        break;

    case BP_REG_LOADTLUT0: // TX_LOADTLUT0
    case BP_REG_LOADTLUT1: // TX_LOADTLUT1
        //gx_states::load_tlut();
        LOG_DEBUG(TGP, "BP-> TX_LOADTLUTx");
        break;

/*
    case BP_REG_TX_SETMODE0 + 0: // TX_SETMODE0_I0 - Texture lookup and filtering mode
    case BP_REG_TX_SETMODE0 + 1: // TX_SETMODE0_I1
    case BP_REG_TX_SETMODE0 + 2: // TX_SETMODE0_I2
    case BP_REG_TX_SETMODE0 + 3: // TX_SETMODE0_I3
    case BP_REG_TX_SETMODE0_4 + 0: // TX_SETMODE0_I4
    case BP_REG_TX_SETMODE0_4 + 1: // TX_SETMODE0_I5
    case BP_REG_TX_SETMODE0_4 + 2: // TX_SETMODE0_I6
    case BP_REG_TX_SETMODE0_4 + 3: // TX_SETMODE0_I7
        //gx_states::tx_setmode0(_addr);
        LOG_DEBUG(TGP, "BP-> TX_SETMODE0_Ix");
        break;

    case 0xc0: // TEV_COLOR_ENV_0
    case 0xc1: // TEV_ALPHA_ENV_0
    case 0xc2: // TEV_COLOR_ENV_1
    case 0xc3: // TEV_ALPHA_ENV_1
    case 0xc4: // TEV_COLOR_ENV_2
    case 0xc5: // TEV_ALPHA_ENV_2
    case 0xc6: // TEV_COLOR_ENV_3
    case 0xc7: // TEV_ALPHA_ENV_3
    case 0xc8: // TEV_COLOR_ENV_4
    case 0xc9: // TEV_ALPHA_ENV_4
    case 0xca: // TEV_COLOR_ENV_5
    case 0xcb: // TEV_ALPHA_ENV_5
    case 0xcc: // TEV_COLOR_ENV_6
    case 0xcd: // TEV_ALPHA_ENV_6
    case 0xce: // TEV_COLOR_ENV_7
    case 0xcf: // TEV_ALPHA_ENV_7
    case 0xd0: // TEV_COLOR_ENV_8
    case 0xd1: // TEV_ALPHA_ENV_8
    case 0xd2: // TEV_COLOR_ENV_9
    case 0xd3: // TEV_ALPHA_ENV_9
    case 0xd4: // TEV_COLOR_ENV_A
    case 0xd5: // TEV_ALPHA_ENV_A
    case 0xd6: // TEV_COLOR_ENV_B
    case 0xd7: // TEV_ALPHA_ENV_B
    case 0xd8: // TEV_COLOR_ENV_C
    case 0xd9: // TEV_ALPHA_ENV_C
    case 0xda: // TEV_COLOR_ENV_D
    case 0xdb: // TEV_ALPHA_ENV_D
    case 0xdc: // TEV_COLOR_ENV_E
    case 0xdd: // TEV_ALPHA_ENV_E
    case 0xde: // TEV_COLOR_ENV_F
    case 0xdf: // TEV_ALPHA_ENV_F
        //if(gx_tev::combiner[_addr - 0xc0] != _value)
        //{
        //	gx_tev::combiner[_addr - 0xc0] = _value;
        //	gx_tev::set_modifed();
        //}
        LOG_DEBUG(TGP, "BP-> TEV_COLOR/ALPHA_ENV_x");
        break;

    case 0xe0: // TEV_REGISTERL_0
    case 0xe2: // TEV_REGISTERL_1
    case 0xe4: // TEV_REGISTERL_2
    case 0xe6: // TEV_REGISTERL_3
    case 0xe1: // TEV_REGISTERH_0
    case 0xe3: // TEV_REGISTERH_1
    case 0xe5: // TEV_REGISTERH_2
    case 0xe7: // TEV_REGISTERH_3
        //gx_tev::upload_color(_addr, _value);
        LOG_DEBUG(TGP, "BP-> TEV_REGISTERx_x");
        break;

    case 0xf3: // TEV_ALPHAFUNC
        //gx_states::set_alphafunc();
        LOG_DEBUG(TGP, "BP-> TEV_ALPHAFUNC");
        break;

    case 0xf6: // TEV_KSEL_0
    case 0xf7: // TEV_KSEL_1
    case 0xf8: // TEV_KSEL_2
    case 0xf9: // TEV_KSEL_3
    case 0xfa: // TEV_KSEL_4
    case 0xfb: // TEV_KSEL_5
    case 0xfc: // TEV_KSEL_6
    case 0xfd: // TEV_KSEL_7
        //gx_tev::set_modifed();
        LOG_DEBUG(TGP, "BP-> TEV_KSEL_x");
        break;*/
    }
}

static const GLenum gx_type_wrapst[8] = 
{
	GL_CLAMP_TO_EDGE,                 
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_REPEAT
};

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

// set texture mode 0
void tx_setmode0(u8 _addr)
{
	GLint mag_filter = ((g_bp_regs.mem[_addr] >> 4) & 1) ? GL_LINEAR : GL_NEAREST; 
	GLint wrap_s = gx_type_wrapst[g_bp_regs.mem[_addr] & 3];
	GLint wrap_t = gx_type_wrapst[(g_bp_regs.mem[_addr] >> 2) & 3];
	//GLfloat lodbias = (((f32)(s8)((g_bp_regs.mem[_addr] >> 9) & 0xff)) / 32.0f);
	//int use_mips = ((g_bp_regs.mem[_addr] >> 5) & 7); // use mip maps
	//GLint min_filter = gx_min_filter[use_mips];
	//use_mips &= 3;

	//if(use_mips)
	//	glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	//else
	//	glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

	//glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, lodbias);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);



#pragma todo(tx_setmod0: Missing LOD and BIAS clamping)
}

void LoadTexture(u8 index) {
	u32 tx_setimage0;
	u32 tx_setimage3;
    int num;
    u32 addr;
	//gx_texture_data tx;

	num = GX_TX_SETIMAGE_NUM(index);

	if(num < 4) {
		tx_setimage0 = g_bp_regs.mem[0x88 + num];
		tx_setimage3 = g_bp_regs.mem[0x94 + num];
	} else {
		tx_setimage0 = g_bp_regs.mem[0xA8 + num - 4];
		tx_setimage3 = g_bp_regs.mem[0xB4 + num - 4];
	}

	addr = (tx_setimage3 & 0xffffff);

	if(texcache[TEX_CACHE_LOCATION(addr)]) {
		glBindTexture(GL_TEXTURE_2D, texcache[TEX_CACHE_LOCATION(addr)]);

		if(num < 4) 
			tx_setmode0(0x80 + num); 
		else 
			tx_setmode0(0xa0 + (num - 4));/*

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);*/

		glEnable(GL_TEXTURE_2D);
	}else{
		DecodeTexture(((tx_setimage0 >> 20) & 0xf), // Format
                      (addr << 5), // Address
                      (((tx_setimage0 >> 10) & 0x3ff) + 1), // Height
                      ((tx_setimage0 & 0x3ff) + 1)); // Width
	}
}

/// Initialize BP
void BPInit() {
    memset(&g_bp_regs, 0, sizeof(g_bp_regs));
}

} // namespace