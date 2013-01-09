/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    texture_manager.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-12-16
 * @brief   Manages textures, texture cache, and defines renderer interface
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

#ifndef VIDEO_CORE_TEXTURE_MANAGER_H_
#define VIDEO_CORE_TEXTURE_MANAGER_H_

#include "types.h"
#include "hash.h"
#include "hash_container.h"

#include "memory.h"

#include "bp_mem.h"
#include "gx_types.h"
#include "texture_decoder.h"

// TODO(ShizZy): Fix video_core so I can remove this shit...
namespace video_core {
extern int g_current_frame;
}

class TextureManager {
public:

    /// Source of texture data
    enum SourceType {
        kSourceType_None = 0,  
        kSourceType_Normal,     ///< Texture is raw RAM data
        kSourceType_EFBCopy,    ///< Texture is result of an EFB copy
    };

    /// Texture cache entry
    class CacheEntry {
    public:
        /// Texture cache renderer-specific data base class
        class BackendData {
        public:
            BackendData() { }
            virtual ~BackendData() { } // Virtual destructor allows for polymorphism
        };

        CacheEntry() {
            address_                = 0;
            size_                   = 0;
            width_                  = -1;
            height_                 = -1;
            type_                   = kSourceType_None;
            format_                 = gp::kTextureFormat_None; 
            backend_data_           = NULL; 
            hash_                   = 0;
            frame_used_             = -1; 
            efb_copy_addr_          = 0; 
            efb_copy_rect_.width    = 0;
            efb_copy_rect_.height   = 0;
            efb_copy_rect_.x        = 0;
            efb_copy_rect_.y        = 0;
        }
        ~CacheEntry() { 
        }

        u32                 address_;       ///< Source address of texture
        int                 width_;         ///< Source texture width in pixels
        int                 height_;        ///< Source texture height in pixels
        SourceType          type_;          ///< Texture source (raw RAM data or EFB-copy)
        gp::TextureFormat   format_;        ///< Source texture format  (dest is always RGBA8)
        size_t              size_;          ///< Source size of texture in bytes
        BackendData*        backend_data_;  ///< Pointer to backend renderer data
        common::Hash64      hash_;          ///< Hash of source texture raw data
        int                 frame_used_;    ///< Last frame that the texture was used
        u32                 efb_copy_addr_; ///< EFB copy address (only if texture is from EFB copy)
        Rect                efb_copy_rect_; ///< EFB copy region (only if texture is from EFB copy)
    };

    typedef HashContainer_STLMap<common::Hash64, CacheEntry> CacheContainer;

    static const int kHashSamples = 128;    ///< Number of texture samples to use for hash

    /// Renderer interface for controlling textures
    class BackendInterface{
    public:
        BackendInterface() { }
        ~BackendInterface() { }

        /**
         * Create a new texture in the backend renderer
         * @param active_texture_unit Active texture unit to bind to for creation
         * @param texture cache_entry to create texture for
         * @param raw_data Raw texture data
         * @return a pointer to CacheEntry::BackendData with renderer-specific texture data
         */
        virtual CacheEntry::BackendData* Create(int active_texture_unit, 
            const CacheEntry& cache_entry, u8* raw_data) = 0;

        /**
         * Delete a texture from the backend renderer
         * @param backend_data Renderer-specific texture data used by renderer to remove it
         */
        virtual void Delete(CacheEntry::BackendData* backend_data) = 0;

        /** 
         * Call to update a texture with a new EFB copy of the region specified by rect
         * @param rect EFB rectangle to copy
         * @param backend_data Pointer to renderer-specific data used for the EFB copy
         */
        virtual void CopyEFB(Rect rect, const CacheEntry::BackendData* backend_data) = 0;

        /**
         * Binds a texture to the backend renderer
         * @param active_texture_unit Active texture unit to bind to
         * @param backend_data Pointer to renderer-specific data used for binding
         */
        virtual void Bind(int active_texture_unit, const CacheEntry::BackendData* backend_data) = 0;

        /**
         * Updates the texture parameters
         * @param active_texture_unit Active texture unit to update the parameters for
         * @param tex_mode_0 BP TexMode0 register to use for the update
         * @param tex_mode_1 BP TexMode1 register to use for the update
         */
        virtual void UpdateParameters(int active_texture_unit, const gp::BPTexMode0& tex_mode_0,
            const gp::BPTexMode1& tex_mode_1) = 0;
    };

    TextureManager(const BackendInterface* backend_interface);
    ~TextureManager();

    /**
     * Updates texture data in the video core for a given active texture unit
     * @param active_texture_unit Texture unit to update (0-7)
     * @param tex_image_0 BP TexImage0 register to use for the update
     * @param tex_image_3 BP TexImage3 register to use for the update
     */
    void UpdateData(int active_texture_unit, const gp::BPTexImage0& tex_image_0, 
        const gp::BPTexImage3& tex_image_3);

    /** 
     * Copy the EFB to a texture
     * @param addr Address in RAM EFB copy is supposed to go
     * @param efb_copy_exec BP efb copy register
     * @param rect EFB rectangle to copy
     */
    void CopyEFB(u32 addr, const gp::BPEFBCopyExec& efb_copy_exec, const Rect& rect);

    /**
     * Updates the texture parameters
     * @param active_texture_unit Active texture unit to update the parameters for
     * @param tex_mode_0 BP TexMode0 register to use for the update
     * @param tex_mode_1 BP TexMode1 register to use for the update
     */
    void UpdateParameters(int active_texture_unit, const gp::BPTexMode0& tex_mode_0, 
        const gp::BPTexMode1& tex_mode_1);

    /**
     * Binds the most recently updated texture for a given active texture unit
     * @param active_texture_unit Texture unit to bind (0-7)
     */
    void Bind(int active_texture_unit);

    /**
     * Gets cached CacheEntry object from a index into the texture cache
     * @param index Index into texture cache of texture to select
     * @param res Result CacheEntry object
     * @return True if lookup succeeded, false if failed
     */
    const CacheEntry& Fetch(int index);

    /**
     * Gets the number of active textures in the texture cache
     * @return Integer number of active textures in the texture cache
     */
    int Size();

    /**
     * Purges expired textures (textures that are older than current_frame + age_limit)
     * @param age_limit Acceptable age limit (in frames) for textures to still be considered fresh
     */
    void Purge(int age_limit=100);

private:

    CacheContainer*     cache_;                                 ///< Texture cache
    BackendInterface*   backend_interface_;                     ///< Backend renderer interface
    CacheEntry*         active_textures_[kGCMaxActiveTextures]; ///< Currently active textures

    DISALLOW_COPY_AND_ASSIGN(TextureManager);
};

#endif // VIDEO_CORE_TEXTURE_MANAGER_H_
