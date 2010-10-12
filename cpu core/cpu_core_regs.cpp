#include "../emu.h"
#include "cpu_core_regs.h"

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

void set_ireg_spr(u32 Register, u32 NewVal)
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

void ValidateRegLayout()
{
	Gekko_Registers	RegBackup;
	u32				x;
	u32				y;

	memcpy(&RegBackup, &ireg, sizeof(ireg));

	for(x=0; x < 32; x++)
	{
		ireg.gpr[x] = x;
		ireg.fpr[x].ps0._f32[0] = (f64)1 / (f64)(x * x);
		ireg.fpr[x].ps0._f32[1] = (f64)10 / (f64)(x * x);
		ireg.fpr[x].ps1._f32[0] = (f64)100 / (f64)(x * x);
		ireg.fpr[x].ps1._f32[1] = (f64)1000 / (f64)(x * x);
	}

	for(x=0; x < 1024; x++)
		ireg.spr[x] = x << 16;

	for(x=0; x < 16; x++)
		ireg.sr[x] = x << 8;

	ireg.TBR.TBL = 0x90909090;
	ireg.TBR.TBU = 0x90909090;

	ireg.CR = 0xD0D0D0D0;
	ireg.MSR = 0xF0F0F0F0;
	ireg.FPSCR = 0xE0E0E0E0;
	ireg.IC = 0xB0B0B0B0;
	ireg.PC = 0xA0A0A0A0;
	
	#include "cpu_core_userid.h"

	for(x=0; x < sizeof(ireg) / 4; x++)
	{
		for(y = 0; y < 8; y++)
			((u32 *)&Mem_L2)[x+y] = ireg.all[x+y];
	}

	x = GenerateCRC((u8 *)&ireg, sizeof(ireg));

	memcpy(&ireg, &RegBackup, sizeof(ireg));
	ireg.gpr[31] = x;

	//printf("!!!Remove Bypass code in ValidateRegLayout function!!!\n");
	//ireg.gpr[31] = 0xAB289822;
}