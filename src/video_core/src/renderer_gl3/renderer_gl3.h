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
#include "shader_manager.h"
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
     * Set the type of postion vertex data
     * @param type Position data type (e.g. GX_F32)
     * @param count Position data count (e.g. GX_POS_XYZ)
     */
    void VertexPosition_SetType(GXCompType type, GXCompCnt count);

    /**
     * Used to signal to the render that a region in XF is required by a primitive
     * @param index Vector index in XF memory that is required
     */
    void VertexPosition_UseIndexXF(u8 index);

    /**
     * Set the type of color vertex data - type is always RGB8/RGBA8, just set count
     * @param color Which color to configure (0 or 1)
     * @param type GXCompType color format type
     * @param count Color data count (e.g. GX_CLR_RGBA)
     */
    void VertexColor_SetType(int color, GXCompType type, GXCompCnt count);

    /**
     * Set the type of texture coordinate vertex data
     * @param texcoord 0-7 texcoord to set type of
     * @param type Texcoord data type (e.g. GX_F32)
     * @param count Texcoord data count (e.g. GX_TEX_ST)
     */
    void VertexTexcoord_SetType(int texcoord, GXCompType type, GXCompCnt count);

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
     * @param blend_mode_ Forces blend mode to update
     */
    void SetBlendMode(bool force_update);

    /// Sets the renderer logic op mode
    void SetLogicOpMode();

    /// Sets the renderer dither mode
    void SetDitherMode();

    /// Sets the renderer color mask mode
    void SetColorMask();

    /// Sets the scissor box
    void SetScissorBox();

    /**
     * Sets the line and point size
     * @param line_width Line width to use
     * @param point_size Point size to use
     */
    void SetLinePointSize(f32 line_width, f32 point_size);

    /** 
     * Blits the EFB to the specified destination buffer
     * @param dest Destination framebuffer
     * @param rect EFB rectangle to copy
     * @param dest_width Destination width in pixels 
     * @param dest_height Destination height in pixels
     */
    void CopyEFB(kFramebuffer dest, Rect rect, u32 dest_width, u32 dest_height);

    /**
     * Clear the screen
     * @param rect Screen rectangle to clear
     * @param enable_color Enable color clearing
     * @param enable_alpha Enable alpha clearing
     * @param enable_z Enable depth clearing
     * @param color Clear color
     * @param z Clear depth
     */
    void Clear(Rect rect, bool enable_color, bool enable_alpha, bool enable_z, u32 color, u32 z);

    /**
     * Set a specific render mode
     * @param flags Render flag mode to enable
     */
    void SetMode(kRenderMode flags);

    /// Restore the render mode
    void RestoreMode();

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

    GLuint      vertex_position_format_;            ///< OpenGL position format (e.g. GL_FLOAT)
    int         vertex_position_format_size_;       ///< Number of bytes to represent one coordinate
    GXCompCnt   vertex_position_component_count_;   ///< Number of coordinates (2 - XY, 3 - XYZ)

    int         vertex_color_cur_;                  ///< Current color, 0 or 1
    GXCompType  vertex_color_cur_type_[2];
    GXCompCnt   vertex_color_cur_count_[2];

    int         vertex_texcoord_cur_;               ///< Current texcoord, 0-kNumTextures
    int         vertex_texcoord_enable_[8];

    GLuint      vertex_texcoord_format_[8];     

    int         vertex_texcoord_format_size_[8];    

    GXCompCnt   vertex_texcoord_component_count_[8];

    EmuWindow*  render_window_;

    ShaderManager*  shader_manager_;
    UniformManager* uniform_manager_;

    GLuint      generic_shader_id_;

    u32         last_mode_;                         ///< Last render mode

    // BP stuff
    // --------

    u32         blend_mode_;

    DISALLOW_COPY_AND_ASSIGN(RendererGL3);
};

#endif // VIDEO_CORE_RENDERER_GL3_H_
