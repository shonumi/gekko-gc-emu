/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    renderer_gl3.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-09
 *   Implementation of a OpenGL 3 renderer
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

#ifndef VIDEO_CORE_RENDERER_GL3_H_
#define VIDEO_CORE_RENDERER_GL3_H_

#include <GL/glew.h>

#include "common.h"
#include "hash_container.h"
#include "gx_types.h"
#include "renderer_base.h"
#include "uniform_manager.h"

#define VBO_MAX_VERTS               (VBO_SIZE / sizeof(GXVertex))     
#define MAX_FRAMEBUFFERS            2
#define MAX_CACHED_TEXTURES         0x1000000

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL 3.x Renderer

class RendererGL3 : virtual public RendererBase {
public:
    RendererGL3();
    ~RendererGL3();

    /**
     * Write data to BP for renderer internal use (e.g. direct to shader)
     * @param addr BP register address
     * @param data Value to write to BP register
     */
    void WriteBP(u8 addr, u32 data);

    /**
     * Write data to CP for renderer internal use (e.g. direct to shader)
     * @param addr CP register address
     * @param data Value to write to CP register
     */
    void WriteCP(u8 addr, u32 data);

    /**
     * Write data to XF for renderer internal use (e.g. direct to shader)
     * @param addr XF address
     * @param length Length (in 32-bit words) to write to XF
     * @param data Data buffer to write to XF
     */
    void WriteXF(u16 addr, int length, u32* data);

    /**
     * Begin renderering of a primitive
     * @param prim Primitive type (e.g. GX_TRIANGLES)
     * @param count Number of vertices to be drawn (used for appropriate memory management, only)
     * @param vbo Pointer to VBO, which will be set by API in this function
     * @param vbo_offset Offset into VBO to use (in bytes)
     */
    void BeginPrimitive(GXPrimitive prim, int count, GXVertex** vbo, u32 vbo_offset);

    /**
     * Set the current vertex state (format and count of each vertex component)
     * @param vertex_state VertexState structure of the current vertex state
     */
    void SetVertexState(const gp::VertexState& vertex_state);

    /**
     * Used to signal to the render that a region in XF is required by a primitive
     * @param index Vector index in XF memory that is required
     */
    void VertexPosition_UseIndexXF(u8 index);

    /// End a primitive (signal renderer to draw it)
    void EndPrimitive(u32 vbo_offset, u32 vertex_num);

    /// Sets the renderer viewport location, width, and height
    void SetViewport(int x, int y, int width, int height);

    /// Sets the renderer depthrange, znear and zfar
    void SetDepthRange(double znear, double zfar);

    /// Sets the renderer depth test mode
    void SetDepthMode();

    /// Sets the renderer generation mode
    void SetGenerationMode();

    /** 
     * Sets the renderer blend mode
     * @param pe_cmode_0 BPPECMode0 register to user for blend settings
     * @param pe_cmode_1 BPPECMode1 register to user for blend settings
     * @param blend_mode_ Forces blend mode to update
     */
    void SetBlendMode(const gp::BPPECMode0& pe_cmode_0, const gp::BPPECMode1& pe_cmode_1, 
        bool force_update);

    /** 
     * Sets the renderer logic op mode
     * @param pe_cmode_0 BPPECMode0 register to user for blend settings
     */
    void SetLogicOpMode(const gp::BPPECMode0& pe_cmode_0);

    /**
     * Sets the renderer dither mode
     * @param pe_cmode_0 BPPECMode0 register to user for blend settings
     */
    void SetDitherMode(const gp::BPPECMode0& pe_cmode_0);

    /**
     * Sets the renderer color mask mode
     * @param pe_cmode_0 BPPECMode0 register to user for blend settings
     */
    void SetColorMask(const gp::BPPECMode0& pe_cmode_0);

    /* Sets the scissor box
     * @param rect Renderer rectangle to set scissor box to
     */
    void SetScissorBox(const Rect& rect);

    /**
     * Sets the line and point size
     * @param line_width Line width to use
     * @param point_size Point size to use
     */
    void SetLinePointSize(f32 line_width, f32 point_size);

    /** 
     * Blits the EFB to the external framebuffer (XFB)
     * @param src_rect Source rectangle in EFB to copy
     * @param dst_rect Destination rectangle in EFB to copy to
     * @param dest_height Destination height in pixels
     */
    void CopyToXFB(const Rect& src_rect, const Rect& dst_rect);

    /**
     * Clear the screen
     * @param rect Screen rectangle to clear
     * @param enable_color Enable color clearing
     * @param enable_alpha Enable alpha clearing
     * @param enable_z Enable depth clearing
     * @param color Clear color
     * @param z Clear depth
     */
    void Clear(const Rect& rect, bool enable_color, bool enable_alpha, bool enable_z, u32 color, 
        u32 z);

    /**
     * Set a specific render mode
     * @param flags Render flag mode to enable
     */
    void SetMode(kRenderMode flags);

    /**
     * Restore the render mode
     * @param pe_cmode_0 BPPECMode0 register to user for blend settings
     */
    void RestoreMode(const gp::BPPECMode0& pe_cmode_0);

    /// Reset the full renderer API to the NULL state
    void ResetRenderState();

    /// Restore the full renderer API state - As the game set it
    void RestoreRenderState();

    /// Swap the display buffers (finish drawing frame)
    void SwapBuffers();

    /**
     * Set the window of the emulator
     * @param window EmuWindow handle to emulator window to use for rendering
     */
    void SetWindow(EmuWindow* window);

    void Init();
    void ShutDown();

    // Framebuffer object(s)
    // ---------------------

    GLuint      fbo_[MAX_FRAMEBUFFERS];                 ///< Framebuffer objects

    UniformManager* uniform_manager_;

private:

    /// Initialize the FBO
    void InitFramebuffer();

    // Blit the FBO to the OpenGL default framebuffer
    void RenderFramebuffer();

    /// Updates the framerate
    void UpdateFramerate();

    int resolution_width_;
    int resolution_height_;

    // Framebuffer object(s)
    // ---------------------

    GLuint      fbo_rbo_[MAX_FRAMEBUFFERS];             ///< Render buffer objects
    GLuint      fbo_depth_buffers_[MAX_FRAMEBUFFERS];   ///< Depth buffers objects

    // Vertex buffer stuff
    // -------------------

    GLuint      vbo_handle_;                        ///< Handle of vertex buffer object
    GXPrimitive prim_type_;                         ///< GX primitive type (e.g. GX_QUADS)
    GLuint      gl_prim_type_;                      ///< OpenGL primitive type (e.g. GL_TRIANGLES)
    
    // Vertex format stuff
    // -------------------

    gp::VertexState vertex_state_;

    // Video core stuff
    // ----------------

    EmuWindow*  render_window_;
    u32         last_mode_;                         ///< Last render mode

    // BP stuff
    // --------

    u32         blend_mode_;

    DISALLOW_COPY_AND_ASSIGN(RendererGL3);
};

#endif // VIDEO_CORE_RENDERER_GL3_H_
