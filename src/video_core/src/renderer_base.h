/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    renderer_base.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-08
 * @brief   Renderer base class for new video core
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

#ifndef VIDEO_CORE_RENDER_BASE_H_
#define VIDEO_CORE_RENDER_BASE_H_

#include "common.h"
#include "hash.h"
#include "fifo.h"
#include "gx_types.h"
#include "video/emuwindow.h"
#include "texture_manager.h"

class RendererBase {
public:

    /// Used to reference a framebuffer
    enum kFramebuffer {
        kFramebuffer_VirtualXFB = 0,
        kFramebuffer_EFB
    };

    /// Used for referencing the render modes
    enum kRenderMode {
        kRenderMode_None = 0,
        kRenderMode_Multipass = 1,
        kRenderMode_ZComp = 2,
        kRenderMode_UseDstAlpha = 4
    };

    RendererBase() : current_fps_(0), current_frame_(0), texture_interface_(NULL) {
    }

    ~RendererBase() {
    }

    /**
     * Write data to BP for renderer internal use (e.g. direct to shader)
     * @param addr BP register address
     * @param data Value to write to BP register
     */
    virtual void WriteBP(u8 addr, u32 data) = 0;

    /**
     * Write data to CP for renderer internal use (e.g. direct to shader)
     * @param addr CP register address
     * @param data Value to write to CP register
     */
    virtual void WriteCP(u8 addr, u32 data) = 0;

    /**
     * Write data to XF for renderer internal use (e.g. direct to shader)
     * @param addr XF address
     * @param length Length (in 32-bit words) to write to XF
     * @param data Data buffer to write to XF
     */
    virtual void WriteXF(u16 addr, int length, u32* data) = 0;

    /**
     * Begin renderering of a primitive
     * @param prim Primitive type (e.g. GX_TRIANGLES)
     * @param count Number of vertices to be drawn (used for appropriate memory management, only)
     * @param vbo Pointer to VBO, which will be set by API in this function
     * @param vbo_offset Offset into VBO to use (in bytes)
     */
    virtual void BeginPrimitive(GXPrimitive prim, int count, GXVertex** vbo, u32 vbo_offset) = 0;

    /**
     * Set the type of postion vertex data
     * @param type Position data type (e.g. GX_F32)
     * @param count Position data count (e.g. GX_POS_XYZ)
     */
    virtual void VertexPosition_SetType(GXCompType type, GXCompCnt count) = 0;

    /**
     * Used to signal to the render that a region in XF is required by a primitive
     * @param index Vector index in XF memory that is required
     */
    virtual void VertexPosition_UseIndexXF(u8 index) = 0;

    /**
     * Set the type of color vertex data - type is always RGB8/RGBA8, just set count
     * @param color Which color to configure (0 or 1)
     * @param type GXCompType color format type
     * @param count Color data count (e.g. GX_CLR_RGBA)
     */
    virtual void VertexColor_SetType(int color, GXCompType type, GXCompCnt count) = 0;

    /**
     * Set the type of texture coordinate vertex data
     * @param texcoord 0-7 texcoord to set type of
     * @param type Texcoord data type (e.g. GX_F32)
     * @param count Texcoord data count (e.g. GX_TEX_ST)
     */
    virtual void VertexTexcoord_SetType(int texcoord, GXCompType type, GXCompCnt count) = 0;

    /// End a primitive (signal renderer to draw it)
    virtual void EndPrimitive(u32 vbo_offset, u32 vertex_num) = 0;
   
    /// Sets the render viewport location, width, and height
    virtual void SetViewport(int x, int y, int width, int height) = 0;

    /// Swap buffers (render frame)
    virtual void SwapBuffers() = 0;

    /// Sets the renderer depthrange, znear and zfar
    virtual void SetDepthRange(double znear, double zfar) = 0;

    /// Sets the renderer depth test mode
    virtual void SetDepthMode() = 0;

    /// Sets the renderer generation mode
    virtual void SetGenerationMode() = 0;

    /** 
     * Sets the renderer blend mode
     * @param blend_mode_ Forces blend mode to update
     */
    virtual void SetBlendMode(bool force_update) = 0;

    /// Sets the renderer logic op mode
    virtual void SetLogicOpMode() = 0;

    /// Sets the renderer dither mode
    virtual void SetDitherMode() = 0;

    /// Sets the renderer color mask mode
    virtual void SetColorMask() = 0;

    /// Sets the scissor box
    virtual void SetScissorBox() = 0;

    /**
     * Sets the line and point size
     * @param line_width Line width to use
     * @param point_size Point size to use
     */
    virtual void SetLinePointSize(f32 line_width, f32 point_size) = 0;

    /** 
     * Blits the EFB to the specified destination buffer
     * @param dest Destination framebuffer
     * @param rect EFB rectangle to copy
     * @param dest_width Destination width in pixels 
     * @param dest_height Destination height in pixels
     */
    virtual void CopyEFB(kFramebuffer dest, Rect rect, u32 dest_width, u32 dest_height) = 0;

    /**
     * Clear the screen
     * @param rect Screen rectangle to clear
     * @param enable_color Enable color clearing
     * @param enable_alpha Enable alpha clearing
     * @param enable_z Enable depth clearing
     * @param color Clear color
     * @param z Clear depth
     */
    virtual void Clear(Rect rect, bool enable_color, bool enable_alpha, bool enable_z, u32 color, 
        u32 z) = 0;

    /**
     * Set a specific render mode
     * @param flag Render flags mode to enable
     */
    virtual void SetMode(kRenderMode flags) = 0;

    /// Restore the render mode
    virtual void RestoreMode() = 0;

    /// Reset the full renderer API to the NULL state
    virtual void ResetRenderState() = 0;

    /// Restore the full renderer API state - As the game set it
    virtual void RestoreRenderState() = 0;

    /** 
     * Set the emulator window to use for renderer
     * @param window EmuWindow handle to emulator window to use for rendering
     */
    virtual void SetWindow(EmuWindow* window) = 0;

    /// Initialize the renderer
    virtual void Init() = 0;

    /// Shutdown the renderer
    virtual void ShutDown() = 0;

    // Getter/setter functions:
    // ------------------------

    f32 current_fps() { return current_fps_; }

    int current_frame() { return current_frame_; }

    TextureManager::BackendInterface* texture_interface() { return texture_interface_; }

protected:
    f32 current_fps_;                       ///< Current framerate, should be set by the renderer
    int current_frame_;                     ///< Current frame, should be set by the renderer

    TextureManager::BackendInterface* texture_interface_;

private:

    DISALLOW_COPY_AND_ASSIGN(RendererBase);
};

#endif // VIDEO_CORE_RENDER_BASE_H_
