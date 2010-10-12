////////////////////////////////////////////////////////////
// TITLE:		Lightning ProjectGC PowerPC 750 / Gekko CPU 
// VERSION:		Recompiler 0.1
// FILE:		cpu_rec_assembler.cpp
// DESC:		Assembler of the IL load/store opcodes for the recompiler
// CREATED:		Mar. 29, 2007
////////////////////////////////////////////////////////////
// Copyright (c) 2007 Lightning
////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "cpu_rec_assembler.h"
#include "../cpu_core_regs.h"

#include "../../low level/hardware core/hw_pe.h"
#include "../../low level/hardware core/hw_vi.h"
#include "../../low level/hardware core/hw_si.h"
#include "../../low level/hardware core/hw_pi.h"
#include "../../low level/hardware core/hw_exi.h"
#include "../../low level/hardware core/hw_dsp.h"
#include "../../low level/hardware core/hw_gx.h"
#include "../../low level/hardware core/hw_ai.h"
#include "../../low level/hardware core/hw_di.h"
#include "../../low level/hardware core/hw_cp.h"

#pragma todo(Research if need to implement CP_WPAR_Write* directly into memory writes)
static void __fastcall GXCP_Write8(u32 Addr, u32 data)
{
	CP_WPAR_Write8(Addr, data);
	return;
}

static void __fastcall GXCP_Write16(u32 Addr, u32 data)
{
	CP_WPAR_Write16(Addr, data);
	return;
}

static void __fastcall GXCP_Write32(u32 Addr, u32 data)
{
	CP_WPAR_Write32(Addr, data);
	return;
}
typedef u32(__fastcall *GekkoHWRead32Ptr)(u32 Addr);
static GekkoHWRead32Ptr	GekkoHWRead32[0x100] =
{
	//0x00
		CP_Read32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x10
		PE_Read32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x20
		VI_Read32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x30
		PI_Read32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x40
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x50
		DSP_Read32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x60
		DI_Read32, 0, 0, 0, SI_Read32, 0, 0, 0,
		EXI_Read32, 0, 0, 0, AI_Read32, 0, 0, 0,
	//0x70
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x80
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x90
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xA0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xB0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xC0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xD0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xE0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xF0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
};

typedef u16(__fastcall *GekkoHWRead16Ptr)(u32 Addr);
static GekkoHWRead16Ptr	GekkoHWRead16[0x100] =
{
	//0x00
		CP_Read16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x10
		PE_Read16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x20
		VI_Read16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x30
		PI_Read16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x40
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x50
		DSP_Read16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x60
		DI_Read16, 0, 0, 0, SI_Read16, 0, 0, 0,
		EXI_Read16, 0, 0, 0, AI_Read16, 0, 0, 0,
	//0x70
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x80
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x90
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xA0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xB0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xC0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xD0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xE0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xF0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
};

typedef u8(__fastcall *GekkoHWRead8Ptr)(u32 Addr);
static GekkoHWRead8Ptr	GekkoHWRead8[0x100] =
{
	//0x00
		CP_Read8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x10
		PE_Read8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x20
		VI_Read8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x30
		PI_Read8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x40
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x50
		DSP_Read8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x60
		DI_Read8, 0, 0, 0, SI_Read8, 0, 0, 0,
		EXI_Read8, 0, 0, 0, AI_Read8, 0, 0, 0,
	//0x70
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x80
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x90
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xA0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xB0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xC0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xD0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xE0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xF0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
};

typedef void (__fastcall *GekkoHWWrite32Ptr)(u32 Addr, u32 data);
static GekkoHWWrite32Ptr	GekkoHWWrite32[0x100] =
{
	//0x00
		CP_Write32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x10
		PE_Write32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x20
		VI_Write32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x30
		PI_Write32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x40
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x50
		DSP_Write32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x60
		DI_Write32, 0, 0, 0, SI_Write32, 0, 0, 0,
		EXI_Write32, 0, 0, 0, AI_Write32, 0, 0, 0,
	//0x70
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x80
		GXCP_Write32, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x90
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xA0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xB0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xC0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xD0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xE0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xF0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
};

typedef void (__fastcall *GekkoHWWrite16Ptr)(u32 Addr, u32 data);
static GekkoHWWrite16Ptr	GekkoHWWrite16[0x100] =
{
	//0x00
		CP_Write16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x10
		PE_Write16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x20
		VI_Write16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x30
		PI_Write16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x40
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x50
		DSP_Write16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x60
		DI_Write16, 0, 0, 0, SI_Write16, 0, 0, 0,
		EXI_Write16, 0, 0, 0, AI_Write16, 0, 0, 0,
	//0x70
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x80
		GXCP_Write16, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x90
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xA0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xB0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xC0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xD0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xE0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xF0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
};

typedef void (__fastcall *GekkoHWWrite8Ptr)(u32 Addr, u32 data);
static GekkoHWWrite8Ptr	GekkoHWWrite8[0x100] =
{
	//0x00
		CP_Write8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x10
		PE_Write8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x20
		VI_Write8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x30
		PI_Write8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x40
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x50
		DSP_Write8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x60
		DI_Write8, 0, 0, 0, SI_Write8, 0, 0, 0,
		EXI_Write8, 0, 0, 0, AI_Write8, 0, 0, 0,
	//0x70
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x80
		GXCP_Write8, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0x90
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xA0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xB0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xC0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xD0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xE0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	//0xF0
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0
};

GekkoRecILOp(SaveAllX86Regs)
{
	u32				x;
	u32				NewOutSize;
	RecInstruction	TempInst;

	*OutSize = 0;
	TempInst.InReg = 0;
	TempInst.OutReg = 0;

	//save off all assigned registers
	for(x = 0; x < X86_REG_COUNT; x++)
	{
		if(X86Regs[x].PPCRegister != -1)
		{
			if(X86Regs[x].ValueChanged && !(X86Regs[x].PPCRegister & (REG_SPECIAL | REG_PPC_SPECIAL)))
			{
				TempInst.X86InReg = x;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[x].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
			X86Regs[x].PPCRegister = -1;
		}
	}
}

GekkoRecILOp(SaveAllFPURegs)
{
	u32				x;
	u32				NewOutSize;
	RecInstruction	TempInst;

	*OutSize = 0;
	TempInst.InReg = 0;
	TempInst.OutReg = 0;

	//save off all assigned registers
	for(x = 0; x < FPU_REG_COUNT; x++)
	{
		if(FPURegs[x].PPCRegister != -1 && !(FPURegs[x].PPCRegister & TEMP_FPU_REG))
		{
			if(FPURegs[x].ValueChanged)
			{
				TempInst.X86InReg = x;
				TempInst.X86OutMemory = (u32)&ireg.fpr[FPURegs[x].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
				GekkoRecIL(MOVAPD)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, 0, FPURegs);
				*OutSize += NewOutSize;
			}
			FPURegs[x].PPCRegister = -1;
		}
	}
}

GekkoRecILOp(SaveAllX86Regs2)
{
	u32				x;
	u32				NewOutSize;
	RecInstruction	TempInst;

	*OutSize = 0;
	TempInst.InReg = 0;
	TempInst.OutReg = 0;

	//save off all assigned registers
	for(x = 0; x < X86_REG_COUNT; x++)
	{
		if(X86Regs[x].PPCRegister != -1)
		{
			if(X86Regs[x].ValueChanged && !(X86Regs[x].PPCRegister & (REG_SPECIAL | REG_PPC_SPECIAL)))
			{
				TempInst.X86InReg = x;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[x].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
		}
	}
}

GekkoRecILOp(SaveAllFPURegs2)
{						   
	u32				x;
	u32				NewOutSize;
	RecInstruction	TempInst;

	*OutSize = 0;
	TempInst.InReg = 0;
	TempInst.OutReg = 0;

	//save off all assigned registers
	for(x = 0; x < FPU_REG_COUNT; x++)
	{
		if(FPURegs[x].PPCRegister != -1 && !(FPURegs[x].PPCRegister & TEMP_FPU_REG))
		{
			TempInst.X86OutReg = x;
			TempInst.X86InMemory = (u32)&ireg.fpr[FPURegs[x].PPCRegister];
			TempInst.Flags = RecInstrFlagX86InMem | RecInstrFlagX86OutReg | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVAPD)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, 0, FPURegs);
			*OutSize += NewOutSize;
		}
	}
}

GekkoRecILOp(GetAllX86Regs)
{
	u32				x;
	u32				NewOutSize;
	RecInstruction	TempInst;

	*OutSize = 0;
	TempInst.InReg = 0;
	TempInst.OutReg = 0;

	//save off all assigned registers
	for(x = 0; x < X86_REG_COUNT; x++)
	{
		if(X86Regs[x].PPCRegister != -1)
		{
			TempInst.X86OutReg = x;
			TempInst.X86InMemory = (u32)&ireg.gpr[X86Regs[x].PPCRegister];
			TempInst.Flags = RecInstrFlagX86InMem | RecInstrFlagX86OutReg | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
		}
	}
}

GekkoRecILOp(GetAllFPURegs)
{						   
	u32				x;
	u32				NewOutSize;
	RecInstruction	TempInst;

	*OutSize = 0;
	TempInst.InReg = 0;
	TempInst.OutReg = 0;

	//save off all assigned registers
	for(x = 0; x < FPU_REG_COUNT; x++)
	{
		if(FPURegs[x].PPCRegister != -1 && !(FPURegs[x].PPCRegister & TEMP_FPU_REG))
		{
			if(FPURegs[x].ValueChanged)
			{
				TempInst.X86InReg = x;
				TempInst.X86OutMemory = (u32)&ireg.fpr[FPURegs[x].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
				GekkoRecIL(MOVAPD)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, 0, FPURegs);
				*OutSize += NewOutSize;
			}
		}
	}
}

GekkoRecILOp(SaveAllKnownX86ValRegs)
{
	u32				x;
	u32				NewOutSize;
	RecInstruction	TempInst;

	*OutSize = 0;
	TempInst.InReg = 0;
	TempInst.OutReg = 0;

	//store all of the known values for current registers
	for(x = 0; x < 32; x++)
	{
		if(PPCRegVals[x].Flags)
		{
			//put in a move to save the register
			TempInst.X86InVal = PPCRegVals[x].KnownValue;
			TempInst.X86OutMemory = (u32)&ireg.gpr[x];
			TempInst.Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
			PPCRegVals[x].Flags = 0;
		}
	}

	//save off the CR entries
	for(x = 0; x < 4; x++)
	{
		if(SetCRKnown[x] & 0x80)
		{
			TempInst.X86InVal = SetCRKnown[x] & 1;
			TempInst.X86OutMemory = (u32)&CR[x];
			TempInst.Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;
			SetCRKnown[x] = 0;
		}
	}

	if(SetXERKnown[3] & 0x80)
	{
		TempInst.X86InVal = SetXERKnown[3] & 1;
		TempInst.X86OutMemory = (u32)&XER_SUMMARYOVERFLOW;
		TempInst.Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
		GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
		SetXERKnown[3] = 0;
	}
	if(SetXERKnown[2] & 0x80)
	{
		TempInst.X86InVal = SetXERKnown[2] & 1;
		TempInst.X86OutMemory = (u32)&XER_OVERFLOW;
		TempInst.Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
		GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
		SetXERKnown[2] = 0;
	}
	if(SetXERKnown[1] & 0x80)
	{
		TempInst.X86InVal = SetXERKnown[1] & 1;
		TempInst.X86OutMemory = (u32)&XER_CARRY;
		TempInst.Flags = RecInstrFlagX86InVal | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
		GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
		*OutSize += NewOutSize;
		SetXERKnown[1] = 0;
	}
}

GekkoRecILOp(READMEM8)
{
	u32				OutReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			//move to ecx if need be
			if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InReg)
			{
				if(Instruction->X86InReg != 1)
				{
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 1) << 8) | 0x89;
					*OutSize += 2;
				}

				//reset the register the value is in
				X86Regs[Instruction->X86InReg].PPCRegister = -1;
			}
			else
			{
				//mov ecx, [mem]
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
				*OutSize += 6;
			}

			//now read the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Read8 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);

			//now read the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Read8 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);
			//and reg, 0xFF
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0x25;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = 0xFF;
			*OutSize += 10;

			//reassign the eax register to our entry
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
			{
				X86Regs[0].PPCRegister = Instruction->OutReg;
				X86Regs[0].ValueChanged = 1;
			}
			else
			{
				//mov [mem], eax
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
				*OutSize += 5;
			}
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			//have a known place we are reading from, check if we need to do
			//a direct read or call a specific function
			*OutSize = 0;
			if(Instruction->X86InVal < 0xC8000000 || 
			   ((Instruction->X86InVal >= 0xE0000000) &&
			    (Instruction->X86InVal < 0xF0000000))
			  )
			{
				if(Instruction->X86InVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//The reason we flip our check on the byte and not on
					//16bit and 32bit for (Val & 1) is that positions 3
					//and position 1 align properly when we want them if we read
					//a word into the requested register without mucking up
					//our register layout

					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if(!(Instruction->X86InVal & 1))
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//pick a register if we did not find one
							if(OutReg == 4)
								OutReg = 1;
						}
					}
					else
					{
						//find a register we can use
						OutReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
					}

					if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged && !(X86Regs[OutReg].PPCRegister & REG_SPECIAL))
					{
						//register returned is in use, move it out to memory
						TempInst.InReg = 0;
						TempInst.OutReg = 0;
						TempInst.X86InReg = OutReg;
						TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
						TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;
					}

					X86Regs[OutReg].PPCRegister = -1;
				}
				else
				{
					//get our register to use
					OutReg = Instruction->X86OutReg;

					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if((!(Instruction->X86InVal & 1)) && (Instruction->X86OutReg >= 4))
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//if no register then pick one
							if(OutReg == 4)
							{
								//register returned is in use, move it out to memory
								OutReg = 1;
								TempInst.InReg = 0;
								TempInst.OutReg = 0;
								TempInst.X86InReg = OutReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//change what register we are using
						X86Regs[OutReg].PPCRegister = X86Regs[Instruction->X86OutReg].PPCRegister;
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;
						Instruction->X86OutReg = OutReg;
					}
				}

				//if not aligned then handle it special
				if(!(Instruction->InVal & 1))
				{
					//mov reg, byte ptr (Mem+0)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = ((Instruction->X86InVal ^ 3) & MemPtrMask) + MemPtrVal;

					*OutSize += 6;
				}
				else
				{
					//aligned properly, just read it
					//mov reg, word [val^3]
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 16) | 0x8B66;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = ((Instruction->X86InVal ^ 3) & MemPtrMask) + MemPtrVal;
					*OutSize += 7;
				}

				//and reg, 0xFF
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = ((0xE0 + OutReg) << 8) | 0x81;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 0x000000FF;
				*OutSize += 6;

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//mov mem, reg
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x89;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
				}
				else
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else if(Instruction->X86InVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else //if(Instruction->X86InVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWRead8[(Instruction->X86InVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWRead8[(Instruction->X86InVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					//and eax, 0xFF
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 10]) = 0x25;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 11]) = 0xFF;
					*OutSize += 15;

					if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
					{
						//mov mem, eax
						*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
						*OutSize += 5;
					}
					else
					{
						//assign the register to eax
						X86Regs[0].PPCRegister = Instruction->OutReg;
						X86Regs[0].ValueChanged = 1;
					}
				}
			}

			break;

		default:
		Unknown_Mask("READMEM8", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

//READMEM8 with sign extention to 32bit
GekkoRecILOp(READMEM8SX)
{
	u32				OutReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			//move to ecx if need be
			if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InReg)
			{
				if(Instruction->X86InReg != 1)
				{
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 1) << 8) | 0x89;
					*OutSize += 2;
				}

				//reset the register the value is in
				X86Regs[Instruction->X86InReg].PPCRegister = -1;
			}
			else
			{
				//mov ecx, [mem]
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
				*OutSize += 6;
			}

			//now read the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Read8 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);

			//movsx eax, al
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xC0BE0F;
			*OutSize += 8;

			//reassign the eax register to our entry
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
			{
				X86Regs[0].PPCRegister = Instruction->OutReg;
				X86Regs[0].ValueChanged = 1;
			}
			else
			{
				//mov [mem], eax
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
				*OutSize += 5;
			}
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			//have a known place we are reading from, check if we need to do
			//a direct read or call a specific function
			*OutSize = 0;
			if(Instruction->X86InVal < 0xC8000000 || 
			   ((Instruction->X86InVal >= 0xE0000000) &&
			    (Instruction->X86InVal < 0xF0000000))
			  )
			{
				if(Instruction->X86InVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//The reason we flip our check on the byte and not on
					//16bit and 32bit for (Val & 1) is that positions 3
					//and position 1 align properly when we want them if we read
					//a word into the requested register without mucking up
					//our register layout

					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if(!(Instruction->X86InVal & 1))
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//pick a register if we did not find one
							if(OutReg == 4)
								OutReg = 1;
						}
					}
					else
					{
						//find a register we can use
						OutReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
					}

					if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged && !(X86Regs[OutReg].PPCRegister & REG_SPECIAL))
					{
						//register returned is in use, move it out to memory
						TempInst.InReg = 0;
						TempInst.OutReg = 0;
						TempInst.X86InReg = OutReg;
						TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
						TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;
					}

					X86Regs[OutReg].PPCRegister = -1;
				}
				else
				{
					//get our register to use
					OutReg = Instruction->X86OutReg;

					//if not a good register to use then find a different register to use
					if(Instruction->X86OutReg >= 4)
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//if no register then pick one
							if(OutReg == 4)
							{
								//register returned is in use, move it out to memory
								OutReg = 1;
								TempInst.InReg = 0;
								TempInst.OutReg = 0;
								TempInst.X86InReg = OutReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//change what register we are using
						X86Regs[OutReg].PPCRegister = X86Regs[Instruction->X86OutReg].PPCRegister;
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;
						Instruction->X86OutReg = OutReg;
					}
				}

				//if not aligned then handle it special
				if(!(Instruction->InVal & 1))
				{
					//mov reg, byte ptr (Mem+0)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = ((Instruction->X86InVal ^ 3) & MemPtrMask) + MemPtrVal;

					*OutSize += 6;
				}
				else
				{
					//aligned properly, just read it
					//mov reg, word [val^3]
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 16) | 0x8B66;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = ((Instruction->X86InVal ^ 3) & MemPtrMask) + MemPtrVal;
					*OutSize += 7;
				}

				//movsx reg, reg[al]
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBE0F;
				*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = ModRM(0x03, OutReg, OutReg);
				*OutSize += 3;

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//mov mem, reg
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x89;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
				}
				else
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else if(Instruction->X86InVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else //if(Instruction->X86InVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWRead8[(Instruction->X86InVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWRead8[(Instruction->X86InVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					//movsx eax, al
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 10]) = 0xC0BE0F;
					*OutSize += 13;

					if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
					{
						//mov mem, eax
						*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
						*OutSize += 5;
					}
					else
					{
						//assign the register to eax
						X86Regs[0].PPCRegister = Instruction->OutReg;
						X86Regs[0].ValueChanged = 1;
					}
				}
			}

			break;

		default:
		Unknown_Mask("READMEM8SX", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

GekkoRecILOp(READMEM16)
{
	u32				OutReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			//move to ecx if need be
			if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InReg)
			{
				if(Instruction->X86InReg != 1)
				{
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 1) << 8) | 0x89;
					*OutSize += 2;
				}

				//reset the register the value is in
				X86Regs[Instruction->X86InReg].PPCRegister = -1;
			}
			else
			{
				//mov ecx, [mem]
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
				*OutSize += 6;
			}

			//now read the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Read16 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);

			//and eax, 0xFFFF
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0x25;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = 0xFFFF;
			*OutSize += 10;

			//reassign the eax register to our entry
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
			{
				X86Regs[0].PPCRegister = Instruction->OutReg;
				X86Regs[0].ValueChanged = 1;
			}
			else
			{
				//mov [mem], eax
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
				*OutSize += 5;
			}
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			//have a known place we are reading from, check if we need to do
			//a direct read or call a specific function
			*OutSize = 0;
			if(Instruction->X86InVal < 0xC8000000 || 
			   ((Instruction->X86InVal >= 0xE0000000) &&
			    (Instruction->X86InVal < 0xF0000000))
			  )
			{
				if(Instruction->X86InVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if(Instruction->X86InVal & 1)
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//pick a register if we did not find one
							if(OutReg == 4)
								OutReg = 1;
						}
					}
					else
					{
						//find a register we can use
						OutReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
					}

					if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged && !(X86Regs[OutReg].PPCRegister & REG_SPECIAL))
					{
						//register returned is in use, move it out to memory
						TempInst.InReg = 0;
						TempInst.OutReg = 0;
						TempInst.X86InReg = OutReg;
						TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
						TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;
					}

					X86Regs[OutReg].PPCRegister = -1;
				}
				else
				{
					//get our register to use
					OutReg = Instruction->X86OutReg;

					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if((Instruction->X86InVal & 1) && (Instruction->X86OutReg >= 4))
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//if no register then pick one
							if(OutReg == 4)
							{
								//register returned is in use, move it out to memory
								OutReg = 1;
								TempInst.InReg = 0;
								TempInst.OutReg = 0;
								TempInst.X86InReg = OutReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//change what register we are using
						X86Regs[OutReg].PPCRegister = X86Regs[Instruction->X86OutReg].PPCRegister;
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;
						Instruction->X86OutReg = OutReg;
					}
				}

				//xor reg, reg
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, OutReg, OutReg) << 8) | 0x33;
				*OutSize += 2;

				//if not aligned then handle it special
				if(Instruction->InVal & 1)
				{
					//mov reg[l], byte ptr (Mem+0)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+0)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//mov reg[h], byte ptr (Mem+1)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg + 4, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+1)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;
				}
				else
				{
					//aligned properly, just read it
					//mov reg, word [val]
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 16) | 0x8B66;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = ((Instruction->X86InVal ^ 2) & MemPtrMask) + MemPtrVal;
					*OutSize += 7;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//mov mem, reg
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x89;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
				}
				else
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else if(Instruction->X86InVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else //if(Instruction->X86InVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWRead16[(Instruction->X86InVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWRead16[(Instruction->X86InVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					//and eax, 0xFFFF
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 10]) = 0x25;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 11]) = 0xFFFF;
					*OutSize += 15;

					if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
					{
						//mov mem, eax
						*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
						*OutSize += 5;
					}
					else
					{
						//assign the register to eax
						X86Regs[0].PPCRegister = Instruction->OutReg;
						X86Regs[0].ValueChanged = 1;
					}
				}
			}

			break;

		default:
		Unknown_Mask("READMEM16", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

//READMEM16 with sign extend to 32bit
GekkoRecILOp(READMEM16SX)
{
	u32				OutReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			//move to ecx if need be
			if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InReg)
			{
				if(Instruction->X86InReg != 1)
				{
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 1) << 8) | 0x89;
					*OutSize += 2;
				}

				//reset the register the value is in
				X86Regs[Instruction->X86InReg].PPCRegister = -1;
			}
			else
			{
				//mov ecx, [mem]
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
				*OutSize += 6;
			}

			//now read the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Read16 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);

			//movsx eax, ax
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xC0BF0F;
			*OutSize += 8;

			//reassign the eax register to our entry
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
			{
				X86Regs[0].PPCRegister = Instruction->OutReg;
				X86Regs[0].ValueChanged = 1;
			}
			else
			{
				//mov [mem], eax
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
				*OutSize += 5;
			}
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			//have a known place we are reading from, check if we need to do
			//a direct read or call a specific function
			*OutSize = 0;
			if(Instruction->X86InVal < 0xC8000000 || 
			   ((Instruction->X86InVal >= 0xE0000000) &&
			    (Instruction->X86InVal < 0xF0000000))
			  )
			{
				if(Instruction->X86InVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if(Instruction->X86InVal & 1)
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//pick a register if we did not find one
							if(OutReg == 4)
								OutReg = 1;
						}
					}
					else
					{
						//find a register we can use
						OutReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
					}

					if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged && !(X86Regs[OutReg].PPCRegister & REG_SPECIAL))
					{
						//register returned is in use, move it out to memory
						TempInst.InReg = 0;
						TempInst.OutReg = 0;
						TempInst.X86InReg = OutReg;
						TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
						TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;
					}

					X86Regs[OutReg].PPCRegister = -1;
				}
				else
				{
					//get our register to use
					OutReg = Instruction->X86OutReg;

					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if((Instruction->X86InVal & 1) && (Instruction->X86OutReg >= 4))
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//if no register then pick one
							if(OutReg == 4)
							{
								//register returned is in use, move it out to memory
								OutReg = 1;
								TempInst.InReg = 0;
								TempInst.OutReg = 0;
								TempInst.X86InReg = OutReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//change what register we are using
						X86Regs[OutReg].PPCRegister = X86Regs[Instruction->X86OutReg].PPCRegister;
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;
						Instruction->X86OutReg = OutReg;
					}
				}

				//if not aligned then handle it special
				if(Instruction->InVal & 1)
				{
					//mov reg[l], byte ptr (Mem+0)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+0)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//mov reg[h], byte ptr (Mem+1)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg + 4, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+1)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;
				}
				else
				{
					//aligned properly, just read it
					//mov reg, word [val]
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 16) | 0x8B66;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = ((Instruction->X86InVal ^ 2) & MemPtrMask) + MemPtrVal;
					*OutSize += 7;
				}

				//movsx reg, reg[x]
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBF0F;
				*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = ModRM(0x03, OutReg, OutReg);
				*OutSize += 3;

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//mov mem, reg
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x89;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
				}
				else
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else if(Instruction->X86InVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else //if(Instruction->X86InVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWRead16[(Instruction->X86InVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWRead16[(Instruction->X86InVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					//movsx eax, ax
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 10]) = 0xC0BF0F;
					*OutSize += 13;

					if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
					{
						//mov mem, eax
						*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
						*OutSize += 5;
					}
					else
					{
						//assign the register to eax
						X86Regs[0].PPCRegister = Instruction->OutReg;
						X86Regs[0].ValueChanged = 1;
					}
				}
			}

			break;

		default:
		Unknown_Mask("READMEM16SX", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

GekkoRecILOp(READMEM32)
{
	u32				OutReg;
	u32				OutReg2;
	u32				OutReg3;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;
	u32				JumpPos[5];

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
			*OutSize = 0;

			//if we are writing out to memory then get a temp out register
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
			{
				//find a register we can use
				OutReg = FindReplacableX86Reg(Instruction, X86Regs, -1, &FindRegMem, &FindCount, 0);

				if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged && !(X86Regs[OutReg].PPCRegister & REG_SPECIAL))
				{
					//register returned is in use, move it out to memory
					TempInst.InReg = 0;
					TempInst.OutReg = 0;
					TempInst.X86InReg = OutReg;
					TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
					TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
					GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
					*OutSize += NewOutSize;
				}
				X86Regs[OutReg].PPCRegister = -1;
			}
			else
				OutReg = Instruction->X86OutReg;

			//find a register we can use for work
			OutReg2 = FindReplacableX86Reg(Instruction, X86Regs, -1, &FindRegMem, &FindCount, 0);
			if(OutReg2 == 8 || OutReg == OutReg2)
			{
				//it told us to stay in memory and didn't have anything
				//try again
				OutReg2 = FindReplacableX86Reg(Instruction, X86Regs, -1, &FindRegMem, &FindCount, 1);
				if(OutReg2 == 8 || OutReg == OutReg2)
				{
					//just pick one
					//if OutReg is already ecx then pick the last
					//otherwise pick ecx incase a memory read is needed
					if(OutReg == 1)
					{
						OutReg2 = 7;
						OutReg3 = 6;
					}
					else
					{
						OutReg2 = 1;
						OutReg3 = (OutReg + 1) % 8;
					}
				}
				else
				{
					//assign out reg
					OutReg3 = 7;
					while(OutReg3 == OutReg2 || OutReg3 == OutReg)
						OutReg3--;
				}
			}
			else
			{
				//get one more register
				OutReg3 = FindReplacableX86Reg(Instruction, X86Regs, -1, &FindRegMem, &FindCount, 1);
				if(OutReg3 == 8 || OutReg == OutReg3 || OutReg2 == OutReg3)
				{
					//just pick one
					//if OutReg or outreg2 is already ecx then pick a register
					//otherwise pick ecx incase a memory read is needed
					if(OutReg == 1 || OutReg2 == 1)
					{
						OutReg3 = 7;
						while(OutReg3 == OutReg2 || OutReg3 == OutReg)
							OutReg3--;
					}
					else
						OutReg3 = 1;
				}
			}

			if((X86Regs[OutReg2].PPCRegister != -1) && X86Regs[OutReg2].ValueChanged && !(X86Regs[OutReg2].PPCRegister & REG_SPECIAL))
			{
				//register returned is in use, move it out to memory
				TempInst.InReg = 0;
				TempInst.OutReg = 0;
				TempInst.X86InReg = OutReg2;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg2].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
			X86Regs[OutReg2].PPCRegister = -1;

			if((X86Regs[OutReg3].PPCRegister != -1) && X86Regs[OutReg3].ValueChanged && !(X86Regs[OutReg3].PPCRegister & REG_SPECIAL))
			{
				//register returned is in use, move it out to memory
				TempInst.InReg = 0;
				TempInst.OutReg = 0;
				TempInst.X86InReg = OutReg3;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg3].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
				*OutSize += NewOutSize;
			}
			X86Regs[OutReg3].PPCRegister = -1;

			//compare if below 0xC8000000, if so then we can read directly
			//cmp reg/mem, 0xC8000000
			TempInst.InReg = 0;
			TempInst.OutReg = 0;
			TempInst.X86OutReg = Instruction->X86InReg;
			TempInst.X86InVal = 0xC8000000;
			TempInst.Flags = RecInstrFlagX86InVal;
			if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InReg)
				TempInst.Flags |= RecInstrFlagX86OutReg;
			else
				TempInst.Flags |= RecInstrFlagX86OutMem;
			GekkoRecIL(CMP)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;

			//mov OutReg, mem/reg in
			//mov reg2, mem/reg in
			TempInst.X86OutReg = OutReg;
			TempInst.X86InReg = Instruction->X86InReg;
			TempInst.Flags = RecInstrFlagX86OutReg;
			if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InReg)
				TempInst.Flags |= RecInstrFlagX86InReg;
			else
				TempInst.Flags |= RecInstrFlagX86InMem;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;

			TempInst.X86OutReg = OutReg2;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[*OutSize], &NewOutSize, X86Regs, 0);
			*OutSize += NewOutSize;

			//ja OtherRead
			JumpPos[0] = *OutSize + 2;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x772E;	//JA (branch not taken hint)
			*OutSize += 3;

			//and OutReg, RAM_MASK & ~3
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 4, OutReg) << 8) | 0x81;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = RAM_MASK & ~3;
			*OutSize += 6;

			//and reg2, 3
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 4, OutReg2) << 8) | 0x83;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 3;
			*OutSize += 3;

			//mov reg3, outreg
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, OutReg3, OutReg) << 8) | 0x8B;
			*OutSize += 2;

			//mov OutReg, [MemPtr + OutReg]
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x02, OutReg, OutReg) << 8) | 0x8B;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)Mem_RAM;
			*OutSize += 6;

			//jz EndRead (branch taken hint)
			JumpPos[1] = *OutSize + 3;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x840F2E;
			*OutSize += 7;

			//cmp reg2, 2
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 7, OutReg2) << 8) | 0x83;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 2;
			*OutSize += 3;

			//mov reg2, [MemPtr + OutReg3 + 4]
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x02, OutReg2, OutReg3) << 8) | 0x8B;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (u32)Mem_RAM + 4;
			*OutSize += 6;

			//jb Shift1
			JumpPos[2] = *OutSize + 1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x72;	//jb shift1
			*OutSize += 2;

			//je Shift2
			JumpPos[3] = *OutSize + 1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x74;	//je shift2
			*OutSize += 2;

			//shl OutReg, 8
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 4, OutReg) << 8) | 0xC1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 8;
			*OutSize += 3;

			//shr reg2, 24
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 5, OutReg2) << 8) | 0xC1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 24;
			*OutSize += 3;

			//jmp Combine
			JumpPos[4] = *OutSize + 1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xEB;	//jmp combine
			*OutSize += 2;

			//Shift2:
			*(u8 *)(&((u8 *)OutInstruction)[JumpPos[3]]) = *OutSize - JumpPos[3] - 1;

			//shl OutReg, 16
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 4, OutReg) << 8) | 0xC1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 16;
			*OutSize += 3;

			//shr OutReg, 16
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 5, OutReg2) << 8) | 0xC1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 16;
			*OutSize += 3;

			//jmp Combine
			JumpPos[3] = *OutSize + 1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xEB;	//jmp combine
			*OutSize += 2;

			//Shift1:
			*(u8 *)(&((u8 *)OutInstruction)[JumpPos[2]]) = *OutSize - JumpPos[2] - 1;

			//shl OutReg, 24
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 4, OutReg) << 8) | 0xC1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 24;
			*OutSize += 3;

			//shr OutReg, 8
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 5, OutReg2) << 8) | 0xC1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = 8;
			*OutSize += 3;

			*(u8 *)(&((u8 *)OutInstruction)[JumpPos[4]]) = *OutSize - JumpPos[4] - 1;
			*(u8 *)(&((u8 *)OutInstruction)[JumpPos[3]]) = *OutSize - JumpPos[3] - 1;

			//Combine:
			//or OutReg, reg2
			*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, OutReg, OutReg2) << 8) | 0x0B;
			*OutSize += 2;

			//jmp EndRead
			JumpPos[2] = *OutSize + 1;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE9;	//jmp EndRead
			*OutSize += 5;

			*(u8 *)(&((u8 *)OutInstruction)[JumpPos[0]]) = *OutSize - JumpPos[0] - 1;

			//OtherRead:
			//we are going to make sure out is not saved, no need
			NewOutSize = X86Regs[OutReg].PPCRegister;
			X86Regs[OutReg].PPCRegister = -1;

			GekkoRecIL(SaveAllX86Regs2)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs2)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			//if need be, move the value to ecx
			if(OutReg != 1 && OutReg2 != 1 && OutReg3 != 1)
			{
				//mov ecx, OutReg
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 1, OutReg) << 8) | 0x8B;
				*OutSize += 2;
			}

			//call ReadMem32
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Read32 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);
			*OutSize += 5;

			//store eax to the out register we are using
			//mov OutReg, eax
			if(OutReg != 0)
			{
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, OutReg, 0) << 8) | 0x8B;
				*OutSize += 2;
			}

			//GetRegs
			GekkoRecIL(GetAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(GetAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			//now update X86Regs to indicate that the out register is used
			X86Regs[OutReg].PPCRegister = NewOutSize;
			X86Regs[OutReg].ValueChanged = 1;

			//EndRead:

			*(u32 *)(&((u8 *)OutInstruction)[JumpPos[1]]) = *OutSize - JumpPos[1] - 4;
			*(u32 *)(&((u8 *)OutInstruction)[JumpPos[2]]) = *OutSize - JumpPos[2] - 4;


			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			//have a known place we are reading from, check if we need to do
			//a direct read or call a specific function
			*OutSize = 0;
			if(Instruction->X86InVal < 0xC8000000 || 
			   ((Instruction->X86InVal >= 0xE0000000) &&
			    (Instruction->X86InVal < 0xF0000000))
			  )
			{
				if(Instruction->X86InVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if(Instruction->X86InVal & 3)
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//pick a register if we did not find one
							if(OutReg == 4)
								OutReg = 1;
						}
					}
					else
					{
						//find a register we can use
						OutReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
					}

					if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged && !(X86Regs[OutReg].PPCRegister & REG_SPECIAL))
					{
						//register returned is in use, move it out to memory
						TempInst.InReg = 0;
						TempInst.OutReg = 0;
						TempInst.X86InReg = OutReg;
						TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
						TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;
					}

					X86Regs[OutReg].PPCRegister = -1;
				}
				else
				{
					//get our register to use
					OutReg = Instruction->X86OutReg;

					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if((Instruction->X86InVal & 3) && (Instruction->X86OutReg >= 4))
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//if no register then pick one
							if(OutReg == 4)
							{
								//register returned is in use, move it out to memory
								OutReg = 1;
								TempInst.InReg = 0;
								TempInst.OutReg = 0;
								TempInst.X86InReg = OutReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//change what register we are using
						X86Regs[OutReg].PPCRegister = X86Regs[Instruction->X86OutReg].PPCRegister;
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;
						Instruction->X86OutReg = OutReg;
					}
				}

				//if not aligned then handle it special
				if(Instruction->InVal & 3)
				{
					//mov reg[l], byte ptr (Mem+0)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+0)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//mov reg[h], byte ptr (Mem+1)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg + 4, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+1)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//shl reg, 16
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x100000 | (ModRM(0x03, 4, OutReg) << 8) | 0xC1;
					*OutSize += 3;

					//mov reg[l], byte ptr (Mem+2)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+2)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//mov reg[h], byte ptr (Mem+3)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg + 4, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+3)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;
				}
				else
				{
					//aligned properly, just read it
					//mov reg, [val]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (Instruction->X86InVal & MemPtrMask) + MemPtrVal;
					*OutSize += 6;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//mov mem, reg
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x89;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
				}
				else
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else if(Instruction->X86InVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else // if(Instruction->X86InVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWRead32[(Instruction->X86InVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					//reset the register the value is in if it exists
					if(((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg))
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWRead32[(Instruction->X86InVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					*OutSize += 10;

					if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
					{
						//mov mem, eax
						*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
						*OutSize += 5;
					}
					else
					{
						//assign the register to eax
						X86Regs[0].PPCRegister = Instruction->OutReg;
						X86Regs[0].ValueChanged = 1;
					}
				}
			}

			break;

		default:
		Unknown_Mask("READMEM32", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}
/*
GekkoRecILOp(READMEM32)
{
	u32				OutReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			//move to ecx if need be
			if((Instruction->Flags & RecInstrFlagX86_INMASK) == RecInstrFlagX86InReg)
			{
				if(Instruction->X86InReg != 1)
				{
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 1) << 8) | 0x89;
					*OutSize += 2;
				}

				//reset the register the value is in
				X86Regs[Instruction->X86InReg].PPCRegister = -1;
			}
			else
			{
				//mov ecx, [mem]
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
				*OutSize += 6;
			}

			//now read the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Read32 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);
			*OutSize += 5;

			//reassign the eax register to our entry
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg)
			{
				X86Regs[0].PPCRegister = Instruction->OutReg;
				X86Regs[0].ValueChanged = 1;
			}
			else
			{
				//mov [mem], eax
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
				*OutSize += 5;
			}
			break;

		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			//have a known place we are reading from, check if we need to do
			//a direct read or call a specific function
			*OutSize = 0;
			if(Instruction->X86InVal < 0xC8000000 || 
			   ((Instruction->X86InVal >= 0xE0000000) &&
			    (Instruction->X86InVal < 0xF0000000))
			  )
			{
				if(Instruction->X86InVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if(Instruction->X86InVal & 3)
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//pick a register if we did not find one
							if(OutReg == 4)
								OutReg = 1;
						}
					}
					else
					{
						//find a register we can use
						OutReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
					}

					if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged && !(X86Regs[OutReg].PPCRegister & REG_SPECIAL))
					{
						//register returned is in use, move it out to memory
						TempInst.InReg = 0;
						TempInst.OutReg = 0;
						TempInst.X86InReg = OutReg;
						TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
						TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;
					}

					X86Regs[OutReg].PPCRegister = -1;
				}
				else
				{
					//get our register to use
					OutReg = Instruction->X86OutReg;

					//if the read is not aligned and not a good register to use
					//then find a different register to use
					if((Instruction->X86InVal & 3) && (Instruction->X86OutReg >= 4))
					{
						//find a register from E*X that we can use
						for(OutReg = 0; OutReg < 4; OutReg++)
						{
							if(X86Regs[OutReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(OutReg == 4)
						{
							for(OutReg = 0; OutReg < 4; OutReg++)
							{
								if(!X86Regs[OutReg].ValueChanged)
									break;
							}

							//if no register then pick one
							if(OutReg == 4)
							{
								//register returned is in use, move it out to memory
								OutReg = 1;
								TempInst.InReg = 0;
								TempInst.OutReg = 0;
								TempInst.X86InReg = OutReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//change what register we are using
						X86Regs[OutReg].PPCRegister = X86Regs[Instruction->X86OutReg].PPCRegister;
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;
						Instruction->X86OutReg = OutReg;
					}
				}

				//if not aligned then handle it special
				if(Instruction->InVal & 3)
				{
					//mov reg[l], byte ptr (Mem+0)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+0)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//mov reg[h], byte ptr (Mem+1)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg + 4, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+1)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//shl reg, 16
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x100000 | (ModRM(0x03, 4, OutReg) << 8) | 0xC1;
					*OutSize += 3;

					//mov reg[l], byte ptr (Mem+2)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+2)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;

					//mov reg[h], byte ptr (Mem+3)^3
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg + 4, 0x05) << 8) | 0x8A;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86InVal+3)^3) & MemPtrMask) + MemPtrVal;
					*OutSize += 6;
				}
				else
				{
					//aligned properly, just read it
					//mov reg, [val]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (Instruction->X86InVal & MemPtrMask) + MemPtrVal;
					*OutSize += 6;
				}

				if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
				{
					//mov mem, reg
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 8) | 0x89;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
				}
				else
					X86Regs[Instruction->X86OutReg].ValueChanged = 1;
			}
			else if(Instruction->X86InVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else // if(Instruction->X86InVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWRead32[(Instruction->X86InVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					//reset the register the value is in if it exists
					if(((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutReg))
						X86Regs[Instruction->X86OutReg].PPCRegister = -1;

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWRead32[(Instruction->X86InVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					*OutSize += 10;

					if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
					{
						//mov mem, eax
						*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xA3;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutMemory;
						*OutSize += 5;
					}
					else
					{
						//assign the register to eax
						X86Regs[0].PPCRegister = Instruction->OutReg;
						X86Regs[0].ValueChanged = 1;
					}
				}
			}

			break;

		default:
		Unknown_Mask("READMEM32", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}
*/

GekkoRecILOp(READMEM64)
{
	u32				OutReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutFloatReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutFloatReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			//have a known place we are reading from, check if we need to do
			//a direct read or read via a register
			*OutSize = 0;

			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
			{
				//find a register we can use
				OutReg = FindReplacableFPUReg(Instruction, FPURegs, -2, &FindRegMem, &FindCount, 0);

				if((FPURegs[OutReg].PPCRegister != -1) && FPURegs[OutReg].ValueChanged)
				{
					//register returned is in use, move it out to memory
					TempInst.InReg = 0;
					TempInst.OutReg = 0;
					TempInst.X86InReg = OutReg;
					TempInst.X86OutMemory = (u32)&ireg.fpr[FPURegs[OutReg].PPCRegister];
					TempInst.Flags = RecInstrFlagX86InFloatReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInFloatReg | RecInstrFlagPPCOutFloatReg;
					GekkoRecIL(MOVAPD)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, FPURegs, 0);
					*OutSize += NewOutSize;
				}
			}
			else
				OutReg = Instruction->X86OutReg;

			//read the value
			if(Instruction->Flags & RecInstrFlagX86InReg)
			{
				//the register in use is a temp register, adjust it for the memory information
				//and reg, RAM_MASK
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 4, Instruction->X86InReg) << 8) | 0x81;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = RAM_MASK;
				*OutSize += 6;

				//movq reg, [reg + Mem_RAM]
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x02, OutReg, Instruction->X86InReg) << 24) | 0x7E0FF3;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 4]) = (u32)Mem_RAM;
				*OutSize += 8;
			}
			else
			{
				//movq reg, [val]
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 24) | 0x7E0FF3;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 4]) = (Instruction->X86InVal & RAM_MASK) + (u32)Mem_RAM;
				*OutSize += 8;
			}

			//now swap the dwords around in the lower portion
			//pshufd OutReg, OutReg, 0xE1 (11100001b)
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, OutReg, OutReg) << 24) | 0x700F66;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 4]) = 0xE1;
			*OutSize += 5;

			//if memory, write it back out
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
			{
				//push it back out to memory
				//movq [Mem], reg
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, OutReg, 0x05) << 24) | 0xD60F66;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 4]) = Instruction->X86OutMemory;
				*OutSize += 8;

				FPURegs[OutReg].PPCRegister = -1;
			}
			else
			{
				//indicate the register is changed
				FPURegs[OutReg].PPCRegister = Instruction->OutReg;
				FPURegs[OutReg].ValueChanged = 1;
			}

			break;

		default:
		Unknown_Mask("READMEM64", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

GekkoRecILOp(WRITEMEM8)
{
	u32				InReg;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			switch(Instruction->Flags & RecInstrFlagX86_MASK)
			{
				case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
					//move our values to ecx and edx if need be

					if(Instruction->X86OutReg != 1)
					{
						//addr is not in ecx, check if it is in edx
						//if so then we need to see if data is in ecx
						//if it is in ecx then xchg, otherwise do mov's
						if(Instruction->X86OutReg == 2)
						{
							//addr in edx, see if data is in ecx
							if(Instruction->X86InReg == 1)
							{
								//data in ecx, xchg
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x87;
								*OutSize += 2;
								Instruction->X86InReg = 2;		//avoid tripping the if below
							}
							else
							{
								//data not in ecx, normal move
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
								*OutSize += 2;
							}
						}
						else if(Instruction->X86InReg == 1)
						{
							//data in ecx, addr not in edx, move data first
							//mov edx, data
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
							*OutSize += 2;
							//mov ecx, addr
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
							*OutSize += 2;

							//avoid tripping the if below
							Instruction->X86InReg = 2;
						}
						else
						{
							//addr not in ecx, data not in edx
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
							*OutSize += 2;
						}
					}
					if(Instruction->X86InReg != 2)
					{
						//addr is fine, set data
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
						*OutSize += 2;
					}
					break;

				case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
					//InMem, OutReg
					if(Instruction->X86OutReg != 1)
					{
						//move addr to ecx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov edx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x158B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
					*OutSize += 6;
					break;

				case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
					if(Instruction->X86InReg != 2)
					{
						//move data to edx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov ecx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
					break;

				case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
					if(Instruction->X86OutReg != 1)
					{
						//move addr to ecx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov edx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal & 0xFF;
					*OutSize += 5;
					break;

				case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
					//mov ecx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;

					//mov edx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal & 0xFF;
					*OutSize += 5;
					break;
			}

			//now write the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Write8 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);

			*OutSize += 5;
			break;

		case (RecInstrFlagX86OutVal | RecInstrFlagX86InReg):
		case (RecInstrFlagX86OutVal | RecInstrFlagX86InMem):
		case (RecInstrFlagX86OutVal | RecInstrFlagX86InVal):
			//have a known place we are writing to, check if we need to do
			//a direct write or call a specific function
			*OutSize = 0;
			if(Instruction->X86OutVal < 0xC8000000 || 
			   ((Instruction->X86OutVal >= 0xE0000000) &&
			    (Instruction->X86OutVal < 0xF0000000))
			  )
			{
				if(Instruction->X86OutVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				InReg = -1;
				switch(Instruction->Flags & RecInstrFlagX86_INMASK)
				{
					case (RecInstrFlagX86InMem):
						//find a register from E*X that we can use
						for(InReg = 0; InReg < 4; InReg++)
						{
							if(X86Regs[InReg].PPCRegister == -1)
								break;
						}

						//if we ran out of registers then see if there is a register
						//with an unchanged value that we can take over
						if(InReg == 4)
						{
							for(InReg = 0; InReg < 4; InReg++)
							{
								if(!X86Regs[InReg].ValueChanged)
									break;
							}

							//pick a register if we did not find one
							if(InReg == 4)
								InReg = 1;
						}

						if((X86Regs[InReg].PPCRegister != -1) && X86Regs[InReg].ValueChanged && !(X86Regs[InReg].PPCRegister & REG_SPECIAL))
						{
							//register returned is in use, move it out to memory
							TempInst.X86InReg = InReg;
							TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[InReg].PPCRegister];
							TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
							GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
							*OutSize += NewOutSize;
						}

						X86Regs[InReg].PPCRegister = -1;

						//mov reg, [inmem]
						TempInst.X86OutReg = InReg;
						TempInst.X86InMemory = Instruction->X86InMemory;
						TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;

						Instruction->X86InReg = InReg;

						//fall thru
					case (RecInstrFlagX86InReg):
						if(InReg == -1)
						{
							InReg = Instruction->X86InReg;

							if(InReg >= 4)
							{
								//find a register from E*X that we can use
								for(InReg = 0; InReg < 4; InReg++)
								{
									if(X86Regs[InReg].PPCRegister == -1)
										break;
								}

								//if we ran out of registers then see if there is a register
								//with an unchanged value that we can take over
								if(InReg == 4)
								{
									for(InReg = 0; InReg < 4; InReg++)
									{
										if(!X86Regs[InReg].ValueChanged)
											break;
									}

									//pick a register if we did not find one
									if(InReg == 4)
										InReg = 1;
								}
							}

							if((X86Regs[InReg].PPCRegister != -1) && X86Regs[InReg].ValueChanged && !(X86Regs[InReg].PPCRegister & REG_SPECIAL))
							{
								//register returned is in use, move it out to memory
								TempInst.X86InReg = InReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[InReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}

							X86Regs[InReg].PPCRegister = -1;

							if(InReg != Instruction->X86InReg)
							{
								//mov reg, oldreg
								TempInst.X86OutReg = InReg;
								TempInst.X86InReg = Instruction->X86InReg;
								TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//mov byte &Mem_RAM[outval ^ 3], inreg
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg, 0x05) << 8) | 0x88;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = MemPtrVal + ((Instruction->X86OutVal ^ 3) & MemPtrMask);
						*OutSize += 6;
						break;

					case (RecInstrFlagX86InVal):
						//mov byte &Mem_RAM[outval ^ 3], inval
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C6;
						*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = MemPtrVal + ((Instruction->X86OutVal ^ 3) & MemPtrMask);
						*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = Instruction->X86InVal & 0xFF;
						*OutSize += 7;
				}
			}
			else if(Instruction->X86OutVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else //if(Instruction->X86OutVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWWrite8[(Instruction->X86OutVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					switch((Instruction->Flags & RecInstrFlagX86_INMASK))
					{
						case RecInstrFlagX86InReg:
							if(Instruction->X86InReg != 2)
							{
								//data not in edx, normal move
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
								*OutSize += 2;
							}
							break;

						case RecInstrFlagX86InMem:
							//mov edx, [mem]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x158B;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
							*OutSize += 6;
							break;

						case RecInstrFlagX86InVal:
							//mov edx, val
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal & 0xFF;
							*OutSize += 5;
							break;
					}

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWWrite8[(Instruction->X86OutVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					*OutSize += 10;
				}
			}

			break;

		default:
		Unknown_Mask("WRITEMEM8", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

GekkoRecILOp(WRITEMEM16)
{
	u32				InReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;
			switch(Instruction->Flags & RecInstrFlagX86_MASK)
			{
				case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
					//move our values to ecx and edx if need be

					if(Instruction->X86OutReg != 1)
					{
						//addr is not in ecx, check if it is in edx
						//if so then we need to see if data is in ecx
						//if it is in ecx then xchg, otherwise do mov's
						if(Instruction->X86OutReg == 2)
						{
							//addr in edx, see if data is in ecx
							if(Instruction->X86InReg == 1)
							{
								//data in ecx, xchg
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x87;
								*OutSize += 2;
								Instruction->X86InReg = 2;		//avoid tripping the if below
							}
							else
							{
								//data not in ecx, normal move
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
								*OutSize += 2;
							}
						}
						else if(Instruction->X86InReg == 1)
						{
							//data in ecx, addr not in edx, move data first
							//mov edx, data
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
							*OutSize += 2;
							//mov ecx, addr
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
							*OutSize += 2;

							//avoid tripping the if below
							Instruction->X86InReg = 2;
						}
						else
						{
							//addr not in ecx, data not in edx
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
							*OutSize += 2;
						}
					}
					if(Instruction->X86InReg != 2)
					{
						//addr is fine, set data
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
						*OutSize += 2;
					}
					break;

				case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
					//InMem, OutReg
					if(Instruction->X86OutReg != 1)
					{
						//move addr to ecx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov edx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x158B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
					*OutSize += 6;
					break;

				case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
					if(Instruction->X86InReg != 2)
					{
						//move data to edx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov ecx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
					break;

				case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
					if(Instruction->X86OutReg != 1)
					{
						//move addr to ecx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov edx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal & 0xFFFF;
					*OutSize += 5;
					break;

				case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
					//mov ecx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;

					//mov edx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal & 0xFFFF;
					*OutSize += 5;
					break;
			}

			//now write the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Write16 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);

			*OutSize += 5;
			break;

		case (RecInstrFlagX86OutVal | RecInstrFlagX86InReg):
		case (RecInstrFlagX86OutVal | RecInstrFlagX86InMem):
		case (RecInstrFlagX86OutVal | RecInstrFlagX86InVal):
			//have a known place we are writing to, check if we need to do
			//a direct write or call a specific function
			*OutSize = 0;
			if(Instruction->X86OutVal < 0xC8000000 || 
			   ((Instruction->X86OutVal >= 0xE0000000) &&
			    (Instruction->X86OutVal < 0xF0000000))
			  )
			{
				if(Instruction->X86OutVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				InReg = -1;
				switch(Instruction->Flags & RecInstrFlagX86_INMASK)
				{
					case (RecInstrFlagX86InMem):
						if(Instruction->X86OutVal & 1)
						{
							//find a register from E*X that we can use
							for(InReg = 0; InReg < 4; InReg++)
							{
								if(X86Regs[InReg].PPCRegister == -1)
									break;
							}

							//if we ran out of registers then see if there is a register
							//with an unchanged value that we can take over
							if(InReg == 4)
							{
								for(InReg = 0; InReg < 4; InReg++)
								{
									if(!X86Regs[InReg].ValueChanged)
										break;
								}

								//pick a register if we did not find one
								if(InReg == 4)
									InReg = 1;
							}
						}
						else
						{
							//we are aligned, use any register
							InReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
						}

						if((X86Regs[InReg].PPCRegister != -1) && X86Regs[InReg].ValueChanged && !(X86Regs[InReg].PPCRegister & REG_SPECIAL))
						{
							//register returned is in use, move it out to memory
							TempInst.X86InReg = InReg;
							TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[InReg].PPCRegister];
							TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
							GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
							*OutSize += NewOutSize;
						}

						X86Regs[InReg].PPCRegister = -1;

						//mov reg, [inmem]
						TempInst.X86OutReg = InReg;
						TempInst.X86InMemory = Instruction->X86InMemory;
						TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;

						Instruction->X86InReg = InReg;

						//fall thru
					case (RecInstrFlagX86InReg):
						if(InReg == -1)
						{
							InReg = Instruction->X86InReg;

							//if the register is not E*X and the position to
							//write to is not aligned then find a new register to use
							if(InReg >= 4 && (Instruction->X86OutVal & 1))
							{
								//find a register from E*X that we can use
								for(InReg = 0; InReg < 4; InReg++)
								{
									if(X86Regs[InReg].PPCRegister == -1)
										break;
								}

								//if we ran out of registers then see if there is a register
								//with an unchanged value that we can take over
								if(InReg == 4)
								{
									for(InReg = 0; InReg < 4; InReg++)
									{
										if(!X86Regs[InReg].ValueChanged)
											break;
									}

									//pick a register if we did not find one
									if(InReg == 4)
										InReg = 1;
								}
							}

							if((X86Regs[InReg].PPCRegister != -1) && X86Regs[InReg].ValueChanged && !(X86Regs[InReg].PPCRegister & REG_SPECIAL))
							{
								//register returned is in use, move it out to memory
								TempInst.X86InReg = InReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[InReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}

							X86Regs[InReg].PPCRegister = -1;

							if(InReg != Instruction->X86InReg)
							{
								//mov reg, oldreg
								TempInst.X86OutReg = InReg;
								TempInst.X86InReg = Instruction->X86InReg;
								TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//if not aligned then handle it special
						if(Instruction->X86OutVal & 1)
						{
							//mov byte ptr (Mem+0)^3, reg[l]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg, 0x05) << 8) | 0x88;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+0)^3) & MemPtrMask) + MemPtrVal;
							*OutSize += 6;

							//mov byte ptr (Mem+1)^3, reg[h]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg + 4, 0x05) << 8) | 0x88;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+1)^3) & MemPtrMask) + MemPtrVal;
							*OutSize += 6;
						}
						else
						{
							//mov &RAM[outval], inreg
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg, 0x05) << 16) | 0x8966;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = MemPtrVal + ((Instruction->X86OutVal ^ 2) & MemPtrMask);
							*OutSize += 7;
						}
						break;

					case (RecInstrFlagX86InVal):
						//if not aligned then handle it special
						if(Instruction->X86OutVal & 3)
						{
							//mov byte ptr (Mem+0)^3, InVal >> 8
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C6;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+0)^3) & MemPtrMask) + MemPtrVal;
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (Instruction->X86InVal >> 8) & 0xFF;
							*OutSize += 7;

							//mov byte ptr (Mem+1)^3, InVal >> 0
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C6;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+1)^3) & MemPtrMask) + MemPtrVal;
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = Instruction->X86InVal & 0xFF;
							*OutSize += 7;
						}
						else
						{
							//mov &RAM[outval], inval
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C766;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 3]) = MemPtrVal + ((Instruction->X86OutVal ^ 2) & MemPtrMask);
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize + 7]) = Instruction->X86InVal & 0xFFFF;
							*OutSize += 9;
						}
				}
			}
			else if(Instruction->X86OutVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else //if(Instruction->X86OutVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWWrite16[(Instruction->X86OutVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					switch((Instruction->Flags & RecInstrFlagX86_INMASK))
					{
						case RecInstrFlagX86InReg:
							if(Instruction->X86InReg != 2)
							{
								//data not in edx, normal move
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
								*OutSize += 2;
							}
							break;

						case RecInstrFlagX86InMem:
							//mov edx, [mem]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x158B;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
							*OutSize += 6;
							break;

						case RecInstrFlagX86InVal:
							//mov edx, val
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal & 0xFFFF;
							*OutSize += 5;
							break;
					}

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWWrite16[(Instruction->X86OutVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					*OutSize += 10;
				}
			}

			break;

		default:
		Unknown_Mask("WRITEMEM16", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}

GekkoRecILOp(WRITEMEM32)
{
	u32				InReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;
	u32				MemPtrVal;
	u32				MemPtrMask;
	u32				SaveSize;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
		case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
			*OutSize = 0;
			GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
			*OutSize += SaveSize;
			GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
			*OutSize += SaveSize;

			switch(Instruction->Flags & RecInstrFlagX86_MASK)
			{
				case (RecInstrFlagX86InReg | RecInstrFlagX86OutReg):
					//move our values to ecx and edx if need be

					if(Instruction->X86OutReg != 1)
					{
						//addr is not in ecx, check if it is in edx
						//if so then we need to see if data is in ecx
						//if it is in ecx then xchg, otherwise do mov's
						if(Instruction->X86OutReg == 2)
						{
							//addr in edx, see if data is in ecx
							if(Instruction->X86InReg == 1)
							{
								//data in ecx, xchg
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x87;
								*OutSize += 2;
								Instruction->X86InReg = 2;		//avoid tripping the if below
							}
							else
							{
								//data not in ecx, normal move
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
								*OutSize += 2;
							}
						}
						else if(Instruction->X86InReg == 1)
						{
							//data in ecx, addr not in edx, move data first
							//mov edx, data
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
							*OutSize += 2;
							//mov ecx, addr
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
							*OutSize += 2;

							//avoid tripping the if below
							Instruction->X86InReg = 2;
						}
						else
						{
							//addr not in ecx, data not in edx
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
							*OutSize += 2;
						}
					}
					if(Instruction->X86InReg != 2)
					{
						//addr is fine, set data
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
						*OutSize += 2;
					}
					break;

				case (RecInstrFlagX86InMem | RecInstrFlagX86OutReg):
					//InMem, OutReg
					if(Instruction->X86OutReg != 1)
					{
						//move addr to ecx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov edx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x158B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
					*OutSize += 6;
					break;

				case (RecInstrFlagX86InReg | RecInstrFlagX86OutMem):
					if(Instruction->X86InReg != 2)
					{
						//move data to edx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov ecx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;
					break;

				case (RecInstrFlagX86InVal | RecInstrFlagX86OutReg):
					if(Instruction->X86OutReg != 1)
					{
						//move addr to ecx
						*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86OutReg, 1) << 8) | 0x89;
						*OutSize += 2;
					}
					
					//mov edx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;
					*OutSize += 5;
					break;

				case (RecInstrFlagX86InVal | RecInstrFlagX86OutMem):
					//mov ecx, [mem]
					*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x0D8B;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86OutMemory;
					*OutSize += 6;

					//mov edx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;
					*OutSize += 5;
					break;
			}

			//now write the memory location
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xE8;
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = (u32)&Memory_Write32 - (u32)(&((u8 *)OutInstruction)[*OutSize + 5]);

			*OutSize += 5;
			break;

		case (RecInstrFlagX86OutVal | RecInstrFlagX86InReg):
		case (RecInstrFlagX86OutVal | RecInstrFlagX86InMem):
		case (RecInstrFlagX86OutVal | RecInstrFlagX86InVal):
			//have a known place we are writing to, check if we need to do
			//a direct write or call a specific function
			*OutSize = 0;
			if(Instruction->X86OutVal < 0xC8000000 || 
			   ((Instruction->X86OutVal >= 0xE0000000) &&
			    (Instruction->X86OutVal < 0xF0000000))
			  )
			{
				if(Instruction->X86OutVal < 0xC8000000)
				{
					MemPtrVal = (u32)Mem_RAM;
					MemPtrMask = RAM_MASK;
				}
				else
				{
					MemPtrVal = (u32)Mem_L2;
					MemPtrMask = L2_MASK;
				}

				InReg = -1;
				switch(Instruction->Flags & RecInstrFlagX86_INMASK)
				{
					case (RecInstrFlagX86InMem):
						if(Instruction->X86OutVal & 3)
						{
							//find a register from E*X that we can use
							for(InReg = 0; InReg < 4; InReg++)
							{
								if(X86Regs[InReg].PPCRegister == -1)
									break;
							}

							//if we ran out of registers then see if there is a register
							//with an unchanged value that we can take over
							if(InReg == 4)
							{
								for(InReg = 0; InReg < 4; InReg++)
								{
									if(!X86Regs[InReg].ValueChanged)
										break;
								}

								//pick a register if we did not find one
								if(InReg == 4)
									InReg = 1;
							}
						}
						else
						{
							//we are aligned, use any register
							InReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);
						}

						if((X86Regs[InReg].PPCRegister != -1) && X86Regs[InReg].ValueChanged && !(X86Regs[InReg].PPCRegister & REG_SPECIAL))
						{
							//register returned is in use, move it out to memory
							TempInst.X86InReg = InReg;
							TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[InReg].PPCRegister];
							TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
							GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
							*OutSize += NewOutSize;
						}

						X86Regs[InReg].PPCRegister = -1;

						//mov reg, [inmem]
						TempInst.X86OutReg = InReg;
						TempInst.X86InMemory = Instruction->X86InMemory;
						TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InMem;
						GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
						*OutSize += NewOutSize;

						Instruction->X86InReg = InReg;

						//fall thru
					case (RecInstrFlagX86InReg):
						if(InReg == -1)
						{
							InReg = Instruction->X86InReg;

							//if the register is not E*X and the position to
							//write to is not aligned then find a new register to use
							if(InReg >= 4 && (Instruction->X86OutVal & 3))
							{
								//find a register from E*X that we can use
								for(InReg = 0; InReg < 4; InReg++)
								{
									if(X86Regs[InReg].PPCRegister == -1)
										break;
								}

								//if we ran out of registers then see if there is a register
								//with an unchanged value that we can take over
								if(InReg == 4)
								{
									for(InReg = 0; InReg < 4; InReg++)
									{
										if(!X86Regs[InReg].ValueChanged)
											break;
									}

									//pick a register if we did not find one
									if(InReg == 4)
										InReg = 1;
								}
							}

							if((X86Regs[InReg].PPCRegister != -1) && X86Regs[InReg].ValueChanged && !(X86Regs[InReg].PPCRegister & REG_SPECIAL))
							{
								//register returned is in use, move it out to memory
								TempInst.X86InReg = InReg;
								TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[InReg].PPCRegister];
								TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}

							X86Regs[InReg].PPCRegister = -1;

							if(InReg != Instruction->X86InReg)
							{
								//mov reg, oldreg
								TempInst.X86OutReg = InReg;
								TempInst.X86InReg = Instruction->X86InReg;
								TempInst.Flags = RecInstrFlagX86OutReg | RecInstrFlagX86InReg;
								GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, X86Regs, 0);
								*OutSize += NewOutSize;
							}
						}

						//if not aligned then handle it special
						if(Instruction->X86OutVal & 3)
						{
							//mov byte ptr (Mem+0)^3, reg[l]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg, 0x05) << 8) | 0x88;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+0)^3) & MemPtrMask) + MemPtrVal;
							*OutSize += 6;

							//mov byte ptr (Mem+1)^3, reg[h]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg + 4, 0x05) << 8) | 0x88;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+1)^3) & MemPtrMask) + MemPtrVal;
							*OutSize += 6;

							//shr reg, 16
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x100000 | (ModRM(0x03, 5, InReg) << 8) | 0xC1;
							*OutSize += 3;

							//mov byte ptr (Mem+2)^3, reg[l]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg, 0x05) << 8) | 0x88;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+2)^3) & MemPtrMask) + MemPtrVal;
							*OutSize += 6;

							//mov byte ptr (Mem+3)^3, reg[h]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg + 4, 0x05) << 8) | 0x88;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+3)^3) & MemPtrMask) + MemPtrVal;
							*OutSize += 6;
						}
						else
						{
							//mov &RAM[outval], inreg
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, InReg, 0x05) << 8) | 0x89;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = MemPtrVal + (Instruction->X86OutVal & MemPtrMask);
							*OutSize += 6;
						}
						break;

					case (RecInstrFlagX86InVal):
						//if not aligned then handle it special
						if(Instruction->X86OutVal & 3)
						{
							//mov byte ptr (Mem+0)^3, InVal >> 24
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C6;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+0)^3) & MemPtrMask) + MemPtrVal;
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = (Instruction->X86InVal >> 24) & 0xFF;
							*OutSize += 7;

							//mov byte ptr (Mem+1)^3, InVal >> 16
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C6;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+1)^3) & MemPtrMask) + MemPtrVal;
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = (Instruction->X86InVal >> 16) & 0xFF;
							*OutSize += 7;

							//mov byte ptr (Mem+2)^3, InVal >> 8
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C6;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+2)^3) & MemPtrMask) + MemPtrVal;
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = (Instruction->X86InVal >> 8) & 0xFF;
							*OutSize += 7;

							//mov byte ptr (Mem+3)^3, InVal >> 0
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C6;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = (((Instruction->X86OutVal+3)^3) & MemPtrMask) + MemPtrVal;
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = Instruction->X86InVal & 0xFF;
							*OutSize += 7;
						}
						else
						{
							//mov &RAM[outval], inval
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x05C7;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = MemPtrVal + (Instruction->X86OutVal & MemPtrMask);
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = Instruction->X86InVal;
							*OutSize += 10;
						}
				}
			}
			else if(Instruction->X86OutVal < 0xCC000000)
			{
				//EFB
				printf("Found invalid EFB access during recompile, 0x%08X\n", Instruction->Address);
			}
			else //if(Instruction->X86OutVal < 0xE0000000)
			{
				//if we have a function to call, call it
				if((u32)GekkoHWWrite32[(Instruction->X86OutVal & 0x0000FF00) >> 8])
				{
					//Hardware access
					*OutSize = 0;
					GekkoRecIL(SaveAllX86Regs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, X86Regs, 0);
					*OutSize += SaveSize;
					GekkoRecIL(SaveAllFPURegs)(Instruction, &((u8 *)OutInstruction)[*OutSize], &SaveSize, 0, FPURegs);
					*OutSize += SaveSize;

					switch((Instruction->Flags & RecInstrFlagX86_INMASK))
					{
						case RecInstrFlagX86InReg:
							if(Instruction->X86InReg != 2)
							{
								//data not in edx, normal move
								*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, 2) << 8) | 0x89;
								*OutSize += 2;
							}
							break;

						case RecInstrFlagX86InMem:
							//mov edx, [mem]
							*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = 0x158B;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = Instruction->X86InMemory;
							*OutSize += 6;
							break;

						case RecInstrFlagX86InVal:
							//mov edx, val
							*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xBA;
							*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86InVal;
							*OutSize += 5;
							break;
					}

					//mov ecx, val
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize]) = 0xB9;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 1]) = Instruction->X86OutVal;

					//call out function
					*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 5]) = 0xE8;
					*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 6]) = (u32)GekkoHWWrite32[(Instruction->X86OutVal & 0x0000FF00) >> 8] - (u32)(&((u8 *)OutInstruction)[*OutSize + 10]);

					*OutSize += 10;
				}
			}

			break;

		default:
			Unknown_Mask("WRITEMEM32", (Instruction->Flags & RecInstrFlagX86_MASK));
			printf("Instruction PC: 0x%08X\n", Instruction->Address);
			break;
	}
}

GekkoRecILOp(WRITEMEM64)
{
	u32				OutReg;
	u32				FindRegMem;
	u32				FindCount;
	u32				NewOutSize;
	RecInstruction	TempInst;

	switch(Instruction->Flags & RecInstrFlagX86_MASK)
	{
		case (RecInstrFlagX86InFloatReg | RecInstrFlagX86OutReg):
		case (RecInstrFlagX86InFloatReg | RecInstrFlagX86OutMem):
			//the register is a temp register so we can modify it as needed

			//have a known place we are writing to, check if we need to do
			//a direct write or write thru a register
			*OutSize = 0;

			//now swap the dwords around in the lower portion
			//pshufd InReg, InReg, 0xE1 (11100001b)
			*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, Instruction->X86InReg, Instruction->X86InReg) << 24) | 0x700F66;
			*(u8 *)(&((u8 *)OutInstruction)[*OutSize + 4]) = 0xE1;
			*OutSize += 5;

			//if memory, write it back out
			if((Instruction->Flags & RecInstrFlagX86_OUTMASK) == RecInstrFlagX86OutMem)
			{
				//push it back out to memory
				//movq [Mem], reg
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x00, Instruction->X86InReg, 0x05) << 24) | 0xD60F66;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 4]) = Instruction->X86OutMemory;
				*OutSize += 8;
			}
			else
			{
				//the register in use is a temp register, adjust it for the memory information
				//and reg, RAM_MASK
				*(u16 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x03, 4, Instruction->X86OutReg) << 8) | 0x81;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 2]) = RAM_MASK;
				*OutSize += 6;

				//movq [reg + Mem_RAM], reg
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize]) = (ModRM(0x02, Instruction->X86InReg, Instruction->X86OutReg) << 24) | 0xD60F66;
				*(u32 *)(&((u8 *)OutInstruction)[*OutSize + 4]) = (u32)Mem_RAM;
				*OutSize += 8;
			}

			break;

		case (RecInstrFlagX86InMem | RecInstrFlagX86OutMem):

			//have a known place we are writing to, check if we need to do
			//a direct write or write thru a register
			*OutSize = 0;

			//find a register we can use
			OutReg = FindReplacableX86Reg(Instruction, X86Regs, -2, &FindRegMem, &FindCount, 0);

			if((X86Regs[OutReg].PPCRegister != -1) && X86Regs[OutReg].ValueChanged)
			{
				//register returned is in use, move it out to memory
				TempInst.InReg = 0;
				TempInst.OutReg = 0;
				TempInst.X86InReg = OutReg;
				TempInst.X86OutMemory = (u32)&ireg.gpr[X86Regs[OutReg].PPCRegister];
				TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
				GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, FPURegs, 0);
				*OutSize += NewOutSize;
			}

			//read a value then push it back out
			//remember we have to flip the 32bit writes around
			//as main memory is 32bit flipped
			TempInst.InReg = 0;
			TempInst.OutReg = 0;
			TempInst.X86InMemory = Instruction->X86InMemory;
			TempInst.X86OutReg = OutReg;
			TempInst.Flags = RecInstrFlagX86InMem | RecInstrFlagX86OutReg | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, FPURegs, 0);
			*OutSize += NewOutSize;

			TempInst.InReg = 0;
			TempInst.OutReg = 0;
			TempInst.X86InReg = OutReg;
			TempInst.X86OutMemory = Instruction->X86OutMemory + 4;
			TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, FPURegs, 0);
			*OutSize += NewOutSize;

			//do the second dword
			TempInst.InReg = 0;
			TempInst.OutReg = 0;
			TempInst.X86InMemory = Instruction->X86InMemory + 4;
			TempInst.X86OutReg = OutReg;
			TempInst.Flags = RecInstrFlagX86InMem | RecInstrFlagX86OutReg | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, FPURegs, 0);
			*OutSize += NewOutSize;

			TempInst.InReg = 0;
			TempInst.OutReg = 0;
			TempInst.X86InReg = OutReg;
			TempInst.X86OutMemory = Instruction->X86OutMemory;
			TempInst.Flags = RecInstrFlagX86InReg | RecInstrFlagX86OutMem | RecInstrFlagPPCInReg | RecInstrFlagPPCOutReg;
			GekkoRecIL(MOVE)(&TempInst, &((u8 *)OutInstruction)[0], &NewOutSize, FPURegs, 0);
			*OutSize += NewOutSize;

			X86Regs[OutReg].PPCRegister = -1;

			break;

		default:
		Unknown_Mask("WRITEMEM64", (Instruction->Flags & RecInstrFlagX86_MASK));
			break;
	}
}
