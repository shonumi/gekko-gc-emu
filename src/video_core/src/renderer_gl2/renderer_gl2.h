/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    renderer_gl2.h
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-03-18
 * \brief   Implementation of a OpenGL 32 renderer
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

#ifndef VIDEO_CORE_RENDERER_GL2_H_
#define VIDEO_CORE_RENDERER_GL2_H_

#include <GL/glew.h>

#include "video/emuwindow.h"

#include "common.h"
#include "renderer_base.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL 2 Renderer

class RendererGL2 : virtual public RendererBase {
public:
    RendererGL2() : resolution_width_(640), resolution_height_(480) {};
    ~RendererGL2() {};

    /// Draw a vertex array
    void DrawPrimitive();

    /// Sets the renderer viewport location, width, and height
    void SetViewport(int x, int y, int width, int height);

    /// Sets the renderer depthrange, znear and zfar
    void SetDepthRange(double znear, double zfar);

    /// Sets the renderer depth test mode
    void SetDepthTest();

    /// Sets the renderer culling mode
    void SetCullMode();

    /// Swap the display buffers (finish drawing frame)
    void SwapBuffers();

    /*! 
     * \brief Set the window of the emulator
     * \param window EmuWindow handle to emulator window to use for rendering
     */
    void SetWindow(EmuWindow* window);

    void Init();
    void PollEvent();
    void ShutDown();

private:

    int resolution_width_;
    int resolution_height_;

    EmuWindow* render_window_;

    GLuint shader_id_;

    DISALLOW_COPY_AND_ASSIGN(RendererGL2);
};

#endif // VIDEO_CORE_RENDERER_GL2_H_
