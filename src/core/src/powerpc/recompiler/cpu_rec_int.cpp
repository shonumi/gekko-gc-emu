// cpu_int_opcodes.cpp

////////////////////////////////////////////////////////////

#include "cpu_rec.h"
#include "powerpc/disassembler/ppc_disasm.h"
#include "powerpc/cpu_core_regs.h"
#include "hw/hw_cp.h"
#include "hle/hle.h"

#include "emu_dbg.h"

#undef SET_XER_CA
#undef RESET_XER_CA
#define SET_XER_CA				(XER_CARRY = 1)
#define RESET_XER_CA			(XER_CARRY = 0)

////////////////////////////////////////////////////////////

	inline void GekkoCPURecompiler::Gekko_CalculateCr0( u32 X )
	{
	#ifdef USE_INLINE_ASM
		_asm
		{
			cmp X, 0
			setl CR[0]
			setg CR[4]
			setz CR[8]

			cmp XER_SUMMARYOVERFLOW, 1
			sete CR[12]
		};
	#else
		CR[0] = CR[1] = CR[2] = CR[3] = 0;
	
		if( (s32) X < 0 ) CR[0] = 1;
		else if( (s32) X > 0 ) CR[1] = 1;
		else CR[2] = 1;

		if( XER_SO ) CR[3] = 1;
	#endif
	}

	inline void GekkoCPURecompiler::Gekko_CalculateCompare_Uint( u32 x, u32 y, u32 B )
	{
		CR[(4 * B)] = CR[(4 * B) + 1] = CR[(4 * B) + 2] = CR[(4 * B) + 3] = 0;
		
		if( x < y ) CR[(4 * B)] = 1;
		else if( x > y ) CR[(4 * B) + 1] = 1;
		else if( x == y ) CR[(4 * B) + 2] = 1;

		if( XER_SO ) CR[(4 * B) + 3] = 1;
	}

	inline void GekkoCPURecompiler::Gekko_CalculateCompare_Sint( s32 x, s32 y, u32 B )
	{
		CR[(4 * B)] = CR[(4 * B) + 1] = CR[(4 * B) + 2] = CR[(4 * B) + 3] = 0;
		
		if( x < y ) CR[(4 * B)] = 1;
		else if( x > y ) CR[(4 * B) + 1] = 1;
		else if( x == y ) CR[(4 * B) + 2] = 1;

		if( XER_SO ) CR[(4 * B) + 3] = 1;
	}

	inline void GekkoCPURecompiler::Gekko_CalculateCompareFloat( f64 x, f64 y, u32 B )
	{
		CR[(4 * B)] = CR[(4 * B) + 1] = CR[(4 * B) + 2] = CR[(4 * B) + 3] = 0;
		
		if( x < y ) CR[(4 * B)] = 1;
		else if( x > y ) CR[(4 * B) + 1] = 1;
		else if( x == y ) CR[(4 * B) + 2] = 1;

		if( XER_SO ) CR[(4 * B) + 3] = 1;
	}

	inline void GekkoCPURecompiler::Gekko_CalculateCr1( void )
	{
		CR[4] = CR[5] = CR[6] = CR[7] = 0;
		if(FPSCR_FX) CR[4] = 1;
		if(FPSCR_FEX) CR[5] = 1;
		if(FPSCR_VX) CR[6] = 1;
		if(FPSCR_OX) CR[7] = 1;
	}

	inline void GekkoCPURecompiler::Gekko_CalculateXerSoOv( u64 X )
	{
		if( ( X >= 0x100000000 ) ) 
		{
			XER_OVERFLOW = 1;			// Set Overflow Bit
			XER_SUMMARYOVERFLOW = 1;	// Set Summary Bit
		}else
		{
			XER_OVERFLOW = 0;			// Reset Overflow Bit
		}
	}

	inline void GekkoCPURecompiler::Gekko_Carry(u32 _a, u32 _b, u32 _c)
	{
	#ifdef USE_INLINE_ASM
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
		if ((_a + _b) < _a) {
			XER_CARRY = 1;
			return;
		}
		if ((_a + _b + _c) < _c) {
			XER_CARRY = 1;
			return;
		}

		XER_CARRY = 0;
	#endif
	}
/*
	static inline u32 Gekko_IsBranchConditionTrue( u32 bit )
	{
		switch( ( BO >> 1 ) & 0xF )
		{
		case 0: CTR--; if( !bit && ( CTR ) ){ return -1; } return 0;
		case 1: CTR--; if( !bit && !( CTR ) ){ return -1; } return 0;
		case 2: if( !bit ){ return -1; } return 0;
		case 3: if( !bit ){ return -1; } return 0;
		case 4: CTR--; if( !bit && ( CTR ) ){ return -1; } return 0;
		case 5: CTR--; if( !bit && !( CTR ) ){ return -1; } return 0;
		case 6: if( bit ){ return -1; } return 0;
		case 7: if( bit ){ return -1; } return 0;
		case 8: CTR--; if( ( CTR ) ){ return -1; } return 0;
		case 9: CTR--; if( !( CTR ) ){ return -1; } return 0;
		case 10: return -1;
		case 11: return -1;
		case 12: CTR--; if( ( CTR ) ){ return -1; } return 0;
		case 13: CTR--; if( !( CTR ) ){ return -1; } return 0;
		case 14: return -1;
		case 15: return -1;
		}
	}
*/
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

#define GEKKO_CARRY			(XER_CARRY)		

#define GEKKO_OPC_ERR(str)			\
{									\
	MessageBox( NULL,			\
		str,						\
		"Error",					\
		MB_ICONEXCLAMATION | MB_OK	\
	);								\
}

////////////////////////////////////////////////////////////
// Desc: Misc Opcodes
//

GekkoRecIntOp(DUMP_OPS)
{
	//dump the last 256k of ops to a debug file
	cpu->pause = true;
	printf("WARNING: CPU Interpreter: Tripped Opcode 0\n");
}

GekkoRecIntOp(HLE)
{
	u32	x;

	HLEFuncPtr ExecuteFunctionHLE;
    ExecuteFunctionHLE = g_hle_func_table[HLE_PC & (MAX_HLE_FUNCTIONS-1)];

	for(x = 0; x < 32; x++)
		ireg.CR = (ireg.CR << 1) | CR[x];

	XER = (XER_SUMMARYOVERFLOW << 31) | (XER_OVERFLOW << 30) |
		(XER_CARRY << 29) | XER_WORDINDEX;

	ExecuteFunctionHLE();

	for(x = 32; x > 0; x--)
	{
		CR[x-1] = ireg.CR & 1;
		ireg.CR >>= 1;
	}

	XER_SUMMARYOVERFLOW = (XER >> 31);
	XER_OVERFLOW = (XER >> 30) & 1;
	XER_CARRY = (XER >> 29) & 1;
	XER_WORDINDEX = (XER & 0x3F);
}

GekkoRecIntOp(NI)
{
	char opcodeStr[32], operandStr[32];
	u32 target;

	DisassembleGekko(opcodeStr, operandStr, cpu->opcode, ireg.PC, &target);
	printf("WARNING: CPU DynaRec Interpreter: Unimplemented Instruction \n\nOpcode %d (ext: %d) (pc=%08X, op=%08X, oldpc=%08X) instr=%s \n",cpu->opcode>>26,((cpu->opcode>>1)&0x3FF),ireg.PC,cpu->opcode,OldPC,opcodeStr);

	if(PauseOnUnknownOp)
		pause = true;

#pragma todo("fixme... shouldnt have direct tie ins to debugger here")
	// if(dbg.active) dbgSetCursor(cpu->PClast);

	branch = true;
}

////////////////////////////////////////////////////////////
// Desc: Branch Opcodes
//

GekkoRecIntOp(B)
{
	ireg.PC += EXTS(LI, 26);
}

GekkoRecIntOp(BL)
{
	LR = ireg.PC + 4;
	ireg.PC += EXTS(LI, 26);
}

GekkoRecIntOp(BA)
{
	ireg.PC = EXTS(LI, 26);
}

GekkoRecIntOp(BLA)
{
	LR = ireg.PC + 4;
	ireg.PC = EXTS(LI, 26);
}

GekkoRecIntOp(BX)
{
	cpu->PClast = ireg.PC;
	optable Gekko_Opset_BX[4] = {GekkoRecInt(B), GekkoRecInt(BL), GekkoRecInt(BA), GekkoRecInt(BLA) };
	Gekko_Opset_BX[ AALK ]();
//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = true;
}

GekkoRecIntOp(BCX)
{
#if 0//#ifdef USE_INLINE_ASM
	_asm
	{
		mov ecx, edx
		mov PClast, edx
		mov eax, 1
		mov ebx, ireg.CR
		shr ecx, 16
		and cl, 1Fh
		inc cl
		ror eax, cl
		and ebx, eax

		push edx
		push ebx
		call Gekko_IsBranchConditionTrue
		add sp, 4

		cmp eax, 0
		pop eax
		je BCX_SkipPCUpdate

		bt eax, 1
		jnc BCX_SkipLRUpdate

		mov ebx, ireg.PC
		add ebx, 4
		mov ireg.spr + (I_LR * 4), ebx

	BCX_SkipLRUpdate:

		mov ecx, eax
		and eax, 0xFFFC
		cwde
		sub eax, 4

		bt ecx, 1
		jc BCX_DontAddToPC

		add ireg.PC, eax
		jmp BCX_SkipPCUpdate

	BCX_DontAddToPC:
		mov ireg.PC, eax

BCX_SkipPCUpdate:
	};
#else
	cpu->PClast = ireg.PC;
	u32 B = CR[BI];

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
#endif

//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = true;
}

GekkoRecIntOp(BCCTRX)
{
	cpu->PClast = ireg.PC;
	u32 B = CR[BI];

	if((((BO >> 4) | (B == ((BO >> 3) & 1)))) & 1)
	{
		if(LK) LR = ireg.PC + 4;
		ireg.PC = ( CTR & ~3 );
	}
	else
		ireg.PC += 4;

//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = true;
}

GekkoRecIntOp(BCLRX)
{
	cpu->PClast = ireg.PC;
	u32 lr;
	u32 B = CR[BI];

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
	branch = true;
}

// Desc: Fixed Point Opcodes
//
GekkoRecIntOp(ADD)
{
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDO)
{
	Gekko_CalculateXerSoOv((u64)RRA + RRB);
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDC)
{
	Gekko_Carry(RRA, RRB, 0);
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDCO)
{
	Gekko_Carry(RRA, RRB, 0);
	Gekko_CalculateXerSoOv((u64)RRA + RRB);
	RRD = RRA + RRB;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, RRB, ca);
	RRD = RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, RRB, ca);
	Gekko_CalculateXerSoOv((u64)RRA + RRB + ca);
	RRD = RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDI)
{
	if(rA)	RRD = RRA + SIMM;
	else	RRD = SIMM;
}

GekkoRecIntOp(ADDIC)
{
	Gekko_Carry(RRA, SIMM, 0);
	RRD = RRA + SIMM;
}

GekkoRecIntOp(ADDICD)
{
	Gekko_Carry(RRA, SIMM, 0);
	RRD = RRA + SIMM;
	Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDIS)
{
	if(rA)	RRD = RRA + ((u32)SIMM << 16);
	else	RRD = ((u32)SIMM << 16);
}

GekkoRecIntOp(ADDME)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA - 1, ca, 0);
	RRD = RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(ADDMEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA - 1, ca, 0);
	Gekko_CalculateXerSoOv((u64)RRA + ca + 0xffffffff);
	RRD = RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(ADDZE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, ca, 0);
	RRD = RRA + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(ADDZEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(RRA, ca, 0);
	Gekko_CalculateXerSoOv((u64)RRA + ca);
	RRD = RRA + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(AND)
{
	RRA = RRS & RRB;
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoRecIntOp(ANDC)
{
	RRA = RRS & ~RRB;
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoRecIntOp(ANDID)
{
	RRA = RRS & UIMM;
	Gekko_CalculateCr0(RRA);
}

GekkoRecIntOp(ANDISD)
{
	u16 uimm = UIMM;
	RRA = RRS & ((u32)UIMM << 16);
	Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(CMP)
{
	int crfD = CRFD;
	Gekko_CalculateCompare_Sint((s32)RRA, (s32)RRB, crfD);
}

GekkoRecIntOp(CMPI)
{
	s16 simm = SIMM;
	int crfD = CRFD;
	Gekko_CalculateCompare_Sint((s32)RRA, (s32)simm, crfD );
}

GekkoRecIntOp(CMPL)
{
	int crfD = CRFD;
	Gekko_CalculateCompare_Uint(RRA, RRB, crfD);
}

GekkoRecIntOp(CMPLI)
{
	u16 uimm = UIMM;
	int crfD = CRFD;
	Gekko_CalculateCompare_Uint(RRA, uimm, crfD);
}

GekkoRecIntOp(CNTLZW)
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

GekkoRecIntOp(CRAND)
{
	if (CR[CRBA] & CR[CRBB])
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(CRANDC)
{
	if (CR[CRBA] & ~CR[CRBB])
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(CREQV)
{
	if ((~CR[CRBA] ^ CR[CRBB]) & 1)
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(CRNAND)
{
	if (~CR[CRBA] & CR[CRBB])
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(CRNOR)
{
	if (~(CR[CRBA] | CR[CRBB]) & 1)
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(CROR)
{
	if (CR[CRBA] | CR[CRBB])
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(CRORC)
{
	if ((CR[CRBA] | ~CR[CRBB]) & 1)
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(CRXOR)
{
	if (CR[CRBA] ^ CR[CRBB])
		CR[CRBD] = 1;
	else
		CR[CRBD] = 0;
}

GekkoRecIntOp(DCBF)
{
	//Data Cache Block Flush
}

GekkoRecIntOp(DCBI)
{
	if(ireg.MSR & MSR_BIT_POW)
	{
		cpu->Exception(GEX_PROG);
	}
//	branch = true;
}

GekkoRecIntOp(DCBST)
{
	//Data Cache Block Store
}

GekkoRecIntOp(DCBT)
{
	//Data Cache Block Touch
}

GekkoRecIntOp(DCBTST)
{
	//Data Cache Block Touch for Store
}

GekkoRecIntOp(DCBZ)
{
	//Data Cache Block to Zero
}

GekkoRecIntOp(DCBZ_L)
{
	//Data Cache Block to Zero Locked
}

GekkoRecIntOp(DIVW)
{
	s32 rra = (s32)RRA, rrb = (s32)((RRB)?(RRB):1);
	RRD = (u32)(s32)(rra / rrb);
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(DIVWO)
{
	s32 rra = (s32)RRA, rrb = (s32)((RRB)?(RRB):1);
	Gekko_CalculateXerSoOv((u64)((s64)rra / rrb));
	RRD = (u32)(s32)(rra / rrb);
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(DIVWU)
{
	u32 rra = RRA, rrb = ((RRB)?(RRB):1);
	RRD = (u32)(rra / rrb);
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(DIVWUO)
{
	u32 rra = RRA, rrb = ((RRB)?(RRB):1);
	Gekko_CalculateXerSoOv(((u64)rra / rrb));
	RRD = (u32)(rra / rrb);
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(ECIWX){}

GekkoRecIntOp(ECOWX){}

GekkoRecIntOp(EIEIO){}

GekkoRecIntOp(EQV)
{
	RRA = ~(RRS ^ RRB);
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoRecIntOp(EXTSB)
{
	RRA = EXTS8(RRS);
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoRecIntOp(EXTSH)
{
	RRA = EXTS16(RRS);
	if(RC) Gekko_CalculateCr0(RRA);
}

GekkoRecIntOp(ICBI)
{
	//Instruction Cache Block Invalidate
}

GekkoRecIntOp(ISYNC)
{
	//Instruction Syncronize
}

GekkoRecIntOp(MCRF)
{
	CR[(CRFD * 4) + 0] = CR[(CRFS * 4) + 0];
	CR[(CRFD * 4) + 1] = CR[(CRFS * 4) + 1];
	CR[(CRFD * 4) + 2] = CR[(CRFS * 4) + 2];
	CR[(CRFD * 4) + 3] = CR[(CRFS * 4) + 3];
}

GekkoRecIntOp(MCRFS)
{
	GEKKO_OPC_ERR("Interpreter Error: MCRFS Unimplemented.");
}

GekkoRecIntOp(MCRXR)
{
	CR[(CRFD * 4) + 0] = XER_SUMMARYOVERFLOW;
	CR[(CRFD * 4) + 1] = XER_OVERFLOW;
	CR[(CRFD * 4) + 2] = XER_CARRY;
	CR[(CRFD * 4) + 3] = 0;
	XER_CARRY = 0;
	XER_OVERFLOW = 0;
	XER_SUMMARYOVERFLOW = 0;
}

GekkoRecIntOp(MFCR)
{
	u32 NewCR = 0;
	u32 x;

	for(x = 0; x < 32; x++)
		NewCR = (NewCR << 1) | CR[x];

	RRD = NewCR;
}

GekkoRecIntOp(MFMSR)
{
	RRD = ireg.MSR;
}

GekkoRecIntOp(MFSPR)
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

		case I_XER:
			RRD = (XER_SUMMARYOVERFLOW << 31) | (XER_OVERFLOW << 30) |
				  (XER_CARRY << 29) | XER_WORDINDEX;
			break;

		default:
			RRD = ireg.spr[reg];
			break;

	}
}

GekkoRecIntOp(MFSR)
{
	RRD = ireg.sr[rA & 0xf];
}

GekkoRecIntOp(MFSRIN)
{
	RRD = ireg.sr[RRB >> 28];
}

GekkoRecIntOp(MFTB)
{
	if(tbr == 268) RRD = ireg.TBR.TBL;
	else if(tbr == 269) RRD = ireg.TBR.TBU;
	else cpu->Exception(GekkoCPU::GEX_PROG);
}

GekkoRecIntOp(MTCRF)
{
	u32 crm = 0;
	s32 x;

	crm = ((CRM & 0x80) ? 0xf0000000 : 0) | ((CRM & 0x40) ? 0x0f000000:0) | ((CRM & 0x20) ? 0x00f00000 : 0) | ((crm & 0x10 )? 0x000f0000 : 0) |
	      ((CRM & 0x08) ? 0x0000f000 : 0) | ((CRM & 0x04) ? 0x00000f00:0) | ((CRM & 0x02) ? 0x000000f0 : 0) | ((crm & 0x01) ? 0x0000000f : 0);

	for(x = 0; x < 32; x++)
	{
		//if the bit is set then update with RRS, else, leave it alone
		if(crm & 0x80000000)
			CR[x] = (RRS >> (31-x)) & 1;
		crm <<= 1;
	}
}

GekkoRecIntOp(MTMSR)
{
	ireg.MSR = RRS;
}

GekkoRecIntOp(MTSPR)
{
	u32 dma_len;
	u32 reg = ((rB << 5) | rA);
	u32 OldVal;

	OldVal = ireg.spr[reg];
	ireg.spr[reg] = RRS;

	switch(reg)
	{
	case I_XER:
		XER_SUMMARYOVERFLOW = (RRS >> 31);
		XER_OVERFLOW = (RRS >> 30) & 1;
		XER_CARRY = (RRS >> 29) & 1;
		XER_WORDINDEX = (RRS & 0x3F);
		break;

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

			u32	i;
			u32	Val;

			if(DMA_LD)
			{
				for(i = 0; i < (dma_len >> 2); i++)
				{
					Val = Memory_Read32(DMA_RAM_ADDR + (i * 4));
					Memory_Write32(DMA_L2C_ADDR + (i * 4), Val);
				}
			}else{
				for(i = 0; i < (dma_len >> 2); i++)
				{
					Val = Memory_Read32(DMA_L2C_ADDR + (i * 4));
					Memory_Write32(DMA_RAM_ADDR + (i * 4), Val);
				}
			}
		}
		break;

	case I_WPAR:
		CP_Update(RRS);
		break;	

	case I_GQR:
	case (I_GQR + 1):
	case (I_GQR + 2):
	case (I_GQR + 3):
	case (I_GQR + 4):
	case (I_GQR + 5):
	case (I_GQR + 6):
	case (I_GQR + 7):
		if(OldVal != ireg.spr[reg])
			printf("I_GQR[%d] changed %08X to %08X\n", reg & 7, OldVal, ireg.spr[reg]);
	}
}

GekkoRecIntOp(MTSR)
{
	ireg.sr[rA & 0xf] = RRS;
}

GekkoRecIntOp(MTSRIN)
{
	ireg.sr[RRB >> 28] = RRS;
}

GekkoRecIntOp(MULHW)
{
	u64 temp = (s64)RRA * (s64)RRB;
	RRD = (u32)( temp >> 32 );
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(MULHWU)
{
	u64 temp = (u64)RRA * (u64)RRB;
	RRD = (u32)( temp >> 32 );
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(MULLI)
{
	RRD = (s32)RRA * SIMM;
}

GekkoRecIntOp(MULLW)
{
	RRD = (s32)RRA * (s32)RRB;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(MULLWO)
{
	Gekko_CalculateXerSoOv((s64)RRA * (s32)RRB );
	RRD = (s32)RRA * (s32)RRB;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(NAND)
{
	RRA = ~( RRS & RRB );
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(NEG)
{
	RRD = ~RRA + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(NEGO)
{
	Gekko_CalculateXerSoOv( (u64)~RRA + 1 );
	RRD = ~RRA + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(NOR)
{
	RRA = ~( RRS | RRB );
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(OR)
{
	RRA = RRS | RRB;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(ORC)
{
	RRA = RRS | ~RRB;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(ORI)
{
	RRA = RRS | UIMM;
}

GekkoRecIntOp(ORIS)
{
	RRA = RRS | ( UIMM << 16 );
}

GekkoRecIntOp(RFI)
{
	//leave MSR bits 1-4,10-12,14-15,24,28-29 alone
	//turn msr bit 13 off
	//copy ssr1 bits 0,5-9,16-23,25-27,30-31 to msr
	ireg.MSR = (ireg.MSR & ~0x87C4FF73) | (SRR1 & 0x87C0FF73);
	ireg.PC = (SRR0 & ~3);

//	cpu->pPC = (u32*)NEW_PC_PTR();
	branch = 1 | Branch_RFI;
}

GekkoRecIntOp(RLWIMI)
{
	u32 m = RotMask[ MB ][ ME ];
	u32 r = Gekko_Rotl( RRS, SH );
	RRA = ( r & m ) | ( RRA & ~m );
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(RLWINM)
{
	u32 m = RotMask[ MB ][ ME ];
	u32 r = Gekko_Rotl( RRS, SH );
	RRA = r & m;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(RLWNM)
{
	u32 m = RotMask[ MB ][ ME ];
	u32 r = Gekko_Rotl( RRS, SH);
	RRA = r & m;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(SC)
{
	ireg.PC += 4;
	cpu->Exception(GekkoCPU::GEX_SCALL);
	branch |= Branch_SC;
}

GekkoRecIntOp(SLW)
{
    if( RRB & 0x20 ) RRA = 0;
    else RRA = RRS << ( RRB & 0x1F );
    if(RC) Gekko_CalculateCr0( RRA );
}
 
GekkoRecIntOp(SRAW)
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
 
GekkoRecIntOp(SRAWI)
{
	u32 sh = SH, mask = BIT_0;
	u32 rra;

    if( sh == 0 ) { rra = RRS; RESET_XER_CA; }
    else
    {
        rra = (s32)RRS >> sh;
        if((s32)RRS < 0 && (RRS << (32 - sh)) != 0) SET_XER_CA; else RESET_XER_CA;
    }

	RRA = rra;

    if(RC) Gekko_CalculateCr0(RRA);
}
 
GekkoRecIntOp(SRW)
{
    if( RRB & 0x20 ) RRA = 0;
    else RRA = RRS >> ( RRB & 0x1F );
    if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(SUBF)
{
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFO)
{
	Gekko_CalculateXerSoOv(( (u64)~RRA + RRB + 1 ) );
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFC)
{
	Gekko_Carry(~RRA, (RRB+1), 0);
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFCO)
{
	Gekko_Carry(~RRA, (RRB + 1), 0);
	Gekko_CalculateXerSoOv((u64)~RRA + RRB + 1);
	RRD = ~RRA + RRB + 1;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, RRB, ca);
	RRD = ~RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0(RRD);
}

GekkoRecIntOp(SUBFEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, RRB, ca);
	Gekko_CalculateXerSoOv((u64)~RRA + RRB + ca);
	RRD = ~RRA + RRB + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFIC)
{
    Gekko_Carry(~RRA , (SIMM + 1), 0);
	RRD = ~RRA + SIMM + 1;
}

GekkoRecIntOp(SUBFME)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA - 1, ca, 0);
	RRD = ~RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFMEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA - 1, ca, 0);
	Gekko_CalculateXerSoOv((u64)~RRA + ca + 0xffffffff);
	RRD = ~RRA + ca + 0xffffffff;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFZE)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, ca, 0);
	RRD = ~RRA + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SUBFZEO)
{
	u8 ca = GEKKO_CARRY;
	Gekko_Carry(~RRA, ca, 0);
	Gekko_CalculateXerSoOv((u64)~RRA + ca);
	RRD = ~RRA + ca;
	if(RC) Gekko_CalculateCr0( RRD );
}

GekkoRecIntOp(SYNC)
{
	//Sync
}

GekkoRecIntOp(TLBIA)
{
	//Translation Lookaside Buffer Invalidate All
}

GekkoRecIntOp(TLBIE)
{
	//Translation Lookaside Buffer Invalidate Entry
}

GekkoRecIntOp(TLBSYNC)
{
	//Translation Lookaside Buffer Sync
}

GekkoRecIntOp(TW){}

GekkoRecIntOp(TWI){}

GekkoRecIntOp(XOR)
{
	RRA = RRS ^ RRB;
	if(RC) Gekko_CalculateCr0( RRA );
}

GekkoRecIntOp(XORI)
{
	RRA = RRS ^ UIMM;
}

GekkoRecIntOp(XORIS)
{
	RRA = RRS ^ ( UIMM << 16 );
}

////////////////////////////////////////////////////////////
// Desc: Load/Store Opcodes
//

GekkoRecIntOp(LBZ)
{
	RRD = (rA) ? Memory_Read8( RRA + SIMM ) : Memory_Read8( SIMM );
}

GekkoRecIntOp(LBZU)
{
	u32 X = RRA + SIMM;
	RRD = Memory_Read8( X );
	RRA = X;
}

GekkoRecIntOp(LBZUX)
{
	u32 X = RRA + RRB;
	RRD = Memory_Read8( X );
	RRA = X;
}

GekkoRecIntOp(LBZX)
{
	RRD = (rA) ? Memory_Read8( RRA + RRB ) : Memory_Read8( RRB );
}

GekkoRecIntOp(LFD)
{
	FBRD = (rA) ? Memory_Read64( RRA + SIMM ) : Memory_Read64( SIMM );
}

GekkoRecIntOp(LFDU)
{
	FBRD = Memory_Read64( ( RRA += SIMM ) );
}

GekkoRecIntOp(LFDUX)
{
	FBRD = Memory_Read64( ( RRA += RRB ) );
}

GekkoRecIntOp(LFDX)
{
	FBRD = (rA) ? Memory_Read64( RRA + RRB ) : Memory_Read64( RRB );
}

GekkoRecIntOp(LFS)
{
	t32 temp;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	temp._u32 = (rA) ? Memory_Read32( RRA + SIMM ) : Memory_Read32( SIMM );
	if(PSE)
	{
		PS0D = PS1D = (double)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoRecIntOp(LFSU)
{
	t32 temp;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	temp._u32 = Memory_Read32( ( RRA += SIMM ) );
	if(PSE)
	{
		PS0D = PS1D = (double)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoRecIntOp(LFSUX)
{
	t32 temp;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	temp._u32 = Memory_Read32( RRA += RRB );
	if(PSE)
	{
		PS0D = PS1D = (f32)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoRecIntOp(LFSX)
{
	t32 temp;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	temp._u32 = (rA) ? Memory_Read32( RRA + RRB ) : Memory_Read32( RRB );
	if(PSE)
	{
		PS0D = PS1D = (f32)temp._f32;
	}else{
		FPRD = (double)temp._f32;
	}
}

GekkoRecIntOp(LHA)
{
	RRD = EXTS16((rA) ? Memory_Read16( RRA + SIMM ) : Memory_Read16( SIMM ));
}

GekkoRecIntOp(LHAU)
{
	u32 X = RRA + SIMM;
	RRD = EXTS16(Memory_Read16( X ));
	RRA = X;
}

GekkoRecIntOp(LHAUX)
{
	u32 X = RRA + RRB;
	RRD = EXTS16(Memory_Read16( X ));
	RRA = X;
}

GekkoRecIntOp(LHAX)
{
	RRD = EXTS16((rA) ? Memory_Read16( RRA + RRB ) : Memory_Read16( RRB ));
}

GekkoRecIntOp(LHBRX)
{
	RRD = BSWAP16( (rA) ? Memory_Read16( RRA + RRB ) : Memory_Read16( RRB ) );
}

GekkoRecIntOp(LHZ)
{
	RRD = (rA) ? Memory_Read16( RRA + SIMM ) : Memory_Read16( SIMM );
}

GekkoRecIntOp(LHZU)
{
	u32 X = RRA + SIMM;
	RRD = Memory_Read16( X );
	RRA = X;
}

GekkoRecIntOp(LHZUX)
{
	u32 X = RRA + RRB;
	RRD = Memory_Read16( X );
	RRA = X;
}

GekkoRecIntOp(LHZX)
{
	RRD = (rA) ? Memory_Read16( RRA + RRB ) : Memory_Read16( RRB );
}

GekkoRecIntOp(LMW)
{
	u32 ea = ( rA ) ? ( RRA + SIMM ) : SIMM;

	for(int i = rD; i < 32; i++, ea += 4 )
		ireg.gpr[i] = Memory_Read32(ea);
}

GekkoRecIntOp(LSWI)
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

GekkoRecIntOp(LSWX)
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

GekkoRecIntOp(LWARX)
{
	u32 EA = RRB;
	if(rA)
		EA += RRA;
	RRD = Memory_Read32(EA);
	is_reserved = 1;
	reserved_addr = EA;
}

GekkoRecIntOp(LWBRX)
{
	RRD = BSWAP32( (rA) ? Memory_Read32( RRA + RRB ) : Memory_Read32( RRB ) );
}

GekkoRecIntOp(LWZ)
{
	RRD = (rA) ? Memory_Read32( RRA + SIMM ) : Memory_Read32( SIMM );
}

GekkoRecIntOp(LWZU)
{
	u32 X = RRA + SIMM;
	RRD = Memory_Read32( X );
	RRA = X;
}

GekkoRecIntOp(LWZUX)
{
	u32 X = RRA + RRB;
	RRD = Memory_Read32( X );
	RRA = X;
}

GekkoRecIntOp(LWZX)
{
	RRD = (rA) ? Memory_Read32( RRA + RRB ) : Memory_Read32( RRB );
}

////////////////////////////////////////////////////////////

GekkoRecIntOp(STB)
{
	if(rA) Memory_Write8( RRA + SIMM, RRS );
	else Memory_Write8( SIMM, RRS );
}

GekkoRecIntOp(STBU)
{
	u32 X = RRA + SIMM;
	Memory_Write8( X, RRS );
	RRA = X;
}

GekkoRecIntOp(STBUX)
{
	u32 X = RRA + RRB;
	Memory_Write8( X , RRS );
	RRA = X;
}

GekkoRecIntOp(STBX)
{
	if(rA) Memory_Write8( RRA + RRB, RRS );
	else Memory_Write8( RRB, RRS );
}

GekkoRecIntOp(STFD)
{
	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	if(rA) Memory_Write64( RRA + SIMM, FBRS );
	else Memory_Write64( SIMM, FBRS );
}

GekkoRecIntOp(STFDU)
{
	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	Memory_Write64( ( RRA += SIMM ) , FBRS );
}

GekkoRecIntOp(STFDUX)
{
	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	Memory_Write64( ( RRA += RRB ) , FBRS );
}

GekkoRecIntOp(STFDX)
{
	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	if(rA) Memory_Write64( RRA + RRB, FBRS );
	else Memory_Write64( RRB, FBRS );
}

GekkoRecIntOp(STFIWX)
{
	if(rA) Memory_Write32( RRA + RRB, *(u32 *)&FPRS );
	else Memory_Write32( RRB, *(u32 *)&FPRS );
}

GekkoRecIntOp(STFS)
{
	t32 data;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	data._f32 = (f32)FPRS;
	if(rA) Memory_Write32( RRA + SIMM, data._u32);
	else Memory_Write32( SIMM, data._u32);
}

GekkoRecIntOp(STFSU)
{
	t32 data;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	data._f32 = (f32)FPRS;
	Memory_Write32( RRA += SIMM, data._u32 );
}

GekkoRecIntOp(STFSUX)
{
	t32 data;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	data._f32 = (f32)FPRS;
	Memory_Write32( RRA += RRB, data._u32);
}

GekkoRecIntOp(STFSX)
{
	t32 data;

	if(!(ireg.MSR & MSR_BIT_FP))
	{
//		cpu->Exception(GekkoCPU::GEX_NFPU);
//		return;
	}

	data._f32 = (f32)FPRS;
	if(rA) Memory_Write32( RRA + RRB, data._u32 );
	else Memory_Write32( RRB, data._u32 );
}

GekkoRecIntOp(STH)
{
	if(rA) Memory_Write16( RRA + SIMM, RRS );
	else Memory_Write16( SIMM, RRS );
}

GekkoRecIntOp(STHBRX) 
{
	if(rA) Memory_Write16( RRA + RRB, BSWAP16( RRS & 0xFFFF ) );
	else Memory_Write16( RRB, BSWAP16( RRS & 0xFFFF ) );
}

GekkoRecIntOp(STHU)
{
	u32 X = RRA + SIMM;
	Memory_Write16( X , RRS );
	RRA = X;
}

GekkoRecIntOp(STHUX)
{
	u32 X = RRA + RRB;
	Memory_Write16( X , RRS );
	RRA = X;
}

GekkoRecIntOp(STHX)
{
	if(rA) Memory_Write16( RRA + RRB, RRS );
	else Memory_Write16( RRB, RRS );
}

GekkoRecIntOp(STMW)
{
	u32 ea = ( rA ) ? ( RRA + SIMM ) : SIMM;

	for(int i = rS; i < 32; i++, ea += 4 )
	{
		Memory_Write32( ea, ireg.gpr[i] );
	}
}

GekkoRecIntOp(STSWI)
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

GekkoRecIntOp(STSWX)
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

GekkoRecIntOp(STW)
{
	if(rA) Memory_Write32( RRA + SIMM, RRS );
	else   Memory_Write32( SIMM, RRS );
}

GekkoRecIntOp(STWBRX) 
{
	if(rA) Memory_Write32( RRA + RRB, BSWAP32( RRS ) );
	else Memory_Write32( RRB, BSWAP32( RRS ) );
}

GekkoRecIntOp(STWCX)
{
	u32	EA;
	if(is_reserved)
	{
		EA = RRB;
		if(rA)
			EA += RRA;
		Memory_Write32(EA, RRS);
		is_reserved = 0;
		CR[29] = 1;
	}
	else
		CR[29] = 0;
}

GekkoRecIntOp(STWU)
{
	u32 X = RRA + SIMM;
	Memory_Write32( X, RRS );
	RRA = X;
}

GekkoRecIntOp(STWUX)
{
	u32 X = RRA + RRB;
	Memory_Write32( X, RRS );
	RRA = X;
}

GekkoRecIntOp(STWX)
{
	if(rA) Memory_Write32( RRA + RRB, RRS );
	else Memory_Write32( RRB, RRS );
}

GekkoRecIntOp(PSQ_L)
{
    u32 data0, data1;
    int type = GQR_LD_TYPE(PSI);
    int scale = (u8)GQR_LD_SCALE(PSI);

    u32 addr = (rA) ? (RRA + PSIMM) : PSIMM;

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

	switch(type)
	{
	case 6:
		if(data0 & 0x80)
			data0 |= 0xffffff00;
		break;
	case 7:
		if(data0 & 0x8000)
			data0 |= 0xffff0000;
		break;
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
			case 6:
				data1 = Memory_Read8(addr+1);
				break;

			case 5:
			case 7:
				data1 = Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

		switch(type)
		{
		case 6:
			if(data1 & 0x80)
				data1 |= 0xffffff00;
			break;
		case 7:
			if(data1 & 0x8000)
				data1 |= 0xffff0000;
			break;
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }
}

GekkoRecIntOp(PSQ_LX)
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

	switch(type)
	{
	case 6:
		if(data0 & 0x80)
			data0 |= 0xffffff00;
		break;
	case 7:
		if(data0 & 0x8000)
			data0 |= 0xffff0000;
		break;
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
			case 6:
				data1 = Memory_Read8(addr+1);
				break;

			case 5:
			case 7:
				data1 = Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

		switch(type)
		{
		case 6:
			if(data1 & 0x80)
				data1 |= 0xffffff00;
			break;
		case 7:
			if(data1 & 0x8000)
				data1 |= 0xffff0000;
			break;
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }
}

GekkoRecIntOp(PSQ_LU)
{
    u32 data0, data1;
    int type = GQR_LD_TYPE(PSI);
    int scale = (u8)GQR_LD_SCALE(PSI);

    u32 addr = (rA) ? (RRA + PSIMM) : PSIMM;

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

	switch(type)
	{
	case 6:
		if(data0 & 0x80)
			data0 |= 0xffffff00;
		break;
	case 7:
		if(data0 & 0x8000)
			data0 |= 0xffff0000;
		break;
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
			case 6:
				data1 = Memory_Read8(addr+1);
				break;

			case 5:
			case 7:
				data1 = Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

		switch(type)
		{
		case 6:
			if(data1 & 0x80)
				data1 |= 0xffffff00;
			break;
		case 7:
			if(data1 & 0x8000)
				data1 |= 0xffff0000;
			break;
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }

	RRA = addr;
}

GekkoRecIntOp(PSQ_LUX)
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

	switch(type)
	{
	case 6:
		if(data0 & 0x80)
			data0 |= 0xffffff00;
		break;
	case 7:
		if(data0 & 0x8000)
			data0 |= 0xffff0000;
		break;
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
			case 6:
				data1 = Memory_Read8(addr+1);
				break;

			case 5:
			case 7:
				data1 = Memory_Read16(addr+2);
				break;

			default:
				data1 = Memory_Read32(addr+4);
		}

		switch(type)
		{
		case 6:
			if(data1 & 0x80)
				data1 |= 0xffffff00;
			break;
		case 7:
			if(data1 & 0x8000)
				data1 |= 0xffff0000;
			break;
		}

        PS1D = (f64)dequantize(data1, type, scale);
    }

	RRA = addr;
}

GekkoRecIntOp(PSQ_ST)
{
     int type = GQR_ST_TYPE(PSI);
     int scale = GQR_ST_SCALE(PSI);

     u32 addr = (rA) ? (RRA + PSIMM) : PSIMM;

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

	 if(!PSW)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }
}

GekkoRecIntOp(PSQ_STX)
{
     int type = GQR_ST_TYPE(PSI_X);
     int scale = GQR_ST_SCALE(PSI_X);

     u32 addr = (rA) ? (RRA + RRB) : RRB;

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

	 if(!PSW_X)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }
}

GekkoRecIntOp(PSQ_STU)
{
     int type = GQR_ST_TYPE(PSI);
     int scale = GQR_ST_SCALE(PSI);

	 u32 addr = (RRA + PSIMM);

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

	 if(!PSW)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }
	 RRA = addr;
}

GekkoRecIntOp(PSQ_STUX)
{
     int type = GQR_ST_TYPE(PSI_X);
     int scale = GQR_ST_SCALE(PSI_X);

     u32 addr = (rA) ? (RRA + RRB) : RRB;

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

	 if(!PSW_X)
     {
		switch(type)
		{
		case 4:
		case 6:
			Memory_Write8(addr + 1, quantize((f32)PS1S, type, scale));
			return;
		case 5:
		case 7:
			Memory_Write16(addr + 2, quantize((f32)PS1S, type, scale));
			return;
		}
        
		Memory_Write32(addr + 4, quantize((f32)PS1S, type, scale));
     }

	RRA = addr;
}

////////////////////////////////////////////////////////////
// Desc: Floating Point Opcodes
//

GekkoRecIntOp(FABS)
{
	FBRD = FBRB & ~SIGNED_BIT64;
}

GekkoRecIntOp(FADD)
{
	FPRD = FPRA + FPRB;
}

GekkoRecIntOp(FADDS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A + PS0B);
	}else{
		FPRD = (f32)(FPRA + FPRB);
	}
}

GekkoRecIntOp(FCMPO)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(FPRA, FPRB, crfD);
}

GekkoRecIntOp(FCMPU)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(FPRA, FPRB, crfD);
}

GekkoRecIntOp(FCTIW)
{
	FBRD = (u64)(u32)(s32)FPRB;
}

GekkoRecIntOp(FCTIWZ)
{
	FBRD = (u64)(u32)(s32)FPRB;
}

GekkoRecIntOp(FDIV)
{
	FPRD = FPRA / FPRB;
}

GekkoRecIntOp(FDIVS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A / PS0B);
	}else{
		FPRD = (f32)(FPRA / FPRB);
	}
}

GekkoRecIntOp(FMADD)
{
	FPRD = ( FPRA * FPRC ) + FPRB;
}

GekkoRecIntOp(FMADDS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)((PS0A * PS0C) + PS0B);
	}else{
		FPRD = (f32)((FPRA * FPRC) + FPRB);
	}
}

GekkoRecIntOp(FMR)
{
	FBRD = FBRB;
}

GekkoRecIntOp(FMSUB)
{
	FPRD = ( FPRA * FPRC ) - FPRB;
}

GekkoRecIntOp(FMSUBS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)((PS0A * PS0C) - PS0B);
	}else{
		FPRD = (f32)((FPRA * FPRC) - FPRB);
	}
}

GekkoRecIntOp(FMUL)
{
	FPRD = FPRA * FPRC;
}

GekkoRecIntOp(FMULS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A * PS0C);
	}else{
		FPRD = (f32)(FPRA * FPRC);
	}
}

GekkoRecIntOp(FNABS)
{
	FBRD = FBRB | SIGNED_BIT64;
}

GekkoRecIntOp(FNEG)
{
	FBRD = FBRB ^ SIGNED_BIT64;
}

GekkoRecIntOp(FNMADD)
{
	FPRD = -((FPRA * FPRC) + FPRB);
}

GekkoRecIntOp(FNMADDS)
{
	if(PSE)
	{
		PS0D = PS1D = (f32) -((PS0A * PS0C) + PS0B);
	}else{
		FPRD = (f32) -((FPRA * FPRC) + FPRB);
	}
}

GekkoRecIntOp(FNMSUB)
{
	FPRD = -((FPRA * FPRC) - FPRB);
}

GekkoRecIntOp(FNMSUBS)
{
	if(PSE)
	{
		PS0D = PS1D = (f32) -((PS0A * PS0C) - PS0B);
	}else{
		FPRD = (f32) -((FPRA * FPRC) - FPRB);
	}
}

GekkoRecIntOp(FRES)
{
	if(PSE)
	{
		PS0D = PS1D = (f32)(1/PS0B);
	}else{
		FPRD = (f32)(1/FPRB);
	}
}

GekkoRecIntOp(FRSP)
{
	if(PSE)
		PS0D = (float)FPRB;
	else
		FPRD = (float)FPRB;
}

GekkoRecIntOp(FRSQRTE)
{
	FPRD = 1.0 / sqrt(FPRB);
}

GekkoRecIntOp(FSEL)
{
	if(FPRA >= 0)
	{
		FPRD = FPRC;
	}else{
		FPRD = FPRB;
	}
}

GekkoRecIntOp(FSQRT)
{
	FPRD = sqrt(FPRB);	
}

GekkoRecIntOp(FSQRTS)
{
	FPRD = (f32)sqrt(FPRB);
}

GekkoRecIntOp(FSUB)
{
	FPRD = FPRA - FPRB;
}

GekkoRecIntOp(FSUBS)
{
	if(HID2 & HID2_PSE)
	{
		PS0D = PS1D = (f32)(PS0A - PS0B);
	}else{
		FPRD = (f32)(FPRA - FPRB);
	}
}

GekkoRecIntOp(MFFS)
{
	FBRD = ireg.FPSCR;
}

GekkoRecIntOp(MTFSB0)
{
	int crbD = CRBD;
	ireg.FPSCR &= ~( BIT_0 >> crbD );
}

GekkoRecIntOp(MTFSB1)
{
	int crbD = CRBD;
	ireg.FPSCR |= ( BIT_0 >> crbD );
}

GekkoRecIntOp(MTFSF)
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

GekkoRecIntOp(MTFSFI)
{
	int crfD = CRFD;
	ireg.FPSCR = ireg.FPSCR & ~( 0xF0000000 >> ( crfD * 4 ) );
	ireg.FPSCR = ireg.FPSCR | ( IMM << ( 31 - ( 4 * ( 7 - crfD ) ) ) );
}

////////////////////////////////////////////////////////////
// Desc: Paired Singles Opcodes
//

GekkoRecIntOp(PS_ABS)
{
	PS0D = abs(PS0B);
	PS1D = abs(PS1B);
}

GekkoRecIntOp(PS_ADD)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_CMPO0)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS0A, PS0B, crfD );
}

GekkoRecIntOp(PS_CMPO1)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS1A, PS1B, crfD );
}


GekkoRecIntOp(PS_CMPU0)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS0A, PS0B, crfD);
}

GekkoRecIntOp(PS_CMPU1)
{
	int crfD = CRFD;
	Gekko_CalculateCompareFloat(PS1A, PS1B, crfD);
}

GekkoRecIntOp(PS_MADD)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_MADDS0)
{
	//avoid rC == rD
	f64 ps0c = PS0C;
	PS0D = (f32)(( PS0A * ps0c ) + PS0B);
	PS1D = (f32)(( PS1A * ps0c ) + PS1B);
}

GekkoRecIntOp(PS_MADDS1)
{
	PS0D = (f32)(( PS0A * PS1C ) + PS0B);
	PS1D = (f32)(( PS1A * PS1C ) + PS1B);
}

GekkoRecIntOp(PS_MERGE00)
{
	f64 ps0a = PS0A, ps0b = PS0B;
	PS0D = ps0a;
	PS1D = ps0b;
}

GekkoRecIntOp(PS_MERGE01)
{
	PS0D = PS0A;
	PS1D = PS1B;
}

GekkoRecIntOp(PS_MERGE10)
{
	f64 ps0b = PS0B;
	PS0D = PS1A;
	PS1D = ps0b;
}

GekkoRecIntOp(PS_MERGE11)
{
	PS0D = PS1A;
	PS1D = PS1B;
}

GekkoRecIntOp(PS_MR)
{
	PS0D = PS0B;
	PS1D = PS1B;
}

GekkoRecIntOp(PS_MSUB)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_MUL)
{
#ifdef USE_INLINE_ASM_X86
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

		movapd xmm0, [eax]
		movapd xmm1, [ecx]

		mulpd xmm0, xmm1
		movapd [edx], xmm0
	};
#else
	PS0D = PS0A * PS0C;
	PS1D = PS1A * PS1C;
#endif
}

GekkoRecIntOp(PS_MULS0)
{
	//avoid rC == rD
	f64 ps0c = PS0C;
	PS0D = PS0A * ps0c;
	PS1D = PS1A * ps0c;
}

GekkoRecIntOp(PS_MULS1)
{
	PS0D = PS0A * PS1C;
	PS1D = PS1A * PS1C;
}

GekkoRecIntOp(PS_NEG)
{
#if 0 //def USE_INLINE_ASM
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

GekkoRecIntOp(PS_NMADD)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_NMSUB)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_RES)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_RSQRTE)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_SUB)
{
#if 0//#ifdef USE_INLINE_ASM
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

GekkoRecIntOp(PS_DIV)
{
	PS0D = PS0A / PS0B;
	PS1D = PS1A / PS1B;
}

GekkoRecIntOp(PS_SUM0)
{
	PS0D = PS0A + PS1B;
	PS1D = PS1C;
}

GekkoRecIntOp(PS_SUM1)
{
	//avoid rD == rA
	f64 ps0a = PS0A;
	PS0D = PS0C;
	PS1D = ps0a + PS1B;
}

GekkoRecIntOp(PS_SEL)
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
