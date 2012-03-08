/*!
 * Copyright (C) 2005-2012 Gekko Emulator
 *
 * \file    x86_utils.h
 * \author  ShizZy <shizzy247@gmail.com>
 * \date    2012-02-11
 * \brief   Utilities for the x86 architecture
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

#ifndef COMMON_X86_UTILS_
#define COMMON_X86_UTILS_

#include "common.h"

// Common namespace
namespace common {

static inline int IsSSE2Supported() {
    u32	sse2_res;
	u32	res;

#ifdef USE_INLINE_ASM
    _asm {
        //lets see if we can execute CPUID by seeing if we can flip the 21st bit
        //of the flags register
        pushfd
        pop eax

        //save the original
        mov ecx, eax

        //swap the 21st bit
        xor eax, 200000h

        //push it back on the stack, set the flags
        push eax
        popfd

        //now get the flags
        pushfd
        pop eax

        //see if the values are the same
        xor eax, ecx
        jz NoCPUIDSupport

        //CPUID supported, go ahead and grab some info
        mov eax, 1
        CPUID

        and edx, 04000000h
        mov sse2_res, edx
        mov res, 1

    NoCPUIDSupport:
    };

    if(!res)
    {
        LOG_ERROR(TCOMMON, "Aborting Execution\n");
        LOG_ERROR(TCOMMON, "CPU does not support the CPUID instruction\n");
        return E_ERR;
    }

    if(!sse2_res)
    {
        LOG_ERROR(TCOMMON, "Aborting Execution\n");
        LOG_ERROR(TCOMMON, "SSE2 is required by application but not found on the chip\n");
        return E_ERR;
    }
    return E_OK;
#else
    return E_ERR;
#endif
}

}

#endif
