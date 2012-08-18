/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    fifo_player.h
 * @author  neobrain <neobrainx@gmail.com>
 * @date    2012-08-18
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

#include <vector>
#include "common.h"

#define FIFO_PLAYER_MAGIC_NUM   0xF1F0
#define FIFO_PLAYER_VERSION     0x0003

#define FPFE_REGISTER_WRITE 0
#define FPFE_MEMORY_UPDATE 1

namespace fifo_player {

#pragma pack(push, 4) // TODO: Change to 1?
struct FPFileHeader {
    u16 magic_num;
    u16 version;
    u32 size;
    u32 checksum;

    u32 num_frames;
    u32 num_elements;
    u32 num_raw_data_bytes;
    u32 frame_info_offset;
    u32 element_info_offset;
    u32 raw_data_offset;

    // TODO: Missing initial register state
};

struct FPFrameInfo {
    u32 base_element; // first element that belongs to this frame
    u32 num_elements;
};

// NOTE: Consecutive frame elements with the same type are assumed to have contiguously stored data! (Important to optimize playback of register writes)
struct FPElementInfo {
    enum Type {
        REGISTER_WRITE = 0xAB,
        MEMORY_UPDATE = 0xCD,
    };
    u8 type;
    u32 size; // TODO: Does this need to be u32?
    u32 offset; // offset to raw fifo data from base raw_data pointer
};

struct FPMemUpdateInfo
{
    u32 addr;
    u32 size;
    // followed by raw data
};

#pragma pack(pop)

struct FPFile
{
    FPFileHeader file_header;
    std::vector<FPFrameInfo> frame_info;
    std::vector<FPElementInfo> element_info;
    std::vector<u8> raw_data;
};


// Status query
bool IsRecording();

// configuration
void SetExpandDisplayLists(bool expand); // TODO

// recording
void StartRecording(char* filename);

void Write(u8* data, int size);

void MemUpdate(u32 address, u8* data, u32 size);

void FrameFinished();

void EndRecording();

// file handling
void Save(const char* filename, FPFile& in);

void Load(const char* filename, FPFile& out);

// playback
void PlayFile(char* filename);

} // namespace

#endif // VIDEO_CORE_FIFO_PLAYER_H_