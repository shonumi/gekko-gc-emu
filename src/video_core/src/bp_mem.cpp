/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    bp_mem.cpp
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-10
 * @brief   Implementation of BP for the graphics processor
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
#include "hash.h"
#include "memory.h"

#include <GL/glew.h>

#include "hw/hw_pe.h"

#include "utils.h"
#include "video_core.h"
#include "vertex_manager.h"
#include "fifo.h"
#include "fifo_player.h"
#include "bp_mem.h"
#include "xf_mem.h"
#include "texture_decoder.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

#undef LOG_DEBUG
#define LOG_DEBUG(x,y, ...)

namespace gp {

BPMemory g_bp_regs; ///< BP memory/registers

/// Sets the scissor box
void BP_SetScissorBox() {
    // The scissor rectangle specifies an area of the screen outside of which all primitives are 
    // culled. This function sets the scissor rectangle in screen coordinates. The screen origin 
    // (xOrigin=0, yOrigin=0) is at the top left corner of the display.
    //
    // The values may be within the range of 0 - 2047 inclusive. Using values that extend beyond the
    // EFB size is allowable since the scissor box may be repositioned within the EFB using 
    // GX_SetScissorBoxOffset().
    const int offset_x = g_bp_regs.scissor_offset.x * 2 - 342;
    const int offset_y = g_bp_regs.scissor_offset.y * 2 - 342;

    Rect rect(g_bp_regs.scissor_top_left.x - offset_x - 342, 
              g_bp_regs.scissor_top_left.y - offset_y - 342,
              g_bp_regs.scissor_bottom_right.x - offset_x - 341, 
              g_bp_regs.scissor_bottom_right.y - offset_y - 341);

    rect.x0_ = CLAMP(rect.x0_, 0, kGCEFBWidth);
    rect.y0_ = CLAMP(rect.y0_, 0, kGCEFBHeight);

    rect.x1_ = CLAMP(rect.x1_, rect.x0_, kGCEFBWidth);
    rect.y1_ = CLAMP(rect.y1_, rect.y0_, kGCEFBHeight);

    Rect target_rect = RendererBase::EFBToRendererRect(rect);

    video_core::g_renderer->SetScissorBox(target_rect);

    XF_UpdateViewport();
}

/// Sets the size of point/line primitives.
void BP_SetLinePointSize() {
    // TODO(ShizZy): scale based on resolution...
    //f32 scale = (g_xf_regs.viewport.wd != 0) ? ((f32)resolution_width_ / kGCEFBWidth) : 1.0f;
    f32 scale = 1.0f;
    f32 line_width = g_bp_regs.line_point_size.linesize * scale / 6.0f;
    f32 point_size = g_bp_regs.line_point_size.pointsize * scale / 6.0f;
    video_core::g_renderer->SetLinePointSize(line_width, point_size);
}

/// Write a BP register
void BP_RegisterWrite(u8 addr, u32 data) {
    LOG_DEBUG(TGP, "BP_LOAD [%02x] = %08x", addr, data);

	// Write data to bp memory
    g_bp_regs.mem[addr] = data;

    // Write to renderer
	//	Note: This should always happen first to compare what was last written!
    video_core::g_renderer->WriteBP(addr, data);

    // Adjust GX globals accordingly
    switch(addr) {
    case BP_REG_GENMODE: // GEN_MODE
        video_core::g_renderer->SetGenerationMode();
        video_core::g_shader_manager->UpdateGenMode(g_bp_regs.genmode);
        break;

    case BP_REG_SCISSORTL: // SU_SCIS0 - Scissorbox Top Left Corner
    case BP_REG_SCISSORBR: // SU_SCIS1 - Scissorbox Bottom Right Corner
    case BP_REG_SCISSOROFFSET:
        BP_SetScissorBox();
        break;

    case BP_REG_LINEPTWIDTH: // SU_LPSIZE - Line and Point Size
        BP_SetLinePointSize();
        break;

    case BP_REG_PE_ZMODE: // PE_ZMODE set z mode
        video_core::g_renderer->SetDepthMode();
        break;

    case BP_REG_PE_CMODE0: // PE_CMODE0 dithering / blend mode/color_update/alpha_update/set_dither
        if (data & 0xFFFF) {
            // Set LogicOp Blending Mode
            if (data & 2) {
                video_core::g_renderer->SetLogicOpMode(g_bp_regs.cmode0);
            }
            // Set Dithering Mode
            if (data & 4) {
                video_core::g_renderer->SetDitherMode(g_bp_regs.cmode0);
            }
            // Set Blending Mode
            if (data & 0xFE1) {
                video_core::g_renderer->SetBlendMode(g_bp_regs.cmode0, g_bp_regs.cmode1, false);
            }
            // Set Color Mask
            if (data & 0x18) {
                video_core::g_renderer->SetColorMask(g_bp_regs.cmode0);
            }
        }
        break;

    case BP_REG_PE_CMODE1: // PE_CMODE1 - Destination alpha
        video_core::g_shader_manager->UpdateFlag(ShaderManager::kFlag_DestinationAlpha, 
                                                 g_bp_regs.cmode1.enable);
        break;

    case BP_REG_PE_CONTROL: // PE_CONTROL comp z location
        //video_core::g_renderer->SetColorMask();
        video_core::g_shader_manager->UpdateEFBFormat((BPPixelFormat)g_bp_regs.zcontrol.pixel_format);
        break;

    case BP_REG_PE_DRAWDONE: // PE_DONE - draw done

	    if (g_bp_regs.mem[0x45] & 0x2) { // enable interrupt
            // Flush vertex buffer
            VertexManager_Flush();

            video_core::g_renderer->SwapBuffers();
            if (fifo_player::IsRecording())
                fifo_player::FrameFinished();
            Fifo_Reset();
            GX_PE_FINISH = 1;
            video_core::g_current_frame++;
            video_core::g_texture_manager->Purge();
        }
        break;

    case BP_REG_PE_TOKEN: // PE_TOKEN
        GX_PE_TOKEN_VALUE = (data & 0xffff);
        break;

    case BP_REG_PE_TOKEN_INT: // PE_TOKEN_INT
        GX_PE_TOKEN_VALUE = (data & 0xffff); 
        GX_PE_TOKEN = 1;
        break;

    case BP_REG_EFB_COPY:  // pe copy execute
        {
            Rect efb_rect(g_bp_regs.efb_top_left.x,
                          g_bp_regs.efb_top_left.y,
                          g_bp_regs.efb_top_left.x + g_bp_regs.efb_height_width.x + 1,
                          g_bp_regs.efb_top_left.y + g_bp_regs.efb_height_width.y + 1);

            BPEFBCopyExec efb_copy_exec;
            efb_copy_exec._u32 = data;

            if (efb_copy_exec.copy_to_xfb) {
                f32 scale_y;
                if (efb_copy_exec.scale_invert) {
                    scale_y = 256.0f / (f32)g_bp_regs.disp_copy_y_scale;
                } else {
                    scale_y = (f32)g_bp_regs.disp_copy_y_scale / 256.0f;
                }
                u32 xfb_height = (u32)(((f32)g_bp_regs.efb_height_width.y + 1.0f) * scale_y);
                u32 xfb_width = g_bp_regs.disp_stride << 4;
                Rect xfb_rect(0, 0, xfb_width, xfb_height);

                video_core::g_renderer->CopyToXFB(RendererBase::EFBToRendererRect(efb_rect), 
                                                  xfb_rect);
            } else {
                video_core::g_texture_manager->CopyEFB(
                    g_bp_regs.efb_copy_addr << 5, 
                    static_cast<BPPixelFormat>(g_bp_regs.zcontrol.pixel_format), 
                    efb_copy_exec,
                    RendererBase::EFBToRendererRect(efb_rect)
                );
            }
            if (efb_copy_exec.clear) {
                bool enable_color = g_bp_regs.cmode0.color_update;
                bool enable_alpha = g_bp_regs.cmode0.alpha_update;
                bool enable_z = g_bp_regs.zmode.update_enable;

                // Disable unused alpha channels
                if (!g_bp_regs.zcontrol.is_efb_alpha_enabled()) {
                    enable_alpha = false;
                }
                if (enable_color || enable_alpha || enable_z) {
                    u32 color = (g_bp_regs.clear_ar << 16) | g_bp_regs.clear_gb;

                    u32 z = g_bp_regs.clear_z;

		            // Drop additional accuracy
		            if (g_bp_regs.zcontrol.pixel_format == kPixelFormat_RGBA6_Z24) {
			            color = format_precision::rgba8_with_rgba6(color);
		            } else if (g_bp_regs.zcontrol.pixel_format == kPixelFormat_RGB565_Z16) {
			            color = format_precision::rgba8_with_rgb565(color);
			            z = format_precision::z24_with_z16(z);
		            }
                    video_core::g_renderer->Clear(
                        RendererBase::EFBToRendererRect(efb_rect),  // Clear rectangle
                        enable_color,                               // Enable color clearing
                        enable_alpha,                               // Enable alpha clearing
                        enable_z,                                   // Enable depth clearing
                        color,                                      // Clear color
                        z);                                         // Clear depth
                }
        
            }
            break;
        }

    case BP_REG_LOADTLUT0: // TX_LOADTLUT0
    case BP_REG_LOADTLUT1: // TX_LOADTLUT1
        {
            u32 cnt = ((g_bp_regs.mem[0x65] >> 10) & 0x3ff) << 5;
	        u32 mem_addr = (g_bp_regs.mem[0x64] & 0x1fffff) << 5;
	        u32 tlut_addr = (g_bp_regs.mem[0x65] & 0x3ff) << 5;

	        memcpy(&tmem[tlut_addr & TMEM_MASK], &Mem_RAM[mem_addr & RAM_MASK], cnt);
            LOG_DEBUG(TGP, "BP-> TX_LOADTLUTx");
            break;
        }
        break;

    // TEV combiner registers
    case BP_REG_TEV_COLOR_ENV + 0:
    case BP_REG_TEV_COLOR_ENV + 2:
    case BP_REG_TEV_COLOR_ENV + 4:
    case BP_REG_TEV_COLOR_ENV + 6:
    case BP_REG_TEV_COLOR_ENV + 8:
    case BP_REG_TEV_COLOR_ENV + 10:
    case BP_REG_TEV_COLOR_ENV + 12:
    case BP_REG_TEV_COLOR_ENV + 14:
    case BP_REG_TEV_COLOR_ENV + 16:
    case BP_REG_TEV_COLOR_ENV + 18:
    case BP_REG_TEV_COLOR_ENV + 20:
    case BP_REG_TEV_COLOR_ENV + 22:
    case BP_REG_TEV_COLOR_ENV + 24:
    case BP_REG_TEV_COLOR_ENV + 26:
    case BP_REG_TEV_COLOR_ENV + 28:
    case BP_REG_TEV_COLOR_ENV + 30:
    case BP_REG_TEV_ALPHA_ENV + 0:
    case BP_REG_TEV_ALPHA_ENV + 2:
    case BP_REG_TEV_ALPHA_ENV + 4:
    case BP_REG_TEV_ALPHA_ENV + 6:
    case BP_REG_TEV_ALPHA_ENV + 8:
    case BP_REG_TEV_ALPHA_ENV + 10:
    case BP_REG_TEV_ALPHA_ENV + 12:
    case BP_REG_TEV_ALPHA_ENV + 14:
    case BP_REG_TEV_ALPHA_ENV + 16:
    case BP_REG_TEV_ALPHA_ENV + 18:
    case BP_REG_TEV_ALPHA_ENV + 20:
    case BP_REG_TEV_ALPHA_ENV + 22:
    case BP_REG_TEV_ALPHA_ENV + 24:
    case BP_REG_TEV_ALPHA_ENV + 26:
    case BP_REG_TEV_ALPHA_ENV + 28:
    case BP_REG_TEV_ALPHA_ENV + 30:
        {
            int stage = (addr & 0x1F) >> 1;
            video_core::g_shader_manager->UpdateTevCombiner(stage, g_bp_regs.combiner[stage]);
        }
        break;

    // TEV order registers
    case BP_REG_TREF + 0:
    case BP_REG_TREF + 1:
    case BP_REG_TREF + 2:
    case BP_REG_TREF + 3:
    case BP_REG_TREF + 4:
    case BP_REG_TREF + 5:
    case BP_REG_TREF + 6:
    case BP_REG_TREF + 7:
        {
            int index = addr - BP_REG_TREF;
            video_core::g_shader_manager->UpdateTevOrder(index, g_bp_regs.tevorder[index]);
        }
        break;

    // Alpha comparison mode
    case BP_REG_ALPHACOMPARE:
        video_core::g_shader_manager->UpdateAlphaFunc(g_bp_regs.alpha_func);
        break;
    }
}

/// Load a texture
void BP_LoadTexture() {
    for (int num = 0; num < kGCMaxActiveTextures; num++) {
        int set = (num & 4) >> 2;
        int index = num & 7;
        for (int stage = 0; stage < kGCMaxTevStages; stage++) {
            if (g_bp_regs.tevorder[stage >> 1].get_texmap(stage) == num) {
                video_core::g_texture_manager->UpdateData(num, g_bp_regs.tex[set].image_0[index],
                    g_bp_regs.tex[set].image_3[index]);
                video_core::g_texture_manager->Bind(num);
                video_core::g_texture_manager->UpdateParameters(num, 
                    g_bp_regs.tex[set].mode_0[index], g_bp_regs.tex[set].mode_1[index]);
                break;
            }
        }
    }
}

/// Initialize BP
void BP_Init() {
    memset(&g_bp_regs, 0, sizeof(g_bp_regs));

    // Clear EFB on startup with alpha of 1.0f
    // TODO(ShizZy): Remove hard coded EFB rect size (still need a video_core or renderer interface
    // for this, actually)
    Rect efb_rect(0, 0, kGCEFBWidth, kGCEFBHeight);
    video_core::g_renderer->Clear(
        RendererBase::EFBToRendererRect(efb_rect),  // Clear rectangle
        true,                                       // Enable color clearing
        true,                                       // Enable alpha clearing
        true,                                       // Enable depth clearing
        0xFF000000,                                 // Clear color, ARGB color black with alpha=1.0f
        0);                                         // Clear depth
}

} // namespace