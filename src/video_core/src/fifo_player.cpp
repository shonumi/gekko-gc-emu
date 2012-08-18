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

#include "fifo_player.h"
#include "video_core.h"
#include "fifo.h"
#include "core.h"

namespace fifo_player {

bool is_recording = false;

FPFileHeader file_header;
std::vector<FPFrameInfo> frame_info;
std::vector<FPElementInfo> element_info;
std::vector<u8> raw_data;

FPFrameInfo* current_frame_info = NULL;

bool IsRecording()
{
    return is_recording;
}

void StartRecording(char* filename) {
    memset(&file_header, 0, sizeof(file_header));
    frame_info.clear();
    element_info.clear();

    file_header.magic_num = FIFO_PLAYER_MAGIC_NUM;
    file_header.version = FIFO_PLAYER_VERSION;
    // TODO: size and checksum

    frame_info.resize(1);
    current_frame_info = &frame_info.back();

    is_recording = true;
    LOG_NOTICE(TGP, "FIFO recording started");
}

void Write(u8* data, int size)
{
    FPElementInfo element;
    element.type = FPElementInfo::REGISTER_WRITE;
    element.size = size;
    element.offset = raw_data.size();
    element_info.push_back(element);

    raw_data.insert(raw_data.end(), data, data + size);
}

void MemUpdate(u32 address, u8* data, u32 size)
{
    FPElementInfo element;
    element.type = FPElementInfo::MEMORY_UPDATE;
    element.size = sizeof(FPMemUpdateInfo) + size;
    element.offset = raw_data.size();
    element_info.push_back(element);

    raw_data[element.offset  ] = address;
    raw_data[element.offset+1] = size;
    raw_data.insert(raw_data.end(), data, data + size);
}

void FrameFinished()
{
    current_frame_info->num_elements = element_info.size() - current_frame_info->base_element;

    frame_info.resize(frame_info.size()+1);
    current_frame_info = &frame_info.back();

    current_frame_info->base_element = element_info.size();
}

void EndRecording() {
    FrameFinished();
    while (frame_info.back().base_element == element_info.size() && !frame_info.empty())
        frame_info.pop_back();

    file_header.num_frames = frame_info.size();
    file_header.num_elements = element_info.size();
    file_header.num_raw_data_bytes = raw_data.size();
    file_header.frame_info_offset = sizeof(FPFileHeader);
    file_header.element_info_offset = file_header.frame_info_offset + file_header.num_frames * sizeof(FPFrameInfo);
    file_header.raw_data_offset = file_header.element_info_offset + file_header.num_elements * sizeof(FPElementInfo);

    is_recording = false;
    LOG_NOTICE(TGP, "FIFO recording ended: %d frames\n", frame_info.size());
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

//#define FIFO_PLAYBACK_SIZE  (32*1024*1024)
//u8 fifo_buff[FIFO_PLAYBACK_SIZE];

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
    // TODO: fseek
    objects_read = fread(&(*out.frame_info.begin()), out.file_header.num_frames * sizeof(FPFrameInfo), 1, file);
    if (objects_read != 1)
    {
        // TODO: Error message
        return;
    }

    out.element_info.resize(out.file_header.num_elements);
    // TODO: fseek
    objects_read = fread(&(*out.element_info.begin()), out.file_header.num_elements * sizeof(FPElementInfo), 1, file);
    if (objects_read != 1)
    {
        // TODO: Error message
        return;
    }

    out.raw_data.resize(out.file_header.num_raw_data_bytes);
    // TODO: fseek
    objects_read = fread(&(*out.raw_data.begin()), out.file_header.num_raw_data_bytes * sizeof(u8), 1, file);
    if (objects_read != 1)
    {
        // TODO: Error message
        return;
    }

    fclose(file);
}

void PlayFile(char* filename) {
/*    FifoPlayerFileHeader header;
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

    core::SetState(core::SYS_DIE);*/
}


} // namespace
