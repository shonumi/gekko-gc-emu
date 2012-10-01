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

    struct UniformStruct_TevStageParams {
        int color_sel_a;
        int color_sel_b;
        int color_sel_c;
        int color_sel_d;
        int color_bias;
        int color_sub;
        int color_clamp;
        int color_shift;
        int color_dest;

        int alpha_sel_a;
        int alpha_sel_b;
        int alpha_sel_c;
        int alpha_sel_d;
        int alpha_bias;
        int alpha_sub;
        int alpha_clamp;
        int alpha_shift;
        int alpha_dest;

        int pad[2];
    };

    // Uniform blocks - These are mappings of the uniform blocks in the shader
    // -----------------------------------------------------------------------

    struct UniformBlocks {

        struct XFRegisters {
            u32 pos_mem[0x100];
        } xf_regs;

        struct BPRegisters {
            UniformStruct_TevStageParams tev_stage[16];
        } bp_regs;

    } uniform_blocks_;

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

private:

    /**
     * Internal function to invalidate a region of the UBO
     * @param region Uniform region to invalidate with function call
     */
    void InvalidateRegion(UniformRegion region);

    int last_invalid_region_xf_;
    int last_invalid_region_bp_;

    
    UniformRegion invalid_regions_xf_[kMaxUniformRegions];

    int invalid_bp_tev_stages_[16];
    //UniformRegion invalid_regions_bp_[kMaxUniformRegions];
};

#endif // VIDEO_CORE_UNIFORM_MANAGER_H_