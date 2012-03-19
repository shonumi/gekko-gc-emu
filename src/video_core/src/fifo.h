/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    fifo.h
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

#ifndef VIDEO_CORE_FIFO_H_
#define VIDEO_CORE_FIFO_H_

#include "common.h"

typedef void(*GPFuncPtr)(void); ///< Function pointer GP opcodes

// GP opcode decoding
#define GP_OPMASK(n)            ((n >> 3) & 0x1f)
#define GP_OPCODE(name)         void EMU_FASTCALL GPOPCODE_##name()
#define GP_SETOP(n, op)         g_exec_op[n] = (GPFuncPtr)op

// FIFO information
#define FIFO_SIZE           0x10000*2                 // 64kb - good size
#define FIFO_HEAD_END       0x4000                  // First 16kb of FIFO
#define FIFO_TAIL_END       (FIFO_SIZE - FIFO_HEAD_END)    // Last 16kb of FIFO... Reset to beginning ASAP
#define FIFO_MASK           (FIFO_SIZE - 1)         // mask

// FIFO write access
#define FIFO_PUSH8(data)    *(gp::g_fifo_write_ptr) = data; gp::g_fifo_write_ptr++
#define FIFO_PUSH16(data)   *(u16*)(gp::g_fifo_write_ptr) = data; gp::g_fifo_write_ptr+=2
#define FIFO_PUSH32(data)   *(u32*)(gp::g_fifo_write_ptr) = data; gp::g_fifo_write_ptr+=4

/// Pop byte off FIFO
#define FIFO_POP8()         *(gp::g_fifo_read_ptr); gp::g_fifo_read_ptr++
/// Pop half off of FIFO
#define FIFO_POP16()        *((u16*)(gp::g_fifo_read_ptr)); gp::g_fifo_read_ptr+=2
/// Pop word off of FIFO
#define FIFO_POP32()        *((u32*)(gp::g_fifo_read_ptr)); gp::g_fifo_read_ptr+=4

/// Get last byte from FIFO
#define FIFO_GET8(ofs)      *(gp::g_fifo_read_ptr + ofs)
/// Get last half from FIFO
#define FIFO_GET16(ofs)     *((u16*)(gp::g_fifo_read_ptr + ofs))
/// Get last word from FIFO
#define FIFO_GET32(ofs)     *((u32*)(gp::g_fifo_read_ptr + ofs))

/// GP message/opcode types
typedef enum {
    GP_NOP                      = 0x00,
    GP_LOAD_CP_REG              = 0x08,
    GP_LOAD_XF_REG              = 0x10,
    GP_LOAD_IDX_A               = 0x20,
    GP_LOAD_IDX_B               = 0x28,
    GP_LOAD_IDX_C               = 0x30,
    GP_LOAD_IDX_D               = 0x38,
    GP_CALL_DISPLAYLIST         = 0x40,
    GP_INVALIDATE_VERTEX_CACHE  = 0x48,
    GP_LOAD_BP_REG              = 0x60,
    GP_DRAW_QUADS               = 0x80,
    GP_DRAW_TRIANGLES           = 0x90,
    GP_DRAW_TRIANGLESTRIP       = 0x98,
    GP_DRAW_TRIANGLEFAN         = 0xa0,
    GP_DRAW_LINES               = 0xa8,
    GP_DRAW_LINESTRIP           = 0xb0,
    GP_DRAW_POINTS              = 0xb8
} GPOpcodeType;

/// GX API primitive types
typedef enum {
    GX_POINTS        = 0xb8,
    GX_LINES         = 0xa8,
    GX_LINESTRIP     = 0xb0,
    GX_TRIANGLES     = 0x90,
    GX_TRIANGLESTRIP = 0x98,
    GX_TRIANGLEFAN   = 0xa0,
    GX_QUADS         = 0x80
} GXPrimitive;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

extern u8* volatile g_fifo_write_ptr;    ///< FIFO write location
extern u8* volatile g_fifo_read_ptr;     ///< FIFO read location
extern SDL_mutex*  g_fifo_write_ptr_mutex;  ///< Mutex for accessing g_fifo_write_ptr

/// Push 8-bit byte into the FIFO
static inline void FifoPush8(u8 data) {
 //   SDL_mutexP(g_fifo_write_ptr_mutex);
    *g_fifo_write_ptr = data;
    g_fifo_write_ptr+=1;
   // SDL_mutexV(g_fifo_write_ptr_mutex); 
}

/// Push 16-bit halfword into the FIFO
static inline void FifoPush16(u16 data) {
  //  SDL_mutexP(g_fifo_write_ptr_mutex);
    *(u16*)gp::g_fifo_write_ptr = data;
    g_fifo_write_ptr+=2;
   // SDL_mutexV(g_fifo_write_ptr_mutex); 
}

/// Push 32-bit word into the FIFO
static inline void FifoPush32(u32 data) {
   // SDL_mutexP(g_fifo_write_ptr_mutex);
    *(u32*)gp::g_fifo_write_ptr = data;
    g_fifo_write_ptr+=4;
   // SDL_mutexV(g_fifo_write_ptr_mutex); 
}

/// Called by CPU core to catch up
void EMU_FASTCALL FifoSynchronize();

/// Thread that sits and waits to decode the FIFO contents
int DecodeThread(void *unused);

/// Called at end of frame to reset FIFO
void FifoReset();

/// Initialize GP FIFO
void FifoInit();

/// Shutdown GP FIFO
void FifoShutdown();

} // namespace

#endif // VIDEO_CORE_FIFO_H_