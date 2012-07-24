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
#include "config.h"

#include "input_common.h"

#include "fifo.h"
#include "vertex_loader.h"
#include "bp_mem.h"
#include "cp_mem.h"
#include "xf_mem.h"

#include "renderer_gl3.h"
#include "shader_manager.h"
#include "raster_font.h"

RasterFont* g_raster_font;

/// OpenGL color source factors
static const GLenum g_src_factors[8] =
{
    GL_ZERO,
    GL_ONE,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA
};

/// OpenGL color destination factors
static const GLenum g_dst_factors[8] = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA
};

/// OpenGL Z compare functions factors
static const GLenum g_compare_funcs[8] = {
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,
    GL_ALWAYS
};

/// OpenGL color logic opcodes
static const GLenum g_logic_opcodes[16] = {
    GL_CLEAR,
    GL_AND,
    GL_AND_REVERSE,
    GL_COPY,
    GL_AND_INVERTED,
    GL_NOOP,
    GL_XOR,
    GL_OR,
    GL_NOR,
    GL_EQUIV,
    GL_INVERT,
    GL_OR_REVERSE,
    GL_COPY_INVERTED,
    GL_OR_INVERTED,
    GL_NAND,
    GL_SET
};

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
    vertex_color_cur_ = 0;
    vertex_texcoord_cur_ = 0;
    last_mode_ = 0;
    for (int i = 0; i < kNumTextures; i++) {
        vertex_texcoord_format_[i] = 0;
        vertex_texcoord_enable_[i] = 0;
        vertex_texcoord_format_size_[i] = 0;
        vertex_texcoord_component_count_[i] = (GXCompCnt)0;
    }
    vertex_num_ = 0;
    blend_mode_ = 0;
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

    // Beginning of primitive - reset vertex info
    vertex_num_ = 0;
    memset(vertex_texcoord_enable_, 0, sizeof(vertex_texcoord_enable_));

    // Set the renderer primitive type
    prim_type_ = prim;
    gl_prim_type_ = gl_types[(prim >> 3) - 16];

    // If no data sent, we are done here
    if (0 == count) {
        return;
    }

    // Set the shader manager primitive type (only used for geometry shaders)
    shader_manager::SetPrimitive(prim);

    // Update shader manager uniforms
    shader_manager::UpdateUniforms();

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
 * Set the type of color vertex data - type is always RGB8/RGBA8, just set count
 * @param color Which color to configure (0 or 1)
 * @param count Color data count (e.g. GX_CLR_RGBA)
 */
void RendererGL3::VertexColor_SetType(int color, GXCompCnt count) {
    vertex_color_cur_ = color;
}

/**
 * Send a vertex color to the renderer (RGB8 or RGBA8, as set by VertexColor_SetType)
 * @param color Color to send, packed as RRGGBBAA or RRGGBB00
 */
void RendererGL3::VertexColor_Send(u32 rgba) {
    (*vbo_ptr_)->color[vertex_color_cur_] = rgba;
}

/**
 * Set the type of texture coordinate vertex data
 * @param texcoord 0-7 texcoord to set type of
 * @param type Texcoord data type (e.g. GX_F32)
 * @param count Texcoord data count (e.g. GX_TEX_ST)
 */

void RendererGL3::VertexTexcoord_SetType(int texcoord, GXCompType type, GXCompCnt count) {
    const GLuint gl_types[5] = {GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT, GL_FLOAT};
    const GLuint gl_types_size[5] = {1, 1, 2, 2, 4};
    vertex_texcoord_cur_ = texcoord;
    vertex_texcoord_format_[texcoord] = gl_types[type];
    vertex_texcoord_format_size_[texcoord] = gl_types_size[type];
    vertex_texcoord_component_count_[texcoord] = count;
    vertex_texcoord_enable_[texcoord] = 1;
}

/**
 * Send a texcoord vector to the renderer as 32-bit floating point
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendFloat(f32* vec) {
    f32* ptr = (f32*)&(*vbo_ptr_)->texcoords[vertex_texcoord_cur_ << 1];
    ptr[0] = vec[0];
    ptr[1] = vec[1];
}

/**
 * Send a texcoord vector to the renderer as 16-bit short (signed or unsigned)
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendShort(u16* vec) {
    u16* ptr = (u16*)&(*vbo_ptr_)->texcoords[vertex_texcoord_cur_ << 1];
    ptr[0] = vec[0];
    ptr[1] = vec[1];
}

/**
 * Send a texcoord vector to the renderer as 8-bit byte (signed or unsigned)
 * @param vec Texcoord vector, XY or XYZ, depending on VertexTexcoord_SetType
 */
void RendererGL3::VertexTexcoord_SendByte(u8* vec) {
    u8* ptr = (u8*)&(*vbo_ptr_)->texcoords[vertex_texcoord_cur_ << 1];
    ptr[0] = vec[0];
    ptr[1] = vec[1];
}

/**
 * @brief Sends position and texcoord matrix indices to the renderer
 * @param pm_idx Position matrix index
 * @param tm_idx Texture matrix indices,
 */
void RendererGL3::Vertex_SendMatrixIndices(u8 pm_idx, u8 tm_idx[]) {
    (*vbo_ptr_)->pm_idx = pm_idx;
   /* (*vbo_ptr_)->tm_idx[0] = tm_idx[0];
    (*vbo_ptr_)->tm_idx[1] = tm_idx[1];
    (*vbo_ptr_)->tm_idx[2] = tm_idx[2];
    (*vbo_ptr_)->tm_idx[3] = tm_idx[3];
    (*vbo_ptr_)->tm_idx[4] = tm_idx[4];
    (*vbo_ptr_)->tm_idx[5] = tm_idx[5];
    (*vbo_ptr_)->tm_idx[6] = tm_idx[6];
    (*vbo_ptr_)->tm_idx[7] = tm_idx[7]; */
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

    // Do nothing if no data sent
    if (vertex_num_ == 0) {
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glUnmapBuffer(GL_ARRAY_BUFFER);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, vertex_position_format_, GL_FALSE, sizeof(GXVertex), 
                          reinterpret_cast<void*>(0));
    // Color 0
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GXVertex), 
                          reinterpret_cast<void*>(12));

    // TexCoords
    if (vertex_texcoord_enable_[0]) {
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, vertex_texcoord_format_[0], GL_FALSE, sizeof(GXVertex), reinterpret_cast<void*>(56));
    }
        
    // Position matrix index
    if (VCD_PMIDX) {
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GXVertex), reinterpret_cast<void*>(120));
    }
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
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(8);
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
void RendererGL3::SetDepthMode() {
    if (gp::g_bp_regs.zmode.test_enable) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(gp::g_bp_regs.zmode.update_enable ? GL_TRUE : GL_FALSE);
        glDepthFunc(g_compare_funcs[gp::g_bp_regs.zmode.function]);
    } else {
        // If the test is disabled write is disabled too
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }
}

/// Sets the renderer generation mode
void RendererGL3::SetGenerationMode() {
    // None, CCW, CW, CCW
    if (gp::g_bp_regs.genmode.cull_mode > 0) {
        glEnable(GL_CULL_FACE);
        glFrontFace(gp::g_bp_regs.genmode.cull_mode == 2 ? GL_CCW : GL_CW);
    } else {
        glDisable(GL_CULL_FACE);
    }
}

/** 
 * @brief Sets the renderer blend mode
 * @param blend_mode_ Forces blend mode to update
 */
void RendererGL3::SetBlendMode(bool force_update) {
    u32 temp = gp::g_bp_regs.cmode0.subtract << 2;

    if (gp::g_bp_regs.cmode0.subtract) {
        temp |= 0x0049;                             // Enable blending src 1 dst 1
    } else if (gp::g_bp_regs.cmode0.blend_enable) {
        temp |= 1;                                  // Enable blending
        temp |= gp::g_bp_regs.cmode0.src_factor << 3;
        temp |= gp::g_bp_regs.cmode0.dst_factor << 6;
    }

    u32 changes = force_update ? 0xFFFFFFFF : temp ^ blend_mode_;

    // Blend enable change
    if (changes & 1) {
        (temp & 1) ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
    }

    if (changes & 4) {
        glBlendEquation(temp & 4 ? GL_FUNC_REVERSE_SUBTRACT : GL_FUNC_ADD);
    }

    if (changes & 0x1F8) {
        glBlendFunc(g_src_factors[(temp >> 3) & 7], g_dst_factors[(temp >> 6) & 7]);
    }

    blend_mode_ = temp;
}

/// Sets the renderer logic op mode
void RendererGL3::SetLogicOpMode() {
    if (gp::g_bp_regs.cmode0.logicop_enable && gp::g_bp_regs.cmode0.logic_mode != 3) {
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(g_logic_opcodes[gp::g_bp_regs.cmode0.logic_mode]);
    } else {
        glDisable(GL_COLOR_LOGIC_OP);
    }
}

/// Sets the renderer dither mode
void RendererGL3::SetDitherMode() {
    if (gp::g_bp_regs.cmode0.dither) {
        glEnable(GL_DITHER);
    } else {
        glDisable(GL_DITHER);
    }
}

/// Sets the renderer color mask mode
void RendererGL3::SetColorMask() {
	GLenum cmask = gp::g_bp_regs.cmode0.color_update ? GL_TRUE : GL_FALSE;
    GLenum amask = GL_FALSE;

    // Enable alpha channel if supported by the current EFB format
	if (gp::g_bp_regs.cmode0.alpha_update && 
        (gp::g_bp_regs.zcontrol.pixel_format == gp::BP_PIXELFORMAT_RGBA6_Z24)) {
		amask = GL_TRUE;
    }
	glColorMask(cmask,  cmask,  cmask,  amask);
}


/**
 * @brief Set a specific render mode
 * @param flag Render flag mode to set
 */
void RendererGL3::SetMode(kRenderMode flags) {
    if(flags & kRenderMode_ZComp) {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    }
    if(flags & kRenderMode_Multipass) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);          
        glDepthFunc(GL_EQUAL);
    }
    if (flags & kRenderMode_UseDstAlpha) {
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        glDisable(GL_BLEND);
    }
    last_mode_ |= flags;
}

/// Restore the render mode
void RendererGL3::RestoreMode() {
    if(last_mode_ & kRenderMode_ZComp) {
        SetColorMask();
    }
    if(last_mode_ & kRenderMode_Multipass) {
        SetDepthMode();
    }
    if (last_mode_ & kRenderMode_UseDstAlpha) {
        SetColorMask();
        if (gp::g_bp_regs.cmode0.blend_enable || gp::g_bp_regs.cmode0.subtract) {
            glEnable(GL_BLEND);
        }
    }
    last_mode_ = kRenderMode_None;
}

/// Reset the full renderer API to the NULL state
void RendererGL3::ResetRenderState() {
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

/// Restore the full renderer API state - As the game set it
void RendererGL3::RestoreRenderState() {
    // TODO(ShizZy):
    //  - Scissor mode
    //  - Viewport
    glDisable(GL_SCISSOR_TEST);
    SetGenerationMode();
    SetColorMask();
    SetDepthMode();
    SetBlendMode(true);
    if (common::g_config->current_renderer_config().enable_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
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

    ResetRenderState();

    // FBO->Window copy
    RenderFramebuffer();

    // Swap buffers
    render_window_->SwapBuffers();

    // Switch back to EFB and clear
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset VBO position
    vbo_write_offset_ = 0;

    RestoreRenderState();
}

/// Set the window of the emulator
void RendererGL3::SetWindow(EmuWindow* window) {
    render_window_ = window;
}


/** 
 * @brief Blits the EFB to the specified destination buffer
 * @param dest Destination framebuffer
 * @param rect EFB rectangle to copy
 * @param dest_width Destination width in pixels 
 * @param dest_height Destination height in pixels
 */
void RendererGL3::CopyEFB(kFramebuffer dest, Rect rect, u32 dest_width, u32 dest_height) {

    ResetRenderState();

    // Render target is destination framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[dest]);
    glViewport(0, 0, resolution_width_, resolution_height_);
  
    // Render source is our EFB
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    // Blit
    glBlitFramebuffer(rect.x, rect.y, rect.width, rect.height,
                      0, 0, dest_width, dest_height,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Rebind EFB
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_[kFramebuffer_EFB]);

    RestoreRenderState();
}

/**
 * @brief Clear the screen
 * @param rect Screen rectangle to clear
 * @param enable_color Enable color clearing
 * @param enable_alpha Enable alpha clearing
 * @param enable_z Enable depth clearing
 * @param color Clear color
 * @param z Clear depth
 */
void RendererGL3::Clear(Rect rect, bool enable_color, bool enable_alpha, bool enable_z, u32 color, 
    u32 z) {
    
    GLboolean const color_mask = enable_color ? GL_TRUE : GL_FALSE;
    GLboolean const alpha_mask = enable_alpha ? GL_TRUE : GL_FALSE;

    ResetRenderState();

    // Clear color
    glColorMask(color_mask,  color_mask,  color_mask,  alpha_mask);
    glClearColor(float((color >> 16) & 0xFF) / 255.0f, float((color >> 8) & 0xFF) / 255.0f,
        float((color >> 0) & 0xFF) / 255.0f, float((color >> 24) & 0xFF) / 255.0f);

    // Clear depth
    glDepthMask(enable_z ? GL_TRUE : GL_FALSE);
    glClearDepth(float(z & 0xFFFFFF) / float(0xFFFFFF));

    // Specify the rectangle of the EFB to clear
    glEnable(GL_SCISSOR_TEST);
    glScissor(rect.x, rect.y, rect.width, rect.height);

    // Clear it!
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RestoreRenderState();
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
    // FPS stuff
    PrintDebugStats();

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

    render_window_->MakeCurrent();

    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    if (common::g_config->current_renderer_config().enable_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        LOG_ERROR(TVIDEO, " Failed to initialize GLEW! Error message: \"%s\". Exiting...", glewGetErrorString(err));
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
