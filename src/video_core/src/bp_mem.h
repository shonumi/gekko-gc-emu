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
typedef struct {
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
}BPGenMode;

/// PE CMode 1
typedef struct {
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
}BPCMode1;

/// TEV color / alpha combiners
typedef struct {
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
}BPTevCombiner;

/// TEV konstant color/alpha selector
typedef struct {
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
}BPTevKSel;

/// TEV raster color order
typedef struct {
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
}BPTevOrder;

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef union {
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
}BPMemory;

/*!
 * \brief Write a BP register
 * \param addr Address (8-bit) of register
 * \param data Data (32-bit) to write to register
 */
void BPRegisterWrite(u8 addr, u32 data);

} // namespace

#endif // VIDEO_CORE_BP_MEM_H_