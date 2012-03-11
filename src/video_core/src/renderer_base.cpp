/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    renderer_base.cpp
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-08
 * \brief   Renderer base class for new video core
 *
 * \section LICENSE
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

#include "common.h"
#include "renderer_base.h"

RendererBase::RendererBase() {
}

RendererBase::~RendererBase() {
}

/// Draw a vertex array
void RendererBase::DrawPrimitive() {
    printf("RendererBase::DrawPrimitive()\n");
}

/// Swap buffers (render frame)
void RendererBase::SwapBuffers() {
    printf("RendererBase::SwapBuffers()\n");
}

/*! 
 * \brief Set the window text of the renderer
 * \param text Text so set the window title bar to
 */
void RendererBase::SetWindowText(const char* text) {
    printf("RendererBase::SetWindowText()\n");
}

/*! 
 * \brief Set the window size of the renderer
 * \param text Text so set the window title bar to
 */
void RendererBase::SetWindowSize(int width, int height) {
    printf("RendererBase::SetWindowSize()\n");
}

/// Initialize the renderer
void RendererBase::Init() {
    printf("RendererBase::Init()\n");
}

/// Shutdown the renderer
void RendererBase::ShutDown() {
    printf("RendererBase::Init()\n");
}
