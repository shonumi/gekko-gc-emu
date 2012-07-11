/*!
* Copyright (C) 2005-2012 Gekko Emulator
*
* \file    bp_mem.h
* \author  ShizZy <shizzy247@gmail.com>
* \date    2012-03-10
* \brief   Implementation of BP for the graphics processor
*
* \section LICENSE
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

#ifndef VIDEO_CORE_BP_MEM_H_
#define VIDEO_CORE_BP_MEM_H_

#include "common.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Graphics Processor namespace

namespace gp {

////////////////////////////////////////////////////////////////////////////////////////////////////
// BP Register Decoding

/// Gen Mode
struct BPGenMode {
    union {
        struct {
            unsigned ntex : 4;
            unsigned ncol : 5;
            unsigned ms_en : 1;
            unsigned ntev : 4;
            unsigned reject_en : 2;
            unsigned nbmp : 3;
            unsigned zfreeze : 5;
            unsigned rid : 8;
        };
        u32 _u32;
    };
};

/// PE CMode 1
struct BPCMode1{
    union {
        struct {
            unsigned alpha : 8;
            unsigned enable : 1;
            unsigned pad0 : 15;
            unsigned rid : 8;
        };
        u32 _u32;
    };

    f32 getalpha() { return alpha / 255.0f; }
};

struct BPPECopyExecute {
    union {
        struct {
            u32 clamp0              : 1; // if set clamp top
            u32 clamp1              : 1; // if set clamp bottom
            u32 yuv                 : 1; // if set, color conversion from RGB to YUV
            u32 target_pixel_format : 4; // realformat is (fmt/2)+((fmt&1)*8).
            u32 gamma               : 2; // gamma correction.. 0 = 1.0 ; 1 = 1.7 ; 2 = 2.2 ; 
                                         // 3 is reserved
            u32 half_scale          : 1; // "mipmap" filter... 0 = no filter (scale 1:1) ; 
                                         // 1 = box filter (scale 2:1)
            u32 scale_invert        : 1; // if set vertical scaling is on
            u32 clear               : 1;
            u32 frame_to_field      : 2; // 0 progressive ; 1 is reserved ; 
                                         // 2 = interlaced (even lines) ; 3 = interlaced 1 (odd)
            u32 copy_to_xfb         : 1;
            u32 intensity_fmt       : 1; // if set, is an intensity format (I4,I8,IA4,IA8)
            u32 auto_conv           : 1; // if 0 auto color conversion by texture format/pixel type
        };
        u32 _u32;
    };
    u32 tp_realFormat() { 
        return target_pixel_format / 2 + (target_pixel_format & 1) * 8;
    }
};

/// TEV color / alpha combiners
struct BPTevCombiner {
    union {
        struct {
            unsigned seld : 4;
            unsigned selc : 4;
            unsigned selb : 4;
            unsigned sela : 4;
            unsigned bias : 2;
            unsigned sub : 1;
            unsigned clamp : 1;
            unsigned shift : 2;
            unsigned dest : 2;
            unsigned rid : 8;
        };
        u32 _u32;
    }color;

    union {
        struct {
            unsigned rswap : 2;
            unsigned tswap : 2;
            unsigned seld : 3;
            unsigned selc : 3;
            unsigned selb : 3;
            unsigned sela : 3;
            unsigned bias : 2;
            unsigned sub : 1;
            unsigned clamp : 1;
            unsigned shift : 2;
            unsigned dest : 2;
            unsigned rid : 8;
        };
        u32 _u32;
    }alpha;
};

/// TEV konstant color/alpha selector
struct BPTevKSel {
    union {
        struct {
            unsigned xrb : 2;
            unsigned xga : 2;
            unsigned kcsel0 : 5;
            unsigned kasel0 : 5;
            unsigned kcsel1 : 5;
            unsigned kasel1 : 5;
            unsigned rid : 8;
        };
        u32 _u32;
    };

    int getkc (int stage) { return (stage&1) ? kcsel1 : kcsel0; }
    int getka (int stage) { return (stage&1) ? kasel1 : kasel0; }
};

/// TEV raster color order
struct BPTevOrder {
    union {
        struct {
            unsigned texmap0 : 3;
            unsigned texcoord0 : 3;
            unsigned texmapenable0 : 1;
            unsigned colorid0 : 3;
            unsigned pad0 : 2;
            unsigned texmap1 : 3;
            unsigned texcoord1 : 3;
            unsigned texmapenable1 : 1;
            unsigned colorid1 : 3;
            unsigned pad1 : 2;
            unsigned rid : 8;
        };
        u32 _u32;
    };

    inline int get_texmap(int stage) { return (stage&1) ? texmap1 : texmap0; }
    inline int get_texcoord(int stage) { return (stage&1) ? texcoord1 : texcoord0; }
    inline int get_enable(int stage) { return (stage&1) ? texmapenable1 : texmapenable0; }
    inline int get_colorchan(int stage) { return (stage&1) ? colorid1 : colorid0; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

union BPMemory {
    struct{
        BPGenMode       genmode;
        u32             pad0[0x27];
        BPTevOrder      tevorder[0x8];
        u32             pad1[0x12];
        BPCMode1        cmode1;
        u32             pad2[0x7D];
        BPTevCombiner   combiner[0x10];
        u32             pad3[0x16];
        BPTevKSel       ksel[0x8];
    };
    u32 mem[0x100];
};

extern BPMemory g_bp_regs; ///< BP memory/registers

/*!
 * \brief Write a BP register
 * \param addr Address (8-bit) of register
 * \param data Data (32-bit) to write to register
 */
void BPRegisterWrite(u8 addr, u32 data);

/// Initialize BP
void BPInit();

} // namespace

#endif // VIDEO_CORE_BP_MEM_H_
