#ifndef CPU_Core_Regs_HPP
#define CPU_Core_Regs_HPP

#include "common.h"
#include "memory.h"
#include "cpu_core.h"

//union for the 64bit timer
typedef union _Gekko_Timer
{
	u64		TBR;
	struct
	{
		u32	TBL;
		u32	TBU;
	};
} Gekko_Timer;

typedef struct t_Gekko_Registers
{
	union
	{
		struct
		{
#include "cpu_core_regs_layout.h"
		};
		u32		all[(32*4)+33+1024+16+1+1+1+1+1+2+8];	//extra 8 are alignment
	};
}Gekko_Registers; 

extern Gekko_Registers	ireg;
extern Gekko_Registers	iregBackup;

////////////////////////////////////////////////////////////

// Gekko Special Purpose Registers
//

#define SP		ireg.gpr[I_GPR] // GPR 1
#define XER		ireg.spr[I_XER] // SPR 1
#define LR		ireg.spr[I_LR] // SPR 8
#define CTR		ireg.spr[I_CTR]	// SPR 9
#define DEC		ireg.spr[I_DEC]	// SPR 22
#define SRR0	ireg.spr[I_SRR0] // SPR 26
#define SRR1	ireg.spr[I_SRR1] // SPR 27
//#define TBL		ireg.spr[I_TBL] // SPR 284
//#define TBU		ireg.spr[I_TBU] // SPR 285
#define HID0	ireg.spr[I_HID0] // SPR 1008
#define HID1	ireg.spr[I_HID1] // SPR 1009
#define HID2	ireg.spr[I_HID2] // SPR 920
#define GQR(X)	ireg.spr[I_GQR + (X & 7)] // SPR 912+
#define DMAU 	ireg.spr[I_DMAU] // SPR 922
#define DMAL 	ireg.spr[I_DMAL] // SPR 923

#define IBAT0U  (ireg.spr[I_IBAT0U])
#define IBAT0L  (ireg.spr[I_IBAT0L])
#define IBAT1U  (ireg.spr[I_IBAT1U])
#define IBAT1L  (ireg.spr[I_IBAT1L])
#define IBAT2U  (ireg.spr[I_IBAT2U])
#define IBAT2L  (ireg.spr[I_IBAT2L])
#define IBAT3U  (ireg.spr[I_IBAT3U])
#define IBAT3L  (ireg.spr[I_IBAT3L])
#define DBAT0U  (ireg.spr[I_DBAT0U])
#define DBAT0L  (ireg.spr[I_DBAT0L])
#define DBAT1U  (ireg.spr[I_DBAT1U])
#define DBAT1L  (ireg.spr[I_DBAT1L])
#define DBAT2U  (ireg.spr[I_DBAT2U])
#define DBAT2L  (ireg.spr[I_DBAT2L])
#define DBAT3U  (ireg.spr[I_DBAT3U])
#define DBAT3L  (ireg.spr[I_DBAT3L])

////////////////////////////////////////////////////////////

// 32bit General Purpose
#define GPR(x)		( ireg.gpr[x] )
#define RRA			( GPR(rA) )
#define RRB			( GPR(rB) )
#define RRD			( GPR(rD) )
#define RRS			( GPR(rS) )

// 64bit Floating Point
#define PS0(x)		( ireg.fpr[x].ps0._f64 )
#define PS1(x)		( ireg.fpr[x].ps1._f64 )
#define PS0_B(x)	( ireg.fpr[x].ps0._u64 )
#define PS1_B(x)	( ireg.fpr[x].ps1._u64 )

#define FPRA		( PS0(frA) )
#define FPRB		( PS0(frB) )
#define FPRC		( PS0(frC) )
#define FPRD		( PS0(frD) )
#define FPRS		( PS0(frS) )

#define FBRA		( PS0_B(frA) )
#define FBRB		( PS0_B(frB) )
#define FBRC		( PS0_B(frC) )
#define FBRD		( PS0_B(frD) )
#define FBRS		( PS0_B(frS) )

// 64bit Paired Singles

#define PS0D		PS0( rD )
#define PS1D		PS1( rD )
#define PS0BD		PS0_B( rD )
#define PS1BD		PS1_B( rD )

#define PS0S		PS0( rS )
#define PS1S		PS1( rS )
#define PS0BS		PS0_B( rS )
#define PS1BS		PS1_B( rS )

#define PS0A		PS0( rA )
#define PS1A		PS1( rA )
#define PS0BA		PS0_B( rA )
#define PS1BA		PS1_B( rA )

#define PS0B		PS0( rB )
#define PS1B		PS1( rB )
#define PS0BB		PS0_B( rB )
#define PS1BB		PS1_B( rB )

#define PS0C		PS0( rC )
#define PS1C		PS1( rC )
#define PS0BC		PS0_B( rC )
#define PS1BC		PS1_B( rC )

////////////////////////////////////////////////////////////

// Gekko Common Routines
//

#define XER_SO					( XER & ( 1 << 31 ) )
#define XER_OV					( XER & ( 1 << 30 ) )
#define XER_CA					( XER & ( 1 << 29 ) )

#define SET_XER_CA				(XER |=  (1 << 29))
#define RESET_XER_CA			(XER &= ~(1 << 29))

#define MSR_MASK				0xCEFF03E1

#define FPSCR_FX				( ireg.FPSCR & BIT_0 )
#define FPSCR_FEX				( ireg.FPSCR & BIT_1 )
#define FPSCR_VX				( ireg.FPSCR & BIT_2 )
#define FPSCR_OX				( ireg.FPSCR & BIT_3 )
#define FPSCR_UX				( ireg.FPSCR & BIT_4 )
#define FPSCR_ZX				( ireg.FPSCR & BIT_5 )
#define FPSCR_XX				( ireg.FPSCR & BIT_6 )
#define FPSCR_VXSNAN			( ireg.FPSCR & BIT_7 )
#define FPSCR_VXISI				( ireg.FPSCR & BIT_8 )
#define FPSCR_VXIDI				( ireg.FPSCR & BIT_9 )
#define FPSCR_VXZDZ				( ireg.FPSCR & BIT_10 )
#define FPSCR_VXIMZ				( ireg.FPSCR & BIT_11 )
#define FPSCR_VXVC				( ireg.FPSCR & BIT_12 )
#define FPSCR_FR				( ireg.FPSCR & BIT_13 )
#define FPSCR_FI				( ireg.FPSCR & BIT_14 )
#define FPSCR_FPRF				( ireg.FPSCR & ( BIT_15 | BIT_16 | BIT_17 |  BIT_18 | BIT_19 ) )
#define FPSCR_C					( ireg.FPSCR & BIT_15 )
#define FPSCR_FPCC				( ireg.FPSCR & ( BIT_16 | BIT_17 |  BIT_18 | BIT_19 ) )
#define FPSCR_LT				( ireg.FPSCR & BIT_16 )
#define FPSCR_GT				( ireg.FPSCR & BIT_17 )
#define FPSCR_EQ				( ireg.FPSCR & BIT_18 )
#define FPSCR_FU				( ireg.FPSCR & BIT_19 )
#define FPSCR_VXSOFT			( ireg.FPSCR & BIT_21 )
#define FPSCR_VXSQRT			( ireg.FPSCR & BIT_22 )
#define FPSCR_VXCVI				( ireg.FPSCR & BIT_23 )
#define FPSCR_VE				( ireg.FPSCR & BIT_24 )
#define FPSCR_OE				( ireg.FPSCR & BIT_25 )
#define FPSCR_UE				( ireg.FPSCR & BIT_26 )
#define FPSCR_ZE				( ireg.FPSCR & BIT_27 )
#define FPSCR_NI				( ireg.FPSCR & BIT_29 )
#define FPSCR_RN				( ireg.FPSCR & ( BIT_30 | BIT_31 ) )

#define HID2_LSQE				(1 << 31)
#define HID2_WPE				(1 << 30)
#define HID2_PSE				(1 << 29)
#define HID2_LCE				(1 << 28)
#define HID2_DMAQL				((HID2 >> 4) & 0xf)
#define PSE						(HID2 & HID2_PSE)

#define DMA_T					(DMAL & 2)
#define DMA_F					(DMAL & 1)
#define	DMA_LD					((DMAL >> 4) & 1)
#define DMA_LEN					(((DMAU & 0x1f) << 2) | ((DMAL >> 2) & 3))
#define DMA_RAM_ADDR			(DMAU &~ 0x1f)
#define DMA_L2C_ADDR			(DMAL &~ 0x1f)

#define GQR_LD_SCALE(x)  	( (GQR(x) >> 24) & 0x3F ) 
#define GQR_LD_TYPE(x)  	( (GQR(x) >> 16) & 0x7 ) 
#define GQR_ST_SCALE(x) 	( (GQR(x) >> 8) & 0x3F )
#define GQR_ST_TYPE(x)  	( GQR(x) & 0x7 )

static inline void * NEW_PC_PTR()
{
	return &Mem_RAM[ireg.PC & RAM_MASK];
}

#endif