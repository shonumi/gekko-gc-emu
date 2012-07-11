/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    renderer_gl3.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-10
 * @brief   Implementation of a OpenGL 3.2 renderer
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

#include "common.h"

#include "input_common.h"

#include "fifo.h"
#include "vertex_loader.h"
#include "cp_mem.h"
#include "xf_mem.h"

#include "renderer_gl3.h"
#include "shader_manager.h"
#include "raster_font.h"

RasterFont* g_raster_font;

/// RendererGL3 constructor
RendererGL3::RendererGL3() {
    memset(fbo_, 0, sizeof(fbo_));  
    memset(fbo_rbo_, 0, sizeof(fbo_rbo_));  
    memset(fbo_depth_buffers_, 0, sizeof(fbo_depth_buffers_));  
    resolution_width_ = 640;
    resolution_height_ = 480;
    vbo_handle_ = 0;
    vbo_ = NULL;
    vbo_ptr_ = NULL;
    vbo_write_offset_ = 0;
    quad_vbo_ = NULL;
    quad_vbo_ptr_ = NULL;
    vertex_position_format_ = 0;
    vertex_position_format_size_ = 0;
    vertex_position_component_count_ = (GXCompCnt)0;
    vertex_num_ = 0;
    render_window_ = NULL;
    generic_shader_id_ = 0;
    prim_type_ = (GXPrimitive)0;
    gl_prim_type_ = 0;
}

/// Sets up the renderer for drawing a primitive
void RendererGL3::BeginPrimitive(GXPrimitive prim, int count) {

#ifdef USE_GEOMETRY_SHADERS
    // Use geometry shaders to emulate GX_QUADS via GL_LINES_ADJACENCY
    static GLenum gl_types[8] = {GL_LINES_ADJACENCY, 0, GL_TRIANGLES, GL_TRIANGLE_STRIP, 
                                 GL_TRIANGLE_FAN, GL_LINES,  GL_LINE_STRIP, GL_POINTS};
#else
    // Use software to emulate GX_QUADS via GL_TRIANGLES
    static GLenum gl_types[8] = {GL_TRIANGLES, 0, GL_TRIANGLES, GL_TRIANGLE_STRIP, 
                                 GL_TRIANGLE_FAN, GL_LINES,  GL_LINE_STRIP, GL_POINTS};
#endif

    // Beginning of primitive - reset vertex number
    vertex_num_ = 0;

    // Set the renderer primitive type
    prim_type_ = prim;
    gl_prim_type_ = gl_types[(prim >> 3) - 16];

    // If no data sent, we are done here
    if (0 == count) {
        return;
    }

    // Set the shader manager primitive type (only used for geometry shaders)
    shader_manager::SetPrimitive(prim);

    // Bind pointers to buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);

    // Map CPU to GPU mem
    static GLbitfield access_flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | 
                                     GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
    vbo_ = (GXVertex*)glMapBufferRange(GL_ARRAY_BUFFER, (vbo_write_offset_ * sizeof(GXVertex)),
                                       (count * sizeof(GXVertex)), access_flags);
    if (vbo_ == NULL) {
        LOG_ERROR(TVIDEO, "Unable to map vertex buffer object to system mem!");
    }
    
#ifndef USE_GEOMETRY_SHADERS
    // When sending quads, store them in a temporary sys mem buffer so we can rearrange
    // them to triangles before copying to GPU mem
    if (prim_type_ == GX_QUADS) {
        quad_vbo_ptr_ = quad_vbo_;
        vbo_ptr_ = &quad_vbo_ptr_;
    } else {
        vbo_ptr_ = &vbo_;
    }
#else
    vbo_ptr_ = &vbo_;
#endif
}

/**
 * Set the type of postion vertex data
 * @param type Position data type (e.g. GX_F32)
 * @param count Position data count (e.g. GX_POS_XYZ)
 */
void RendererGL3::VertexPosition_SetType(GXCompType type, GXCompCnt count) {
    static GLuint gl_types[5] = {GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_FLOAT};
    static GLuint gl_types_size[5] = {1, 1, 2, 2, 4};
    vertex_position_format_ = gl_types[type];
    vertex_position_format_size_ = gl_types_size[type];
    vertex_position_component_count_ = count;
}

/**
 * Send a position vector to the renderer as 32-bit floating point
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
void RendererGL3::VertexPosition_SendFloat(f32* vec) {
    f32* ptr = (f32*)(*vbo_ptr_)->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Send a position vector to the renderer as 16-bit short (signed or unsigned)
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
void RendererGL3::VertexPosition_SendShort(u16* vec) {
    u16* ptr = (u16*)(*vbo_ptr_)->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}

/**
 * Send a position vector to the renderer an 8-bit byte (signed or unsigned)
 * @param vec Position vector, XY or XYZ, depending on VertexPosition_SetType
 */
void RendererGL3::VertexPosition_SendByte(u8* vec) {
    u8* ptr = (u8*)(*vbo_ptr_)->position;
    ptr[0] = vec[0];
    ptr[1] = vec[1];
    ptr[2] = vec[2];
}
/**
 * Set the type of color 0 vertex data - type is always RGB8/RGBA8, just set count
 * @param count Color data count (e.g. GX_CLR_RGBA)
 */
void RendererGL3::VertexColor0_SetType(GXCompCnt count) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a vertex color 0 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
 * @param color Color to send, packed as RRGGBBAA or RRGGBB00
 */
void RendererGL3::VertexColor0_Send(u32 rgba) {
    (*vbo_ptr_)->color0 = rgba;
}

/**
 * Set the type of color 1 vertex data - type is always RGB8/RGBA8, just set count
 * @param count Color data count (e.g. GX_CLR_RGBA)
 */
void RendererGL3::VertexColor1_SetType(GXCompCnt count) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a vertex color 1 to the renderer (RGB8 or RGBA8, as set by VertexColor0_SetType)
 * @param color Color to send, packed as RRGGBBAA or RRGGBB00
 */
void RendererGL3::VertexColor1_Send(u32 color) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Set the type of texture coordinate vertex data
 * @param texcoord 0-7 texcoord to set type of
 * @param type Texcoord data type (e.g. GX_F32)
 * @param count Texcoord data count (e.g. GX_TEX_ST)
 */
void RendererGL3::VertexTexcoord_SetType(int texcoord, GXCompType type, GXCompCnt count) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a texcoord vector to the renderer as 32-bit floating point
 * @param texcoord 0-7 texcoord to configure
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendFloat(int texcoord, f32* vec) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a texcoord vector to the renderer as 16-bit short (signed or unsigned)
 * @param texcoord 0-7 texcoord to configure
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendShort(int texcoord, u16* vec) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/**
 * Send a texcoord vector to the renderer as 8-bit byte (signed or unsigned)
 * @param texcoord 0-7 texcoord to configure
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendByte(int texcoord, u8* vec) {
    LOG_ERROR(TVIDEO, "Unimplemented method!");
}

/// Used for specifying next GX vertex is being sent to the renderer
void RendererGL3::VertexNext() {
#ifndef USE_GEOMETRY_SHADERS
    if (prim_type_ == GX_QUADS) {

        // End of quad
        if ((vertex_num_ & 3) == 3) {

            // Copy quad to GPU mem has 2 triangles
            vbo_[0] = quad_vbo_[0];
            vbo_[1] = quad_vbo_[1];
            vbo_[2] = quad_vbo_[2];
            vbo_[3] = quad_vbo_[2];
            vbo_[4] = quad_vbo_[3];
            vbo_[5] = quad_vbo_[0];
            vbo_+=6;

            // Reset quad buffer
            quad_vbo_ptr_ = quad_vbo_;

        } else {
            quad_vbo_ptr_++;
        }

    // All other primitives write directly to GPU mem
    } else {
        vbo_++;
    }
#else
    vbo_++;
#endif
    vertex_num_++;
}

/// Draws a primitive from the previously decoded vertex array
void RendererGL3::EndPrimitive() {
    f32*    gmtx = XF_GEOMETRY_MATRIX;
    //f32*    pmtx = XF_POSITION_MATRIX;
    f32     gmtx44[16];

    // convert 4x3 ode to gl 4x4
    /*if (VCD_PMIDX) {
        gmtx44[0]  = 1;   gmtx44[1]  = 0;   gmtx44[2]  = 0;  gmtx44[3]  = 0;
        gmtx44[4]  = 0;   gmtx44[5]  = 1;   gmtx44[6]  = 0;  gmtx44[7]  = 0;
        gmtx44[8]  = 0;   gmtx44[9]  = 0;   gmtx44[10] = 1;  gmtx44[11] = 0;
        gmtx44[12] = 0;   gmtx44[13] = 0;   gmtx44[14] = 0;  gmtx44[15] = 1;
    } else {*/
        gmtx44[0]  = gmtx[0];   gmtx44[1]  = gmtx[4];   gmtx44[2]  = gmtx[8];   gmtx44[3]  = 0;
        gmtx44[4]  = gmtx[1];   gmtx44[5]  = gmtx[5];   gmtx44[6]  = gmtx[9];   gmtx44[7]  = 0;
        gmtx44[8]  = gmtx[2];   gmtx44[9]  = gmtx[6];   gmtx44[10] = gmtx[10];  gmtx44[11] = 0;
        gmtx44[12] = gmtx[3];   gmtx44[13] = gmtx[7];   gmtx44[14] = gmtx[11];  gmtx44[15] = 1;
    //}

    // Do nothing if no data sent
    if (vertex_num_ == 0) {
        return;
    }

    // Update XF matrices
    GLuint m_id = glGetUniformLocation(shader_manager::GetCurrentShaderID(), "projectionMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &gp::g_projection_matrix[0]);

    m_id = glGetUniformLocation(shader_manager::GetCurrentShaderID(), "modelMatrix");
    glUniformMatrix4fv(m_id, 1, GL_FALSE, &gmtx44[0]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glVertexAttribPointer(0, 3, vertex_position_format_, GL_FALSE, sizeof(GXVertex), 
                          reinterpret_cast<void*>(0));
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GXVertex), 
                          reinterpret_cast<void*>(12));
    
    // When quads, compensate for extra triangles (4 vertices->6)
#ifndef USE_GEOMETRY_SHADERS
    if (prim_type_ == GX_QUADS) {
        vertex_num_*=1.5;
    }
#endif

    glDrawArrays(gl_prim_type_, vbo_write_offset_, vertex_num_);
    vbo_write_offset_ += vertex_num_;

    _ASSERT_MSG(TVIDEO, (vbo_write_offset_ < VBO_MAX_VERTS), 
                "VBO is filled up! There is either a bug or it must be > %dMB!", 
                (VBO_SIZE / 1048576));

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}


/// Sets the renderer viewport location, width, and height
void RendererGL3::SetViewport(int x, int y, int width, int height) {
    glViewport(x, ((480 - (y + height))), width, height);
}

/// Sets the renderer depthrange, znear and zfar
void RendererGL3::SetDepthRange(double znear, double zfar) {
    glDepthRange(znear, zfar);
}

/// Sets the renderer depth test mode
void RendererGL3::SetDepthTest() {
}

/// Sets the renderer culling mode
void RendererGL3::SetCullMode() {
}

/// Prints some useful debug information to the screen
void RendererGL3::PrintDebugStats() {
	static u32 swaps = 0, last = 0;
    static f32 fps = 0;
    static char str[255];

	u32 t = SDL_GetTicks();
	swaps++;
		
	if(t - last > 500) {
		fps = 1000.0f * swaps / (t - last);
		swaps = 0;
		last = t;
	}

    f32 read_pos    = 100.0f * ((f32)(gp::g_fifo_read_ptr - gp::g_fifo_buffer)) / FIFO_SIZE;
    f32 write_pos   = 100.0f * ((f32)gp::g_fifo_write_ptr) / FIFO_SIZE;
    
    sprintf(str, 
            "Framerate    : %02.02f\n"
            "Vertex count : %d\n"
            "FIFO in pos  : %02.01f%%\n"
            "FIFO out pos : %02.01f%%", 
            fps, vbo_write_offset_, write_pos, read_pos);

    g_raster_font->printMultilineText(str, -0.98, 0.95, 0, 200, 400);
}

/// Swap buffers (render frame)
void RendererGL3::SwapBuffers() {

    // FBO->Window copy
    RenderFramebuffer();

    // Swap buffers
    render_window_->SwapBuffers();

    // Update input
    input_common::g_user_input->PollEvent();

    // Switch back to EFB and clear
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset VBO position
    vbo_write_offset_ = 0;
}

/// Set the window of the emulator
void RendererGL3::SetWindow(EmuWindow* window) {
    render_window_ = window;
}


/** 
 * @brief Blits the EFB to the specified destination buffer
 * @param framebuffer Destination framebuffer
 */
void RendererGL3::CopyEFB(kFramebuffer dest) {

    // Render target is destination framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[dest]);
    glViewport(0, 0, resolution_width_, resolution_height_);
  
    // Render source is our EFB
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // Blit
    glBlitFramebuffer(0, 0, resolution_width_, resolution_height_,
                      0, 0, resolution_width_, resolution_height_,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Rebind EFB
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);
}

/// Shutdown the renderer
void RendererGL3::ShutDown() {

    // Framebuffer object
    // ------------------
    glDeleteFramebuffers(MAX_FRAMEBUFFERS, fbo_);

    // TODO(ShizZy): There is a lot more stuff we should be cleaning up here...
}

/// Renders the XFB the screen
void RendererGL3::RenderFramebuffer() {

    // Render target is default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, resolution_width_, resolution_height_);
  
    // Render source is our XFB
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_[kFramebuffer_VirtualXFB]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // Blit
    glBlitFramebuffer(0, 0, resolution_width_, resolution_height_,
                      0, 0, resolution_width_, resolution_height_,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // FPS stuff
    PrintDebugStats();

    // Rebind EFB
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);
}

/// Initialize the primary framebuffer used for drawing
void RendererGL3::InitFramebuffer() {

    // Init the FBOs
    // -------------

    glGenFramebuffers(MAX_FRAMEBUFFERS, fbo_); // Generate primary framebuffer
    glGenRenderbuffers(MAX_FRAMEBUFFERS, fbo_rbo_); // Generate primary RBOs
    glGenRenderbuffers(MAX_FRAMEBUFFERS, fbo_depth_buffers_); // Generate primary depth buffer

    for (int i = 0; i < MAX_FRAMEBUFFERS; i++) {
        // Generate color buffer storage
        glBindRenderbuffer(GL_RENDERBUFFER, fbo_rbo_[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, resolution_width_, resolution_height_);

        // Generate depth buffer storage
        glBindRenderbuffer(GL_RENDERBUFFER, fbo_depth_buffers_[i]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, resolution_width_, 
                              resolution_height_);
    
        // Attach the buffers
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[i]);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, fbo_depth_buffers_[i]);
        glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, fbo_rbo_[i]);

        // Check for completeness
        if (GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)) {
            LOG_NOTICE(TGP, "framebuffer(S) initialized ok");
        } else {
            LOG_ERROR(TVIDEO, "couldn't create OpenGL frame buffer");
            exit(1);
        } 
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind our frame buffer(s)
} 

/// Initialize the renderer and create a window
void RendererGL3::Init() {

    glClearColor(0.30f, 0.05f, 0.65f, 1.0f); // GameCube purple :-)
    glEnable(GL_TEXTURE_2D); // Enable texturing so we can bind our frame buffer texture  
    //glEnable(GL_DEPTH_TEST); // Enable depth testing
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glFrontFace(GL_CW);
    glShadeModel(GL_SMOOTH);

    GLenum err = glewInit();
	if (GLEW_OK != err) {
        LOG_ERROR(TVIDEO, " Failed to initialize GLEW! Exiting...");
        exit(E_ERR);
	}

    // Initialize vertex buffers
    // -------------------------

    glGenBuffers(1, &vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, NULL, GL_DYNAMIC_DRAW);

    // Allocate a buffer for storing a quad in CPU mem
    quad_vbo_ = (GXVertex*) malloc(3 * sizeof(GXVertex));

    // Initialize everything else
    // --------------------------

    InitFramebuffer();

    shader_manager::Init();

    g_raster_font = new RasterFont();
}