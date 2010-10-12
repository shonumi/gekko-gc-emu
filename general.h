// general.h
// (c) 2005,2008 Gekko Team / Wiimu Project

#ifndef _GENERAL_H_
#define _GENERAL_H_

#include "platform.h"

////////////////////////////////////////////////////////////////////////////////
// Preprocessor stuff
#define GEKKO_QUOTE_INPLACE(x) # x
#define GEKKO_QUOTE(x) GEKKO_QUOTE_INPLACE(x)
#define __FILE__LINE__ __FILE__ "(" GEKKO_QUOTE(__LINE__) ") : "
#define GEKKO_TODO(x) __FILE__LINE__ x "\n"

#if GEKKO_PLATFORM == GEKKO_PLATFORM_WINDOWS
//#  define LEAK_DETECT
#  define USE_INLINE_ASM
#  pragma warning( disable : 4786 )	//disable the truncated 255 character limit warning for debug identifiers
#  ifdef LEAK_DETECT
		//#pragma message("leak-hunting enabled")
#    define _CRTDBG_MAP_ALLOC
#    define _INC_MALLOC
#    include <stdlib.h>
#    include <crtdbg.h>
#  endif
#  define TODO( x )  message( __FILE__LINE__" TODO :   " #x "\n" )
#  define todo( x )  message( __FILE__LINE__" TODO :   " #x "\n" ) 
#endif

////////////////////////////////////////////////////////////////////////////////
// Includes
#include <xmmintrin.h>

////////////////////////////////////////////////////////////////////////////////
// C Includes
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <csignal>

////////////////////////////////////////////////////////////////////////////////
// C++ Includes
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// OS-Specific Includes
#if GEKKO_PLATFORM == GEKKO_PLATFORM_WINDOWS
  #include <direct.h>
  #include <windows.h>
  #include <commctrl.h>
  #include <commdlg.h>
  #include <shlwapi.h>
  #include <shlobj.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Big Endian bit Access (Bits numbered ascending from leftmost to rightmost)
#define BIT_0	 0x80000000
#define BIT_1	 0x40000000
#define BIT_2	 0x20000000
#define BIT_3	 0x10000000
#define BIT_4	 0x8000000
#define BIT_5	 0x4000000
#define BIT_6	 0x2000000
#define BIT_7	 0x1000000
#define BIT_8	 0x800000
#define BIT_9	 0x400000
#define BIT_10 0x200000
#define BIT_11 0x100000
#define BIT_12 0x80000
#define BIT_13 0x40000
#define BIT_14 0x20000
#define BIT_15 0x10000
#define BIT_16 0x8000
#define BIT_17 0x4000
#define BIT_18 0x2000
#define BIT_19 0x1000
#define BIT_20 0x800
#define BIT_21 0x400
#define BIT_22 0x200
#define BIT_23 0x100
#define BIT_24 0x80
#define BIT_25 0x40
#define BIT_26 0x20
#define BIT_27 0x10
#define BIT_28 0x8
#define BIT_29 0x4
#define BIT_30 0x2
#define BIT_31 0x1

#define SIGNED_BIT8	 ((u8)	1 << 7)
#define SIGNED_BIT16 ((u16)	1 << 15)
#define SIGNED_BIT32 ((u32)	1 << 31)
#define SIGNED_BIT64 ((u64)	1 << 63)

////////////////////////////////////////////////////////////////////////////////
// Data Types

#if defined (_MSC_VER)

typedef __int8  s8;
typedef __int16 s16;
typedef __int32 s32;
typedef __int64 s64;

typedef unsigned __int8  u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

#else

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char      s8;
typedef signed short     s16;
typedef signed int       s32;
typedef signed long long s64;

#endif

typedef float	 f32;
typedef double f64;

////////////////////////////////////////////////////////////////////////////////

void DisplayError (char * Message, ...);

#ifdef _MSC_VER
#  ifdef LEAK_DETECT
#    undef malloc
#    define DEBUG_NEW		new(_NORMAL_BLOCK,__FILE__, __LINE__)
#    define new DEBUG_NEW
#    define malloc(s)		_malloc_dbg(s,_NORMAL_BLOCK,__FILE__,__LINE__)
#    define realloc(p, s)	_realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#    define free(p)			_free_dbg(p, _NORMAL_BLOCK)
#  endif
#  define U64(a) a ## ui64
#  define S64(a) a ## si64
#else //gcc
#  define U64(a) a ## ull
#  define S64(a) a ## sll
#endif

////////////////////////////////////////////////////////////////////////////////

__inline static s16 toSHORT(u16 x)
{
	return *(s16*)&x;
}

__inline static f32 toFLOAT(u32 x)
{
	return *(f32*)&x;
}

__inline static f32 toFLOATS(s32 x)
{
	return *(f32*)&x;
}

__inline static f64 toDOUBLE(u64 x)
{
	return *(f64*)&x;
}

////////////////////////////////////////////////////////////////////////////////

typedef union _t32
{
	f32 _f32;
	u32 _u32;
	u16 _u16[2];
	u8  _u8[4];
} t32;

typedef union _t64
{
	f64 _f64;
	u64 _u64;
	f32 _f32[2];
	u32 _u32[2];
	u16 _u16[4];
	u8  _u8[8];
} t64;

typedef struct _t128
{
	union
	{
		struct
		{
			t64	ps0;
			t64 ps1;
		};
		__m128	a;
	};
} t128;

typedef void(GEKKO_FASTCALL *optable)(void);
typedef	void(GEKKO_FASTCALL *hwtable)(u32,u32*);

////////////////////////////////////////////////////////////////////////////////
// Fast Macros
#define MIN(a,b)	((a)<(b)?(a):(b))
#define MAX(a,b)	((a)>(b)?(a):(b))
#define CLAMP(X,min,max)	(((X) > max) ? max : (((X) < min) ? min : (X)))

__inline static u32 BSWAP24(u32 x)
{	
	return (((x & 0xff0000) >> 16) | (x & 0xff00) | ((x & 0xff) << 16));
}

#if _MSC_VER > 1200
#  define BSWAP16(x) _byteswap_ushort(x)
#  define BSWAP32(x) _byteswap_ulong(x)
#  define BSWAP64(x) 	_byteswap_uint64(x)
#else
__inline static u16 BSWAP16(u16 x)
{
  return ((x)>>8) | ((x)<<8);
}

__inline static u32 BSWAP32(u32 x)
{
  return (BSWAP16((x)&0xffff)<<16) | (BSWAP16((x)>>16));
}

__inline static u64 BSWAP64(u64 x)
{
  return (u64)(((u64)BSWAP32((u32)(x&0xffffffff)))<<32) | (BSWAP32((u32)(x>>32)));
} 
#endif
//
/*
#ifdef __cplusplus
extern "C" {
#endif

void * Internal_ASM_MMX_memcpy(void *Ptr1, const void *Ptr2, long Len);

#undef memcpy
#define memcpy(a,b,c) Internal_ASM_MMX_memcpy(a,b,c)

#ifdef __cplusplus
}
#endif
*/
#endif
