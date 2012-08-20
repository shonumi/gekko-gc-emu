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

#include <stdio.h>
#include <vector>

#include "memory.h"

#include "fifo_player.h"
#include "video_core.h"
#include "fifo.h"
#include "core.h"
#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

namespace fifo_player {

bool is_recording = false;

// TODO: Move these into a FPFile struct ASAP!
FPFile current_file;

FPFrameInfo* current_frame_info = NULL;

bool IsRecording()
{
    return is_recording;
}

// NOTE: Should be called from GPU thread to make sure register states are consistent!
void StartRecording()
{
    memset(&current_file.file_header, 0, sizeof(FPFileHeader));
    current_file.frame_info.clear();
    current_file.element_info.clear();
    current_file.raw_data.clear();

    current_file.file_header.magic_num = FIFO_PLAYER_MAGIC_NUM;
    current_file.file_header.version = FIFO_PLAYER_VERSION;
    // TODO: size and checksum

    current_file.frame_info.resize(1);
    current_frame_info = &current_file.frame_info.back();

    // TODO: Record initial mem state => Mem_RAM

    // TODO: Check if this works correctly
    current_file.file_header.initial_bpmem_data_offset = current_file.raw_data.size();
    current_file.raw_data.insert(current_file.raw_data.end(), (u8*)&gp::g_bp_regs, (u8*)(&gp::g_bp_regs + 1));
    current_file.file_header.initial_cpmem_data_offset = current_file.raw_data.size();
    current_file.raw_data.insert(current_file.raw_data.end(), (u8*)&gp::g_cp_regs, (u8*)(&gp::g_cp_regs + 1));
    current_file.file_header.initial_xfmem_data_offset = current_file.raw_data.size();
    current_file.raw_data.insert(current_file.raw_data.end(), (u8*)&gp::g_xf_regs, (u8*)(&gp::g_xf_regs + 1));

    is_recording = true;
    LOG_NOTICE(TGP, "FIFO recording started");
}

void Write(u8* data, int size)
{
    FPElementInfo element;
    element.type = FPElementInfo::REGISTER_WRITE;
    element.size = size;
    element.offset = current_file.raw_data.size();
    current_file.element_info.push_back(element);

    current_file.raw_data.insert(current_file.raw_data.end(), data, data + size);
}

void MemUpdate(u32 address, u8* data, u32 size)
{
    FPElementInfo element;
    element.type = FPElementInfo::MEMORY_UPDATE;
    element.size = sizeof(FPMemUpdateInfo) + size;
    element.offset = current_file.raw_data.size();
    current_file.element_info.push_back(element);

    FPMemUpdateInfo update_info;
    update_info.addr = address;
    update_info.size = size;
    current_file.raw_data.insert(current_file.raw_data.end(), (u8*)&update_info, (u8*)(&update_info+1));
    current_file.raw_data.insert(current_file.raw_data.end(), data, data + size);
}

void FrameFinished()
{
    current_frame_info->num_elements = current_file.element_info.size() - current_frame_info->base_element;

    current_file.frame_info.resize(current_file.frame_info.size()+1);
    current_frame_info = &current_file.frame_info.back();

    current_frame_info->base_element = current_file.element_info.size();
}

const FPFile& EndRecording()
{
    FrameFinished();
    while (current_file.frame_info.back().base_element == current_file.element_info.size() && !current_file.frame_info.empty())
        current_file.frame_info.pop_back();

    current_file.file_header.num_frames = current_file.frame_info.size();
    current_file.file_header.num_elements = current_file.element_info.size();
    current_file.file_header.num_raw_data_bytes = current_file.raw_data.size();
    current_file.file_header.frame_info_offset = sizeof(FPFileHeader);
    current_file.file_header.element_info_offset = current_file.file_header.frame_info_offset + current_file.file_header.num_frames * sizeof(FPFrameInfo);
    current_file.file_header.raw_data_offset = current_file.file_header.element_info_offset + current_file.file_header.num_elements * sizeof(FPElementInfo);

    is_recording = false;
    LOG_NOTICE(TGP, "FIFO recording ended: %d frames\n", current_file.frame_info.size());

    return current_file;
}

void Save(const char* filename, FPFile& in)
{
    // TODO: Error checking...
    FILE* file = fopen(filename, "wb");

    fwrite(&in.file_header, sizeof(FPFileHeader), 1, file);
    fwrite(&(*in.frame_info.begin()), in.file_header.num_frames * sizeof(FPFrameInfo), 1, file);
    fwrite(&(*in.element_info.begin()), in.file_header.num_elements * sizeof(FPElementInfo), 1, file);
    fwrite(&(*in.raw_data.begin()), in.raw_data.size(), 1, file);

    fclose(file);
}

void Load(const char* filename, FPFile& out)
{
    // TODO: Error checking...
    FILE* file = fopen(filename, "rb");
    int objects_read = 0;

    objects_read = fread(&out.file_header, sizeof(FPFileHeader), 1, file);
    if (objects_read != 1)
    {
        // TODO: Error message
        return;
    }
    if (out.file_header.magic_num != FIFO_PLAYER_MAGIC_NUM)
    {
        // TODO: Error message
        return;
    }
    if (out.file_header.version != FIFO_PLAYER_VERSION)
    {
        // TODO: Error message
        return;
    }

    out.frame_info.resize(out.file_header.num_frames);
    fseek(file, out.file_header.frame_info_offset, SEEK_SET);
    objects_read = fread(&(*out.frame_info.begin()), out.file_header.num_frames * sizeof(FPFrameInfo), 1, file);
    if (objects_read != 1)
    {
        // TODO: Error message
        return;
    }

    out.element_info.resize(out.file_header.num_elements);
    fseek(file, out.file_header.element_info_offset, SEEK_SET);
    objects_read = fread(&(*out.element_info.begin()), out.file_header.num_elements * sizeof(FPElementInfo), 1, file);
    if (objects_read != 1)
    {
        // TODO: Error message
        return;
    }

    out.raw_data.resize(out.file_header.num_raw_data_bytes);
    fseek(file, out.file_header.raw_data_offset, SEEK_SET);
    objects_read = fread(&(*out.raw_data.begin()), out.file_header.num_raw_data_bytes * sizeof(u8), 1, file);
    if (objects_read != 1)
    {
        // TODO: Error message
        return;
    }

    fclose(file);
}

void PlayFile(FPFile& in)
{
    gp::BPMemory* bpmem = (gp::BPMemory*)&(*(in.raw_data.begin() + in.file_header.initial_bpmem_data_offset));
    for (unsigned int i = 0; i < sizeof(gp::BPMemory) / sizeof(u32); ++i)
    {
        // TODO: This is dangerous since it e.g. triggers EFB copy requests!
        gp::FifoPush8(GP_LOAD_BP_REG);
        gp::FifoPush32((i << 24) | (bpmem->mem[i] & 0x00FFFFFF));
    }


    gp::CPMemory* cpmem = (gp::CPMemory*)&(*(in.raw_data.begin() + in.file_header.initial_cpmem_data_offset));
    for (unsigned int i = 0; i < sizeof(gp::CPMemory) / sizeof(u32); ++i)
    {
        gp::FifoPush8(GP_LOAD_CP_REG);
        gp::FifoPush8(i << 24);
        gp::FifoPush32(cpmem->mem[i]);
    }

    gp::XFMemory* xfmem = (gp::XFMemory*)&(*(in.raw_data.begin() + in.file_header.initial_xfmem_data_offset));
    // TODO: Push XF regs

    // TODO: Loop over all frames but wait until the last frame has been processed before pushing the first one again
    std::vector<FPFrameInfo>::iterator frame;
    for (frame = in.frame_info.begin(); frame != in.frame_info.end(); ++frame)
    {
        std::vector<FPElementInfo>::iterator element;
        for (element = in.element_info.begin() + frame->base_element; element != in.element_info.begin() + frame->base_element + frame->num_elements; ++element)
        {
            switch (element->type)
            {
                case FPElementInfo::REGISTER_WRITE:
                {
                    std::vector<u8>::iterator byte;
                    for (byte = in.raw_data.begin() + element->offset; byte != in.raw_data.begin() + element->offset + element->size; ++byte)
                        gp::FifoPush8(*byte);

                    break;
                }

                case FPElementInfo::MEMORY_UPDATE:
                {
                    // TODO: Wait for GPU thread to catch up before doing this
                    FPMemUpdateInfo* update_info = (FPMemUpdateInfo*)&(*(in.raw_data.begin() + element->offset));
                    memcpy(&Mem_RAM[update_info->addr & RAM_MASK], &*(in.raw_data.begin() + element->offset + 2), update_info->size);

                    break;
                }
            }
        }
        // TODO: Flush WGP once we have accurate fifo emulation
    }
}


} // namespace
