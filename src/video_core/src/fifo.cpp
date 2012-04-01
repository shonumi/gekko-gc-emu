/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    fifo.cpp
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-08
 * \brief   Implementation of the GP FIFO
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

#include "SDL.h"
#include "SDL_thread.h"

#include "common.h"
#include "memory.h"
#include "core.h"

#include "video/opengl.h"

#include "video_core.h"
#include "vertex_loader.h"
#include "fifo.h"
#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

GPFuncPtr g_exec_op[0x20];  ///< GPOpcode table

u8 g_cur_cmd = 0;           ///< Current command to be executed
u8 g_cur_vat = 0;           ///< Current vertex attribute table

u8* volatile g_fifo_write_ptr;       ///< FIFO write location
u8* volatile g_fifo_read_ptr;        ///< FIFO read location
u8* volatile g_fifo_tail_ptr;        ///< Tail of the primary FIFO buffer
u8* volatile g_fifo_head_ptr;        ///< Head of the primary FIFO buffer
u8* volatile g_fifo_end_ptr;         ///< End of the primary FIFO buffer

u8 g_fifo_buffer[FIFO_SIZE];    ///< Primary FIFO buffer storage - Don't use directly

SDL_mutex*  g_fifo_synch_mutex;
SDL_mutex*  g_fifo_write_ptr_mutex;

bool volatile g_reset_fifo = false;


/********************************* TEST CODE ***********************************/

// retrieve the size of the next vertex in the fifo
int GetVertexSize(u8 vat)
{
    u16 size = 0;
    u8 count = 0;
    int i = 0;

    if(VCD_PMIDX) {
        size+=1;
    }


    for(; i < 8; i++) {
        size += VCD_TMIDX(i);
    }

    switch(VCD_POS)
    {	
    case 1: // direct
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
    {	
        case 1: size+=kVertexNormalSize[((VAT_NRMCNT << 3) | VAT_NRMFMT)]; break; // direct
        case 2: size+=1; break; // index 8
        case 3: size+=2; break; // index 16
    }

    switch(VCD_COL0)
    {	
    case 1: // direct
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
    {	
    case 1: // direct
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

    return size;
}

/******************************************************************************/

/// Called by CPU core to catch up
void EMU_FASTCALL FifoSynchronize() {
    if (g_fifo_write_ptr > g_fifo_tail_ptr) {
        SDL_mutexP(g_fifo_synch_mutex);
        g_reset_fifo = true;
        SDL_mutexV(g_fifo_synch_mutex);

        while (g_reset_fifo) {
        }
    }
}

/// Called at end of frame to reset FIFO
void FifoReset() {
    g_fifo_write_ptr    = g_fifo_buffer;
    g_fifo_read_ptr     = g_fifo_buffer;
    g_fifo_end_ptr      = &g_fifo_buffer[FIFO_SIZE-1];
}

bool FifoNextCommandReady() {
    static int last_required_size = -1;
    u8 cmd = FIFO_GET8(0);
    u8 vat = cmd & 0x7;

    // We haven't started (at the beginning), or something went terrible wrong...
    if (g_fifo_read_ptr == g_fifo_write_ptr) {
        return false;
    }
    // Otherwise, read_ptr < write_ptr:
    uintptr_t bytes_in_fifo = g_fifo_write_ptr - g_fifo_read_ptr;

    // Last size still right...
	if ((last_required_size != -1) && (last_required_size > (s32)bytes_in_fifo)) {
	    return false;
    }

    // Determine opcode size
    switch(GP_OPMASK(cmd))
    {
    case 0: // NOP
        last_required_size = -1;
        return true;
    
    case 1: // LOAD_CP
        if(bytes_in_fifo >= 6) {
            last_required_size = -1;
            return true;
        }
        last_required_size = 6;
        break; 
    
    case 2: // LOAD XF
        if (bytes_in_fifo >= 5) { // if header is present
            u16 size = (4 * (FIFO_GET16(1) & 0xf) + 1);
            size += 5;
            if(bytes_in_fifo >= size)
            {
                last_required_size = -1;
                return true;
            }
            else {
                last_required_size = size;
            }
        } else {
            last_required_size = 5;
        }
        break;

    case 4: // LOAD IDX A
    case 5: // " B
    case 6: // " C 
    case 7: // " D
        if (bytes_in_fifo >= 5) {
            last_required_size = -1;
            return true;
        }
        last_required_size = 5;
        break; 

    case 8: // CALL_DL
        if (bytes_in_fifo >= 9) {
            last_required_size = -1;
            return true;
        }
        last_required_size = 9;
        break; 

    case 9: // INVALID_VTX_CACHE
        last_required_size = -1;
        return true;

    case 0xC: // LOAD BP
        if (bytes_in_fifo >= 5) {
            last_required_size = -1;
            return true;
        }
        last_required_size = 5;
        return false; 

    default: 		
        if(cmd & 0x80) { // Draw command
            if(bytes_in_fifo >= 3) {    // See if header exists
                u16 numverts = FIFO_GET16(1);
                u16 vertsize = GetVertexSize(vat);
                u16 size = 3;

                size += numverts * vertsize;
                if(bytes_in_fifo >= size) {
                    last_required_size = -1;
                    return true;
                } else {
                    last_required_size = size;
                }
            } else {
                last_required_size = 3;
            }
        }
        return false;
    }
    return false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor Instructions

/// unknown command
GP_OPCODE(UNKNOWN) {
	LOG_ERROR(TGP, "GP Fifo has been corrupted.");
}

/// nop - do nothing
GP_OPCODE(NOP) {
    LOG_DEBUG(TGP, "Called NOP");  
}

// load cp register with data
GP_OPCODE(LOAD_CP_REG) {
	u8 addr = FIFO_POP8();
	u32 data = FIFO_POP32();
	CPRegisterWrite(addr, data);
}

/// load xf register with data
GP_OPCODE(LOAD_XF_REG) {
	u32 temp    = FIFO_POP32();
	u16 length  = (temp >> 16) + 1;
    u16 addr    = temp & 0xFFFF;
    u32 regs[64];

    //ASSERT_T(length <= 64, "GP core fu***d up....");

	for(int i = 0; i < length; i++) {
        regs[i] = FIFO_POP32();
    }

    XFRegisterWrite(length, addr, regs);
    LOG_DEBUG(TGP, "Called LOAD_XF_REG: length = %d addr = %04x", length, addr);
}

/// load xf register with data indexed A
GP_OPCODE(LOAD_IDX_A) {
	u8 length;
	u16 index = FIFO_POP16();
    u16 data = FIFO_POP16();
    u16 addr;
	length = (data >> 12) + 1;
	addr = data & 0xfff;
	//GX_XFLoadIndexed(GX_IDX_A, index, length, addr);
    LOG_DEBUG(TGP, "Called LOAD_IDX_A");
}

/// load xf register with data indexed B
GP_OPCODE(LOAD_IDX_B) {
	u8 length;
	u16 index = FIFO_POP16();
    u16 data = FIFO_POP16();
    u16 addr;
	length = (data >> 12) + 1;
	addr = data & 0xfff;
	//GX_XFLoadIndexed(GX_IDX_B, index, length, addr);
    LOG_DEBUG(TGP, "Called LOAD_IDX_B");
}

/// load xf register with data indexed C
GP_OPCODE(LOAD_IDX_C) {
	u8 length;
	u16 index = FIFO_POP16();
    u16 data = FIFO_POP16();
    u16 addr;
	length = (data >> 12) + 1;
	addr = data & 0xfff;
	//GX_XFLoadIndexed(GX_IDX_C, index, length, addr);
    LOG_DEBUG(TGP, "Called LOAD_IDX_C");
}

/// load xf register with data indexed D
GP_OPCODE(LOAD_IDX_D) {
	u8 length;
	u16 index = FIFO_POP16();
    u16 data = FIFO_POP16();
    u16 addr;
	length = (data >> 12) + 1;
	addr = data & 0xfff;
	//GX_XFLoadIndexed(GX_IDX_D, index, length, addr);
    LOG_DEBUG(TGP, "Called LOAD_IDX_D");
}

/// call a display list
GP_OPCODE(CALL_DISPLAYLIST) {
	u32 addr = FIFO_POP32();
    u32 size = FIFO_POP32();
	//call_displaylist(addr, size);
    LOG_DEBUG(TGP, "Called CALL_DISPLAYLIST");
}

/// invalidate vertex cache
GP_OPCODE(INVALIDATE_VERTEX_CACHE) {
    LOG_DEBUG(TGP, "Called INVALIDATE_VERTEX_CACHE");
}

/// load bp register with data
GP_OPCODE(LOAD_BP_REG) {
    u32 data = FIFO_POP32();
	BPRegisterWrite(data >> 24, data & 0x00FFFFFF);
}

/// draw a primitive - quads
GP_OPCODE(DRAW_QUADS) {
	u16 count = FIFO_POP16();
	DecodePrimitive(GX_QUADS, count, g_cur_vat);
    LOG_DEBUG(TGP, "Called DRAW_QUADS");
}

/// draw a primitive - triangles
GP_OPCODE(DRAW_TRIANGLES) {
	u16 count = FIFO_POP16();
    DecodePrimitive(GX_TRIANGLES, count, g_cur_vat);
	//gx_vertex::draw_primitive(_gxlist, , count, vat);
    LOG_DEBUG(TGP, "Called DRAW_TRIANGLES");
}

/// draw a primitive - trianglestrip
GP_OPCODE(DRAW_TRIANGLESTRIP) {
	u16 count = FIFO_POP16();
	//gx_vertex::draw_primitive(_gxlist, GL_TRIANGLE_STRIP, count, vat);
    LOG_DEBUG(TGP, "Called DRAW_TRIANGLESTRIP");
}

/// draw a primitive - trianglefan
GP_OPCODE(DRAW_TRIANGLEFAN) {
	u16 count = FIFO_POP16();
	//gx_vertex::draw_primitive(_gxlist, GL_TRIANGLE_FAN, count, vat);
    LOG_DEBUG(TGP, "Called DRAW_TRIANGLEFAN");
}

/// draw a primitive - lines
GP_OPCODE(DRAW_LINES) {
	u16 count = FIFO_POP16();
	//gx_vertex::draw_primitive(_gxlist, GL_LINES, count, vat);
    LOG_DEBUG(TGP, "Called DRAW_LINES");
}

/// draw a primitive - linestrip
GP_OPCODE(DRAW_LINESTRIP) {
	u16 count = FIFO_POP16();
	//gx_vertex::draw_primitive(_gxlist, GL_LINE_STRIP, count, vat);
    LOG_DEBUG(TGP, "Called DRAW_LINESTRIP");
}

/// draw a primitive - points
GP_OPCODE(DRAW_POINTS) {
	u16 count = FIFO_POP16();
	//gx_vertex::draw_primitive(_gxlist, GL_POINTS, count, vat);
    LOG_DEBUG(TGP, "Called DRAW_POINTS");
}

/// Thread that sits and waits to decode the FIFO contents
int DecodeThread(void *unused) {
    LOG_NOTICE(TGP, "Thread starting...");
    
    while (core::SYS_RUNNING == core::g_state) {
        int bytes_in_fifo = g_fifo_write_ptr - g_fifo_read_ptr;

        if (bytes_in_fifo < 1) {
            if (!g_reset_fifo) {
                continue;
            }
        }

        // Synchronize the CPU<-->FIFO
        SDL_mutexP(g_fifo_synch_mutex);
        if (g_reset_fifo) {
            // Lock writes from CPU to FIFO, reset FIFO to beginning
            //SDL_mutexP(g_fifo_write_ptr_mutex);
            bytes_in_fifo = g_fifo_write_ptr - g_fifo_read_ptr;
            g_fifo_write_ptr = g_fifo_buffer + bytes_in_fifo;
            memcpy(g_fifo_buffer, g_fifo_read_ptr, (size_t)bytes_in_fifo);
            //SDL_mutexV(g_fifo_write_ptr_mutex);

            // Move FIFO to beginning
            g_fifo_read_ptr = g_fifo_buffer;

            g_reset_fifo = false;
        }
        SDL_mutexV(g_fifo_synch_mutex);

        // Get the next GP opcode and decode it
        if (FifoNextCommandReady()) {
            g_cur_cmd = FIFO_POP8();
            g_cur_vat = g_cur_cmd & 0x7;
            g_exec_op[GP_OPMASK(g_cur_cmd)]();
        }
    }
    LOG_NOTICE(TGP, "Thread closing...");
    return E_OK;
}

/// Initialize GP FIFO
void FifoInit() {
    // Synchronization mutex
    g_fifo_synch_mutex = SDL_CreateMutex();
    g_fifo_write_ptr_mutex = SDL_CreateMutex();

    // FIFO pointers
	///g_fifo_buffer = (u8*)malloc(FIFO_SIZE);
    FifoReset();
    g_fifo_tail_ptr = g_fifo_buffer + FIFO_TAIL_END;
    g_fifo_head_ptr = g_fifo_buffer + FIFO_HEAD_END;

    // Zero FIFO memory
	memset(g_fifo_buffer, 0, FIFO_SIZE);

	// init op table
	for(int i = 0; i < 0x20; i++) {
		GP_SETOP(i, GPOPCODE_UNKNOWN);
    }

	// create op table
	GP_SETOP(GP_OPMASK(GP_NOP), GPOPCODE_NOP);	
	GP_SETOP(GP_OPMASK(GP_LOAD_CP_REG), GPOPCODE_LOAD_CP_REG);				
	GP_SETOP(GP_OPMASK(GP_LOAD_XF_REG), GPOPCODE_LOAD_XF_REG);
	GP_SETOP(GP_OPMASK(GP_LOAD_IDX_A), GPOPCODE_LOAD_IDX_A);
	GP_SETOP(GP_OPMASK(GP_LOAD_IDX_B), GPOPCODE_LOAD_IDX_B);
	GP_SETOP(GP_OPMASK(GP_LOAD_IDX_C), GPOPCODE_LOAD_IDX_C);
	GP_SETOP(GP_OPMASK(GP_LOAD_IDX_D), GPOPCODE_LOAD_IDX_D);
	GP_SETOP(GP_OPMASK(GP_CALL_DISPLAYLIST), GPOPCODE_CALL_DISPLAYLIST);
	GP_SETOP(GP_OPMASK(GP_INVALIDATE_VERTEX_CACHE), GPOPCODE_INVALIDATE_VERTEX_CACHE);
	GP_SETOP(GP_OPMASK(GP_LOAD_BP_REG), GPOPCODE_LOAD_BP_REG);	
	GP_SETOP(GP_OPMASK(GP_DRAW_QUADS), GPOPCODE_DRAW_QUADS);
	GP_SETOP(GP_OPMASK(GP_DRAW_TRIANGLES), GPOPCODE_DRAW_TRIANGLES);
	GP_SETOP(GP_OPMASK(GP_DRAW_TRIANGLESTRIP), GPOPCODE_DRAW_TRIANGLESTRIP);
	GP_SETOP(GP_OPMASK(GP_DRAW_TRIANGLEFAN), GPOPCODE_DRAW_TRIANGLEFAN);
	GP_SETOP(GP_OPMASK(GP_DRAW_LINES), GPOPCODE_DRAW_LINES);
	GP_SETOP(GP_OPMASK(GP_DRAW_LINESTRIP), GPOPCODE_DRAW_LINESTRIP);
	GP_SETOP(GP_OPMASK(GP_DRAW_POINTS), GPOPCODE_DRAW_POINTS);

    VertexLoaderInit();
    BPInit();
    CPInit();
    XFInit();
}

/// Shutdown GP FIFO
void FifoShutdown() {
    SDL_DestroyMutex(g_fifo_synch_mutex);
    SDL_DestroyMutex(g_fifo_write_ptr_mutex);
    VertexLoaderShutdown();
}


} // namespace
