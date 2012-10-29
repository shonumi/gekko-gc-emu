/**
* Copyright (C) 2005-2012 Gekko Emulator
*
* @file    renderer_gl3.cpp
* @author  ShizZy <shizzy247@gmail.com>
* @date    2012-03-10
*   Implementation of a OpenGL 3.2 renderer
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

#include "video_core.h"
#include "fifo.h"
#include "vertex_manager.h"
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
    memset(texture_cache_, 0, sizeof(texture_cache_));
    resolution_width_ = 640;
    resolution_height_ = 480;
    vbo_handle_ = 0;
    vertex_position_format_ = 0;
    vertex_position_format_size_ = 0;
    vertex_position_component_count_ = (GXCompCnt)0;
    vertex_color_cur_ = 0;
    vertex_color_cur_type_[0] = vertex_color_cur_type_[1] = GX_RGBA8;
    vertex_color_cur_count_[0] = vertex_color_cur_count_[1] = GX_CLR_RGBA;
    vertex_texcoord_cur_ = 0;
    last_mode_ = 0;
    for (int i = 0; i < kGXNumTextures; i++) {
        vertex_texcoord_format_[i] = 0;
        vertex_texcoord_enable_[i] = 0;
        vertex_texcoord_format_size_[i] = 0;
        vertex_texcoord_component_count_[i] = (GXCompCnt)0;
    }
    blend_mode_ = 0;
    render_window_ = NULL;
    generic_shader_id_ = 0;
    prim_type_ = (GXPrimitive)0;
    gl_prim_type_ = 0;
}

/**
* Write data to BP for renderer internal use (e.g. direct to shader)
* @param addr BP register address
* @param data Value to write to BP register
*/
void RendererGL3::WriteBP(u8 addr, u32 data) {
    uniform_manager_->WriteBP(addr, data);
}

/**
* Write data to CP for renderer internal use (e.g. direct to shader)
* @param addr CP register address
* @param data Value to write to CP register
*/
void RendererGL3::WriteCP(u8 addr, u32 data) {
}

/**
* Write data to XF for renderer internal use (e.g. direct to shader)
* @param addr XF address
* @param length Length (in 32-bit words) to write to XF
* @param data Data buffer to write to XF
*/
void RendererGL3::WriteXF(u16 addr, int length, u32* data) {
    uniform_manager_->WriteXF(addr, length, data);
}

/**
 * Used to signal to the render that a region in XF is required by a primitive
 * @param index Vector index in XF memory that is required
 */
void RendererGL3::VertexPosition_UseIndexXF(u8 index) {
}

/**
* Adds a new texturer to the renderer (must be 32-bit RGBA)
* @param width Width of texture in pixels
* @param height Height of texture in pixels
* @param hash A unique hash of the texture, to be used as an ID
* @param data Buffer of raw texture data stored in correct format
*/
void RendererGL3::AddTexture(u16 width, u16 height, u32 hash, u8* data) {
    int id = hash & (MAX_CACHED_TEXTURES - 1);
    glGenTextures(1, &texture_cache_[id]);
    glBindTexture(GL_TEXTURE_2D, texture_cache_[id]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

/**
* Sets texture parameters for the selected texture (filtering, LOD, etc.)
* @param num Texture number to set parameters for (0-7)
*/
void RendererGL3::SetTextureParameters(int num) {
    int set = (num & 4) >> 2;
    int index = num & 7; 
    gp::BPTexMode0 tex_mode_0 = gp::g_bp_regs.tex[set].mode_0[index];
    gp::BPTexMode1 tex_mode_1 = gp::g_bp_regs.tex[set].mode_1[index];

    static const GLint gl_tex_wrap[4] = {
        GL_CLAMP_TO_EDGE,
        GL_REPEAT,
        GL_MIRRORED_REPEAT,
        GL_REPEAT
    };
    static const GLint gl_mag_filter[2] = {
        GL_NEAREST,
        GL_LINEAR
    };
    static const GLint gl_min_filter[8] = {
        GL_NEAREST,
        GL_NEAREST_MIPMAP_NEAREST,
        GL_NEAREST_MIPMAP_LINEAR,
        GL_NEAREST,
        GL_LINEAR,
        GL_LINEAR_MIPMAP_NEAREST,
        GL_LINEAR_MIPMAP_LINEAR,
        GL_LINEAR
    };
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_mag_filter[tex_mode_0.mag_filter]);
    /* TODO(ShizZy): Replace this code. Works sortof for autogenerated mip maps, but it's deprecated
            OpenGL. Currently, forward compatability is enabled, so anything deprecated will not work.
    if (tex_mode_0.use_mipmaps()) {
        glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
            gl_min_filter[tex_mode_0.min_filter & 7]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, tex_mode_1.min_lod >> 4);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, tex_mode_1.max_lod >> 4);
        glTexEnvf(GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, (tex_mode_0.lod_bias / 31.0f));
    } else {*/
        glTexParameterf(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
            (tex_mode_0.min_filter >= 4) ? GL_LINEAR : GL_NEAREST);
    //}
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_tex_wrap[tex_mode_0.wrap_s]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_tex_wrap[tex_mode_0.wrap_t]);

    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2);
}

/**
* Binds a texture that was previously added to the renderer via AddTexture
* @param hash The unique hash of the texture to bind
* @param num Number of texture to bind to (0-7)
* @return True if bind succeeded, false if failed
*/
bool RendererGL3::BindTexture(u32 hash, int num) {
    int id = hash & (MAX_CACHED_TEXTURES - 1);

    glActiveTexture(GL_TEXTURE0 + num);

    if (texture_cache_[id]) {
        glBindTexture(GL_TEXTURE_2D, texture_cache_[id]);
        return true;
    }
    return false;
}

/**
* Begin renderering of a primitive
* @param prim Primitive type (e.g. GX_TRIANGLES)
* @param count Number of vertices to be drawn (used for appropriate memory management, only)
* @param vbo Pointer to VBO, which will be set by API in this function
* @param vbo_offset Offset into VBO to use (in bytes)
*/
void RendererGL3::BeginPrimitive(GXPrimitive prim, int count, GXVertex** vbo, u32 vbo_offset) {
    // GL_QUADS is only suppported in compatibility mode. This is emulated in software by
    // converting to triangles (therefore, it should never be used). Alternatively, it works 
    // pretty well to emulate GX_QUADS via GL_LINES_ADJACENCY in a geometry shader.
    static GLenum gl_types[8] = {GL_QUADS, 0, GL_TRIANGLES, GL_TRIANGLE_STRIP, 
        GL_TRIANGLE_FAN, GL_LINES,  GL_LINE_STRIP, GL_POINTS};

    // Beginning of primitive - reset vertex info
    memset(vertex_texcoord_enable_, 0, sizeof(vertex_texcoord_enable_));

    // Set the renderer primitive type
    prim_type_ = prim;
    gl_prim_type_ = gl_types[(prim >> 3) - 16];

    // If no data sent, we are done here
    if (0 == count) {
        return;
    }

    // Update shader manager uniforms
    shader_manager::UpdateUniforms();

    uniform_manager_->ApplyChanges();

    // Bind pointers to buffers
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);

    // Map CPU to GPU mem
    static GLbitfield access_flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT |
        GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
    *vbo = (GXVertex*)glMapBufferRange(GL_ARRAY_BUFFER, (vbo_offset * sizeof(GXVertex)), 
        (count * sizeof(GXVertex)), access_flags);
    if (vbo == NULL) {
        LOG_ERROR(TVIDEO, "Unable to map vertex buffer object to system mem!");
    }
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
* Set the type of color vertex data - type is always RGB8/RGBA8, just set count
* @param color Which color to configure (0 or 1)
* @param type GXCompType color format type
* @param count Color data count (e.g. GX_CLR_RGBA)
*/
void RendererGL3::VertexColor_SetType(int color, GXCompType type, GXCompCnt count) {
    vertex_color_cur_ = color;
    vertex_color_cur_type_[color] = type;
    vertex_color_cur_count_[color] = count;
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

/// Draws a primitive from the previously decoded vertex array
void RendererGL3::EndPrimitive(u32 vbo_offset, u32 vertex_num) {

    // Do nothing if no data sent
    if (vertex_num == 0) {
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
    if (gp::g_cp_regs.vcd_lo[0].pos_midx_enable) {
        glEnableVertexAttribArray(8);
        glVertexAttribPointer(8, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(GXVertex), reinterpret_cast<void*>(120));
    }

    glDrawArrays(gl_prim_type_, vbo_offset, vertex_num);

    _ASSERT_MSG(TVIDEO, (vbo_offset < VBO_MAX_VERTS), 
        "VBO is full! There is either a bug or it must be > %dMB!", 
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
* Sets the renderer blend mode
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

/// Sets the scissor box
void RendererGL3::SetScissorBox() {
    const int x_ofs = gp::g_bp_regs.scissor_offset.x * 2 - 342;
    const int y_ofs = gp::g_bp_regs.scissor_offset.y * 2 - 342;

    GLint left = CLAMP((gp::g_bp_regs.scissor_top_left.x - x_ofs - 342), 0, 640);
    GLint top = CLAMP((gp::g_bp_regs.scissor_top_left.y - y_ofs - 342), 0, 480);
    GLsizei right = CLAMP((gp::g_bp_regs.scissor_bottom_right.x - x_ofs - 341), 0, 640);
    GLsizei bottom = CLAMP((gp::g_bp_regs.scissor_bottom_right.y - y_ofs - 341), 0, 480);

    if (left > right) left = right;
    if (top > bottom) top = bottom;

    //glScissor(left, top, right-left, bottom-top);
    // Is this right?? or should it be:
    //glScissor(left, top, right, bottom);
    glScissor(0, 0, 640, 480);
}

/**
* Sets the line and point size
* @param line_width Line width to use
* @param point_size Point size to use
*/
void RendererGL3::SetLinePointSize(f32 line_width, f32 point_size) {
}

/**
* Set a specific render mode
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
    //  - Viewport
    SetGenerationMode();
    glEnable(GL_SCISSOR_TEST);
    SetScissorBox();
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
    /*
    f32 read_pos    = 100.0f * ((f32)(gp::g_fifo_read_ptr - gp::g_fifo_buffer)) / FIFO_SIZE;
    f32 write_pos   = 100.0f * ((f32)gp::g_fifo_write_ptr) / FIFO_SIZE;


    sprintf(str, 
    "Framerate    : %02.02f\n"
    "Vertex count : %d\n"
    "FIFO in pos  : %02.01f%%\n"
    "FIFO out pos : %02.01f%%", 
    fps, 0, write_pos, read_pos);
    */
    sprintf(str, "%02.02f", fps);
    g_raster_font->printMultilineText(str, -0.98, 0.93, 0, 200, 400);
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

    RestoreRenderState();
}

/// Set the window of the emulator
void RendererGL3::SetWindow(EmuWindow* window) {
    render_window_ = window;
}

/** 
* Blits the EFB to the specified destination buffer
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
* Clear the screen
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


    render_window_->MakeCurrent();
    glShadeModel(GL_SMOOTH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glStencilFunc(GL_ALWAYS, 0, 0);
    glBlendFunc(GL_ONE, GL_ONE);

    glViewport(0, 0, 640, 480);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDepthFunc(GL_LEQUAL);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glDisable(GL_STENCIL_TEST);
    glEnable(GL_SCISSOR_TEST);

    glScissor(0, 0, 640, 480);
    glClearDepth(1.0f);

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        LOG_ERROR(TVIDEO, " Failed to initialize GLEW! Error message: \"%s\". Exiting...", 
            glewGetErrorString(err));
        exit(E_ERR);
    }

    // Initialize primary VBO
    // ----------------------

    glGenBuffers(1, &vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, VBO_SIZE, NULL, GL_DYNAMIC_DRAW);

    // Initialize everything else
    // --------------------------

    InitFramebuffer();

    shader_manager::Init();

    uniform_manager_ = new UniformManager();
    uniform_manager_->Init();

    g_raster_font = new RasterFont();

    LOG_NOTICE(TGP, "GL_VERSION: %s\n", glGetString(GL_VERSION));
}
