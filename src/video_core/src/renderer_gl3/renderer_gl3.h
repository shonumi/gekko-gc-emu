/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    renderer_gl3.h
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-09
 * \brief   Implementation of a OpenGL 3.3 renderer
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

#ifndef VIDEO_CORE_RENDERER_GL3_H_
#define VIDEO_CORE_RENDERER_GL3_H_

#include "common.h"
#include "renderer_base.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL 3.3 Renderer

class RendererGL3  : virtual public RendererBase {
public:
    RendererGL3() : resolution_width_(640), resolution_height_(480) {};
    ~RendererGL3() {};

    void DrawPrimitive();

    void SetViewport(int x, int y, int width, int height);
    void SetDepthRange(double znear, double zfar);

    void SwapBuffers();
    void SetWindowText(const char* text);
    void SetWindowSize(int width, int height);

    void Init();
    void PollEvent();
    void ShutDown();

private:

    int resolution_width_;
    int resolution_height_;

    DISALLOW_COPY_AND_ASSIGN(RendererGL3);
};

#endif // VIDEO_CORE_RENDERER_GL3_H_
