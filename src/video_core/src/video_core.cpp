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

#include "video/emuwindow.h"

#include "renderer_gl3/renderer_gl3.h"

#include "video_core.h"
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

EmuWindow*      g_emu_window = NULL;    ///< Frontend emulator window
RendererBase*   g_renderer = NULL;      ///< Renderer plugin
SDL_Thread*     g_video_thread = NULL;
TextureManager* g_texture_manager = NULL;
int             g_current_frame = 0;

int VideoEntry(void*) {
    if (g_emu_window == NULL) {
        LOG_ERROR(TGP, "video_core::VideoEntry called without calling Init()!");
    }
    g_emu_window->MakeCurrent();
    for(;;) {
        gp::Fifo_DecodeCommand();
    }
    return E_OK;
}

/// Start the video core
void Start() {
    if (g_emu_window == NULL) {
        LOG_ERROR(TGP, "video_core::Start called without calling Init()!");
    }
    if (common::g_config->enable_multicore()) {
        g_emu_window->DoneCurrent();
        g_video_thread = SDL_CreateThread(VideoEntry, NULL, NULL);
        if (g_video_thread == NULL) {
            LOG_ERROR(TVIDEO, "Unable to create thread: %s... Exiting\n", SDL_GetError());
            exit(1);
        }
    }
}

/// Initialize the video core
void Init(EmuWindow* emu_window) {
    g_emu_window = emu_window;
    g_renderer = new RendererGL3();
    g_renderer->SetWindow(g_emu_window);
    g_renderer->Init();

    gp::Fifo_Init();
    gp::VertexManager_Init();
    gp::VertexLoader_Init();
    gp::BP_Init();
    gp::CP_Init();
    gp::XF_Init();

    g_texture_manager = new TextureManager(g_renderer->texture_interface());

    g_current_frame = 0;

    LOG_NOTICE(TVIDEO, "initialized ok");
}

/// Shutdown the video core
void Shutdown() {
    gp::Fifo_Shutdown();
    gp::VertexManager_Shutdown();
    gp::VertexLoader_Shutdown();

    delete g_renderer;
    delete g_texture_manager;
}

} // namespace
