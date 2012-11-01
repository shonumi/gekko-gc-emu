/**
* Copyright (C) 2005-2012 Gekko Emulator
*
* @file    uniform_manager.h
* @author  ShizZy <shizzy247@gmail.com>
* @date    2012-09-07
* @brief   Managers shader uniform data for the GL3 renderer
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

#ifndef VIDEO_CORE_UNIFORM_MANAGER_H_
#define VIDEO_CORE_UNIFORM_MANAGER_H_

#include "renderer_gl3/renderer_gl3.h"
#include "gx_types.h"

/// Struct to represent a Vec4 GLSL color in a UBO
struct Vec4Color {
    f32 r, g, b, a;

    Vec4Color(const f32 r = 0, const f32 g = 0, const f32 b = 0, const f32 a = 0) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    inline bool operator == (const Vec4Color &val) const {
        return (r == val.r && g  == val.g && b == val.b &&  a == val.a);
    }
};

class UniformManager {

public:

    static const int kMaxUniformRegions = 1024;     ///< Maximum number of regions to invalidate

    UniformManager();
    ~UniformManager() {};

    /// Struct to represent a memory region in a UBO
    struct UniformRegion {
        u8* start_addr;
        int length;
        int offset;
    };

    // Uniform structures - These are structs used in the shader
    // ---------------------------------------------------------

    struct UniformStruct_TevState {
        int alpha_func_ref0;
        int alpha_func_ref1;
        int alpha_func_comp0;
        int alpha_func_comp1;

        int pad0;
        int pad1;
        int pad2;
        int pad3;

        Vec4Color color[4];

        inline bool operator == (const UniformStruct_TevState &val) const {
            return (alpha_func_ref0  == val.alpha_func_ref0  &&
                    alpha_func_ref1  == val.alpha_func_ref1  &&
                    alpha_func_comp0 == val.alpha_func_comp0 &&
                    alpha_func_comp1 == val.alpha_func_comp1 &&

                    color[0]  == val.color[0]  &&
                    color[1]  == val.color[1]  &&
                    color[2]  == val.color[2]  &&
                    color[3]  == val.color[3]);
        }
    };

    struct UniformStruct_TevStageParams {
        int color_sel_a;
        int color_sel_b;
        int color_sel_c;
        int color_sel_d;
        f32 color_bias;
        f32 color_sub;
        int color_clamp;
        f32 color_scale;
        int color_dest;

        int alpha_sel_a;
        int alpha_sel_b;
        int alpha_sel_c;
        int alpha_sel_d;
        f32 alpha_bias;
        f32 alpha_sub;
        int alpha_clamp;
        f32 alpha_scale;
        int alpha_dest;

        int pad0;
        int pad1;

        Vec4Color konst;

        inline bool operator == (const UniformStruct_TevStageParams &val) const {
            return (color_sel_a     == val.color_sel_a     &&
                    color_sel_b     == val.color_sel_b     &&
                    color_sel_c     == val.color_sel_c     &&
                    color_sel_d     == val.color_sel_d     &&
                    color_bias      == val.color_bias      &&
                    color_sub       == val.color_sub       &&
                    color_clamp     == val.color_clamp     &&
                    color_scale     == val.color_scale     &&
                    color_dest      == val.color_dest      &&

                    alpha_sel_a     == val.alpha_sel_a     &&
                    alpha_sel_b     == val.alpha_sel_b     &&
                    alpha_sel_c     == val.alpha_sel_c     &&
                    alpha_sel_d     == val.alpha_sel_d     &&
                    alpha_bias      == val.alpha_bias      &&
                    alpha_sub       == val.alpha_sub       &&
                    alpha_clamp     == val.alpha_clamp     &&
                    alpha_scale     == val.alpha_scale     &&
                    alpha_dest      == val.alpha_dest      &&

                    konst           == val.konst);
        }

    };

    // Uniform blocks - These are mappings of the uniform blocks in the shader
    // -----------------------------------------------------------------------

    struct UniformBlocks {

        struct XFRegisters {
            u32 pos_mem[0x100];
        } xf_regs;

        struct BPRegisters {
            UniformStruct_TevState tev_state;
            UniformStruct_TevStageParams tev_stages[kGXNumTevStages];
        } bp_regs;

    };

	UniformBlocks __uniform_data_;
	UniformBlocks staged_uniform_data_;

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


    /// Updates the uniform changes
    void ApplyChanges();

    /// Initialize the shader manager
    void Init();

    GLuint	ubo_handle_bp_;
    GLuint	ubo_handle_xf_;

    int invalid_xf_region_[0x40];

private:

    /**
     * Internal function to invalidate a region of the UBO
     * @param region Uniform region to invalidate with function call
     */
    void InvalidateRegion(UniformRegion region);

    /**
     * Lookup the TEV konst color value for a given kont selector
     * @param sel Konst selector corresponding to the desired konst color
     */
    Vec4Color GetTevKonst(int sel);

    int             last_invalid_region_xf_;
    UniformRegion   invalid_regions_xf_[kMaxUniformRegions];

    Vec4Color konst_[4];
};

#endif // VIDEO_CORE_UNIFORM_MANAGER_H_