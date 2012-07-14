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
#include "gx_types.h"
#include "video/emuwindow.h"

class RendererBase {
public:

    enum kFramebuffer {
        kFramebuffer_VirtualXFB = 0,
        kFramebuffer_EFB
    };

    RendererBase() {
    }

    ~RendererBase() {
    }

    /**
     * Begin renderering of a primitive
     * @param prim Primitive type (e.g. GX_TRIANGLES)
     * @param count Number of vertices to be drawn (used for appropriate memory management, only)
     */
    virtual void BeginPrimitive(GXPrimitive prim, int count) = 0;

    /**
     * Set the type of postion vertex data
     * @param type Position data type (e.g. GX_F32)
     * @param count Position data count (e.g. GX_POS_XYZ)
     */
    virtual void VertexPosition_SetType(GXCompType type, GXCompCnt count) = 0;

    /**
     * Send a position vector to the renderer as 32-bit floating point
     * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
     */
    virtual void VertexPosition_SendFloat(f32* vec) = 0;

    /**
     * Send a position vector to the renderer as 16-bit short (signed or unsigned)
     * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
     */
    virtual void VertexPosition_SendShort(u16* vec) = 0;

    /**
     * Send a position vector to the renderer an 8-bit byte (signed or unsigned)
     * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
     */
    virtual void VertexPosition_SendByte(u8* vec) = 0;

    /**
     * Set the type of color 0 vertex data - type is always RGB8/RGBA8, just set count
     * @param count Color data count (e.g. GX_CLR_RGBA)
     */
    virtual void VertexColor0_SetType(GXCompCnt count) = 0;

    /**
     * Send a vertex color 0 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
     * @param color Color to send, packed as RRGGBBAA or RRGGBB00
     */
    virtual void VertexColor0_Send(u32 color) = 0;
    
    /**
     * Set the type of color 1 vertex data - type is always RGB8/RGBA8, just set count
     * @param count Color data count (e.g. GX_CLR_RGBA)
     */
    virtual void VertexColor1_SetType(GXCompCnt count) = 0;

    /**
     * Send a vertex color 1 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
     * @param color Color to send, packed as RRGGBBAA or RRGGBB00
     */
    virtual void VertexColor1_Send(u32 color) = 0;

    /**
     * Set the type of texture coordinate vertex data
     * @param texcoord 0-7 texcoord to set type of
     * @param type Texcoord data type (e.g. GX_F32)
     * @param count Texcoord data count (e.g. GX_TEX_ST)
     */
    virtual void VertexTexcoord_SetType(int texcoord, GXCompType type, GXCompCnt count) = 0;

    /**
     * Send a texcoord vector to the renderer as 32-bit floating point
     * @param texcoord 0-7 texcoord to configure
     * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
     */
    virtual void VertexTexcoord_SendFloat(int texcoord, f32* vec) = 0;

    /**
     * Send a texcoord vector to the renderer as 16-bit short (signed or unsigned)
     * @param texcoord 0-7 texcoord to configure
     * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
     */
    virtual void VertexTexcoord_SendShort(int texcoord, u16* vec) = 0;

    /**
     * Send a texcoord vector to the renderer as 8-bit byte (signed or unsigned)
     * @param texcoord 0-7 texcoord to configure
     * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
     */
    virtual void VertexTexcoord_SendByte(int texcoord, u8* vec) = 0;

    /**
     * @brief Sends position and texcoord matrix indices to the renderer
     * @param pm_idx Position matrix index
     * @param tm_idx Texture matrix indices
     */
    virtual void Vertex_SendMatrixIndices(u8 pm_idx, u8 tm_idx[]) = 0;

    /// Done with the current vertex - go to the next
    virtual void VertexNext() = 0;

    /// End a primitive (signal renderer to draw it)
    virtual void EndPrimitive() = 0;
   
    /// Sets the render viewport location, width, and height
    virtual void SetViewport(int x, int y, int width, int height) = 0;

    /// Swap buffers (render frame)
    virtual void SwapBuffers() = 0;

    /// Sets the renderer depthrange, znear and zfar
    virtual void SetDepthRange(double znear, double zfar) = 0;

    /// Sets the renderer depth test mode
    virtual void SetDepthTest() = 0;

    /// Sets the renderer culling mode
    virtual void SetCullMode() = 0;

    /** 
     * @brief Blits the EFB to the specified destination buffer
     * @param dest Destination framebuffer
     */
    virtual void CopyEFB(kFramebuffer dest) = 0;

    /** 
     * @brief Set the emulator window to use for renderer
     * @param window EmuWindow handle to emulator window to use for rendering
     */
    virtual void SetWindow(EmuWindow* window) = 0;

    /// Initialize the renderer
    virtual void Init() = 0;

    /// Shutdown the renderer
    virtual void ShutDown() = 0;

private:

    DISALLOW_COPY_AND_ASSIGN(RendererBase);
};

#endif // VIDEO_CORE_RENDER_BASE_H_
