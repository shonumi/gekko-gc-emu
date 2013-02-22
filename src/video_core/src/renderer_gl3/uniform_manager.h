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

#include <GL/glew.h>

#include "common.h"
#include "xf_mem.h"
#include "gx_types.h"

/// Struct to represent a Vec4 in GLSL
struct Vec4 {
    union {
        struct {
            f32 r, g, b, a;
        };
        struct {
            f32 x, y, z, w;
        };
        f32 mem[4];
    };
    Vec4(const f32 x = 0, const f32 y = 0, const f32 z = 0, const f32 w = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
    static Vec4 RGBA8(u32 rgba) {
        return Vec4((f32)((rgba & 0xFF000000) >> 24) / 255.0f,
                    (f32)((rgba & 0x00FF0000) >> 16) / 255.0f,
                    (f32)((rgba & 0x0000FF00) >>  8) / 255.0f,
                    (f32)((rgba & 0x000000FF) >>  0) / 255.0f);
    }
    inline bool operator == (const Vec4 &val) const {
        return (x == val.x && y  == val.y && z == val.z &&  w == val.w);
    }
};

class UniformManager {

public:

    static const int kMaxUniformRegions = 1024;     ///< Maximum number of regions to invalidate

    UniformManager();
    ~UniformManager() {}

    /// Struct to represent a memory region in a UBO
    struct UniformRegion {
        u8* start_addr;
        int length;
        int offset;
    };

    // Uniform structures - These are structs used in the shader
    // ---------------------------------------------------------

    struct UniformStuct_Light {
        Vec4 col; 
        Vec4 cos_atten; 
        Vec4 dist_atten; 
        Vec4 pos; 
        Vec4 dir;

        inline bool operator == (const UniformStuct_Light& val) const {
            return (col == val.col && cos_atten == val.cos_atten && dist_atten == val.dist_atten && 
                    pos == val.pos && dir == val.dir);
        }
    };

    struct UniformStruct_VertexState {
        f32 cp_pos_dqf; 
        int cp_pos_matrix_offset;

        int pad0;
        int pad1;

        f32 cp_tex_dqf[8];
        int cp_tex_matrix_offset[8];

        f32 projection_matrix[4][4];

        Vec4 material_color[2];
        Vec4 ambient_color[2];

        UniformStuct_Light light[kGCMaxLights];

        inline bool operator == (const UniformStruct_VertexState &val) const {
            return (
                cp_pos_dqf              == val.cp_pos_dqf &&
                cp_pos_matrix_offset    == val.cp_pos_matrix_offset &&
                cp_tex_dqf[0]           == val.cp_tex_dqf[0] &&
                cp_tex_dqf[1]           == val.cp_tex_dqf[1] &&
                cp_tex_dqf[2]           == val.cp_tex_dqf[2] &&
                cp_tex_dqf[3]           == val.cp_tex_dqf[3] &&
                cp_tex_dqf[4]           == val.cp_tex_dqf[4] &&
                cp_tex_dqf[5]           == val.cp_tex_dqf[5] &&
                cp_tex_dqf[6]           == val.cp_tex_dqf[6] &&
                cp_tex_dqf[7]           == val.cp_tex_dqf[7] &&
                cp_tex_matrix_offset[0] == val.cp_tex_matrix_offset[0] &&
                cp_tex_matrix_offset[1] == val.cp_tex_matrix_offset[1] &&
                cp_tex_matrix_offset[2] == val.cp_tex_matrix_offset[2] &&
                cp_tex_matrix_offset[3] == val.cp_tex_matrix_offset[3] &&
                cp_tex_matrix_offset[4] == val.cp_tex_matrix_offset[4] &&
                cp_tex_matrix_offset[5] == val.cp_tex_matrix_offset[5] &&
                cp_tex_matrix_offset[6] == val.cp_tex_matrix_offset[6] &&
                cp_tex_matrix_offset[7] == val.cp_tex_matrix_offset[7] &&
                projection_matrix[0][0] == val.projection_matrix[0][0] &&
                projection_matrix[0][1] == val.projection_matrix[0][1] &&
                projection_matrix[0][2] == val.projection_matrix[0][2] &&
                projection_matrix[0][3] == val.projection_matrix[0][3] &&
                projection_matrix[1][0] == val.projection_matrix[1][0] &&
                projection_matrix[1][1] == val.projection_matrix[1][1] &&
                projection_matrix[1][2] == val.projection_matrix[1][2] &&
                projection_matrix[1][3] == val.projection_matrix[1][3] &&
                projection_matrix[2][0] == val.projection_matrix[2][0] &&
                projection_matrix[2][1] == val.projection_matrix[2][1] &&
                projection_matrix[2][2] == val.projection_matrix[2][2] &&
                projection_matrix[2][3] == val.projection_matrix[2][3] &&
                projection_matrix[3][0] == val.projection_matrix[3][0] &&
                projection_matrix[3][1] == val.projection_matrix[3][1] &&
                projection_matrix[3][2] == val.projection_matrix[3][2] &&
                projection_matrix[3][3] == val.projection_matrix[3][3] && 
                material_color[0]       == val.material_color[0] &&
                material_color[1]       == val.material_color[1] &&
                ambient_color[0]        == val.ambient_color[0] &&
                ambient_color[1]        == val.ambient_color[1] &&
                light[0]                == val.light[0] && 
                light[1]                == val.light[1] && 
                light[2]                == val.light[2] && 
                light[3]                == val.light[3] && 
                light[4]                == val.light[4] && 
                light[5]                == val.light[5] && 
                light[6]                == val.light[6] && 
                light[7]                == val.light[7]);
        }
    };

    struct UniformStruct_TevState {
        int alpha_func_ref0;
        int alpha_func_ref1;

        f32 dest_alpha;
        int pad1;

        Vec4 color[4];

        inline bool operator == (const UniformStruct_TevState &val) const {
            return (
                alpha_func_ref0 == val.alpha_func_ref0  &&
                alpha_func_ref1 == val.alpha_func_ref1  &&
                dest_alpha      == val.dest_alpha &&
                color[0]        == val.color[0]  &&
                color[1]        == val.color[1]  &&
                color[2]        == val.color[2]  &&
                color[3]        == val.color[3]);
        }
    };

    struct UniformStruct_TevStageParams {
        f32 color_bias;
        f32 color_sub;
        f32 color_scale;

        f32 alpha_bias;
        f32 alpha_sub;
        f32 alpha_scale;

        int pad0;
        int pad1;

        Vec4 konst;

        inline bool operator == (const UniformStruct_TevStageParams &val) const {
            return (
                color_bias      == val.color_bias		&&
                color_sub       == val.color_sub		&&
                color_scale     == val.color_scale		&&
                alpha_bias      == val.alpha_bias		&&
                alpha_sub       == val.alpha_sub		&&
                alpha_scale     == val.alpha_scale		&&
                konst           == val.konst);
        }
    };

    // Uniform blocks - These are mappings of the uniform blocks in the shader
    // -----------------------------------------------------------------------

    struct UniformBlocks {
        /// Vertex shader UBO
        struct _VS_UBO {
            UniformStruct_VertexState state;
            Vec4 tf_mem[kGCMatrixMemSize];
            Vec4 nrm_mem[kGCNormalMemSize]; // vec4 for tight packing (normal is just using xyz)
        } vs_ubo;

        /// Fragment shader UBO
        struct _FS_UBO {
            UniformStruct_TevState tev_state;
            UniformStruct_TevStageParams tev_stages[kGCMaxTevStages];
        } fs_ubo;
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

    /**
     * Attach a shader to the Uniform Manager for uniform binding
     * @param shader Compiled GLSL shader program
     */
    void AttachShader(GLuint shader);

    /// Initialize the shader manager
    void Init(GLuint default_shader);

    GLuint	ubo_fs_handle_;         ///< Fragment shader UBO handle
    GLuint	ubo_vs_handle_;         ///< Vertex shader UBO handle

    GLuint  ubo_fs_block_index_;    ///< Fragment shader UBO block index
    GLuint  ubo_vs_block_index_;    ///< Vertex shader UBO block index

private:

    /**
     * Internal function to invalidate a region of the UBO
     * @param region Uniform region to invalidate with function call
     */
    void InvalidateRegion(UniformRegion region);

    /// Updates any staged data to be written in the next uniform data upload
    void UpdateStagedData();

    /**
     * Lookup the TEV konst color value for a given kont selector
     * @param sel Konst selector corresponding to the desired konst color
     */
    Vec4 GetTevKonst(int sel);

    int             last_invalid_region_xf_;
    int             last_invalid_region_nrm_;
    UniformRegion   invalid_regions_xf_[kMaxUniformRegions];
    UniformRegion   invalid_regions_nrm_[kMaxUniformRegions];

    Vec4 konst_[4];
};

#endif // VIDEO_CORE_UNIFORM_MANAGER_H_
