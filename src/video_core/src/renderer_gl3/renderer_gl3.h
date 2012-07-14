/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    renderer_gl3.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-09
 * @brief   Implementation of a OpenGL 3 renderer
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
#include "gx_types.h"
#include "renderer_base.h"

#define VBO_SIZE                    (1024 * 1024 * 32)
#define VBO_MAX_VERTS               (VBO_SIZE / sizeof(GXVertex))     
#define USE_GEOMETRY_SHADERS        1
#define MAX_FRAMEBUFFERS            2

////////////////////////////////////////////////////////////////////////////////////////////////////
// OpenGL 3.x Renderer

class RendererGL3  : virtual public RendererBase {
public:
    RendererGL3();
    ~RendererGL3() {};

    /**
     * Begin renderering of a primitive
     * @param prim Primitive type (e.g. GX_TRIANGLES)
     * @param count Number of vertices to be drawn (used for appropriate memory management, only)
     */
    void BeginPrimitive(GXPrimitive prim, int count);

    /**
     * Set the type of postion vertex data
     * @param type Position data type (e.g. GX_F32)
     * @param count Position data count (e.g. GX_POS_XYZ)
     */
    void VertexPosition_SetType(GXCompType type, GXCompCnt count);

    /**
     * Send a position vector to the renderer as 32-bit floating point
     * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
     */
    void VertexPosition_SendFloat(f32* vec);

    /**
     * Send a position vector to the renderer as 16-bit short (signed or unsigned)
     * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
     */
    void VertexPosition_SendShort(u16* vec);

    /**
     * Send a position vector to the renderer an 8-bit byte (signed or unsigned)
     * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
     */
    void VertexPosition_SendByte(u8* vec);

    /**
     * Set the type of color 0 vertex data - type is always RGB8/RGBA8, just set count
     * @param count Color data count (e.g. GX_CLR_RGBA)
     */
    void VertexColor0_SetType(GXCompCnt count);

    /**
     * Send a vertex color 0 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
     * @param color Color to send, packed as RRGGBBAA or RRGGBB00
     */
    void VertexColor0_Send(u32 color);
    
    /**
     * Set the type of color 1 vertex data - type is always RGB8/RGBA8, just set count
     * @param count Color data count (e.g. GX_CLR_RGBA)
     */
    void VertexColor1_SetType(GXCompCnt count);

    /**
     * Send a vertex color 1 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
     * @param color Color to send, packed as RRGGBBAA or RRGGBB00
     */
    void VertexColor1_Send(u32 color);

    /**
     * Set the type of texture coordinate vertex data
     * @param texcoord 0-7 texcoord to set type of
     * @param type Texcoord data type (e.g. GX_F32)
     * @param count Texcoord data count (e.g. GX_TEX_ST)
     */
    void VertexTexcoord_SetType(int texcoord, GXCompType type, GXCompCnt count);

    /**
     * Send a texcoord vector to the renderer as 32-bit floating point
     * @param texcoord 0-7 texcoord to configure
     * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
     */
    void VertexTexcoord_SendFloat(int texcoord, f32* vec);

    /**
     * Send a texcoord vector to the renderer as 16-bit short (signed or unsigned)
     * @param texcoord 0-7 texcoord to configure
     * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
     */
    void VertexTexcoord_SendShort(int texcoord, u16* vec);

    /**
     * Send a texcoord vector to the renderer as 8-bit byte (signed or unsigned)
     * @param texcoord 0-7 texcoord to configure
     * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
     */
    void VertexTexcoord_SendByte(int texcoord, u8* vec);

    /**
     * @brief Sends position and texcoord matrix indices to the renderer
     * @param pm_idx Position matrix index
     * @param tm_idx Texture matrix indices
     */
    void Vertex_SendMatrixIndices(u8 pm_idx, u8 tm_idx[]);

    /// Used for specifying next GX vertex is being sent to the renderer
    void VertexNext();

    /// End a primitive (signal renderer to draw it)
    void EndPrimitive();

    /// Sets the renderer viewport location, width, and height
    void SetViewport(int x, int y, int width, int height);

    /// Sets the renderer depthrange, znear and zfar
    void SetDepthRange(double znear, double zfar);

    /// Sets the renderer depth test mode
    void SetDepthTest();

    /// Sets the renderer culling mode
    void SetCullMode();

    /** 
     * @brief Blits the EFB to the specified destination buffer
     * @param dest Destination framebuffer
     */
    void CopyEFB(kFramebuffer dest);

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

    /// Prints some useful debug information to the screen
    void PrintDebugStats();

    /// Initialize the FBO
    void InitFramebuffer();

    // Blit the FBO to the OpenGL default framebuffer
    void RenderFramebuffer();

    int resolution_width_;
    int resolution_height_;

    // Framebuffer object
    // ------------------

    GLuint      fbo_[MAX_FRAMEBUFFERS];                 ///< Framebuffer objects
    GLuint      fbo_rbo_[MAX_FRAMEBUFFERS];             ///< Render buffer objects
    GLuint      fbo_depth_buffers_[MAX_FRAMEBUFFERS];   ///< Depth buffers objects

    // Vertex buffer object
    // --------------------

    GLuint      vbo_handle_;                ///< Handle to the GL VBO
    GXVertex*   vbo_;                       ///< Pointer to VBO data (when mapped, in GPU mem)
    GXVertex**  vbo_ptr_;                   ///< Pointer to VBO (used for switching to quad buff)
    u32         vbo_write_offset_;          ///< Offset into VBO of current vertex writes
    GXVertex*   quad_vbo_;                  ///< Buffer for temporarily storing quads in CPU mem
    GXVertex*   quad_vbo_ptr_;              ///< Ptr to quad_vbo_

    GXPrimitive prim_type_;                         ///< GX primitive type (e.g. GX_QUADS)
    GLuint      gl_prim_type_;                      ///< OpenGL primitive type (e.g. GL_TRIANGLES)
    
    GLuint      vertex_position_format_;            ///< OpenGL position format (e.g. GL_FLOAT)
    int         vertex_position_format_size_;       ///< Number of bytes to represent one coordinate
    GXCompCnt   vertex_position_component_count_;   ///< Number of coordinates (2 - XY, 3 - XYZ)
    int         vertex_num_;                        ///< Number of vertices per primitive

    EmuWindow*  render_window_;

    GLuint      generic_shader_id_;

    DISALLOW_COPY_AND_ASSIGN(RendererGL3);
};

#endif // VIDEO_CORE_RENDERER_GL3_H_
