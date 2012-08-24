#include "common.h"
#include "cpu_core_regs.h"
#include "crc.h"

/*
u32	ireg_PC()
{
	return ireg.PC;
}

void set_ireg_PC(u32 NewPC)
{
	ireg.PC = NewPC;
}

u32 ireg_gpr(u32 Register)
{
	return ireg.gpr[Register];
}

void set_ireg_gpr(u32 Register, u32 NewVal)
{
	ireg.gpr[Register] = NewVal;
}

u32 ireg_spr(u32 Register)
{
	return ireg.spr[Register];
}

void ireg.spr[u32 Register, u32 NewVal)
{
	ireg.spr[Register] = NewVal;
}

void set_ireg_MSR(u32 NewVal)
{
	ireg.MSR = NewVal;
}

u32 ireg_MSR()
{
	return ireg.MSR;
}

void set_ireg_CR(u32 NewVal)
{
	ireg.CR = NewVal;
}

u32 ireg_CR()
{
	return ireg.CR;
}

void set_ireg_fpr_64(u32 Register, u32 ps, f64 NewVal)
{
	switch(ps)
	{
		case 0:
			ireg.fpr[Register].ps0._f64 = NewVal;
			break;
		case 1:
			ireg.fpr[Register].ps1._f64 = NewVal;
			break;
	};
}

f64 ireg_fpr_64(u32 Register, u32 ps)
{
	if(ps)
		return ireg.fpr[Register].ps1._f64;
	else
		return ireg.fpr[Register].ps0._f64;
}

void set_ireg_fpr_32(u32 Register, u32 ps, f32 NewVal)
{
	if(ps)
		ireg.fpr[Register].ps1._f32[1] = NewVal;
	else
		ireg.fpr[Register].ps0._f32[1] = NewVal;
}

f32 ireg_fpr_32(u32 Register, u32 ps)
{
	if(ps)
		return ireg.fpr[Register].ps1._f32[1];
	else
		return ireg.fpr[Register].ps0._f32[1];
}

u32 ireg_FPSCR()
{
	return ireg.FPSCR;
}

void set_ireg_FPSCR(u32 NewVal)
{
	ireg.FPSCR = NewVal;
}

u32 ireg_IC()
{
	return ireg.IC;
}

void set_ireg_IC(u32 NewVal)
{
	ireg.IC = NewVal;
}

u32 ireg_sr(u32 Register)
{
	return ireg.sr[Register];
}

void set_ireg_sr(u32 Register, u32 NewVal)
{
	ireg.sr[Register] = NewVal;
}
*/