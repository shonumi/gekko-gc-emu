/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    fifo_player.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-07-08
 * @brief   Saves and plays back GP data sent to FIFO
 *
 * @section LICENSE
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
 *  Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */


#include <SDL.h>

#include "fifo_player.h"
#include "video_core.h"
#include "fifo.h"
#include "core.h"

namespace fifo_player {

FILE* g_out_file_ptr = NULL;    ///< Output file pointer

void StartRecording(char* filename) {
    FifoPlayerFileHeader header;
    
    g_out_file_ptr = fopen(filename, "w");

    header.magic_num = FIFO_PLAYER_MAGIC_NUM;
    header.version = FIFO_PLAYER_VERSION;
    header.size = 0;    // TODO(ShizZy): ImplementMe
    header.checksum = 0;    // TODO(ShizZy): ImplementMe

    fwrite(&header, sizeof(FifoPlayerFileHeader), 1, g_out_file_ptr);

    LOG_NOTICE(TGP, "FIFO recording started");
}

void WriteDisplayList(u32 addr, u32 size) {
    // TODO(ShizZy): ImplementMe
}

void Write8(u8 data) {
    fwrite(&data, 1, 1, g_out_file_ptr);
}

void Write16(u16 data) {
    fwrite(&data, 2, 1, g_out_file_ptr);
}

void Write32(u32 data) {
    fwrite(&data, 4, 1, g_out_file_ptr);
}

void EndRecording() {
    fclose(g_out_file_ptr);
    LOG_NOTICE(TGP, "FIFO recording ended");
}

#define FIFO_PLAYBACK_SIZE  (32*1024*1024)
u8 fifo_buff[FIFO_PLAYBACK_SIZE];

void PlayFile(char* filename) {
    FifoPlayerFileHeader header;
    FILE* in_file_ptr = fopen(filename, "r");


    
    memset(fifo_buff, 0, FIFO_PLAYBACK_SIZE);

    if (in_file_ptr == NULL) {
        LOG_ERROR(TGP, "Failed to load FifoPlayer file %s", filename);
        return;
    }

    fread(&header, sizeof(header), 1, in_file_ptr);

    core::SetState(core::SYS_RUNNING);

    video_core::Init();
    video_core::Start();

    SDL_Delay(5000);

    fseek(in_file_ptr, sizeof(FifoPlayerFileHeader)  , SEEK_SET);
    fread(fifo_buff, FIFO_PLAYBACK_SIZE, 1, in_file_ptr);
    
    for (int i = 0; i < FIFO_PLAYBACK_SIZE; i++) {
        //u8 data;
        //u8 data = fgetc(in_file_ptr);
        //_ASSERT_MSG(TGP, bytes_read == 1, "WTF?! Failed to read 1 word...\n"); 
        //printf("%02x ", fifo_buff[i]);
        gp::FifoPush8(fifo_buff[i]);

    }

    SDL_Delay(5000);

    fclose(in_file_ptr);

    core::SetState(core::SYS_DIE);
}


} // namespace