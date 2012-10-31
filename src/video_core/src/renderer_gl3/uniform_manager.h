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

class UniformManager {

public:

    static const int kMaxUniformRegions = 1024;     ///< Maximum number of regions to invalidate

    UniformManager();
    ~UniformManager() {};

    struct UniformRegion {
        u8* start_addr;
        int length;
        int offset;
    };

    // Uniform structures - These are structs used in the shader
    // ---------------------------------------------------------

    struct UniformStruct_TevState {
        int num_stages;

        int alpha_func_ref0;
        int alpha_func_ref1;
        int alpha_func_comp0;
        int alpha_func_comp1;

        int pad0;
        int pad1;
        int pad2;

        f32 color[16];
        f32 konst[16];

        inline bool operator == (const UniformStruct_TevState &val) const {
            return (num_stages       == val.num_stages       &&
                    alpha_func_ref0  == val.alpha_func_ref0  &&
                    alpha_func_ref1  == val.alpha_func_ref1  &&
                    alpha_func_comp0 == val.alpha_func_comp0 &&
                    alpha_func_comp1 == val.alpha_func_comp1 &&

                    color[0]  == val.color[0]  &&
                    color[1]  == val.color[1]  &&
                    color[2]  == val.color[2]  &&
                    color[3]  == val.color[3]  &&
                    color[4]  == val.color[4]  &&
                    color[5]  == val.color[5]  &&
                    color[6]  == val.color[6]  &&
                    color[7]  == val.color[7]  &&
                    color[8]  == val.color[8]  &&
                    color[9]  == val.color[9]  &&
                    color[10] == val.color[10] &&
                    color[11] == val.color[11] &&
                    color[12] == val.color[12] &&
                    color[13] == val.color[13] &&
                    color[14] == val.color[14] &&
                    color[15] == val.color[15] &&

                    konst[0]  == val.konst[0]  &&
                    konst[1]  == val.konst[1]  &&
                    konst[2]  == val.konst[2]  &&
                    konst[3]  == val.konst[3]  &&
                    konst[4]  == val.konst[4]  &&
                    konst[5]  == val.konst[5]  &&
                    konst[6]  == val.konst[6]  &&
                    konst[7]  == val.konst[7]  &&
                    konst[8]  == val.konst[8]  &&
                    konst[9]  == val.konst[9]  &&
                    konst[10] == val.konst[10] &&
                    konst[11] == val.konst[11] &&
                    konst[12] == val.konst[12] &&
                    konst[13] == val.konst[13] &&
                    konst[14] == val.konst[14] &&
                    konst[15] == val.konst[15]);
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

		int konst_color_sel;
		int konst_alpha_sel;

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

                    konst_color_sel == val.konst_color_sel &&
                    konst_alpha_sel == val.konst_alpha_sel );
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

    int last_invalid_region_xf_;
    int last_invalid_region_bp_;
    
    UniformRegion invalid_regions_xf_[kMaxUniformRegions];
};

#endif // VIDEO_CORE_UNIFORM_MANAGER_H_