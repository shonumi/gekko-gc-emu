/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    types.h
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-02-11
 * \brief   Common types used throughout the project
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

#ifndef COMMON_TYPES_
#define COMMON_TYPES_

#include <xmmintrin.h> // data_types__m128.cpp

typedef unsigned char       u8;     ///< 8-bit unsigned byte
typedef unsigned short      u16;    ///< 16-bit unsigned short
typedef unsigned int        u32;    ///< 32-bit unsigned word

typedef signed char         s8;     ///< 8-bit signed byte
typedef signed short        s16;    ///< 16-bit signed short
typedef signed int          s32;    ///< 32-bit signed word

typedef signed int          x32;    ///< S15.16 fixed point int 

typedef float               f32;    ///< 32-bit floating point
typedef double              f64;    ///< 64-bit floating point

#ifdef _MSC_VER

typedef unsigned __int64    u64;    ///< 64-bit unsigned int
typedef signed __int64      s64;    ///< 64-bit signed int

#elif defined(__GNUC__)

typedef signed long long    s64;    ///< 64-bit unsigned int
typedef unsigned long long  u64;    ///< 64-bit signed int

#define U64(a) a ## ull
#define S64(a) a ## sll

#endif

/// Union for fast 16-bit type casting
typedef union {
	u8	_u8[2];             ///< 8-bit unsigned char(s)
	u16 _u16;               ///< 16-bit unsigned shorts(s)
} t16;

/// Union for fast 32-bit type casting
typedef union {
    f32 _f32;               ///< 32-bit floating point(s)
    u32 _u32;               ///< 32-bit unsigned int(s)
    x32 _x32;               ///< 32-bit fixed point(s)
    u16 _u16[2];            ///< 16-bit unsigned shorts(s)
    u8  _u8[4];             ///< 8-bit unsigned char(s)
} t32;

/// Union for fast 64-bit type casting
typedef union {
    f64 _f64;               ///< 64-bit floating point
    u64 _u64;               ///< 64-bit unsigned long
    f32 _f32[2];            ///< 32-bit floating point(s)
    u32 _u32[2];            ///< 32-bit unsigned int(s)
    x32 _x32[2];            ///< 32-bit fixed point(s)
    u16 _u16[4];            ///< 16-bit unsigned shorts(s)
    u8  _u8[8];             ///< 8-bit unsigned char(s)
} t64;

/// Union for fast 128-bit type casting
typedef union {
    struct
    {
        t64 ps0;            ///< 64-bit paired single 0
        t64 ps1;            ///< 64-bit paired single 1
    };
    __m128  a;              ///< 128-bit floating point (__m128 maps to the XMM[0-7] registers)
} t128;

#endif // COMMON_TYPES_