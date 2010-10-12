// cpu_int_opcodes.cpp

////////////////////////////////////////////////////////////

#include "cpu_int.h"
#include "../../low level/hardware core/hw_cp.h"
#include "../../dvd interface/realdvd.h"
#include "../cpu_core_regs.h"

////////////////////////////////////////////////////////////

static u32 ppc_cmp_and_mask[8] = {
	0xfffffff0,
	0xffffff0f,
	0xfffff0ff,
	0xffff0fff,
	0xfff0ffff,
	0xff0fffff,
	0xf0ffffff,
	0x0fffffff,
};

static void Gekko_CalculateCr0( u32 X )
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov edx, ireg.CR
		and edx, 0x0FFFFFFF
		cmp X, 0
		jl CR0_SetLessThan
		jg CR0_SetGreaterThan

		or edx, BIT_2
		jmp CR0_SetXER

	CR0_SetLessThan:
		or edx, BIT_0
		jmp CR0_SetXER

	CR0_SetGreaterThan:
		or edx, BIT_1

	CR0_SetXER:
		mov eax, ireg.spr + (I_XER * 4)
		and eax, 0x80000000
		jz CR0_Done

		or edx, BIT_3

	CR0_Done:
		mov ireg.CR, edx
	};
#else
	ireg.CR = ( ireg.CR & 0x0FFFFFFF );
	
	if( (s32) X < 0 ) ireg.CR |= BIT_0;
	else if( (s32) X > 0 ) ireg.CR |= BIT_1;
	else ireg.CR |= BIT_2;

	if( XER_SO ) ireg.CR |= BIT_3;
#endif
}

static inline void Gekko_CalculateCompare_Uint( u32 x, u32 y, u32 B )
{
	u8 shift = 4 * ( B & 7 );
	ireg.CR = ( ireg.CR & ~( 0xF0000000 >> shift ) );
	
	if( x < y ) ireg.CR |= ( BIT_0 >> shift );
	else if( x > y ) ireg.CR |= ( BIT_1 >> shift );
	else if( x == y ) ireg.CR |= ( BIT_2 >> shift );

	if( XER_SO ) ireg.CR |= ( BIT_3 >> shift );
}

static inline void Gekko_CalculateCompare_Sint( s32 x, s32 y, u32 B )
{
	u8 shift = 4 * ( B & 7 );
	ireg.CR = ( ireg.CR & ~( 0xF0000000 >> shift ) );
	
	if( x < y ) ireg.CR |= ( BIT_0 >> shift );
	else if( x > y ) ireg.CR |= ( BIT_1 >> shift );
	else if( x == y ) ireg.CR |= ( BIT_2 >> shift );

	if( XER_SO ) ireg.CR |= ( BIT_3 >> shift );
}

static inline void Gekko_CalculateCompareFloat( f64 x, f64 y, u32 B )
{
	u8 shift = 4 * ( B & 7 );
	ireg.CR = ( ireg.CR & ~( 0xF0000000 >> shift ) );
	
	if( x < y ) ireg.CR |= ( BIT_0 >> shift );
	else if( (f64) x > y ) ireg.CR |= ( BIT_1 >> shift );
	else if( (f64) x == y ) ireg.CR |= ( BIT_2 >> shift );

	if( XER_SO ) ireg.CR |= ( BIT_3 >> shift );
}

static inline void Gekko_CalculateCr1( void )
{
	ireg.CR = ( ireg.CR & 0xF0FFFFFF ) | FPSCR_FX | FPSCR_FEX | FPSCR_VX | FPSCR_OX;
}

static inline void Gekko_CalculateXerSoOv( u64 X )
{
	if( X >= 0x100000000 ) 
	{
		XER |= BIT_1;		// Set Overflow Bit
		XER |= BIT_0;		// Set Summary Bit
	}else
	{
		XER &= ~BIT_1;	// Reset Overflow Bit
	}
}

static inline void Gekko_Carry(u32 _a, u32 _b, u32 _c)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov eax, _a
		add eax, _b
		jc Carry_AddXer
		add eax, _c
		jc Carry_AddXer
		and ireg.spr + (I_XER * 4), 0xDFFFFFFF
		jmp Carry_Done

Carry_AddXer:
		or ireg.spr + (I_XER * 4), 0x20000000

Carry_Done:
	};
#else
	u64	t;
	t = (u64)_a + (u64)_b + (u64)_c;
	if (t > 0xFFFFFFFF) {
		XER |= BIT_2;
		return;
	}

	XER &= ~BIT_2;
#endif
}

#define GEKKO_CARRY			(XER_CA >> 29)		

#define GEKKO_OPC_ERR(str)			\
{									\
	MessageBox( wnd.hWnd,			\
		str,						\
		"Error",					\
		MB_ICONEXCLAMATION | MB_OK	\
	);								\
}

////////////////////////////////////////////////////////////
// Desc: Misc Opcodes
//

GekkoIntOp(DUMP_OPS)
{
	//dump the last 256k of ops to a debug file
	char opcodeStr[32], operandStr[32];
	char dumpfilename[1024];

	u32 target;
	u32 CurOp;
	u32 Rs;
	u32 CurOpBegin, CurOpEnd;
	u32 i;
	HANDLE FileHandle;
	DWORD	WriteBytes;
	char OutData[256];
	pause = true;

	sprintf(dumpfilename, "%slastops.txt", ProgramDirectory);

	printf("WARNING: CPU Interpreter: Tripped Opcode 0\n");
	printf("CPU Paused. Last %dk ops being dumped to %s\n", CPU_OPSTORE_COUNT / 1000, dumpfilename);

	FileHandle = CreateFile(dumpfilename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		printf("Error writing to %s\n", dumpfilename);
		printf("CPU Paused\n");
		return;
	}

	WriteFile(FileHandle, CurrentGameName, strlen(CurrentGameName), &WriteBytes, 0);
	WriteFile(FileHandle, "\r\n", 2, &WriteBytes, 0);

	for(Rs = 0; Rs < (CPU_OPSTORE_COUNT * 64); Rs++)
	{
//		GekkoCPUInterpreter::LastOldStack[Rs] = BSWAP32(GekkoCPUInterpreter::LastOldStack[Rs]);
		GekkoCPUInterpreter::LastNewStack[Rs] = BSWAP32(GekkoCPUInterpreter::LastNewStack[Rs]);
	}

	CurOpBegin = LastOpEntry + 1;
	CurOpEnd = CPU_OPSTORE_COUNT - 1;
	for(i = 0; i < 2; i++)
	{
		//dump out the ops
		for(CurOp = CurOpBegin; CurOp <= CurOpEnd; CurOp++)
		{
			DisassembleGekko(opcodeStr, operandStr, LastOps[CurOp], LastOpsPC[CurOp], &target);
			sprintf(OutData, "0x%08X %08X %s %s  LR: %08X  SRR0: %08X\r\n", LastOpsPC[CurOp], LastOps[CurOp], opcodeStr, operandStr, LastNewLR[CurOp], LastNewSRR0[CurOp]);
			WriteFile(FileHandle, OutData, strlen(OutData), &WriteBytes, 0);
			for(Rs = 0; Rs < 32; Rs+=4)
			{
				sprintf(OutData, "  R%02d: %08X  R%02d: %08X  R%02d: %08X  R%02d: %08X\r\n", \
					Rs, LastNewR[(CurOp * 32) + Rs], \
					Rs+1, LastNewR[(CurOp * 32) + Rs + 1], \
					Rs+2, LastNewR[(CurOp * 32) + Rs + 2], \
					Rs+3, LastNewR[(CurOp * 32) + Rs + 3]);
				WriteFile(FileHandle, OutData, strlen(OutData), &WriteBytes, 0);
			}
			WriteFile(FileHandle, "\r\n", 2, &WriteBytes, 0);
			for(Rs = 0; Rs < 64; Rs += 8)
			{
				sprintf(OutData, "  %08X %08X %08X %08X %08X %08X %08X %08X\r\n", \
					LastNewStack[(CurOp * 64) + Rs], \
					LastNewStack[(CurOp * 64) + Rs + 1], \
					LastNewStack[(CurOp * 64) + Rs + 2], \
					LastNewStack[(CurOp * 64) + Rs + 3], \
					LastNewStack[(CurOp * 64) + Rs + 4], \
					LastNewStack[(CurOp * 64) + Rs + 5], \
					LastNewStack[(CurOp * 64) + Rs + 6], \
					LastNewStack[(CurOp * 64) + Rs + 7]);
				WriteFile(FileHandle, OutData, strlen(OutData), &WriteBytes, 0);
			}
			WriteFile(FileHandle, "\r\n", 2, &WriteBytes, 0);
		}

		CurOpBegin = 0;
		CurOpEnd = LastOpEntry;
	}
	CloseHandle(FileHandle);
	printf("Finished opcode 0 dump to %s\n", dumpfilename);
}

GekkoIntOp(HLE)
{
	typedef void(*EXECUTEFUNCTIONHLE)(void);
	EXECUTEFUNCTIONHLE ExecuteFunctionHLE;
	ExecuteFunctionHLE = (EXECUTEFUNCTIONHLE)Memory_Read32(ireg.PC+8);
	ExecuteFunctionHLE();
}

GekkoIntOp(NI)
{
	char opcodeStr[32], operandStr[32];
	u32 target;

	DisassembleGekko(opcodeStr, operandStr, cpu->opcode, ireg.PC, &target);
	printf("WARNING: CPU Interpreter: Unimplemented Instruction \n\nOpcode %d (ext: %d) (pc=%08X, op=%08X) instr=%s \n",cpu->opcode>>26,((cpu->opcode>>1)&0x3FF),ireg.PC,cpu->opcode,opcodeStr);
	
	if(PauseOnUnknownOp)
		pause = true;

	if(dbg.active) dbgSetCursor(cpu->PClast);
	branch = OPCODE_BRANCH;
}

////////////////////////////////////////////////////////////
// Desc: Branch Opcodes
//

GekkoIntOp(B)
{
	ireg.PC += EXTS(LI, 26);
}

GekkoIntOp(BL)
{
	LR = ireg.PC + 4;
	ireg.PC += EXTS(LI, 26);
}

GekkoIntOp(BA)
{
	ireg.PC = EXTS(LI, 26);
}

GekkoIntOp(BLA)
{
	LR = ireg.PC + 4;
	ireg.PC = EXTS(LI, 26);
}

GekkoIntOp(BX)
{
	cpu->PClast = ireg.PC;
	optable Gekko_Opset_BX[4] = {GekkoInt(B), GekkoInt(BL), GekkoInt(BA), GekkoInt(BLA) };
	Gekko_Opset_BX[ AALK ]();
//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = OPCODE_BRANCH;
}

GekkoIntOp(BCX)
{
	cpu->PClast = ireg.PC;
	u32 B = (ireg.CR >> (31 - BI)) & 1;

	if(!(BO & 0x04))
		CTR--;

	if((((BO >> 2) | ((CTR != 0) ^ (BO >> 1))) & 1) &
	   (((BO >> 4) | (B == ((BO >> 3) & 1)))) & 1)
	{
		if(LK) LR = ireg.PC + 4;

		if(AA)
			ireg.PC = EXTS16(BD);
		else
			ireg.PC += EXTS16(BD);
	}
	else
		ireg.PC += 4;

//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = OPCODE_BRANCH;
}

GekkoIntOp(BCCTRX)
{
	cpu->PClast = ireg.PC;
	u32 B = (ireg.CR >> (31 - BI)) & 1;

	if((((BO >> 4) | (B == ((BO >> 3) & 1)))) & 1)
	{
		if(LK) LR = ireg.PC + 4;
		ireg.PC = ( CTR & ~3 );
	}
	else
		ireg.PC += 4;

//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = OPCODE_BRANCH;
}

GekkoIntOp(BCLRX)
{
	cpu->PClast = ireg.PC;
	u32 lr;
	u32 B = (ireg.CR >> (31 - BI)) & 1;

	if(!(BO & 0x04))
		CTR--;

	if((((BO >> 2) | ((CTR != 0) ^ (BO >> 1))) & 1) &
	   (((BO >> 4) | (B == ((BO >> 3) & 1)))) & 1)
	{
		if(LK) lr = ireg.PC + 4;
		ireg.PC = ( (LR & ~3) );
		if(LK) LR = lr;
	}
	else
		ireg.PC += 4;

//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = OPCODE_BRANCH;
}

// Desc: Fixed Point Opcodes
//

GekkoIntOp(ADD)
{
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDO)
{
	Gekko_CalculateXerSoOv((u64)RRA + RRB);
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDC)
{
	Gekko_Carry(RRA, RRB, 0);
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDCO)
{
	Gekko_Carry(RRA, RRB, 0);
	Gekko_CalculateXerSoOv((u64)RRA + RRB);
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, RRB, ca);
	RRD = RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, RRB, ca);
	Gekko_CalculateXerSoOv((u64)RRA + RRB + ca);
	RRD = RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDI)
{
	if(rA)	RRD = RRA + SIMM;
	else	RRD = SIMM;
}

GekkoIntOp(ADDIC)
{
	Gekko_Carry(RRA, SIMM, 0);
	RRD = RRA + SIMM;
}

GekkoIntOp(ADDICD)
{
	Gekko_Carry(RRA, SIMM, 0);
	RRD = RRA + SIMM;
	Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDIS)
{
	if(rA)	RRD = RRA + ((u32)SIMM << 16);
	else	RRD = ((u32)SIMM << 16);
}

GekkoIntOp(ADDME)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, ca, -1);
	RRD = RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(ADDMEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, ca, -1);
	Gekko_CalculateXerSoOv((u64)RRA + ca + 0xffffffff);
	RRD = RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(ADDZE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, ca, 0);
	RRD = RRA + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(ADDZEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, ca, 0);
	Gekko_CalculateXerSoOv((u64)RRA + ca);
	RRD = RRA + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(AND)
{
	RRA = RRS & RRB;
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoIntOp(ANDC)
{
	RRA = RRS & ~RRB;
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoIntOp(ANDID)
{
	RRA = RRS & UIMM;
	Gekko_CalculateCr0(RRA);
}

GekkoIntOp(ANDISD)
{
	u16 uimm = UIMM;
	RRA = RRS & ((u32)UIMM << 16);
	Gekko_CalculateCr0( RRA );
}

GekkoIntOp(CMP)
{
	int crfD = CRFD;
	Gekko_CalculateCompare_Sint((s32)RRA, (s32)RRB, crfD);
}

GekkoIntOp(CMPI)
{
	s16 simm = SIMM;
	int crfD = CRFD;
	Gekko_CalculateCompare_Sint((s32)RRA, (s32)simm, crfD );
}

GekkoIntOp(CMPL)
{
	int crfD = CRFD;
	Gekko_CalculateCompare_Uint(RRA, RRB, crfD);
}

GekkoIntOp(CMPLI)
{
	u16 uimm = UIMM;
	int crfD = CRFD;
	Gekko_CalculateCompare_Uint(RRA, uimm, crfD);
}

GekkoIntOp(CNTLZW)
{
	u32 rs;
	u32 mask = 1 << 31;
	for(rs = 0; rs < 32; rs++, mask >>= 1)
	{
		if(RRS & mask) break;
	}

	RRA = rs;
	if(RC)
		Gekko_CalculateCr0(RRA);	//range is 0 to 32 so LT can never be set
}

GekkoIntOp(CRAND)
{
	if (((ireg.CR >> (31-CRBA)) & (ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(CRANDC)
{
	if (((ireg.CR >> (31-CRBA)) & ~(ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(CREQV)
{
	if (~((ireg.CR >> (31-CRBA))) ^ ((ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(CRNAND)
{
	if (~((ireg.CR >> (31-CRBA)) & (ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(CRNOR)
{
	if (~((ireg.CR >> (31-CRBA)) | (ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(CROR)
{
	if (((ireg.CR >> (31-CRBA)) | (ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(CRORC)
{
	if (((ireg.CR >> (31-CRBA)) | ~(ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(CRXOR)
{
	if (((ireg.CR >> (31-CRBA)) ^ (ireg.CR >> (31-CRBB))) & 1)
		ireg.CR |= (1 << (31-CRBD));
	else
		ireg.CR &= ~(1 << (31-CRBD));
}

GekkoIntOp(DCBF)
{
	//Data Cache Block Flush
}

GekkoIntOp(DCBI)
{
	if(ireg.MSR & MSR_BIT_POW)
	{
		cpu->Exception(GEX_PROG);
	}
//	branch = OPCODE_BRANCH;
}

GekkoIntOp(DCBST)
{
	//Data Cache Block Store
}

GekkoIntOp(DCBT)
{
	//Data Cache Block Touch
}

GekkoIntOp(DCBTST)
{
	//Data Cache Block Touch for Store
}

GekkoIntOp(DCBZ)
{
	//Data Cache Block to Zero
}

GekkoIntOp(DCBZ_L)
{
	//Data Cache Block to Zero Locked
}

GekkoIntOp(DIVW)
{
	s32 rra = (s32)RRA, rrb = (s32)((RRB)?(RRB):1);
	RRD = (u32)(s32)(rra / rrb);
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(DIVWO)
{
	s32 rra = (s32)RRA, rrb = (s32)((RRB)?(RRB):1);
	Gekko_CalculateXerSoOv((u64)((s64)rra / rrb));
	RRD = (u32)(s32)(rra / rrb);
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(DIVWU)
{
	u32 rra = RRA, rrb = ((RRB)?(RRB):1);
	RRD = (u32)(rra / rrb);
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(DIVWUO)
{
	u32 rra = RRA, rrb = ((RRB)?(RRB):1);
	Gekko_CalculateXerSoOv(((u64)rra / rrb));
	RRD = (u32)(rra / rrb);
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(ECIWX)
{
	GEKKO_OPC_ERR("ECIWX unimplemented!");
}

GekkoIntOp(ECOWX)
{
	GEKKO_OPC_ERR("ECOWX unimplemented!");
}

GekkoIntOp(EIEIO){}

GekkoIntOp(EQV)
{
	RRA = ~(RRS ^ RRB);
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoIntOp(EXTSB)
{
	RRA = EXTS8(RRS);
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoIntOp(EXTSH)
{
	RRA = EXTS16(RRS);
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoIntOp(ICBI)
{
	//Instruction Cache Block Invalidate
}

GekkoIntOp(ISYNC)
{
	//Instruction Syncronize
}

GekkoIntOp(MCRF)
{
	u32 crfD = CRFD, crfS = CRFS;
	crfD = 7-crfD;
	crfS = 7-crfS;
	u32 c = (ireg.CR >> (crfS * 4)) & 0xf;
	ireg.CR &= ppc_cmp_and_mask[crfD];
	ireg.CR |= c << (crfD * 4);
}

GekkoIntOp(MCRFS)
{
	GEKKO_OPC_ERR("Interpreter Error: MCRFS Unimplemented.");
}

GekkoIntOp(MCRXR)
{
	ireg.CR = (ireg.CR & ~(0xF0000000 >> (CRFD * 4))) | ((XER & 0xF0000000) >> (CRFD * 4));
	XER &= 0x0FFFFFFF;
}

GekkoIntOp(MFCR)
{
	RRD = ireg.CR;
}

GekkoIntOp(MFMSR)
{
	RRD = ireg.MSR;
}

GekkoIntOp(MFSPR)
{
	u32 reg = ((rB << 5) | rA);

	switch(reg)
	{
		case I_TBL:
			RRD = ireg.TBR.TBL;
			break;

		case I_TBU:
			RRD = ireg.TBR.TBU;
			break;

		default:
			RRD = ireg.spr[reg];
			break;

	}
}

GekkoIntOp(MFSR)
{
	RRD = ireg.sr[rA & 0xf];
}

GekkoIntOp(MFSRIN)
{
	RRD = ireg.sr[RRB >> 28];
}

GekkoIntOp(MFTB)
{
	if(tbr == 268) RRD = ireg.TBR.TBL;
	else if(tbr == 269) RRD = ireg.TBR.TBU;
	else cpu->Exception(GekkoCPU::GEX_PROG);
}

GekkoIntOp(MTCRF)
{
	u32 crm = 0;

	crm = ((CRM & 0x80) ? 0xf0000000 : 0) | ((CRM & 0x40) ? 0x0f000000:0) | ((CRM & 0x20) ? 0x00f00000 : 0) | ((crm & 0x10 )? 0x000f0000 : 0) |
	      ((CRM & 0x08) ? 0x0000f000 : 0) | ((CRM & 0x04) ? 0x00000f00:0) | ((CRM & 0x02) ? 0x000000f0 : 0) | ((crm & 0x01) ? 0x0000000f : 0);
	
	ireg.CR = (RRS & crm) | (ireg.CR & ~crm);
}

GekkoIntOp(MTMSR)
{
	ireg.MSR = RRS;
}

GekkoIntOp(MTSPR)
{
	u32 dma_len;
	u32 reg = ((rB << 5) | rA);
	u32 data;
	u32 i;
//	u32 OldVal;

//	OldVal = ireg.spr[reg];

	ireg.spr[reg] = RRS;

	switch(reg)
	{
	case I_TBL:
		ireg.TBR.TBL = RRS;
		break;

	case I_TBU:
		ireg.TBR.TBU = RRS;
		break;

	case I_DMAL:
		if((HID2 & HID2_LCE) && DMA_T)
		{
			dma_len = (DMA_LEN) ? (DMA_LEN) : (0x80);
			dma_len <<= 5;

			if(DMA_LD)
			{
				for(i = 0; i < (dma_len >> 2); i++)
				{
					data = Memory_Read32(DMA_RAM_ADDR + (i * 4));
					Memory_Write32(DMA_L2C_ADDR + (i * 4), data);
				}
			}else{
				for(i = 0; i < (dma_len >> 2); i++)
				{
					data = Memory_Read32(DMA_L2C_ADDR + (i * 4));
					Memory_Write32(DMA_RAM_ADDR + (i * 4), data);
				}
			}
		}
		break;
		
	case I_WPAR:
		CP_Update(RRS);
		break;		
/*
	case I_GQR:
	case (I_GQR + 1):
	case (I_GQR + 2):
	case (I_GQR + 3):
	case (I_GQR + 4):
	case (I_GQR + 5):
	case (I_GQR + 6):
	case (I_GQR + 7):
		if(OldVal != ireg.spr[reg])
		{
			printf("I_GQR[%d] changed %08X to %08X at address 0x%08X\n", reg & 7, OldVal, ireg.spr[reg], ireg.PC);
			cpu->pause = true;
		}
		break;*/
	}
}

GekkoIntOp(MTSR)
{
	ireg.sr[rA & 0xf] = RRS;
}

GekkoIntOp(MTSRIN)
{
	ireg.sr[RRB >> 28] = RRS;
}

GekkoIntOp(MULHW)
{
	s64 temp = (s64)RRA * (s64)RRB;
	RRD = (s32)( temp >> 32 );
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(MULHWU)
{
	u64 temp = (u64)RRA * (u64)RRB;
	RRD = (u32)( temp >> 32 );
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(MULLI)
{
	RRD = (s32)RRA * SIMM;
}

GekkoIntOp(MULLW)
{
	RRD = (s32)RRA * (s32)RRB;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(MULLWO)
{
	Gekko_CalculateXerSoOv((s64)RRA * (s32)RRB );
	RRD = (s32)RRA * (s32)RRB;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(NAND)
{
	RRA = ~( RRS & RRB );
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(NEG)
{
	RRD = ~RRA + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(NEGO)
{
	Gekko_CalculateXerSoOv( (u64)~RRA + 1 );
	RRD = ~RRA + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(NOR)
{
	RRA = ~( RRS | RRB );
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(OR)
{
	RRA = RRS | RRB;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(ORC)
{
	RRA = RRS | ~RRB;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(ORI)
{
	RRA = RRS | UIMM;
}

GekkoIntOp(ORIS)
{
	RRA = RRS | ( UIMM << 16 );
}

GekkoIntOp(RFI)
{
	//leave MSR bits 1-4,10-12,14-15,24,28-29 alone
	//turn msr bit 13 off
	//copy ssr1 bits 0,5-9,16-23,25-27,30-31 to msr
	ireg.MSR = (ireg.MSR & ~0x87C4FF73) | (SRR1 & 0x87C0FF73);
	ireg.PC = (SRR0 & ~3);

//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = OPCODE_BRANCH | OPCODE_RFI;
}

GekkoIntOp(RLWIMI)
{
	u32 m = RotMask[ MB ][ ME ];
	u32 r = Gekko_Rotl( RRS, SH );
	RRA = ( r & m ) | ( RRA & ~m );
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(RLWINM)
{
	u32 m = RotMask[ MB ][ ME ];
	u32 r = Gekko_Rotl( RRS, SH );
	RRA = r & m;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(RLWNM)
{
	u32 m = RotMask[ MB ][ ME ];
	u32 r = Gekko_Rotl( RRS, ( rB & 0x1F ) );
	RRA = r & m;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(SC)
{
	ireg.PC += 4;
	cpu->Exception(GekkoCPU::GEX_SCALL);
}

GekkoIntOp(SLW)
{
    if( RRB & 0x20 ) RRA = 0;
    else RRA = RRS << ( RRB & 0x1F );
    if(RC) Gekko_CalculateCr0( RRA );
}
 
GekkoIntOp(SRAW)
{
	if(RRB == 0)
	{
		RRA = RRS;
		RESET_XER_CA;
	}
    else if(RRB & 0x20)
    {
        if((s32)RRS < 0)
		{
			if(RRS & 0x7FFFFFFF)
				SET_XER_CA;
			else
				RESET_XER_CA;
			RRA = 0xFFFFFFFF;
		}
        else
		{
			RRA = 0;
			RESET_XER_CA;
		}
    }
    else
    {
        if((s32)RRS < 0 && (RRS << (32 - RRB) != 0))
			SET_XER_CA;
		else
			RESET_XER_CA;
		RRA = (s32)RRS >> (RRB & 0x1F);
    }
    if(RC)
		Gekko_CalculateCr0(RRA);
}
 
GekkoIntOp(SRAWI)
{
	u32 sh = SH, mask = BIT_0, rra;

    if( sh == 0 ) { rra = RRS; RESET_XER_CA; }
    else
    {
        rra = (s32)RRS >> sh;
        if((s32)RRS < 0 && (RRS << (32 - sh)) != 0) SET_XER_CA; else RESET_XER_CA;
    }

	RRA = rra;

    if(RC) Gekko_CalculateCr0(RRA);
}
 
GekkoIntOp(SRW)
{
    if( RRB & 0x20 ) RRA = 0;
    else RRA = RRS >> ( RRB & 0x1F );
    if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(SUBF)
{
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFO)
{
	Gekko_CalculateXerSoOv(( (u64)~RRA + RRB + 1 ) );
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFC)
{
	Gekko_Carry(~RRA, RRB, 1);
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFCO)
{
	Gekko_Carry(~RRA, RRB, 1);
	Gekko_CalculateXerSoOv((u64)~RRA + RRB + 1);
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, RRB, ca);
	RRD = ~RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoIntOp(SUBFEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, RRB, ca);
	Gekko_CalculateXerSoOv((u64)~RRA + RRB + ca);
	RRD = ~RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFIC)
{
    Gekko_Carry(~RRA , SIMM, 1);
	RRD = ~RRA + SIMM + 1;
}

GekkoIntOp(SUBFME)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, ca, -1);
	RRD = ~RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFMEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, ca, -1);
	Gekko_CalculateXerSoOv((u64)~RRA + ca + 0xffffffff);
	RRD = ~RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFZE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, ca, 0);
	RRD = ~RRA + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SUBFZEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, ca, 0);
	Gekko_CalculateXerSoOv((u64)~RRA + ca);
	RRD = ~RRA + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoIntOp(SYNC)
{
	//Sync
}

GekkoIntOp(TLBIA)
{
	//Translation Lookaside Buffer Invalidate All
}

GekkoIntOp(TLBIE)
{
	//Translation Lookaside Buffer Invalidate Entry
}

GekkoIntOp(TLBSYNC)
{
	//Translation Lookaside Buffer Sync
}

GekkoIntOp(TW){}

GekkoIntOp(TWI){}

GekkoIntOp(XOR)
{
	RRA = RRS ^ RRB;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoIntOp(XORI)
{
	RRA = RRS ^ UIMM;
}

GekkoIntOp(XORIS)
{
	RRA = RRS ^ ( UIMM << 16 );
}

////////////////////////////////////////////////////////////
// Desc: Load/Store Opcodes
//

GekkoIntOp(LBZ)
{
	RRD = (rA) ? Memory_Read8( RRA + SIMM ) : Memory_Read8( SIMM );
}

GekkoIntOp(LBZU)
{
	u32 X = RRA + SIMM;
	RRD = Memory_Read8( X );
	RRA = X;
}

GekkoIntOp(LBZUX)
{
	u32 X = RRA + RRB;
	RRD = Memory_Read8( X );
	RRA = X;
}

GekkoIntOp(LBZX)
{
	RRD = (rA) ? Memory_Read8( RRA + RRB ) : Memory_Read8( RRB );
}

GekkoIntOp(LFD)
{
	FBRD = (rA) ? Memory_Read64( RRA + SIMM ) : Memory_Read64( SIMM );
}

GekkoIntOp(LFDU)
{
	FBRD = Memory_Read64( ( RRA += SIMM ) );
}

GekkoIntOp(LFDUX)
{
	FBRD = Memory_Read64( ( RRA += RRB ) );
}

GekkoIntOp(LFDX)
{
	FBRD = (rA) ? Memory_Read64( RRA + RRB ) : Memory_Read64( RRB );
}

GekkoIntOp(LFS)
{
	t32 temp;
	temp._u32 = (rA) ? Memory_Read32( RRA + SIMM ) : Memory_Read32( SIMM );

	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoIntOp(LFSU)
{
	t32 temp;
	temp._u32 = Memory_Read32( ( RRA += SIMM ) );

	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoIntOp(LFSUX)
{
	t32 temp;
	temp._u32 = Memory_Read32( RRA += RRB );

	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoIntOp(LFSX)
{
	t32 temp;
	temp._u32 = (rA) ? Memory_Read32( RRA + RRB ) : Memory_Read32( RRB );

	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoIntOp(LHA)
{
	RRD = EXTS16((rA) ? Memory_Read16( RRA + SIMM ) : Memory_Read16( SIMM ));
}

GekkoIntOp(LHAU)
{
	u32 X = RRA + SIMM;
	RRD = EXTS16(Memory_Read16( X ));
	RRA = X;
}

GekkoIntOp(LHAUX)
{
	u32 X = RRA + RRB;
	RRD = EXTS16(Memory_Read16( X ));
	RRA = X;
}

GekkoIntOp(LHAX)
{
	RRD = EXTS16((rA) ? Memory_Read16( RRA + RRB ) : Memory_Read16( RRB ));
}

GekkoIntOp(LHBRX)
{
	RRD = BSWAP16( (rA) ? Memory_Read16( RRA + RRB ) : Memory_Read16( RRB ) );
}

GekkoIntOp(LHZ)
{
	RRD = (rA) ? Memory_Read16( RRA + SIMM ) : Memory_Read16( SIMM );
}

GekkoIntOp(LHZU)
{
	u32 X = RRA + SIMM;
	RRD = Memory_Read16( X );
	RRA = X;
}

GekkoIntOp(LHZUX)
{
	u32 X = RRA + RRB;
	RRD = Memory_Read16( X );
	RRA = X;
}

GekkoIntOp(LHZX)
{
	RRD = (rA) ? Memory_Read16( RRA + RRB ) : Memory_Read16( RRB );
}

GekkoIntOp(LMW)
{
	u32 ea = ( rA ) ? ( RRA + SIMM ) : SIMM;

	for(int i = rD; i < 32; i++, ea += 4 )
		ireg.gpr[i] = Memory_Read32(ea);
}

GekkoIntOp(LSWI)
{
	u32 EA = 0;
	u32 n = 32;
	u32 r;

	if(rA)
		EA = RRA;
	if(NB)
		n = NB;

	r = rD;
	while(n > 4)
	{
		ireg.gpr[r] = Memory_Read32(EA);
		r = (r + 1) % 32;
		EA+=4;
		n-=4;
	};

	switch(n)
	{
		case 3:
			ireg.gpr[r] = Memory_Read32(EA) & 0xFFFFFF00;
			break;

		case 2:
			ireg.gpr[r] = Memory_Read16(EA) << 16;
			break;

		case 1:
			ireg.gpr[r] = Memory_Read8(EA) << 24;
			break;
	}
}

GekkoIntOp(LSWX)
{
	u32 EA = RRB;
	u32 n = XER >> 24;
	u32 r;

	if(rA)
		EA += RRA;

	r = rD;
	while(n > 4)
	{
		ireg.gpr[r] = Memory_Read32(EA);
		r = (r + 1) % 32;
		EA+=4;
		n-=4;
	};

	switch(n)
	{
		case 3:
			ireg.gpr[r] = Memory_Read32(EA) & 0xFFFFFF00;
			break;

		case 2:
			ireg.gpr[r] = Memory_Read16(EA) << 16;
			break;

		case 1:
			ireg.gpr[r] = Memory_Read8(EA) << 24;
			break;
	}
}

GekkoIntOp(LWARX)
{
	u32 EA = RRB;
	if(rA)
		EA += RRA;
	RRD = Memory_Read32(EA);
	GekkoCPUInterpreter::is_reserved = 1;
	GekkoCPUInterpreter::reserved_addr = EA;
}

GekkoIntOp(LWBRX)
{
	RRD = BSWAP32( (rA) ? Memory_Read32( RRA + RRB ) : Memory_Read32( RRB ) );
}

GekkoIntOp(LWZ)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		//edx is set to opcode from the caller when using asm
		mov eax, edx
		movsx ecx, dx
		shr eax, 10h
		shr edx, 15h

		and edx, 1Fh
		lea edx, [edx*4 + ireg.gpr]
		and eax, 1Fh

		jz DoLWZRead
		lea eax, [eax*4 + ireg.gpr]
		add ecx, [eax]

DoLWZRead:
		push edx
		call Memory_Read32

		pop edx
		mov [edx], eax
	};
#else
	RRD = (rA) ? Memory_Read32( RRA + SIMM ) : Memory_Read32( SIMM );
#endif
}

GekkoIntOp(LWZU)
{
	u32 X = RRA + SIMM;
	RRD = Memory_Read32( X );
	RRA = X;
}

GekkoIntOp(LWZUX)
{
	u32 X = RRA + RRB;
	RRD = Memory_Read32( X );
	RRA = X;
}

GekkoIntOp(LWZX)
{
	RRD = (rA) ? Memory_Read32( RRA + RRB ) : Memory_Read32( RRB );
}

////////////////////////////////////////////////////////////

GekkoIntOp(STB)
{
	if(rA) Memory_Write8( RRA + SIMM, RRS );
	else Memory_Write8( SIMM, RRS );
}

GekkoIntOp(STBU)
{
	u32 X = RRA + SIMM;
	Memory_Write8( X, RRS );
	RRA = X;
}

GekkoIntOp(STBUX)
{
	u32 X = RRA + RRB;
	Memory_Write8( X , RRS );
	RRA = X;
}

GekkoIntOp(STBX)
{
	if(rA) Memory_Write8( RRA + RRB, RRS );
	else Memory_Write8( RRB, RRS );
}

GekkoIntOp(STFD)
{
	if(rA) Memory_Write64( RRA + SIMM, FBRS );
	else Memory_Write64( SIMM, FBRS );
}

GekkoIntOp(STFDU)
{
	Memory_Write64( ( RRA += SIMM ) , FBRS );
}

GekkoIntOp(STFDUX)
{
	Memory_Write64( ( RRA += RRB ) , FBRS );
}

GekkoIntOp(STFDX)
{
	if(rA) Memory_Write64( RRA + RRB, FBRS );
	else Memory_Write64( RRB, FBRS );
}

GekkoIntOp(STFIWX)
{
	if(rA) Memory_Write32( RRA + RRB, *(u32 *)&FPRS );
	else Memory_Write32( RRB, *(u32 *)&FPRS );
}

GekkoIntOp(STFS)
{
	t32 data;
	data._f32 = (f32)FPRS;
	if(rA) Memory_Write32( RRA + SIMM, data._u32);
	else Memory_Write32( SIMM, data._u32);
}

GekkoIntOp(STFSU)
{
	t32 data;
	data._f32 = (f32)FPRS;
	Memory_Write32( RRA += SIMM, data._u32 );
}

GekkoIntOp(STFSUX)
{
	t32 data;
	data._f32 = (f32)FPRS;
	Memory_Write32( RRA += RRB, data._u32);
}

GekkoIntOp(STFSX)
{
	t32 data;
	data._f32 = (f32)FPRS;

	if(rA) Memory_Write32( RRA + RRB, data._u32 );
	else Memory_Write32( RRB, data._u32 );
}

GekkoIntOp(STH)
{
	if(rA) Memory_Write16( RRA + SIMM, RRS );
	else Memory_Write16( SIMM, RRS );
}

GekkoIntOp(STHBRX) 
{
	if(rA) Memory_Write16( RRA + RRB, BSWAP16( RRS & 0xFFFF ) );
	else Memory_Write16( RRB, BSWAP16( RRS & 0xFFFF ) );
}

GekkoIntOp(STHU)
{
	u32 X = RRA + SIMM;
	Memory_Write16( X , RRS );
	RRA = X;
}

GekkoIntOp(STHUX)
{
	u32 X = RRA + RRB;
	Memory_Write16( X , RRS );
	RRA = X;
}

GekkoIntOp(STHX)
{
	if(rA) Memory_Write16( RRA + RRB, RRS );
	else Memory_Write16( RRB, RRS );
}

GekkoIntOp(STMW)
{
	u32 ea = ( rA ) ? ( RRA + SIMM ) : SIMM;

	for(int i = rS; i < 32; i++, ea += 4 )
	{
		Memory_Write32( ea, ireg.gpr[i] );
	}
}

GekkoIntOp(STSWI)
{
	u32 EA = 0;
	u32 n = 32;
	u32 r;

	if(rA)
		EA = RRA;
	if(NB)
		n = NB;

	r = rD;
	while(n > 4)
	{
		Memory_Write32(EA, ireg.gpr[r]);
		r = (r + 1) % 32;
		EA+=4;
		n-=4;
	};

	switch(n)
	{
		case 3:
			Memory_Write16(EA, ireg.gpr[r] >> 16);
			Memory_Write8(EA, (ireg.gpr[r] >> 8) & 0xFF);
			break;

		case 2:
			Memory_Write16(EA, ireg.gpr[r] >> 16);
			break;

		case 1:
			Memory_Write8(EA, ireg.gpr[r] >> 24);
			break;
	}
}

GekkoIntOp(STSWX)
{
	u32 EA = RRB;
	u32 n = XER >> 24;
	u32 r;

	if(rA)
		EA += RRA;

	r = rD;
	while(n > 4)
	{
		Memory_Write32(EA, ireg.gpr[r]);
		r = (r + 1) % 32;
		EA+=4;
		n-=4;
	};

	switch(n)
	{
		case 3:
			Memory_Write16(EA, ireg.gpr[r] >> 16);
			Memory_Write8(EA, (ireg.gpr[r] >> 8) & 0xFF);
			break;

		case 2:
			Memory_Write16(EA, ireg.gpr[r] >> 16);
			break;

		case 1:
			Memory_Write8(EA, ireg.gpr[r] >> 24);
			break;
	}
}

GekkoIntOp(STW)
{
#if(0) //#ifdef USE_INLINE_ASM
	//bug in here somewhere, not sure where
	_asm
	{
		//edx is set to opcode from the caller when using asm
		mov eax, edx
		movsx ecx, dx
		shr eax, 10h	//rA
		shr edx, 15h	//rS

		and edx, 1Fh
		lea edx, [edx*4 + ireg.gpr]
		and eax, 1Fh

		jz DoSTWWrite
		lea eax, [eax*4 + ireg.gpr]
		add ecx, [eax]

DoSTWWrite:
		push edx
		call Memory_Write32
	};
#else
	if(rA) Memory_Write32( RRA + SIMM, RRS );
	else   Memory_Write32( SIMM, RRS );
#endif
}

GekkoIntOp(STWBRX) 
{
	if(rA) Memory_Write32( RRA + RRB, BSWAP32( RRS ) );
	else Memory_Write32( RRB, BSWAP32( RRS ) );
}

GekkoIntOp(STWCX)
{
	u32	EA;
	if(GekkoCPUInterpreter::is_reserved)
	{
		EA = RRB;
		if(rA)
			EA += RRA;
		Memory_Write32(EA, RRS);
		GekkoCPUInterpreter::is_reserved = 0;
		ireg.CR |= 4;	
	}
	else
		ireg.CR &= ~4;
}

GekkoIntOp(STWU)
{
	u32 X = RRA + SIMM;
	Memory_Write32( X, RRS );
	RRA = X;
}

GekkoIntOp(STWUX)
{
	u32 X = RRA + RRB;
	Memory_Write32( X, RRS );
	RRA = X;
}

GekkoIntOp(STWX)
{
	if(rA) Memory_Write32( RRA + RRB, RRS );
	else Memory_Write32( RRB, RRS );
}

GekkoIntOp(PSQ_L)
{
    u32 data0, data1;
    int type = GQR_LD_TYPE(PSI);
    int scale = (u8)GQR_LD_SCALE(PSI);

    u32 addr = (rA) ? (RRA + PSIMM) : PSIMM;

	switch(type)
	{
		case 4:
			data0 = Memory_Read8(addr);
			break;

		case 6:
			data0 = (s8)Memory_Read8(addr);
			break;

		case 5:
			data0 = Memory_Read16(addr);
			break;

		case 7:
			data0 = (s16)Memory_Read16(addr);
			break;

		default:
			data0 = Memory_Read32(addr);
	}

    PS0D = (f64)dequantize(data0, type, scale);

	if(PSW)
    {
        PS1D = 1.0f;
    }
    else
    {
		switch(type)
		{
			case 4:
				data1 = Memory_Read8(addr+1);
				break;

			case 6:
				data1 = (s8)Memory_Read8(addr+1);
				break;

			case 5:
				data1 = Memory_Read16(addr+2);
				break;

			case 7:
				data1 = (s16)Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }
}

GekkoIntOp(PSQ_LX)
{
    u32 data0, data1;
    int type = GQR_LD_TYPE(PSI_X);
    int scale = (u8)GQR_LD_SCALE(PSI_X);

    u32 addr = (rA) ? (RRA + RRB) : RRB;

	switch(type)
	{
		case 4:
			data0 = Memory_Read8(addr);
			break;

		case 6:
			data0 = (s8)Memory_Read8(addr);
			break;

		case 5:
			data0 = Memory_Read16(addr);
			break;

		case 7:
			data0 = (s16)Memory_Read16(addr);
			break;

		default:
			data0 = Memory_Read32(addr);
	}

    PS0D = (f64)dequantize(data0, type, scale);

	if(PSW_X)
    {
        PS1D = 1.0f;
    }
    else
    {
		switch(type)
		{
			case 4:
				data1 = Memory_Read8(addr+1);
				break;

			case 6:
				data1 = (s8)Memory_Read8(addr+1);
				break;

			case 5:
				data1 = Memory_Read16(addr+2);
				break;

			case 7:
				data1 = (s16)Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }
}

GekkoIntOp(PSQ_LU)
{
    u32 data0, data1;
    int type = GQR_LD_TYPE(PSI);
    int scale = (u8)GQR_LD_SCALE(PSI);

    u32 addr = (rA) ? (RRA + PSIMM) : PSIMM;

	switch(type)
	{
		case 4:
			data0 = Memory_Read8(addr);
			break;

		case 6:
			data0 = (s8)Memory_Read8(addr);
			break;

		case 5:
			data0 = Memory_Read16(addr);
			break;

		case 7:
			data0 = (s16)Memory_Read16(addr);
			break;

		default:
			data0 = Memory_Read32(addr);
	}

    PS0D = (f64)dequantize(data0, type, scale);

	if(PSW)
    {
        PS1D = 1.0f;
    }
    else
    {
		switch(type)
		{
			case 4:
				data1 = Memory_Read8(addr+1);
				break;

			case 6:
				data1 = (s8)Memory_Read8(addr+1);
				break;

			case 5:
				data1 = Memory_Read16(addr+2);
				break;

			case 7:
				data1 = (s16)Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }

	RRA = addr;
}

GekkoIntOp(PSQ_LUX)
{
    u32 data0, data1;
    int type = GQR_LD_TYPE(PSI_X);
    int scale = (u8)GQR_LD_SCALE(PSI_X);

    u32 addr = (rA) ? (RRA + RRB) : RRB;

	switch(type)
	{
		case 4:
		case 6:
			data0 = Memory_Read8(addr);
			break;

		case 5:
		case 7:
			data0 = Memory_Read16(addr);
			break;

		default:
			data0 = Memory_Read32(addr);
	}

    PS0D = (f64)dequantize(data0, type, scale);

	if(PSW_X)
    {
        PS1D = 1.0f;
    }
    else
    {
		switch(type)
		{
			case 4:
				data1 = Memory_Read8(addr+1);
				break;

			case 6:
				data1 = (s8)Memory_Read8(addr+1);
				break;

			case 5:
				data1 = Memory_Read16(addr+2);
				break;

			case 7:
				data1 = (s16)Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }

	RRA = addr;
}

GekkoIntOp(PSQ_ST)
{
     int type = GQR_ST_TYPE(PSI);
     int scale = GQR_ST_SCALE(PSI);

     u32 addr = (rA) ? (RRA + PSIMM) : PSIMM;

     if(PSW)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
     }
     else
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }
}

GekkoIntOp(PSQ_STX)
{
     int type = GQR_ST_TYPE(PSI_X);
     int scale = GQR_ST_SCALE(PSI_X);

     u32 addr = (rA) ? (RRA + RRB) : RRB;

     if(PSW_X)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
     }
     else
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }
}

GekkoIntOp(PSQ_STU)
{
     int type = GQR_ST_TYPE(PSI);
     int scale = GQR_ST_SCALE(PSI);

	 u32 addr = (RRA + PSIMM);

     if(PSW)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
     }
     else
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }
	 RRA = addr;
}

GekkoIntOp(PSQ_STUX)
{
     int type = GQR_ST_TYPE(PSI_X);
     int scale = GQR_ST_SCALE(PSI_X);

     u32 addr = (rA) ? (RRA + RRB) : RRB;

     if(PSW_X)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
     }
     else
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr, quantize((f32)PS0S, type, scale));
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr, quantize((f32)PS0S, type, scale));
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr, quantize((f32)PS0S, type, scale));
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }

	RRA = addr;
}

////////////////////////////////////////////////////////////
// Desc: Floating Point Opcodes
//

GekkoIntOp(FABS)
{
	FBRD = FBRB & ~SIGNED_BIT64;
}

GekkoIntOp(FADD)
{
	FPRD = FPRA + FPRB;
}

GekkoIntOp(FADDS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A + PS0B);
	}else{
		FPRD = (f32)(FPRA + FPRB);
	}
}

GekkoIntOp(FCMPO)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(FPRA, FPRB, crfD);
}

GekkoIntOp(FCMPU)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(FPRA, FPRB, crfD);
}

GekkoIntOp(FCTIW)
{
	FBRD = (u64)(u32)(s32)FPRB;
}

GekkoIntOp(FCTIWZ)
{
	FBRD = (u64)(u32)(s32)FPRB;
}

GekkoIntOp(FDIV)
{
	FPRD = FPRA / FPRB;
}

GekkoIntOp(FDIVS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A / PS0B);
	}else{
		FPRD = (f32)(FPRA / FPRB);
	}
}

GekkoIntOp(FMADD)
{
	FPRD = ( FPRA * FPRC ) + FPRB;
}

GekkoIntOp(FMADDS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)((PS0A * PS0C) + PS0B);
	}else{
		FPRD = (f32)((FPRA * FPRC) + FPRB);
	}
}

GekkoIntOp(FMR)
{
	FBRD = FBRB;
}

GekkoIntOp(FMSUB)
{
	FPRD = ( FPRA * FPRC ) - FPRB;
}

GekkoIntOp(FMSUBS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)((PS0A * PS0C) - PS0B);
	}else{
		FPRD = (f32)((FPRA * FPRC) - FPRB);
	}
}

GekkoIntOp(FMUL)
{
	FPRD = FPRA * FPRC;
}

GekkoIntOp(FMULS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A * PS0C);
	}else{
		FPRD = (f32)(FPRA * FPRC);
	}
}

GekkoIntOp(FNABS)
{
	FBRD = FBRB | SIGNED_BIT64;
}

GekkoIntOp(FNEG)
{
	FBRD = FBRB ^ SIGNED_BIT64;
}

GekkoIntOp(FNMADD)
{
	FPRD = -((FPRA * FPRC) + FPRB);
}

GekkoIntOp(FNMADDS)
{
	if(PSE)
	{
		PS0D = PS1D = (f32) -((PS0A * PS0C) + PS0B);
	}else{
		FPRD = (f32) -((FPRA * FPRC) + FPRB);
	}
}

GekkoIntOp(FNMSUB)
{
	FPRD = -((FPRA * FPRC) - FPRB);
}

GekkoIntOp(FNMSUBS)
{
	if(PSE)
	{
		PS0D = PS1D = (f32) -((PS0A * PS0C) - PS0B);
	}else{
		FPRD = (f32) -((FPRA * FPRC) - FPRB);
	}
}

GekkoIntOp(FRES)
{
	if(PSE)
	{
		PS0D = PS1D = (f32)(1/PS0B);
	}else{
		FPRD = (f32)(1/FPRB);
	}
}

GekkoIntOp(FRSP)
{
	if(PSE)
		PS0D = (float)FPRB;
	else
		FPRD = (float)FPRB;
}

GekkoIntOp(FRSQRTE)
{
	FPRD = 1.0 / sqrt(FPRB);
}

GekkoIntOp(FSEL)
{
	if(FPRA >= 0)
	{
		FPRD = FPRC;
	}else{
		FPRD = FPRB;
	}
}

GekkoIntOp(FSQRT)
{
	FPRD = sqrt(FPRB);	
}

GekkoIntOp(FSQRTS)
{
	FPRD = (f32)sqrt(FPRB);
}

GekkoIntOp(FSUB)
{
	FPRD = FPRA - FPRB;
}

GekkoIntOp(FSUBS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A - PS0B);
	}else{
		FPRD = (f32)(FPRA - FPRB);
	}
}

GekkoIntOp(MFFS)
{
	FBRD = ireg.FPSCR;
}

GekkoIntOp(MTFSB0)
{
	int crbD = CRBD;
	ireg.FPSCR &= ~( BIT_0 >> crbD );
}

GekkoIntOp(MTFSB1)
{
	int crbD = CRBD;
	ireg.FPSCR |= ( BIT_0 >> crbD );
}

GekkoIntOp(MTFSF)
{
	for(int i = 0; i < 7; i++ )
	{
		if( FM & ( 0x80 >> i ) )
		{
			ireg.FPSCR = ireg.FPSCR & ~( 0xF0000000 >> ( i * 4 ) );
			u8 temp = ( RRB & ( 0xF0000000 >> ( i * 4 ) ) ) >> ( 31 - ( 4 * ( 7 - i ) ) );
			ireg.FPSCR = ireg.FPSCR | ( temp << ( 31 - ( 4 * ( 7 - i ) ) ) );
		}
	}
}

GekkoIntOp(MTFSFI)
{
	int crfD = CRFD;
	ireg.FPSCR = ireg.FPSCR & ~( 0xF0000000 >> ( crfD * 4 ) );
	ireg.FPSCR = ireg.FPSCR | ( IMM << ( 31 - ( 4 * ( 7 - crfD ) ) ) );
}

////////////////////////////////////////////////////////////
// Desc: Paired Singles Opcodes
//

GekkoIntOp(PS_ABS)
{
	PS0D = abs(PS0B);
	PS1D = abs(PS1B);
}

GekkoIntOp(PS_ADD)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov eax, [GekkoCPU::opcode]
		shr eax, 11
		mov ebx, eax

		shr eax, 5
		and ebx, 0x1F

		mov ecx, eax
		shl ebx, 4
		shr eax, 5

		and ecx, 0x1F
		and eax, 0x1F
		add ebx, offset ireg.fpr		//frB

		shl ecx, 4
		shl eax, 4
		add ecx, offset ireg.fpr		//frA
		add eax, offset ireg.fpr		//frD

		movupd xmm0, [ecx]
		movupd xmm1, [ebx]
		addpd xmm0, xmm1
		movupd [eax], xmm0
	};
#else
	PS0D = PS0A + PS0B;
	PS1D = PS1A + PS1B;
#endif
}

GekkoIntOp(PS_CMPO0)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS0A, PS0B, crfD );
}

GekkoIntOp(PS_CMPO1)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS1A, PS1B, crfD );
}


GekkoIntOp(PS_CMPU0)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS0A, PS0B, crfD);
}

GekkoIntOp(PS_CMPU1)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS1A, PS1B, crfD);
}

GekkoIntOp(PS_MADD)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 6
		mov ecx, edx

		shr edx, 5
		mov ebx, edx
		and ecx, 0x1F

		shr edx, 5
		shl ecx, 4
		and ebx, 0x1F
		mov eax, edx

		shr edx, 5
		add ecx, offset ireg.fpr		//frC
		shl ebx, 4
		and eax, 0x1F

		add ebx, offset ireg.fpr
		and edx, 0x1F
		shl eax, 4

		shl edx, 4
		add eax, offset ireg.fpr
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ecx]
		movupd xmm2, [ebx]

		mulpd xmm0, xmm1
		addpd xmm0, xmm2

		movupd [edx], xmm0
	};
#else
	PS0D = ( PS0A * PS0C ) + PS0B;
	PS1D = ( PS1A * PS1C ) + PS1B;
#endif
}

GekkoIntOp(PS_MADDS0)
{
	//avoid rC == rD
	f64 ps0c = PS0C;
	PS0D = (f32)(( PS0A * ps0c ) + PS0B);
	PS1D = (f32)(( PS1A * ps0c ) + PS1B);
}

GekkoIntOp(PS_MADDS1)
{
	PS0D = (f32)(( PS0A * PS1C ) + PS0B);
	PS1D = (f32)(( PS1A * PS1C ) + PS1B);
}

GekkoIntOp(PS_MERGE00)
{
	f64 ps0a = PS0A, ps0b = PS0B;
	PS0D = ps0a;
	PS1D = ps0b;
}

GekkoIntOp(PS_MERGE01)
{
	PS0D = PS0A;
	PS1D = PS1B;
}

GekkoIntOp(PS_MERGE10)
{
	f64 ps0b = PS0B;
	PS0D = PS1A;
	PS1D = ps0b;
}

GekkoIntOp(PS_MERGE11)
{
	PS0D = PS1A;
	PS1D = PS1B;
}

GekkoIntOp(PS_MR)
{
	PS0D = PS0B;
	PS1D = PS1B;
}

GekkoIntOp(PS_MSUB)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 6
		mov ecx, edx

		shr edx, 5
		mov ebx, edx
		and ecx, 0x1F

		shr edx, 5
		shl ecx, 4
		and ebx, 0x1F
		mov eax, edx

		shr edx, 5
		add ecx, offset ireg.fpr		//frC
		shl ebx, 4
		and eax, 0x1F

		add ebx, offset ireg.fpr
		and edx, 0x1F
		shl eax, 4

		shl edx, 4
		add eax, offset ireg.fpr
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ecx]
		movupd xmm2, [ebx]

		mulpd xmm0, xmm1
		subpd xmm0, xmm2

		movupd [edx], xmm0
	};
#else
	PS0D = ( PS0A * PS0C ) - PS0B;
	PS1D = ( PS1A * PS1C ) - PS1B;
#endif
}

GekkoIntOp(PS_MUL)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 6
		mov ecx, edx

		shr edx, 10
		and ecx, 0x1F

		shl ecx, 4
		mov eax, edx

		shr edx, 5
		add ecx, offset ireg.fpr		//frC
		and eax, 0x1F
		shl eax, 4

		and edx, 0x1F
		shl edx, 4
		add eax, offset ireg.fpr
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ecx]

		mulpd xmm0, xmm1
		movupd [edx], xmm0
	};
#else
	PS0D = PS0A * PS0C;
	PS1D = PS1A * PS1C;
#endif
}

GekkoIntOp(PS_MULS0)
{
	//avoid rC == rD
	f64 ps0c = PS0C;
	PS0D = PS0A * ps0c;
	PS1D = PS1A * ps0c;
}

GekkoIntOp(PS_MULS1)
{
	PS0D = PS0A * PS1C;
	PS1D = PS1A * PS1C;
}

GekkoIntOp(PS_NEG)
{
#if(0) //#ifdef USE_INLINE_ASM
	t128 Val0;
	Val0.ps0._f64 = 0;
	Val0.ps1._f64 = 0;
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 11

		mov ebx, edx

		shr edx, 10
		and ebx, 0x1F
		lea eax, Val0

		shl ebx, 4
		add ebx, offset ireg.fpr
		and edx, 0x1F

		shl edx, 4
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ebx]

		subpd xmm0, xmm1

		movupd [edx], xmm0
	};
#else
    PS0D = -PS0B;
    PS1D = -PS1B;
#endif
}

GekkoIntOp(PS_NMADD)
{
#if(0) //#ifdef USE_INLINE_ASM
	t128 Val0;
	Val0.ps0._f64 = 0;
	Val0.ps1._f64 = 0;
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 6
		mov ecx, edx

		shr edx, 5
		mov ebx, edx
		and ecx, 0x1F

		shr edx, 5
		shl ecx, 4
		and ebx, 0x1F
		mov eax, edx

		shr edx, 5
		add ecx, offset ireg.fpr		//frC
		shl ebx, 4
		and eax, 0x1F

		add ebx, offset ireg.fpr
		and edx, 0x1F
		shl eax, 4

		shl edx, 4
		add eax, offset ireg.fpr
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ecx]
		movupd xmm2, [ebx]

		lea eax, Val0
		mulpd xmm0, xmm1
		addpd xmm0, xmm2

		movupd xmm1, [eax]
		subpd xmm1, xmm0

		movupd [edx], xmm1
	};
#else
    PS0D = -((PS0A * PS0C) + PS0B);
    PS1D = -((PS1A * PS1C) + PS1B);
#endif
}

GekkoIntOp(PS_NMSUB)
{
#if(0) //#ifdef USE_INLINE_ASM
	t128 Val0;
	Val0.ps0._f64 = 0;
	Val0.ps1._f64 = 0;
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 6
		mov ecx, edx

		shr edx, 5
		mov ebx, edx
		and ecx, 0x1F

		shr edx, 5
		shl ecx, 4
		and ebx, 0x1F
		mov eax, edx

		shr edx, 5
		add ecx, offset ireg.fpr		//frC
		shl ebx, 4
		and eax, 0x1F

		add ebx, offset ireg.fpr
		and edx, 0x1F
		shl eax, 4

		shl edx, 4
		add eax, offset ireg.fpr
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ecx]
		movupd xmm2, [ebx]

		lea eax, Val0
		mulpd xmm0, xmm1
		subpd xmm0, xmm2

		movupd xmm1, [eax]
		subpd xmm1, xmm0

		movupd [edx], xmm1
	};
#else
    PS0D = -((PS0A * PS0C) - PS0B);
    PS1D = -((PS1A * PS1C) - PS1B);
#endif
}

GekkoIntOp(PS_RES)
{
#if(0) //#ifdef USE_INLINE_ASM
	t128 Val1;
	Val1.ps0._f64 = 1.0;
	Val1.ps1._f64 = 1.0;
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 11

		mov ebx, edx

		shr edx, 10
		and ebx, 0x1F
		lea eax, [Val1]

		shl ebx, 4
		add ebx, offset ireg.fpr
		and edx, 0x1F

		shl edx, 4
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ebx]

		divpd xmm0, xmm1

		movupd [edx], xmm0
	};
#else
	PS0D = (f32)(1.0 / PS0B);
	PS1D = (f32)(1.0 / PS1B);
#endif
}

GekkoIntOp(PS_RSQRTE)
{
#if(0) //#ifdef USE_INLINE_ASM
	t128 Val1;
	Val1.ps0._f64 = 1.0;
	Val1.ps1._f64 = 1.0;
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 11

		mov ebx, edx

		shr edx, 5
		and ebx, 0x1F
		lea eax, [Val1]

		shr edx, 5
		shl ebx, 4

		add ebx, offset ireg.fpr
		and edx, 0x1F

		shl edx, 4
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ebx]

		sqrtpd xmm1, xmm1
		divpd xmm0, xmm1

		movupd [edx], xmm0
	};
#else
	PS0D = (f32)(1.0 / sqrtf(PS0B));
	PS1D = (f32)(1.0 / sqrtf(PS1B));
#endif
}

GekkoIntOp(PS_SUB)
{
#if(0) //#ifdef USE_INLINE_ASM
	_asm
	{
		mov edx, [GekkoCPU::opcode]
		shr edx, 11
		mov ebx, edx

		shr edx, 5
		and ebx, 0x1F
		mov eax, edx

		shr edx, 5
		shl ebx, 4
		and eax, 0x1F

		add ebx, offset ireg.fpr
		and edx, 0x1F
		shl eax, 4

		shl edx, 4
		add eax, offset ireg.fpr
		add edx, offset ireg.fpr

		movupd xmm0, [eax]
		movupd xmm1, [ebx]

		subpd xmm0, xmm1

		movupd [edx], xmm0
	};
#else
	PS0D = PS0A - PS0B;
	PS1D = PS1A - PS1B;
#endif
}

GekkoIntOp(PS_DIV)
{
	PS0D = PS0A / PS0B;
	PS1D = PS1A / PS1B;
}

GekkoIntOp(PS_SUM0)
{
	PS0D = PS0A + PS1B;
	PS1D = PS1C;
}

GekkoIntOp(PS_SUM1)
{
	//avoid rD == rA
	f64 ps0a = PS0A;
	PS0D = PS0C;
	PS1D = ps0a + PS1B;
}

GekkoIntOp(PS_SEL)
{
	if(PS0A >= 0.0)
		PS0D = PS0C;
	else
		PS0D = PS0B;

	if(PS1A >= 0.0)
		PS1D = PS1C;
	else
		PS1D = PS1B;
}
////////////////////////////////////////////////////////////
