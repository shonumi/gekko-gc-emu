/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    video_core.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-08
 * @brief   Main module for new video core
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
 * Official project repository can be found at:
 * http://code.google.com/p/gekko-gc-emu/
 */

#include "SDL.h"

#include "common.h"
#include "config.h"

#include "core.h"

#include "emuwindow/emuwindow_glfw.h"

#include "renderer_gl3/renderer_gl3.h"
#include "renderer_gl2/renderer_gl2.h"

#include "video_core.h"
#include "renderer_base.h"
#include "vertex_manager.h"
#include "vertex_loader.h"
#include "fifo.h"
#include "fifo_player.h"
#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Video Core namespace

namespace video_core {

RendererBase*   g_renderer;         ///< Renderer plugin
SDL_Thread      *g_video_thread;

int VideoEntry(void* emu_window) {

    g_renderer = new RendererGL3();
    g_renderer->SetWindow((EmuWindow*)emu_window);
    g_renderer->Init();

    if (common::g_config->enable_multicore()) {
        while (core::SYS_RUNNING == core::g_state) {
            gp::DecodeCommand();
        }
    }
    return E_OK;
}

/// Start the video core
void Start(EmuWindow* emu_window) {

    if (common::g_config->enable_multicore()) {
        g_video_thread = SDL_CreateThread(VideoEntry, NULL, emu_window);
        if (g_video_thread == NULL) {
            LOG_ERROR(TVIDEO, "Unable to create thread: %s... Exiting\n", SDL_GetError());
            exit(1);
        }
    } else {
        VideoEntry(emu_window);
    }
}

/// Initialize the video core
void Init() {
    gp::FifoInit();
    
    vertex_manager::Init();
    gp::VertexLoaderInit();
    gp::BPInit();
    gp::CPInit();
    gp::XFInit();

    LOG_NOTICE(TVIDEO, "initialized ok");
}

/// Shutdown the video core
void Shutdown() {
    gp::FifoShutdown();
}

} // namespace
