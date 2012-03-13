// hle_func.h
// (c) 2005,2006 Gekko Team

#ifndef _HLE_FUNC_H_
#define _HLE_FUNC_H_

#include "hle.h"

////////////////////////////////////////////////////////////

typedef void(*hle_functions)();
extern hle_functions exec_func_HLE[80];

////////////////////////////////////////////////////////////

HLE(ignore);
HLE(ignore_return_true);
HLE(ignore_return_false);

HLE(memcpy);
HLE(memset);
HLE(strchr);
HLE(strcmp);
HLE(strcpy);
HLE(strlen);
HLE(sinfcosf);

HLE(OSGetConsoleType);
HLE(OSEnableInterrupts);
HLE(OSRestoreInterrupts);
HLE(OSDisableInterrupts);
HLE(OSReport);
HLE(OSPanic);
HLE(DBPrintf);
HLE(OSInitFont);
HLE(OSGetResetButtonState);
HLE(__OSContextInit);
HLE(OSInitContext);
HLE(OSSetCurrentContext);
HLE(OSGetCurrentContext);
HLE(OSSaveContext);
HLE(OSLoadContext);
HLE(OSClearContext);
HLE(OSLoadFPUContext);
HLE(OSSaveFPUContext);
HLE(OSFillFPUContext);

HLE(PSMTXConcat);
HLE(PSMTXRotRad);
HLE(PSMTXTrans);
HLE(PSMTXScale);
HLE(PSMTXMultVec);
HLE(PSMTXInverse);

HLE(DVDOpen);
HLE(DVDReadPrio);
HLE(DVDClose);

HLE(StreamLibStart);
HLE(StreamLibStop);

HLE(DEMOWinLogPrintf);

////////////////////////////////////////////////////////////

#define OS_PHYSICAL_CONTEXT			0x800000C0      // OSContext *
#define OS_CURRENT_CONTEXT			0x800000D4      // OSContext *
#define OS_DEFAULT_THREAD			0x800000D8      // OSThread *

#define OS_CONTEXT_STATE_FPSAVED    1				// set when FPU is saved
#define OS_CONTEXT_STATE_EXC        2				// set when saved by exception

#define MSR_EE      0x8000							// external interrupt enable
#define MSR_ME      0x1000							// machine check enable
#define MSR_FP      0x2000							// floating point available
#define MSR_IR      0x0020							// instruction address translation
#define MSR_DR      0x0010							// data address translation
#define MSR_RI      0x0002							// recoverable exception

struct OSContext
{
    u32 gpr[32];
    u32 cr,lr,ctr,xer;

    double fpr[32];

    u32 fpscr_pad, fpscr;

    u32 srr0,srr1;

    u16 mode, state; 

    u32 gqr[8];
	u32 padding;
    double psr[32];
};

////////////////////////////////////////////////////////////

#endif