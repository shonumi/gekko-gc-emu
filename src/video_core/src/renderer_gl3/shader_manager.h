/**
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * @file    shader_manager.h
 * @author  ShizZy <shizzy247@gmail.com>
 * @date    2012-03-09
 * @brief   Managers shaders for the GL3 renderer
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

#ifndef VIDEO_CORE_SHADER_MANAGER_H_
#define VIDEO_CORE_SHADER_MANAGER_H_

#include <GL/glew.h>

#include "common.h"
#include "gx_types.h"
#include "uniform_manager.h"

#define MAX_SHADERS         0x100
#define MAX_CACHE_SIZE      0x1000

class ShaderManager {

public:

    ShaderManager();
    ~ShaderManager() {};

    /// Updates the uniform values for the current shader
    void UpdateUniforms();

    /// Sets the current shader program based on a set of GP parameters
    void SetShader();

    /**
     * Gets the default shader
     * @returns Handle to the default shader program
     */
    GLuint GetDefaultShader();

    /*
     * Initialize the shader manager
     * @param uniform_manager Handle to the UniformManager instance that handles uniform data
     */
    void Init(UniformManager* uniform_manager);

private:

    /// Shader cache entry
    struct CacheEntry {
        u32 hash;
        GLuint program;
    };

    /// Simple shader cache
    struct Cache {
        CacheEntry _cache[MAX_CACHE_SIZE][MAX_SHADERS];
        int _cache_count[MAX_CACHE_SIZE];

        /// Create a partial hash to reduce the chance that we have to "search" for our shader
        inline u16 _partial_hash(u32 hash) {
            return (u16) ((hash >> 24) ^ (hash >> 12) ^ hash) & 0xFFF;
        }

        /// Gets a shader program from the cache, returns 0 if it does not exist
        inline GLuint GetProgram(u32 hash) {
            u16 hash16 = _partial_hash(hash);

            for (int i = 0; i < _cache_count[hash16]; i++) {
                if (_cache[hash16][i].hash == hash) {
                    return _cache[hash16][i].program;
                }
            }
            return 0;
        }

        /// Adds a new shader program to the cache
        inline void AddProgram(u32 hash, GLuint program) {
            u16 hash16 = _partial_hash(hash);
            _cache[hash16][_cache_count[hash16]].hash = hash;
            _cache[hash16][_cache_count[hash16]].program = program;
            _cache_count[hash16]++;
            _ASSERT_MSG(TGP, _cache_count[hash16] < MAX_SHADERS, 
                "Exceeded maximum shader limit %d! Should this number be larger?", MAX_SHADERS);
        }
    };

    /**
     * Compiles a shader program
     * @param vs_def Preprocessor string to include before vertex shader program
     * @param fs_def Preprocessor string to include before fragment shader program
     * @return GLuint of new shader program
     */
    GLuint CompileShaderProgram(const char* vs_def, const char* fs_def);

    /**
     * Compute a 32-bit hash for the current TEV state, used for identifying the current shader
     * @return Unsigned short hash
     */
    u32 GetCurrentHash();

    /// Compiles a shader program given the specified shader inputs
    GLuint LoadShader();

    GLuint current_shader_;             ///< Handle to current shader program
    GLuint default_shader_;             ///< Handle to default shader program

    Cache cache_;                       ///< Simple shader cache

    std::string vertex_shader_src_;
    std::string fragment_shader_src_;

    char vertex_shader_path_[MAX_PATH];
    char fragment_shader_path_[MAX_PATH];

    UniformManager* uniform_manager_;
};

#endif // VIDEO_CORE_SHADER_MANAGER_H_
