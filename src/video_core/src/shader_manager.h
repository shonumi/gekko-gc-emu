/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_manager.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2013-01-26
 * @brief   Managers shaders
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

#include "types.h"
#include "gx_types.h"
#include "bp_mem.h"
#include "vertex_loader.h"
#include "hash.h"
#include "hash_container.h"

#ifndef VIDEO_CORE_SHADER_MANAGER_H_
#define VIDEO_CORE_SHADER_MANAGER_H_

class ShaderManager {
public:
    enum Flag {
        kFlag_None                      = 0x00000000,
        kFlag_MatrixIndexed_Position    = 0x00000001,
        kFlag_MatrixIndexed_TexCoord_0  = 0x00000002,
        kFlag_MatrixIndexed_TexCoord_1  = 0x00000004,
        kFlag_MatrixIndexed_TexCoord_2  = 0x00000008,
        kFlag_MatrixIndexed_TexCoord_3  = 0x00000010,
        kFlag_MatrixIndexed_TexCoord_4  = 0x00000020,
        kFlag_MatrixIndexed_TexCoord_5  = 0x00000040,
        kFlag_MatrixIndexed_TexCoord_6  = 0x00000080,
        kFlag_MatrixIndexed_TexCoord_7  = 0x00000100,
        kFlag_DestinationAlpha          = 0x00000200,
    };

    enum VertexComponent {
        kVertexComponent_None = 0,
        kVertexComponent_Position,
        kVertexComponent_Color0,
        kVertexComponent_Color1,
        kVertexComponent_Normal,
        kVertexComponent_TexCoord0,
        kVertexComponent_TexCoord1,
        kVertexComponent_TexCoord2,
        kVertexComponent_TexCoord3,
        kVertexComponent_TexCoord4,
        kVertexComponent_TexCoord5,
        kVertexComponent_TexCoord6,
        kVertexComponent_TexCoord7,
        kVertexComponent_NumberOf
    };

    /// Shader cache entry
    class CacheEntry {
    public:
        /// Shader cache renderer-specific data base class
        class BackendData {
        public:
            BackendData() { }
            virtual ~BackendData() { } // Virtual destructor allows for polymorphism
        };

        CacheEntry() {
            backend_data_   = NULL; 
            hash_           = 0;
            frame_used_     = -1;
        }
        ~CacheEntry() { 
        }
        BackendData*        backend_data_;  ///< Pointer to backend renderer data
        common::Hash64      hash_;          ///< Hash of the shader state
        int                 frame_used_;    ///< Last frame that the shader was used
    };

    typedef HashContainer_STLMap<common::Hash64, CacheEntry> CacheContainer;

    /// Renderer interface for controlling shaders
    class BackendInterface{
    public:
        BackendInterface() { }
        ~BackendInterface() { }

        /**
         * Create a new shader in the backend renderer
         * @param vs_header Vertex shader header definitions
         * @param fs_header Fragment shader header definitions
         * @return a pointer to CacheEntry::BackendData with renderer-specific shader data
         */
        virtual CacheEntry::BackendData* Create(const char* vs_header, const char* fs_header) = 0;

        /**
         * Delete a shader from the backend renderer
         * @param backend_data Renderer-specific shader data used by renderer to remove it
         */
        virtual void Delete(CacheEntry::BackendData* backend_data) = 0;

        /**
         * Binds a shader to the backend renderer
         * @param backend_data Pointer to renderer-specific data used for binding
         */
        virtual void Bind(const CacheEntry::BackendData* backend_data) = 0;

    };

    ShaderManager(const BackendInterface* backend_interface);
    ~ShaderManager();

    // Control interface
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /// Sets the current shader
    void Bind();

    /**
     * Gets cached CacheEntry object from a index into the shader cache
     * @param index Index into shader cache of shader to select
     * @param res Result CacheEntry object
     * @return True if lookup succeeded, false if failed
     */
    const CacheEntry& Fetch(int index);

    /**
     * Gets the number of active shaders in the shader cache
     * @return Integer number of active shaders in the shader cache
     */
    int Size();

    /**
     * Purges expired shaders (shaders that are older than current_frame + age_limit)
     * @param age_limit Acceptable age limit (in frames) for shaders to still be considered fresh
     */
    void Purge(int age_limit=280);

    // Update - this is the interface used by the rest of the video core to produce the shader
    ////////////////////////////////////////////////////////////////////////////////////////////////

    void UpdateFlag(Flag flag, int enable);
    void UpdateVertexState(gp::VertexState& vertex_state);
    void UpdateGenMode(const gp::BPGenMode& gen_mode);
    void UpdateAlphaFunc(const gp::BPAlphaFunc& alpha_func);
    void UpdateEFBFormat(gp::BPPixelFormat efb_format);
    void UpdateTevCombiner(int index, const gp::BPTevCombiner& tev_combiner);
    void UpdateTevOrder(int index, const gp::BPTevOrder& tev_order);

private:
    CacheEntry*         active_shader_;         ///< Pointer to active shader in shader cache
    CacheContainer*     cache_;                 ///< Shader cache
    BackendInterface*   backend_interface_;     ///< Backend renderer interface

    /// Structure to hold the current shader state
    union State {
        struct _Fields{
            u32                 flags;
            GXCompType          vertex_component[kVertexComponent_NumberOf];
            u8                  num_stages;
            gp::BPAlphaFunc     alpha_func;
            gp::BPTevCombiner   tev_combiner[kGCMaxTevStages];
            gp::BPTevOrder      tev_order[0x8];
            gp::BPPixelFormat   efb_format;
            gp::VertexState     vertex_state;
        } fields;
        u8  mem[sizeof(_Fields)/4];
    } state_;

    class ShaderHeader {
    public:
        ShaderHeader(int buff_size=0x2000) : buff_size_(0), offset_(0) {
            buff_size_ = buff_size;
            buff_ = new char[buff_size_];
        }
        ~ShaderHeader() {
            delete buff_;
        }

        void Write(const char* fmt, ...) {
            va_list arg;
            va_start(arg, fmt);
            offset_ += vsprintf(&buff_[offset_], fmt, arg);
            va_end(arg);
        }

        void Reset() {
            offset_ = 0;
            memset(buff_, 0, buff_size_);
        }

        const char* Read() const {
            return buff_;
        }
    private:
        int     buff_size_;
        char*   buff_;
        int     offset_;
    };

    ShaderHeader* vsh_;
    ShaderHeader* fsh_;

    /// Generates the vertex shader header for the current state
    void GenerateVertexHeader();

    /// Generates the lighting shader header for the current state
    void GenerateVertexLightingHeader();

    /// Generates the fragment shader header for the current state
    void GenerateFragmentHeader();

    DISALLOW_COPY_AND_ASSIGN(ShaderManager);
};

#endif // VIDEO_CORE_SHADER_MANAGER_H_
