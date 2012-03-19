/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    renderer_base.h
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

#ifndef VIDEO_CORE_RENDER_BASE_H_
#define VIDEO_CORE_RENDER_BASE_H_

#include "common.h"
#include "fifo.h"
#include "video/emuwindow.h"

class RendererBase {
public:
	RendererBase();
	virtual ~RendererBase();

    /// Draw a vertex array
    virtual void DrawPrimitive();

    /// Sets the render viewport location, width, and height
    virtual void SetViewport(int x, int y, int width, int height);

    /// Swap buffers (render frame)
    virtual void SwapBuffers();

    /// Sets the renderer depthrange, znear and zfar
    virtual void SetDepthRange(double znear, double zfar);

    /// Sets the renderer depth test mode
    virtual void SetDepthTest();

    /// Sets the renderer culling mode
    virtual void SetCullMode();

    /*! 
     * \brief Set the emulator window to use for renderer
     * \param window EmuWindow handle to emulator window to use for rendering
     */
    virtual void SetWindow(EmuWindow* window);

    /// Initialize the renderer
    virtual void Init();

    /// Shutdown the renderer
    virtual void ShutDown();

private:

    DISALLOW_COPY_AND_ASSIGN(RendererBase);
};

#endif // VIDEO_CORE_RENDER_BASE_H_
