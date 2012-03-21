#ifndef CPU_Core_HPP
#define CPU_Core_HPP

#if EMU_PLATFORM == PLATFORM_WINDOWS
#include <windows.h>
#endif

#include "common.h"
#include "config.h"

typedef void(*GekkoFP)(void);
typedef void GekkoF;

//Generic CPU Core interface class
class GekkoCPU
{
public:
	virtual GekkoF ExecuteInstruction();

protected:
	void*	hGekkoThread;

	static void*	PipeHandle;
	static u8 *		PipeHandleData;
	static u32		PipeIsClient;

public:
	static const f32	ldScale[64];
	static const f32	stScale[64];

	static bool	is_on;
	static bool	loaded;
	static bool	pause;
	static bool is_dec;
	static bool	is_sc;
	static bool is_reserved;

	static bool DumpOp0;
	static bool PauseOnUnknownOp;

	static u32		reserved_addr;
	static u8		mode;

	static bool		step;
	static u32		opcode;

	static u32		PClast;

	static u32*	pPC;

    static optable		GekkoCPUOpsGroup4XO0Table[0x400];
	static optable		GekkoCPUOpsGroup4Table[0x20];
	static optable		GekkoCPUOpsGroup19Table[0x400];
	static optable		GekkoCPUOpsGroup31Table[0x400];
	static optable		GekkoCPUOpsGroup59Table[0x400];
	static optable		GekkoCPUOpsGroup63XO0Table[0x400];
	static optable		GekkoCPUOpsGroup63Table[0x20];
	static optable		GekkoCPUOpset[0x40];

// Gekko Exceptions
//

	typedef enum
	{
		GEX_RES0	= 0x80000000,
		GEX_RESET	= 0x80000100,
		GEX_CHECK	= 0x80000200,
		GEX_DSI		= 0x80000300,
		GEX_ISI		= 0x80000400,
		GEX_EXT		= 0x80000500,
		GEX_ALIGN	= 0x80000600,
		GEX_PROG	= 0x80000700,
		GEX_NFPU	= 0x80000800,
		GEX_DEC		= 0x80000900,

		GEX_RESA	= 0x80000A00,	// Reserved
		GEX_RESB	= 0x80000B00,	// Reserved

		GEX_SCALL	= 0x80000C00,
		GEX_TRACE	= 0x80000D00,
		GEX_RESE	= 0x80000E00,

		GEX_PERF	= 0x80000F00,
		GEX_BREAK	= 0x80001300,

		GEX_THERM	= 0x80001700,	// Thermal meltdown, destruction imminent..

		GEX_LAST	= 0x80008000,	// last entry

	} tGekkoException;

	typedef enum
	{
		Unknown = 0,
		Interpreter = 1,
		DynaRec = 2
	} CPUType;

    typedef struct
	{
		u32		Position;
		optable	OpPtr;
	} OpData;

	GekkoCPU();
	virtual ~GekkoCPU();
	virtual CPUType	GetCPUType();
	virtual GekkoF Open(u32 EntryPoint);
	virtual GekkoF Start();
	virtual GekkoF Halt();
	virtual GekkoF Exception(tGekkoException which);
	virtual u32 GetTicksPerSecond();

	GekkoF StartPipe(u32 IsClient);
	void SendPipeData(void *Data, u32 DataLen);
	u32 IsCPUCompareActive();
	GekkoF ComparePipeData(u32 LastAddress);
	void HandleSpecialPipeData();
	virtual GekkoF DumpInternalData(u32 DumpPC, u32 DumpLen);

	//functions defined by the core class
	u64 GetTicks();
	virtual GekkoF execStep();

	static inline unsigned long Gekko_Rotl(u32 X, u32 Y)
	{
	#if(0)
		u32	RetVal = 0;
		_asm
		{
			mov eax, X
			mov ecx, Y
			rol eax, cl
			mov RetVal, eax
		}

		return RetVal;
	#else
		return ( X << Y ) | ( X >> ( 32 - Y ) );
	#endif
	}

	static inline f32 dequantize(u32 data, s32 type, u8 scale)
	{
		f32 flt;

		switch(type)
		{
			case 4: flt = (f32)(u8)data; break;     // U8
			case 5: flt = (f32)(u16)data; break;    // U16
			case 6: flt = (f32)(s8)data; break;     // S8
			case 7: flt = (f32)(s16)data; break;    // S16
			case 0: 
			default: flt = *((f32 *)&data); return flt; //note return, not break!
		}

		return flt * ldScale[scale];
	}

	static inline u32 quantize(f32 data, s32 type, u8 scale)
	{
		u32 uval;

		if(!type)
			return *(u32 *)&data;

		data *= stScale[scale];
		switch(type)
		{
			case 4:                                 // U8
				if(data < 0)
					data = 0;
				else if(data > 255)
					data = 255;
				uval = (u8)(u32)data; break;
			case 5:                                 // U16
				if(data < 0)
					data = 0;
				else if(data > 65535)
					data = 65535;
				uval = (u16)(u32)data; break;
			case 6:                                 // S8
				if(data < -128)
					data = -128;
				else if(data > 127)
					data = 127;
				uval = (s8)(u8)(s32)(u32)data; break;
			case 7:                                 // S16
				if(data < -32768)
					data = -32768;
				else if(data > 32767)
					data = 32767;
				uval = (s16)(u16)(s32)(u32)data; break;
			default: *((float *)&uval) = data; break;
		}

		return uval;
	}

	static inline u32 Gekko_Bit( u32 X, u32 Y )
	{
		return ( ( X & ( BIT_0 >> Y ) ) >> (31 - Y) );
	}
};

//#define	GEKKO_CLOCK			( 486000000u )
#define	GEKKO_CLOCK         (common::g_config->powerpc_frequency() * 1000000u)

#define GEKKO_INT_HALT			0
#define GEKKO_INT_RUN			1
#define GEKKO_INT_RUN_STEP		2

extern class GekkoCPU*	cpu;

////////////////////////////////////////////////////////////

// Gekko Opcode Decoding
//

#define OPCODE		( cpu->opcode ) //cpu.opcode )
#define AA			( OPCODE >> 1 ) & 1
#define AALK		( OPCODE & 0x3 )
#define BD			( OPCODE & 0xFFFC )
#define BI			( ( OPCODE >> 16 ) & 0x1F )
#define BO			( ( OPCODE >> 21 ) & 0x1F )
#define CRBA		( ( OPCODE >> 16 ) & 0x1F )
#define CRBB		( ( OPCODE >> 11 ) & 0x1F )
#define CRBD		( ( OPCODE >> 21 ) & 0x1F )
#define CRFD		( ( OPCODE >> 23 ) & 0x7 )
#define CRFS		( ( OPCODE >> 18 ) & 0x7 )
#define CRM			( ( OPCODE >> 12 ) & 0xFF )
#define D0			( (s16) ( OPCODE & 0xFFFF ) )
#define D1			( OPCODE & 0xFFF )
#define FM			( ( OPCODE >> 17 ) & 0xFF )
#define frA			( ( OPCODE >> 16 ) & 0x1F )
#define frB			( ( OPCODE >> 11 ) & 0x1F )
#define frC			( ( OPCODE >> 6 ) & 0x1F )
#define frD			( ( OPCODE >> 21 ) & 0x1F )
#define frS			( ( OPCODE >> 21 ) & 0x1F )
#define I0			( ( OPCODE >> 12 ) & 0x7 )
#define I1			( ( OPCODE >> 7 ) & 0x7 )
#define IMM			( ( OPCODE >> 12 ) & 0xF )
#define L			( ( OPCODE >> 21 ) & 0x1 )
#define LI			( OPCODE & 0x03FFFFFC )
#define LK			( OPCODE & 1 )
#define MB			( ( OPCODE >> 6 ) & 0x1F )
#define ME			( ( OPCODE >> 1 ) & 0x1F )
#define NB			( ( OPCODE >> 11 ) & 0x1F )
#define OE			( ( OPCODE >> 10 ) & 0x1 )
#define OPCD		( OPCODE >> 26 )
#define rA			( ( OPCODE >> 16 ) & 0x1F )
#define rB			( ( OPCODE >> 11 ) & 0x1F )
#define rC			( ( OPCODE >> 6 ) & 0x1F )
#define RC			( OPCODE & 1 )
#define rD			( ( OPCODE >> 21 ) & 0x1F )
#define rS			( ( OPCODE >> 21 ) & 0x1F )
#define SH			( ( OPCODE >> 11 ) & 0x1F )
#define SIMM		( (s16)(OPCODE & 0xFFFF) )
#define SR			( ( OPCODE >> 16 ) & 0xF )
#define fspr		( ( OPCODE >> 11 ) & 0x3FF )
#define tbr			( ( rB << 5) | rA )
#define TO			( ( OPCODE >> 21 ) & 0x1F )
#define UIMM		( (u16) OPCODE )
#define XO0			( ( OPCODE >> 1 ) & 0x3FF )
#define XO1			( ( OPCODE >> 1 ) & 0x1FF )
#define XO2			( ( OPCODE >> 1 ) & 0x3F )
#define XO3			( ( OPCODE >> 1 ) & 0x1F )

#define PSI			( ( OPCODE >> 12 ) & 0x7 )
#define PSI_X		( ( OPCODE >> 7 ) & 0x7 )
#define PSW			( ( OPCODE >> 15 ) & 0x1 )
#define PSW_X		( ( OPCODE >> 10 ) & 0x1 )
#define PSIMM		( EXTS( ( OPCODE & 0xFFF ), 12 ) )

#define OE_T		0x200
#define OE_F		0x0

// Gekko Register Access
//

#define EXTS(X,n)					((s32)((X) << (32 - (n))) >> (32 - (n)))
#define EXTS8(X)					((s32)(s8) (X))
#define EXTS16(X)					((s32)(s16) (X))

#define I_GPR	1
#define I_SP	I_GPR

#define I_XER	1
#define I_LR	8
#define I_CTR	9
#define I_DEC	22
#define I_SRR0	26
#define I_SRR1	27
#define I_TBL	284
#define I_TBU	285
#define I_HID0	1008
#define I_HID1	1009
#define I_HID2	920
#define I_GQR	912
#define I_DMAU	922
#define I_WPAR  921
#define I_DMAL	923

#define I_IBAT0U 528
#define I_IBAT0L 529
#define I_IBAT1U 530
#define I_IBAT1L 531
#define I_IBAT2U 532
#define I_IBAT2L 533
#define I_IBAT3U 534
#define I_IBAT3L 535
#define I_DBAT0U 536
#define I_DBAT0L 537
#define I_DBAT1U 538
#define I_DBAT1L 539
#define I_DBAT2U 540
#define I_DBAT2L 541
#define I_DBAT3U 542
#define I_DBAT3L 543

#define MSR_BIT_LE			0x00000001
#define MSR_BIT_RI			0x00000002
#define MSR_BIT_DR			0x00000010
#define MSR_BIT_IR			0x00000020
#define MSR_BIT_IP			0x00000040
#define MSR_BIT_FE1			0x00000100
#define MSR_BIT_BE			0x00000200
#define MSR_BIT_SE			0x00000400
#define MSR_BIT_FE0			0x00000800
#define MSR_BIT_ME			0x00001000
#define MSR_BIT_FP			0x00002000
#define MSR_BIT_PR			0x00004000
#define MSR_BIT_EE			0x00008000
#define MSR_BIT_ILE			0x00010000
#define MSR_BIT_POW			0x00040000

u32 ireg_PC();
void set_ireg_PC(u32 NewPC);
u32 ireg_gpr(u32 Register);
void set_ireg_gpr(u32 Register, u32 NewVal);
#define ireg_GPR		ireg_gpr
#define set_ireg_GPR	set_ireg_gpr
u32 ireg_spr(u32 Register);
void set_ireg_spr(u32 Register, u32 NewVal);
u32 ireg_CR();
void set_ireg_CR(u32 NewVal);
u32 ireg_MSR();
void set_ireg_MSR(u32 NewVal);
f64 ireg_fpr_64(u32 Register, u32 ps);
void set_ireg_fpr_64(u32 Register, u32 ps, f64 NewVal);
f32 ireg_fpr_32(u32 Register, u32 ps);
void set_ireg_fpr_32(u32 Register, u32 ps, f32 NewVal);
void set_ireg_CR(u32 NewVal);
u32 ireg_MSR();
u32 ireg_FPSCR();
void set_ireg_FPSCR(u32 NewVal);
u32 ireg_IC();
void set_ireg_IC(u32 NewVal);
u32 ireg_sr(u32 Register);
void set_ireg_sr(u32 Register, u32 NewVal);
void ValidateRegLayout();

#endif
