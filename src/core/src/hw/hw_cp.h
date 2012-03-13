// hw_CP.h
// (c) 2005,2008 Gekko Team

#ifndef _HW_CP_H_
#define _HW_CP_H_

////////////////////////////////////////////////////////////////////////////////

#ifndef MEM_NATIVE_LE32
# define REGCP16(X)			(*((u16 *) &CPRegisters[REG_SIZE - (X & REG_MASK) - 2]))
# define REGCP32(X)			(*((u32 *) &CPRegisters[REG_SIZE - (X & REG_MASK) - 4]))
#else
# define REGCP16(X)			(*((u16 *) &CPRegisters[X & REG_MASK]))
# define REGCP32(X)			(*((u32 *) &CPRegisters[X & REG_MASK]))
#endif

extern u8 CPRegisters[REG_SIZE];

////////////////////////////////////////////////////////////////////////////////

#define CP_SR					0xCC000000	// Status Register
#define CP_CR					0xCC000002	// Control Register
#define CP_CLEAR				0xCC000004	// Clear Register
#define CP_TOKEN				0xCC00000E	// Token Register
#define CP_BOUNDLEFT			0xCC000010	// Bounding Box Left
#define CP_BOUNDRIGHT			0xCC000012	// Bounding Box Right
#define CP_BOUNDTOP				0xCC000014	// Bounding Box Top
#define CP_BOUNDBOTTOM			0xCC000016	// Bounding Box Bottom
#define CP_FIFOBASELO			0xCC000020	// FIFO Base Lo
#define CP_FIFOBASEHI			0xCC000022	// FIFO Base Hi
#define CP_FIFOENDLO			0xCC000024	// FIFO End Lo
#define CP_FIFOENDHI			0xCC000026	// FIFO End Hi
#define CP_FIFOHIGHWATERLO		0xCC000028	// FIFO Watermark Lo
#define CP_FIFOHIGHWATERHI		0xCC00002A	// FIFO Watermark Hi
#define CP_FIFOLOWWATERLO		0xCC00002C	// FIFO Watermark Lo
#define CP_FIFOLOWWATERHI		0xCC00002E	// FIFO Watermark Hi
#define CP_FIFOREADWRITEDISTLO	0xCC000030	// FIFO Read/Write Distance Lo
#define CP_FIFOREADWRITEDISTHI	0xCC000032	// FIFO Read/Write Distance Hi
#define CP_FIFOWRITELO			0xCC000034	// FIFO Write Pointer Lo
#define CP_FIFOWRITEHI			0xCC000036	// FIFO Write Pointer Hi
#define CP_FIFOREADLO			0xCC000038	// FIFO Read Pointer Lo
#define CP_FIFOREADHI			0xCC00003A	// FIFO Read Pointer Hi
#define CP_FIFOBPLO				0xCC00003C	// FIFO Breakpoint Lo
#define CP_FIFOBPHI				0xCC00003E	// FIFO Breakpoint Hi

#define CP_FIFO_DISTANCE        REGCP32(CP_FIFOREADWRITEDISTLO)
#define CP_FIFO_BASE            REGCP32(CP_FIFOBASELO)
#define CP_FIFO_END             REGCP32(CP_FIFOENDLO)
#define CP_FIFO_WRITE_POINTER   REGCP32(CP_FIFOWRITELO)
#define CP_FIFO_READ_POINTER    REGCP32(CP_FIFOREADLO)

#define CP_SR_BPINT				(1 << 4)
#define CP_SR_GPIDLECMD			(1 << 3)
#define CP_SR_GPIDLEREAD		(1 << 2)
#define CP_SR_GXUNDERFLOW		(1 << 1)
#define CP_SR_GXOVERFLOW		(1 << 0)

#define CP_CR_BPENABLE			(1 << 5)
#define CP_CR_GPLINKENABLE		(1 << 4)
#define CP_CR_UNDERFLOWENABLE	(1 << 3)
#define CP_CR_OVERFLOWENABLE	(1 << 2)
#define CP_CR_BPCLEAR			(1 << 1)
#define CP_CR_GPREAD			(1 << 0)

#define CP_CLEAR_UNDERFLOW		(1 << 1)
#define CP_CLEAR_OVERFLOW		(1 << 0)

#define CP_WPAR_MASK            0xfff0

typedef void					(__fastcall *wwpar)(u32 addr, u32 data);

////////////////////////////////////////////////////////////////////////////////

typedef struct t_sCP
{
	int		gp_link_enable;			// 1 - gp fifo attached, 0 - cpu fifo attached
	u32     wpar_address;
}sCP;

extern sCP commandprocessor;

////////////////////////////////////////////////////////////////////////////////

void CP_Open(void);
void CP_Update(u32 _addr);

////////////////////////////////////////////////////////////////////////////////

u8		__fastcall	CP_Read8(u32 addr);
void	__fastcall	CP_Write8(u32 addr, u32 data);
u16		__fastcall	CP_Read16(u32 addr);
void	__fastcall	CP_Write16(u32 addr, u32 data);
u32		__fastcall	CP_Read32(u32 addr);
void	__fastcall	CP_Write32(u32 addr, u32 data);

extern wwpar CP_WPAR_Write8;
extern wwpar CP_WPAR_Write16;
extern wwpar CP_WPAR_Write32;

//

#endif
