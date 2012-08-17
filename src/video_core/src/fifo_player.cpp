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
struct FPFrameWithElementsInfo {
    FPFrameInfo frame_info;
    std::vector<FPElementInfo> element_info;
};
std::vector<FPFrameWithElementsInfo> frame_info;
std::vector<u8> raw_data;

FPFrameWithElementsInfo* current_frame_info = NULL;

bool IsRecording()
{
    return is_recording;
}

void StartRecording(char* filename) {
    memset(&file_header, 0, sizeof(file_header));
    frame_info.clear();

    file_header.magic_num = FIFO_PLAYER_MAGIC_NUM;
    file_header.version = FIFO_PLAYER_VERSION;
    file_header.size = 0; // TODO
    file_header.checksum = 0; // TODO

    frame_info.resize(1);
    current_frame_info = &frame_info.front();

    is_recording = true;
    LOG_NOTICE(TGP, "FIFO recording started");
}

void Write(u8* data, int size)
{
    FPElementInfo element;
    element.type = FPElementInfo::REGISTER_WRITE;
    element.size = size;
    element.offset = raw_data.size();
    current_frame_info->element_info.push_back(element);

    raw_data.insert(raw_data.end(), data, data + size);
}

void MemUpdate(u32 address, u8* data, u32 size)
{
    FPElementInfo element;
    element.type = FPElementInfo::MEMORY_UPDATE;
    element.size = sizeof(FPMemUpdateInfo) + size;
    element.offset = raw_data.size();

    raw_data[element.offset  ] = address;
    raw_data[element.offset+1] = size;
    raw_data.insert(raw_data.end(), data, data + size);
}

void FrameFinished()
{
    current_frame_info->frame_info.num_elements = current_frame_info->element_info.size();
    frame_info.resize(frame_info.size()+1);
    current_frame_info = &frame_info.front();
}

void EndRecording() {
    FrameFinished();
    while (frame_info.back().element_info.empty() && !frame_info.empty())
        frame_info.pop_back();

    file_header.num_frames = frame_info.size();
    file_header.data_offset = sizeof(FPFileHeader) + file_header.num_frames * sizeof(FPFrameInfo);
    for (std::vector<FPFrameWithElementsInfo>::iterator frame = frame_info.begin(); frame != frame_info.end(); ++frame)
    {
        frame->frame_info.element_info_offset = file_header.data_offset;
        file_header.data_offset += frame->frame_info.num_elements * sizeof(FPElementInfo);
    }

    is_recording = false;
    LOG_NOTICE(TGP, "FIFO recording ended: %d frames\n", frame_info.size());
}

void Save(const char* filename)
{
    // TODO: Error checking...
    FILE* file = fopen(filename, "wb");

    fwrite(&file_header, sizeof(FPFileHeader), 1, file);
    for (std::vector<FPFrameWithElementsInfo>::iterator frame = frame_info.begin(); frame != frame_info.end(); ++frame)
        fwrite(&frame->frame_info, sizeof(FPFrameInfo), 1, file);

    for (std::vector<FPFrameWithElementsInfo>::iterator frame = frame_info.begin(); frame != frame_info.end(); ++frame)
        fwrite(&(*frame->element_info.begin()), frame->frame_info.num_elements * sizeof(FPElementInfo), 1, file);

    fwrite(&(*raw_data.begin()), raw_data.size(), 1, file);

    fclose(file);
}

//#define FIFO_PLAYBACK_SIZE  (32*1024*1024)
//u8 fifo_buff[FIFO_PLAYBACK_SIZE];

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
