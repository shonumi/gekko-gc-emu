// hw_gx.cpp
// (c) 2005,2008 Gekko Team / Wiimu Project

#include "common.h"
#include "config.h"

#include "memory.h"
#include "hw.h"
#include "hw_gx.h"
#include "hw_pi.h"
#include "hw_pe.h"

#include "video_core.h"
#include "bp_mem.h" 
#include "fifo_player.h"

#include "fifo.h"

////////////////////////////////////////////////////////////////////////////////
// GX - Graphics Processor
////////////////////////////////////////////////////////////////////////////////

// Desc: Read/Write from/to GX Hardware
//

void EMU_FASTCALL GX_Fifo_Write8(u32 addr, u32 data)
{
    gp::g_fifo_buffer[gp::g_fifo_write_ptr++] = data;
    if (!common::g_config->enable_multicore()) {
        gp::Fifo_DecodeCommand();
    }
}

void EMU_FASTCALL GX_Fifo_Write16(u32 addr, u32 data)
{
    *(u16*)(gp::g_fifo_buffer + gp::g_fifo_write_ptr) = BSWAP16(data);
    gp::g_fifo_write_ptr += 2;
    if (!common::g_config->enable_multicore()) {
        gp::Fifo_DecodeCommand();
    }
}

void EMU_FASTCALL GX_Fifo_Write32(u32 addr, u32 data) {
    static u8 cmd = FIFO_GET8(0);
    
    *(u32*)(gp::g_fifo_buffer + gp::g_fifo_write_ptr) = BSWAP32(data);
    gp::g_fifo_write_ptr += 4;
    
    /*if ((data == 0x45000002) && ((cmd & 0xf8) == 0x60)) {

        while (!gp::g_frame_finished) {
        }
        gp::g_frame_finished = 0;
        
        GX_PE_FINISH = 1;
        PE_Update();
    }*/
    if (!common::g_config->enable_multicore()) {
        gp::Fifo_DecodeCommand();
    }
}


u16 EMU_FASTCALL GX_Fifo_Read16(u32 addr)
{
	return 0;
}

u32 EMU_FASTCALL GX_Fifo_Read32(u32 addr)
{
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

// Desc: Open GX Hardware
//

void GX_Open() {
	LOG_NOTICE(TGP, "initialized ok");
#ifdef USE_FIFO_RECORDING
    fifo_player::StartRecording("fifo_recording.gfp");
#endif
}

////////////////////////////////////////////////////////////////////////////////
