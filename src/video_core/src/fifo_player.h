/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    fifo_player.h
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
 
#ifndef VIDEO_CORE_FIFO_PLAYER_H_
#define VIDEO_CORE_FIFO_PLAYER_H_

#include "common.h"

#define FIFO_PLAYER_MAGIC_NUM   0xF1F0
#define FIFO_PLAYER_VERSION     0x0001

namespace fifo_player {

struct FifoPlayerFileHeader {
    u16 magic_num;
    u16 version;
    u32 size;
    u32 checksum;
};

void StartRecording(char* filename);

void WriteDisplayList(u32 addr, u32 size);

void Write8(u8 data);

void Write16(u16 data);

void Write32(u32 data);

void EndRecording();

void PlayFile(char* filename);

} // namespace

#endif // VIDEO_CORE_FIFO_PLAYER_H_