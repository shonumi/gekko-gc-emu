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
#define FIFO_PLAYER_VERSION     0x0004

#define FPFE_REGISTER_WRITE 0
#define FPFE_MEMORY_UPDATE 1

namespace fifo_player {

#pragma pack(push, 4) // TODO: Change to 1?
struct FPFileHeader {
    u16 magic_num;                  // 0x0
    u16 version;                    // 0x2
    u32 size;                       // 0x4
    u32 checksum;                   // 0x8

    u32 num_frames;                 // 0xC
    u32 num_elements;               // 0x10
    u32 num_raw_data_bytes;         // 0x14
    u32 frame_info_offset;          // 0x18
    u32 element_info_offset;        // 0x1C
    u32 raw_data_offset;            // 0x20

    u32 initial_bpmem_data_offset;  // 0x24
    u32 initial_cpmem_data_offset;  // 0x28
    u32 initial_xfmem_data_offset;  // 0x2C
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
    std::vector<u8> raw_data; // TODO: Should split this into initial state and actual raw data
};


// Status query
bool IsRecording();

// configuration
void SetExpandDisplayLists(bool expand); // TODO

// recording
void StartRecording();

void Write(u8* data, int size);

void MemUpdate(u32 address, u8* data, u32 size);

void FrameFinished();

const FPFile& EndRecording();

// file handling
void Save(const char* filename, FPFile& in);

void Load(const char* filename, FPFile& out);

// playback
void PlayFile(FPFile& in);

} // namespace

#endif // VIDEO_CORE_FIFO_PLAYER_H_