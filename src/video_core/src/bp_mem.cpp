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

#include "hw/hw_pe.h"

#include "video_core.h"
#include "fifo.h"
#include "bp_mem.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

#undef LOG_DEBUG
#define LOG_DEBUG(x,y, ...)

namespace gp {

BPMemory g_bp_regs; ///< BP memory/registers

/// Write a BP register
void BPRegisterWrite(u8 addr, u32 data) {
    LOG_DEBUG(TGP, "BP_LOAD [%02x] = %08x", addr, data);

    // write data to bp memory
    g_bp_regs.mem[addr] = data;

    // adjust gx globals accordingly
    switch(addr)
    {
    case 0x0: // GEN_MODE
        //gx_states::set_cullmode();
        //gx_tev::active_stages = bp.genmode.ntev + 1;
        //gx_tev::set_modifed();

        LOG_DEBUG(TGP, "BP-> GEN_MODE");
        break;

    case 0x20: // SU_SCIS0 - Scissorbox Top Left Corner
    case 0x21: // SU_SCIS1 - Scissorbox Bottom Right Corner
        //gx_states::set_scissors();
        LOG_DEBUG(TGP, "BP-> SU_SCISx");
        break;

    case 0x22: // SU_LPSIZE - Line and Point Size
        //gx_states::set_lpsize();
        LOG_DEBUG(TGP, "BP-> SU_LPSIZE");
        break;

    case 0x28: // RAS_TREF0
    case 0x29: // RAS_TREF1
    case 0x2a: // RAS_TREF2
    case 0x2b: // RAS_TREF3
    case 0x2c: // RAS_TREF4
    case 0x2d: // RAS_TREF5
    case 0x2e: // RAS_TREF6
    case 0x2f: // RAS_TREF7
        //gx_tev::set_modifed();
        LOG_DEBUG(TGP, "BP-> RAS_TREFx");
        break;

    case 0x40: // PE_ZMODE
        //gx_states::set_zmode();
        LOG_DEBUG(TGP, "BP-> PE_ZMODE");
        break;

    case 0x41: // PE_CMODE0
        //gx_states::set_cmode0();
        LOG_DEBUG(TGP, "BP-> PE_CMODE0");
        break;

    case 0x42: // PE_CMODE1
        //gx_tev::set_modifed();
        LOG_DEBUG(TGP, "BP-> PE_CMODE1");
        break;

    case 0x45: // PE_DONE - draw done
        //gx_states::draw_done(); //do on write ~ShizZy
        LOG_DEBUG(TGP, "BP-> PE_DONE");

	    if(g_bp_regs.mem[0x45] & 0x2) { // enable interrupt
		    //GX_PE_FINISH = 1;
            //FifoReset();
        }
        break;

    case 0x47: // PE_TOKEN
        //GX_PE_TOKEN_VALUE = (_value & 0xffff);
        LOG_DEBUG(TGP, "BP-> PE_TOKEN");
        break;

    case 0x48: // PE_TOKEN_INT
        //GX_PE_TOKEN_VALUE = (_value & 0xffff); 
        //GX_PE_TOKEN = 1; //seems to break mario
        LOG_DEBUG(TGP, "BP-> PE_TOKEN_INT");
        break;

    case 0x4f: // PE copy clear AR - set clear alpha and red components
    case 0x50: // PE copy clear GB - green and blue
        //gx_states::set_copyclearcolor();
        LOG_DEBUG(TGP, "BP-> PE_COPY_CLEAR_COLOR");
        break;

    case 0x51: // PE copy clear Z - 24-bit Z value
        //gx_states::set_copyclearz();
        LOG_DEBUG(TGP, "BP-> PE_COPY_CLEAR_X");
        break;

    case 0x52: // pe copy execute
        //gx_states::copy_efb();
        video_core::g_renderer->SwapBuffers();
        LOG_DEBUG(TGP, "BP-> PE_COPY_EFB");
        break;

    case 0x59: // Scissorbox Offset
        //gx_states::set_scissors();
        LOG_DEBUG(TGP, "BP-> Set scissors");
        break;

    case 0x64: // TX_LOADTLUT0
    case 0x65: // TX_LOADTLUT1
        //gx_states::load_tlut();
        LOG_DEBUG(TGP, "BP-> TX_LOADTLUTx");
        break;

    case 0x80: // TX_SETMODE0_I0 - Texture lookup and filtering mode
    case 0x81: // TX_SETMODE0_I1
    case 0x82: // TX_SETMODE0_I2
    case 0x83: // TX_SETMODE0_I3
    case 0xa0: // TX_SETMODE0_I4
    case 0xa1: // TX_SETMODE0_I5
    case 0xa2: // TX_SETMODE0_I6
    case 0xa3: // TX_SETMODE0_I7
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
        break;
    }
}

} // namespace