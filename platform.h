////////////////////////////////////////////////////////////////////////////////
// Gekko - Gamecube Emulator
// (c) 2005,2008,2010 Gekko Team / Wiimu Project
//
// Description:
//   Platform detection macros for portable compilation.
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_H__
#define PLATFORM_H__

////////////////////////////////////////////////////////////////////////////////
// Platform definitions
#define GEKKO_PLATFORM_WINDOWS 1
#define GEKKO_PLATFORM_MAC     2
#define GEKKO_PLATFORM_LINUX   3

#define GEKKO_ARCHITECTURE_32  1
#define GEKKO_ARCHITECTURE_64  2

////////////////////////////////////////////////////////////////////////////////
// Platform detection
extern char *kGekkoOS;

#ifndef GEKKO_PLATFORM
#  if defined( __WIN32__ ) || defined( _WIN32 )
#    define GEKKO_PLATFORM GEKKO_PLATFORM_WINDOWS
#  elif defined( __APPLE__ ) || defined( __APPLE_CC__ )
#    define GEKKO_PLATFORM GEKKO_PLATFORM_MAC
#  else // Assume linux if not mac or windows
#    define GEKKO_PLATFORM GEKKO_PLATFORM_LINUX
#  endif
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__alpha__) || defined(__ia64__)
#  define GEKKO_ARCH_TYPE GEKKO_ARCHITECTURE_64
#else
#  define GEKKO_ARCH_TYPE GEKKO_ARCHITECTURE_32
#endif

////////////////////////////////////////////////////////////////////////////////
// Compiler-Specific Definitions
#if GEKKO_PLATFORM == GEKKO_PLATFORM_WINDOWS
#  define GEKKO_FASTCALL __fastcall
#else
#  define GEKKO_FASTCALL __attribute__((fastcall))
#endif

#endif // PLATFORM_H__
